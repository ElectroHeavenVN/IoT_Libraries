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
    ZaloBotClient(String botToken);
    ZaloBotClient(const char *botToken) : ZaloBotClient(String(botToken)) {}
    ZaloBotESPResponse SendMsg(String threadID, String content);
    ZaloBotESPResponse SendMsg(const char *threadID, const char *content)
    {
        return SendMsg(String(threadID), String(content));
    }
    ZaloBotESPResponse GetPollingUpdates();
    ZaloBotESPResponse GetMe();
    ZaloBotESPResponse SendPhoto(String threadID, String photoUrl, String caption);
    ZaloBotESPResponse SendPhoto(const char *threadID, const char *photoUrl, const char *caption)
    {
        return SendPhoto(String(threadID), String(photoUrl), String(caption));
    }
    ZaloBotESPResponse SendSticker(String threadID, String stickerID);
    ZaloBotESPResponse SendSticker(const char *threadID, const char *stickerID)
    {
        return SendSticker(String(threadID), String(stickerID));
    }
    ZaloBotESPResponse SendChatAction(String threadID, String action);
    ZaloBotESPResponse SendChatAction(const char *threadID, const char *action)
    {
        return SendChatAction(String(threadID), String(action));
    }

private:
    ZaloBotESPResponse _httpGet(String endpoint);
    ZaloBotESPResponse _httpPost(String endpoint, const char *keys[], const char *values[], int count);
    void _httpGetAsync(String endpoint);
    void _cancelPolling();
    ZaloBotESPResponse _httpGetAsyncGetResponse();
    static String _buildFormBody(const char *keys[], const char *values[], int count);
    String _botToken;
    WiFiClientSecure _wifiClient;
    HTTPClientMod _httpClient;
    HTTPClientMod _asyncHttpClient;
    bool _poolingInProgress = false;
};