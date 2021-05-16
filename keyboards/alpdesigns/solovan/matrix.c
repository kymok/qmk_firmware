#include <stdint.h>
#include <stdbool.h>
#include "util.h"
#include "matrix.h"
#include "debounce.h"
#include "quantum.h"

static const pin_t row_pins[] = {D1, B0, F0};
static const pin_t col_pins[] = {C6, B6, B5, B4, D7, D6, D5, D2, D3};
const int CUSTOM_MATRIX_ROWS = 3;
const int CUSTOM_MATRIX_COLS = 9;

// Matrix State
extern matrix_row_t raw_matrix[MATRIX_ROWS];  // raw values
extern matrix_row_t matrix[MATRIX_ROWS];      // debounced values

// Pin Operations
static inline void setPinOutput_writeLow(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        setPinOutput(pin);
        writePinLow(pin);
    }
}

static inline void setPinInputHigh_atomic(pin_t pin) {
    ATOMIC_BLOCK_FORCEON { setPinInputHigh(pin); }
}

// Col2ROW

static void select_row(uint8_t row) { setPinOutput_writeLow(row_pins[row]); }

static void unselect_row(uint8_t row) { setPinInputHigh_atomic(row_pins[row]); }

static bool read_cols_on_row(matrix_row_t current_matrix[], uint8_t current_row, uint8_t row_offset) {
    // Start with a clear matrix row
    matrix_row_t current_row_value = 0;

    // Select row
    select_row(current_row);
    matrix_output_select_delay();

    // For each col...
    for (uint8_t col_index = 0; col_index < CUSTOM_MATRIX_COLS; col_index++) {
        // Select the col pin to read (active low)
        uint8_t pin_state = readPin(col_pins[col_index]);

        // Populate the matrix row with the state of the col pin
        current_row_value |= pin_state ? 0 : (MATRIX_ROW_SHIFTER << col_index);
    }

    // Unselect row
    unselect_row(current_row);
    if (current_row + 1 < CUSTOM_MATRIX_ROWS) {
        matrix_output_unselect_delay();  // wait for row signal to go HIGH
    }

    // If the row has changed, store the row and return the changed flag.
    if (current_matrix[current_row + row_offset] != current_row_value) {
        current_matrix[current_row + row_offset] = current_row_value;
        return true;
    }
    return false;
}

// ROW2COL

static void select_col(uint8_t col) { setPinOutput_writeLow(col_pins[col]); }

static void unselect_col(uint8_t col) { setPinInputHigh_atomic(col_pins[col]); }

static bool read_rows_on_col(matrix_row_t current_matrix[], uint8_t current_col, uint8_t row_offset) {
    bool matrix_changed = false;

    // Select col
    select_col(current_col);
    matrix_output_select_delay();

    // For each row...
    for (uint8_t row_index = 0; row_index < CUSTOM_MATRIX_ROWS; row_index++) {
        // Store last value of row prior to reading
        matrix_row_t last_row_value    = current_matrix[row_index + row_offset];
        matrix_row_t current_row_value = last_row_value;

        // Check row pin state
        if (readPin(row_pins[row_index]) == 0) {
            // Pin LO, set col bit
            current_row_value |= (MATRIX_ROW_SHIFTER << current_col);
        } else {
            // Pin HI, clear col bit
            current_row_value &= ~(MATRIX_ROW_SHIFTER << current_col);
        }

        // Determine if the matrix changed state
        if ((last_row_value != current_row_value)) {
            matrix_changed |= true;
            current_matrix[row_index + row_offset] = current_row_value;
        }
    }

    // Unselect col
    unselect_col(current_col);
    if (current_col + 1 < CUSTOM_MATRIX_COLS) {
        matrix_output_unselect_delay();  // wait for col signal to go HIGH
    }

    return matrix_changed;
}

// Unselect All

static void init_pins(void) {
    for (uint8_t x = 0; x < CUSTOM_MATRIX_COLS; x++) {
        setPinInputHigh_atomic(col_pins[x]);
    }
    for (uint8_t x = 0; x < CUSTOM_MATRIX_ROWS; x++) {
        setPinInputHigh_atomic(row_pins[x]);
    }
}

void matrix_init_custom(void) {
    init_pins();
}

// Scan Code

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool matrix_has_changed = false;

    // Read cols
    init_pins();
    for (uint8_t current_row = 0; current_row < CUSTOM_MATRIX_ROWS; current_row++) {
        matrix_has_changed |= read_cols_on_row(raw_matrix, current_row, 0);
    }

    // Read rows
    init_pins();
    for (uint8_t current_col = 0; current_col < CUSTOM_MATRIX_COLS; current_col++) {
        matrix_has_changed |= read_rows_on_col(raw_matrix, current_col, CUSTOM_MATRIX_ROWS);
    }

    debounce(raw_matrix, matrix, MATRIX_ROWS, matrix_has_changed);

    return matrix_has_changed;
}
