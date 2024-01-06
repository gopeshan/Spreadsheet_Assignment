#include "model.h"
#include "interface.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LEN 256
#define EQUALS_CHAR '='

// Define a structure to assist in managing dynamic arrays.
typedef struct {
    // Pointer to the base of the dynamic array
    double *base;
    // Stack pointer pointing to the current position in the array
    double *sp;
    // Current size (number of elements) in the array
    size_t size;
    // Capacity (maximum number of elements) of the array
    size_t capacity;
} DoubleAssist;

// Function to create and initialize a new dynamic array assist structure.
DoubleAssist make_double_assist() {

    // Allocate memory for a dynamic array of doubles with an initial capacity of 16.
    double *temp = malloc(16 * sizeof(double));

    // Create and return a new DoubleAssist structure with initialized properties.
    return (DoubleAssist){temp, temp, 0, 16};
}


// Function to grow the capacity of the dynamic array used in DoubleAssist structure.
void double_assist_grow(DoubleAssist *assist) {
    // Attempt to reallocate memory for the dynamic array with double the current capacity.
    double *temp = realloc(assist->base, 2 * assist->capacity * sizeof(double));

    // Check if the reallocation was successful.
    if (temp) {
        // Update the DoubleAssist structure with the new base address, stack pointer, and capacity.
        assist->base = temp;
        assist->sp = assist->base + assist->size;
        assist->capacity *= 2;
    } else {
        // If memory allocation fails, print an error message and exit the program.
        fprintf(stderr, "ERROR: Memory allocation failure\n");
        exit(EXIT_FAILURE);
    }
}


// Function to push a new element onto the dynamic array.
void double_assist_push(DoubleAssist *assist, double value) {
    // Check if the current size has reached the capacity, and if so, grow the array.
    if (assist->size >= assist->capacity)
        double_assist_grow(assist);

    // Add the new element to the dynamic array and update the stack pointer and size.
    *(assist->sp)++ = value;
    ++assist->size;
}


// Function to pop an element from the dynamic array.
double double_assist_pop(DoubleAssist *assist) {
    // Check if the array is empty, and if so, print an error message and exit.
    if (assist->size == 0) {
        fprintf(stderr, "ERROR: Invalid operation\n");
        exit(EXIT_FAILURE);
    }

    // Decrement the size and stack pointer, then return the popped element.
    --assist->size;
    --assist->sp;
    return *(assist->sp);
}

// Function to delete the dynamic array and free associated memory.
void double_assist_delete(DoubleAssist *assist) {
    // Free the memory allocated for the dynamic array.
    free(assist->base);

    // Reset the size and capacity to indicate an empty array.
    assist->size = 0;
    assist->capacity = 0;
}

// Enumeration to represent different types of cells.
typedef enum {
    // Represents an empty or uninitialized cell
    none,
    // Represents a cell containing a string value
    str,
    // Represents a cell containing a numeric value
    num,
    // Represents a cell containing an equation or formula
    eqn,
} CELL_TYPE;

// Structure to represent a cell with a type, numeric value, and string value.
typedef struct cell {
    // Type of the cell (string, numeric, etc.)
    CELL_TYPE type;
    // Numeric value of the cell (valid if type is num)
    double numVal;
    // String value of the cell (valid if type is str or eqn)
    char *strVal;
} cell;


// Definition of a 2D array 'sheet' representing a table of cells with NUM_ROWS rows and NUM_COLS columns.
cell sheet[NUM_ROWS][NUM_COLS] = {0};

// Function to skip leading whitespace characters in a given text.
const char *skip_whitespace(const char *text) {
    while (*text && isspace((unsigned char)*text)) {
        text++;
    }
    return text;
}

// Function to check if a given string is a valid numeric representation.
bool is_valid_num(const char *str) {
    if (!str)
        return false;

    // Skip leading whitespace characters.
    str = skip_whitespace(str);

    // Flags to track presence of digits and decimal point.
    bool hasDigit = false;
    bool hasDecimalPoint = false;

    // Iterate through the characters in the string.
    while (*str) {
        if (isdigit((unsigned char)*str)) {
            hasDigit = true;
        } else if (*str == '.') {
            // Check for multiple decimal points.
            if (hasDecimalPoint) {
                return false;
            }
            hasDecimalPoint = true;
        } else {
            // Invalid character encountered.
            return false;
        }
        ++str;
    }

    // Check if at least one digit is present.
    return hasDigit;
}

// Function to check if a given string is a valid formula.
bool is_valid_formula(const char *formula) {
    // Skip leading whitespaces.
    formula = skip_whitespace(formula);

    // Check if the formula starts with '='.
    if (formula[0] != '=')
        return false;

    // Move to the next character after '='.
    ++formula;

    // Iterate through the formula characters.
    while (*formula) {
        // Skip leading whitespaces.
        formula = skip_whitespace(formula);

        // Check for valid formula characters.
        if (isalpha((unsigned char)*formula)) {
            // Check for uppercase alphabets.
            if (islower((unsigned char)*formula)) {
                return false;
            }
        } else if (isdigit((unsigned char)*formula) || *formula == '.' || *formula == '+') {
            // Valid characters for formula.
        } else {
            // Invalid character encountered.
            return false;
        }

        // Move to the next character.
        ++formula;
    }

    // The formula is considered valid if all characters pass the checks.
    return true;
}

// Function to set the numeric value in a cell and free existing memory.
void set_num_value(ROW row, COL col, const char *text) {
    // Free existing memory for the string value.
    free(sheet[row][col].strVal);

    // Set the cell type to num.
    sheet[row][col].type = num;

    // Convert the input text to a double and set the numeric value.
    sheet[row][col].numVal = strtod(text, NULL);

    // Set the string value to NULL.
    sheet[row][col].strVal = NULL;
}

// Function to set the string value in a cell and free existing memory.
void set_string_value(ROW row, COL col, const char *text) {
    // Free existing memory for the string value.
    free(sheet[row][col].strVal);

    // Set the cell type to str.
    sheet[row][col].type = str;

    // Set the numeric value to 0.
    sheet[row][col].numVal = 0;

    // Duplicate and set the string value.
    sheet[row][col].strVal = strdup(text);
}

// Function to initialize a cell if its type is none.

void initialize_cell(ROW row, COL col) {
    // Check if the cell type is already set.
    if (sheet[row][col].type != none)
        return;

    // Free existing memory for the string value.
    free(sheet[row][col].strVal);

    // Initialize the cell with type num, numeric value 0, and an empty string.
    sheet[row][col] = (cell){.type = num, .numVal = 0, .strVal = strdup("")};
}

// Function to parse and calculate the result of a formula.
bool parse_and_calculate_formula(char *formula, double *result) {
    // Initialize the result to 0.
    *result = 0;

    // Check if the formula is valid.
    if (!is_valid_formula(formula)) {
        return false;
    }

    // Create a dynamic array assist structure for numeric values.
    DoubleAssist numAssist = make_double_assist();

    // Variable to count the number of operators in the formula.
    int opcount = 0;

    // Loop through the characters in the formula.
    while (*formula) {
        if (*formula == EQUALS_CHAR || *formula == ' ') {
            // Skip equals sign and spaces.
            formula++;
        } else if (*formula == '+') {
            // Increment the operator count and move to the next character.
            opcount++;
            formula++;
        } else if (isalpha(*formula)) {
            // Check if the next character is a digit.
            if (!isdigit(*(formula + 1))) {
                // Invalid formula if the character after the alphabet is not a digit.
                double_assist_delete(&numAssist);
                return false;
            }

            // Calculate the column index from the alphabet character.
            COL curCol = (COL)(*formula - 'A');
            formula++;

            // Calculate the row index from the following digits.
            ROW curRow = strtol(formula, &formula, 10) - 1;

            // Check if the row and column indices are within bounds.
            if (curRow < 0 || curRow >= NUM_ROWS || curCol < 0 || curCol >= NUM_COLS) {
                double_assist_delete(&numAssist);
                return false;
            }

            // Push the numeric value from the referenced cell onto the numeric assist stack.
            double_assist_push(&numAssist, sheet[curRow][curCol].numVal);
        } else if (isdigit(*formula) || *formula == '.') {
            // Push the numeric value onto the numeric assist stack and update the formula pointer.
            double_assist_push(&numAssist, strtod(formula, &formula));
        } else {
            // Invalid character in the formula.
            double_assist_delete(&numAssist);
            return false;
        }
    }

    // Check if the number of numeric values is one less than the number of operators.
    if (numAssist.size != opcount + 1) {
        double_assist_delete(&numAssist);
        return false;
    }

    // Sum all numeric values in the stack to calculate the final result.
    while (numAssist.size > 0) {
        *result += double_assist_pop(&numAssist);
    }

    // Free the memory used by the numeric assist stack.
    double_assist_delete(&numAssist);

    // Formula parsing and calculation successful.
    return true;
}

// Function to update the value of a cell based on its type.
void update_cell_value(ROW row, COL col) {
    // Check if the cell contains a formula.
    if (sheet[row][col].type == eqn) {
        double result;

        // Parse and calculate the formula.
        if (parse_and_calculate_formula(sheet[row][col].strVal, &result)) {
            sheet[row][col].numVal = result;
            sheet[row][col].type = num;
            update_cell_display(row, col, sheet[row][col].strVal);
        } else {
            // Display "ERROR" if the formula is invalid.
            update_cell_display(row, col, "ERROR");
        }
    }
}

// Function to set the value of a cell in a spreadsheet.
void set_cell_value(ROW row, COL col, char *text) {
    // Check if the input text is empty or NULL.
    if (text == NULL || *text == '\0') {
        return;
    }

    // Check if the cell is uninitialized and initialize it.
    if (sheet[row][col].type == none) {
        initialize_cell(row, col);
    }

    // Check if the input text represents a numeric value.
    if (is_valid_num(text)) {
        set_num_value(row, col, text);
    } else {
        // Set the string value and handle formulas.
        set_string_value(row, col, text);
        if (is_valid_formula(text)) {
            double result;

            // Parse and calculate the formula.
            if (parse_and_calculate_formula(text, &result)) {
                sheet[row][col].type = eqn;
                char *formattedResult = calloc(MAX_LEN, sizeof(char));
                snprintf(formattedResult, MAX_LEN, "%lg", result);
                free(text);
                text = formattedResult;
            } else {
                // Set the cell value to "ERROR" if the formula is invalid.
                free(text);
                text = strdup("ERROR");
            }
        }
    }

    // Update all cells in the spreadsheet.
    for (ROW i = ROW_1; i < NUM_ROWS; ++i) {
        for (COL j = COL_A; j < NUM_COLS; ++j) {
            if (!(i == row && j == col)) {
                update_cell_value(i, j);
            }
        }
    }

    // Update the cell display with the final value.
    update_cell_display(row, col, text);

    // Free memory if the cell type is a formula or string.
    if (sheet[row][col].type == eqn || sheet[row][col].type == str) {
        free(text);
    }
}

// Function to clear a cell in a 2D array and update its display.
void clear_cell(ROW row, COL col) {
    // Check if the cell is already empty.
    if (sheet[row][col].type == none)
        return;

    // Free memory if the cell contains a string value.
    free(sheet[row][col].strVal);

    // Set the cell to its default state.
    sheet[row][col] = (cell){.type = none, .numVal = 0.0, .strVal = NULL};

    // Update the cell display.
    update_cell_display(row, col, "");
}

// Function to retrieve the textual value of a cell in a 2D array.
char *get_textual_value(ROW row, COL col) {
    // Get the cell at the specified position.
    cell *this = &(sheet[row][col]);

    // Allocate memory for the result string.
    char *result = calloc(MAX_LEN, sizeof(char));

    // Check for memory allocation failure.
    if (result == NULL) {
        return NULL;
    }

    // Copy the value based on the cell type.
    switch (this->type) {
        case eqn:
        case num:
        case str:
            strncpy(result, this->strVal, MAX_LEN - 1);
            result[MAX_LEN - 1] = '\0';
            break;
        case none:
            *result = '\0';
            break;
    }

    return result;
}

// Function to free the memory allocated for the textual value.
void free_textual_value(char *textual_value) {
    free(textual_value);
}

// Attempted Implementation For Dependence
/*

// Function to count the number of cells with type function in the spreadsheet.
int count_function_cells() {
    int numForm = 0;

    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLS; j++) {
            if (sheet[i][j].type == function) {
                numForm++;
            }
        }
    }

    return numForm;
}

// Function to perform dependency check and update cells in the spreadsheet.
void perform_dependency_check() {
    int numForm = count_function_cells();

    for (int q = 0; q < numForm; q++) {
        for (int i = 0; i < NUM_ROWS; i++) {
            for (int j = 0; j < NUM_COLS; j++) {
                if (!(i == row && j == col)) {
                    dependencyCheck(i, j);
                }
            }
        }
    }
}

// Function to check dependencies, update cell value, and display in the spreadsheet.
void dependencyCheck(ROW row, COL col) {
    if (sheet[row][col].type == function) {
        sheet[row][col].numVal = parser(row, col);
    }
    update_cell_value(row, col);
    update_cell_display(row, col, get_display_value(row, col));
    free(get_display_value(row, col));
}

*/
