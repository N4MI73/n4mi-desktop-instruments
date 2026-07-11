// N4MI Desktop Instrument Series - Propagation Monitor
// data_client.cpp -- mock data source for Phase 2
//
// No HTTP client, no Wi-Fi -- these are hardcoded scenarios standing
// in for what PropMon's live endpoint will eventually return. See
// data_client.h for the Phase 3 migration note.

#include "data_client.h"

#define NUM_MOCK_SCENARIOS 4

// Scenario 0: mirrors the real sample captured in the project brief --
// mixed conditions, GOOD is the best tier present.
static BandReading scenario0_bands[PROPMON_BAND_COUNT] = {
    {"160m", BAND_GOOD}, {"80m", BAND_FAIR}, {"40m", BAND_GOOD},
    {"30m",  BAND_GOOD}, {"20m", BAND_FAIR}, {"17m", BAND_FAIR},
    {"15m",  BAND_POOR}, {"12m", BAND_POOR}, {"10m", BAND_POOR},
    {"6m",   BAND_POOR},
};

// Scenario 1: geomagnetic storm -- mostly poor, tower on CAUTION.
// Exercises the tower-status color mapping and a POOR-dominant tier.
static BandReading scenario1_bands[PROPMON_BAND_COUNT] = {
    {"160m", BAND_FAIR}, {"80m", BAND_POOR}, {"40m", BAND_FAIR},
    {"30m",  BAND_POOR}, {"20m", BAND_POOR}, {"17m", BAND_POOR},
    {"15m",  BAND_POOR}, {"12m", BAND_POOR}, {"10m", BAND_POOR},
    {"6m",   BAND_POOR},
};

// Scenario 2: excellent day -- every band GOOD, no secondary tier at
// all. Exercises the "no secondary line" edge case.
static BandReading scenario2_bands[PROPMON_BAND_COUNT] = {
    {"160m", BAND_GOOD}, {"80m", BAND_GOOD}, {"40m", BAND_GOOD},
    {"30m",  BAND_GOOD}, {"20m", BAND_GOOD}, {"17m", BAND_GOOD},
    {"15m",  BAND_GOOD}, {"12m", BAND_GOOD}, {"10m", BAND_GOOD},
    {"6m",   BAND_GOOD},
};

// Scenario 3: dead band day -- everything POOR. Headline itself
// should render in the POOR color, not just a secondary line.
static BandReading scenario3_bands[PROPMON_BAND_COUNT] = {
    {"160m", BAND_POOR}, {"80m", BAND_POOR}, {"40m", BAND_POOR},
    {"30m",  BAND_POOR}, {"20m", BAND_POOR}, {"17m", BAND_POOR},
    {"15m",  BAND_POOR}, {"12m", BAND_POOR}, {"10m", BAND_POOR},
    {"6m",   BAND_POOR},
};

// Alert sets, one per scenario. Scenario 0 and 2 use PropMon's real
// steady-state shape (a single NONE-level placeholder entry, per the
// live JSON sample captured in the project brief -- not an empty
// array). Scenario 1 has two simultaneous alerts of different
// categories and severities, to exercise "pick the worst one" logic
// and the "+N more" indicator together. Scenario 3 is the only mock
// case reaching CRITICAL -- nothing else in this file has tested that
// tier yet.
static AlertEntry scenario0_alerts[1] = {
    {ALERT_CAT_TOWER, ALERT_NONE, "No active alerts"},
};

static AlertEntry scenario1_alerts[2] = {
    {ALERT_CAT_TOWER, ALERT_CAUTION, "Lightning detected within 10mi"},
    {ALERT_CAT_PROPAGATION, ALERT_WARNING, "Geomagnetic storm in progress (Kp=6)"},
};

static AlertEntry scenario2_alerts[1] = {
    {ALERT_CAT_TOWER, ALERT_NONE, "No active alerts"},
};

static AlertEntry scenario3_alerts[1] = {
    {ALERT_CAT_TOWER, ALERT_CRITICAL, "Wind gust 62mph - secure equipment"},
};

void data_client_get_mock(PropMonData &out, uint8_t scenario_index) {
    scenario_index %= NUM_MOCK_SCENARIOS;

    out.last_updated_ms = millis();

    switch (scenario_index) {
        case 0:
            out.sfi = 163; out.a_index = 12; out.k_index = 3; out.sunspots = 99;
            out.solar_wind = 654.6;
            strncpy(out.xray, "C1.3", sizeof(out.xray));
            strncpy(out.tower_status, "NONE", sizeof(out.tower_status));
            memcpy(out.bands, scenario0_bands, sizeof(scenario0_bands));
            memcpy(out.alerts, scenario0_alerts, sizeof(scenario0_alerts));
            out.alert_count = 1;
            break;
        case 1:
            // Geomagnetic storm scenario -- also given a moderate (M-class)
            // flare and elevated wind speed so this is the one mock case
            // exercising the "severe" tier on both K-index and X-ray at
            // once, alongside the tower CAUTION status. Two simultaneous
            // alerts here too (tower CAUTION + propagation WARNING) --
            // the worst-alert-wins headline logic should pick WARNING.
            out.sfi = 98; out.a_index = 28; out.k_index = 6; out.sunspots = 41;
            out.solar_wind = 780.2;
            strncpy(out.xray, "M5.2", sizeof(out.xray));
            strncpy(out.tower_status, "CAUTION", sizeof(out.tower_status));
            memcpy(out.bands, scenario1_bands, sizeof(scenario1_bands));
            memcpy(out.alerts, scenario1_alerts, sizeof(scenario1_alerts));
            out.alert_count = 2;
            break;
        case 2:
            out.sfi = 210; out.a_index = 4; out.k_index = 1; out.sunspots = 187;
            out.solar_wind = 380.0;
            strncpy(out.xray, "A0.5", sizeof(out.xray));
            strncpy(out.tower_status, "NONE", sizeof(out.tower_status));
            memcpy(out.bands, scenario2_bands, sizeof(scenario2_bands));
            memcpy(out.alerts, scenario2_alerts, sizeof(scenario2_alerts));
            out.alert_count = 1;
            break;
        case 3:
            // K-index here is GOOD (2) despite the dead-band day -- low
            // SFI, not geomagnetic activity, is why the bands are poor.
            // X-ray given a middling C-class so this scenario also tests
            // K and X-ray landing in *different* tiers from each other.
            // Tower alert is CRITICAL -- the only mock scenario reaching
            // this severity.
            out.sfi = 68; out.a_index = 6; out.k_index = 2; out.sunspots = 12;
            out.solar_wind = 410.0;
            strncpy(out.xray, "C4.5", sizeof(out.xray));
            strncpy(out.tower_status, "CRITICAL", sizeof(out.tower_status));
            memcpy(out.bands, scenario3_bands, sizeof(scenario3_bands));
            memcpy(out.alerts, scenario3_alerts, sizeof(scenario3_alerts));
            out.alert_count = 1;
            break;
    }
}

uint8_t data_client_mock_scenario_count() {
    return NUM_MOCK_SCENARIOS;
}
