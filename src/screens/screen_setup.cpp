// N4MI Desktop Instrument Series - Propagation Monitor
// screen_setup.cpp
//
// Matches the SVG mockup Dan approved, now wired to real status from
// wifi_portal.h instead of the "Portal not yet built" placeholder from
// the foundation-only pass.

#include "screens/screen_setup.h"
#include "display_driver.h"
#include "ui_common.h"
#include "wifi_portal.h"
#include "config.h"

static const char *status_text(uint16_t &out_color) {
    switch (wifi_portal_get_status()) {
        case WifiPortalStatus::SCANNING:
            out_color = UI_COLOR_FAIR;
            return "Scanning for networks...";
        case WifiPortalStatus::WAITING:
            out_color = UI_COLOR_GOOD;
            if (wifi_portal_client_count() > 0) {
                return "Phone connected";
            }
            return "Waiting for connection...";
        case WifiPortalStatus::CONNECTING:
            out_color = UI_COLOR_FAIR;
            return "Connecting...";
        case WifiPortalStatus::CONNECTED:
            out_color = UI_COLOR_GOOD;
            return "Connected!";
        case WifiPortalStatus::FAILED:
            out_color = UI_COLOR_POOR;
            return "Failed - retry from phone";
    }
    out_color = UI_COLOR_MUTED;
    return "";
}

void screen_setup_draw() {
    if (!gfx) return;
    display_clear();

    ui_draw_centered_text_bold("WI-FI SETUP", 66, 3, UI_COLOR_GOOD);
    gfx->drawLine(120, 80, 270, 80, UI_COLOR_DIVIDER);

    ui_draw_centered_text("1. CONNECT PHONE TO:", 112, 2, UI_COLOR_LABEL);
    ui_draw_centered_text_bold(WIFI_SETUP_AP_NAME, 140, 2, UI_COLOR_VALUE);

    ui_draw_centered_text("2. OPEN A BROWSER TO:", 172, 2, UI_COLOR_LABEL);
    ui_draw_centered_text_bold(WIFI_SETUP_AP_IP, 198, 2, UI_COLOR_VALUE);

    uint16_t status_color;
    const char *text = status_text(status_color);
    ui_draw_centered_text("STATUS", 240, 2, UI_COLOR_LABEL);
    ui_draw_centered_text_bold(text, 260, 2, status_color);

    ui_draw_centered_text("ROTATE TO EXIT", 358, 2, UI_COLOR_MUTED);
}
