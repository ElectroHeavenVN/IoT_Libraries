#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

enum class DisHookESPResponseCode : int
{
    Success,
    InvalidBuilder,
    WifiNotConnected,
    HttpConnectionFailed,
    InvalidResponse,
    JsonDeserializationFailed,
    BadRequest,
    InvalidWebhook,
    WebhookNotFound,
    RateLimited,
    UnknownError
};

struct DisHookESPResponse
{
    DisHookESPResponseCode errorCode;
    String errorMessage = "Success";
    JsonDocument responseData;

    DisHookESPResponse(JsonDocument doc) : errorCode(DisHookESPResponseCode::Success), responseData(doc) {}
    DisHookESPResponse(DisHookESPResponseCode code, JsonDocument doc) : errorCode(code), responseData(doc) { setErrorMessageFromCode(code); }
    DisHookESPResponse(DisHookESPResponseCode code, String message) : errorCode(code), errorMessage(message) {}
    DisHookESPResponse(DisHookESPResponseCode code, String message, JsonDocument doc) : errorCode(code), errorMessage(message), responseData(doc) {}
    DisHookESPResponse(DisHookESPResponseCode code) : errorCode(code) { setErrorMessageFromCode(code); }

    void setErrorMessageFromCode(DisHookESPResponseCode code)
    {
        switch (code)
        {
        case DisHookESPResponseCode::Success:
            errorMessage = "Success";
            break;
        case DisHookESPResponseCode::InvalidBuilder:
            errorMessage = "Invalid message builder";
            break;
        case DisHookESPResponseCode::WifiNotConnected:
            errorMessage = "WiFi not connected";
            break;
        case DisHookESPResponseCode::HttpConnectionFailed:
            errorMessage = "HTTP connection failed";
            break;
        case DisHookESPResponseCode::InvalidResponse:
            errorMessage = "Invalid response from server";
            break;
        case DisHookESPResponseCode::JsonDeserializationFailed:
            errorMessage = "JSON deserialization failed";
            break;
        case DisHookESPResponseCode::BadRequest:
            errorMessage = "Bad request";
            break;
        case DisHookESPResponseCode::InvalidWebhook:
            errorMessage = "Invalid webhook";
            break;
        case DisHookESPResponseCode::WebhookNotFound:
            errorMessage = "Webhook not found";
            break;
        case DisHookESPResponseCode::RateLimited:
            errorMessage = "Rate limited by server";
            break;
        default:
            errorMessage = "Unknown error";
            break;
        }
    }
};
