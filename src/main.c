#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include "gb.h"

#define STDIN_FILENO 0

typedef struct {
    GapBuffer *buffer;
    bool dirty;
    bool quit_pending;
    char status_msg[128];
    char *filename;
    size_t screen_rows;
    size_t screen_cols;
    bool is_running;
    size_t row_offset;
    size_t col_offset;
} Editor;

typedef struct {
    int rows;
    int cols;
} WindowSize;

struct termios orig_termios;

void save_terminal_state() {
    tcgetattr(STDIN_FILENO, &orig_termios);
}

void raw_mode_restore() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void raw_mode_on() {
    struct termios raw = orig_termios;

    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);

    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void clear_terminal() {
    write(STDOUT_FILENO, "\x1b[H\x1b[2J", 7);
}

char read_key() {
    char c;
    if (read(STDIN_FILENO, &c, 1) == 1){}

    return c;
}

WindowSize get_window(void) {
    struct winsize ws;
    WindowSize size = {0, 0};

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        size.rows = 24;
        size.cols = 80;
    } else {
        size.rows = ws.ws_row;
        size.cols = ws.ws_col;
    }

    return size;
}

void init(Editor *editor){
    editor->buffer  = gb_init(NULL, 0);
    editor->dirty = false;
    editor->quit_pending = false;
    editor->is_running = true;
    editor->col_offset = 0;
    editor->row_offset = 0;

    WindowSize win = get_window();
    editor->screen_rows = win.rows;
    editor->screen_cols = win.cols;
}

void load_file(char *fpath[], Editor *editor){
    FILE *file = fopen(*fpath, "r");
    if (!file) {
        perror("failed to open file");
        return;
    }

    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    char *tmp = malloc(size);
    if (!tmp) {
        perror("failed to allocate memory");
        return;
    }

    if (fread(tmp, 1, size, file) != size) {
        perror("failed to read file");
        free(tmp);
        fclose(file);
        return;
    }

    fclose(file);

    if (editor->buffer) {
        gb_dest(editor->buffer);
    }
    editor->buffer = gb_init(tmp, size);

    free(tmp);

    editor->dirty = false;
    snprintf(editor->status_msg, sizeof(editor->status_msg), "Opened file: %s", *fpath);
}

void save_file(const char *fpath, Editor *editor) {
    if (!editor->buffer) return;

    size_t text_len = gb_length(editor->buffer);
    char *text = malloc(text_len);
    if (!text) {
        snprintf(editor->status_msg, sizeof(editor->status_msg), "save failed: out of memory");
        return;
    }

    gb_copy_text(editor->buffer, text, text_len);

    FILE *file = fopen(fpath, "w");
    if (!file) {
        snprintf(editor->status_msg, sizeof(editor->status_msg), "save failed: cannot open file");
        free(text);
        return;
    }

    if (fwrite(text, 1, text_len, file) != text_len) {
        snprintf(editor->status_msg, sizeof(editor->status_msg), "save failed: write error");
    } else {
        snprintf(editor->status_msg, sizeof(editor->status_msg), "file saved: %s", fpath);
        editor->dirty = false;
    }

    fclose(file);
    free(text);
}

void get_cursor_screen_pos(Editor *editor, size_t *out_row, size_t *out_col) {
    size_t cursor_index = gb_cursor(editor->buffer);
    size_t text_len = gb_length(editor->buffer);

    char *text = malloc(text_len);
    if (!text) {
        *out_row = 0;
        *out_col = 0;
        return;
    }

    gb_copy_text(editor->buffer, text, text_len);

    size_t row = 0;
    size_t col = 0;

    for (size_t i = 0; i < cursor_index && i < text_len; i++) {
        char c = text[i];

        if (c == '\n') {
            row++;
            col = 0;
        } else {
            col++;
            if (col == editor->screen_cols) {
                row++;
                col = 0;
            }
        }
    }

    free(text);

    *out_row = row;
    *out_col = col;
}

void draw_status_bar(Editor *editor, const char *filename) {
    char bar[editor->screen_cols + 1];
    memset(bar, ' ', sizeof(bar));

    // put filename and byte count on the left
    int n = snprintf(bar, sizeof(bar),
                     "  %s  |  %zu bytes",
                     filename ? filename : "[No Name]",
                     gb_length(editor->buffer));
    if (n > (int)editor->screen_cols) n = editor->screen_cols;

    // then put status message at the right if it fits
    if (editor->status_msg[0] != '\0') {
        size_t msg_len = strlen(editor->status_msg);
        if (msg_len > editor->screen_cols - 1) msg_len = editor->screen_cols - 1;
        // overwrite at left margin for simplicity
        memcpy(bar, editor->status_msg, msg_len);
    }

    write(STDOUT_FILENO, "\x1b[7m", 4);  // invert
    write(STDOUT_FILENO, bar, editor->screen_cols);
    write(STDOUT_FILENO, "\x1b[0m", 4);  // reset
}

void render(Editor *editor, const char *filename) {
    const char *CLEAR_SCREEN_ANSI = "\x1b[H\x1b[2J";
    write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, strlen(CLEAR_SCREEN_ANSI));

    size_t text_len = gb_length(editor->buffer);
    char *text = malloc(text_len);
    if (!text) return;

    gb_copy_text(editor->buffer, text, text_len);

    size_t row = 0, col = 0;

    size_t start_row = editor->row_offset;
    size_t end_row = start_row + editor->screen_rows - 1;

    for (size_t i = 0; i < text_len; i++) {
        char c = text[i];

        if (c == '\n' || col == editor->screen_cols) {
            if (row >= start_row && row < end_row) write(STDOUT_FILENO, "\r\n", 2);
            row++;
            col = 0;
            continue;
        }

        if (row >= start_row && row < end_row) write(STDOUT_FILENO, &c, 1);
        col++;
    }

    for (size_t r = row; r < end_row; r++) write(STDOUT_FILENO, "\r\n", 2);

    draw_status_bar(editor, filename);

    size_t screen_cursor_row, screen_cursor_col;
    get_cursor_screen_pos(editor, &screen_cursor_row, &screen_cursor_col);

    size_t total_rows = 0;
    for (size_t i = 0; i < gb_length(editor->buffer); i++)
        if (text[i] == '\n') total_rows++;

    if (editor->row_offset + editor->screen_rows > total_rows)
        editor->row_offset = total_rows > editor->screen_rows ? total_rows - editor->screen_rows : 0;

    char buf[32];
    int n = snprintf(buf, sizeof(buf), "\x1b[%zu;%zuH", screen_cursor_row + 1, screen_cursor_col + 1);
    write(STDOUT_FILENO, buf, n);

    free(text);
}

void handle_input(char key, Editor *editor, const char *filename) {
    bool needs_render = false;

    switch (key) {
        case '\x11': // Ctrl-Q
            if (!editor->quit_pending) {
                editor->quit_pending = true;
                snprintf(editor->status_msg, sizeof(editor->status_msg),
                         "Press Ctrl-Q again to quit.");
            } else {
                editor->is_running = false;
            }
            needs_render = true;
            break;

        case '\x13': // ctrl-s
            if (filename) save_file(filename, editor);
            else snprintf(editor->status_msg, sizeof(editor->status_msg), "No filename provided.");
            needs_render = true;
            break;

        case 'w':
            if (editor->row_offset > 0) editor->row_offset--;
            needs_render = true;
            break;

        case 's':
            editor->row_offset++;
            needs_render = true;
            break;

        default:
            editor->quit_pending = false;
            break;
    }

    if (needs_render) render(editor, filename);
}

int main(int argc, char *argv[]) {
    save_terminal_state();
    raw_mode_on();
    atexit(raw_mode_restore);

    const char *filename = NULL;
    filename = argv[1];

    Editor editor;
    init(&editor);
    render(&editor, filename);

    if (argc > 1) {
        load_file(&argv[1], &editor);
        render(&editor, filename);
    }

    while(editor.is_running){
        char key = read_key();
        handle_input(key, &editor, filename);
    }

    if (editor.buffer) gb_dest(editor.buffer);
    clear_terminal();

    return 0;
}
