//#pragma message "leader_key_user.c"

#include "leader_key_user.h"
#include "mystrings.h"

#include "leader.h"
#include "keycodes.h"
#include "color.h"
#include "rgb_matrix.h"
#include "send_string.h"

#define SEND_STRING_DELAY_VALUE 5


static int rgb_matrix_previous_mode;
static int rgb_matrix_previous_speed;
static HSV rgb_matrix_previous_hsv;

extern uint16_t leader_time;

void leader_start_user(void) {
    rgb_matrix_previous_mode = rgb_matrix_get_mode();
    rgb_matrix_previous_speed = rgb_matrix_get_speed();
    rgb_matrix_previous_hsv = rgb_matrix_get_hsv();


    rgb_matrix_mode_noeeprom(RGB_MATRIX_BREATHING );
    rgb_matrix_set_speed_noeeprom(255);
    rgb_matrix_sethsv_noeeprom(0,255,255);
}

void leader_end_user(void) {

    if (leader_sequence_two_keys(KC_U, KC_G)) {
        SEND_STRING_DELAY(LM_UG,SEND_STRING_DELAY_VALUE);
    }else
    if (leader_sequence_two_keys(KC_U, KC_B)) {
        SEND_STRING_DELAY(LM_UB,SEND_STRING_DELAY_VALUE);
    }else
    if (leader_sequence_two_keys(KC_U, KC_C)) {
        SEND_STRING_DELAY(LM_UC,SEND_STRING_DELAY_VALUE);
    }else

    if (leader_sequence_two_keys(KC_P, KC_7)) {
        SEND_STRING_DELAY(LM_P7,SEND_STRING_DELAY_VALUE);
    }else
    if (leader_sequence_two_keys(KC_P, KC_K)) {
        SEND_STRING_DELAY(LM_PK,SEND_STRING_DELAY_VALUE);
    }else
    if (leader_sequence_two_keys(KC_E, KC_G)) {
        SEND_STRING_DELAY(LM_EG,SEND_STRING_DELAY_VALUE);
    }else
    if (leader_sequence_two_keys(KC_E, KC_V)) {
        SEND_STRING_DELAY(LM_EV,SEND_STRING_DELAY_VALUE);
    }else
    if (leader_sequence_two_keys(KC_E, KC_B)) {
        SEND_STRING_DELAY(LM_EB,SEND_STRING_DELAY_VALUE);
    }
    rgb_matrix_mode_noeeprom (rgb_matrix_previous_mode);
    rgb_matrix_sethsv_noeeprom (rgb_matrix_previous_hsv.h,rgb_matrix_previous_hsv.s,rgb_matrix_previous_hsv.v);
    rgb_matrix_set_speed_noeeprom(rgb_matrix_previous_speed);
}
