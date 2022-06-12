#ifndef ARDUINO_EXPRESS_HEADER
#define ARDUINO_EXPRESS_HEADER

#include <ESP8266WiFi.h>
#include "HTTP_Request.h"
#include "HTTP_Response.h"
#include <Vector.h>

// Req is an alias for HTTP_Request, Res is an alias for HTTP_Response
using Req = HTTP_Request;
using Res = HTTP_Response;

// Next is an alias for void(*)(), which is a function that takes no arguments and returns void
using Next = std::function<void()>;
using EndpointFunction = std::function<void*(Req&, Res&)>;
using MiddlewareFunction = std::function<void*(Req&, Res&, Next)>;

/* Inherited by RouteCallback, MiddlewarCallback and the ArduinoExpressRouter which implement 
   * the pure virtual execute() method.
   *
   * method: execute(route, req, res, next)
   *  - route: the route this callback is called on. A callback will respond to one or more routes.
   *  - req: a reference to the HTTP_Request object
   *  - res: a reference to this request's response, an HTTP_Response object that can be populated 
   *    with data passed to each callback
   *  - next: next() will call the next callback in the callback chain handled by the ArduinoExpressRouter
   *  
   *  calling res.send() will send the response to the client, and terminate the callback chain
   * */
struct Callback{
  virtual void execute(const String&, Req&, Res&, Next ) = 0;
};


/* The Route callback encapsulates a single endpoint in the ArduinoExpressRouter.
  * This include the path, the HTTP method, middleware(s) and the actual endpoint callback.
  * Note: The path is the relative path to the endpoint in the router where this callback belongs.
*/
struct RouteCallback: public Callback{
  private:
    String _path; // the path this callback listens to
    HTTP_Method _method;  // the http method this callback listens to
    // This can be different from the complete HTTP request route if a router with a prefix is used
    // with this callback.

    MiddlewareFunction _middleware = nullptr;
    EndpointFunction _callback = nullptr;
  
  public:
    RouteCallback() {}
    RouteCallback(HTTP_Method method, const String& path, 
                  MiddlewareFunction middleware, 
                  EndpointFunction callback)
      : _method{method}, _path{path}, _middleware{middleware}, _callback{callback} {}
    
    // void get(path, middleware, callback)
    // creates a callback that responds to a GET request to the given path
    void get(const String&, MiddlewareFunction, EndpointFunction);
    
    // void post(path, middleware, callback)
    // creates a callback that responds to a POST request to the given path
    void post(const String&, MiddlewareFunction, EndpointFunction);
    
    // void inspect(path, middleware, callback)
    // creates a callback that responds to an INSPECT request to the given path
    void inspect(const String&, MiddlewareFunction, EndpointFunction);

    // void put(const String&, MiddlewareFunction, EndpointFunction);
    // void deleteMethod(const String&, MiddlewareFunction, EndpointFunction);
    

    /* returns the path this callback is created on. 
      * This can be different from the complete HTTP request route if a router with a prefix is used
      * with this callback.
      * */
    const String& path() const {return this->_path;}

    /* executes all the middlewares and callbacks on this route */
    void executeCallbacks(Req&, Res& );

    /* Returns true if the request matches this route */
    bool match(const String&, const Req& ) const;

    /* void execute(prefix, req, res, next)
      * - prefix: the prefix used by the router that this callback belongs to
      * executes the callback, if the request matches this route 
      * */
    void execute(const String&, Req&, Res&, Next);
};


/* A middleware acts on a request before the request reaches it's endpoint. It can modify the 
  * request and/or the response, or terminate the request - by sending the response before it get's
  * to its endpoint.
  * The MiddlewareCallback encapsulates a single middleware in the ArduinoExpressRouter.
  * This include the path and the actual middleware callback.
  * Note: The path is the relative path this middleware acts on in the router where it belongs.
*/
struct MiddlewareCallback: public Callback{
  private:
    String _path; // the path this middleware acts on
    MiddlewareFunction _callback = nullptr;

  public:
    MiddlewareCallback() {}
    MiddlewareCallback(const String& path, MiddlewareFunction callback)
      : _path{path}, _callback{callback} {}

    // const String& path()
    // returns the path this middleware acts on
    const String& path() const {return this->_path;}

    // void executeCallback(req, res, next)
    // executes the middleware callback
    void executeCallbacks(Req &req, Res &res, Next next) {this->_callback(req, res, next);}
    
    // void match(prefix, req)
    // returns true if the request's route matches this middleware (in the router it belongs)
    bool match(const String &prefix, const Req &req) const 
    {
      return req.route.startsWith(prefix + this->_path) || (prefix + this->_path).isEmpty();
    }
    
    // void execute(prefix, req, res, next)
    // compares the request's route to the middleware's path, and if it matches, executes the middleware
    void execute(const String &prefix, Req &req, Res &res, Next next) 
    {
      if (match(prefix, req)){
        executeCallbacks(req, res, next);
      }
    }
  
};


struct ArduinoExpressRouter: public Callback{
  protected:
    String _routePrefix = "";
    
    const static int MAX_ROUTECALLBACKS_COUNT = ArduinoExpressConfig::MAX_ROUTECALLBACKS_COUNT;
    RouteCallback _routeCallbacksStorageArray[MAX_ROUTECALLBACKS_COUNT];
    Vector<RouteCallback> _routeCallbacks{_routeCallbacksStorageArray};

    const static int MAX_MIDDLEWARECALLBACKS_COUNT = ArduinoExpressConfig::MAX_MIDDLEWARECALLBACKS_COUNT;
    MiddlewareCallback _middlewareCallbacksStorageArray[MAX_MIDDLEWARECALLBACKS_COUNT];
    Vector<MiddlewareCallback> _middlewareCallbacks{_middlewareCallbacksStorageArray};

    const static int MAX_CALLBACKS_COUNT = ArduinoExpressConfig::MAX_CALLBACKS_COUNT;
    Callback* _callbacksStorageArray[MAX_CALLBACKS_COUNT];
    Vector<Callback*> _allCallbacks{_callbacksStorageArray};

    int _currentCallback = 0;

    // void addRouteCallback(RouteCallback& callback)
    // adds a new callback to _routeCallbacks.
    // This also calls addAllCallbacks() to add the callback to _allCallbacks
    void addRouteCallback(const RouteCallback& );

    // void addAllCallback(Callback* callback)
    // adds a new callback to _allCallbacks
    void addAllCallback(Callback* );

    // bool match (prefix, req)
    // returns true if the request's route matches (prefix + this router's prefix)
    // This allows a router to be used in another router - the root app is also a router
    bool match(const String&, const Req&) const;

    // void executeNext(prefix, req, res)
    // executes the next callback in the callback chain
    virtual void executeNext(const String&, Req&, Res&);

    // void execute(prefix, req, res, next)
    // executes all the middlewares and callbacks on this router if the prefix matches
    virtual void execute(const String&, Req&, Res&, Next);

  public:
    // Adds a RouteCallback with the HTTP GET method to the router
    // no middleware is added to the callback
    void get(const String&, EndpointFunction);

    // Adds a RouteCallback with the HTTP GET method to the router
    void get(const String&, MiddlewareFunction, EndpointFunction);
    
    // Adds a RouteCallback with the HTTP POST method to the router
    // no middleware is added to this callback
    void post(const String&, EndpointFunction);

    // Adds a RouteCallback with the HTTP POST method to the router
    void post(const String&, MiddlewareFunction, EndpointFunction);

    void setRoutePrefix(const String& prefix) {this->_routePrefix = prefix;}

    // adds a middleware to the router on the specified path
    virtual void use(const String&, MiddlewareFunction);

    // adds a middleware to the router at the root of the router. 
    // i.e. all request to this router will go through it
    virtual void use(MiddlewareFunction);
};
// FIXME: add functionality to add router objects to the router


struct ArduinoExpress : public ArduinoExpressRouter
{
  private:
    WiFiClient* _client = nullptr;
    WiFiServer _server{80};

    HTTP_Request _req;
    HTTP_Response _res{nullptr};

    void parseRequest();
    void execute();

    const static int MAX_ROUTERS_COUNT = ArduinoExpressConfig::MAX_ROUTERS_COUNT;
    ArduinoExpressRouter* _routersStorageArray[MAX_ROUTERS_COUNT];
    Vector<ArduinoExpressRouter*> _routers{_routersStorageArray};
    

  public:
    // Pass a callback function to perform tasks at the end of each ArduinoExpress pass
    void listen(int port, std::function<void()> callback); // iterate_all option: default False || add express callback

    // add a router on the specified path
    void use(const String&, ArduinoExpressRouter* );
    
    // add a router on the root path
    void use(ArduinoExpressRouter* );
    
    // add a middleware on the specified path
    void use(const String &path, MiddlewareFunction callback) 
    {ArduinoExpressRouter::use(path, callback);}

    // add a middleware on the root path
    void use(MiddlewareFunction callback) 
    {ArduinoExpressRouter::use(callback);}
    
    // creates and return an ArduinoExpressRouter object
    static ArduinoExpressRouter Router() {return ArduinoExpressRouter();}
  
};

#endif
