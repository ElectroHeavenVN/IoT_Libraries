#include "DisHookESP.hpp"
#include <StreamUtils.hpp>

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

enum class DiscordMessageFlags : uint64_t
{
    SuppressEmbeds = 1 << 2,
    SuppressNotifications = 1 << 12,
    IsComponentV2 = 1 << 15
};

WiFiClientSecure DisHookESP::_wifiClient;
HTTPClient DisHookESP::_httpClient;

// Add thread id to the url as ?thread_id=THREAD_ID to send message to a thread
// Pass threadName to create a new thread with that name
DisHookESPResponse DisHookESP::SendMessage(String url, DiscordWebhookMessageBuilder &builder, String threadName, vector<uint64_t> tagIDs)
{
    if (url.isEmpty())
        return DisHookESPResponse(DisHookESPResponseCode::InvalidParameter, "Webhook URL is empty.");
    if (WiFi.status() != WL_CONNECTED)
        return DisHookESPResponse(DisHookESPResponseCode::WifiNotConnected);
    if (!url.endsWith("wait=true"))
    {
        if (url.indexOf('?') == -1)
            url += "?wait=true";
        else
            url += "&wait=true";
    }
    bool isComponentV2 = builder.IsComponentV2();
    auto components = builder.GetComponents();
    auto embeds = builder.GetEmbeds();
    if (isComponentV2)
    {
        if (components.empty())
            return DisHookESPResponse(DisHookESPResponseCode::InvalidParameter, "Message marked as ComponentV2 but has no components.");
        if (!embeds.empty() || builder.GetContent().has_value())
            return DisHookESPResponse(DisHookESPResponseCode::InvalidParameter, "Message marked as ComponentV2 can only contain components.");
        url += "&with_components=true";
    }
    uint64_t flags = 0;
    if (builder.SuppressesEmbeds())
        flags |= static_cast<uint64_t>(DiscordMessageFlags::SuppressEmbeds);
    if (builder.SuppressesNotifications())
        flags |= static_cast<uint64_t>(DiscordMessageFlags::SuppressNotifications);
    if (isComponentV2)
        flags |= static_cast<uint64_t>(DiscordMessageFlags::IsComponentV2);

    JsonDocument doc;
    doc["flags"] = flags;
    if (!threadName.isEmpty())
    {
        doc["thread_name"] = threadName;
        JsonArray tagIDsArray = doc["applied_tags"].to<JsonArray>();
        for (const uint64_t &tagID : tagIDs)
            tagIDsArray.add(tagID);
    }
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
        for (DiscordComponent &component : components)
            componentsArray.add(component.ToJsonDocument());
    }
    return _sendMessage(url, doc);
}

DisHookESPResponse DisHookESP::SendMessage(String url, String content, String username, String avatarUrl, String threadName, vector<uint64_t> tagIDs)
{
    if (url.isEmpty())
        return DisHookESPResponse(DisHookESPResponseCode::InvalidParameter, "Webhook URL is empty.");
    if (WiFi.status() != WL_CONNECTED)
        return DisHookESPResponse(DisHookESPResponseCode::WifiNotConnected);
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
    if (!url.endsWith("wait=true"))
    {
        if (url.indexOf('?') == -1)
            url += "?wait=true";
        else
            url += "&wait=true";
    }
    return _sendMessage(url, doc);
}

DisHookESPResponse DisHookESP::_sendMessage(String url, JsonDocument doc)
{
    if (!_httpClient.begin(_wifiClient, url))
        return DisHookESPResponse(DisHookESPResponseCode::HttpConnectionFailed);
    _httpClient.addHeader("Content-Type", "application/json");
    const char* keys[] = {"Transfer-Encoding"};
    _httpClient.collectHeaders(keys, 1);
    String jsonString;
    serializeJson(doc, jsonString);
    int httpResponseCode = _httpClient.POST(jsonString);
    if (httpResponseCode < 0)
    {
        _httpClient.end();
        return DisHookESPResponse(static_cast<DisHookESPResponseCode>(httpResponseCode));
    }
    JsonDocument responseDoc;
    Stream& rawStream = _httpClient.getStream();
    StreamUtils::ChunkDecodingStream decodedStream(_httpClient.getStream());
    Stream& response = _httpClient.header("Transfer-Encoding") == "chunked" ? decodedStream : rawStream;
    DeserializationError error = deserializeJson(responseDoc, response);
    _httpClient.end();
    if (error)
        return DisHookESPResponse(DisHookESPResponseCode::JsonDeserializationFailed);
    if (httpResponseCode == 400)
        return DisHookESPResponse(DisHookESPResponseCode::BadRequest, responseDoc);
    if (httpResponseCode == 401)
        return DisHookESPResponse(DisHookESPResponseCode::Unauthorized, responseDoc);
    if (httpResponseCode == 403)
        return DisHookESPResponse(DisHookESPResponseCode::Forbidden, responseDoc);
    if (httpResponseCode == 404)
        return DisHookESPResponse(DisHookESPResponseCode::NotFound, responseDoc);
    if (httpResponseCode == 408)
        return DisHookESPResponse(DisHookESPResponseCode::RequestTimeout, responseDoc);
    if (httpResponseCode == 429)
        return DisHookESPResponse(DisHookESPResponseCode::RateLimitExceeded, responseDoc);
    return DisHookESPResponse(responseDoc);
}

void DisHookESP::SetupClient()
{
#if defined(ESP8266)
    _wifiClient.setTrustAnchors(new BearSSL::X509List(DISCORD_COM_CA));
    _wifiClient.setBufferSizes(1024, 1024);
#elif defined(ESP32)
    _wifiClient.setCACert(DISCORD_COM_CA);
#endif
    _httpClient.setTimeout(5000);
}