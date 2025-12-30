#include "DiscordBotClient.hpp"

WiFiClientSecure DiscordBotClient::_wifiClient;
HTTPClient DiscordBotClient::_httpClient;

static const char DISCORD_COM_CA[] PROGMEM = R"(
-----BEGIN CERTIFICATE-----
MIICCTCCAY6gAwIBAgINAgPlwGjvYxqccpBQUjAKBggqhkjOPQQDAzBHMQswCQYD
VQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEUMBIG
A1UEAxMLR1RTIFJvb3QgUjQwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAwMDAw
WjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2Vz
IExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjQwdjAQBgcqhkjOPQIBBgUrgQQAIgNi
AATzdHOnaItgrkO4NcWBMHtLSZ37wWHO5t5GvWvVYRg1rkDdc/eJkTBa6zzuhXyi
QHY7qca4R9gq55KRanPpsXI5nymfopjTX15YhmUPoYRlBtHci8nHc8iMai/lxKvR
HYqjQjBAMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQW
BBSATNbrdP9JNqPV2Py1PsVq8JQdjDAKBggqhkjOPQQDAwNpADBmAjEA6ED/g94D
9J+uHXqnLrmvT/aDHQ4thQEd0dlq7A/Cr8deVl5c1RxYIigL9zC2L7F8AjEA8GE8
p/SgguMh1YQdc4acLa/KNJvxn7kjNuK8YAOdgLOaVsjh4rsUecrNIdSUtUlD
-----END CERTIFICATE-----)";

#define BASE_DISCORD_API_URL "https://discord.com/api/v10/"

ESPDiscordBotResponse DiscordBotClient::SendMessage(String token, String channelId, String content)
{
    if (token.isEmpty())
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::InvalidParameters, "Token is empty.");
    if (channelId.isEmpty())
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::InvalidParameters, "Channel ID is empty.");
    if (content.isEmpty())
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::InvalidParameters, "Content is empty.");

    if (WiFi.status() != WL_CONNECTED)
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::WifiNotConnected);

    String url = BASE_DISCORD_API_URL "channels/" + channelId + "/messages";

    JsonDocument doc;
    doc["content"] = content;

    return _sendRequest(token, url, "POST", doc);
}

ESPDiscordBotResponse DiscordBotClient::AddReaction(String token, String channelId, String messageId, String emoji)
{
    if (token.isEmpty())
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::InvalidParameters, "Token is empty.");
    if (channelId.isEmpty())
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::InvalidParameters, "Channel ID is empty.");
    if (messageId.isEmpty())
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::InvalidParameters, "Message ID is empty.");
    if (emoji.isEmpty())
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::InvalidParameters, "Emoji is empty.");

    if (WiFi.status() != WL_CONNECTED)
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::WifiNotConnected);

    String encodedEmoji = emoji;
    if (emoji.indexOf(':') == -1) {
        encodedEmoji = _urlEncode(emoji);
    }

    String url = BASE_DISCORD_API_URL "channels/" + channelId + "/messages/" + messageId + "/reactions/" + encodedEmoji + "/@me";

    return _sendRequest(token, url, "PUT");
}

ESPDiscordBotResponse DiscordBotClient::GetMessages(String token, String channelId, String around, String before, String after, int limit)
{
    if (token.isEmpty())
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::InvalidParameters, "Token is empty.");
    if (channelId.isEmpty())
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::InvalidParameters, "Channel ID is empty.");
    if (limit < 1 || limit > 100)
        limit = 50;  // Discord API limit is 1-100

    if (WiFi.status() != WL_CONNECTED)
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::WifiNotConnected);

    String url = BASE_DISCORD_API_URL "channels/" + channelId + "/messages?";
    if (!around.isEmpty()) url += "around=" + around + "&";
    if (!before.isEmpty()) url += "before=" + before + "&";
    if (!after.isEmpty()) url += "after=" + after + "&";
    url += "limit=" + String(limit);

    return _sendRequest(token, url, "GET");
}

ESPDiscordBotResponse DiscordBotClient::_sendRequest(String token, String url, String method, JsonDocument doc)
{
    if (!_httpClient.begin(_wifiClient, url))
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::HttpConnectionFailed);
    _httpClient.addHeader("Authorization", "Bot " + token);
    _httpClient.addHeader("Content-Type", "application/json");
    String jsonString;
    if (!doc.isNull())
        serializeJson(doc, jsonString);
    int httpResponseCode;
    if (method == "POST")
        httpResponseCode = _httpClient.POST(jsonString);
    else if (method == "PUT")
        httpResponseCode = _httpClient.PUT(jsonString);
    else
        httpResponseCode = _httpClient.sendRequest(method.c_str(), jsonString);

    JsonDocument responseDoc;
    if (httpResponseCode < 0)
    {
        _httpClient.end();
        return ESPDiscordBotResponse(static_cast<ESPDiscordBotResponseCode>(httpResponseCode));

    }
    if (httpResponseCode == 200 || httpResponseCode == 201)
    {
        String responseBody = _httpClient.getString();
        if (!responseBody.isEmpty())
        {
            DeserializationError error = deserializeJson(responseDoc, responseBody);
            if (error)
                return ESPDiscordBotResponse(ESPDiscordBotResponseCode::JsonDeserializationFailed);
        }
        _httpClient.end();
        return ESPDiscordBotResponse(responseDoc);
    }
    else if (httpResponseCode == 204)
    {
        _httpClient.end();
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::NoContent);
    }
    // Error responses
    DeserializationError error = deserializeJson(responseDoc, _httpClient.getString());
    _httpClient.end();
    if (error)
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::JsonDeserializationFailed);
    if (httpResponseCode == 400)
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::BadRequest, responseDoc);
    if (httpResponseCode == 401)
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::Unauthorized, responseDoc);
    if (httpResponseCode == 403)
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::Forbidden, responseDoc);
    if (httpResponseCode == 404)
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::NotFound, responseDoc);
    if (httpResponseCode == 408)
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::RequestTimeout, responseDoc);
    if (httpResponseCode == 429)
        return ESPDiscordBotResponse(ESPDiscordBotResponseCode::RateLimitExceeded, responseDoc);
    return ESPDiscordBotResponse(ESPDiscordBotResponseCode::UnknownError, responseDoc);
}

void DiscordBotClient::SetupClient()
{
#if defined(ESP8266)
    _wifiClient.setTrustAnchors(new BearSSL::X509List(DISCORD_COM_CA));
    _wifiClient.setBufferSizes(4096, 2048);
    _httpClient.setUserAgent("DiscordBot (https://github.com/ElectroHeavenVN/IoT_Libraries/tree/main/ESPDiscordBot, 1.0), ESP8266HTTPClient");
#elif defined(ESP32)
_wifiClient.setCACert(DISCORD_COM_CA);
_httpClient.setUserAgent("DiscordBot (https://github.com/ElectroHeavenVN/IoT_Libraries/tree/main/ESPDiscordBot, 1.0), ESP32HTTPClient");
#endif
    _httpClient.setTimeout(5000);
}

String DiscordBotClient::_urlEncode(const String& str) {
    String encodedString = "";
    char c;
    char buf[4];
    for (size_t i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encodedString += c;
        } else {
            snprintf(buf, sizeof(buf), "%%%02X", (unsigned char)c);
            encodedString += buf;
        }
    }
    return encodedString;
}

#undef BASE_DISCORD_API_URL