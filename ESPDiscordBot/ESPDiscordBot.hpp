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
    static ESPDiscordBotResponse AddReaction(String token, String channelId, String messageId, String emoji);
    static ESPDiscordBotResponse GetMessages(String token, String channelId, String around = "", String before = "", String after = "", int limit = 50);

private:
    static String _urlEncode(const String& str);
    static ESPDiscordBotResponse _sendRequest(String token, String url, String method, JsonDocument doc = JsonDocument());
    static WiFiClientSecure _wifiClient;
    static HTTPClient _httpClient;
};