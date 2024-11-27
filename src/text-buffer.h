#pragma once
#include <stdio.h>

typedef struct TextBuffer {
    char *data;
    size_t count;
    size_t capacity;
} text_buffer_t;

text_buffer_t *create_text_buffer(size_t capacity);
void insert_char(text_buffer_t *buffer, size_t index, char *c);
void remove_char(text_buffer_t *buffer, int index);
