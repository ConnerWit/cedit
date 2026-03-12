#include <stdlib.h>
#include <string.h>
#include "clipboard.h"

void clipboard_init(Clipboard *cb) {
    cb->data = NULL;
    cb->len = 0;
}

void clipboard_set(Clipboard *cb, const char *data, size_t len) {
    free(cb->data);
    cb->data = malloc(len);
    if (!cb->data) {
        cb->len = 0;
        return;
    }
    memcpy(cb->data, data, len);
    cb->len = len;
}

void clipboard_free(Clipboard *cb) {
    free(cb->data);
    cb->data = NULL;
    cb->len = 0;
}
