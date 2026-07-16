// N4MI Desktop Instrument Series - Propagation Monitor
// wifi_client.h -- Wi-Fi connection management + credential storage
//
// UPDATED (Wi-Fi setup foundation): added NVS-backed credential
// storage (via Preferences) as the real long-term credential source,
// replacing wifi_credentials.h as the primary path. wifi_client_connect()
// now tries stored NVS credentials first; if none exist yet, it falls
// back to the hardcoded wifi_credentials.h values *and seeds NVS with
// them* on success -- a one-time migration, not a permanent reliance
// on the hardcoded file. Once the real captive portal exists (next
// session), submitted credentials go straight into NVS via
// wifi_client_save_credentials(), and the hardcoded fallback becomes
// unnecessary for anyone who's been through setup at least once.

#pragma once

#include <Arduino.h>

// Attempts to connect to Wi-Fi. Tries stored NVS credentials first;
// falls back to wifi_credentials.h's hardcoded values (and seeds NVS
// with them on success) if nothing is stored yet. Blocks up to
// WIFI_CONNECT_TIMEOUT_MS (config.h) waiting for association. Returns
// true if connected by the time it returns, false on timeout. Safe to
// call again later to retry -- returns immediately (true) if already
// connected.
bool wifi_client_connect();

// Returns true if currently associated to Wi-Fi.
bool wifi_client_is_connected();

// Returns true if valid Wi-Fi credentials are currently stored in NVS
// (independent of the hardcoded wifi_credentials.h fallback).
bool wifi_client_has_stored_credentials();

// Attempts a connection using only stored NVS credentials -- does not
// fall back to wifi_credentials.h. Returns false immediately (no
// connection attempt) if nothing is stored. Same blocking/timeout
// behavior as wifi_client_connect().
bool wifi_client_connect_stored();

// Saves credentials to NVS, overwriting any previous values. Does not
// attempt a connection itself -- call wifi_client_connect_stored()
// (or wifi_client_connect()) afterward if a connection is wanted.
void wifi_client_save_credentials(const char *ssid, const char *password);
