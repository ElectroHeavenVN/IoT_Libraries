#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

enum class ESPDiscordBotResponseCode : int
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

struct ESPDiscordBotResponse
{
    ESPDiscordBotResponseCode errorCode;
    String errorMessage = "Success";
    JsonDocument responseData;

    ESPDiscordBotResponse(JsonDocument doc) : errorCode(ESPDiscordBotResponseCode::Success), responseData(doc) {}
    ESPDiscordBotResponse(ESPDiscordBotResponseCode code, JsonDocument doc) : errorCode(code), responseData(doc) { setErrorMessageFromCode(code); }
    ESPDiscordBotResponse(ESPDiscordBotResponseCode code, String message) : errorCode(code), errorMessage(message) {}
    ESPDiscordBotResponse(ESPDiscordBotResponseCode code, String message, JsonDocument doc) : errorCode(code), errorMessage(message), responseData(doc) {}
    ESPDiscordBotResponse(ESPDiscordBotResponseCode code) : errorCode(code) { setErrorMessageFromCode(code); }

    void setErrorMessageFromCode(ESPDiscordBotResponseCode code)
    {
        switch (code)
        {
        case ESPDiscordBotResponseCode::HttpConnectionFailed:
            errorMessage = "Connection failed";
            break;
        case ESPDiscordBotResponseCode::HttpSendHeaderFailed:
            errorMessage = "Send header failed";
            break;
        case ESPDiscordBotResponseCode::HttpSendPayloadFailed:
            errorMessage = "Send payload failed";
            break;
        case ESPDiscordBotResponseCode::HttpNotConnected:
            errorMessage = "Not connected";
            break;
        case ESPDiscordBotResponseCode::HttpConnectionLost:
            errorMessage = "Connection lost";
            break;
        case ESPDiscordBotResponseCode::HttpNoRespStream:
            errorMessage = "No response stream";
            break;
        case ESPDiscordBotResponseCode::HttpNotAHttpServer:
            errorMessage = "Not a HTTP server";
            break;
        case ESPDiscordBotResponseCode::HttpNotEnoughRam:
            errorMessage = "Not enough RAM";
            break;
        case ESPDiscordBotResponseCode::HttpTransferEncodingNotSupported:
            errorMessage = "Transfer-Encoding not supported";
            break;
        case ESPDiscordBotResponseCode::HttpStreamWriteFailed:
            errorMessage = "Stream write failed";
            break;
        case ESPDiscordBotResponseCode::HttpReadTimeout:
            errorMessage = "Read timeout";
            break;

        case ESPDiscordBotResponseCode::Success:
            errorMessage = "Success";
            break;
        case ESPDiscordBotResponseCode::InvalidParameter:
            errorMessage = "Invalid parameter";
            break;
        case ESPDiscordBotResponseCode::WifiNotConnected:
            errorMessage = "WiFi not connected";
            break;
        case ESPDiscordBotResponseCode::InvalidResponse:
            errorMessage = "Invalid response from server";
            break;
        case ESPDiscordBotResponseCode::JsonDeserializationFailed:
            errorMessage = "JSON deserialization failed";
            break;

        case ESPDiscordBotResponseCode::NoContent:
            errorMessage = "No Content";
            break;
        case ESPDiscordBotResponseCode::BadRequest:
            errorMessage = "Bad request";
            break;
        case ESPDiscordBotResponseCode::Unauthorized:
            errorMessage = "Unauthorized";
            break;
        case ESPDiscordBotResponseCode::Forbidden:
            errorMessage = "Forbidden";
            break;
        case ESPDiscordBotResponseCode::NotFound:
            errorMessage = "Not found";
            break;
        case ESPDiscordBotResponseCode::RequestTimeout:
            errorMessage = "Request timeout";
            break;
        case ESPDiscordBotResponseCode::RateLimitExceeded:
            errorMessage = "Rate limit exceeded";
            break;

        default:
            errorMessage = "Unknown error";
            break;
        }
    }
};
