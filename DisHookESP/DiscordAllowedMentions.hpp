#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

enum class DiscordMentionType
{
    None,
    NoMention,
    Users,
    Roles,
    Everyone
};

class DiscordAllowedMentions
{
public:
    DiscordAllowedMentions &setType(DiscordMentionType type)
    {
        _type = type;
        return *this;
    }

    DiscordAllowedMentions &addUserId(const String &userId)
    {
        if (_userIds.size() < 100)
            _userIds.push_back(userId);
        return *this;
    }

    DiscordAllowedMentions &addRoleId(const String &roleId)
    {
        if (_roleIds.size() < 100)
            _roleIds.push_back(roleId);
        return *this;
    }

    JsonDocument ToJsonDocument() const
    {
        JsonDocument doc;
        switch (_type)
        {
        case DiscordMentionType::Users:
            doc["parse"].add("users");
            break;
        case DiscordMentionType::Roles:
            doc["parse"].add("roles");
            break;
        case DiscordMentionType::Everyone:
            doc["parse"].add("everyone");
            break;
        case DiscordMentionType::NoMention:
            doc["parse"].to<JsonArray>();
            break;
        }
        if (!_userIds.empty())
        {
            JsonArray usersArray = doc["users"].to<JsonArray>();
            for (const String &userId : _userIds)
                usersArray.add(userId);
        }
        if (!_roleIds.empty())
        {
            JsonArray rolesArray = doc["roles"].to<JsonArray>();
            for (const String &roleId : _roleIds)
                rolesArray.add(roleId);
        }
        return doc;
    }

private:
    DiscordMentionType _type = DiscordMentionType::None;
    std::vector<String> _userIds;
    std::vector<String> _roleIds;
};
