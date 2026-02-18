#ifndef GAP_BUFFER_H
#define GAP_BUFFER_H

#define GB_VERSION_MAJOR 1
#define GB_VERSION_MINOR 1
#define GB_VERSION_PATCH 0

#include <stddef.h>

typedef struct GapBuffer GapBuffer;

// initialize a gap buffer with 'len' bytes from 'data'
GapBuffer *gb_init(const void *data, size_t len);

// destroy gap buffer and free all owned memory
void gb_dest(GapBuffer *gb); 

// cursor movement
GapBuffer *gb_move_left(GapBuffer *gb, size_t n);
GapBuffer *gb_move_right(GapBuffer *gb, size_t n);

// insert 'len' bytes at the cursor position
GapBuffer *gb_insert(GapBuffer *gb, const void *data, size_t len);
// delete 'n' bytes forward from the cursor
GapBuffer *gb_delete(GapBuffer *gb, size_t n);
// delete 'n' bytes backward from the cursor
GapBuffer *gb_backspace(GapBuffer *gb, size_t n);

// total text length (excluding gap)
size_t gb_length(const GapBuffer *gb);
// cursor position
size_t gb_cursor(const GapBuffer *gb);

// copy buffer contents (excluding gap) into 'out' up to 'out_size' bytes
// this function does NOT guarantee null-termination
size_t gb_copy_text(const GapBuffer *gb, char *out, size_t out_size);

#endif
