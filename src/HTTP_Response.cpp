#include "HTTP_Response.h"

void HTTP_Response::setStatus(int status, const String& statusText = "")
{
  // the status text is optional, it will be filled if none is provided
  this->_status = status;
  if(!statusText.isEmpty()) this->_statusText = statusText;
  else{
    this->_statusText = HTTPStatusText(status);
  }
}


void HTTP_Response::setHeader(const String& key, const String& value)
{
  int index = getHeaderIndex(key);
  if (index != -1){
    this->_headers[index].value = value;
  }
  else if (this->_headersCount < this->MAX_HEADERS_COUNT){
    HTTP_Header newHeader;
    newHeader.key = key;
    newHeader.value = value;
    
    this->_headers[this->_headersCount] = newHeader;

    this->_headersCount = this->_headersCount + 1;
  }
}


bool HTTP_Response::hasHeader(const String& headerKey) const
{
  for(int i = 0; i < this->MAX_HEADERS_COUNT; ++i){
    if (this->_headers[i].key == headerKey) return true; 
  }
  
  return false;
}


const String& HTTP_Response::getHeader(const String& headerKey) const
{
  for(int i = 0; i < this->MAX_HEADERS_COUNT; ++i){
    if (this->_headers[i].key == headerKey) return this->_headers[i].value; 
  }
  
  return "";
}


int HTTP_Response::getHeaderIndex(const String& headerKey) const
{
  for(int i = 0; i < this->MAX_HEADERS_COUNT; ++i){
    if (this->_headers[i].key == headerKey) return i;
  }
  
  return -1;
}


void HTTP_Response::setBody(const String& contentType, const String& body)
{
  setHeader("Content-type", contentType);
  this->_body = body;
}


bool HTTP_Response::send()
{
  // CONFIRM STATUS AND STATUS TEXT IS SET
  if(this->_status == 0 || this->_statusText.isEmpty()){
    // throw "Status and/or statusText not set";
    Serial.println("Status and/or statusText not set");
    return false;
  }
  
  // CONFIRIM A RESPONSE HAS NOT BEING SENT
  if(responseSent()){
    // throw "A response has being sent earlier";
    Serial.println("A response has being sent earlier");
    return false;
  }

  // CONFIRM THERE IS A CLIENT
  if(!this->_client){
    //throw "No client to send response to";
    Serial.println("No client to send response to");
    return false;
  }

  // -------------------------------------------------------------

  // ADD STATUS LINE
  String response = "HTTP/1.1 " + String(this->_status) + " " + this->_statusText + "\n";

  // ADD HEADERS
  setHeader("Content-Length", String(this->_body.length()));
  setHeader("Connection", "close");
  for(int i = 0; i < this->MAX_HEADERS_COUNT; ++i){
    HTTP_Header header = this->_headers[i];
    if(header.key == "") continue;

    response += header.key + ": " + header.value + "\n";
  }
  response += "\n";

  // ADD BODY
  response += this->_body;
  response += "\n";

  // SEND THE RESPONSE
  this->_client->print(response);
  this->_responseSent = true;
  loginfo("RESPONSE START <=====\n" + response + "\nRESPONSE END =====>");

  return true;
}


bool HTTP_Response::send(int status, const String& contentType, const String& body)
{
  setStatus(status);
  setBody(contentType, body);
  return send();
}

bool HTTP_Response::json(int status, const String& body)
{
  return send(status, "application/json", body);
}
