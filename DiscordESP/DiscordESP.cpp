#include <DiscordESP.hpp>
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

DiscordESPResponse DiscordESP::Bot::SendMessage(String token, String channelId, String content)
{
    if (token.isEmpty())
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Token is empty.");
    if (channelId.isEmpty())
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Channel ID is empty.");
    if (content.isEmpty())
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Content is empty.");
    if (WiFi.status() != WL_CONNECTED)
        return DiscordESPResponse(DiscordESPResponseCode::WifiNotConnected);
    if (content.length() > 2000)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Content exceeds 2000 characters.");

    JsonDocument doc;
    doc["content"] = content;
    char url[256];
    snprintf(url, sizeof(url), BASE_DISCORD_API_URL "channels/%s/messages", channelId.c_str());
    return _sendRequest(token, String(url), "POST", doc);
}

DiscordESPResponse DiscordESP::Bot::SendMessage(String token, String channelId, DiscordMessageBuilder &builder)
{
    if (token.isEmpty())
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Token is empty.");
    if (channelId.isEmpty())
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Channel ID is empty.");
    if (WiFi.status() != WL_CONNECTED)
        return DiscordESPResponse(DiscordESPResponseCode::WifiNotConnected);
    if (builder.IsComponentV2())
    {
        if (builder.GetComponents().empty())
            return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Message marked as ComponentV2 but has no components.");
        if (!builder.GetEmbeds().empty() || builder.GetContent().has_value())
            return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Message marked as ComponentV2 can only contain components.");
    }
    else
    {
        if (builder.GetEmbeds().size() > 10)
            return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Cannot have more than 10 embeds in a message.");
    }
    JsonDocument doc = _build(builder, false);
    char url[256];
    snprintf(url, sizeof(url), BASE_DISCORD_API_URL "channels/%s/messages", channelId.c_str());
    return _sendRequest(token, String(url), "POST", doc);
}

DiscordESPResponse DiscordESP::Bot::AddReaction(String token, String channelId, String messageId, String emoji)
{
    if (token.isEmpty())
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Token is empty.");
    if (channelId.isEmpty())
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Channel ID is empty.");
    if (messageId.isEmpty())
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Message ID is empty.");
    if (emoji.isEmpty())
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Emoji is empty.");
    if (WiFi.status() != WL_CONNECTED)
        return DiscordESPResponse(DiscordESPResponseCode::WifiNotConnected);

    String encodedEmoji = emoji;
    if (emoji.indexOf(':') == -1)
        encodedEmoji = _urlEncode(emoji);
    char url[256];
    snprintf(url, sizeof(url), BASE_DISCORD_API_URL "channels/%s/messages/%s/reactions/%s/@me", channelId.c_str(), messageId.c_str(), encodedEmoji.c_str());
    return _sendRequest(token, String(url), "PUT", JsonDocument());
}

DiscordESPResponse DiscordESP::Bot::GetMessages(String token, String channelId, String around, String before, String after, int limit)
{
    if (token.isEmpty())
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Token is empty.");
    if (channelId.isEmpty())
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Channel ID is empty.");
    if (limit < 1 || limit > 100)
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Limit must be between 1 and 100.");
    if (WiFi.status() != WL_CONNECTED)
        return DiscordESPResponse(DiscordESPResponseCode::WifiNotConnected);

    String url = BASE_DISCORD_API_URL "channels/" + channelId + "/messages?";
    if (!around.isEmpty())
        url += "around=" + around + "&";
    if (!before.isEmpty())
        url += "before=" + before + "&";
    if (!after.isEmpty())
        url += "after=" + after + "&";
    url += "limit=" + String(limit);
    return _sendRequest(token, url, "GET", JsonDocument());
}

// Add thread id to the webhook url as ?thread_id=THREAD_ID to send message to a thread
// Pass threadName to create a new thread with that name
DiscordESPResponse DiscordESP::Webhook::SendMessage(String webhookUrl, DiscordMessageBuilder &builder, String threadName, vector<uint64_t> tagIDs)
{
    if (webhookUrl.isEmpty())
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Webhook URL is empty.");
    if (WiFi.status() != WL_CONNECTED)
        return DiscordESPResponse(DiscordESPResponseCode::WifiNotConnected);
    if (!webhookUrl.endsWith("wait=true"))
    {
        if (webhookUrl.indexOf('?') == -1)
            webhookUrl += "?wait=true";
        else
            webhookUrl += "&wait=true";
    }
    if (builder.IsComponentV2())
    {
        if (builder.GetComponents().empty())
            return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Message marked as ComponentV2 but has no components.");
        if (!builder.GetEmbeds().empty() || builder.GetContent().has_value())
            return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Message marked as ComponentV2 can only contain components.");
        webhookUrl += "&with_components=true";
    }
    else
    {
        if (builder.GetEmbeds().size() > 10)
            return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Cannot have more than 10 embeds in a message.");
    }
    JsonDocument doc = _build(builder, true);
    if (!threadName.isEmpty())
    {
        doc["thread_name"] = threadName;
        JsonArray tagIDsArray = doc["applied_tags"].to<JsonArray>();
        for (const uint64_t &tagID : tagIDs)
            tagIDsArray.add(tagID);
    }
    return _sendRequest("", webhookUrl, "POST", doc);
}

DiscordESPResponse DiscordESP::Webhook::SendMessage(String webhookUrl, String content, String username, String avatarUrl, String threadName, vector<uint64_t> tagIDs)
{
    if (webhookUrl.isEmpty())
        return DiscordESPResponse(DiscordESPResponseCode::InvalidParameter, "Webhook URL is empty.");
    if (WiFi.status() != WL_CONNECTED)
        return DiscordESPResponse(DiscordESPResponseCode::WifiNotConnected);
    JsonDocument doc;
    doc["content"] = content;
    if (username != "")
        doc["username"] = username;
    if (avatarUrl != "")
        doc["avatar_url"] = avatarUrl;
    if (!threadName.isEmpty())
    {
        doc["thread_name"] = threadName;
        JsonArray tagIDsArray = doc["applied_tags"].to<JsonArray>();
        for (const uint64_t &tagID : tagIDs)
            tagIDsArray.add(tagID);
    }
    if (!webhookUrl.endsWith("wait=true"))
    {
        if (webhookUrl.indexOf('?') == -1)
            webhookUrl += "?wait=true";
        else
            webhookUrl += "&wait=true";
    }
    return _sendRequest("", webhookUrl, "POST", doc);
}

// ---------------------------------------------------------------

DiscordESPResponse DiscordESP::_sendRequest(String token, String url, String method, JsonDocument doc)
{
    if (!_httpClient.begin(_wifiClient, url))
        return DiscordESPResponse(DiscordESPResponseCode::HttpConnectionFailed);
    if (!token.isEmpty())
        _httpClient.addHeader("Authorization", "Bot " + token);
    _httpClient.addHeader("Content-Type", "application/json");
    const char *keys[] = {"Transfer-Encoding"};
    _httpClient.collectHeaders(keys, 1);
    String jsonString;
    if (!doc.isNull())
        serializeJson(doc, jsonString);
    int httpResponseCode = _httpClient.sendRequest(method.c_str(), jsonString);
    JsonDocument responseDoc;
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
        return DiscordESPResponse(DiscordESPResponseCode::JsonDeserializationFailed, "JSON deserialization failed: " + String(error.c_str()) + " (" + String(error.code()) + ")");
    if (httpResponseCode == 200 || httpResponseCode == 201)
        return DiscordESPResponse(responseDoc);
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
    doc["flags"] = flags;
    if (builder.GetContent().has_value())
        doc["content"] = builder.GetContent().value();
    if (builder.GetUsername().has_value())
        doc["username"] = builder.GetUsername().value();
    if (builder.GetAvatarUrl().has_value())
        doc["avatar_url"] = builder.GetAvatarUrl().value();
    if (builder.IsTTS())
        doc["tts"] = true;
    if (builder.GetAllowedMentions().has_value())
        doc["allowed_mentions"] = builder.GetAllowedMentions().value().ToJsonDocument();
    if (!embeds.empty())
    {
        JsonArray embedsArray = doc["embeds"].to<JsonArray>();
        for (auto &embed : embeds)
            embedsArray.add(embed.ToJsonDocument());
    }
    if (!components.empty())
    {
        JsonArray componentsArray = doc["components"].to<JsonArray>();
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

void DiscordESP::SetupClient()
{
#if defined(ESP8266)
    _wifiClient.setTrustAnchors(new BearSSL::X509List(DISCORD_COM_CA));
    // _wifiClient.setBufferSizes(4096, 2048);
    _httpClient.setUserAgent("DiscordBot (https://github.com/ElectroHeavenVN/IoT_Libraries/tree/main/DiscordESP, 1.0), ESP8266HTTPClient");
#elif defined(ESP32)
    _wifiClient.setCACert(DISCORD_COM_CA);
    _httpClient.setUserAgent("DiscordBot (https://github.com/ElectroHeavenVN/IoT_Libraries/tree/main/DiscordESP, 1.0), ESP32HTTPClient");
#endif
    _httpClient.setTimeout(5000);
}