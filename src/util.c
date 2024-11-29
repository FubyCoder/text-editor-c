#include "stdlib.h"

int get_number_of_chars(size_t value) {
    int size = 0;

    while (value > 0) {
        value = value / 10;
        size++;
    }

    return size;
}
