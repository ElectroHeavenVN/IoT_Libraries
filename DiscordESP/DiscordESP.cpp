#include "DiscordESP.hpp"
#include <StreamUtils.hpp>
#include "DiscordMessageFlags.h"

#define BASE_DISCORD_API_URL "https://discord.com/api/v10/"

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

WiFiClientSecure DiscordESP::_wifiClient;
HTTPClient DiscordESP::_httpClient;
std::optional<DeserializationOption::Filter> DiscordESP::_currentFilter = std::nullopt;

DiscordESPResponse DiscordESP::Bot::SendMessage(const char *token, const char *channelId, const char *content)
{
    if (token == nullptr || strlen(token) == 0)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 1);
    if (channelId == nullptr || strlen(channelId) == 0)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 2);
    if (content == nullptr || strlen(content) == 0)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 3);
    if (WiFi.status() != WL_CONNECTED)
        return DiscordESPResponse(DiscordESPResponseCode::WifiNotConnected);
    if (strlen(content) > 2000)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 4);

    JsonDocument doc;
    doc[F("content")] = content;
    char url[256];
    snprintf(url, sizeof(url), BASE_DISCORD_API_URL "channels/%s/messages", channelId);
    return _sendRequest(token, url, "POST", doc);
}

DiscordESPResponse DiscordESP::Bot::SendMessage(const char *token, const char *channelId, DiscordMessageBuilder &builder)
{
    if (token == nullptr || strlen(token) == 0)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 1);
    if (channelId == nullptr || strlen(channelId) == 0)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 2);
    if (WiFi.status() != WL_CONNECTED)
        return DiscordESPResponse(DiscordESPResponseCode::WifiNotConnected);
    if (builder.IsComponentV2())
    {
        if (builder.GetComponents().empty())
            return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 5);
        if (!builder.GetEmbeds().empty() || builder.GetContent().has_value())
            return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 6);
    }
    else
    {
        if (builder.GetEmbeds().size() > 10)
            return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 7);
    }
    JsonDocument doc = _build(builder, false);
    char url[256];
    snprintf(url, sizeof(url), BASE_DISCORD_API_URL "channels/%s/messages", channelId);
    return _sendRequest(token, url, "POST", doc);
}

DiscordESPResponse DiscordESP::Bot::AddReaction(const char *token, const char *channelId, const char *messageId, const char *emoji)
{
    if (token == nullptr || strlen(token) == 0)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 1);
    if (channelId == nullptr || strlen(channelId) == 0)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 2);
    if (messageId == nullptr || strlen(messageId) == 0)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 8);
    if (emoji == nullptr || strlen(emoji) == 0)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 9);
    if (WiFi.status() != WL_CONNECTED)
        return DiscordESPResponse(DiscordESPResponseCode::WifiNotConnected);

    char encodedEmoji[64] = {0};
    strcpy(encodedEmoji, emoji);
    if (strchr(emoji, ':') == nullptr)
        _urlEncode(emoji, encodedEmoji);
    char url[256];
    snprintf(url, sizeof(url), BASE_DISCORD_API_URL "channels/%s/messages/%s/reactions/%s/@me", channelId, messageId, encodedEmoji);
    return _sendRequest(token, url, "PUT", JsonDocument());
}

DiscordESPResponse DiscordESP::Bot::GetMessages(const char *token, const char *channelId, const char *around, const char *before, const char *after, int limit)
{
    if (token == nullptr || strlen(token) == 0)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 1);
    if (channelId == nullptr || strlen(channelId) == 0)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 2);
    if (limit < 1 || limit > 100)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 10);
    int count = 0;
    if (around != nullptr && strlen(around) > 0)
        count++;
    if (before != nullptr && strlen(before) > 0)
        count++;
    if (after != nullptr && strlen(after) > 0)
        count++;
    if (count > 1)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 11);
    if (WiFi.status() != WL_CONNECTED)
        return DiscordESPResponse(DiscordESPResponseCode::WifiNotConnected);
    char url[256];
    snprintf(url, sizeof(url), BASE_DISCORD_API_URL "channels/%s/messages?", channelId);
    if (around != nullptr && strlen(around) > 0)
    {
        strcat(url, "around=");
        strcat(url, around);
        strcat(url, "&");
    }
    else if (before != nullptr && strlen(before) > 0)
    {
        strcat(url, "before=");
        strcat(url, before);
        strcat(url, "&");
    }
    else if (after != nullptr && strlen(after) > 0)
    {
        strcat(url, "after=");
        strcat(url, after);
        strcat(url, "&");
    }
    strcat(url, "limit=");
    char limitStr[4];
    snprintf(limitStr, sizeof(limitStr), "%d", limit);
    strcat(url, limitStr);
    return _sendRequest(token, url, "GET", JsonDocument());
}

// Add thread id to the webhook url as ?thread_id=THREAD_ID to send message to a thread
// Pass threadName to create a new thread with that name
DiscordESPResponse DiscordESP::Webhook::SendMessage(const char *webhookUrl, DiscordMessageBuilder &builder, const char *threadName, vector<uint64_t> tagIDs)
{
    if (webhookUrl == nullptr || strlen(webhookUrl) == 0)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 12);
    if (WiFi.status() != WL_CONNECTED)
        return DiscordESPResponse(DiscordESPResponseCode::WifiNotConnected);
    if (builder.IsComponentV2())
    {
        if (builder.GetComponents().empty())
            return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 5);
        if (!builder.GetEmbeds().empty() || builder.GetContent().has_value())
            return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 6);
    }
    else
    {
        if (builder.GetEmbeds().size() > 10)
            return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 7);
    }
    char webhookUrlBuffer[256];
    strncpy(webhookUrlBuffer, webhookUrl, sizeof(webhookUrlBuffer) - 1);
    webhookUrlBuffer[sizeof(webhookUrlBuffer) - 1] = '\0';
    if (strstr(webhookUrl, "wait=true") == nullptr)
    {
        if (strchr(webhookUrl, '?') == nullptr)
            strcat(webhookUrlBuffer, "?wait=true");
        else
            strcat(webhookUrlBuffer, "&wait=true");
    }
    if (builder.IsComponentV2())
        strcat(webhookUrlBuffer, "&with_components=true");
    JsonDocument doc = _build(builder, true);
    if (threadName != nullptr && strlen(threadName) > 0)
    {
        doc[F("thread_name")] = threadName;
        JsonArray tagIDsArray = doc[F("applied_tags")].to<JsonArray>();
        for (const uint64_t &tagID : tagIDs)
            tagIDsArray.add(tagID);
    }
    return _sendRequest("", webhookUrlBuffer, "POST", doc);
}

// Add thread id to the webhook url as ?thread_id=THREAD_ID to send message to a thread
// Pass threadName to create a new thread with that name
DiscordESPResponse DiscordESP::Webhook::SendMessageNoWait(const char *webhookUrl, DiscordMessageBuilder &builder, const char *threadName, vector<uint64_t> tagIDs)
{
    if (webhookUrl == nullptr || strlen(webhookUrl) == 0)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 12);
    if (WiFi.status() != WL_CONNECTED)
        return DiscordESPResponse(DiscordESPResponseCode::WifiNotConnected);
    if (builder.IsComponentV2())
    {
        if (builder.GetComponents().empty())
            return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 5);
        if (!builder.GetEmbeds().empty() || builder.GetContent().has_value())
            return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 6);
    }
    else
    {
        if (builder.GetEmbeds().size() > 10)
            return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 7);
    }

    char webhookUrlBuffer[256];
    strncpy(webhookUrlBuffer, webhookUrl, sizeof(webhookUrlBuffer) - 1);
    webhookUrlBuffer[sizeof(webhookUrlBuffer) - 1] = '\0';
    if (builder.IsComponentV2()) 
    {
        if (strchr(webhookUrl, '?') == nullptr)
            strcat(webhookUrlBuffer, "?with_components=true");
        else
            strcat(webhookUrlBuffer, "&with_components=true");
    }
    JsonDocument doc = _build(builder, true);
    if (threadName != nullptr && strlen(threadName) > 0)
    {
        doc[F("thread_name")] = threadName;
        JsonArray tagIDsArray = doc[F("applied_tags")].to<JsonArray>();
        for (const uint64_t &tagID : tagIDs)
            tagIDsArray.add(tagID);
    }
    return _sendRequest("", webhookUrlBuffer, "POST", doc);
}

DiscordESPResponse DiscordESP::Webhook::SendMessage(const char *webhookUrl, const char *content, const char *username, const char *avatarUrl, const char *threadName, vector<uint64_t> tagIDs)
{
    if (webhookUrl == nullptr || strlen(webhookUrl) == 0)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 12);
    if (content == nullptr || strlen(content) == 0)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 3);
    if (WiFi.status() != WL_CONNECTED)
        return DiscordESPResponse(DiscordESPResponseCode::WifiNotConnected);
    char webhookUrlBuffer[256];
    strncpy(webhookUrlBuffer, webhookUrl, sizeof(webhookUrlBuffer) - 1);
    webhookUrlBuffer[sizeof(webhookUrlBuffer) - 1] = '\0';
    if (strstr(webhookUrl, "wait=true") == nullptr)
    {
        if (strchr(webhookUrl, '?') == nullptr)
            strcat(webhookUrlBuffer, "?wait=true");
        else
            strcat(webhookUrlBuffer, "&wait=true");
    }
    JsonDocument doc;
    doc[F("content")] = content;
    if (username != nullptr && strlen(username) > 0)
        doc[F("username")] = username;
    if (avatarUrl != nullptr && strlen(avatarUrl) > 0)
        doc[F("avatar_url")] = avatarUrl;
    if (threadName != nullptr && strlen(threadName) > 0)
    {
        doc[F("thread_name")] = threadName;
        JsonArray tagIDsArray = doc[F("applied_tags")].to<JsonArray>();
        for (const uint64_t &tagID : tagIDs)
            tagIDsArray.add(tagID);
    }
    return _sendRequest("", webhookUrlBuffer, "POST", doc);
}

DiscordESPResponse DiscordESP::Webhook::SendMessageNoWait(const char *webhookUrl, const char *content, const char *username, const char *avatarUrl, const char *threadName, vector<uint64_t> tagIDs)
{
    if (webhookUrl == nullptr || strlen(webhookUrl) == 0)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 12);
    if (content == nullptr || strlen(content) == 0)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, 3);
    if (WiFi.status() != WL_CONNECTED)
        return DiscordESPResponse(DiscordESPResponseCode::WifiNotConnected);
    JsonDocument doc;
    doc[F("content")] = content;
    if (username != nullptr && strlen(username) > 0)
        doc[F("username")] = username;
    if (avatarUrl != nullptr && strlen(avatarUrl) > 0)
        doc[F("avatar_url")] = avatarUrl;
    if (threadName != nullptr && strlen(threadName) > 0)
    {
        doc[F("thread_name")] = threadName;
        JsonArray tagIDsArray = doc[F("applied_tags")].to<JsonArray>();
        for (const uint64_t &tagID : tagIDs)
            tagIDsArray.add(tagID);
    }
    return _sendRequest("", webhookUrl, "POST", doc);
}

// ---------------------------------------------------------------

DiscordESPResponse DiscordESP::_sendRequest(const char *token, const char *url, const char *method, JsonDocument doc)
{
    if (!_httpClient.begin(_wifiClient, url))
        return DiscordESPResponse(DiscordESPResponseCode::HttpConnectionFailed);
    if (token != nullptr && strlen(token) > 0) 
    {
        char authHeader[128];
        snprintf(authHeader, sizeof(authHeader), "Bot %s", token);
        _httpClient.addHeader("Authorization", authHeader);
    }
    _httpClient.addHeader("Content-Type", "application/json");
    const char *keys[] = {"Transfer-Encoding"};
    _httpClient.collectHeaders(keys, 1);
    String jsonString;
    if (!doc.isNull())
        serializeJson(doc, jsonString);
    int httpResponseCode = _httpClient.sendRequest(method, jsonString);
    if (httpResponseCode < 0)
    {
        _httpClient.end();
        return DiscordESPResponse(static_cast<DiscordESPResponseCode>(httpResponseCode));
    }
    if (httpResponseCode == 204)
    {
        _httpClient.end();
        return DiscordESPResponse(DiscordESPResponseCode::NoContent);
    }
    JsonDocument responseDoc;
    DeserializationError error;
    if (strcasecmp(_httpClient.header("Transfer-Encoding").c_str(), "chunked") == 0)
    {
        StreamUtils::ChunkDecodingStream decodedStream(_httpClient.getStream());
        if (_currentFilter.has_value())
            error = deserializeJson(responseDoc, decodedStream, _currentFilter.value());
        else
            error = deserializeJson(responseDoc, decodedStream);
    }
    else
    {
        if (_currentFilter.has_value())
            error = deserializeJson(responseDoc, _httpClient.getString(), _currentFilter.value());
        else
            error = deserializeJson(responseDoc, _httpClient.getString());
    }
    _httpClient.end();
    if (error.code() != 0)
    {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "JSON deserialization failed: %s (%d)", error.c_str(), error.code());
        return DiscordESPResponse(DiscordESPResponseCode::JsonDeserializationFailed, String(buffer));
    }
    if (httpResponseCode == 200 || httpResponseCode == 201)
        return DiscordESPResponse(responseDoc);
    if (httpResponseCode == 204)
        return DiscordESPResponse(DiscordESPResponseCode::NoContent);
    if (httpResponseCode == 400)
        return DiscordESPResponse(DiscordESPResponseCode::BadRequest, responseDoc);
    if (httpResponseCode == 401)
        return DiscordESPResponse(DiscordESPResponseCode::Unauthorized, responseDoc);
    if (httpResponseCode == 403)
        return DiscordESPResponse(DiscordESPResponseCode::Forbidden, responseDoc);
    if (httpResponseCode == 404)
        return DiscordESPResponse(DiscordESPResponseCode::NotFound, responseDoc);
    if (httpResponseCode == 408)
        return DiscordESPResponse(DiscordESPResponseCode::RequestTimeout, responseDoc);
    if (httpResponseCode == 429)
        return DiscordESPResponse(DiscordESPResponseCode::RateLimitExceeded, responseDoc);
    return DiscordESPResponse(DiscordESPResponseCode::UnknownError, responseDoc);
}

JsonDocument DiscordESP::_build(DiscordMessageBuilder &builder, bool forWebhook)
{
    JsonDocument doc;
    bool isComponentV2 = builder.IsComponentV2();
    const auto &components = builder.GetComponents();
    auto embeds = builder.GetEmbeds();
    uint64_t flags = 0;
    if (builder.SuppressesEmbeds())
        flags |= static_cast<uint64_t>(DiscordMessageFlags::SuppressEmbeds);
    if (builder.SuppressesNotifications())
        flags |= static_cast<uint64_t>(DiscordMessageFlags::SuppressNotifications);
    if (isComponentV2)
        flags |= static_cast<uint64_t>(DiscordMessageFlags::IsComponentV2);
    if (builder.IsVoiceMessage() && !forWebhook)
        flags |= static_cast<uint64_t>(DiscordMessageFlags::IsVoiceMessage);
    doc[F("flags")] = flags;
    if (builder.GetContent().has_value())
        doc[F("content")] = builder.GetContent().value();
    if (builder.GetUsername().has_value())
        doc[F("username")] = builder.GetUsername().value();
    if (builder.GetAvatarUrl().has_value())
        doc[F("avatar_url")] = builder.GetAvatarUrl().value();
    if (builder.IsTTS())
        doc[F("tts")] = true;
    if (builder.GetAllowedMentions().has_value())
        doc[F("allowed_mentions")] = builder.GetAllowedMentions().value().ToJsonDocument();
    if (!embeds.empty())
    {
        JsonArray embedsArray = doc[F("embeds")].to<JsonArray>();
        for (auto &embed : embeds)
            embedsArray.add(embed.ToJsonDocument());
    }
    if (!components.empty())
    {
        JsonArray componentsArray = doc[F("components")].to<JsonArray>();
        for (const auto &component : components)
        {
            if (forWebhook && component->GetType() == DiscordComponentType::Button)
            {
                const ButtonComponent *button = static_cast<const ButtonComponent *>(component.get());
                if (button->GetStyle() != DiscordButtonStyle::Link)
                    continue;
            }
            componentsArray.add(component->ToJsonDocument());
        }
    }
    return doc;
}

void DiscordESP::_urlEncode(const char *str, char *buffer)
{
    char buf[4];
    for (size_t i = 0; i < strlen(str); i++)
    {
        char c = str[i];
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            strncat(buffer, &c, 1);
        else
        {
            snprintf(buf, sizeof(buf), "%%%02X", (unsigned char)c);
            strncat(buffer, buf, sizeof(buf) - 1);
        }
    }
}

void DiscordESP::SetupClient()
{
#if defined(ESP8266)
    _wifiClient.setTrustAnchors(new BearSSL::X509List(DISCORD_COM_CA));
    _wifiClient.setBufferSizes(4096, 2048);
    _httpClient.setUserAgent("DiscordBot (https://github.com/ElectroHeavenVN/IoT_Libraries/tree/main/DiscordESP, 1.0), ESP8266HTTPClient");
#elif defined(ESP32)
    _wifiClient.setCACert(DISCORD_COM_CA);
    _httpClient.setUserAgent("DiscordBot (https://github.com/ElectroHeavenVN/IoT_Libraries/tree/main/DiscordESP, 1.0), ESP32HTTPClient");
#endif
    _httpClient.setTimeout(5000);
}