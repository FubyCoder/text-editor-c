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

#define DEBUG

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
                size_t new_cursor_x = editor->rows[cursor->y - 1]->count;
                merge_rows(editor, editor->rows[cursor->y - 1], editor->rows[cursor->y]);
                move_cursor(cursor, new_cursor_x, cursor->y - 1);
            } else {
                move_cursor(cursor, cursor->x - 1, cursor->y);
            }

        } else if (c == DEL_KEY) {
            if (cursor->x == current_row->count && cursor->y < editor->row_count - 1) {
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
                exit(1);
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
            move_cursor(cursor, 0, cursor->y + 1);
        } else if (c == ARROW_UP) {
            move_cursor(cursor, cursor->x, cursor->y - 1);
        } else if (c == ARROW_DOWN) {
            if (cursor->y + 1 < editor->row_count) {
                move_cursor(cursor, cursor->x, cursor->y + 1);
            }
        } else if (c == ARROW_LEFT) {
            move_cursor(cursor, cursor->rx - 1, cursor->y);
        } else if (c == ARROW_RIGHT) {
            if (cursor->x < current_row->count) {
                move_cursor(cursor, cursor->rx + 1, cursor->y);
            }
        } else {
            if (isprint(c)) {
                insert_char(current_row, cursor->rx, (char *)&c);
                move_cursor(cursor, cursor->rx + 1, cursor->y);
            }
        }

        clear_terminal();
        for (size_t i = 0; i < editor->row_count; i++) {
            move_cursor_in_terminal(0, i + 1);
            write(STDOUT_FILENO, editor->rows[i]->data, editor->rows[i]->count);
        }

        update_cursor_render_position(editor, cursor);

#ifdef DEBUG
        printf("\n");
        move_cursor_in_terminal(0, editor->row_count + 1);
        printf("cursor pos render:(%zu,%zu) real:(%zu,%zu)\n", cursor->rx, cursor->ry, cursor->x, cursor->y);
#endif

        move_cursor_in_terminal(cursor->rx, cursor->ry + 1);
    }

    /*restore the old settings*/
    tcsetattr(STDIN_FILENO, TCSANOW, &old_config);

    return 0;
}
