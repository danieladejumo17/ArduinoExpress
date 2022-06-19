// TODO: Change implementattion to store char* for route/path instead of String. strcmp can be used instead of ==

#include <ArduinoExpress.h>

// Setup WiFi
const char *ssid = "Alarm AP";
// WiFiServer server(80);

void setup()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid);
  IPAddress myIP = WiFi.softAPIP();

  Serial.begin(115200);

  auto app = ArduinoExpress();

  app.use([](Req &req, Res &res, Next next) -> void *
          { 
            // FIXME: Add this middleware to ArdiunoExpress
            DynamicJsonBuffer buffer(200);
            if (buffer.parseObject(req.body).success()){
              next();
            }
            else{
              res.send(400, "text/plain", "Invalid JSON body");
            }
          });

  app.get("/",
      [](Req &req, Res &res, Next next) -> void *
      { 
        res.send(200, "text/plain", "This route is under maintenance");
      },

      [](Req &req, Res &res) -> void *
      {
        logdebug("Root got request");
        res.send(200, "text/plain", "Hello World!");
      });

  app.post("/raise",
      [](Req &req, Res &res, Next next) -> void *
      {
        // We just want to test this
        next();
      },

      [](Req &req, Res &res) -> void *
      {
        logdebug("Raise got request");
        res.send(200, "text/plain", "Hello AlarmInitiate!");
      });

  app.listen(80, []() -> void {}); // TODO: Only provide this callback as an extra feature
}

void loop() {}