// N4MI Desktop Instrument Series - Propagation Monitor
// wifi_client.h -- Wi-Fi connection management + credential storage
//
// UPDATED (Wi-Fi setup foundation): added NVS-backed credential
// storage (via Preferences) as the real long-term credential source,
// replacing wifi_credentials.h as the primary path. wifi_client_connect()
// tries stored NVS credentials first; falls back to wifi_credentials.h
// (seeding NVS on success) if nothing is stored yet.
//
// UPDATED (real captive portal): added wifi_client_try_credentials(),
// exposed for the setup portal's own connection-validation flow.
// IMPORTANT: this function deliberately does NOT touch WiFi.mode() --
// see wifi_client.cpp's header comment for why. The portal (see
// wifi_portal.cpp) must already be in WIFI_AP_STA mode before calling
// this, or its own AP will drop out from under the phone mid-request.

#pragma once

#include <Arduino.h>

// Attempts to connect to Wi-Fi. Tries stored NVS credentials first;
// falls back to wifi_credentials.h's hardcoded values (and seeds NVS
// with them on success) if nothing is stored yet. Sets WiFi.mode() to
// pure WIFI_STA -- only call this from the normal (non-setup-portal)
// boot/reconnect path. Blocks up to WIFI_CONNECT_TIMEOUT_MS (config.h).
// Returns true if connected by the time it returns, false on timeout.
bool wifi_client_connect();

// Returns true if currently associated to Wi-Fi.
bool wifi_client_is_connected();

// Returns true if valid Wi-Fi credentials are currently stored in NVS
// (independent of the hardcoded wifi_credentials.h fallback).
bool wifi_client_has_stored_credentials();

// Attempts a connection using only stored NVS credentials -- does not
// fall back to wifi_credentials.h. Sets WiFi.mode() to pure WIFI_STA,
// same caveat as wifi_client_connect(). Returns false immediately (no
// connection attempt) if nothing is stored.
bool wifi_client_connect_stored();

// Saves credentials to NVS, overwriting any previous values. Does not
// attempt a connection itself.
void wifi_client_save_credentials(const char *ssid, const char *password);

// Attempts a connection using the given credentials, blocking up to
// WIFI_CONNECT_TIMEOUT_MS. Does NOT save to NVS (caller decides,
// typically only on success) and does NOT touch WiFi.mode() -- the
// caller is responsible for setting the correct mode first. This is
// what lets the setup portal validate a submitted password while
// keeping its own AP interface alive throughout the attempt, so the
// phone's connection to the setup network doesn't drop mid-request.
bool wifi_client_try_credentials(const char *ssid, const char *password);
