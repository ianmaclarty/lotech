/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltinput)

enum LTKey {
    LT_KEY_UNKNOWN,
    LT_KEY_0,
    LT_KEY_1,
    LT_KEY_2,
    LT_KEY_3,
    LT_KEY_4,
    LT_KEY_5,
    LT_KEY_6,
    LT_KEY_7,
    LT_KEY_8,
    LT_KEY_9,
    LT_KEY_A,
    LT_KEY_B,
    LT_KEY_C,
    LT_KEY_D,
    LT_KEY_E,
    LT_KEY_F,
    LT_KEY_G,
    LT_KEY_H,
    LT_KEY_I,
    LT_KEY_J,
    LT_KEY_K,
    LT_KEY_L,
    LT_KEY_M,
    LT_KEY_N,
    LT_KEY_O,
    LT_KEY_P,
    LT_KEY_Q,
    LT_KEY_R,
    LT_KEY_S,
    LT_KEY_T,
    LT_KEY_U,
    LT_KEY_V,
    LT_KEY_W,
    LT_KEY_X,
    LT_KEY_Y,
    LT_KEY_Z,
    LT_KEY_SPACE,
    LT_KEY_TAB,
    LT_KEY_ENTER,
    LT_KEY_UP,
    LT_KEY_DOWN,
    LT_KEY_LEFT,
    LT_KEY_RIGHT,
    LT_KEY_RIGHT_BRACKET,
    LT_KEY_LEFT_BRACKET,
    LT_KEY_BACKSLASH,
    LT_KEY_SEMI_COLON,
    LT_KEY_APOS,
    LT_KEY_COMMA,
    LT_KEY_PERIOD,
    LT_KEY_SLASH,
    LT_KEY_PLUS,
    LT_KEY_MINUS,
    LT_KEY_TICK,
    LT_KEY_DEL,
    LT_KEY_ESC,

    LT_KEY_BACK, // Android back button
};

enum LTGamePadButton {
    LT_GAMEPAD_UP = 0,
    LT_GAMEPAD_DOWN,
    LT_GAMEPAD_LEFT,
    LT_GAMEPAD_RIGHT,
    LT_GAMEPAD_START,
    LT_GAMEPAD_BACK,
    LT_GAMEPAD_LSTICK_BUTTON,
    LT_GAMEPAD_RSTICK_BUTTON,
    LT_GAMEPAD_LB,
    LT_GAMEPAD_RB,
    LT_GAMEPAD_HOME,
    LT_GAMEPAD_A,
    LT_GAMEPAD_B,
    LT_GAMEPAD_X,
    LT_GAMEPAD_Y,
    LT_GAMEPAD_NUM_BUTTONS
};

enum LTGamePadAxis {
    LT_GAMEPAD_LSTICK_X = 0,
    LT_GAMEPAD_LSTICK_Y,
    LT_GAMEPAD_RSTICK_X,
    LT_GAMEPAD_RSTICK_Y,
    LT_GAMEPAD_LT,
    LT_GAMEPAD_RT,
    LT_GAMEPAD_NUM_AXES
};

#define LT_MAX_GAMEPADS 4

extern LTbool lt_gamepad_button_state[LT_MAX_GAMEPADS][LT_GAMEPAD_NUM_BUTTONS];
