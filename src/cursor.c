#include "cursor.h"
#include <stdio.h>
#include <stdlib.h>

cursor_t *create_cursor(size_t x, size_t y) {
    cursor_t *cursor = malloc(sizeof(cursor_t));

    if (cursor == NULL) {
        return NULL;
    }

    cursor->x = x;
    cursor->y = y;
    return cursor;
}

void move_cursor(cursor_t *cursor, long new_x, long new_y) {
    if (new_x < 0) {
        cursor->x = 0;
    } else {
        cursor->x = new_x;
    }

    if (new_y < 0) {
        cursor->y = 0;
    } else {
        cursor->y = new_y;
    }
}
