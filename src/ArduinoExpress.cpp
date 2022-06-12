#include "ArduinoExpress.h"

void RouteCallback::get(const String& path, MiddlewareFunction middleware, EndpointFunction callback)
{
    this->_path = path;
    this->_middleware = middleware;
    this->_callback = callback;
    this->_method = HTTP_Method::GET;
}


void RouteCallback::post(const String& path, MiddlewareFunction middleware, EndpointFunction callback)
{
    this->_path = path;
    this->_middleware = middleware;
    this->_callback = callback;
    this->_method = HTTP_Method::POST;
}


void RouteCallback::inspect(const String& path, MiddlewareFunction middleware, EndpointFunction callback)
{
    this->_path = path;
    this->_middleware = middleware;
    this->_callback = callback;
    this->_method = HTTP_Method::INSPECT;
}


void RouteCallback::executeCallbacks(HTTP_Request &req, HTTP_Response &res)
{
  if(this->_middleware){
    this->_middleware(req, res, [&](){
                                      this->_callback(req, res);
                                    });
  }else if(this->_callback){
    this->_callback(req, res);
  }
}


bool RouteCallback::match(const String &prefix, const HTTP_Request &req) const
{
  return ((prefix + this->_path == req.route) && (toText(this->_method) == req.method));
}


void RouteCallback::execute(const String &prefix, HTTP_Request &req, HTTP_Response &res, Next next)
{
  logdebug(_path + "got request");
  // If request is matched, the callback is executed and the callback chain is broken
  // else, the next callback is called
  if(match(prefix, req)){
    logdebug(_path + "matched request");
    executeCallbacks(req, res);
  } else {
    next();
  }
}


// --------------------ArduinoExpressRouter-----------------------------
// ---------------------------------------------------------------------


void ArduinoExpressRouter::get(const String& path, EndpointFunction callback)
{
  get(path, nullptr, callback);
}


void ArduinoExpressRouter::get(const String& path, MiddlewareFunction middleware, EndpointFunction callback)
{
  addRouteCallback(RouteCallback{HTTP_Method::GET, path, middleware, callback});
}


void ArduinoExpressRouter::post(const String& path, EndpointFunction callback)
{
  post(path, nullptr, callback);
}


void ArduinoExpressRouter::post(const String& path, MiddlewareFunction middleware, EndpointFunction callback)
{
  addRouteCallback(RouteCallback{HTTP_Method::POST, path, middleware, callback});
}


void ArduinoExpressRouter::addRouteCallback(const RouteCallback& routeCallback)
{
  if(this->_routeCallbacks.size() < MAX_ROUTECALLBACKS_COUNT){
    this->_routeCallbacks.push_back(routeCallback);
    addAllCallback(&_routeCallbacks.back()); 
  }else{
    // FIXME: Throw an error
  }
}


void ArduinoExpressRouter::addAllCallback(Callback* callback)
{
  if(this->_allCallbacks.size() < MAX_CALLBACKS_COUNT){
    this->_allCallbacks.push_back(callback);   
  }else{
    // FIXME: Throw an error
  }
}


bool ArduinoExpressRouter::match(const String& prefix, const HTTP_Request &req) const
{
  return ((prefix + this->_routePrefix).isEmpty() || req.route.startsWith(prefix + this->_routePrefix));
}


void ArduinoExpressRouter::executeNext(const String& prefix, HTTP_Request &req, HTTP_Response &res)
{
  if(_currentCallback < _allCallbacks.size())
  {
    logdebug("Calling callback " + String(_currentCallback));
    _allCallbacks[_currentCallback++]->execute(prefix + this->_routePrefix, req, res, [&](){
      executeNext(prefix, req, res);
    });
  }
}


void ArduinoExpressRouter::execute(const String& prefix, HTTP_Request &req, HTTP_Response &res, Next next)
{
  if(match(prefix, req)){
    // reset the _currentCallback to 0
    _currentCallback = 0;

    // start executing the callback chain
    executeNext(prefix, req, res);
  }
  
  // If no callback in this router has handled the request, call the next on the callback chain
  // else, the callback chain is broken
  if (!res.responseSent())
    next();
}


void ArduinoExpressRouter::use(const String &path, MiddlewareFunction callback) 
{
  if(_middlewareCallbacks.size() < MAX_MIDDLEWARECALLBACKS_COUNT){
    this->_middlewareCallbacks.push_back(MiddlewareCallback{path, callback});
    addAllCallback(&_middlewareCallbacks.back());
  }else{
    // FIXME: Throw an error
  }
}

void ArduinoExpressRouter::use(MiddlewareFunction callback)
{
  use("", callback);
}


// ----------------------ArduinoExpress---------------------------------
// ---------------------------------------------------------------------


void ArduinoExpress::listen(int port, std::function<void()> callback)
{
  this->_server = WiFiServer{port};
  this->_server.begin();

  while(true){
    execute();
    //if (callback) callback();   // FIXME: Enable this, use a default of nullptr
  }
}


void ArduinoExpress::execute()
{
    WiFiClient client = this->_server.available();
    this->_client = &client;

    // if there's a new client
    if(*_client){
      this->_req.clear();
      this->_res.clear();

      parseRequest();
      this->_res = HTTP_Response{this->_client};

      _req.printToSerial();

      // Execute the request
      ArduinoExpressRouter::execute("", this->_req, this->_res, [](){});
      

      // Confirm a response has being sent. If not, send one.
      if (!this->_res.responseSent()){
        this->_res.send(500, "text/plain", "No response");
      }
      
      // Disconnect the client
      client.stop();
      loginfo("[Client disconnected]");
    }

}


void ArduinoExpress::parseRequest()
{
  // GET THE REQUEST LINE
  while(this->_client->connected()){
    if (this->_client->available())
    {
      this->_req.method = this->_client->readStringUntil(' ');
      this->_req.route = this->_client->readStringUntil(' ');
      // Get the request's query
      this->_req.method.trim();
      this->_req.route.trim();
      this->_client->readStringUntil('\r');
      break;
    }
  }
    

  // GET THE REQUEST HEADERS
  int count = 0;
  while(this->_client->connected())
  {
    // read line by line what the client (web browser) is requesting
    if (this->_client->available())
    {
      String line = this->_client->readStringUntil('\r');

      if (line.length() == 1 && line[0] == '\n')
      {
        break;
      }

      if (count < this->_req.MAX_HEADERS_COUNT)
      {
        int colonIndex = line.indexOf(':');
        this->_req.headers[count].key = line.substring(0, colonIndex);
        this->_req.headers[count].value = line.substring(colonIndex+1);
        this->_req.headers[count].key.trim();
        this->_req.headers[count].value.trim();
        count = count + 1;
      }
    }
  }


  // GET THE REQUEST BODY
  while (this->_client->available()) {
    // but first, let client finish its request
    // that's diplomatic compliance to protocols
    // (and otherwise some clients may complain, like curl)
    // (that is an example, prefer using a proper webserver library)
    this->_req.body += this->_client->readStringUntil('\r');
  }
}


void ArduinoExpress::use(const String &path, ArduinoExpressRouter *router)
{
  if(this->_routers.size() < MAX_ROUTERS_COUNT){
    this->_routers.push_back(router);
    this->_routers.back()->setRoutePrefix(path);
    addAllCallback(this->_routers.back());
  }else{
    // FIXME: Throw an error
  }
}


void ArduinoExpress::use(ArduinoExpressRouter *router)
{
  use("", router);
}
