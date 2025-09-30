#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* todo list
 *
 * inserting text
 *  growing gap
 *  cursor moves at end of inserted text
 *
 * deleting text
 *  shrinking gap
 *  cursor moves to text before deleted section
 *
 *
*/

size_t GAP_SIZE = 6;

typedef struct {
    char *buffer;
    size_t buf_size;
    size_t text_size;
    size_t gap_start;
    size_t gap_end;
    size_t cursor_pos;
} GapBuffer;

void *allocCheck(void *ptr) {
      if (ptr == NULL) {
            fprintf(stderr, "mem alloc fail");
            exit(1);
      }


      return ptr;
}

GapBuffer *moveGap(GapBuffer *Buffer) {
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

    return Buffer;
}

GapBuffer *moveLeft(GapBuffer *Buffer, size_t to_move) {
    if (to_move >= Buffer->cursor_pos) {
        Buffer->cursor_pos = 0;
    } else {
        Buffer->cursor_pos = Buffer->cursor_pos - to_move;
    }

    moveGap(Buffer);

    return Buffer;
}

GapBuffer *moveRight(GapBuffer *Buffer, size_t to_move) {
    if (to_move >= Buffer->text_size - Buffer->cursor_pos) {
        Buffer->cursor_pos = Buffer->text_size;
    } else {
        Buffer->cursor_pos = Buffer->cursor_pos + to_move;
    }

    moveGap(Buffer);


    return Buffer;
}

GapBuffer *initBuffer(const char *str) {
    size_t str_size = strlen(str);

    GapBuffer *Buffer = allocCheck(malloc(sizeof(GapBuffer)));
    Buffer->buf_size = str_size + GAP_SIZE;
    Buffer->text_size = str_size;
    Buffer->buffer = allocCheck(malloc(Buffer->buf_size));

    memcpy(Buffer->buffer, str, str_size);

    Buffer->gap_start = str_size;
    Buffer->gap_end = str_size + GAP_SIZE;
    Buffer->cursor_pos = Buffer->gap_start;


    return Buffer;
}

void printBuffer(GapBuffer *Buffer) {
    fwrite(Buffer->buffer, 1, Buffer->gap_start, stdout);

    printf("[GAP]");

    size_t after_len = Buffer->text_size - Buffer->gap_start;
    fwrite(Buffer->buffer + Buffer->gap_end, 1, after_len, stdout);

    printf("\n");
}

int main() {
    char str[] = "kitten on pizza slice in space";
    GapBuffer *Buffer = initBuffer(str);

    printBuffer(Buffer);
    printf("%zu\n", Buffer->cursor_pos);

    moveLeft(Buffer, 31);
    printBuffer(Buffer);
    printf("%zu\n", Buffer->cursor_pos);

    moveRight(Buffer, 40);
    printBuffer(Buffer);
    printf("%zu\n", Buffer->cursor_pos);

    free(Buffer->buffer);
    free(Buffer);

    
    return 0;
}
