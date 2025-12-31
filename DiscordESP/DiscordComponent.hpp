#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <optional>
#include <functional>
#include "DiscordEmbed.hpp"
#include "DiscordEmoji.hpp"
#include "DiscordUnfurledMediaItem.hpp"
#include "DiscordMediaGalleryItem.hpp"

using namespace std;

enum class DiscordComponentType : uint32_t
{
    ActionRow = 1,
    Button = 2,
    StringSelect = 3,
    TextInput = 4,
    UserSelect = 5,
    RoleSelect = 6,
    MentionableSelect = 7,
    ChannelSelect = 8,
    Section = 9,
    TextDisplay = 10,
    Thumbnail = 11,
    MediaGallery = 12,
    File = 13,
    Separator = 14,
    Container = 17,
    Label = 18,
    FileUpload = 19
};

enum class DiscordButtonStyle : uint32_t
{
    Primary = 1,
    Secondary = 2,
    Success = 3,
    Danger = 4,
    Link = 5,
    Premium = 6
};

class DiscordComponent
{
public:
    virtual JsonDocument ToJsonDocument() const
    {
        JsonDocument doc;
        doc["type"] = static_cast<uint32_t>(_type);
        if (_id.has_value())
            doc["id"] = _id.value();
        return doc;
    }

    DiscordComponentType GetType() const
    {
        return _type;
    }

    bool IsV2() const
    {
        return _type >= DiscordComponentType::Section && _type < DiscordComponentType::Label;
    }

protected:
    DiscordComponent(DiscordComponentType type) : _type(type) {}
    DiscordComponent(DiscordComponentType type, uint32_t id) : _type(type), _id(id) {}

private:
    DiscordComponentType _type;
    optional<uint32_t> _id;
};

class ButtonComponent : public DiscordComponent
{
public:
    ButtonComponent() : DiscordComponent(DiscordComponentType::Button) {}
    ButtonComponent(uint32_t id) : DiscordComponent(DiscordComponentType::Button, id) {}

    ButtonComponent &WithStyle(DiscordButtonStyle style)
    {
        _style = style;
        return *this;
    }

    ButtonComponent &WithLabel(String &label)
    {
        _label = label;
        return *this;
    }

    ButtonComponent &WithLabel(const char *label)
    {
        _label = String(label);
        return *this;
    }

    ButtonComponent &WithEmoji(DiscordEmoji &emoji)
    {
        _emoji = emoji;
        return *this;
    }

    ButtonComponent &WithCustomId(String &customId)
    {
        _customId = customId;
        return *this;
    }

    ButtonComponent &WithCustomId(const char* customId)
    {
        _customId = String(customId);
        return *this;
    }

    ButtonComponent &WithUrl(String &url)
    {
        _url = url;
        return *this;
    }

    ButtonComponent &WithUrl(const char *url)
    {
        _url = String(url);
        return *this;
    }

    ButtonComponent &SetDisabled(bool disabled)
    {
        _disabled = disabled;
        return *this;
    }

    DiscordButtonStyle GetStyle() const
    {
        return _style;
    }

    JsonDocument ToJsonDocument() const override
    {
        JsonDocument doc = DiscordComponent::ToJsonDocument();
        doc["style"] = static_cast<uint32_t>(_style);
        if (_label.has_value())
            doc["label"] = _label.value();
        if (_emoji.has_value())
            doc["emoji"] = _emoji.value().ToJsonDocument();
        doc["disabled"] = _disabled;
        if (_style == DiscordButtonStyle::Link && _url.has_value())
            doc["url"] = _url.value();
        else
        {
            if (_customId.length() > 0)
                doc["custom_id"] = _customId;
        }
        return doc;
    }

private:
    DiscordButtonStyle _style = DiscordButtonStyle::Primary;
    optional<String> _label;
    optional<DiscordEmoji> _emoji;
    String _customId;
    optional<String> _url;
    bool _disabled = false;
};

// We do not implement select components so action row can only contain buttons
class ActionRowComponent : public DiscordComponent
{
public:
    ActionRowComponent() : DiscordComponent(DiscordComponentType::ActionRow) {}
    ActionRowComponent(uint32_t id) : DiscordComponent(DiscordComponentType::ActionRow, id) {}

    ActionRowComponent &AddComponent(DiscordComponent &component)
    {
        if (_components.size() >= 5 || component.GetType() != DiscordComponentType::Button)
            return *this;
        _components.push_back(component);
        return *this;
    }

    ActionRowComponent &AddComponents(vector<DiscordComponent> &components)
    {
        for (DiscordComponent &component : components)
        {
            if (_components.size() >= 5)
                break;
            if (component.GetType() != DiscordComponentType::Button)
                continue;
            _components.push_back(component);
        }
        return *this;
    }

    ActionRowComponent &ClearComponents()
    {
        _components.clear();
        return *this;
    }

    JsonDocument ToJsonDocument() const override
    {
        JsonDocument doc = DiscordComponent::ToJsonDocument();
        JsonArray componentsArray = doc["components"].to<JsonArray>();
        for (DiscordComponent &component : _components)
            componentsArray.add(component.ToJsonDocument());
        return doc;
    }

private:
    vector<reference_wrapper<DiscordComponent>> _components;
};

// Discord said "Don't hardcode components to contain only text components", so we component-related methods still accept DiscordComponent as parameter
class SectionComponent : public DiscordComponent
{
public:
    SectionComponent() : DiscordComponent(DiscordComponentType::Section) {}
    SectionComponent(uint32_t id) : DiscordComponent(DiscordComponentType::Section, id) {}

    SectionComponent &AddComponent(DiscordComponent &component)
    {
        if (_components.size() >= 3)
            return *this;
        if (component.GetType() != DiscordComponentType::TextDisplay)
            return *this;
        _components.push_back(component);
        return *this;
    }

    SectionComponent &AddComponents(vector<DiscordComponent> &components)
    {
        for (DiscordComponent &component : components)
        {
            if (_components.size() >= 3)
                break;
            if (component.GetType() != DiscordComponentType::TextDisplay)
                continue;
            _components.push_back(component);
        }
        return *this;
    }

    SectionComponent &ClearComponents()
    {
        _components.clear();
        return *this;
    }

    SectionComponent &SetAccessory(DiscordComponent &accessory)
    {
        DiscordComponentType type = accessory.GetType();
        if (type == DiscordComponentType::Button ||
            type == DiscordComponentType::Thumbnail)
        {
            _accessory = accessory;
        }
        return *this;
    }

    JsonDocument ToJsonDocument() const override
    {
        JsonDocument doc = DiscordComponent::ToJsonDocument();
        JsonArray componentsArray = doc["components"].to<JsonArray>();
        for (DiscordComponent &component : _components)
            componentsArray.add(component.ToJsonDocument());
        if (_accessory.has_value())
            doc["accessory"] = _accessory.value().get().ToJsonDocument();
        return doc;
    }

private:
    vector<reference_wrapper<DiscordComponent>> _components;
    optional<reference_wrapper<DiscordComponent>> _accessory;
};

class TextDisplayComponent : public DiscordComponent
{
public:
    TextDisplayComponent() : DiscordComponent(DiscordComponentType::TextDisplay) {}
    TextDisplayComponent(uint32_t id) : DiscordComponent(DiscordComponentType::TextDisplay, id) {}

    TextDisplayComponent &WithContent(String &content)
    {
        _content = content;
        return *this;
    }

    TextDisplayComponent &WithContent(const char *content)
    {
        _content = String(content);
        return *this;
    }

    JsonDocument ToJsonDocument() const override
    {
        JsonDocument doc = DiscordComponent::ToJsonDocument();
        doc["content"] = _content;
        return doc;
    }

private:
    String _content;
};

class ThumbnailComponent : public DiscordComponent
{
public:
    ThumbnailComponent() : DiscordComponent(DiscordComponentType::Thumbnail) {}
    ThumbnailComponent(uint32_t id) : DiscordComponent(DiscordComponentType::Thumbnail, id) {}

    ThumbnailComponent &WithDescription(String &description)
    {
        _description = description;
        return *this;
    }

    ThumbnailComponent &WithDescription(const char *description)
    {
        _description = String(description);
        return *this;
    }

    ThumbnailComponent &SetSpoiler(bool spoiler)
    {
        _spoiler = spoiler;
        return *this;
    }

    ThumbnailComponent &WithMedia(DiscordUnfurledMediaItem &media)
    {
        _media = media;
        return *this;
    }

    JsonDocument ToJsonDocument() const override
    {
        JsonDocument doc = DiscordComponent::ToJsonDocument();
        if (_description.has_value())
            doc["description"] = _description.value();
        if (_spoiler.has_value())
            doc["spoiler"] = _spoiler.value();
        doc["media"] = _media.ToJsonDocument();
        return doc;
    }

private:
    optional<String> _description;
    optional<bool> _spoiler = false;
    DiscordUnfurledMediaItem _media;
};

class MediaGalleryComponent : public DiscordComponent
{
public:
    MediaGalleryComponent() : DiscordComponent(DiscordComponentType::MediaGallery) {}
    MediaGalleryComponent(uint32_t id) : DiscordComponent(DiscordComponentType::MediaGallery, id) {}

    MediaGalleryComponent &AddMediaItem(DiscordMediaGalleryItem &item)
    {
        if (_mediaItems.size() >= 10)
            return *this;
        _mediaItems.push_back(item);
        return *this;
    }

    MediaGalleryComponent &AddMediaItems(vector<DiscordMediaGalleryItem> &items)
    {
        for (DiscordMediaGalleryItem &item : items)
        {
            if (_mediaItems.size() >= 10)
                break;
            _mediaItems.push_back(item);
        }
        return *this;
    }

    MediaGalleryComponent &ClearMediaItems()
    {
        _mediaItems.clear();
        return *this;
    }

    JsonDocument ToJsonDocument() const override
    {
        JsonDocument doc = DiscordComponent::ToJsonDocument();
        JsonArray mediaArray = doc["items"].to<JsonArray>();
        for (auto &item : _mediaItems)
            mediaArray.add(item.ToJsonDocument());
        return doc;
    }

private:
    vector<DiscordMediaGalleryItem> _mediaItems;
};

class FileComponent : public DiscordComponent
{
public:
    FileComponent() : DiscordComponent(DiscordComponentType::File) {}
    FileComponent(uint32_t id) : DiscordComponent(DiscordComponentType::File, id) {}

    FileComponent &WithFile(DiscordUnfurledMediaItem &file)
    {
        _file = file;
        return *this;
    }

    FileComponent &SetSpoiler(bool spoiler)
    {
        _spoiler = spoiler;
        return *this;
    }

    JsonDocument ToJsonDocument() const override
    {
        JsonDocument doc = DiscordComponent::ToJsonDocument();
        doc["file"] = _file.ToJsonDocument();
        if (_spoiler.has_value())
            doc["spoiler"] = _spoiler.value();
        return doc;
    }

private:    
    DiscordUnfurledMediaItem _file;
    optional<bool> _spoiler;
    // String _name;
    // int64_t _size;
};

class SeparatorComponent : public DiscordComponent
{
public:
    SeparatorComponent() : DiscordComponent(DiscordComponentType::Separator) {}
    SeparatorComponent(uint32_t id) : DiscordComponent(DiscordComponentType::Separator, id) {}

    SeparatorComponent &SetDivider(bool divider)
    {
        _divider = divider;
        return *this;
    }

    SeparatorComponent &SetSpacing(uint32_t spacing)
    {
        _spacing = spacing;
        return *this;
    }

    JsonDocument ToJsonDocument() const override
    {
        JsonDocument doc = DiscordComponent::ToJsonDocument();
        if (_divider.has_value())
            doc["divider"] = _divider.value();
        if (_spacing.has_value())
            doc["spacing"] = _spacing.value();
        return doc;
    }

private:
    optional<bool> _divider;
    optional<uint32_t> _spacing;
};

class ContainerComponent : public DiscordComponent
{
public:
    ContainerComponent() : DiscordComponent(DiscordComponentType::Container) {}
    ContainerComponent(uint32_t id) : DiscordComponent(DiscordComponentType::Container, id) {}

    ContainerComponent &AddComponent(DiscordComponent &component)
    {
        DiscordComponentType type = component.GetType();
        if (type == DiscordComponentType::ActionRow ||
            type == DiscordComponentType::TextDisplay ||
            type == DiscordComponentType::Section ||
            type == DiscordComponentType::MediaGallery ||
            type == DiscordComponentType::Separator ||
            type == DiscordComponentType::File)
        {
            _components.push_back(component);
        }
        return *this;
    }

    ContainerComponent &AddComponents(vector<DiscordComponent> &components)
    {
        for (DiscordComponent &component : components)
        {
            DiscordComponentType type = component.GetType();
            if (type == DiscordComponentType::ActionRow ||
                type == DiscordComponentType::TextDisplay ||
                type == DiscordComponentType::Section ||
                type == DiscordComponentType::MediaGallery ||
                type == DiscordComponentType::Separator ||
                type == DiscordComponentType::File)
            {
                _components.push_back(component);
            }
        }
        return *this;
    }

    ContainerComponent &ClearComponents()
    {
        _components.clear();
        return *this;
    }

    ContainerComponent &WithAccentColor(uint32_t accentColor)
    {
        _accentColor = accentColor;
        return *this;
    }

    ContainerComponent &SetSpoiler(bool spoiler)
    {
        _spoiler = spoiler;
        return *this;
    }

    JsonDocument ToJsonDocument() const override
    {
        JsonDocument doc = DiscordComponent::ToJsonDocument();
        JsonArray componentsArray = doc["components"].to<JsonArray>();
        for (DiscordComponent &component : _components)
            componentsArray.add(component.ToJsonDocument());
        if (_accentColor.has_value())
            doc["accent_color"] = _accentColor.value();
        if (_spoiler.has_value())
            doc["spoiler"] = _spoiler.value();
        return doc;
    }

private:
    vector<reference_wrapper<DiscordComponent>> _components;
    optional<uint32_t> _accentColor;
    optional<bool> _spoiler;
};

// TextInput, LabelComponent and FileUpload is only used inside Modal.
// StringSelect, UserSelect, RoleSelect, MentionableSelect, ChannelSelect, FileUpload
// is interactive components.
// We didn't implement websocket (not sure if it even possible on ESP8266/ESP32), 
// so interactive components are not implemented.
