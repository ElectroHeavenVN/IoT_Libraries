#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

enum class ZaloBotESPResponseCode : int
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
    HttpAsyncCancelled = -12,

    Success = 0,
    InvalidParameter,
    WifiNotConnected,
    InvalidResponse,
    JsonDeserializationFailed,
    PollingStarted,
    PollingInProgress,

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

struct ZaloBotESPResponse
{
    ZaloBotESPResponseCode errorCode;
    String errorMessage = "Success";
    JsonDocument responseData;

    ZaloBotESPResponse(JsonDocument doc) : errorCode(ZaloBotESPResponseCode::Success), responseData(doc) {}
    ZaloBotESPResponse(ZaloBotESPResponseCode code, JsonDocument doc) : errorCode(code), responseData(doc) { setErrorMessageFromCode(code); }
    ZaloBotESPResponse(ZaloBotESPResponseCode code, String message) : errorCode(code), errorMessage(message) {}
    ZaloBotESPResponse(ZaloBotESPResponseCode code, String message, JsonDocument doc) : errorCode(code), errorMessage(message), responseData(doc) {}
    ZaloBotESPResponse(ZaloBotESPResponseCode code) : errorCode(code) { setErrorMessageFromCode(code); }

    void setErrorMessageFromCode(ZaloBotESPResponseCode code)
    {
        switch (code)
        {
        case ZaloBotESPResponseCode::HttpConnectionFailed:
            errorMessage = "Connection failed";
            break;
        case ZaloBotESPResponseCode::HttpSendHeaderFailed:
            errorMessage = "Send header failed";
            break;
        case ZaloBotESPResponseCode::HttpSendPayloadFailed:
            errorMessage = "Send payload failed";
            break;
        case ZaloBotESPResponseCode::HttpNotConnected:
            errorMessage = "Not connected";
            break;
        case ZaloBotESPResponseCode::HttpConnectionLost:
            errorMessage = "Connection lost";
            break;
        case ZaloBotESPResponseCode::HttpNoRespStream:
            errorMessage = "No response stream";
            break;
        case ZaloBotESPResponseCode::HttpNotAHttpServer:
            errorMessage = "Not a HTTP server";
            break;
        case ZaloBotESPResponseCode::HttpNotEnoughRam:
            errorMessage = "Not enough RAM";
            break;
        case ZaloBotESPResponseCode::HttpTransferEncodingNotSupported:
            errorMessage = "Transfer-Encoding not supported";
            break;
        case ZaloBotESPResponseCode::HttpStreamWriteFailed:
            errorMessage = "Stream write failed";
            break;
        case ZaloBotESPResponseCode::HttpReadTimeout:
            errorMessage = "Read timeout";
            break;
        case ZaloBotESPResponseCode::HttpAsyncCancelled:
            errorMessage = "Async cancelled";
            break;

        case ZaloBotESPResponseCode::Success:
        {
            if (!responseData.isNull())
            {
                if (!responseData["ok"].as<bool>())
                {
                    errorCode = static_cast<ZaloBotESPResponseCode>(responseData["error_code"].as<int>());
                    errorMessage = responseData["description"].as<String>();
                }
                responseData = responseData["result"];
                break;
            }
            errorMessage = "Success";
            break;
        }
        case ZaloBotESPResponseCode::InvalidParameter:
            errorMessage = "Invalid parameter";
            break;
        case ZaloBotESPResponseCode::WifiNotConnected:
            errorMessage = "WiFi not connected";
            break;
        case ZaloBotESPResponseCode::InvalidResponse:
            errorMessage = "Invalid response from server";
            break;
        case ZaloBotESPResponseCode::JsonDeserializationFailed:
            errorMessage = "JSON deserialization failed";
            break;
        case ZaloBotESPResponseCode::PollingStarted:
            errorMessage = "Polling started";
            break;
        case ZaloBotESPResponseCode::PollingInProgress:
            errorMessage = "Polling in progress";
            break;

        case ZaloBotESPResponseCode::NoContent:
            errorMessage = "No Content";
            break;
        case ZaloBotESPResponseCode::BadRequest:
            errorMessage = "Bad request";
            break;
        case ZaloBotESPResponseCode::Unauthorized:
            errorMessage = "Unauthorized";
            break;
        case ZaloBotESPResponseCode::Forbidden:
            errorMessage = "Forbidden";
            break;
        case ZaloBotESPResponseCode::NotFound:
            errorMessage = "Not found";
            break;
        case ZaloBotESPResponseCode::RequestTimeout:
            errorMessage = "Request timeout";
            break;
        case ZaloBotESPResponseCode::RateLimitExceeded:
            errorMessage = "Rate limit exceeded";
            break;

        default:
            errorMessage = "Unknown error";
            break;
        }
    }
};
