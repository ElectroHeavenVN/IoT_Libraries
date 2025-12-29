#pragma once

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include "ESP8266HTTPClientMod.h"
#elif defined(ESP32)
#include <WiFi.h>
#include "HTTPClientMod.h"
#endif
#include <WiFiClientSecure.h>
#include "ZaloBotESPResponse.h"

// #define DEBUG_BOTCLIENT Serial

#ifdef DEBUG_BOTCLIENT
#define BC_DEBUG(...) DEBUG_BOTCLIENT.printf(__VA_ARGS__)
#define BC_DEBUG_LN(...) DEBUG_BOTCLIENT.println(__VA_ARGS__)
#else
#define BC_DEBUG(...) do { (void)0; } while (0)
#define BC_DEBUG_LN(...) do { (void)0; } while (0)
#endif

class ZaloBotClient
{
public:
    ZaloBotClient(const String &botToken);
    ZaloBotESPResponse SendMsg(const String &threadID, const String &content);
    ZaloBotESPResponse GetPollingUpdates();
    ZaloBotESPResponse GetMe();
    ZaloBotESPResponse SendPhoto(const String &threadID, const String &photoUrl, const String &caption);
    ZaloBotESPResponse SendSticker(const String &threadID, const String &stickerID);
    ZaloBotESPResponse SendChatAction(const String &threadID, const String &action);

private:
    ZaloBotESPResponse _httpGet(const String &endpoint);
    ZaloBotESPResponse _httpPost(const String &endpoint, const char *keys[], const char *values[], int count);
    void _httpGetAsync(const String &endpoint);
    void _cancelPolling();
    ZaloBotESPResponse _httpGetAsyncGetResponse();
    static String _buildFormBody(const char *keys[], const char *values[], int count);
    String _botToken;
    WiFiClientSecure _wifiClient;
    HTTPClientMod _httpClient;
    HTTPClientMod _asyncHttpClient;
    bool _poolingInProgress = false;
};