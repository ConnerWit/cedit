#ifndef EDITOR_H
#define EDITOR_H

#include <stdbool.h>
#include <stddef.h>
#include "gb.h"
#include "terminal.h"


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

    enum {
        NORMAL,
        INSERT,
        VISUAL,
    } EditorState;
} Editor;

void editor_init(Editor *editor);
void editor_load_file(Editor *editor, const char *filename);
void editor_save_file(Editor *editor);
void editor_render(Editor *editor);
void editor_handle_input(Editor *editor, char key);
void editor_destroy(Editor *editor);

#endif
