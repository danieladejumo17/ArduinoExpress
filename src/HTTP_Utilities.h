#ifndef HTTP_UTILITIES_HEADER
#define HTTP_UTILITIES_HEADER

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "utilities.h"

enum HTTP_Method : int{GET = 1, POST = 2, PUT = 3, DELETE = 4, INSPECT = 5};

String toText(const HTTP_Method& method);

struct HTTP_User{
  String user_id;
  String user_auth;
};

struct HTTP_Header{
  String key;
  String value;
};

struct HTTP_Param{
  String key;
  String value;
};

// JsonObject& textToJSON(const char* , int );

String HTTPStatusText(int status);


// ---------- LOGGING ---------------
void logdebug(const String& message);
void loginfo(const String& message);
void logerr(const String& message);
void logwarn(const String& message);
void logfatal(const String& message);

#endif
