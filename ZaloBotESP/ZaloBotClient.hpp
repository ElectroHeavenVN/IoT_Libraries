#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include "ESP8266HTTPClientMod.h"
#elif defined(ESP32)
#include <WiFi.h>
#include "HTTPClientMod.h"
#endif
#include <optional>
#include "ZaloBotESPResponse.h"

// #define DEBUG_BOTCLIENT Serial

#ifdef DEBUG_BOTCLIENT
#define BC_DEBUG(...) DEBUG_BOTCLIENT.printf(__VA_ARGS__)
#else
#define BC_DEBUG(...) do { (void)0; } while (0)
#endif

class ZaloBotClient
{
public:
    ZaloBotClient(String botToken);
    ZaloBotClient(const char *botToken) : ZaloBotClient(String(botToken)) { }

    void SetJSONFilter(DeserializationOption::Filter filter) { _currentFilter = filter; }
    void ClearJSONFilter() { _currentFilter = std::nullopt; }

    ZaloBotESPResponse GetPollingUpdates();
    ZaloBotESPResponse GetMe();
    
    ZaloBotESPResponse SendMsg(String threadID, String content) { return SendMsg(threadID.c_str(), content.c_str()); }
    ZaloBotESPResponse SendMsg(const char *threadID, const char *content);
    
    ZaloBotESPResponse SendPhoto(String threadID, String photoUrl, String caption) { return SendPhoto(threadID.c_str(), photoUrl.c_str(), caption.c_str()); }
    ZaloBotESPResponse SendPhoto(const char *threadID, const char *photoUrl, const char *caption);

    ZaloBotESPResponse SendSticker(String threadID, String stickerID) { return SendSticker(threadID.c_str(), stickerID.c_str()); }
    ZaloBotESPResponse SendSticker(const char *threadID, const char *stickerID);

    ZaloBotESPResponse SendChatAction(String threadID, String action) { return SendChatAction(threadID.c_str(), action.c_str()); }
    ZaloBotESPResponse SendChatAction(const char *threadID, const char *action);

private:
    ZaloBotESPResponse _httpGet(const char* endpoint);
    ZaloBotESPResponse _httpPost(const char* endpoint, const char *keys[], const char *values[], int count);
    void _httpGetAsync(const char* endpoint);
    void _cancelPolling();
    ZaloBotESPResponse _httpGetAsyncGetResponse();
    static void _buildFormBody(char* buffer, const char *keys[], const char *values[], int count);
    String _botToken;
    WiFiClientSecure _wifiClient;
    HTTPClientMod _httpClient;
    HTTPClientMod _asyncHttpClient;
    bool _poolingInProgress = false;
    std::optional<DeserializationOption::Filter> _currentFilter;
};