#include "gb.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>


#define CHECK_INVARIANTS(buf)                          \
    do {                                                       \
        GapInvariantResult r = gb_check_invariants(buf); \
        if (r != GAP_OK) {                                     \
            fprintf(stderr, "invariant failed: %d\n", r);      \
            abort();                                           \
        }                                                      \
    } while (0)

typedef enum {
    GAP_OK = 0,
    GAP_NULL,
    GAP_BOUNDS,
    GAP_TEXT_SIZE,
    GAP_CURSOR_RANGE,
    GAP_CURSOR_NOT_AT_GAP
} GapInvariantResult;


static const size_t GAP_SIZE = 64;

struct GapBuffer {
    char *buffer;
    size_t buf_size;
    size_t text_size;
    size_t gap_start;
    size_t gap_end;
    size_t cursor_pos;
};


static GapInvariantResult gb_check_invariants(const GapBuffer *Buffer) {
    if (!Buffer) return GAP_NULL;

    size_t gap_size = Buffer->gap_end - Buffer->gap_start;

    // structural bounds
    if (!(Buffer->gap_start <= Buffer->gap_end &&
            Buffer->gap_end <= Buffer->buf_size)) {
        return GAP_BOUNDS;
    }

    // text size consistency
    if (Buffer->text_size != Buffer->buf_size - gap_size) {
        return GAP_TEXT_SIZE;
    }

    // cursor validity
    if (Buffer->cursor_pos > Buffer->text_size) {
        return GAP_CURSOR_RANGE;
    }

    return GAP_OK;
}

static void *gb_xmalloc(size_t size) {
    void *ptr = malloc(size);
      if (!ptr) {
            fprintf(stderr, "out of memory\n");
            abort();
      }

      return ptr;
}

size_t gb_length(const GapBuffer *Buffer) {
    if (!Buffer) return 0;
    size_t length = Buffer->text_size;

    return length;
}

size_t gb_cursor(const GapBuffer *Buffer) {
    if (!Buffer) return 0;
    size_t cursor = Buffer->cursor_pos;

    return cursor;
}

size_t gb_copy_text(const GapBuffer *Buffer, char *out, size_t out_size) {
    if (!Buffer || !out || out_size == 0) return 0;
    
    size_t to_copy = Buffer->text_size;
    if (to_copy > out_size) {
        to_copy = out_size;
    }

    size_t before_gap = Buffer->gap_start;
    size_t after_gap = Buffer->text_size - before_gap;

    if (before_gap > to_copy) {
        before_gap = to_copy;
        after_gap = 0;
    } else if (before_gap + after_gap > to_copy) {
        after_gap = to_copy - before_gap;
    }

    memcpy(out, Buffer->buffer, before_gap);
    memcpy(out + before_gap, Buffer->buffer + Buffer->gap_end, after_gap);

    return before_gap + after_gap;
}

void gb_dest(GapBuffer *Buffer) {
    if (!Buffer) return;
    free(Buffer->buffer);
    free(Buffer);
}

static void gb_shrink_gap(GapBuffer *Buffer) {
    size_t gap_size = Buffer->gap_end - Buffer->gap_start;

    size_t min_gap = GAP_SIZE;

    if (gap_size <= min_gap) return;
    if (gap_size <= Buffer->text_size) return;

    size_t new_gap = GAP_SIZE;
    size_t new_buf_size = Buffer->text_size + new_gap;

    char *new_buf = gb_xmalloc(new_buf_size);

    memcpy(new_buf, Buffer->buffer, Buffer->gap_start);

    size_t tail = Buffer->text_size - Buffer->gap_start;
    memcpy(
            new_buf + Buffer->gap_start + new_gap,
            Buffer->buffer + Buffer->gap_end,
            tail
    );
    
    free(Buffer->buffer);
    Buffer->buffer = new_buf;
    Buffer->buf_size = new_buf_size;
    Buffer->gap_end = Buffer->gap_start + new_gap;

    CHECK_INVARIANTS(Buffer);
}

static void gb_grow_gap(GapBuffer *Buffer, size_t n){
    size_t old_gap = Buffer->gap_end - Buffer->gap_start;
    size_t req_gap = n + GAP_SIZE;
    if(req_gap <= old_gap) return;

    size_t new_buf_size = Buffer->buf_size + (req_gap - old_gap);
    char *new_buf = gb_xmalloc(new_buf_size);

    memcpy(new_buf, Buffer->buffer, Buffer->gap_start);

    size_t tail = Buffer->buf_size - Buffer->gap_end;
    memcpy(
            new_buf + Buffer->gap_start + req_gap,
            Buffer->buffer + Buffer->gap_end,
            tail
    );
    
    free(Buffer->buffer);
    Buffer->buffer = new_buf;
    Buffer->gap_end = Buffer->gap_start + req_gap;
    Buffer->buf_size = new_buf_size;

    CHECK_INVARIANTS(Buffer);
}

GapBuffer *gb_insert(GapBuffer *Buffer, const void *data, size_t len) {
    if(!Buffer || !data || len == 0) return Buffer;

    size_t gap_size = Buffer->gap_end - Buffer->gap_start;
    if (len > gap_size) {
        gb_grow_gap(Buffer, len);
    }

    memcpy(Buffer->buffer + Buffer->gap_start, data, len);
    Buffer->gap_start += len;
    Buffer->cursor_pos += len;
    Buffer->text_size += len;

    CHECK_INVARIANTS(Buffer);
    return Buffer;
}

// physically moves gap to the cursor position by shifting text
// must be called before any operation that assumes cursor == gap_start
static void gb_move_gap(GapBuffer *Buffer) {
    size_t gap_size = Buffer->gap_end - Buffer->gap_start;
    
    size_t physical_pos;
    if (Buffer->cursor_pos <= Buffer->gap_start) {
        physical_pos = Buffer->cursor_pos;
    } else {
        physical_pos = Buffer->cursor_pos + gap_size;
    }

    if (physical_pos < Buffer->gap_start) {
        size_t move_len = Buffer->gap_start - physical_pos;
        memmove(
                Buffer->buffer + physical_pos + gap_size,
                Buffer->buffer + physical_pos,
                move_len
                );
        Buffer->gap_start = physical_pos;
        Buffer->gap_end = physical_pos + gap_size;
    } else if (physical_pos > Buffer->gap_end) {
        size_t move_len = physical_pos - Buffer->gap_end;
        memmove(
                Buffer->buffer + Buffer->gap_start,
                Buffer->buffer + Buffer->gap_end,
                move_len
               );
        Buffer->gap_start += move_len;
        Buffer->gap_end += move_len;
    }

    Buffer->cursor_pos = Buffer->gap_start;

    CHECK_INVARIANTS(Buffer);
}

GapBuffer *gb_move_left(GapBuffer *Buffer, size_t to_move) {
    if (to_move >= Buffer->cursor_pos) {
        Buffer->cursor_pos = 0;
    } else {
        Buffer->cursor_pos = Buffer->cursor_pos - to_move;
    }

    gb_move_gap(Buffer);

    return Buffer;
}

GapBuffer *gb_move_right(GapBuffer *Buffer, size_t to_move) {
    if (to_move >= Buffer->text_size - Buffer->cursor_pos) {
        Buffer->cursor_pos = Buffer->text_size;
    } else {
        Buffer->cursor_pos = Buffer->cursor_pos + to_move;
    }

    gb_move_gap(Buffer);

    return Buffer;
}

GapBuffer *gb_delete(GapBuffer *Buffer, size_t n) {
    gb_move_gap(Buffer);

    size_t max_n = Buffer->text_size - Buffer->cursor_pos;
    if (n > max_n) {
        n = max_n;
    }

    Buffer->gap_end += n;
    Buffer->text_size -= n;

    CHECK_INVARIANTS(Buffer);
    gb_shrink_gap(Buffer);
    return Buffer;
}

GapBuffer *gb_backspace(GapBuffer *Buffer, size_t n) {
    gb_move_gap(Buffer);

    size_t max_n = Buffer->cursor_pos;
    if (n > max_n) {
        n = max_n;
    }

    Buffer->gap_start -= n;
    Buffer->cursor_pos -= n;
    Buffer->text_size -= n;

    CHECK_INVARIANTS(Buffer);
    gb_shrink_gap(Buffer);
    return Buffer;
}


GapBuffer *gb_init(const void *data, size_t len) {
    GapBuffer *Buffer = gb_xmalloc(sizeof(GapBuffer));
    Buffer->buf_size = len + GAP_SIZE;
    Buffer->text_size = len;
    Buffer->buffer = gb_xmalloc(Buffer->buf_size);

    if (data && len > 0) {
        memcpy(Buffer->buffer, data, len);
    }

    Buffer->gap_start = len;
    Buffer->gap_end = len + GAP_SIZE;
    Buffer->cursor_pos = Buffer->gap_start;

    CHECK_INVARIANTS(Buffer);
    return Buffer;
}
