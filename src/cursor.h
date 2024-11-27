#pragma once
#include <stdio.h>

typedef struct Cursor {
    size_t x;
    size_t y;
} cursor_t;

cursor_t *create_cursor(int x, int y);
void move_cursor(cursor_t *cursor, int new_x, int new_y);
