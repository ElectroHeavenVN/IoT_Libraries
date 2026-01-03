#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

enum class DiscordESPResponseCode : int
{
    // HTTP client errors
    HttpConnectionFailed = -1,
    HttpSendHeaderFailed = -2,
    HttpSendPayloadFailed = -3,
    HttpNotConnected = -4,
    HttpConnectionLost = -5,
    HttpNoRespStream = -6,
    HttpNotAHttpServer = -7,
    HttpNotEnoughRam = -8,
    HttpTransferEncodingNotSupported = -9,
    HttpStreamWriteFailed = -10,
    HttpReadTimeout = -11,

    Success = 0,
    InvalidParameter,
    WifiNotConnected,
    InvalidResponse,
    JsonDeserializationFailed,

    // HTTP status codes
    NoContent = 204,
    BadRequest = 400,
    Unauthorized = 401,
    Forbidden = 403,
    NotFound = 404,
    RequestTimeout = 408,
    RateLimitExceeded = 429,

    UnknownError = 1000
};

struct DiscordESPResponse
{
public:
    DiscordESPResponseCode errorCode;
    JsonDocument responseData;
    
    DiscordESPResponse(JsonDocument doc) : errorCode(DiscordESPResponseCode::Success), responseData(doc) { }
    DiscordESPResponse(DiscordESPResponseCode code, JsonDocument doc) : errorCode(code), responseData(doc) { }
    DiscordESPResponse(DiscordESPResponseCode code, uint32_t innerErrorCode) : errorCode(code), _innerErrorCode(innerErrorCode) { }
    DiscordESPResponse(DiscordESPResponseCode code, uint32_t innerErrorCode, JsonDocument doc) : errorCode(code), _innerErrorCode(innerErrorCode), responseData(doc) { }
    DiscordESPResponse(DiscordESPResponseCode code, String additionalMessage) : errorCode(code), _additionalErrorMessage(additionalMessage) { }
    DiscordESPResponse(DiscordESPResponseCode code, String additionalMessage, JsonDocument doc) : errorCode(code), _additionalErrorMessage(additionalMessage), responseData(doc) { }
    DiscordESPResponse(DiscordESPResponseCode code) : errorCode(code) { }
    
    const char* GetLastError()
    {
        if (!_additionalErrorMessage.isEmpty())
            return _additionalErrorMessage.c_str();
        switch (errorCode)
        {
            case DiscordESPResponseCode::HttpConnectionFailed:
                return "Connection failed";
            case DiscordESPResponseCode::HttpSendHeaderFailed:
                return "Send header failed";
            case DiscordESPResponseCode::HttpSendPayloadFailed:
                return "Send payload failed";
            case DiscordESPResponseCode::HttpNotConnected:
                return "Not connected";
            case DiscordESPResponseCode::HttpConnectionLost:
                return "Connection lost";
            case DiscordESPResponseCode::HttpNoRespStream:
                return "No response stream";
            case DiscordESPResponseCode::HttpNotAHttpServer:
                return "Not a HTTP server";
            case DiscordESPResponseCode::HttpNotEnoughRam:
                return "Not enough RAM";
            case DiscordESPResponseCode::HttpTransferEncodingNotSupported:
                return "Transfer-Encoding not supported";
            case DiscordESPResponseCode::HttpStreamWriteFailed:
                return "Stream write failed";
            case DiscordESPResponseCode::HttpReadTimeout:
                return "Read timeout";

            case DiscordESPResponseCode::Success:
                return "Success";
            case DiscordESPResponseCode::InvalidParameter:
            {
                switch (_innerErrorCode)
                {
                    case 1:
                        return "Token is empty";
                    case 2:
                        return "Channel ID is empty";
                    case 3:
                        return "Content is empty";
                    case 4:
                        return "Content exceeds 2000 characters";
                    case 5:
                        return "Message marked as ComponentV2 but has no components";
                    case 6:
                        return "Message marked as ComponentV2 can only contain components";
                    case 7:
                        return "Cannot have more than 10 embeds in a message";
                    case 8:
                        return "Message ID is empty";
                    case 9:
                        return "Emoji is empty";
                    case 10:
                        return "Limit must be between 1 and 100";
                    case 11:
                        return "Only one of around, before, or after can be specified";
                    case 12:
                        return "Webhook URL is empty";
                }
                return "Invalid parameter";
            }
            case DiscordESPResponseCode::WifiNotConnected:
                return "WiFi not connected";
            case DiscordESPResponseCode::InvalidResponse:
                return "Invalid response from server";
            case DiscordESPResponseCode::JsonDeserializationFailed:
                return "JSON deserialization failed";

            case DiscordESPResponseCode::NoContent:
                return "No Content";
            case DiscordESPResponseCode::BadRequest:
                return "Bad request";
            case DiscordESPResponseCode::Unauthorized:
                return "Unauthorized";
            case DiscordESPResponseCode::Forbidden:
                return "Forbidden";
            case DiscordESPResponseCode::NotFound:
                return "Not found";
            case DiscordESPResponseCode::RequestTimeout:
                return "Request timeout";
            case DiscordESPResponseCode::RateLimitExceeded:
                return "Rate limit exceeded";

            default:
                return "Unknown error";
        }
    }

private:
    uint32_t _innerErrorCode;
    String _additionalErrorMessage;
};
