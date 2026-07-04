// N4MI Desktop Instrument Series - Propagation Monitor
// encoder.cpp
//
// FIXED 2026-07-xx: rotation was previously decoded by polling A/B
// pins once per loop() iteration. Fast knob turns were silently lost
// whenever loop() was blocked elsewhere -- most notably during
// display_clear()'s full-frame QSPI fillScreen() call on every
// rotation-triggered redraw. Confirmed by testing: slow, deliberate
// turns always registered correctly; normal-speed turns sometimes
// required several physical clicks to register one count.
//
// Fix: quadrature decoding now runs in a GPIO change interrupt, which
// fires the instant a pin transitions regardless of what loop() is
// doing. loop() (via encoder_poll()) just drains a shared counter
// whenever it gets a chance to run. This decouples "did we count the
// turn" from "how busy is everything else right now" -- important
// since Phase 2's real screens will have more to draw, not less.
//
// The transition table itself (which combinations of previous+current
// 2-bit state count as a valid CW/CCW step) is unchanged from the
// polled version -- it was already correct and bounce-resistant.

#include "encoder.h"
#include "config.h"

// Shared between the ISR and loop()/encoder_poll() -- must only be
// touched inside a critical section, since an ISR can fire on either
// CPU core while encoder_poll() is running on the other.
static portMUX_TYPE encoder_mux = portMUX_INITIALIZER_UNLOCKED;
static volatile uint8_t last_encoded = 0;
static volatile int32_t encoder_delta_accum = 0;

static bool button_was_pressed = false;
static unsigned long button_press_started_ms = 0;
static bool long_press_already_fired = false;

// IRAM_ATTR: keeps this handler resident in internal RAM rather than
// flash. Not strictly required yet, but becomes mandatory once Phase
// 2/3 start writing to NVS (Wi-Fi credentials, PropMon URL) --
// ESP32 briefly makes flash-resident code unreachable during flash
// writes, and an interrupt firing during that window would otherwise
// crash the device. Cheap to add now, would be a confusing bug to
// chase down later if skipped.
static void IRAM_ATTR encoder_isr() {
    uint8_t a = digitalRead(PIN_ENCODER_A);
    uint8_t b = digitalRead(PIN_ENCODER_B);
    uint8_t encoded = (a << 1) | b;
    uint8_t sum = (last_encoded << 2) | encoded;

    int8_t step = 0;
    switch (sum) {
        case 0b0001: case 0b0111: case 0b1110: case 0b1000: step =  1; break;
        case 0b0010: case 0b1011: case 0b1101: case 0b0100: step = -1; break;
        default: step = 0; break;
    }
    last_encoded = encoded;

    if (step != 0) {
        portENTER_CRITICAL_ISR(&encoder_mux);
        encoder_delta_accum += step;
        portEXIT_CRITICAL_ISR(&encoder_mux);
    }
}

void encoder_init() {
    pinMode(PIN_ENCODER_A, INPUT_PULLUP);
    pinMode(PIN_ENCODER_B, INPUT_PULLUP);
    pinMode(PIN_ENCODER_KEY, INPUT_PULLUP);

    uint8_t a = digitalRead(PIN_ENCODER_A);
    uint8_t b = digitalRead(PIN_ENCODER_B);
    last_encoded = (a << 1) | b;

    // GPIO1/GPIO2 (PIN_ENCODER_A/B) have no interrupt restrictions on
    // the ESP32-S3 -- unlike the input-only pins on some older ESP32
    // variants. Same handler is attached to both pins since the ISR
    // always re-reads both regardless of which one changed.
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_A), encoder_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_B), encoder_isr, CHANGE);
}

EncoderEvent encoder_poll() {
    // Threshold of 2 (not 4) confirmed correct via real hardware
    // testing 2026-07-04 -- this encoder produces 2 quadrature
    // transitions per detent, not the 4 initially assumed.
    //
    // Subtracting 2 (rather than zeroing) on trigger preserves any
    // extra accumulated steps from a fast multi-detent turn, so a
    // burst of rotation doesn't lose partial progress toward the next
    // count -- it just gets applied over a couple of extra
    // encoder_poll() calls instead of being dropped.
    portENTER_CRITICAL(&encoder_mux);
    int32_t accum = encoder_delta_accum;
    portEXIT_CRITICAL(&encoder_mux);

    if (accum >= 2) {
        portENTER_CRITICAL(&encoder_mux);
        encoder_delta_accum -= 2;
        portEXIT_CRITICAL(&encoder_mux);
        return EncoderEvent::ROTATE_CW;
    }
    if (accum <= -2) {
        portENTER_CRITICAL(&encoder_mux);
        encoder_delta_accum += 2;
        portEXIT_CRITICAL(&encoder_mux);
        return EncoderEvent::ROTATE_CCW;
    }

    // Button press/release detection unchanged -- still polled, since
    // it doesn't need interrupt-level precision the way rotation did.
    bool is_pressed = (digitalRead(PIN_ENCODER_KEY) == LOW);
    if (is_pressed && !button_was_pressed) {
        button_was_pressed = true;
        button_press_started_ms = millis();
        long_press_already_fired = false;
    } else if (is_pressed && button_was_pressed) {
        if (!long_press_already_fired &&
            (millis() - button_press_started_ms) >= LONG_PRESS_MS) {
            long_press_already_fired = true;
            return EncoderEvent::LONG_PRESS;
        }
    } else if (!is_pressed && button_was_pressed) {
        button_was_pressed = false;
        if (!long_press_already_fired &&
            (millis() - button_press_started_ms) < LONG_PRESS_MS) {
            return EncoderEvent::SHORT_PRESS;
        }
    }
    return EncoderEvent::NONE;
}