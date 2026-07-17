// N4MI Desktop Instrument Series - Propagation Monitor
// screen_setup.h
//
// UPDATED (real captive portal): now shows real, live status queried
// directly from wifi_portal.h -- same pattern screen_config.cpp
// already uses for WiFi.status()/WiFi.localIP() (query the relevant
// module directly rather than having main.cpp fetch and pass state
// in). No parameters needed as a result.
//
// Entered via a continued knob hold past KNOB_RESET_HOLD_MS (see
// encoder.h's RESET_HOLD event). Exited by knob rotation or another
// long-press -- see main.cpp for that state handling.

#pragma once

void screen_setup_draw();
