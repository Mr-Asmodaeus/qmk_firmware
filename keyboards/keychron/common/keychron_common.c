/* Copyright 2023 ~ 2025 @ Keychron (https://www.keychron.com)
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
#include "keychron_common.h"
#include "raw_hid.h"
#include "version.h"

#ifdef FACTORY_TEST_ENABLE
#    include "factory_test.h"
#    include "keychron_common.h"
#endif

#ifdef LK_WIRELESS_ENABLE
#    include "lkbt51.h"
#endif

bool     is_siri_active = false;
uint32_t siri_timer     = 0;

static uint8_t mac_keycode[4] = {
    KC_LOPT,
    KC_ROPT,
    KC_LCMD,
    KC_RCMD,
};

// clang-format off
static key_combination_t key_comb_list[] = {
    {2, {KC_LWIN, KC_TAB}},
    {2, {KC_LWIN, KC_E}},
    {3, {KC_LSFT, KC_LCMD, KC_4}},
    {2, {KC_LWIN, KC_C}},
#ifdef WIN_LOCK_SCREEN_ENABLE
    {2, {KC_LWIN, KC_L}},
#endif
#ifdef MAC_LOCK_SCREEN_ENABLE
    {3, {KC_LCTL, KC_LCMD, KC_Q}},
#endif
};
// clang-format on

bool process_record_keychron_common(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_MCTRL:
            if (record->event.pressed) {
                register_code(KC_MISSION_CONTROL);
            } else {
                unregister_code(KC_MISSION_CONTROL);
            }
            return false; // Skip all further processing of this key
        case KC_LNPAD:
            if (record->event.pressed) {
                register_code(KC_LAUNCHPAD);
            } else {
                unregister_code(KC_LAUNCHPAD);
            }
            return false; // Skip all further processing of this key
        case KC_LOPTN:
        case KC_ROPTN:
        case KC_LCMMD:
        case KC_RCMMD:
            if (record->event.pressed) {
                register_code(mac_keycode[keycode - KC_LOPTN]);
            } else {
                unregister_code(mac_keycode[keycode - KC_LOPTN]);
            }
            return false; // Skip all further processing of this key
        case KC_SIRI:
            if (record->event.pressed) {
                if (!is_siri_active) {
                    is_siri_active = true;
                    register_code(KC_LCMD);
                    register_code(KC_SPACE);
                }
                siri_timer = timer_read32();
            } else {
                // Do something else when release
            }
            return false; // Skip all further processing of this key
        case KC_TASK:
        case KC_FILE:
        case KC_SNAP:
        case KC_CTANA:
#ifdef WIN_LOCK_SCREEN_ENABLE
        case KC_WLCK:
#endif
#ifdef MAC_LOCK_SCREEN_ENABLE
        case KC_MLCK:
#endif
            if (record->event.pressed) {
                for (uint8_t i = 0; i < key_comb_list[keycode - KC_TASK].len; i++) {
                    register_code(key_comb_list[keycode - KC_TASK].keycode[i]);
                }
            } else {
                for (uint8_t i = 0; i < key_comb_list[keycode - KC_TASK].len; i++) {
                    unregister_code(key_comb_list[keycode - KC_TASK].keycode[i]);
                }
            }
            return false; // Skip all further processing of this key
#ifdef LED_MATRIX_ENABLE
        case BL_SPI:
                if (record->event.pressed) {
                    led_matrix_increase_speed();
                }
            return true;
        case BL_SPD:
                if (record->event.pressed) {
                    led_matrix_decrease_speed();
                }
            return true;
#endif
        default:
            return true; // Process all other keycodes normally
    }
}

void keychron_common_task(void) {
    if (is_siri_active && timer_elapsed32(siri_timer) > 500) {
        unregister_code(KC_LCMD);
        unregister_code(KC_SPACE);
        is_siri_active = false;
        siri_timer     = 0;
    }
}

#ifdef ENCODER_ENABLE
static void encoders_pins_cb(void *param) {
    uint8_t index = (uint32_t)param;
    extern void encoder_quadrature_handle_inerrupt_read(uint8_t index);
    encoder_quadrature_handle_inerrupt_read(index);
}

void encoder_cb_init(void) {
    pin_t encoders_pin_a[] = ENCODER_A_PINS;
    pin_t encoders_pin_b[] = ENCODER_B_PINS;
    for (uint32_t i=0; i<NUM_ENCODERS; i++)
    {
        palEnableLineEvent(encoders_pin_a[i], PAL_EVENT_MODE_BOTH_EDGES);
        palEnableLineEvent(encoders_pin_b[i], PAL_EVENT_MODE_BOTH_EDGES);
        palSetLineCallback(encoders_pin_a[i], encoders_pins_cb, (void*)i);
        palSetLineCallback(encoders_pin_b[i], encoders_pins_cb, (void*)i);
    }
}
#endif

//__attribute__((weak)) bool raw_hid_receive_keychron(uint8_t *data, uint8_t length) { return true; }
#define PROTOCOL_VERSION 0x02

enum { kc_get_protocol_version = 0xA0, kc_get_firmware_version = 0xA1, kc_get_support_feature = 0xA2, kc_get_default_layer = 0xA3 };

enum {
    FEATURE_DEFAULT_LAYER = 0x01 << 0,
    FEATURE_BLUETOOTH     = 0x01 << 1,
    FEATURE_P2P4G         = 0x01 << 2,
    FEATURE_ANALOG_MATRIX = 0x01 << 3,
};

void get_support_feature(uint8_t *data) {
    data[1] = FEATURE_DEFAULT_LAYER
#ifdef KC_BLUETOOTH_ENABLE
              | FEATURE_BLUETOOTH
#endif
#ifdef LK_WIRELESS_ENABLE
              | FEATURE_BLUETOOTH | FEATURE_P2P4G
#endif
#ifdef ANANLOG_MATRIX
              | FEATURE_ANALOG_MATRIX
#endif
        ;
}

bool kc_raw_hid_rx(uint8_t *data, uint8_t length) {
    // if (!raw_hid_receive_keychron(data, length))
    //     return false;
    switch (data[0]) {
        case kc_get_protocol_version:
            data[1] = PROTOCOL_VERSION;
            raw_hid_send(data, length);
            break;

        case kc_get_firmware_version: {
            uint8_t i = 1;
            data[i++] = 'v';
            if ((DEVICE_VER & 0xF000) != 0) itoa((DEVICE_VER >> 12), (char *)&data[i++], 16);
            itoa((DEVICE_VER >> 8) & 0xF, (char *)&data[i++], 16);
            data[i++] = '.';
            itoa((DEVICE_VER >> 4) & 0xF, (char *)&data[i++], 16);
            data[i++] = '.';
            itoa(DEVICE_VER & 0xF, (char *)&data[i++], 16);
            data[i++] = ' ';
            memcpy(&data[i], QMK_BUILDDATE, sizeof(QMK_BUILDDATE));
            i += sizeof(QMK_BUILDDATE);
            raw_hid_send(data, length);
        } break;

        case kc_get_support_feature:
            get_support_feature(&data[1]);
            raw_hid_send(data, length);
            break;

        case kc_get_default_layer:
            data[1] = get_highest_layer(default_layer_state);
            raw_hid_send(data, length);
            break;

#ifdef ANANLOG_MATRIX
        case 0xA9:
            analog_matrix_rx(data, length);
            break;
#endif
#ifdef LK_WIRELESS_ENABLE
        case 0xAA:
            lkbt51_dfu_rx(data, length);
            break;
#endif
#ifdef FACTORY_TEST_ENABLE
        case 0xAB:
            factory_test_rx(data, length);
            break;
#endif
        default:
            return false;
    }

    return true;
}

#if defined(VIA_ENABLE)
bool via_command_kb(uint8_t *data, uint8_t length) {
    return kc_raw_hid_rx(data, length);
}
#else
void raw_hid_receive(uint8_t *data, uint8_t length) {
    kc_raw_hid_rx(data, length);
}
#endif

