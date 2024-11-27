#pragma once

#include <sys/ioctl.h>

enum KEY_ACTION {
    KEY_NULL = 0,    /* NULL */
    CTRL_C = 3,      /* Ctrl-c */
    CTRL_D = 4,      /* Ctrl-d */
    CTRL_F = 6,      /* Ctrl-f */
    CTRL_H = 8,      /* Ctrl-h */
    TAB = 9,         /* Tab */
    CTRL_L = 12,     /* Ctrl+l */
    ENTER = 13,      /* Enter */
    CTRL_Q = 17,     /* Ctrl-q */
    CTRL_S = 19,     /* Ctrl-s */
    CTRL_U = 21,     /* Ctrl-u */
    ESC = 27,        /* Escape */
    BACKSPACE = 127, /* Backspace */
    /* The following are just soft codes, not really reported by the
     * terminal directly. */
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN
};


void move_cursor_in_terminal(int x, int y);
void clear_terminal();
int read_key(int fd);

struct winsize *get_window_size();
