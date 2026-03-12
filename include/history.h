#ifndef HISTORY_H
#define HISTORY_H

#include <stddef.h>

#define HISTORY_MAX 128

typedef enum {
    OP_NONE,
    OP_INSERT,
    OP_DELETE,
    OP_CUT,
    OP_PASTE
} UndoOpType;

typedef struct {
    char *text;
    size_t text_len;
    size_t cursor_pos;
} HistoryEntry;

typedef struct {
    HistoryEntry entries[HISTORY_MAX];
    int top;
    UndoOpType last_op;
} History;

void history_init(History *h);
void history_free(History *h);

void history_push(History *h, const char *text, size_t text_len, size_t cursor_pos);
int  history_pop(History *h, char **out_text, size_t *out_len, size_t *out_cursor);

#endif
