#include "ZaloBotClient.hpp"
#include <StreamUtils.hpp>

#define BASE_ZALO_BOT_API_URL "https://bot-api.zaloplatforms.com/bot"

const char ZALOPLATFORMS_COM_CA[] PROGMEM = R"(
-----BEGIN CERTIFICATE-----
MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI
2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx
1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ
q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz
tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ
vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP
BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV
5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY
1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4
NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG
Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91
8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe
pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl
MrY=
-----END CERTIFICATE-----)";

ZaloBotClient::ZaloBotClient(String botToken) : _botToken(botToken)
{
#if defined(ESP8266)
    _wifiClient.setTrustAnchors(new BearSSL::X509List(ZALOPLATFORMS_COM_CA));
    _wifiClient.setBufferSizes(1024, 1024);
#elif defined(ESP32)
    _wifiClient.setCACert(ZALOPLATFORMS_COM_CA);
#endif
    _httpClient.setTimeout(5000);
    _asyncHttpClient.setTimeout(60000); // DON'T KNOW WHY BUT IT WORKS
}

ZaloBotESPResponse ZaloBotClient::_httpGet(const char* endpoint)
{
	if (WiFi.status() != WL_CONNECTED)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::WifiNotConnected);
    _cancelPolling();
    char url[256];
    snprintf(url, sizeof(url), BASE_ZALO_BOT_API_URL "%s/%s", _botToken.c_str(), endpoint);
    BC_DEBUG("GET %s\n", url);
    if (!_httpClient.begin(_wifiClient, url))
        return ZaloBotESPResponse(ZaloBotESPResponseCode::HttpConnectionFailed);
    int httpResponseCode = _httpClient.GET();
    if (httpResponseCode < 0)
    {
        _httpClient.end();
        BC_DEBUG("GET error code: %d\n", httpResponseCode);
        return ZaloBotESPResponse(static_cast<ZaloBotESPResponseCode>(httpResponseCode));
    }
    if (httpResponseCode == 204)
    {
        _httpClient.end();
        BC_DEBUG("No Content\n");
        return ZaloBotESPResponse(ZaloBotESPResponseCode::NoContent);
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
        return ZaloBotESPResponse(ZaloBotESPResponseCode::JsonDeserializationFailed, String(buffer));
    }
#if defined(DEBUG_BOTCLIENT)
    BC_DEBUG("Response code: %d\n", httpResponseCode);
    serializeJson(responseDoc, DEBUG_BOTCLIENT);
    BC_DEBUG("\n");
#endif
    if (httpResponseCode == 200 || httpResponseCode == 201 || httpResponseCode == 0)
        return ZaloBotESPResponse(responseDoc);
    if (httpResponseCode == 204)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::NoContent);
    if (httpResponseCode == 400)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::BadRequest, responseDoc);
    if (httpResponseCode == 401)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::Unauthorized, responseDoc);
    if (httpResponseCode == 403)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::Forbidden, responseDoc);
    if (httpResponseCode == 404)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::NotFound, responseDoc);
    if (httpResponseCode == 408)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::RequestTimeout, responseDoc);
    if (httpResponseCode == 429)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::RateLimitExceeded, responseDoc);
    return ZaloBotESPResponse(ZaloBotESPResponseCode::UnknownError, responseDoc);
}

ZaloBotESPResponse ZaloBotClient::_httpPost(const char* endpoint, const char *keys[], const char *values[], int count)
{
	if (WiFi.status() != WL_CONNECTED)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::WifiNotConnected);
    _cancelPolling();
    int estimatedSize = 0;
    for (int i = 0; i < count; ++i)
        estimatedSize += strlen(keys[i]) + 1 + strlen(values[i]) * 3 + 1; // {key}={value (may be fully encoded)}&
    char body[estimatedSize + 1] = {0};
    _buildFormBody(body, keys, values, count);
    char url[256];
    snprintf(url, sizeof(url), BASE_ZALO_BOT_API_URL "%s/%s", _botToken.c_str(), endpoint);
    BC_DEBUG("POST %s -> %s\n", url, body);
    if (!_httpClient.begin(_wifiClient, url))
        return ZaloBotESPResponse(ZaloBotESPResponseCode::HttpConnectionFailed);
    _httpClient.addHeader("Content-Type", "application/x-www-form-urlencoded");
    char bodyLength[8];
    snprintf(bodyLength, sizeof(bodyLength), "%d", strlen(body));
    _httpClient.addHeader("Content-Length", bodyLength);
    int httpResponseCode = _httpClient.POST(body);
    
    if (httpResponseCode < 0)
    {
        _httpClient.end();
        BC_DEBUG("GET error code: %d\n", httpResponseCode);
        return ZaloBotESPResponse(static_cast<ZaloBotESPResponseCode>(httpResponseCode));
    }
    if (httpResponseCode == 204)
    {
        _httpClient.end();
        BC_DEBUG("No Content\n");
        return ZaloBotESPResponse(ZaloBotESPResponseCode::NoContent);
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
        return ZaloBotESPResponse(ZaloBotESPResponseCode::JsonDeserializationFailed, String(buffer));
    }
#if defined(DEBUG_BOTCLIENT)
    BC_DEBUG("Response code: %d\n", httpResponseCode);
    serializeJson(responseDoc, DEBUG_BOTCLIENT);
    BC_DEBUG("\n");
#endif
    if (httpResponseCode == 200 || httpResponseCode == 201 || httpResponseCode == 0)
        return ZaloBotESPResponse(responseDoc);
    if (httpResponseCode == 204)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::NoContent);
    if (httpResponseCode == 400)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::BadRequest, responseDoc);
    if (httpResponseCode == 401)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::Unauthorized, responseDoc);
    if (httpResponseCode == 403)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::Forbidden, responseDoc);
    if (httpResponseCode == 404)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::NotFound, responseDoc);
    if (httpResponseCode == 408)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::RequestTimeout, responseDoc);
    if (httpResponseCode == 429)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::RateLimitExceeded, responseDoc);
    return ZaloBotESPResponse(ZaloBotESPResponseCode::UnknownError, responseDoc);
}

// ESP doesn't support multiple simultaneous HTTP connections, so we need to cancel polling before each new request
void ZaloBotClient::_cancelPolling()
{
    if (!_poolingInProgress)
        return;
    _asyncHttpClient.cancelRequestAsync();
    _poolingInProgress = false;
}

void ZaloBotClient::_httpGetAsync(const char* endpoint)
{
	if (WiFi.status() != WL_CONNECTED)
		return;
    char url[256];
    snprintf(url, sizeof(url), BASE_ZALO_BOT_API_URL "%s/%s", _botToken.c_str(), endpoint);
    BC_DEBUG("Async GET: %s\n", url);
    _asyncHttpClient.begin(_wifiClient, url);
    _asyncHttpClient.sendGetAsync();
}

void ZaloBotClient::_buildFormBody(char* body, const char *keys[], const char *values[], int count)
{
    char buf[4];
    for (int i = 0; i < count; ++i)
    {
        for (int j = 0; j < strlen(keys[i]); ++j)
        {
            char c = keys[i][j];
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
                strncat(body, &c, 1);
            else
            {
                snprintf(buf, sizeof(buf), "%%%02X", (unsigned char)c);
                strcat(body, buf);
            }
        }
        strcat(body, "=");
        for (int j = 0; j < strlen(values[i]); ++j)
        {
            char c = values[i][j];
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
                strncat(body, &c, 1);
            else if (c == ' ')
                strcat(body, "+");
            else
            {
                snprintf(buf, sizeof(buf), "%%%02X", (unsigned char)c);
                strcat(body, buf);
            }
        }
        if (i < count - 1)
            strcat(body, "&");
    }
}

ZaloBotESPResponse ZaloBotClient::_httpGetAsyncGetResponse()
{
	if (WiFi.status() != WL_CONNECTED)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::WifiNotConnected);
    int httpResponseCode = _asyncHttpClient.processResponseAsync();
    if (httpResponseCode < 0)
    {
        BC_DEBUG("Async GET process response error code: %d\n", httpResponseCode);
        _asyncHttpClient.end();
        return ZaloBotESPResponse(static_cast<ZaloBotESPResponseCode>(httpResponseCode));
    }
    int responseCode = _asyncHttpClient.asyncResponseCode();
    if (responseCode < 0)
    {
        BC_DEBUG("Async GET error code: %d\n", responseCode);
        _asyncHttpClient.end();
        return ZaloBotESPResponse(static_cast<ZaloBotESPResponseCode>(responseCode));
    }
    JsonDocument responseDoc;
    DeserializationError error;
    if (strcasecmp(_asyncHttpClient.header("Transfer-Encoding").c_str(), "chunked") == 0)
    {
        StreamUtils::ChunkDecodingStream decodedStream(_asyncHttpClient.getStream());
        if (_currentFilter.has_value())
            error = deserializeJson(responseDoc, decodedStream, _currentFilter.value());
        else
            error = deserializeJson(responseDoc, decodedStream);
    }
    else
    {
        if (_currentFilter.has_value())
            error = deserializeJson(responseDoc, _asyncHttpClient.getStringAsync(), _currentFilter.value());
        else
            error = deserializeJson(responseDoc, _asyncHttpClient.getStringAsync());
    }
    _asyncHttpClient.end();
    if (error.code() != 0)
    {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "JSON deserialization failed: %s (%d)", error.c_str(), error.code());
        return ZaloBotESPResponse(ZaloBotESPResponseCode::JsonDeserializationFailed, String(buffer));
    }
#if defined(DEBUG_BOTCLIENT)
    BC_DEBUG("Response code: %d\n", httpResponseCode);
    serializeJson(responseDoc, DEBUG_BOTCLIENT);
    BC_DEBUG("\n");
#endif
    if (httpResponseCode == 200 || httpResponseCode == 201 || httpResponseCode == 0)
        return ZaloBotESPResponse(responseDoc);
    if (httpResponseCode == 204)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::NoContent);
    if (httpResponseCode == 400)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::BadRequest, responseDoc);
    if (httpResponseCode == 401)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::Unauthorized, responseDoc);
    if (httpResponseCode == 403)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::Forbidden, responseDoc);
    if (httpResponseCode == 404)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::NotFound, responseDoc);
    if (httpResponseCode == 408)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::RequestTimeout, responseDoc);
    if (httpResponseCode == 429)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::RateLimitExceeded, responseDoc);
    return ZaloBotESPResponse(ZaloBotESPResponseCode::UnknownError, responseDoc);
}

ZaloBotESPResponse ZaloBotClient::GetPollingUpdates()
{
	if (WiFi.status() != WL_CONNECTED)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::WifiNotConnected);
    if (!_poolingInProgress)
    {
        _httpGetAsync("getUpdates");
        _poolingInProgress = true;
        return ZaloBotESPResponse(ZaloBotESPResponseCode::PollingStarted);
    }
    if (_asyncHttpClient.available() <= 0)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::PollingInProgress);
    _poolingInProgress = false;
    return _httpGetAsyncGetResponse();
}

ZaloBotESPResponse ZaloBotClient::GetMe()
{
    return _httpGet("getMe");
}

ZaloBotESPResponse ZaloBotClient::SendPhoto(const char* threadID, const char* photoUrl, const char* caption)
{
    if (threadID == nullptr || strlen(threadID) == 0)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::InvalidParameter, 1);
    if (photoUrl == nullptr || strlen(photoUrl) == 0)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::InvalidParameter, 2);
    const char *keys[] = {"chat_id", "photo", "caption"};
    const char *values[] = {threadID, photoUrl, caption};
    return _httpPost("sendPhoto", keys, values, 3);
}

ZaloBotESPResponse ZaloBotClient::SendSticker(const char* threadID, const char* stickerID)
{
    if (threadID == nullptr || strlen(threadID) == 0)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::InvalidParameter, 1);
    if (stickerID == nullptr || strlen(stickerID) == 0)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::InvalidParameter, 3);
    const char *keys[] = {"chat_id", "sticker"};
    const char *values[] = {threadID, stickerID};
    return _httpPost("sendSticker", keys, values, 2);
}

ZaloBotESPResponse ZaloBotClient::SendChatAction(const char* threadID, const char* action)
{
    if (threadID == nullptr || strlen(threadID) == 0)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::InvalidParameter, 1);
    if (action == nullptr || strlen(action) == 0)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::InvalidParameter, 4);
    const char *keys[] = {"chat_id", "action"};
    const char *values[] = {threadID, action};
    return _httpPost("sendChatAction", keys, values, 2);
}

ZaloBotESPResponse ZaloBotClient::SendMsg(const char* threadID, const char* content)
{
    if (threadID == nullptr || strlen(threadID) == 0)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::InvalidParameter, 1);
    if (content == nullptr || strlen(content) == 0)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::InvalidParameter, 5);
    if (strlen(content) > 2000)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::InvalidParameter, 6);
    const char *keys[] = {"chat_id", "text"};
    const char *values[] = {threadID, content};
    return _httpPost("sendMessage", keys, values, 2);
}