#ifndef CURSOR_H
#define CURSOR_H

#include <stdbool.h>
#include <stddef.h>

#include "editor.h"


void get_cursor_screen_pos(
        Editor *editor,
        size_t *out_row,
        size_t *out_col
        );
void cursor_move(Editor *editor, char dir);
void cursor_render(Editor *editor);

#endif
