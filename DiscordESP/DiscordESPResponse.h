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
    String errorMessage = "Success";
    JsonDocument responseData;

    DiscordESPResponse(JsonDocument doc) : errorCode(DiscordESPResponseCode::Success), responseData(doc) {}
    DiscordESPResponse(DiscordESPResponseCode code, JsonDocument doc) : errorCode(code), responseData(doc) { setErrorMessageFromCode(code); }
    DiscordESPResponse(DiscordESPResponseCode code, String message) : errorCode(code), errorMessage(message) {}
    DiscordESPResponse(DiscordESPResponseCode code, String message, JsonDocument doc) : errorCode(code), errorMessage(message), responseData(doc) {}
    DiscordESPResponse(DiscordESPResponseCode code) : errorCode(code) { setErrorMessageFromCode(code); }

private:
    void setErrorMessageFromCode(DiscordESPResponseCode code)
    {
        switch (code)
        {
        case DiscordESPResponseCode::HttpConnectionFailed:
            errorMessage = "Connection failed";
            break;
        case DiscordESPResponseCode::HttpSendHeaderFailed:
            errorMessage = "Send header failed";
            break;
        case DiscordESPResponseCode::HttpSendPayloadFailed:
            errorMessage = "Send payload failed";
            break;
        case DiscordESPResponseCode::HttpNotConnected:
            errorMessage = "Not connected";
            break;
        case DiscordESPResponseCode::HttpConnectionLost:
            errorMessage = "Connection lost";
            break;
        case DiscordESPResponseCode::HttpNoRespStream:
            errorMessage = "No response stream";
            break;
        case DiscordESPResponseCode::HttpNotAHttpServer:
            errorMessage = "Not a HTTP server";
            break;
        case DiscordESPResponseCode::HttpNotEnoughRam:
            errorMessage = "Not enough RAM";
            break;
        case DiscordESPResponseCode::HttpTransferEncodingNotSupported:
            errorMessage = "Transfer-Encoding not supported";
            break;
        case DiscordESPResponseCode::HttpStreamWriteFailed:
            errorMessage = "Stream write failed";
            break;
        case DiscordESPResponseCode::HttpReadTimeout:
            errorMessage = "Read timeout";
            break;

        case DiscordESPResponseCode::Success:
            errorMessage = "Success";
            break;
        case DiscordESPResponseCode::InvalidParameter:
            errorMessage = "Invalid parameter";
            break;
        case DiscordESPResponseCode::WifiNotConnected:
            errorMessage = "WiFi not connected";
            break;
        case DiscordESPResponseCode::InvalidResponse:
            errorMessage = "Invalid response from server";
            break;
        case DiscordESPResponseCode::JsonDeserializationFailed:
            errorMessage = "JSON deserialization failed";
            break;

        case DiscordESPResponseCode::NoContent:
            errorMessage = "No Content";
            break;
        case DiscordESPResponseCode::BadRequest:
            errorMessage = "Bad request";
            break;
        case DiscordESPResponseCode::Unauthorized:
            errorMessage = "Unauthorized";
            break;
        case DiscordESPResponseCode::Forbidden:
            errorMessage = "Forbidden";
            break;
        case DiscordESPResponseCode::NotFound:
            errorMessage = "Not found";
            break;
        case DiscordESPResponseCode::RequestTimeout:
            errorMessage = "Request timeout";
            break;
        case DiscordESPResponseCode::RateLimitExceeded:
            errorMessage = "Rate limit exceeded";
            break;

        default:
            errorMessage = "Unknown error";
            break;
        }
    }
};
