#include <stdlib.h>
#include <string.h>

#include "editor.h"

void append_row(editor_file_t *editor, text_buffer_t *row) {
    editor->rows = realloc(editor->rows, (editor->row_count + 1) * sizeof(text_buffer_t));
    editor->rows[editor->row_count] = row;
    row->index = editor->row_count;
    editor->row_count++;
}

void insert_and_move_rows(text_buffer_t **elements, size_t count, size_t index, text_buffer_t *new_item) {
    for (size_t i = count; i > index; i--) {
        elements[i] = elements[i - 1];
        if (elements[i] != NULL) {
            elements[i]->index++;
        }
    }

    new_item->index = index;
    elements[index] = new_item;
}

void insert_row(editor_file_t *editor, size_t index, text_buffer_t *row) {
    editor->row_count++;
    editor->rows = realloc(editor->rows, editor->row_count * sizeof(text_buffer_t));
    insert_and_move_rows(editor->rows, editor->row_count, index, row);
}

void merge_rows(editor_file_t *editor, text_buffer_t *row, text_buffer_t *row_to_append) {
    row = realloc(row, row->capacity + row_to_append->capacity);

    if (row_to_append->count > 0) {
        memmove(row->data + row->count, row_to_append->data, row_to_append->count);
        row->count += row_to_append->count;
    }

    // Compact rows
    for (size_t i = row_to_append->index; i < editor->row_count - 1; i++) {
        editor->rows[i] = editor->rows[i + 1];
        editor->rows[i]->index--;
    }

    editor->row_count--;

    free_text_buffer(row_to_append);
}

text_buffer_t *get_row(editor_file_t *editor, size_t row_index) {
    if (row_index > editor->row_count) {
        return NULL;
    }

    return editor->rows[row_index];
}

editor_file_t *create_editor_file() {
    editor_file_t *editor = malloc(sizeof(editor_file_t));
    if (editor == NULL) {
        return NULL;
    }

    editor->row_count = 0;

    return editor;
}
