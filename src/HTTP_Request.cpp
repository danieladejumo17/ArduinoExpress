#include "HTTP_Request.h"

void HTTP_Request::printToSerial() const
{
  String reqLine = "Method: ";
  Serial.println(reqLine + this->method + " Route: " + this->route + "\n");
  
  int count = 0;
  while(count < this->MAX_HEADERS_COUNT && this->headers[count].key != ""){
    Serial.print(this->headers[count].key);
    Serial.print(": ");
    Serial.println(this->headers[count].value);
    count = count + 1; 
  }
  Serial.println();
  Serial.println(this->body);
  Serial.println();
}


const String& HTTP_Request::getHeader(const String& headerKey) const
{
  for(int i = 0; i < this->MAX_HEADERS_COUNT; ++i){
    if (this->headers[i].key == headerKey) return this->headers[i].value; 
  }
  
  return "";
}


const String& HTTP_Request::getParam(const String& paramKey) const 
{
  for(int i = 0; i < this->MAX_PARAMS_COUNT; ++i){
    if (this->params[i].key == paramKey) return this->params[i].value; 
  }
  
  return "";
}


bool HTTP_Request::hasHeader(const String& headerKey) const
{
  for(int i = 0; i < this->MAX_HEADERS_COUNT; ++i){
    if (this->headers[i].key == headerKey) return true; 
  }
  
  return false;
}


bool HTTP_Request::hasParam(const String& paramKey) const
{
  for(int i = 0; i < this->MAX_PARAMS_COUNT; ++i){
    if (this->params[i].key == paramKey) return true; 
  }
  
  return false;
}

ArduinoJson::Internals::JsonObjectSubscript<const String &> HTTP_Request::bodyJSON(const String& key)
  {
    DynamicJsonBuffer buffer(200);
    auto json = buffer.parse(body);

    if(!json.success())
      logErr("Could not convert request body to JSON");
    
    return json[key];
  }
