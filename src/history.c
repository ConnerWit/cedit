#include <stdlib.h>
#include <string.h>
#include "history.h"

void history_init(History *h) {
    memset(h->entries, 0, sizeof(h->entries));
    h->top = -1;
    h->last_op = OP_NONE;
}

void history_free(History *h) {
    for (int i = 0; i <= h->top; i++) {
        free(h->entries[i].text);
        h->entries[i].text = NULL;
    }
    h->top = -1;
    h->last_op = OP_NONE;
}

void history_push(History *h, const char *text, size_t text_len, size_t cursor_pos) {
    if (h->top == HISTORY_MAX - 1) {
        free(h->entries[0].text);
        memmove(&h->entries[0], &h->entries[1], sizeof(HistoryEntry) * (HISTORY_MAX - 1));
        h->top--;
    }

    h->top++;
    h->entries[h->top].text = malloc(text_len);
    if (!h->entries[h->top].text) {
        h->top--;
        return;
    }

    memcpy(h->entries[h->top].text, text, text_len);
    h->entries[h->top].text_len = text_len;
    h->entries[h->top].cursor_pos = cursor_pos;
}

int history_pop(History *h, char **out_text, size_t *out_len, size_t *out_cursor) {
    if (h->top < 0) return 0;

    *out_text   = h->entries[h->top].text;
    *out_len    = h->entries[h->top].text_len;
    *out_cursor = h->entries[h->top].cursor_pos;

    h->entries[h->top].text = NULL;
    h->top--;

    return 1;
}
