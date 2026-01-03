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
    JsonDocument responseData;
    
    ZaloBotESPResponse(JsonDocument doc) : errorCode(ZaloBotESPResponseCode::Success), responseData(doc) { _parseResult(); }
    ZaloBotESPResponse(ZaloBotESPResponseCode code, JsonDocument doc) : errorCode(code), responseData(doc) { _parseResult(); }
    ZaloBotESPResponse(ZaloBotESPResponseCode code, uint32_t innerErrorCode) : errorCode(code), _innerErrorCode(innerErrorCode) { }
    ZaloBotESPResponse(ZaloBotESPResponseCode code, uint32_t innerErrorCode, JsonDocument doc) : errorCode(code), _innerErrorCode(innerErrorCode), responseData(doc) { _parseResult(); }
    ZaloBotESPResponse(ZaloBotESPResponseCode code, String additionalMessage) : errorCode(code), _additionalErrorMessage(additionalMessage) { }
    ZaloBotESPResponse(ZaloBotESPResponseCode code, String additionalMessage, JsonDocument doc) : errorCode(code), _additionalErrorMessage(additionalMessage), responseData(doc) { _parseResult(); }
    ZaloBotESPResponse(ZaloBotESPResponseCode code) : errorCode(code) { }

    const char* GetLastError()
    {
        if (!_additionalErrorMessage.isEmpty())
            return _additionalErrorMessage.c_str();
        switch (errorCode)
        {
            case ZaloBotESPResponseCode::HttpConnectionFailed:
                return "Connection failed";
            case ZaloBotESPResponseCode::HttpSendHeaderFailed:
                return "Send header failed";
            case ZaloBotESPResponseCode::HttpSendPayloadFailed:
                return "Send payload failed";
            case ZaloBotESPResponseCode::HttpNotConnected:
                return "Not connected";
            case ZaloBotESPResponseCode::HttpConnectionLost:
                return "Connection lost";
            case ZaloBotESPResponseCode::HttpNoRespStream:
                return "No response stream";
            case ZaloBotESPResponseCode::HttpNotAHttpServer:
                return "Not a HTTP server";
            case ZaloBotESPResponseCode::HttpNotEnoughRam:
                return "Not enough RAM";
            case ZaloBotESPResponseCode::HttpTransferEncodingNotSupported:
                return "Transfer-Encoding not supported";
            case ZaloBotESPResponseCode::HttpStreamWriteFailed:
                return "Stream write failed";
            case ZaloBotESPResponseCode::HttpReadTimeout:
                return "Read timeout";
            case ZaloBotESPResponseCode::HttpAsyncCancelled:
                return "Async cancelled";

            case ZaloBotESPResponseCode::Success:
                return "Success";
            case ZaloBotESPResponseCode::InvalidParameter:
            {
                switch (_innerErrorCode)
                {
                    case 1:
                        return "ThreadID is empty";
                    case 2:
                        return "PhotoUrl is empty";
                    case 3:
                        return "StickerID is empty";
                    case 4:
                        return "Action is empty";
                    case 5:
                        return "Content is empty";
                    case 6:
                        return "Content length exceeds 2000 characters";
                }
                return "Invalid parameter";
            }
            case ZaloBotESPResponseCode::WifiNotConnected:
                return "WiFi not connected";
            case ZaloBotESPResponseCode::InvalidResponse:
                return "Invalid response from server";
            case ZaloBotESPResponseCode::JsonDeserializationFailed:
                return "JSON deserialization failed";
            case ZaloBotESPResponseCode::PollingStarted:
                return "Polling started";
            case ZaloBotESPResponseCode::PollingInProgress:
                return "Polling in progress";

            case ZaloBotESPResponseCode::NoContent:
                return "No Content";
            case ZaloBotESPResponseCode::BadRequest:
                return "Bad request";
            case ZaloBotESPResponseCode::Unauthorized:
                return "Unauthorized";
            case ZaloBotESPResponseCode::Forbidden:
                return "Forbidden";
            case ZaloBotESPResponseCode::NotFound:
                return "Not found";
            case ZaloBotESPResponseCode::RequestTimeout:
                return "Request timeout";
            case ZaloBotESPResponseCode::RateLimitExceeded:
                return "Rate limit exceeded";

            default:
                return "Unknown error";
        }
    }
    
private:
    uint32_t _innerErrorCode;
    String _additionalErrorMessage;

    void _parseResult()
    {
        if (responseData.isNull())
            return;
        if (!responseData[F("ok")].as<bool>())
        {
            errorCode = static_cast<ZaloBotESPResponseCode>(responseData[F("error_code")].as<int>());
            String description = responseData[F("description")].as<String>();
            if (_additionalErrorMessage.isEmpty() && !description.isEmpty())
                _additionalErrorMessage = description;
        }
        JsonDocument doc = responseData[F("result")];
        responseData.clear();
        responseData = doc;
    }
};
