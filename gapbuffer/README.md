# Gap Buffer Library (C)
**Version:** v1.1.0

A written in C implementation of a gap buffer intended for use in real text editors.

This library implements the core data structure used by many editors to support
efficient text insertion and deletion around a cursor. The focus is on correctness,
clear invariants, and editor-accurate behavior rather than a minimal or academic demo.

## Changelog
### v1.1.0
- Added gap shrinking to optimize memory usage
- Improved invariant checks for safer operations
- Refactored `init` and `insert` functions for binary correctness
- Updated minor version to 1.1.0

## Features

- Insert and delete text at the cursor
- Move the cursor left and right while maintaining the gap
- Automatically grow the gap when needed
- Keep the internal state consistent via debug-time invariant checks
- Hide internal structure behind a clean, opaque API

## Design notes

- The gap is always kept aligned with the cursor
- All mutations preserve strict size and bounds invariants
- Memory ownership is explicit; no global state
- Written to be embedded as a low-level editor component

## Intended for

- Text editors (terminal or GUI)
- Interactive tools that edit text in-place
- As a base for experimenting with editor internals

### Next steps / ideas

- Configurable gap size and growth strategy
- Optional gap shrinking
- Rope structure built from gap-buffer-backed nodes for mixed-size workloads

This is intended to serve as a practical, editor-grade text editing core.
