/**
 * ESP8266HTTPClient.h
 *
 * Created on: 02.11.2015
 *
 * Copyright (c) 2015 Markus Sattler. All rights reserved.
 * This file is part of the ESP8266HTTPClient for Arduino.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Modified by Jeroen Döll, June 2018
 */

#ifndef ESP8266HTTPClientMod_H_
#define ESP8266HTTPClientMod_H_

#if defined(ESP8266)

#include <Arduino.h>
#include <StreamString.h>
#include <WiFiClient.h>

#include <memory>

#ifdef DEBUG_ESP_HTTP_CLIENT
#ifdef DEBUG_ESP_PORT
#define DEBUG_HTTPCLIENT(fmt, ...) DEBUG_ESP_PORT.printf_P( (PGM_P)PSTR(fmt), ## __VA_ARGS__ )
#endif
#endif

#ifndef DEBUG_HTTPCLIENT
#define DEBUG_HTTPCLIENT(...) do { (void)0; } while (0)
#endif

#define HTTPCLIENTMOD_DEFAULT_TCP_TIMEOUT (5000)

/// HTTP client errors
#define HTTPCLIENTMOD_HTTPC_ERROR_CONNECTION_FAILED   (-1)
#define HTTPCLIENTMOD_HTTPC_ERROR_SEND_HEADER_FAILED  (-2)
#define HTTPCLIENTMOD_HTTPC_ERROR_SEND_PAYLOAD_FAILED (-3)
#define HTTPCLIENTMOD_HTTPC_ERROR_NOT_CONNECTED       (-4)
#define HTTPCLIENTMOD_HTTPC_ERROR_CONNECTION_LOST     (-5)
#define HTTPCLIENTMOD_HTTPC_ERROR_NO_STREAM           (-6)
#define HTTPCLIENTMOD_HTTPC_ERROR_NO_HTTP_SERVER      (-7)
#define HTTPCLIENTMOD_HTTPC_ERROR_TOO_LESS_RAM        (-8)
#define HTTPCLIENTMOD_HTTPC_ERROR_ENCODING            (-9)
#define HTTPCLIENTMOD_HTTPC_ERROR_STREAM_WRITE        (-10)
#define HTTPCLIENTMOD_HTTPC_ERROR_READ_TIMEOUT        (-11)
#define HTTPCLIENTMOD_HTTPC_ASYNC_ERROR_CANCELLED     (-12)

constexpr int HTTPCLIENTMOD_HTTPC_ERROR_CONNECTION_REFUSED __attribute__((deprecated)) = HTTPCLIENTMOD_HTTPC_ERROR_CONNECTION_FAILED;

/// size for the stream handling
#define HTTPCLIENTMOD_TCP_BUFFER_SIZE (1460)

/// HTTP codes see RFC7231
typedef enum {
    HTTPCLIENTMOD_HTTP_CODE_CONTINUE = 100,
    HTTPCLIENTMOD_HTTP_CODE_SWITCHING_PROTOCOLS = 101,
    HTTPCLIENTMOD_HTTP_CODE_PROCESSING = 102,
    HTTPCLIENTMOD_HTTP_CODE_OK = 200,
    HTTPCLIENTMOD_HTTP_CODE_CREATED = 201,
    HTTPCLIENTMOD_HTTP_CODE_ACCEPTED = 202,
    HTTPCLIENTMOD_HTTP_CODE_NON_AUTHORITATIVE_INFORMATION = 203,
    HTTPCLIENTMOD_HTTP_CODE_NO_CONTENT = 204,
    HTTPCLIENTMOD_HTTP_CODE_RESET_CONTENT = 205,
    HTTPCLIENTMOD_HTTP_CODE_PARTIAL_CONTENT = 206,
    HTTPCLIENTMOD_HTTP_CODE_MULTI_STATUS = 207,
    HTTPCLIENTMOD_HTTP_CODE_ALREADY_REPORTED = 208,
    HTTPCLIENTMOD_HTTP_CODE_IM_USED = 226,
    HTTPCLIENTMOD_HTTP_CODE_MULTIPLE_CHOICES = 300,
    HTTPCLIENTMOD_HTTP_CODE_MOVED_PERMANENTLY = 301,
    HTTPCLIENTMOD_HTTP_CODE_FOUND = 302,
    HTTPCLIENTMOD_HTTP_CODE_SEE_OTHER = 303,
    HTTPCLIENTMOD_HTTP_CODE_NOT_MODIFIED = 304,
    HTTPCLIENTMOD_HTTP_CODE_USE_PROXY = 305,
    HTTPCLIENTMOD_HTTP_CODE_TEMPORARY_REDIRECT = 307,
    HTTPCLIENTMOD_HTTP_CODE_PERMANENT_REDIRECT = 308,
    HTTPCLIENTMOD_HTTP_CODE_BAD_REQUEST = 400,
    HTTPCLIENTMOD_HTTP_CODE_UNAUTHORIZED = 401,
    HTTPCLIENTMOD_HTTP_CODE_PAYMENT_REQUIRED = 402,
    HTTPCLIENTMOD_HTTP_CODE_FORBIDDEN = 403,
    HTTPCLIENTMOD_HTTP_CODE_NOT_FOUND = 404,
    HTTPCLIENTMOD_HTTP_CODE_METHOD_NOT_ALLOWED = 405,
    HTTPCLIENTMOD_HTTP_CODE_NOT_ACCEPTABLE = 406,
    HTTPCLIENTMOD_HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED = 407,
    HTTPCLIENTMOD_HTTP_CODE_REQUEST_TIMEOUT = 408,
    HTTPCLIENTMOD_HTTP_CODE_CONFLICT = 409,
    HTTPCLIENTMOD_HTTP_CODE_GONE = 410,
    HTTPCLIENTMOD_HTTP_CODE_LENGTH_REQUIRED = 411,
    HTTPCLIENTMOD_HTTP_CODE_PRECONDITION_FAILED = 412,
    HTTPCLIENTMOD_HTTP_CODE_PAYLOAD_TOO_LARGE = 413,
    HTTPCLIENTMOD_HTTP_CODE_URI_TOO_LONG = 414,
    HTTPCLIENTMOD_HTTP_CODE_UNSUPPORTED_MEDIA_TYPE = 415,
    HTTPCLIENTMOD_HTTP_CODE_RANGE_NOT_SATISFIABLE = 416,
    HTTPCLIENTMOD_HTTP_CODE_EXPECTATION_FAILED = 417,
    HTTPCLIENTMOD_HTTP_CODE_MISDIRECTED_REQUEST = 421,
    HTTPCLIENTMOD_HTTP_CODE_UNPROCESSABLE_ENTITY = 422,
    HTTPCLIENTMOD_HTTP_CODE_LOCKED = 423,
    HTTPCLIENTMOD_HTTP_CODE_FAILED_DEPENDENCY = 424,
    HTTPCLIENTMOD_HTTP_CODE_UPGRADE_REQUIRED = 426,
    HTTPCLIENTMOD_HTTP_CODE_PRECONDITION_REQUIRED = 428,
    HTTPCLIENTMOD_HTTP_CODE_TOO_MANY_REQUESTS = 429,
    HTTPCLIENTMOD_HTTP_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    HTTPCLIENTMOD_HTTP_CODE_INTERNAL_SERVER_ERROR = 500,
    HTTPCLIENTMOD_HTTP_CODE_NOT_IMPLEMENTED = 501,
    HTTPCLIENTMOD_HTTP_CODE_BAD_GATEWAY = 502,
    HTTPCLIENTMOD_HTTP_CODE_SERVICE_UNAVAILABLE = 503,
    HTTPCLIENTMOD_HTTP_CODE_GATEWAY_TIMEOUT = 504,
    HTTPCLIENTMOD_HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED = 505,
    HTTPCLIENTMOD_HTTP_CODE_VARIANT_ALSO_NEGOTIATES = 506,
    HTTPCLIENTMOD_HTTP_CODE_INSUFFICIENT_STORAGE = 507,
    HTTPCLIENTMOD_HTTP_CODE_LOOP_DETECTED = 508,
    HTTPCLIENTMOD_HTTP_CODE_NOT_EXTENDED = 510,
    HTTPCLIENTMOD_HTTP_CODE_NETWORK_AUTHENTICATION_REQUIRED = 511
} httpclientmod_t_http_codes;

typedef enum {
    HTTPCLIENTMOD_HTTPC_TE_IDENTITY,
    HTTPCLIENTMOD_HTTPC_TE_CHUNKED
} httpclientmod_transferEncoding_t;

/**
 * redirection follow mode.
 * + `HTTPCLIENTMOD_HTTPC_DISABLE_FOLLOW_REDIRECTS` - no redirection will be followed.
 * + `HTTPCLIENTMOD_HTTPC_STRICT_FOLLOW_REDIRECTS` - strict RFC2616, only requests using
 *      GET or HEAD methods will be redirected (using the same method),
 *      since the RFC requires end-user confirmation in other cases.
 * + `HTTPCLIENTMOD_HTTPC_FORCE_FOLLOW_REDIRECTS` - all redirections will be followed,
 *      regardless of a used method. New request will use the same method,
 *      and they will include the same body data and the same headers.
 *      In the sense of the RFC, it's just like every redirection is confirmed.
 */
typedef enum {
    HTTPCLIENTMOD_HTTPC_DISABLE_FOLLOW_REDIRECTS,
    HTTPCLIENTMOD_HTTPC_STRICT_FOLLOW_REDIRECTS,
    HTTPCLIENTMOD_HTTPC_FORCE_FOLLOW_REDIRECTS
} httpclientmod_followRedirects_t;

class HttpClientMod_TransportTraits;
typedef std::unique_ptr<HttpClientMod_TransportTraits> HttpClientMod_TransportTraitsPtr;

class HTTPClientMod
{
public:
    HTTPClientMod() = default;
    ~HTTPClientMod() = default;
    HTTPClientMod(HTTPClientMod&&) = default;
    HTTPClientMod& operator=(HTTPClientMod&&) = default;

    // Note that WiFiClient's underlying connection *will* be captured
    bool begin(WiFiClient &client, const String& url);
    bool begin(WiFiClient &client, const String& host, uint16_t port, const String& uri = "/", bool https = false);

    // old API is now explicitly forbidden
    bool begin(String url)  __attribute__ ((error("obsolete API, use ::begin(WiFiClient, url)")));
    bool begin(String host, uint16_t port, String uri = "/")  __attribute__ ((error("obsolete API, use ::begin(WiFiClient, host, port, uri)")));
    bool begin(String url, const uint8_t httpsFingerprint[20])  __attribute__ ((error("obsolete API, use ::begin(WiFiClientSecure, ...)")));
    bool begin(String host, uint16_t port, String uri, const uint8_t httpsFingerprint[20])  __attribute__ ((error("obsolete API, use ::begin(WiFiClientSecure, ...)")));
    bool begin(String host, uint16_t port, String uri, bool https, String httpsFingerprint)  __attribute__ ((error("obsolete API, use ::begin(WiFiClientSecure, ...)")));

    void end(void);

    bool connected(void);

    void setReuse(bool reuse); /// keep-alive
    void setUserAgent(const String& userAgent);
    void setAuthorization(const char * user, const char * password);
    void setAuthorization(const char * auth);
    void setAuthorization(String auth);
    void setTimeout(uint16_t timeout);

    // Redirections
    void setFollowRedirects(httpclientmod_followRedirects_t follow);
    void setRedirectLimit(uint16_t limit); // max redirects to follow for a single request

    bool setURL(const String& url); // handy for handling redirects
    void useHTTP10(bool usehttp10 = true);

    /// request handling
    int GET();
    int DELETE();
    int POST(const uint8_t* payload, size_t size);
    int POST(const String& payload);
    int PUT(const uint8_t* payload, size_t size);
    int PUT(const String& payload);
    int PATCH(const uint8_t* payload, size_t size);
    int PATCH(const String& payload);
    int sendRequest(const char* type, const String& payload);
    int sendRequest(const char* type, const uint8_t* payload = NULL, size_t size = 0);
    int sendRequest(const char* type, Stream * stream, size_t size = 0);

    void addHeader(const String& name, const String& value, bool first = false, bool replace = true);

    /// Response handling
    void collectHeaders(const char* headerKeys[], const size_t headerKeysCount);
    String header(const char* name);   // get request header value by name
    String header(size_t i);              // get request header value by number
    String headerName(size_t i);          // get request header name by number
    int headers();                     // get header count
    bool hasHeader(const char* name);  // check if header exists


    int getSize(void);
    const String& getLocation(void); // Location header from redirect if 3XX

    WiFiClient& getStream(void);
    WiFiClient* getStreamPtr(void);
	int writeToPrint(Print* print);
    int writeToStream(Stream* stream);
    const String& getString(void);
    static String errorToString(int error);

    // Async
    int sendRequestAsync(const char *type, const String &payload);
    int sendRequestAsync(const char *type, const uint8_t *payload = NULL, size_t size = 0);
    int sendRequestAsync(const char *type, Stream *stream, size_t size = 0);
    int available();
    // Must read response text before calling this
    int asyncResponseCode();
    int processResponseAsync();
    int cancelRequestAsync();
    bool asyncTimeout();
    const String &getStringAsync(void);
    int sendGetAsync();
    int sendDeleteAsync();
    int sendPostAsync(const uint8_t *payload, size_t size);
    int sendPostAsync(const String &payload);
    int sendPutAsync(const uint8_t *payload, size_t size);
    int sendPutAsync(const String &payload);
    int sendPatchAsync(const uint8_t *payload, size_t size);
    int sendPatchAsync(const String &payload);

protected:
    struct RequestArgument {
        String key;
        String value;
    };

    bool beginInternal(const String& url, const char* expectedProtocol);
    void disconnect(bool preserveClient = false);
    void clear();
    int returnError(int error);
    bool connect(void);
    bool sendHeader(const char * type);
    int handleHeaderResponse();
    int writeToStreamDataBlock(Stream * stream, int len);

    // The common pattern to use the class is to
    // {
    //     WiFiClient socket;
    //     HTTPClientMod http;
    //     http.begin(socket, "http://blahblah");
    // }
    // Make sure it's not possible to break things in an opposite direction

    std::unique_ptr<WiFiClient> _client;

    /// request handling
    String _host;
    uint16_t _port = 0;
    bool _reuse = true;
    uint16_t _tcpTimeout = HTTPCLIENTMOD_DEFAULT_TCP_TIMEOUT;
    bool _useHTTP10 = false;

    String _uri;
    String _protocol;
    String _headers;
    String _base64Authorization;

    static const String defaultUserAgent;
    String _userAgent = defaultUserAgent;

    /// Response handling
    std::unique_ptr<RequestArgument[]> _currentHeaders;
    size_t _headerKeysCount = 0;

    int _returnCode = 0;
    int _size = -1;
    bool _canReuse = false;
    httpclientmod_followRedirects_t _followRedirects = HTTPCLIENTMOD_HTTPC_DISABLE_FOLLOW_REDIRECTS;
    uint16_t _redirectLimit = 10;
    String _location;
    httpclientmod_transferEncoding_t _transferEncoding = HTTPCLIENTMOD_HTTPC_TE_IDENTITY;
    std::unique_ptr<StreamString> _payload;

    // Async
    int asyncHttpCode = 0;
    unsigned long lastSendRequestAsyncTime = 0;
};

#endif
    
#endif /* ESP8266HTTPClientMod_H_ */