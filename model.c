#include "model.h"
#include "interface.h"

#include <stddef.h>
#include <stdlib.h>

void model_init() {
    // TODO: implement this.
}

void set_cell_value(ROW row, COL col, char *text) {
    // TODO: implement this.

    // This just displays the text without saving it in any data structure. You will need to change this.
    update_cell_display(row, col, text);
    free(text);
}

void clear_cell(ROW row, COL col) {
    // TODO: implement this.

    // This just clears the display without updating any data structure. You will need to change this.
    update_cell_display(row, col, "");
}

char *get_textual_value(ROW row, COL col) {
    // TODO: implement this.
    return NULL;
}
