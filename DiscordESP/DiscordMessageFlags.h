#pragma once
#include <cstdint>

enum class DiscordMessageFlags : uint64_t
{
    SuppressEmbeds = 1 << 2,
    SuppressNotifications = 1 << 12,
    IsVoiceMessage = 1 << 13,
    IsComponentV2 = 1 << 15
};