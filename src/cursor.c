#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "editor.h"

#define ARROW_UP 1000
#define ARROW_DOWN 1001
#define ARROW_LEFT 1002
#define ARROW_RIGHT 1003


void get_cursor_screen_pos(Editor *editor,
                                  size_t *out_row,
                                  size_t *out_col) {
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

void cursor_move(Editor *editor, int dir) {
    size_t text_len = gb_length(editor->buffer);
    if (text_len == 0) return;

    char *text = malloc(text_len);
    if (!text) return;
    gb_copy_text(editor->buffer, text, text_len);

    size_t pos = gb_cursor(editor->buffer);

    switch (dir) {
        case ARROW_LEFT:
        case 'h':
            if (pos > 0)
                gb_move_left(editor->buffer, 1);
            break;

        case ARROW_RIGHT:
        case 'l':
            if (pos < text_len)
                gb_move_right(editor->buffer, 1);
            break;

        case ARROW_UP:
        case 'k': {
            // find start of current line
            size_t cur_line_start = 0;
            for (size_t i = 0; i < pos; i++)
                if (text[i] == '\n') cur_line_start = i + 1;

            size_t col = pos - cur_line_start;

            // find start of previous line
            if (cur_line_start == 0) break; // already on first line
            size_t prev_line_end = cur_line_start - 1; // the '\n'
            size_t prev_line_start = 0;
            for (size_t i = 0; i < prev_line_end; i++)
                if (text[i] == '\n') prev_line_start = i + 1;

            size_t prev_line_len = prev_line_end - prev_line_start;
            size_t target = prev_line_start + (col < prev_line_len ? col : prev_line_len);

            if (target < pos)
                gb_move_left(editor->buffer, pos - target);
            break;
        }

        case ARROW_DOWN:
        case 'j': {
            // find start of current line
            size_t cur_line_start = 0;
            for (size_t i = 0; i < pos; i++)
                if (text[i] == '\n') cur_line_start = i + 1;

            size_t col = pos - cur_line_start;

            // find end of current line (the '\n')
            size_t cur_line_end = pos;
            while (cur_line_end < text_len && text[cur_line_end] != '\n')
                cur_line_end++;

            if (cur_line_end >= text_len) break; // already on last line

            size_t next_line_start = cur_line_end + 1;

            // find end of next line
            size_t next_line_end = next_line_start;
            while (next_line_end < text_len && text[next_line_end] != '\n')
                next_line_end++;

            size_t next_line_len = next_line_end - next_line_start;
            size_t target = next_line_start + (col < next_line_len ? col : next_line_len);

            if (target > pos)
                gb_move_right(editor->buffer, target - pos);
            break;
        }
    }

    free(text);
}

void cursor_scroll(Editor *editor) {
    size_t row, col;
    get_cursor_screen_pos(editor, &row, &col);

    // vertical scrolling
    size_t visible_rows = editor->screen_rows - 1;

    if (row < editor->row_offset) {
        editor->row_offset = row;
    } else if (row >= editor->row_offset + visible_rows) {
        editor->row_offset = row - visible_rows + 1;
    }

    // horizontal scrolling
    if (col < editor->col_offset) {
        editor->col_offset = col;
    } else if (col >= editor->col_offset + (size_t)editor->screen_cols) {
        editor->col_offset = col - editor->screen_cols + 1;
    }
}

void cursor_render(Editor *editor) {
    size_t row, col;
    get_cursor_screen_pos(editor, &row, &col);

    char buf[32];
    int n = snprintf(buf, sizeof(buf),
                     "\x1b[%zu;%zuH",
                     (row - editor->row_offset) + 1,
                     (col - editor->col_offset) + 1);
    write(STDOUT_FILENO, buf, n);
}
