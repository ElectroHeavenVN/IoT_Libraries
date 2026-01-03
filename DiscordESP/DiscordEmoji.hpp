#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

class DiscordEmoji
{
public:
    DiscordEmoji() : _name(""), _id(0), _animated(false) {}
    DiscordEmoji(String unicodeEmoji) : _name(unicodeEmoji), _id(0), _animated(false) {}
    DiscordEmoji(String name, uint64_t id, bool animated = false) : _name(name), _id(id), _animated(animated) {}

    void SetName(String name) { _name = name; }
    void SetId(uint64_t id) { _id = id; }
    void SetAnimated(bool animated) { _animated = animated; }

    String GetName() const { return _name; }
    uint64_t GetId() const { return _id; }
    bool IsAnimated() const { return _animated; }

    String ToString() const
    {
        if (_id == 0)
            return _name;
        char buffer[64];
        if (_animated)
            snprintf(buffer, sizeof(buffer), "<a:%s:%llu>", _name.c_str(), _id);
        else
            snprintf(buffer, sizeof(buffer), "<:%s:%llu>", _name.c_str(), _id);
        return String(buffer);
    }

    JsonDocument ToJsonDocument() const
    {
        JsonDocument doc;
        if (_id != 0)
        {
            doc[F("id")] = String(_id);
            doc[F("animated")] = _animated;
        }
        doc[F("name")] = _name;
        return doc;
    }

private:
    String _name;
    uint64_t _id;
    bool _animated;
};