#pragma once
#include <stdio.h>

typedef struct Cursor {
    size_t x;
    size_t y;
} cursor_t;

cursor_t *create_cursor(size_t x, size_t y);
void move_cursor(cursor_t *cursor, long new_x, long new_y);
