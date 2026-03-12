# cedit
**cedit** is a lightweight terminal text editor written in C, designed for simplicity and fast text editing in the terminal.

Current version: **v1.0.0 (MVP Complete)**

---

## Features
- Modal editing: normal, insert, and visual modes
- Gap buffer for efficient text insertion and deletion
- File loading, saving, and overwrite
- Full cursor navigation with scrolling support for large files
- Visual selection with yank and cut
- Clipboard buffer with paste
- Undo history (up to 128 steps)
- Status bar showing filename, byte count, and current mode
- Clean terminal restore on exit

---

## Project Structure
```
cedit/
├── build/          # Compiled binaries
├── docs/           # Documentation & todo.txt
├── include/        # Header files
├── src/            # Source files
│   ├── main.c
│   ├── editor.c    # Core editor logic and input handling
│   ├── cursor.c    # Cursor movement, scrolling, rendering
│   ├── gb.c        # Gap buffer implementation
│   ├── terminal.c  # Raw terminal setup and key reading
│   ├── clipboard.c # Yank/cut clipboard buffer
│   └── history.c   # Undo history stack
├── tests/          # Example text files
├── Makefile        # Compilation instructions
└── README.md
```

---

## Compilation
Make sure you are in the `cedit/` directory:
```bash
make
```

This produces the binary: `build/cedit`

---

## Usage
```
./build/cedit [filename]
```

`filename` is optional. If omitted, the editor starts with an empty buffer.

---

## Key Bindings

### Normal Mode
| Key | Action |
|-----|--------|
| `h` / `l` | Move left / right |
| `k` / `j` | Move up / down |
| `i` | Enter insert mode |
| `v` | Enter visual mode |
| `p` | Paste clipboard at cursor |
| `r` | Undo last change |
| `ctrl-s` | Save file |
| `ctrl-q` | Quit (prompts if unsaved changes exist) |

### Insert Mode
| Key | Action |
|-----|--------|
| Arrow keys | Move cursor |
| `backspace` | Delete character behind cursor |
| `delete` | Delete character at cursor |
| `enter` | Insert newline |
| `esc` | Return to normal mode |

### Visual Mode
| Key | Action |
|-----|--------|
| `h` / `j` / `k` / `l` | Extend selection |
| Arrow keys | Extend selection |
| `c` | Copy selection to clipboard |
| `x` | Cut selection to clipboard |
| `esc` | Return to normal mode |

---

## Roadmap

**Phase 0 – Fundamental Setup [DONE]**
- Main function loop
- Basic file to buffer writing
- Save buffer to file or new file
- Text-terminal rendering

**Phase 1 – MVP [DONE]**
- Normal, insert, and visual modes
- Cursor rendering and navigation
- Insertion and deletion
- Visual selection, yank, cut, paste
- Undo history

**Phase 2 – Navigation & Editing**
- Redo
- Word-wise navigation
- Line start / end movement
- Search

**Phase 3 – Buffer Management**
- Multiple buffers
- Buffer navigation
- Gap tuning for large files
