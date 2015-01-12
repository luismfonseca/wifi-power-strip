#include "uartWIFI.h"
#include <SoftwareSerial.h>

#define DEFAULT_PASSWORD "abcde12345"
#define DEFAULT_AP_CHANNEL 11
int relay = A5;

WIFI wifi;

int getSelfId() {
  randomSeed(analogRead(A0));
  return random(0, 1024);
  //return 0;
}

String requestGetSsid(String request) {
  return request.substring(0, request.indexOf('&pw=') - 3);
}

String requestGetPass(String request) {
  int start = request.indexOf('&pw=') + 1;
  return request.substring(start, request.indexOf(' '));
}

boolean setupRequestSuccessful() {
  
  char buf[444];
  int len = wifi.ReceiveMessage(buf);
  if (len > 0)
  {
    int space = String(buf).indexOf(' ');
    String ssid = requestGetSsid(&buf[space + 5]);
    String pass = requestGetPass(&buf[space + 5]);
    
    boolean successfulConnection = wifi.confJAP(ssid, pass);

    extern int chlID;
    if (successfulConnection == false) {
      wifi.Send(chlID, "Error.");
      delay(2000);
      wifi.closeMux(chlID);
      return false;
    }
    else {
      String ip = wifi.showIP();
      wifi.Send(chlID, ip);
      delay(3000);
      wifi.closeMux(chlID);
      setupAsClient(ssid, pass);
      return true;
    }
  } else {
    return false;
  }
}

void setupAsServer() {
  wifi.confMode(3);
  wifi.Reset();
  wifi.confSAP("Legrand Outlet #" + String(getSelfId()), DEFAULT_PASSWORD, DEFAULT_AP_CHANNEL, 3);
  wifi.Reset();
  wifi.confMux(true);
  wifi.confServer(1, 80);
  
  while (setupRequestSuccessful() == false);
}

void setupAsClient(String ssid, String pass) {
  wifi.confMode(1);
  wifi.Reset();
  wifi.confJAP(ssid, pass);
  wifi.confMux(true);
  wifi.confServer(1, 80);
  // TODO: Check if the connection was fruitful :o
}

void setup() {
  wifi.begin();
  setupAsServer();
}

// Control the switch on/off
boolean switchStatus = false;
void loop() {
  
  char buf[444];
  int len = wifi.ReceiveMessage(buf);
  if (len > 0)
  {
    String sbuf = String(buf);
    int space = sbuf.indexOf(' ');
    String request = sbuf.substring(space + 1);
    if (request.startsWith("/off")) {
      switchStatus = false;
    }
    else if (request.startsWith("/on")) {
      switchStatus = true;
    }
    else if (request.startsWith("/t")) {
      switchStatus = !switchStatus;
    }
    if (switchStatus) {
      analogWrite(relay, 255);
    } else {
      analogWrite(relay, 0);
    }
    extern int chlID;
    wifi.Send(chlID, String(switchStatus));
    delay(3000);
    wifi.closeMux(chlID);
  }
}

