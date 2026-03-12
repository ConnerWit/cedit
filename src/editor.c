#include <bits/posix2_lim.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "editor.h"
#include "cursor.h"
#include "clipboard.h"
#include "history.h"

#define ARROW_UP 1000
#define ARROW_DOWN 1001
#define ARROW_LEFT 1002
#define ARROW_RIGHT 1003
#define KEY_DELETE 1004


void editor_init(Editor *editor) {
    editor->buffer = gb_init(NULL, 0);
    editor->dirty = false;
    editor->quit_pending = false;
    editor->is_running = true;
    editor->row_offset = 0;
    editor->col_offset = 0;
    editor->filename = NULL;
    editor->status_msg[0] = '\0';

    WindowSize win = get_window();
    editor->screen_rows = win.rows;
    editor->screen_cols = win.cols;

    editor->mode = MODE_NORMAL;
    editor->visual_start = 0;
    clipboard_init(&editor->clipboard);
    history_init(&editor->history);
}

void editor_load_file(Editor *editor, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        snprintf(editor->status_msg, sizeof(editor->status_msg),
                 "failed to open file");
        return;
    }

    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    char *tmp = malloc(size);
    if (!tmp) {
        fclose(file);
        snprintf(editor->status_msg, sizeof(editor->status_msg),
                 "out of memory");
        return;
    }

    if (fread(tmp, 1, size, file) != size) {
        free(tmp);
        fclose(file);
        snprintf(editor->status_msg, sizeof(editor->status_msg),
                 "failed to read file");
        return;
    }

    fclose(file);

    if (editor->buffer)
        gb_dest(editor->buffer);

    editor->buffer = gb_init(tmp, size);
    free(tmp);

    editor->filename = strdup(filename);
    editor->dirty = false;

    snprintf(editor->status_msg, sizeof(editor->status_msg),
             "opened file: %s", filename);
}

void editor_save_file(Editor *editor) {
    if (!editor->filename) {
        snprintf(editor->status_msg, sizeof(editor->status_msg),
                 "no filename provided");
        return;
    }

    size_t text_len = gb_length(editor->buffer);
    char *text = malloc(text_len);
    if (!text) {
        snprintf(editor->status_msg, sizeof(editor->status_msg),
                 "save failed: out of memory");
        return;
    }

    gb_copy_text(editor->buffer, text, text_len);

    FILE *file = fopen(editor->filename, "w");
    if (!file) {
        free(text);
        snprintf(editor->status_msg, sizeof(editor->status_msg),
                 "save failed: cannot open file");
        return;
    }

    if (fwrite(text, 1, text_len, file) != text_len) {
        snprintf(editor->status_msg, sizeof(editor->status_msg),
                 "save failed: write error");
    } else {
        snprintf(editor->status_msg, sizeof(editor->status_msg),
                 "file saved: %s", editor->filename);
        editor->dirty = false;
    }

    fclose(file);
    free(text);
}

static void draw_status_bar(Editor *editor, const char *state) {
    char bar[editor->screen_cols];
    memset(bar, ' ', sizeof(bar));

    size_t n = snprintf(bar, sizeof(bar),
                     "  %s  |  %zu bytes | state: %s",
                     editor->filename ? editor->filename : "[No Name]",
                     gb_length(editor->buffer), state);

    if (n > (size_t)editor->screen_cols)
        n = editor->screen_cols;

    if (editor->status_msg[0] != '\0') {
        size_t msg_len = strlen(editor->status_msg);
        if (msg_len > editor->screen_cols)
            msg_len = editor->screen_cols;

        memcpy(bar, editor->status_msg, msg_len);
    }

    write(STDOUT_FILENO, "\x1b[7m", 4);
    write(STDOUT_FILENO, bar, editor->screen_cols);
    write(STDOUT_FILENO, "\x1b[0m", 4);
}

void editor_render(Editor *editor) {
    cursor_scroll(editor);

    write(STDOUT_FILENO, "\x1b[H\x1b[2J", 7);

    size_t text_len = gb_length(editor->buffer);
    char *text = malloc(text_len);
    if (!text) return;

    gb_copy_text(editor->buffer, text, text_len);

    size_t cursor_pos = gb_cursor(editor->buffer);
    size_t sel_start = 0, sel_end = 0;
    bool has_selection = (editor->mode == MODE_VISUAL);
    if (has_selection) {
        sel_start = editor->visual_start < cursor_pos ? editor->visual_start : cursor_pos;
        sel_end   = editor->visual_start < cursor_pos ? cursor_pos : editor->visual_start;
    }

    size_t row = 0, col = 0;
    size_t start_row = editor->row_offset;
    size_t end_row = start_row + editor->screen_rows - 1;
    bool in_highlight = false;

    for (size_t i = 0; i < text_len; i++) {
        char c = text[i];

        if (has_selection) {
            bool should_hl = (i >= sel_start && i < sel_end);
            if (should_hl && !in_highlight) {
                write(STDOUT_FILENO, "\x1b[7m", 4);
                in_highlight = true;
            } else if (!should_hl && in_highlight) {
                write(STDOUT_FILENO, "\x1b[0m", 4);
                in_highlight = false;
            }
        }

        if (c == '\n') {
            if (row >= start_row && row < end_row) {
                if (in_highlight) write(STDOUT_FILENO, "\x1b[0m", 4);
                write(STDOUT_FILENO, "\r\n", 2);
                if (in_highlight) write(STDOUT_FILENO, "\x1b[7m", 4);
            }
            row++;
            col = 0;
            continue;
        }

        if (col == editor->col_offset + (size_t)editor->screen_cols) {
            if (row >= start_row && row < end_row) {
                if (in_highlight) write(STDOUT_FILENO, "\x1b[0m", 4);
                write(STDOUT_FILENO, "\r\n", 2);
                if (in_highlight) write(STDOUT_FILENO, "\x1b[7m", 4);
            }
            row++;
            col = 0;
        }

        if (row >= start_row && row < end_row &&
            col >= editor->col_offset &&
            col < editor->col_offset + (size_t)editor->screen_cols) {
            write(STDOUT_FILENO, &c, 1);
        }

        col++;
    }

    if (in_highlight) write(STDOUT_FILENO, "\x1b[0m", 4);

    for (size_t r = row; r < end_row; r++)
        write(STDOUT_FILENO, "\r\n", 2);

    const char *state;
    switch (editor->mode) {
        case MODE_INSERT:   state = "INSERT"; break;
        case MODE_VISUAL:   state = "VISUAL"; break;
        default:            state = "NORMAL"; break;
    }

    draw_status_bar(editor, state);

    cursor_render(editor);

    free(text);
}

static void snapshot(Editor *editor) {
    size_t text_len = gb_length(editor->buffer);
    char *text = malloc(text_len);
    if (!text) return;
    gb_copy_text(editor->buffer, text, text_len);
    history_push(&editor->history, text, text_len, gb_cursor(editor->buffer));
    free(text);
}

static void maybe_snapshot(Editor *editor, UndoOpType op) {
    if (editor->history.last_op != op)
        snapshot(editor);
    editor->history.last_op = op;
}

static void editor_undo(Editor *editor) {
    char *text;
    size_t text_len, cursor_pos;

    if (!history_pop(&editor->history, &text, &text_len, &cursor_pos)) {
        snprintf(editor->status_msg, sizeof(editor->status_msg), "nothing to undo");
        return;
    }

    gb_dest(editor->buffer);
    editor->buffer = gb_init(text, text_len);
    free(text);

    gb_move_to(editor->buffer, cursor_pos);
    editor->dirty = true;
    editor->history.last_op = OP_NONE;

    snprintf(editor->status_msg, sizeof(editor->status_msg), "reverted");
}

static void visual_yank(Editor *editor) {
    size_t cursor_pos = gb_cursor(editor->buffer);
    size_t sel_start = editor->visual_start < cursor_pos ? editor->visual_start : cursor_pos;
    size_t sel_end   = editor->visual_start < cursor_pos ? cursor_pos : editor->visual_start;
    size_t sel_len   = sel_end - sel_start;

    if (sel_len == 0) return;

    size_t text_len = gb_length(editor->buffer);
    char *text = malloc(text_len);
    if (!text) return;

    gb_copy_text(editor->buffer, text, text_len);
    clipboard_set(&editor->clipboard, text + sel_start, sel_len);
    free(text);
}

static void visual_cut(Editor *editor) {
    size_t cursor_pos = gb_cursor(editor->buffer);
    size_t sel_start = editor->visual_start < cursor_pos ? editor->visual_start : cursor_pos;
    size_t sel_end   = editor->visual_start < cursor_pos ? cursor_pos : editor->visual_start;
    size_t sel_len   = sel_end - sel_start;

    if (sel_len == 0) return;

    maybe_snapshot(editor, OP_CUT);
    visual_yank(editor);

    gb_move_to(editor->buffer, sel_start);
    gb_delete(editor->buffer, sel_len);
    editor->dirty = true;
}

void editor_handle_insert_input(Editor *editor, int key) {
    bool needs_render = false;

    switch (key) {
        case 27: // esc key
            editor->mode = MODE_NORMAL; 
            needs_render = true;
            break;

        case '\r': //enter key
            maybe_snapshot(editor, OP_INSERT);
            gb_insert(editor->buffer, "\n", 1);
            editor->dirty = true;
            needs_render = true;
            break;

        case KEY_DELETE:
            maybe_snapshot(editor, OP_DELETE);
            gb_delete(editor->buffer, 1);
            editor->dirty = true;
            needs_render = true;
            break;

        case 127: // modern backspace ASCII code
        case 8: // old-school backspace code
            maybe_snapshot(editor, OP_DELETE);
            gb_backspace(editor->buffer, 1);
            needs_render = true;
            break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            cursor_move(editor, key);
            needs_render = true;
            break;

        default:
            if (key >= 32 && key <= 126) {
                maybe_snapshot(editor, OP_INSERT);
                const void *data = &key;
                gb_insert(editor->buffer, data, 1);
                editor->dirty = true;
                needs_render = true;
            }
            break;
    }

    if (needs_render)
        editor_render(editor);
}

void editor_handle_visual_input(Editor *editor, int key) {
    bool needs_render = false;

    switch (key) {
        case 27: // esc key
            editor->mode = MODE_NORMAL;
            needs_render = true;
            break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
        case 'h':
        case 'j':
        case 'k':
        case 'l':
            cursor_move(editor, key);
            needs_render = true;
            break;

        case 'c':
            visual_yank(editor);
            editor->mode = MODE_NORMAL;
            snprintf(editor->status_msg, sizeof(editor->status_msg), "yanked");
            needs_render = true;
            break;

        case 'x':
            visual_cut(editor);
            editor->mode = MODE_NORMAL;
            snprintf(editor->status_msg, sizeof(editor->status_msg), "cut");
            needs_render = true;
            break;

        default:
            editor->quit_pending = false;
            break;
    }

    if (needs_render)
        editor_render(editor);
}

void editor_handle_normal_input(Editor *editor, int key) {
    bool needs_render = false;

    switch (key) {
        case '\x11':  // ctrl-q
            if (!editor->quit_pending && editor->dirty) {
                editor->quit_pending = true;
                snprintf(editor->status_msg,
                         sizeof(editor->status_msg),
                         "you have unsaved changes. press ctrl-s to save or ctrl-q again to quit now ");
            } else {
                editor->is_running = false;
            }
            needs_render = true;
            break;

        case '\x13':  // ctrl-s
            editor_save_file(editor);
            needs_render = true;
            break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
        case 'h':
        case 'j':
        case 'k':
        case 'l':
            cursor_move(editor, key);
            needs_render = true;
            break;

        case 'i': // 'insert' key
            if (editor->mode == MODE_NORMAL) {
                editor->mode = MODE_INSERT;
            }
            needs_render = true;
            break;

        case 'v': // 'visual' key
            if (editor->mode == MODE_NORMAL) {
                editor->mode = MODE_VISUAL;
                editor->visual_start = gb_cursor(editor->buffer);
            }
            needs_render = true;
            break;

        case 'p':
            if (editor->clipboard.len > 0) {
                maybe_snapshot(editor, OP_PASTE);
                gb_insert(editor->buffer, editor->clipboard.data, editor->clipboard.len);
                editor->dirty = true;
                needs_render = true;
            }
            break;

        case 'r':
            editor_undo(editor);
            needs_render = true;
            break;

        default:
            editor->quit_pending = false;
            break;
    }

    if (needs_render)
        editor_render(editor);
}

void editor_destroy(Editor *editor) {
    if (editor->buffer)
        gb_dest(editor->buffer);

    if (editor->filename)
        free(editor->filename);

    clipboard_free(&editor->clipboard);
    history_free(&editor->history);
}
