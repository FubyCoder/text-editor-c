#include "cursor.h"
#include <stdio.h>
#include <stdlib.h>

cursor_t *create_cursor(int x, int y) {
    cursor_t *cursor = malloc(sizeof(cursor_t));

    if (cursor == NULL) {
        return NULL;
    }

    cursor->x = x;
    cursor->y = y;
    return cursor;
}

void move_cursor(cursor_t *cursor, int new_x, int new_y) {
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
