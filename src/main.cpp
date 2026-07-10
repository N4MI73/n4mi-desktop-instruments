// N4MI Desktop Instrument Series - Propagation Monitor
// main.cpp -- Phase 2: real Overview screen against mock data
//
// USB-CDC note: uses while(!Serial) to wait for monitor connection
// before printing anything -- required for ESP32-S3 native USB-CDC
// which must re-enumerate after every reset.
//
// Encoder mapping for this phase (only Overview screen exists so far):
//   ROTATE_CW/CCW -- step through mock data scenarios (acceptance
//                    testing aid, not the final Phase 3+ behavior)
//   SHORT_PRESS    -- force refresh (mock: redraws current scenario)
//   LONG_PRESS     -- placeholder message; config menu not built yet

#include <Arduino.h>
#include "config.h"
#include "display_driver.h"
#include "encoder.h"
#include "data_client.h"
#include "screens/screen_overview.h"

static PropMonData current_data;
static uint8_t scenario_index = 0;
static bool showing_placeholder = false;
static unsigned long placeholder_started_ms = 0;

static void refresh_and_draw() {
    data_client_get_mock(current_data, scenario_index);
    screen_overview_draw(current_data);
}

void setup() {
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && (millis() - start) < 3000) {
        delay(10);
    }
    delay(200);
    Serial.println();
    Serial.println("=== N4MI Propagation Monitor -- Phase 2 ===");
    Serial.println("Overview screen against mock data");

    encoder_init();
    Serial.println("[setup] encoder_init() done");

    if (display_init()) {
        Serial.println("[setup] display_init() OK");
        display_show_boot_message("N4MI PropMon", "Phase 2 - Overview");
        delay(1500);
        refresh_and_draw();
    } else {
        Serial.println("[setup] display_init() FAILED");
    }
}

void loop() {
    EncoderEvent ev = encoder_poll();

    switch (ev) {
        case EncoderEvent::ROTATE_CW:
            scenario_index = (scenario_index + 1) % data_client_mock_scenario_count();
            Serial.printf("[mock] scenario -> %d\n", scenario_index);
            refresh_and_draw();
            break;
        case EncoderEvent::ROTATE_CCW:
            scenario_index = (scenario_index + data_client_mock_scenario_count() - 1) % data_client_mock_scenario_count();
            Serial.printf("[mock] scenario -> %d\n", scenario_index);
            refresh_and_draw();
            break;
        case EncoderEvent::SHORT_PRESS:
            Serial.println("[encoder] SHORT PRESS -- force refresh");
            refresh_and_draw();
            break;
        case EncoderEvent::LONG_PRESS:
            Serial.println("[encoder] LONG PRESS -- config menu (not built yet)");
            display_show_boot_message("Config menu", "not built yet");
            placeholder_started_ms = millis();
            showing_placeholder = true;
            break;
        default:
            break;
    }

    if (showing_placeholder && (millis() - placeholder_started_ms) >= 1500) {
        showing_placeholder = false;
        refresh_and_draw();
    }

    delay(2);
}
