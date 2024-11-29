#include <stdio.h>
#include <stdlib.h>

#include "cursor.h"
#include "editor.h"
#include "text-buffer.h"

cursor_t *create_cursor(size_t x, size_t y) {
    cursor_t *cursor = malloc(sizeof(cursor_t));

    if (cursor == NULL) {
        return NULL;
    }

    cursor->x = x;
    cursor->y = y;
    cursor->rx = x;
    cursor->ry = y;

    cursor->ox = 0;
    cursor->ox = 0;
    return cursor;
}

void move_cursor(cursor_t *cursor, editor_state_t *editor, int delta_x, int delta_y) {
    set_cursor_position(cursor, editor, cursor->x + delta_x, cursor->y + delta_y);
}

void set_cursor_position(cursor_t *cursor, editor_state_t *editor, long new_x, long new_y) {
    if (new_x < 0) {
        cursor->x = 0;
        cursor->rx = 0;
    } else {
        if (new_x != cursor->x) {
            cursor->x = new_x;
            cursor->rx = new_x;
        }
    }

    if (cursor->rx > editor->max_text_window_width) {
        cursor->ox = cursor->rx - editor->max_text_window_width;
    } else {
        cursor->ox = 0;
    }

    if (new_y < 0) {
        cursor->y = 0;
        cursor->ry = 0;
    } else {
        cursor->y = new_y;
        cursor->ry = new_y;
    }
}

void update_cursor_render_position(editor_state_t *editor, cursor_t *cursor) {
    text_buffer_t *row = get_row(editor, cursor->y);

    if (cursor->x > row->count) {
        cursor->rx = row->count;
    } else {
        cursor->rx = cursor->x;
    }

    if (cursor->rx > editor->max_text_window_width) {
        cursor->ox = cursor->rx - editor->max_text_window_width;
    } else {
        cursor->ox = 0;
    }

    const int MAX_ITEMS_TO_RENDER = editor->max_text_window_height;

    if (editor->row_count < MAX_ITEMS_TO_RENDER) {
        cursor->ry = cursor->y;
    } else {
        if (cursor->y + MAX_ITEMS_TO_RENDER <= editor->row_count) {
            cursor->ry = 0;
        } else {
            cursor->ry = MAX_ITEMS_TO_RENDER - (editor->row_count - cursor->y);
        }
    }
}
