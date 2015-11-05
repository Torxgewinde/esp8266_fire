#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <FS.h>

ESP8266WebServer server(80);

// to populate filesystem:
// curl -F "file=@website/index.htm;filename=/index.htm" 192.168.4.1/edit
File uploadFile;

bool loadFromFlash(String path){
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";

  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";

  File dataFile = SPIFFS.open(path.c_str(), "r");
  Serial.println("file to open: "+String(path.c_str()));

  if (!dataFile) {
    return false;
  }

  if (server.hasArg("download")) dataType = "application/octet-stream";

  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
    Serial.println("Sent less data than expected!");
  }

  dataFile.close();
  return true;
}

void handleFileUpload(){
  if(server.uri() != "/edit") return;
  
  HTTPUpload& upload = server.upload();
  
  if(upload.status == UPLOAD_FILE_START){
    if(SPIFFS.exists((char *)upload.filename.c_str())) SPIFFS.remove((char *)upload.filename.c_str());
    uploadFile = SPIFFS.open(upload.filename.c_str(), "w");
    Serial.print("Upload: START, filename: "); Serial.println(upload.filename);
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(uploadFile) uploadFile.write(upload.buf, upload.currentSize);
    Serial.print("Upload: WRITE, Bytes: "); Serial.println(upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(uploadFile) uploadFile.close();
    Serial.print("Upload: END, Size: "); Serial.println(upload.totalSize);
  }
}

void handleNotFound(){
  // try to load file from flash first
  if(loadFromFlash(server.uri())) return;

  // ok, file was not found, generate a 404 page
  String message = "File not found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  
  server.send(404, "text/plain", message);
  Serial.print(message);
}

void setup_webserver() {
  
  // Initialize file system.
  for(int i=0; i<=3; i++) {
    if( !SPIFFS.begin() ) {
      Serial.println("Failed to mount file system, trying to format...");
      SPIFFS.format();
    } else {
      Serial.println("Filesystem mounted");
      break;
    }
  }

  // this is a catch-all, first it tries to load file from flash
  // if file is not present, a 404 page is created
  server.onNotFound(handleNotFound);

  // If there is an file upload to URL "/edit", it will be stored to flash
  server.on("/edit", HTTP_POST, [](){ 
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "");
  });
  server.onFileUpload(handleFileUpload);

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
