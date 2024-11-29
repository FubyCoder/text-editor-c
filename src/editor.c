#include <stdlib.h>
#include <string.h>
#include <threads.h>

#include "editor.h"
#include "text-buffer.h"

void append_row(editor_state_t *editor, text_buffer_t *row) {
    text_buffer_t **tmp = realloc(editor->rows, (editor->row_count + 1) * sizeof(text_buffer_t));
    if (tmp == NULL) {
        return;
    }

    editor->rows = tmp;
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

void insert_row(editor_state_t *editor, size_t index, text_buffer_t *row) {
    editor->row_count++;

    text_buffer_t **tmp = realloc(editor->rows, editor->row_count * sizeof(text_buffer_t));

    if (tmp == NULL) {
        return;
    }

    editor->rows = tmp;
    insert_and_move_rows(editor->rows, editor->row_count, index, row);
}

void merge_rows(editor_state_t *editor, text_buffer_t *row, text_buffer_t *row_to_append) {
    char *tmp = realloc(row->data, row->capacity + row_to_append->count + 1);

    if (tmp == NULL) {
        return;
    }

    row->data = tmp;

    if (row_to_append->count > 0) {
        strncat(row->data, row_to_append->data, row_to_append->count);
        row->count += row_to_append->count;
    }

    delete_row(editor, row_to_append);
}

void delete_row(editor_state_t *editor, text_buffer_t *row) {
    // Compact rows
    for (size_t i = row->index; i < editor->row_count - 1; i++) {
        editor->rows[i] = editor->rows[i + 1];
        editor->rows[i]->index--;
    }

    free_text_buffer(row);
    editor->row_count--;
}

text_buffer_t *get_row(editor_state_t *editor, size_t row_index) {
    if (row_index >= editor->row_count) {
        return NULL;
    }

    return editor->rows[row_index];
}

editor_state_t *create_editor_file() {
    editor_state_t *editor = malloc(sizeof(editor_state_t));
    if (editor == NULL) {
        return NULL;
    }

    text_buffer_t **rows = malloc(sizeof(text_buffer_t));

    if (rows == NULL) {
        return NULL;
    }

    editor->row_count = 0;
    editor->rows = rows;

    return editor;
}
