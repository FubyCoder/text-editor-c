#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "text-buffer.h"

text_buffer_t *create_text_buffer(size_t capacity) {
    text_buffer_t *buffer = malloc(sizeof(text_buffer_t));

    if (buffer == NULL) {
        return NULL;
    }

    buffer->data = malloc(capacity);

    if (buffer->data == NULL) {
        free(buffer);
        return NULL;
    }

    buffer->capacity = capacity;
    buffer->count = 0;
    buffer->index = 0;

    return buffer;
}

void resize_buffer(text_buffer_t *buffer) {
    if (buffer->count != buffer->capacity) {
        return;
    }

    buffer->capacity *= 2;
    char *tmp = realloc(buffer->data, buffer->capacity);

    if (buffer->data == NULL) {
        free(buffer);
        return;
    }

    buffer->data = tmp;
}

void insert_char(text_buffer_t *buffer, size_t index, char *c) {
    resize_buffer(buffer);

    if (index > buffer->count) {
        return;
    }

    for (size_t i = buffer->count + 1; i > index; i--) {
        buffer->data[i] = buffer->data[i - 1];
    }

    buffer->data[index] = *c;
    buffer->count++;
}

void append_char(text_buffer_t *buffer, char *c) {
    resize_buffer(buffer);

    buffer->data[buffer->count] = *c;
    buffer->count++;
}

void remove_char(text_buffer_t *buffer, size_t index) {
    if (index < 0 || index >= buffer->count) {
        return;
    }

    if (buffer->count == 0) {
        return;
    }

    for (size_t i = index; i < buffer->count; i++) {
        buffer->data[i] = buffer->data[i + 1];
    }

    buffer->data[buffer->count] = 0;
    buffer->count--;
}

void free_text_buffer(text_buffer_t *text_buffer) {
    free(text_buffer->data);
    free(text_buffer);
}
