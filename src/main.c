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
        editor_handle_input(&editor, key);
    }

    editor_destroy(&editor);
    clear_terminal();

    return 0;
}
