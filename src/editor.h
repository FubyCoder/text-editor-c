#pragma once

#include "text-buffer.h"

typedef struct EditorFile {
    text_buffer_t **rows;
    unsigned long row_count;
} editor_file_t;

void append_row(editor_file_t *editor, text_buffer_t *row);
void insert_and_move_rows(text_buffer_t **elements, size_t count, size_t index, text_buffer_t *new_item);
void insert_row(editor_file_t *editor, size_t index, text_buffer_t *row);
void merge_rows(editor_file_t *editor, text_buffer_t *row, text_buffer_t *row_to_append);
text_buffer_t *get_row(editor_file_t *editor, size_t row_index);

editor_file_t *create_editor_file();
