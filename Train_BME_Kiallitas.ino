#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <DNSServer.h>
#include "webserver_handling.h"

DNSServer dnsServer;
ESP8266HTTPUpdateServer httpUpdater;

#define host "mozdony"
#define update_path "/srZMB1AxoY1uW7pC"
#define update_username "NCOJx3JefsgPn7sP"
#define update_password "t8oBaXHbuaFzAK5R"

class sTask : public Task {
    void loop() { 
      server.handleClient();
      dnsServer.processNextRequest();
    }
} serverTask;

void stopTrain() {
  train.reset();
  ap.start();
}

void setup() {
  #ifdef TRAIN_DBG 
    Serial.begin(115200);
    Serial.println();
    Serial.println("Starting...");
  #endif

  analogWriteRange(1023);
  WiFi.persistent(false);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.hostname("mozdony");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IPAddress(192, 168, 0, 1), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP("Okos Mozdony", "", 4, false, 5); //WiFi.softAP(ssid, psk, channel, hidden, max_connection);
  ap.set(&train, &mon);

  server.on("/", handleRoot);
  server.on("/logout", logout);
  server.on("/turaibotond", easterEgg);
  server.onNotFound(handleNotFound);
  server.begin();

  dnsServer.start(53, "*", IPAddress(192, 168, 0, 1));
  httpUpdater.setup(&server, update_path, update_username, update_password);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  unsigned long timeout = millis();
  while (!ap.start() && timeout + 2000 > millis()) {
    #ifdef TRAIN_DBG 
      Serial.println("Try to start AutoPilote"); 
    #endif
    ap.set(&train, &mon);
    delay(100);
  }
  mon.setReleaseCallBack(&stopTrain);
  train.startProtectEngine();

  //Scheduler.start(&serverTask);
  Scheduler.start(&train);
  Scheduler.start(&ap);
  Scheduler.start(&serverTask);
  Scheduler.start(&mon);

  Scheduler.begin();
}

void loop() {}
