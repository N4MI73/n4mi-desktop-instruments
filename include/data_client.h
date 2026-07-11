// N4MI Desktop Instrument Series - Propagation Monitor
// data_client.h -- PropMon data model + mock data source (Phase 2)
//
// Field names/types mirror PropMon's real JSON shape (see project brief)
// so that swapping this mock source for a live HTTP fetch in Phase 3 is
// a drop-in change -- no screen code should need to change, only how
// PropMonData gets populated.
//
// UPDATED 2026-07-11: xray and solar_wind added when the Solar screen
// needed them. alerts[] added now that the Alerts screen needs it --
// both were previously deferred per the project's "no premature
// abstraction" framework scope decision.

#pragma once

#include <Arduino.h>

#define PROPMON_BAND_COUNT 10
#define PROPMON_MAX_ALERTS 4

enum BandCondition : uint8_t {
    BAND_POOR = 0,
    BAND_FAIR = 1,
    BAND_GOOD = 2,
};

struct BandReading {
    const char *name;
    BandCondition condition;
};

// Mirrors PropMon's alert level strings exactly: "NONE", "CAUTION",
// "WARNING", "CRITICAL". A single NONE-level entry with a placeholder
// message (e.g. "No active alerts") is PropMon's normal steady-state
// response, per the real JSON sample in the project brief -- not an
// empty array.
enum AlertLevel : uint8_t {
    ALERT_NONE     = 0,
    ALERT_CAUTION  = 1,
    ALERT_WARNING  = 2,
    ALERT_CRITICAL = 3,
};

// Mirrors PropMon's alert category strings: "tower" or "propagation".
enum AlertCategory : uint8_t {
    ALERT_CAT_TOWER       = 0,
    ALERT_CAT_PROPAGATION = 1,
};

struct AlertEntry {
    AlertCategory category;
    AlertLevel level;
    char message[48];
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
    float solar_wind;

    // Mirrors PropMon's raw xray string exactly, e.g. "C1.3" -- the
    // Solar screen parses the leading letter (A/B/C/M/X) itself to
    // pick a severity color; PropMon doesn't pre-classify this field.
    char xray[8];

    // Mirrors PropMon's tower_status field exactly: "NONE", "CAUTION",
    // "WARNING", or "CRITICAL".
    char tower_status[16];

    BandReading bands[PROPMON_BAND_COUNT];

    AlertEntry alerts[PROPMON_MAX_ALERTS];
    uint8_t alert_count;
};

// Populates `out` with one of a small set of hardcoded mock scenarios,
// selected by scenario_index (wraps via data_client_mock_scenario_count()).
// Scenarios are deliberately varied -- mixed conditions, all-good,
// all-poor, and a tower alert -- to exercise the screen's tier-selection
// and color logic before any live data exists.
void data_client_get_mock(PropMonData &out, uint8_t scenario_index);

uint8_t data_client_mock_scenario_count();
