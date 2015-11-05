#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/html", "<h1>You are connected</h1>");
}

void setup_webserver() {
  server.on("/", handleRoot);

  server.on("/defaults", [](){
    String message = "OK";

    g_cool=40, g_low=14, g_high=25;
    
    server.send(200, "text/plain", message);
  });

  server.on("/set", [](){
    String message;
    
    for (uint8_t i=0; i<server.args(); i++){
      if(server.argName(i).equals("cool")) {
        g_cool = server.arg(i).toInt();
        message = String(g_cool);
      }
      if(server.argName(i).equals("low")) {
        g_low = server.arg(i).toInt();
        message = String(g_low);
      }
      if(server.argName(i).equals("high")) {
        g_high = server.arg(i).toInt();
        message = String(g_high);
      }
    }

    server.send(200, "text/plain", message);
  });

  server.on("/get", [](){
    String message;

    for (uint8_t i=0; i<server.args(); i++){
      if(server.argName(i).equals("cool")) {
        message = String(g_cool);
      }
      if(server.argName(i).equals("low")) {
        message = String(g_low);
      }
      if(server.argName(i).equals("high")) {
        message = String(g_high);
      }
    }
    
    server.send(200, "text/plain", message); 
  });
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop_webserver() {
  server.handleClient();
}
