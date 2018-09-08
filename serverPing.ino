/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <WiFi.h>
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`


const char* ssid     = "tabinet";
const char* password = "aaaabbbbccccddddaaaabbbbaa";

SSD1306  display(0x3c, 5, 4);

//const char* serverAddress = "10.0.0.182";
//const char* serverAddress = "34.211.42.63";
//const char* serverAddress = "ec2-34-211-42-63.us-west-2.compute.amazonaws.com";
const char* serverAddress = "engine-dev.apollovoice.ai";
const int apolloServerPort = 5000;
const int rnnServerPort = 5500;

#define NUM_LINES 6
String displayBuffer[NUM_LINES];

void setup()
{
    Serial.begin(115200);
    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    display.init();
  
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
}

int value = 0;


void drawLines(String line, int linenum){
  int fontSize = 10;
  int yoffset = linenum * fontSize;
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, yoffset, line);
  //display.setTextAlignment(TEXT_ALIGN_RIGHT);
  //display.drawString(10, 128, String(millis()));
}

void drawDisplayBuffer(){
  display.resetDisplay();
  for (int i = 0; i<NUM_LINES;i++){
    Serial.print("printing display line ");
    Serial.print("[");
    Serial.print(i);
    Serial.print("]: ");
    if (displayBuffer[i] != ""){
      drawLines(displayBuffer[i],i); 
      Serial.println(displayBuffer[i]);
    }else{
      Serial.println("NULL");
    }
  }
  display.display();
}

void clearDisplayBuffer(){
  for (int i = 0; i<NUM_LINES;i++){
    displayBuffer[i] = "";
  }
}

void checkServer(int &linenum, const char *address, const int port){
  
  WiFiClient client;
  if (!client.connect(address, port)) {
      Serial.println("connection failed");
      String failstring = "Connection Failed";
      displayBuffer[linenum] = failstring;     
      return;
  }
  
  // We now create a URI for the request
  String url = "/";
 
  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + address + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
      if (millis() - timeout > 5000) {
          Serial.println(">>> Client Timeout !");
          client.stop();
          return;
      }
  }

  // Read all the lines of the reply from server and print them to Serial
  
  int bDisplayLines = false;
  while(client.available()) {
      String line = client.readStringUntil('\r');
      line.trim();
      Serial.println(line);

      if (bDisplayLines) {
        Serial.print("analyzing found line: ");
        Serial.println(line);
        int nextstart = 0;
        int nextend = line.indexOf('\n');
        Serial.print("index: ");
        Serial.println(nextend);
        while(nextend != -1){
          String subline = line.substring(nextstart,nextend);
          subline.trim();
          Serial.print("adding line: ");
          Serial.print("[");
          Serial.print(linenum);
          Serial.print("]: ");
          Serial.println(subline.c_str());
          displayBuffer[linenum] = subline;
          nextstart = nextend + 1;
          nextend = line.indexOf("\n", nextstart);
          linenum++;
        }
        // add the last part:
        String subline = line.substring(nextstart);
        subline.trim();
        Serial.print("adding line: ");
        Serial.print("[");
        Serial.print(linenum);
        Serial.print("]: ");
        Serial.println(subline.c_str());
        displayBuffer[linenum] = subline;
        linenum++;
        
      }

      if (!line.length()){
        bDisplayLines = true;
      }
  }
}

void loop()
{
    int linenum = 2;
    clearDisplayBuffer();
    ++value;

    Serial.print("connecting to ");
    Serial.println(serverAddress);

    String initialString = "Connecting to: ";
    initialString += serverAddress;


    // Use WiFiClient class to create TCP connections
    displayBuffer[0] = initialString;
    
    checkServer(linenum, serverAddress, apolloServerPort);
    linenum++;
    checkServer(linenum, serverAddress, rnnServerPort);
    
    drawDisplayBuffer();
    Serial.println();
    Serial.println("closing connection");
    delay(60000);
}

