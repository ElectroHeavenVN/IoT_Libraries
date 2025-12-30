#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include "ESPDiscordBotResponse.h"
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
#endif

class ESPDiscordBot
{
public:
    static void SetupClient();

    static ESPDiscordBotResponse SendMessage(String token, String channelId, String content);
    static ESPDiscordBotResponse SendMessage(const char *token, const char *channelId, const char *content)
    {
        return SendMessage(String(token), String(channelId), String(content));
    }
    static ESPDiscordBotResponse SendMessage(const char *token, uint64_t channelId, const char *content)
    {
        return SendMessage(String(token), String(channelId), String(content));
    }
    static ESPDiscordBotResponse SendMessage(String token, uint64_t channelId, String content)
    {
        return SendMessage(token, String(channelId), content);
    }
    static ESPDiscordBotResponse AddReaction(String token, String channelId, String messageId, String emoji);
    static ESPDiscordBotResponse AddReaction(const char *token, const char *channelId, const char *messageId, const char *emoji)
    {
        return AddReaction(String(token), String(channelId), String(messageId), String(emoji));
    }
    static ESPDiscordBotResponse AddReaction(const char *token, uint64_t channelId, uint64_t messageId, const char *emoji)
    {
        return AddReaction(String(token), String(channelId), String(messageId), String(emoji));
    }
    static ESPDiscordBotResponse AddReaction(String token, uint64_t channelId, uint64_t messageId, String emoji)
    {
        return AddReaction(token, String(channelId), String(messageId), emoji);
    }
    static ESPDiscordBotResponse GetMessages(String token, String channelId, String around = "", String before = "", String after = "", int limit = 50);
    static ESPDiscordBotResponse GetMessages(const char *token, const char *channelId, const char *around = "", const char *before = "", const char *after = "", int limit = 50)
    {
        return GetMessages(String(token), String(channelId), String(around), String(before), String(after), limit);
    }
    static ESPDiscordBotResponse GetMessages(const char *token, uint64_t channelId, const char *around = "", const char *before = "", const char *after = "", int limit = 50)
    {
        return GetMessages(String(token), String(channelId), String(around), String(before), String(after), limit);
    }
    static ESPDiscordBotResponse GetMessages(String token, uint64_t channelId, String around = "", String before = "", String after = "", int limit = 50)
    {
        return GetMessages(token, String(channelId), String(around), String(before), String(after), limit);
    }

private:
    static String _urlEncode(String str);
    static ESPDiscordBotResponse _sendRequest(String token, String url, String method, JsonDocument doc = JsonDocument());
    static WiFiClientSecure _wifiClient;
    static HTTPClient _httpClient;
};