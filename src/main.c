#include <stdlib.h>
#include "editor.h"
#include "terminal.h"

int main(int argc, char *argv[]) {
    terminal_init();

    Editor editor;
    editor_init(&editor);

    if (argc > 1) {
        editor_load_file(&editor, argv[1]);
    }

    editor_render(&editor);

    while (editor.is_running) {
        char key = read_key();

        switch (editor.mode) {
            case MODE_INSERT: editor_handle_insert_input(&editor, key); break;
            case MODE_VISUAL: editor_handle_visual_input(&editor, key); break;
            case MODE_NORMAL: editor_handle_normal_input(&editor, key); break;
        }
    }

    editor_destroy(&editor);
    clear_terminal();

    return 0;
}
