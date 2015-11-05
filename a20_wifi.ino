#include <ESP8266WiFi.h>

void setup_wifi() {
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);  
}

void loop_wifi() {
  
}
