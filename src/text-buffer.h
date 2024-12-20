#pragma once
#include <stdio.h>

typedef struct TextBuffer {
    char *data;
    size_t index;
    size_t count;
    size_t capacity;
} text_buffer_t;

text_buffer_t *create_text_buffer(size_t capacity);
void insert_char(text_buffer_t *buffer, size_t index, char *c);
void append_char(text_buffer_t *buffer, char *c);
void remove_char(text_buffer_t *buffer, size_t index);
void free_text_buffer(text_buffer_t *text_buffer);
