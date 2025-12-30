#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include "DiscordWebhookMessageBuilder.hpp"
#include "DisHookESPResponse.h"
#include "DiscordComponent.hpp"
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
#endif

class DisHookESP
{
public:
    static void SetupClient();

    static DisHookESPResponse SendMessage(String url, DiscordWebhookMessageBuilder &builder, String threadName = "", vector<uint64_t> tagIDs = {});
    static DisHookESPResponse SendMessage(String url, String content, String username = "", String avatarUrl = "", String threadName = "", vector<uint64_t> tagIDs = {});
    static DisHookESPResponse SendMessage(const char *url, DiscordWebhookMessageBuilder &builder, const char *threadName = "", vector<uint64_t> tagIDs = {})
    {
        return SendMessage(String(url), builder, String(threadName), tagIDs);
    }
    static DisHookESPResponse SendMessage(const char *url, const char *content, const char *username = "", const char *avatarUrl = "", const char *threadName = "", vector<uint64_t> tagIDs = {})
    {
        return SendMessage(String(url), String(content), String(username), String(avatarUrl), String(threadName), tagIDs);
    }

private:
    static DisHookESPResponse _sendMessage(String url, JsonDocument doc);
    static WiFiClientSecure _wifiClient;
    static HTTPClient _httpClient;
};
