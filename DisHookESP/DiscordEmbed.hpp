#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <optional>

using namespace std;

class DiscordEmbedFooter
{
public:
    DiscordEmbedFooter &WithText(String text)
    {
        _text = text;
        return *this;
    }

    DiscordEmbedFooter &WithIconUrl(String iconUrl)
    {
        _iconUrl = iconUrl;
        return *this;
    }

    DiscordEmbedFooter &WithProxyIconUrl(String proxyIconUrl)
    {
        _proxyIconUrl = proxyIconUrl;
        return *this;
    }

    JsonDocument ToJsonDocument() const
    {
        JsonDocument doc;
        doc["text"] = _text;
        if (_iconUrl.has_value())
            doc["icon_url"] = _iconUrl.value();
        if (_proxyIconUrl.has_value())
            doc["proxy_icon_url"] = _proxyIconUrl.value();
        return doc;
    }

private:
    String _text;
    optional<String> _iconUrl;
    optional<String> _proxyIconUrl;
};

class DiscordEmbedMedia
{
public:
    DiscordEmbedMedia &WithUrl(String url)
    {
        _url = url;
        return *this;
    }

    DiscordEmbedMedia &WithProxyUrl(String proxyUrl)
    {
        _proxyUrl = proxyUrl;
        return *this;
    }

    DiscordEmbedMedia &WithHeight(uint32_t height)
    {
        _height = height;
        return *this;
    }

    DiscordEmbedMedia &WithWidth(uint32_t width)
    {
        _width = width;
        return *this;
    }

    JsonDocument ToJsonDocument() const
    {
        JsonDocument doc;
        doc["url"] = _url;
        if (_proxyUrl.has_value())
            doc["proxy_url"] = _proxyUrl.value();
        if (_height.has_value())
            doc["height"] = _height.value();
        if (_width.has_value())
            doc["width"] = _width.value();
        return doc;
    }

private:
    String _url;
    optional<String> _proxyUrl;
    optional<uint32_t> _height;
    optional<uint32_t> _width;
};

class DiscordEmbedProvider
{
public:
    DiscordEmbedProvider &WithName(String name)
    {
        _name = name;
        return *this;
    }

    DiscordEmbedProvider &WithUrl(String url)
    {
        _url = url;
        return *this;
    }

    JsonDocument ToJsonDocument() const
    {
        JsonDocument doc;
        if (_name.has_value())
            doc["name"] = _name.value();
        if (_url.has_value())
            doc["url"] = _url.value();
        return doc;
    }

private:
    optional<String> _name;
    optional<String> _url;
};

class DiscordEmbedAuthor
{
public:
    DiscordEmbedAuthor &WithName(String name)
    {
        _name = name;
        return *this;
    }

    DiscordEmbedAuthor &WithUrl(String url)
    {
        _url = url;
        return *this;
    }

    DiscordEmbedAuthor &WithIconUrl(String iconUrl)
    {
        _iconUrl = iconUrl;
        return *this;
    }

    DiscordEmbedAuthor &WithProxyIconUrl(String proxyIconUrl)
    {
        _proxyIconUrl = proxyIconUrl;
        return *this;
    }

private:
    String _name;
    optional<String> _url;
    optional<String> _iconUrl;
    optional<String> _proxyIconUrl;
};

class DiscordEmbedField
{
public:
    DiscordEmbedField &WithName(String name)
    {
        _name = name;
        return *this;
    }

    DiscordEmbedField &WithValue(String value)
    {
        _value = value;
        return *this;
    }

    DiscordEmbedField &SetInline(bool inlineField = true)
    {
        _inline = inlineField;
        return *this;
    }

    JsonDocument ToJsonDocument() const
    {
        JsonDocument doc;
        doc["name"] = _name;
        doc["value"] = _value;
        doc["inline"] = _inline;
        return doc;
    }

private:
    String _name;
    String _value;
    bool _inline = false;
};

class DiscordEmbed
{
public:
    DiscordEmbed &WithTitle(String title)
    {
        _title = title;
        return *this;
    }

    DiscordEmbed &WithDescription(String description)
    {
        _description = description;
        return *this;
    }

    DiscordEmbed &WithUrl(String url)
    {
        _url = url;
        return *this;
    }

    DiscordEmbed &WithTimestamp(String timestamp)
    {
        _timestamp = timestamp;
        return *this;
    }

    DiscordEmbed &WithColor(uint32_t color)
    {
        _color = color;
        return *this;
    }

    DiscordEmbed &WithFooter(const DiscordEmbedFooter &footer)
    {
        _footer = footer;
        return *this;
    }

    DiscordEmbed &WithImage(const DiscordEmbedMedia &image)
    {
        _image = image;
        return *this;
    }

    DiscordEmbed &WithThumbnail(const DiscordEmbedMedia &thumbnail)
    {
        _thumbnail = thumbnail;
        return *this;
    }

    DiscordEmbed &WithVideo(const DiscordEmbedMedia &video)
    {
        _video = video;
        return *this;
    }

    DiscordEmbed &WithProvider(const DiscordEmbedProvider &provider)
    {
        _provider = provider;
        return *this;
    }

    DiscordEmbed &WithAuthor(const DiscordEmbedAuthor &author)
    {
        _author = author;
        return *this;
    }

    DiscordEmbed &AddField(const DiscordEmbedField &field)
    {
        if (_fields.size() < 25)
            _fields.push_back(field);
        return *this;
    }

    DiscordEmbed &AddFields(const vector<DiscordEmbedField> &fields)
    {
        for (const auto &field : fields)
        {
            if (_fields.size() >= 25)
                break;
            _fields.push_back(field);
        }
        return *this;
    }

    JsonDocument ToJsonDocument() const
    {
        JsonDocument doc;
        if (_title.has_value())
            doc["title"] = _title.value();
        if (_description.has_value())
            doc["description"] = _description.value();
        if (_url.has_value())
            doc["url"] = _url.value();
        if (_timestamp.has_value())
            doc["timestamp"] = _timestamp.value();
        if (_color.has_value())
            doc["color"] = _color.value();
        if (_footer.has_value())
            doc["footer"] = _footer.value().ToJsonDocument();
        if (_image.has_value())
            doc["image"] = _image.value().ToJsonDocument();
        if (_thumbnail.has_value())
            doc["thumbnail"] = _thumbnail.value().ToJsonDocument();
        if (_video.has_value())
            doc["video"] = _video.value().ToJsonDocument();
        if (_provider.has_value())
            doc["provider"] = _provider.value().ToJsonDocument();
        if (!_fields.empty())
        {
            JsonArray fieldsArray = doc["fields"].to<JsonArray>();
            for (const auto &field : _fields)
                fieldsArray.add(field.ToJsonDocument());
        }
        return doc;
    }

private:
    optional<String> _title;
    optional<String> _description;
    optional<String> _url;
    optional<String> _timestamp;
    optional<uint32_t> _color;
    optional<DiscordEmbedFooter> _footer;
    optional<DiscordEmbedMedia> _image;
    optional<DiscordEmbedMedia> _thumbnail;
    optional<DiscordEmbedMedia> _video;
    optional<DiscordEmbedProvider> _provider;
    optional<DiscordEmbedAuthor> _author;
    vector<DiscordEmbedField> _fields;
};