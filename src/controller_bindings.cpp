#include "globals.h"

// Definition of the controller bindings array (declared extern in globals.h)
ControllerBinding bindings[ACTION_COUNT] = {
    { false, GAMEPAD_BUTTON_RIGHT_FACE_DOWN, 0.0f, "A" },
    { false, GAMEPAD_BUTTON_LEFT_THUMB,      0.0f, "L3" },
    { false, GAMEPAD_BUTTON_RIGHT_FACE_UP,   0.0f, "Y" },
    { false, GAMEPAD_BUTTON_RIGHT_FACE_LEFT, 0.0f, "X" },
    { false, GAMEPAD_BUTTON_DPAD_RIGHT,      0.0f, "D-Right" },
    { false, GAMEPAD_BUTTON_LEFT_TRIGGER_1,  0.0f, "LB" },
    { false, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,0.0f, "B" },
    { false, GAMEPAD_BUTTON_RIGHT_TRIGGER_1, 0.0f, "RB" }
};