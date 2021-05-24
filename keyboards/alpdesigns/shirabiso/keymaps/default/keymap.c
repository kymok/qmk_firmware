/* Copyright 2021 kymok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H

// Defines names for use in layer keycodes and the keymap
enum layer_names {
    _BASE,
    _L1,
    _L2,
    _NUM,
};

// Defines the keycodes used by our macros in process_record_user
enum custom_keycodes {
    L1_SPC = SAFE_RANGE,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Base */
    [_BASE] = LAYOUT(
        KC_ESC, KC_TAB, KC_Q,   KC_W,   KC_E,   KC_R,   KC_T,   KC_Y,   KC_U,   KC_I,   KC_O,   KC_P,   KC_MINS,KC_EQL,
        KC_LCTL,        KC_A,   KC_S,   KC_D,   KC_F,   KC_G,   KC_H,   KC_J,   KC_K,   KC_L,   KC_SCLN,KC_ENT,
        KC_LSFT,        KC_Z,   KC_X,   KC_C,   KC_V,   KC_B,   KC_N,   KC_M,   KC_COMM,KC_DOT, KC_SLSH,KC_RSFT,
                KC_SPC, KC_LOPT,KC_LCMD,MO(_L2),L1_SPC,                 L1_SPC, MO(_L2),KC_RCMD,KC_ROPT,KC_BSPC
    ),
    [_L1] = LAYOUT(
        KC_GRV, KC_BSLS,KC_1,   KC_2,   KC_3,   KC_4,   KC_5,   KC_6,   KC_7,   KC_8,   KC_9,   KC_0,   KC_LBRC,KC_RBRC,
        _______,     _______,S(KC_LBRC),KC_LBRC,S(KC_9),KC_MINS,KC_EQL, S(KC_0),KC_RBRC,S(KC_RBRC),KC_QUOT,_______,
        _______,        _______,_______,_______,S(KC_COMM),S(KC_MINS),S(KC_EQL),S(KC_DOT),_______,_______,_______,_______,
                _______,_______,_______,_______,_______,                _______,_______,_______,_______,KC_DEL
    ),
    [_L2] = LAYOUT(
        _______,_______,KC_PGUP,KC_HOME,KC_UP,  KC_END, _______,_______,_______,KC_UP,  _______,_______,_______,_______,
        _______,        KC_PGDN,KC_LEFT,KC_DOWN,KC_RGHT,_______,_______,KC_LEFT,KC_DOWN,KC_RGHT,_______, _______,
        _______,        _______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,
                _______,_______,_______,_______,_______,                _______,_______,_______,_______,_______
    ),
    [_NUM] = LAYOUT(
        KC_NLCK,_______,KC_PSLS,KC_P7,  KC_P8,  KC_P9,  KC_PMNS,KC_F6,  KC_F7,  KC_F8,  KC_F9,  KC_F10, KC_F11, KC_F12,
        KC_CAPS,        KC_PAST,KC_P4,  KC_P5,  KC_P6,  KC_PPLS,KC_F1,  KC_F2,  KC_F3,  KC_F4,  KC_F5,  _______,
        _______,        _______,KC_P1,  KC_P2,  KC_P3,  KC_PENT,_______,_______,_______,_______,_______,_______,
                _______,KC_P0,  KC_PDOT,_______,_______,                _______,_______,_______,_______,_______
    )
};

// Permissive hold for layers

static uint16_t start;
uint16_t prev_press = 0x00;
const int tap_limit = 200;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {

        // FN1 press then L1 on without delay.
        // KC_SPC on FN1 release if no other keys were pressed and FN1 was released within tap_limit.

        case L1_SPC:
        if (record -> event.pressed) {
            start = timer_read();
            layer_on(_L1);
        }
        else {
            if (prev_press == L1_SPC && timer_elapsed(start) < tap_limit) {
                tap_code16(KC_SPC);
            }
            layer_off(_L1);
        }
        break;
    }

    // Store previous key press
    if (record -> event.pressed) {
        prev_press = keycode;
    }

    return true;
}


const rgblight_segment_t PROGMEM base_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {0, 2, 0, 0, 0}
);

const rgblight_segment_t PROGMEM fn1_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {0, 2, 0, 0, 32}
);

const rgblight_segment_t PROGMEM fn2_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {0, 2, 0, 0, 128}
);

const rgblight_segment_t PROGMEM num_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {0, 2, 85, 255, 128}
);

const rgblight_segment_t PROGMEM caps_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {0, 1, 0, 255, 128} // Left LED Only
);

const rgblight_segment_t* const PROGMEM my_rgb_layers[] = RGBLIGHT_LAYERS_LIST(
    base_layer,
    fn1_layer,
    fn2_layer,
    num_layer,
    caps_layer
);

void keyboard_post_init_user(void) {
    // Enable the LED layers
    rgblight_layers = my_rgb_layers;
    rgblight_enable_noeeprom();
    rgblight_set_layer_state(0, true);
}

layer_state_t layer_state_set_user(layer_state_t state) {
    state = update_tri_layer_state(state, _L1, _L2, _NUM);
    rgblight_set_layer_state(1, layer_state_cmp(state, _L1));
    rgblight_set_layer_state(2, layer_state_cmp(state, _L2));
    rgblight_set_layer_state(3, layer_state_cmp(state, _NUM));
    return state;
}

bool led_update_user(led_t led_state) {
    rgblight_set_layer_state(4, led_state.caps_lock);
    return true;
}
