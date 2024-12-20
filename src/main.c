#include <complex.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "cursor.h"
#include "editor.h"
#include "terminal.h"
#include "text-buffer.h"
#include "util.h"

#define DEBUG 1

void load_file_in_editor(editor_state_t *editor, char *file_path) {
    FILE *file = fopen(file_path, "r");
    editor->file_path = file_path;

    char c;

    if (file == NULL) {
        return;
    }

    size_t row_index = 0;
    text_buffer_t *row = get_row(editor, row_index);
    if (row == NULL) {
        row = create_text_buffer(10);
        append_row(editor, row);
    }

    while ((c = fgetc(file)) != EOF) {
        if (c == '\n') {
            row_index++;
            row = get_row(editor, row_index);

            if (row == NULL) {
                row = create_text_buffer(10);
                append_row(editor, row);
            }
        } else {
            append_char(row, &c);
        }
    }
}

void save_file(editor_state_t *editor, char *file_path) {
    if (!file_path) {
        return;
    }

    FILE *file = fopen(file_path, "w");

    for (size_t i = 0; i < editor->row_count; i++) {
        fputs(editor->rows[i]->data, file);
        if (i < editor->row_count - 1) {
            fputs("\n", file);
        }
    };

    fclose(file);
}

void append_text(char **buffer, char *text, size_t size) {
    char *tmp = realloc(*buffer, strlen(*buffer) + size + 1);
    if (tmp == NULL) {
        return;
    }

    *buffer = tmp;
    strncat(*buffer, text, size);
}

void append_text_slice(char **buffer, char *text, size_t start, size_t end) {
    size_t size = (end - start);

    if (size <= 0) {
        return;
    }

    char *tmp = realloc(*buffer, (strlen(*buffer) + size + 1));
    if (tmp == NULL) {
        return;
    }

    *buffer = tmp;
    strncat(*buffer, text + start, size);
}

void render_text(editor_state_t *editor, cursor_t *cursor) {
    char *buffer = calloc(1, sizeof(char));

    if (buffer == NULL) {
        return;
    }

    char terminal_cursor[20];

    int render_y = 0;
    size_t y = editor->text_window_start;

    int line_number_size = get_number_of_chars(editor->row_count + 1);
    char line_number[line_number_size];

    append_text(&buffer, (char *)CLEAR_TERMINAL_STRING, strlen(CLEAR_TERMINAL_STRING));

    for (; render_y < editor->max_text_window_height; y++, render_y++) {
        text_buffer_t *row = get_row(editor, y);

        if (row == NULL) {
            append_text(&buffer, " ", 1);
        } else {
            int number_size = get_number_of_chars(y + 1);
            int number_of_spaces = line_number_size - number_size;
            char spaces[number_of_spaces];
            int end = row->count;

            for (int i = 0; i <= number_of_spaces; i++) {
                spaces[i] = ' ';
            }

            append_text(&buffer, spaces, number_of_spaces);
            sprintf(line_number, " %zu  ", y + 1);
            append_text(&buffer, line_number, number_size + 3);

            if (row->count > editor->max_text_window_width) {
                if (cursor->ox + editor->max_text_window_width < row->count) {
                    end = cursor->ox + editor->max_text_window_width;
                }
            }

            if (cursor->ox >= 0 && cursor->ox < row->count) {
                append_text_slice(&buffer, row->data, cursor->ox, end);
            }
        }

        sprintf(terminal_cursor, "\033[%d;%dH", render_y + 2, 1);
        append_text(&buffer, terminal_cursor, strlen(terminal_cursor));
    }

    write(STDOUT_FILENO, buffer, strlen(buffer));
    free(buffer);
}

void render_footer(editor_state_t *editor, cursor_t *cursor) {
    char footer_text[editor->terminal_width + 1];
    char text[editor->terminal_width * 2];

    sprintf(footer_text, " %zu:%zu - %s", cursor->y + 1, cursor->x + 1, editor->file_path);

    int i = strlen(footer_text);
    while (i < editor->terminal_width) {
        strncat(footer_text, " ", 1);
        i++;
    }
    sprintf(text, "\033[47m\033[30m%s\033[0m", footer_text);

    move_cursor_in_terminal(1, editor->terminal_height);
    write(STDOUT_FILENO, text, strlen(text));
}

void render(editor_state_t *editor, cursor_t *cursor) {
    render_text(editor, cursor);
    render_footer(editor, cursor);
}

int main(int argc, char *argv[]) {
    int c;

    cursor_t *cursor = create_cursor(0, 0);
    editor_state_t *editor = create_editor_file();
    update_window_size(editor);
    text_buffer_t *first_row = create_text_buffer(2);

    if (first_row == NULL || editor == NULL) {
        exit(1);
    }

    append_row(editor, first_row);

    if (argc > 1) {
        load_file_in_editor(editor, argv[1]);
    }

    enable_raw_mode();
    render(editor, cursor);

    update_cursor_render_position(editor, cursor);

    int offset = get_number_of_chars(editor->row_count + 1) + 3;
    move_cursor_in_terminal(cursor->rx + offset + 1, cursor->ry + 1);

    text_buffer_t *current_row;

    while (1) {
        c = read_key(STDIN_FILENO);
        update_window_size(editor);
        current_row = get_row(editor, cursor->y);

        if (current_row == NULL) {
            break;
        }

        if (c == ESC) {
            break;
        } else if (c == BACKSPACE) {
            remove_char(current_row, cursor->rx - 1);

            if (cursor->rx == 0 && cursor->y > 0) {
                size_t new_cursor_x = editor->rows[cursor->y - 1]->count;
                merge_rows(editor, editor->rows[cursor->y - 1], editor->rows[cursor->y]);
                set_cursor_position(cursor, editor, new_cursor_x, cursor->y - 1);
            } else {
                set_cursor_position(cursor, editor, cursor->rx - 1, cursor->y);
            }

        } else if (c == DEL_KEY) {
            if (cursor->rx == current_row->count && cursor->y < editor->row_count - 1) {
                merge_rows(editor, editor->rows[cursor->y], editor->rows[cursor->y + 1]);
            } else {
                remove_char(current_row, cursor->x);
            }
        } else if (c == ENTER) {
            size_t new_len = current_row->count - cursor->x;
            text_buffer_t *new_line;

            if (new_len > 0) {
                new_line = create_text_buffer(new_len);
            } else {
                new_line = create_text_buffer(2);
            }

            if (new_line == NULL) {
                break;
            }

            if (new_len > 0) {
                // Move bytes in new line
                memmove(new_line->data, current_row->data + cursor->x, new_len);

                // Clear moved memory in current_row (now this is in new_line)
                for (size_t i = 0; i < new_len; i++) {
                    current_row->data[i + cursor->x] = 0x0;
                }

                current_row->count -= new_len;
                new_line->count = new_len;
            }

            insert_row(editor, cursor->y + 1, new_line);
            set_cursor_position(cursor, editor, 0, cursor->y + 1);
        } else if (c == ARROW_UP) {
            set_cursor_position(cursor, editor, cursor->x, cursor->y - 1);
        } else if (c == ARROW_DOWN) {
            if (cursor->y + 1 < editor->row_count) {
                set_cursor_position(cursor, editor, cursor->x, cursor->y + 1);
            }
        } else if (c == ARROW_LEFT) {
            if (cursor->y > 0 && cursor->x == 0) {
                text_buffer_t *row = get_row(editor, cursor->y - 1);

                set_cursor_position(cursor, editor, row->count, cursor->y - 1);
            } else {
                set_cursor_position(cursor, editor, cursor->rx - 1, cursor->y);
            }
        } else if (c == ARROW_RIGHT) {
            if (cursor->x < current_row->count) {
                set_cursor_position(cursor, editor, cursor->rx + 1, cursor->y);
            } else {
                if (cursor->y + 1 < editor->row_count) {
                    set_cursor_position(cursor, editor, 0, cursor->y + 1);
                }
            }
        } else if (c == CTRL_O) {
            save_file(editor, editor->file_path);
        } else {
            if (isprint(c)) {
                insert_char(current_row, cursor->rx, (char *)&c);
                set_cursor_position(cursor, editor, cursor->rx + 1, cursor->y);
            }
        }

        update_cursor_render_position(editor, cursor);
        render(editor, cursor);

#if DEBUG == 1
        move_cursor_in_terminal(1, editor->terminal_height - 2);
        printf(" Char Pressed char: '%c' val: '%i' \n", c, c);
        move_cursor_in_terminal(1, editor->terminal_height - 1);
        printf(" ^O Save \n");

#endif

        // The offset is used for the line number at the left of the terminal screen
        int offset = get_number_of_chars(editor->row_count + 1) + 3;
        move_cursor_in_terminal(cursor->rx + offset + 1, cursor->ry + 1);
    }

    disable_raw_mode();
    return 0;
}
