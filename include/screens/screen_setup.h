// N4MI Desktop Instrument Series - Propagation Monitor
// screen_setup.h
//
// SVG mockup approved by Dan before this file was written. Matches
// screen_config.h's own honesty precedent: the portal (AP mode, DNS
// redirect, web server) doesn't exist yet this session, so the status
// line says so plainly rather than showing "Waiting for connection..."
// like the mockup did -- that text becomes accurate once the real
// portal is built.
//
// Entered via a continued knob hold past KNOB_RESET_HOLD_MS (see
// encoder.h's RESET_HOLD event) -- "hold longer than the regular
// long-press to go further". Exited by knob rotation or another
// long-press, same convention as screen_config.h.

#pragma once

void screen_setup_draw();
