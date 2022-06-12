/*
 * This library provides functionalities to encapsulate and process and process an HTT_Reqest
 */

#ifndef HTTP_REQUEST_HEADER
#define HTTP_REQUEST_HEADER

#include "HTTP_Utilities.h"

struct HTTP_Request{
  String method = "";
  String route = "";

  const static int MAX_PARAMS_COUNT = ArduinoExpressConfig::MAX_PARAMS_COUNT;
  HTTP_Param params[MAX_PARAMS_COUNT];

  const static int MAX_HEADERS_COUNT = ArduinoExpressConfig::MAX_HEADERS_COUNT;
  HTTP_Header headers[MAX_HEADERS_COUNT];
  
  String body = "";


  HTTP_User user;

  const String& getHeader(const String& ) const;
  const String& getParam(const String& ) const;
  bool hasHeader(const String& ) const;
  bool hasParam(const String& ) const;
  void printToSerial() const;

  void clear()
  {
    *this = HTTP_Request{};
  }

  ArduinoJson::Internals::JsonObjectSubscript<const String &> bodyJSON(const String& key);
};

#endif
