// N4MI Desktop Instrument Series - Propagation Monitor
// display_driver.h -- AMOLED display setup (CO5300, QSPI)
//
// CONFIRMED 2026-07-04: this hardware revision uses the Arduino_CO5300
// driver, not Arduino_SH8601 -- see display_driver.cpp for the full
// story. display_init() is called from main.cpp's setup().

#pragma once

#include <Arduino_GFX_Library.h>
#include "config.h"

extern Arduino_GFX *gfx;

bool display_init();
void display_clear();
void display_show_boot_message(const char *line1, const char *line2 = nullptr);