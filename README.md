# ceditor

**ceditor** is a lightweight terminal text editor written in C, designed for simplicity and fast text editing in the terminal.

Current version: **v0.1.0 (Phase 0 Complete)**

---

## Features (Phase 0)

- File loading into a gap buffer
- Save buffer to file (overwrite or new file)
- Text rendering in terminal
- Scrolling support for files larger than the screen
- Status bar showing file name and byte count
- Clean terminal on exit

> Future phases will add modes, cursor navigation, insertion/deletion, and multiple buffer support.

---

## Project Structure

```
cedit/
├── build/      # Compiled binaries
├── docs/       # Documentation & todo.txt
├── include/    # Header files (e.g., gb.h)
├── src/        # Source files (main.c, gb.c, etc.)
├── tests/      # Example text files
├── Makefile    # Compilation instructions
└── README.md
```

---

## Compilation

Make sure you are in the `cedit/` directory:

```bash
make
```

This produces the binary: `build/ceditor`

---

## Usage

```
./build/ceditor [filename]
```

- `filename` is optional. If provided, the file will be loaded into the editor.
- Basic key bindings (Phase 0):
  - `ctrl-s` → Save file
  - `ctrl-q` → Quit editor (prompts if there are unsaved changes)
  - `h` / `l` → navigate left / right through file
  - `k` / `j` → Scroll up / down through file

---

## Roadmap

**Phase 0 - Fundamental setup [DONE]**

- Main function loop
- Basic file to buffer writing
- Save buffer to file or new file
- Text-terminal rendering


**Phase 1 – MVP Finish**

- Implement basic modes: normal, insert
- Cursor rendering
- Navigation (`h`, `j`, `k`, `l`)
- Basic insertion/deletion (typing, backspace, delete)
- Visual mode

**Phase 2 – Navigation & Editing**

- Undo / redo
- Word-wise navigation
- Line start/end movement

**Phase 3 – Buffer Management**

- Multiple buffers
- Buffer navigation
- Adjust gap for larger files
