#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <optional>
#include "DiscordUnfurledMediaItem.hpp"

class DiscordMediaGalleryItem
{
public:
    DiscordMediaGalleryItem& WithDescription(String description)
    {
        _description = description;
        return *this;
    }

    DiscordMediaGalleryItem& SetSpoiler(bool spoiler)
    {
        _spoiler = spoiler;
        return *this;
    }

    DiscordMediaGalleryItem& WithMedia(DiscordUnfurledMediaItem media)
    {
        _media = media;
        return *this;
    }

    JsonDocument ToJsonDocument() const
    {
        JsonDocument doc;
        if (_description.has_value())
            doc[F("description")] = _description.value();
        doc[F("spoiler")] = _spoiler;
        doc[F("media")] = _media.ToJsonDocument();
        return doc;
    }

private:
    std::optional<String> _description;
    bool _spoiler = false;
    DiscordUnfurledMediaItem _media;
};
