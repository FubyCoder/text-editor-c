// Text used to clear the terminal texts
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "editor.h"
#include "terminal.h"

#define DEBUG 1

const char *CLEAR_TERMINAL_STRING = "\033[2J\033[1;1H";

void move_cursor_in_terminal(int x, int y) {
    char buffer[256];
    sprintf(buffer, "\033[%d;%dH", y, x);
    write(STDOUT_FILENO, buffer, strlen(buffer));
}

void clear_terminal() {
    //
    write(STDOUT_FILENO, CLEAR_TERMINAL_STRING, strlen(CLEAR_TERMINAL_STRING));
}

int read_key(int fd) {
    int nread;
    char c, seq[3];

    while ((nread = read(fd, &c, 1)) == 0)
        ;

    if (nread == -1) {
        exit(1);
    }

    while (1) {
        switch (c) {
        case ESC: /* escape sequence */
            /* If this is just an ESC, we'll timeout here. */
            if (read(fd, seq, 1) == 0)
                return ESC;
            if (read(fd, seq + 1, 1) == 0)
                return ESC;

            /* ESC [ sequences. */
            if (seq[0] == '[') {
                if (seq[1] >= '0' && seq[1] <= '9') {
                    /* Extended escape, read additional byte. */
                    if (read(fd, seq + 2, 1) == 0)
                        return ESC;
                    if (seq[2] == '~') {
                        switch (seq[1]) {
                        case '3':
                            return DEL_KEY;
                        case '5':
                            return PAGE_UP;
                        case '6':
                            return PAGE_DOWN;
                        }
                    }
                } else {
                    switch (seq[1]) {
                    case 'A':
                        return ARROW_UP;
                    case 'B':
                        return ARROW_DOWN;
                    case 'C':
                        return ARROW_RIGHT;
                    case 'D':
                        return ARROW_LEFT;
                    case 'H':
                        return HOME_KEY;
                    case 'F':
                        return END_KEY;
                    }
                }
            } else if (seq[0] == 'O') {
                /* ESC O sequences. */
                switch (seq[1]) {
                case 'H':
                    return HOME_KEY;
                case 'F':
                    return END_KEY;
                }
            }
            break;
        default:
            return c;
        }
    }
}

void update_window_size(editor_state_t *editor) {
    struct winsize *w = malloc(sizeof(struct winsize));
    ioctl(STDOUT_FILENO, TIOCGWINSZ, w);

    editor->terminal_width = w->ws_col;
    editor->terminal_height = w->ws_row;

    editor->max_text_window_height = editor->terminal_height - 1;

#if DEBUG == 1
    editor->max_text_window_height -= 1;
#endif

    free(w);
}
