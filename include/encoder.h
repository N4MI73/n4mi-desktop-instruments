// N4MI Desktop Instrument Series - Propagation Monitor
// encoder.h -- rotary encoder rotation + knob button reading
//
// Rotation decoding is interrupt-driven (see encoder.cpp) so that
// quadrature transitions are never missed while loop() is blocked on
// something else (e.g. a display redraw). Button press/release is
// still polled from loop() via encoder_poll() -- press timing doesn't
// need interrupt precision the way rotation does.

#pragma once

#include <Arduino.h>

enum class EncoderEvent {
    NONE,
    ROTATE_CW,
    ROTATE_CCW,
    SHORT_PRESS,
    LONG_PRESS,
};

void encoder_init();
EncoderEvent encoder_poll();