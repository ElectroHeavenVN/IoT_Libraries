#pragma once

#include <Arduino.h>
#include <vector>
#include <optional>
#include <functional>
#include "DiscordEmbed.hpp"
#include "DiscordComponent.hpp"
#include "DiscordAllowedMentions.hpp"

using namespace std;

//TODO: add message reference, sticker, attachments, etc.

class DiscordMessageBuilder
{
public:
    DiscordMessageBuilder &WithContent(String &content)
    {
        _content = content;
        return *this;
    }

    DiscordMessageBuilder &WithContent(const char* content)
    {
        _content = String(content);
        return *this;
    }

    DiscordMessageBuilder &WithUsername(String &username)
    {
        _username = username;
        return *this;
    }

    DiscordMessageBuilder &WithUsername(const char* username)
    {
        _username = String(username);
        return *this;
    }

    DiscordMessageBuilder &WithAvatarUrl(String &avatarUrl)
    {
        _avatarUrl = avatarUrl;
        return *this;
    }

    DiscordMessageBuilder &WithAvatarUrl(const char* avatarUrl)
    {
        _avatarUrl = String(avatarUrl);
        return *this;
    }

    DiscordMessageBuilder &SetTTS(bool tts)
    {
        _tts = tts;
        return *this;
    }

    DiscordMessageBuilder &SetIsVoiceMessage(bool isVoiceMessage)
    {
        _isVoiceMessage = isVoiceMessage;
        return *this;
    }

    DiscordMessageBuilder &WithAllowedMentions(DiscordAllowedMentions &allowedMentions)
    {
        _allowedMentions = allowedMentions;
        return *this;
    }

    DiscordMessageBuilder &AddComponent(DiscordComponent &component)
    {
        _components.push_back(component);
        return *this;
    }

    DiscordMessageBuilder &AddComponents(vector<reference_wrapper<DiscordComponent>> &components)
    {
        for (DiscordComponent &component : components)
            _components.push_back(component);
        return *this;
    }

    DiscordMessageBuilder &ClearComponents()
    {
        _components.clear();
        return *this;
    }

    DiscordMessageBuilder &AddEmbed(DiscordEmbed &embed)
    {
        _embeds.push_back(embed);
        return *this;
    }

    DiscordMessageBuilder &AddEmbeds(vector<DiscordEmbed> &embeds)
    {
        _embeds.insert(_embeds.end(), embeds.begin(), embeds.end());
        return *this;
    }

    DiscordMessageBuilder &ClearEmbeds()
    {
        _embeds.clear();
        return *this;
    }

    DiscordMessageBuilder &SetSuppressEmbeds(bool suppress = true)
    {
        _suppressEmbeds = suppress;
        return *this;
    }

    DiscordMessageBuilder &SetSuppressNotifications(bool suppress = true)
    {
        _suppressNotifications = suppress;
        return *this;
    }

    bool IsComponentV2()
    {
        for (DiscordComponent &component : _components)
        {
            if (component.IsV2())
                return true;
        }
        return false;
    }

    bool SuppressesEmbeds() const
    {
        return _suppressEmbeds;
    }

    bool SuppressesNotifications() const
    {
        return _suppressNotifications;
    }

    bool IsTTS() const
    {
        return _tts;
    }

    bool IsVoiceMessage() const
    {
        return _isVoiceMessage;
    }

    optional<String> GetContent() const
    {
        return _content;
    }

    optional<String> GetUsername() const
    {
        return _username;
    }

    optional<String> GetAvatarUrl() const
    {
        return _avatarUrl;
    }

    vector<DiscordEmbed> GetEmbeds() const
    {
        return _embeds;
    }

    optional<DiscordAllowedMentions> GetAllowedMentions() const
    {
        return _allowedMentions;
    }

    vector<reference_wrapper<DiscordComponent>> GetComponents() const
    {
        return _components;
    }

private:
    optional<String> _content;
    optional<String> _username;
    optional<String> _avatarUrl;
    bool _tts = false;
    vector<DiscordEmbed> _embeds;
    optional<DiscordAllowedMentions> _allowedMentions;
    vector<reference_wrapper<DiscordComponent>> _components;
    bool _suppressEmbeds = false;
    bool _suppressNotifications = false;
    bool _isVoiceMessage = false;
};