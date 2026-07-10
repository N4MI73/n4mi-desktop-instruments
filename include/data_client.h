// N4MI Desktop Instrument Series - Propagation Monitor
// data_client.h -- PropMon data model + mock data source (Phase 2)
//
// Field names/types mirror PropMon's real JSON shape (see project brief)
// so that swapping this mock source for a live HTTP fetch in Phase 3 is
// a drop-in change -- no screen code should need to change, only how
// PropMonData gets populated.
//
// Fields not yet used by any built screen (solar_wind, xray, alerts[])
// are intentionally left out for now -- added when the screens that
// need them (Solar, Alerts) get built, per the project's "no premature
// abstraction" framework scope decision.

#pragma once

#include <Arduino.h>

#define PROPMON_BAND_COUNT 10

enum BandCondition : uint8_t {
    BAND_POOR = 0,
    BAND_FAIR = 1,
    BAND_GOOD = 2,
};

struct BandReading {
    const char *name;
    BandCondition condition;
};

struct PropMonData {
    // millis() timestamp of when this data was considered "fresh" --
    // used to compute "Updated Xm ago" without needing NTP/Wi-Fi time
    // sync. Phase 3's real fetch sets this to millis() on every
    // successful parse.
    uint32_t last_updated_ms;

    int sfi;
    int a_index;
    int k_index;
    int sunspots;

    // Mirrors PropMon's tower_status field exactly: "NONE", "CAUTION",
    // "WARNING", or "CRITICAL".
    char tower_status[16];

    BandReading bands[PROPMON_BAND_COUNT];
};

// Populates `out` with one of a small set of hardcoded mock scenarios,
// selected by scenario_index (wraps via data_client_mock_scenario_count()).
// Scenarios are deliberately varied -- mixed conditions, all-good,
// all-poor, and a tower alert -- to exercise the screen's tier-selection
// and color logic before any live data exists.
void data_client_get_mock(PropMonData &out, uint8_t scenario_index);

uint8_t data_client_mock_scenario_count();
