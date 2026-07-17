// N4MI Desktop Instrument Series - Propagation Monitor
// wifi_client.cpp
//
// UPDATED (real captive portal): connect_with_credentials() no longer
// sets WiFi.mode() itself -- moved that responsibility to each caller.
// Reason: the setup portal (wifi_portal.cpp) needs to validate a
// submitted password while its own AP stays alive (WIFI_AP_STA mode),
// so a phone connected to the setup network can actually receive the
// success/failure response. The original version forced WIFI_STA mode
// unconditionally, which would have silently dropped the AP -- and the
// phone's connection to it -- mid-request. wifi_client_connect() (the
// normal boot path, which never needs an AP) now sets WIFI_STA
// explicitly itself, right before delegating to the same shared
// low-level helper the portal also uses.

#include "wifi_client.h"
#include "wifi_credentials.h"
#include "config.h"
#include <WiFi.h>
#include <Preferences.h>

// See main.cpp for the full explanation -- set to 1 to compile in
// diagnostic Serial output, 0 (default) to compile it out entirely.
#define DEBUG_VERBOSE 0

static const char *NVS_NAMESPACE = "wifi";

// Deliberately does not touch WiFi.mode() -- see file header. Callers
// (wifi_client_connect(), wifi_client_connect_stored(), and the portal
// via wifi_client_try_credentials()) are each responsible for ensuring
// the correct mode is already set before calling this.
static bool connect_with_credentials(const char *ssid, const char *password) {
    if (WiFi.status() == WL_CONNECTED) return true;

    WiFi.begin(ssid, password);

#if DEBUG_VERBOSE
    if (Serial) Serial.printf("[wifi] connecting to \"%s\"...\n", ssid);
#endif

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < WIFI_CONNECT_TIMEOUT_MS) {
        delay(250);
    }

    bool connected = (WiFi.status() == WL_CONNECTED);

    if (!connected) {
        // UPDATED (found 2026-07-17): confirmed on real hardware that
        // leaving a failed attempt "dangling" here -- without an
        // explicit disconnect -- left the Wi-Fi driver stuck silently
        // retrying it in the background, which then blocked later
        // unrelated operations (specifically, every WiFi.scanNetworks()
        // call in the setup portal failed outright until this was
        // added). Explicitly aborting the attempt frees the radio for
        // whatever comes next.
        WiFi.disconnect();
    }

#if DEBUG_VERBOSE
    if (Serial) {
        if (connected) {
            Serial.printf("[wifi] connected, IP=%s\n", WiFi.localIP().toString().c_str());
        } else {
            Serial.printf("[wifi] connect FAILED after %lums, status=%d -- disconnected to free the radio\n",
                (unsigned long)(millis() - start), (int)WiFi.status());
        }
    }
#endif
    return connected;
}

bool wifi_client_try_credentials(const char *ssid, const char *password) {
    // UPDATED (bug found 2026-07-17): unlike connect_with_credentials()'s
    // other callers, this one must NOT benefit from the "already
    // connected, short-circuit true" check at the top of that function.
    // The whole point here is testing whether *these specific*
    // credentials work -- but if the device was already connected
    // (e.g. it boots using stored credentials, and setup mode doesn't
    // drop that existing connection when it starts the AP alongside
    // it), that check would report success immediately without ever
    // attempting WiFi.begin() with what was actually submitted. This
    // exact scenario was confirmed on real hardware: a wrong password
    // was accepted as "Connected!" because the device was still
    // connected to its original network from before setup mode began.
    // Explicitly disconnecting first forces a real, clean attempt
    // against the submitted credentials every time.
    WiFi.disconnect();
    delay(100); // brief settle time between disconnect and the new attempt
    return connect_with_credentials(ssid, password);
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

    // Normal boot/reconnect path -- never runs the setup portal's AP
    // at the same time, so pure STA mode is always correct here.
    WiFi.mode(WIFI_STA);

    if (wifi_client_has_stored_credentials()) {
#if DEBUG_VERBOSE
        if (Serial) Serial.println("[wifi] using stored NVS credentials");
#endif
        return wifi_client_connect_stored();
    }

    // No stored credentials yet -- fall back to the temporary hardcoded
    // values (wifi_credentials.h) and seed NVS with them on success.
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
