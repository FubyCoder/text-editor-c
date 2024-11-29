#pragma once
#include "editor.h"
#include <stdio.h>

// Cursors in text editors act in particular way
// If you dont change the x position they will try to return to the original position if the row is long enough
// for example
// row 1 = 5 chars
// row 2 = 3 chars
// row 3 = 7 chars
// if we position the cursor to the last char of row 1 and change vertically we should see someting like this
// ex1 : (r1)5 -> (r2)3 -> (r3)5 -> (r2)3 -> (r1) 5
// ex2 : (r3)7 -> (r2)3 -> (r1)5 -> (r2)3 -> (r3) 7
// However if we change the x position when we are in a different row that would be the max cursor position moving vertically

typedef struct Cursor {
    size_t x;
    size_t y;

    // Render position
    size_t rx;
    size_t ry;

    // Offset position
    size_t ox;
    size_t oy;
} cursor_t;

cursor_t *create_cursor(size_t x, size_t y);
void set_cursor_position(cursor_t *cursor, editor_state_t *editor, long new_x, long new_y);
void update_cursor_render_position(editor_state_t *editor, cursor_t *cursor);
