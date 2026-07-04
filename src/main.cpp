// N4MI Desktop Instrument Series - Propagation Monitor
// main.cpp -- Phase 1: display init + rotary encoder test
//
// USB-CDC note: uses while(!Serial) to wait for monitor connection
// before printing anything -- required for ESP32-S3 native USB-CDC
// which must re-enumerate after every reset. Without this, all Serial
// output during setup() is lost. Confirmed working 2026-07-04.

#include <Arduino.h>
#include "config.h"
#include "display_driver.h"
#include "encoder.h"

static int32_t test_counter = 0;
static unsigned long last_event_display_ms = 0;
static bool showing_press_message = false;

static void redraw_counter_screen() {
    display_clear();
    gfx->setTextColor(RGB565_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(40, 140);
    gfx->println("PHASE 1 TEST");
    gfx->setTextSize(4);
    gfx->setCursor(140, 190);
    gfx->println(test_counter);
    gfx->setTextSize(1);
    gfx->setCursor(40, 320);
    gfx->println("Turn knob: count");
    gfx->setCursor(40, 335);
    gfx->println("Short press: test msg");
    gfx->setCursor(40, 350);
    gfx->println("Long press: test msg");
}

void setup() {
    Serial.begin(115200);
    // Wait up to 3 seconds for a Serial monitor to connect -- long
    // enough to catch boot output during active development (with the
    // monitor already open before reset, per our normal workflow), but
    // bounded so the device still boots and runs normally when powered
    // standalone with no computer attached at all (the real deployed
    // use case -- a desk instrument plugged into a wall adapter).
    unsigned long start = millis();
    while (!Serial && (millis() - start) < 3000) {
        delay(10);
    }
    delay(200);
    Serial.println();
    Serial.println("=== N4MI Propagation Monitor -- Phase 1 ===");
    Serial.println("Display init + encoder test");

    encoder_init();
    Serial.println("[setup] encoder_init() done");

    if (display_init()) {
        Serial.println("[setup] display_init() OK");
        display_show_boot_message("N4MI PropMon", "Phase 1 - Encoder Test");
        delay(1500);
        redraw_counter_screen();
    } else {
        Serial.println("[setup] display_init() FAILED");
    }
}

void loop() {
    EncoderEvent ev = encoder_poll();

    switch (ev) {
        case EncoderEvent::ROTATE_CW:
            test_counter++;
            Serial.printf("[encoder] CW  -> counter=%ld\n", test_counter);
            if (gfx) redraw_counter_screen();
            break;
        case EncoderEvent::ROTATE_CCW:
            test_counter--;
            Serial.printf("[encoder] CCW -> counter=%ld\n", test_counter);
            if (gfx) redraw_counter_screen();
            break;
        case EncoderEvent::SHORT_PRESS:
            Serial.println("[encoder] SHORT PRESS");
            if (gfx) {
                display_show_boot_message("SHORT PRESS", "detected");
                last_event_display_ms = millis();
                showing_press_message = true;
            }
            break;
        case EncoderEvent::LONG_PRESS:
            Serial.println("[encoder] LONG PRESS");
            if (gfx) {
                display_show_boot_message("LONG PRESS", "detected");
                last_event_display_ms = millis();
                showing_press_message = true;
            }
            break;
        default:
            break;
    }

    if (showing_press_message && gfx &&
        (millis() - last_event_display_ms) >= 1500) {
        showing_press_message = false;
        redraw_counter_screen();
    }

    delay(2);
}
