// N4MI Desktop Instrument Series - Propagation Monitor
// screen_setup.cpp
//
// Matches the SVG mockup Dan approved. AP name and IP are compile-time
// constants for now, matching where the real portal will actually run
// once built. Status line is an honest placeholder -- see screen_setup.h.

#include "screens/screen_setup.h"
#include "display_driver.h"
#include "ui_common.h"

// Matches the AP name/IP the real portal will use once built --
// centralized here rather than in config.h since nothing else needs
// them yet.
#define WIFI_SETUP_AP_NAME "N4MI-PropMon-Setup"
#define WIFI_SETUP_AP_IP   "192.168.4.1"

void screen_setup_draw() {
    if (!gfx) return;
    display_clear();

    ui_draw_centered_text_bold("WI-FI SETUP", 66, 3, UI_COLOR_GOOD);
    gfx->drawLine(120, 80, 270, 80, UI_COLOR_DIVIDER);

    ui_draw_centered_text("1. CONNECT PHONE TO:", 112, 2, UI_COLOR_LABEL);
    ui_draw_centered_text_bold(WIFI_SETUP_AP_NAME, 140, 2, UI_COLOR_VALUE);

    ui_draw_centered_text("2. OPEN A BROWSER TO:", 172, 2, UI_COLOR_LABEL);
    ui_draw_centered_text_bold(WIFI_SETUP_AP_IP, 198, 2, UI_COLOR_VALUE);

    ui_draw_centered_text("STATUS", 240, 2, UI_COLOR_LABEL);
    ui_draw_centered_text_bold("Portal not yet built", 260, 2, UI_COLOR_FAIR);

    ui_draw_centered_text("ROTATE TO EXIT", 358, 2, UI_COLOR_MUTED);
}
