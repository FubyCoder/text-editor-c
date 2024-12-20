#pragma once

#include "text-buffer.h"

typedef struct EditorState {
    text_buffer_t **rows;
    unsigned long row_count;
    char* file_path;

    unsigned int terminal_width;
    unsigned int terminal_height;

    unsigned int max_text_window_height;
    unsigned int max_text_window_width;


    // Used for scroll and avoid scrolls when cursor is inside this window
    unsigned int text_window_start;

    unsigned int vertical_offset;
} editor_state_t;

void append_row(editor_state_t *editor, text_buffer_t *row);
void insert_and_move_rows(text_buffer_t **elements, size_t count, size_t index, text_buffer_t *new_item);
void insert_row(editor_state_t *editor, size_t index, text_buffer_t *row);
void merge_rows(editor_state_t *editor, text_buffer_t *row, text_buffer_t *row_to_append);
text_buffer_t *get_row(editor_state_t *editor, size_t row_index);

editor_state_t *create_editor_file();
void delete_row(editor_state_t *editor,text_buffer_t *row);
