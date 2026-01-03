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
        static DiscordESPResponse SendMessage(String webhookUrl, DiscordMessageBuilder &builder, String threadName = "", vector<uint64_t> tagIDs = {}) { return SendMessage(webhookUrl.c_str(), builder, threadName.c_str(), tagIDs); }
        static DiscordESPResponse SendMessage(String webhookUrl, String content, String username = "", String avatarUrl = "", String threadName = "", vector<uint64_t> tagIDs = {}) { return SendMessage(webhookUrl.c_str(), content.c_str(), username.c_str(), avatarUrl.c_str(), threadName.c_str(), tagIDs); }
        static DiscordESPResponse SendMessage(const char *webhookUrl, DiscordMessageBuilder &builder, const char *threadName = "", vector<uint64_t> tagIDs = {});
        static DiscordESPResponse SendMessage(const char *webhookUrl, const char *content, const char *username = "", const char *avatarUrl = "", const char *threadName = "", vector<uint64_t> tagIDs = {});

        static DiscordESPResponse SendMessageNoWait(String webhookUrl, DiscordMessageBuilder &builder, String threadName = "", vector<uint64_t> tagIDs = {}) { return SendMessageNoWait(webhookUrl.c_str(), builder, threadName.c_str(), tagIDs); }
        static DiscordESPResponse SendMessageNoWait(String webhookUrl, String content, String username = "", String avatarUrl = "", String threadName = "", vector<uint64_t> tagIDs = {}) { return SendMessageNoWait(webhookUrl.c_str(), content.c_str(), username.c_str(), avatarUrl.c_str(), threadName.c_str(), tagIDs); }
        static DiscordESPResponse SendMessageNoWait(const char *webhookUrl, DiscordMessageBuilder &builder, const char *threadName = "", vector<uint64_t> tagIDs = {});
        static DiscordESPResponse SendMessageNoWait(const char *webhookUrl, const char *content, const char *username = "", const char *avatarUrl = "", const char *threadName = "", vector<uint64_t> tagIDs = {});
    };

    struct Bot 
    {
        static DiscordESPResponse SendMessage(String token, String channelId, String content) { return SendMessage(token.c_str(), channelId.c_str(), content.c_str()); }
        static DiscordESPResponse SendMessage(const char *token, const char *channelId, const char *content);
        static DiscordESPResponse SendMessage(const char *token, uint64_t channelId, const char *content)
        {
            char channelIdStr[21];
            snprintf(channelIdStr, sizeof(channelIdStr), "%llu", channelId);
            return SendMessage(token, channelIdStr, content);
        }
        static DiscordESPResponse SendMessage(String token, uint64_t channelId, String content)
        {
            char channelIdStr[21];
            snprintf(channelIdStr, sizeof(channelIdStr), "%llu", channelId);
            return SendMessage(token.c_str(), channelIdStr, content.c_str());
        }
        
        static DiscordESPResponse SendMessage(String token, String channelId, DiscordMessageBuilder &builder) { return SendMessage(token.c_str(), channelId.c_str(), builder); }
        static DiscordESPResponse SendMessage(const char *token, const char *channelId, DiscordMessageBuilder &builder);
        static DiscordESPResponse SendMessage(const char *token, uint64_t channelId, DiscordMessageBuilder &builder)
        {
            char channelIdStr[21];
            snprintf(channelIdStr, sizeof(channelIdStr), "%llu", channelId);
            return SendMessage(token, channelIdStr, builder);
        }
        static DiscordESPResponse SendMessage(String token, uint64_t channelId, DiscordMessageBuilder &builder) 
        {
            char channelIdStr[21];
            snprintf(channelIdStr, sizeof(channelIdStr), "%llu", channelId);
            return SendMessage(token.c_str(), channelIdStr, builder);
        }

        static DiscordESPResponse AddReaction(String token, String channelId, String messageId, String emoji) { return AddReaction(token.c_str(), channelId.c_str(), messageId.c_str(), emoji.c_str()); }
        static DiscordESPResponse AddReaction(const char *token, const char *channelId, const char *messageId, const char *emoji);
        static DiscordESPResponse AddReaction(const char *token, uint64_t channelId, uint64_t messageId, const char *emoji) 
        {
            char channelIdStr[21];
            char messageIdStr[21];
            snprintf(channelIdStr, sizeof(channelIdStr), "%llu", channelId);
            snprintf(messageIdStr, sizeof(messageIdStr), "%llu", messageId);
            return AddReaction(token, channelIdStr, messageIdStr, emoji);
        }
        static DiscordESPResponse AddReaction(String token, uint64_t channelId, uint64_t messageId, String emoji) 
        {
            char channelIdStr[21];
            char messageIdStr[21];
            snprintf(channelIdStr, sizeof(channelIdStr), "%llu", channelId);
            snprintf(messageIdStr, sizeof(messageIdStr), "%llu", messageId);
            return AddReaction(token.c_str(), channelIdStr, messageIdStr, emoji.c_str());
        }
        
        static DiscordESPResponse GetMessages(String token, String channelId, String around = "", String before = "", String after = "", int limit = 50) { return GetMessages(token.c_str(), channelId.c_str(), around.c_str(), before.c_str(), after.c_str(), limit); }
        static DiscordESPResponse GetMessages(const char *token, const char *channelId, const char *around = "", const char *before = "", const char *after = "", int limit = 50);
        static DiscordESPResponse GetMessages(String token, uint64_t channelId, String around = "", String before = "", String after = "", int limit = 50) 
        {
            char channelIdStr[21];
            snprintf(channelIdStr, sizeof(channelIdStr), "%llu", channelId);
            return GetMessages(token.c_str(), channelIdStr, around.c_str(), before.c_str(), after.c_str(), limit);
        }
    };

private:
    static JsonDocument _build(DiscordMessageBuilder &builder, bool forWebhook);
    static void _urlEncode(const char* str, char* buffer);
    static DiscordESPResponse _sendRequest(const char* token, const char* url, const char* method, JsonDocument doc);
    static WiFiClientSecure _wifiClient;
    static HTTPClient _httpClient;
    static std::optional<DeserializationOption::Filter> _currentFilter;
};
