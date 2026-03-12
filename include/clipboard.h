#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <stddef.h>

typedef struct {
    char *data;
    size_t len;
} Clipboard;

void clipboard_init(Clipboard *cb);
void clipboard_set(Clipboard *cb, const char *data, size_t len);
void clipboard_free(Clipboard *cb);

#endif
