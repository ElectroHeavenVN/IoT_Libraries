#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
#endif
#include <optional>
#include "DiscordMessageBuilder.hpp"
#include "DiscordESPResponse.h"
#include "DiscordComponent.hpp"

class DiscordESP
{
public:
    static void SetupClient();
    static void SetJSONFilter(DeserializationOption::Filter filter) { _currentFilter = filter; }
    static void ClearJSONFilter() { _currentFilter = std::nullopt; }

    struct Webhook
    {
        static DiscordESPResponse SendMessage(String webhookUrl, DiscordMessageBuilder &builder, String threadName = "", vector<uint64_t> tagIDs = {});
        static DiscordESPResponse SendMessage(String webhookUrl, String content, String username = "", String avatarUrl = "", String threadName = "", vector<uint64_t> tagIDs = {});
        static DiscordESPResponse SendMessage(const char *webhookUrl, DiscordMessageBuilder &builder, const char *threadName = "", vector<uint64_t> tagIDs = {}) { return SendMessage(String(webhookUrl), builder, String(threadName), tagIDs); }
        static DiscordESPResponse SendMessage(const char *webhookUrl, const char *content, const char *username = "", const char *avatarUrl = "", const char *threadName = "", vector<uint64_t> tagIDs = {}) { return SendMessage(String(webhookUrl), String(content), String(username), String(avatarUrl), String(threadName), tagIDs); }
    };

    struct Bot 
    {
        static DiscordESPResponse SendMessage(String token, String channelId, String content);
        static DiscordESPResponse SendMessage(const char *token, const char *channelId, const char *content) { return SendMessage(String(token), String(channelId), String(content)); }
        static DiscordESPResponse SendMessage(const char *token, uint64_t channelId, const char *content) { return SendMessage(String(token), String(channelId), String(content)); }
        static DiscordESPResponse SendMessage(String token, uint64_t channelId, String content) { return SendMessage(token, String(channelId), content); }
        
        static DiscordESPResponse SendMessage(String token, String channelId, DiscordMessageBuilder &builder);
        static DiscordESPResponse SendMessage(const char *token, const char *channelId, DiscordMessageBuilder &builder) { return SendMessage(String(token), String(channelId), builder); }
        static DiscordESPResponse SendMessage(const char *token, uint64_t channelId, DiscordMessageBuilder &builder) { return SendMessage(String(token), String(channelId), builder); }
        static DiscordESPResponse SendMessage(String token, uint64_t channelId, DiscordMessageBuilder &builder) { return SendMessage(token, String(channelId), builder); }

        static DiscordESPResponse AddReaction(String token, String channelId, String messageId, String emoji);
        static DiscordESPResponse AddReaction(const char *token, const char *channelId, const char *messageId, const char *emoji) { return AddReaction(String(token), String(channelId), String(messageId), String(emoji)); }
        static DiscordESPResponse AddReaction(const char *token, uint64_t channelId, uint64_t messageId, const char *emoji) { return AddReaction(String(token), String(channelId), String(messageId), String(emoji)); }
        static DiscordESPResponse AddReaction(String token, uint64_t channelId, uint64_t messageId, String emoji) { return AddReaction(token, String(channelId), String(messageId), emoji); }
        
        static DiscordESPResponse GetMessages(String token, String channelId, String around = "", String before = "", String after = "", int limit = 50);
        static DiscordESPResponse GetMessages(const char *token, const char *channelId, const char *around = "", const char *before = "", const char *after = "", int limit = 50) { return GetMessages(String(token), String(channelId), String(around), String(before), String(after), limit); }
        static DiscordESPResponse GetMessages(const char *token, uint64_t channelId, const char *around = "", const char *before = "", const char *after = "", int limit = 50) { return GetMessages(String(token), String(channelId), String(around), String(before), String(after), limit); }
        static DiscordESPResponse GetMessages(String token, uint64_t channelId, String around = "", String before = "", String after = "", int limit = 50) { return GetMessages(token, String(channelId), String(around), String(before), String(after), limit); }
    };

private:
    static JsonDocument _build(DiscordMessageBuilder &builder, bool forWebhook);
    static String _urlEncode(String str);
    static DiscordESPResponse _sendRequest(String token, String url, String method, JsonDocument doc);
    static WiFiClientSecure _wifiClient;
    static HTTPClient _httpClient;
    static std::optional<DeserializationOption::Filter> _currentFilter;
};
