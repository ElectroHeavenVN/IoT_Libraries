#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

class DiscordUnfurledMediaItem
{
public:
    DiscordUnfurledMediaItem() : _url("") {}
    DiscordUnfurledMediaItem(String url) : _url(url) {}

    String GetUrl() const { return _url; }
    void SetUrl(String url) { _url = url; }

    JsonDocument ToJsonDocument() const {
        JsonDocument doc;
        doc["url"] = _url;
        //fields ignored in requests, only present in responses
        // if (_proxyUrl.length() > 0)
        //     doc["proxy_url"] = _proxyUrl;
        // if (_width > 0)
        //     doc["width"] = _width;
        // if (_height > 0)
        //     doc["height"] = _height;
        // if (_contentType.length() > 0)
        //     doc["content_type"] = _contentType;
        // if (_attachmentId > 0)
        //     doc["attachment_id"] = _attachmentId;
        return doc;
    }

    static DiscordUnfurledMediaItem FromJson(JsonDocument doc) {
        DiscordUnfurledMediaItem item;
        if (doc["url"].is<String>())
            item._url = doc["url"].as<String>();
        if (doc["proxy_url"].is<String>())
            item._proxyUrl = doc["proxy_url"].as<String>();
        if (doc["width"].is<uint32_t>())
            item._width = doc["width"].as<uint32_t>();
        if (doc["height"].is<uint32_t>())
            item._height = doc["height"].as<uint32_t>();
        if (doc["content_type"].is<String>())
            item._contentType = doc["content_type"].as<String>();
        if (doc["attachment_id"].is<uint32_t>())
            item._attachmentId = doc["attachment_id"].as<uint32_t>();
        return item;
    }

private:
    String _url;
    String _proxyUrl;
    uint32_t _width = 0;
    uint32_t _height = 0;
    String _contentType;
    uint32_t _attachmentId = 0;
};
