// N4MI Desktop Instrument Series - Propagation Monitor
// wifi_client.cpp

#include "wifi_client.h"
#include "wifi_credentials.h"
#include "config.h"
#include <WiFi.h>
#include <Preferences.h>

// See main.cpp for the full explanation -- set to 1 to compile in
// diagnostic Serial output, 0 (default) to compile it out entirely.
#define DEBUG_VERBOSE 0

// NVS namespace for stored Wi-Fi credentials. A single Preferences
// instance, opened/closed per call rather than held open -- these
// calls are infrequent (boot, and eventually setup-portal submission),
// so there's no real cost to the extra open/close pairs, and it avoids
// ever leaving an NVS handle open across unrelated code.
static const char *NVS_NAMESPACE = "wifi";

static bool connect_with_credentials(const char *ssid, const char *password) {
    if (WiFi.status() == WL_CONNECTED) return true;

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

#if DEBUG_VERBOSE
    if (Serial) Serial.printf("[wifi] connecting to \"%s\"...\n", ssid);
#endif

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < WIFI_CONNECT_TIMEOUT_MS) {
        delay(250);
    }

    bool connected = (WiFi.status() == WL_CONNECTED);
#if DEBUG_VERBOSE
    if (Serial) {
        if (connected) {
            Serial.printf("[wifi] connected, IP=%s\n", WiFi.localIP().toString().c_str());
        } else {
            Serial.printf("[wifi] connect FAILED after %lums, status=%d\n",
                (unsigned long)(millis() - start), (int)WiFi.status());
        }
    }
#endif
    return connected;
}

bool wifi_client_has_stored_credentials() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true); // read-only
    bool has = prefs.isKey("ssid");
    prefs.end();
    return has;
}

void wifi_client_save_credentials(const char *ssid, const char *password) {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, false); // read-write
    prefs.putString("ssid", ssid);
    prefs.putString("pass", password);
    prefs.end();
#if DEBUG_VERBOSE
    if (Serial) Serial.printf("[wifi] credentials saved to NVS for \"%s\"\n", ssid);
#endif
}

bool wifi_client_connect_stored() {
    if (!wifi_client_has_stored_credentials()) return false;

    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);
    String ssid = prefs.getString("ssid", "");
    String pass = prefs.getString("pass", "");
    prefs.end();

    if (ssid.length() == 0) return false;
    return connect_with_credentials(ssid.c_str(), pass.c_str());
}

bool wifi_client_connect() {
    if (WiFi.status() == WL_CONNECTED) return true;

    if (wifi_client_has_stored_credentials()) {
#if DEBUG_VERBOSE
        if (Serial) Serial.println("[wifi] using stored NVS credentials");
#endif
        return wifi_client_connect_stored();
    }

    // No stored credentials yet -- fall back to the temporary hardcoded
    // values (wifi_credentials.h) and seed NVS with them on success, so
    // future boots use the same stored-credentials path the real
    // captive portal will eventually populate directly. One-time
    // migration, not a permanent reliance on the hardcoded file.
#if DEBUG_VERBOSE
    if (Serial) Serial.println("[wifi] no stored credentials, falling back to wifi_credentials.h");
#endif
    bool connected = connect_with_credentials(WIFI_SSID, WIFI_PASSWORD);
    if (connected) {
        wifi_client_save_credentials(WIFI_SSID, WIFI_PASSWORD);
    }
    return connected;
}

bool wifi_client_is_connected() {
    return WiFi.status() == WL_CONNECTED;
}
