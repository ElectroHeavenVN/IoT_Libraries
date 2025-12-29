#pragma once

#include <Arduino.h>
#include <vector>
#include <optional>
#include <functional>
#include "DiscordEmbed.hpp"
#include "DiscordComponent.hpp"
#include "DiscordAllowedMentions.hpp"

using namespace std;

class DiscordWebhookMessageBuilder
{
public:
    DiscordWebhookMessageBuilder &WithContent(String &content)
    {
        _content = content;
        return *this;
    }

    DiscordWebhookMessageBuilder &WithContent(const char* content)
    {
        _content = String(content);
        return *this;
    }

    DiscordWebhookMessageBuilder &WithUsername(String &username)
    {
        _username = username;
        return *this;
    }

    DiscordWebhookMessageBuilder &WithUsername(const char* username)
    {
        _username = String(username);
        return *this;
    }

    DiscordWebhookMessageBuilder &WithAvatarUrl(String &avatarUrl)
    {
        _avatarUrl = avatarUrl;
        return *this;
    }

    DiscordWebhookMessageBuilder &WithAvatarUrl(const char* avatarUrl)
    {
        _avatarUrl = String(avatarUrl);
        return *this;
    }

    DiscordWebhookMessageBuilder &SetTTS(bool tts)
    {
        _tts = tts;
        return *this;
    }

    DiscordWebhookMessageBuilder &WithAllowedMentions(DiscordAllowedMentions &allowedMentions)
    {
        _allowedMentions = allowedMentions;
        return *this;
    }

    DiscordWebhookMessageBuilder &AddComponent(DiscordComponent &component)
    {
        _components.push_back(component);
        return *this;
    }

    DiscordWebhookMessageBuilder &AddComponents(vector<reference_wrapper<DiscordComponent>> &components)
    {
        for (DiscordComponent &component : components)
            _components.push_back(component);
        return *this;
    }

    DiscordWebhookMessageBuilder &ClearComponents()
    {
        _components.clear();
        return *this;
    }

    DiscordWebhookMessageBuilder &AddEmbed(DiscordEmbed &embed)
    {
        _embeds.push_back(embed);
        return *this;
    }

    DiscordWebhookMessageBuilder &AddEmbeds(vector<DiscordEmbed> &embeds)
    {
        _embeds.insert(_embeds.end(), embeds.begin(), embeds.end());
        return *this;
    }

    DiscordWebhookMessageBuilder &ClearEmbeds()
    {
        _embeds.clear();
        return *this;
    }

    DiscordWebhookMessageBuilder &SetSuppressEmbeds(bool suppress = true)
    {
        _suppressEmbeds = suppress;
        return *this;
    }

    DiscordWebhookMessageBuilder &SetSuppressNotifications(bool suppress = true)
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
};