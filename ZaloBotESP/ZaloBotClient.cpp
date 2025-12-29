#include "ZaloBotClient.hpp"
#include <ArduinoJson.h>

const char ZAPPS_ME_CA[] PROGMEM = R"(
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

ZaloBotClient::ZaloBotClient(const String &botToken) : _botToken(botToken)
{
#if defined(ESP8266)
    _wifiClient.setTrustAnchors(new BearSSL::X509List(ZAPPS_ME_CA));
    _wifiClient.setBufferSizes(1024, 1024);
#elif defined(ESP32)
    _wifiClient.setCACert(ZAPPS_ME_CA);
#endif
    _httpClient.setTimeout(5000);
    _asyncHttpClient.setTimeout(60000); // DON'T KNOW WHY BUT IT WORKS
}

ZaloBotESPResponse ZaloBotClient::_httpGet(const String &endpoint)
{
	if (WiFi.status() != WL_CONNECTED)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::WifiNotConnected);
    _cancelPolling();
    String url = "https://bot-api.zapps.me/bot" + _botToken + "/" + endpoint;
    String json = "";
    BC_DEBUG_LN("GET " + url);
    if (!_httpClient.begin(_wifiClient, url))
        return ZaloBotESPResponse(ZaloBotESPResponseCode::HttpConnectionFailed);
    int httpCode = _httpClient.GET();
    if (httpCode > 0)
        json = _httpClient.getString();
    else
        BC_DEBUG("GET error code: %d\n", httpCode);
    _httpClient.end();
    BC_DEBUG_LN("Response: " + json);
    if (json.length() <= 0)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::EmptyResponse);
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    if (error)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::JsonDeserializationFailed);
    return ZaloBotESPResponse(doc);
}

ZaloBotESPResponse ZaloBotClient::_httpPost(const String &endpoint, const char *keys[], const char *values[], int count)
{
	if (WiFi.status() != WL_CONNECTED)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::WifiNotConnected);
    _cancelPolling();
    String body = _buildFormBody(keys, values, count);
    String url = "https://bot-api.zapps.me/bot" + _botToken + "/" + endpoint;
    String json = "";
    BC_DEBUG_LN("POST " + url + " -> " + body);
    _httpClient.begin(_wifiClient, url);
    _httpClient.addHeader("Content-Type", "application/x-www-form-urlencoded");
    _httpClient.addHeader("Content-Length", String(body.length()));
    int httpCode = _httpClient.POST(body);
    if (httpCode > 0)
        json = _httpClient.getString();
    else
        BC_DEBUG("POST error code: %d\n", httpCode);
    _httpClient.end();
    BC_DEBUG_LN("Response: " + json);
    if (json.length() <= 0)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::EmptyResponse);
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    if (error)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::JsonDeserializationFailed);
    return ZaloBotESPResponse(doc);
}

// ESP doesn't support multiple simultaneous HTTP connections, so we need to cancel polling before each new request
void ZaloBotClient::_cancelPolling()
{
    if (_poolingInProgress)
    {
        _asyncHttpClient.cancelRequestAsync();
        _poolingInProgress = false;
    }
}

void ZaloBotClient::_httpGetAsync(const String &endpoint)
{
	if (WiFi.status() != WL_CONNECTED)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::WifiNotConnected);
    String url = "https://bot-api.zapps.me/bot" + _botToken + "/" + endpoint;
    BC_DEBUG_LN("Async GET: " + url);
    _asyncHttpClient.begin(_wifiClient, url);
    _asyncHttpClient.sendGetAsync();
}

String ZaloBotClient::_buildFormBody(const char *keys[], const char *values[], int count)
{
    String body = "";
    for (int i = 0; i < count; ++i)
    {
        for (int j = 0; j < strlen(keys[i]); ++j)
        {
            char c = keys[i][j];
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
                body += c;
            else
            {
                char buf[4];
                snprintf(buf, sizeof(buf), "%%%02X", (unsigned char)c);
                body += buf;
            }
        }
        body += "=";
        for (int j = 0; j < strlen(values[i]); ++j)
        {
            char c = values[i][j];
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
                body += c;
            else if (c == ' ')
                body += "+";
            else
            {
                char buf[4];
                snprintf(buf, sizeof(buf), "%%%02X", (unsigned char)c);
                body += buf;
            }
        }
        if (i < count - 1)
            body += "&";
    }
    return body;
}

ZaloBotESPResponse ZaloBotClient::_httpGetAsyncGetResponse()
{
	if (WiFi.status() != WL_CONNECTED)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::WifiNotConnected);
    int httpCode = _asyncHttpClient.processResponseAsync();
    if (httpCode < 0)
    {
        BC_DEBUG("Async GET process response error code: %d\n", httpCode);
        _asyncHttpClient.end();
        return ZaloBotESPResponse(static_cast<ZaloBotESPResponseCode>(httpCode));
    }
    String json = _asyncHttpClient.getStringAsync();
    int responseCode = _asyncHttpClient.asyncResponseCode();
    if (responseCode < 0)
    {
        BC_DEBUG("Async GET error code: %d\n", responseCode);
        _asyncHttpClient.end();
        return ZaloBotESPResponse(static_cast<ZaloBotESPResponseCode>(responseCode));
    }
    _asyncHttpClient.end();
    BC_DEBUG_LN("Async response: " + json);
    if (json.length() <= 0)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::EmptyResponse);
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    if (error)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::JsonDeserializationFailed);
    return ZaloBotESPResponse(doc);
}

ZaloBotESPResponse ZaloBotClient::GetPollingUpdates()
{
    if (!_poolingInProgress)
    {
        _httpGetAsync("getUpdates");
        _poolingInProgress = true;
        return ZaloBotESPResponse(ZaloBotESPResponseCode::PollingStarted);
    }
    BC_DEBUG(".");
    if (_asyncHttpClient.available() <= 0)
        return ZaloBotESPResponse(ZaloBotESPResponseCode::PollingInProgress);
    _poolingInProgress = false;
    return _httpGetAsyncGetResponse();
}

ZaloBotESPResponse ZaloBotClient::GetMe()
{
    return _httpGet("getMe");
}

ZaloBotESPResponse ZaloBotClient::SendPhoto(const String &threadID, const String &photoUrl, const String &caption)
{
    const char *keys[] = {"chat_id", "photo", "caption"};
    const char *values[] = {threadID.c_str(), photoUrl.c_str(), caption.c_str()};
    return _httpPost("sendPhoto", keys, values, 3);
}

ZaloBotESPResponse ZaloBotClient::SendSticker(const String &threadID, const String &stickerID)
{
    const char *keys[] = {"chat_id", "sticker"};
    const char *values[] = {threadID.c_str(), stickerID.c_str()};
    return _httpPost("sendSticker", keys, values, 2);
}

ZaloBotESPResponse ZaloBotClient::SendChatAction(const String &threadID, const String &action)
{
    const char *keys[] = {"chat_id", "action"};
    const char *values[] = {threadID.c_str(), action.c_str()};
    return _httpPost("sendChatAction", keys, values, 2);
}

ZaloBotESPResponse ZaloBotClient::SendMsg(const String &threadID, const String &content)
{
    const char *keys[] = {"chat_id", "text"};
    const char *values[] = {threadID.c_str(), content.c_str()};
    return _httpPost("sendMessage", keys, values, 2);
}