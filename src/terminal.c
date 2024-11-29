// Text used to clear the terminal texts
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "editor.h"
#include "terminal.h"
#include "util.h"

#define DEBUG 1

const char *CLEAR_TERMINAL_STRING = "\033[2J\033[1;1H";
const char *ENABLE_ALT_SCREEN = "\x1b[?1049h";
const char *DISABLE_ALT_SCREEN = "\x1b[?1049l";
const char *ENABLE_MOUSE_SCROLL = "\x1b[?1000h";
const char *DISABLE_MOUSE_SCROLL = "\x1b[?1000l";

static struct termios old_config, config;

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_config);
    write(STDOUT_FILENO, DISABLE_MOUSE_SCROLL, 8);
    write(STDOUT_FILENO, DISABLE_ALT_SCREEN, 8);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &old_config);
    config = old_config;

    config.c_iflag &= ~(BRKINT | INPCK | PARMRK | INLCR | IGNCR | ISTRIP | ICRNL | IXON);
    config.c_oflag &= ~(OPOST);
    config.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    config.c_cflag &= ~(CSIZE | PARENB);
    config.c_cflag |= (CS8);
    /* control chars - set return condition: min number of bytes and timer. */
    config.c_cc[VMIN] = 0;  /* Return each byte, or zero for timeout. */
    config.c_cc[VTIME] = 1; /* 100 ms timeout (unit is tens of second). */

    write(STDOUT_FILENO, ENABLE_ALT_SCREEN, 8);
    write(STDOUT_FILENO, ENABLE_MOUSE_SCROLL, 8);

    tcsetattr(STDIN_FILENO, TCSANOW, &config);

    atexit(disable_raw_mode);
}

void move_cursor_in_terminal(int x, int y) {
    char buffer[256];
    sprintf(buffer, "\033[%d;%dH", y, x);
    write(STDOUT_FILENO, buffer, strlen(buffer));
}

int read_key(int fd) {
    int nread;
    char c, seq[6];

    while ((nread = read(fd, &c, 1)) == 0)
        ;

    if (nread == -1) {
        exit(1);
    }

    while (1) {
        switch (c) {
        case ESC:
            /* escape sequence */
            /* If this is just an ESC, we'll timeout here. */
            if (read(fd, seq, 1) == 0)
                return ESC;
            if (read(fd, seq + 1, 1) == 0)
                return ESC;

            /* ESC [ sequences. */
            if (seq[0] == '[') {
                if (seq[1] >= '0' && seq[1] <= '9') {
                    /* Extended escape, read additional byte. */
                    if (read(fd, seq + 2, 1) == 0) {
                        return ESC;
                    }

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
                    case 'M':
                        // Mouse Sequence
                        if (read(fd, seq + 2, 1) == 0) {
                            return ESC;
                        }

                        char t[5];
                        // WORKAROUND to ignore mouse position and fix multiple mouse scroll events
                        // When you scroll fast stdin stores multiple mouse scroll actions as a single one -_-
                        // mouse events have a total of 5 items so the next 2 reads fulfill the sequence
                        //
                        // ALSO some terminal (like zed's editor terminal) scroll multiple times causing issues
                        if (read(fd, &t, 1) != 0) {
                            read(fd, &t, 1);
                        }

                        if (seq[2] == 'a') {
                            // SCROLL_DOWN
                            return ARROW_DOWN;
                        } else if (seq[2] == '`') {
                            // SCROLL_UP
                            return ARROW_UP;
                        }
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

    editor->max_text_window_height = editor->terminal_height - 2;
    editor->max_text_window_width = editor->terminal_width - 3 - get_number_of_chars(editor->row_count + 1);

#if DEBUG == 1
    editor->max_text_window_height -= 1;
#endif

    free(w);
}
