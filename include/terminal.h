#ifndef TERMINAL_H
#define TERMINAL_H

#include <stddef.h>

typedef struct {
    int rows;
    int cols;
} WindowSize;

void terminal_init(void);
void terminal_cleanup(void);

void clear_terminal(void);
char read_key(void);
WindowSize get_window(void);

#endif
