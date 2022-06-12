/*
 * This library provides functionalities to encapsulate and process an HTTP_Response
 */

#ifndef HTTP_RESPONSE_HEADER
#define HTTP_RESPONSE_HEADER

#include "HTTP_Utilities.h"
#include <ESP8266WiFi.h>

struct HTTP_Response{
  private:
    int _status = 0;
    String _statusText;
  
    const static int MAX_HEADERS_COUNT = ArduinoExpressConfig::MAX_HEADERS_COUNT;
    HTTP_Header _headers[MAX_HEADERS_COUNT];
    int _headersCount = 0;
  
    String _body;
    
    WiFiClient *_client = nullptr;
    bool _responseSent = false;


    // ALWAYS UPDATE CLEAR

    int getHeaderIndex(const String& ) const;

  public:
    HTTP_Response(WiFiClient *client): _client{client} {};

    int status() const {return this->_status;}
    const String& statusText() const {return this->_statusText;}
    const HTTP_Header* headers() const {return this->_headers;}
    const String& body() const {return this->_body;}
    bool responseSent() const {return this->_responseSent;}
    
    void setStatus(int, const String& ); // the status text is optional, it will be filled if none is provided
    bool hasHeader(const String& ) const;
    const String& getHeader(const String& ) const;
    void setHeader(const String&, const String& );
    void setBody(const String&, const String& );  //Content-Type, Body
    

    bool send();
    bool send(int, const String&, const String& ); //status, Content-Type, Body
    bool json(int, const String& ); // use the send function with content-type = text/json

    void clear() {*this = HTTP_Response{nullptr};}
 };


#endif
