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
    DiscordAllowedMentions &SetType(DiscordMentionType type)
    {
        _type = type;
        return *this;
    }

    DiscordAllowedMentions &AddUserId(String userId)
    {
        if (_userIds.size() < 100)
            _userIds.push_back(userId);
        return *this;
    }

    DiscordAllowedMentions &AddRoleId(String roleId)
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
            doc[F("parse")].add(F("users"));
            break;
        case DiscordMentionType::Roles:
            doc[F("parse")].add(F("roles"));
            break;
        case DiscordMentionType::Everyone:
            doc[F("parse")].add(F("everyone"));
            break;
        case DiscordMentionType::NoMention:
            doc[F("parse")].to<JsonArray>();
            break;
        }
        if (!_userIds.empty())
        {
            JsonArray usersArray = doc[F("users")].to<JsonArray>();
            for (String userId : _userIds)
                usersArray.add(userId);
        }
        if (!_roleIds.empty())
        {
            JsonArray rolesArray = doc[F("roles")].to<JsonArray>();
            for (String roleId : _roleIds)
                rolesArray.add(roleId);
        }
        return doc;
    }

private:
    DiscordMentionType _type = DiscordMentionType::None;
    std::vector<String> _userIds;
    std::vector<String> _roleIds;
};
