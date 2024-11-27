#include <complex.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "cursor.h"
#include "terminal.h"
#include "text-buffer.h"

// #define DEBUG

typedef struct EditorFile {
    text_buffer_t **rows;
    unsigned long row_count;
} editor_file_t;

void append_row(editor_file_t *editor, text_buffer_t *row) {
    editor->rows = realloc(editor->rows, (editor->row_count + 1) * sizeof(text_buffer_t));
    editor->rows[editor->row_count] = row;
    editor->row_count++;
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

int main() {
    int c;

    struct winsize *window = get_window_size();
    cursor_t *cursor = create_cursor(0, 0);
    editor_file_t *editor = create_editor_file();
    text_buffer_t *first_row = create_text_buffer(2);

    if (first_row == NULL || window == NULL || editor == NULL) {
        exit(1);
    }

    append_row(editor, first_row);

    static struct termios old_config, config;

    /*tcgetattr gets the parameters of the current terminal
    STDIN_FILENO will tell tcgetattr that it should write the settings
    of stdin to oldt*/
    tcgetattr(STDIN_FILENO, &old_config);
    /*now the settings will be copied*/
    config = old_config;

    /*
     * input modes: no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control.
     */
    config.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /* output modes - disable post processing */
    config.c_oflag &= ~(OPOST);
    /* control modes - set 8 bit chars */
    config.c_cflag |= (CS8);
    /* local modes - choing off, canonical off, no extended functions,
     * no signal chars (^Z,^C) */
    config.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    /* control chars - set return condition: min number of bytes and timer. */
    config.c_cc[VMIN] = 0;  /* Return each byte, or zero for timeout. */
    config.c_cc[VTIME] = 1; /* 100 ms timeout (unit is tens of second). */

    /*Those new settings will be set to STDIN
    TCSANOW tells tcsetattr to change attributes immediately. */
    tcsetattr(STDIN_FILENO, TCSANOW, &config);

    clear_terminal();

    text_buffer_t *current_row;

    while (1) {
        // window = get_window_size();
        // if (window == NULL) {
        //     exit(1);
        // }

        c = read_key(STDIN_FILENO);
        current_row = get_row(editor, cursor->y);

        if (current_row == NULL) {
            break;
        }

        if (c == ESC) {
            break;
        } else if (c == BACKSPACE) {
            remove_char(current_row, cursor->x - 1);
            if (cursor->x == 0 && cursor->y > 0) {
                move_cursor(cursor, editor->rows[cursor->y - 1]->count, cursor->y - 1);
            } else {
                move_cursor(cursor, cursor->x - 1, cursor->y);
            }

        } else if (c == DEL_KEY) {
            remove_char(current_row, cursor->x);
        } else if (c == ENTER) {
            text_buffer_t *new_line = create_text_buffer(2);
            if (new_line == NULL) {
                exit(1);
            }

            // TODO handle new line in the middle of a row
            append_row(editor, new_line);
            move_cursor(cursor, 0, cursor->y + 1);
        } else if (c == ARROW_UP) {
            if (cursor->y > 0) {
                // TODO: keep cursor position instead of resetting to 0
                cursor->x = 0;
            }
            move_cursor(cursor, cursor->x, cursor->y - 1);
        } else if (c == ARROW_DOWN) {
            if (cursor->y + 1 < editor->row_count) {
                move_cursor(cursor, 0, cursor->y + 1);
            }
        } else if (c == ARROW_LEFT) {
            move_cursor(cursor, cursor->x - 1, cursor->y);
        } else if (c == ARROW_RIGHT) {
            if (cursor->x < first_row->count) {
                move_cursor(cursor, cursor->x + 1, cursor->y);
            }
        } else {
            if (isprint(c)) {
                insert_char(current_row, cursor->x, (char *)&c);
                move_cursor(cursor, cursor->x + 1, cursor->y);
            }
        }

        clear_terminal();
        for (size_t i = 0; i < editor->row_count; i++) {
            move_cursor_in_terminal(0, i + 1);
            write(STDOUT_FILENO, editor->rows[i]->data, editor->rows[i]->count);
        }

#ifdef DEBUG
        printf("\n");
        move_cursor_in_terminal(0, editor->row_count + 1);
        printf("cursor pos (%zu,%zu)\n", cursor->x, cursor->y);
#endif

        move_cursor_in_terminal(cursor->x + 1, cursor->y + 1);
    }

    /*restore the old settings*/
    tcsetattr(STDIN_FILENO, TCSANOW, &old_config);

    return 0;
}
