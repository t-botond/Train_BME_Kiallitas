#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <Scheduler.h>
#ifndef WS_HANDLING
#define WS_HANDLING
#include "server_content.h"
#include "train.h"
#include "monitor.h"
#include "autoPilot.h"


Train train("mozdony", 12, 14); //D5, D6
ESP8266WebServer server(80);
Monitor mon;
autoPilote ap=autoPilote();

void logout() {
  IPAddress clientIP = server.client().remoteIP();
  if (mon.hasAccess(clientIP)) {
    server.send(200, "text/html", thx);
    mon.release();
    train.reset();
    return;
  }
  server.send(404, "text/html", notFound);
}
void handleRoot() {
  IPAddress clientIP = server.client().remoteIP();
  if (mon.tryAccess(clientIP)) {
    ap.stop();
    train.reset();
    server.send(200, "text/html", controlPage);
  } else {
    for (uint8_t i = 0; i < server.args(); i++) {
      if (server.argName(i) == String("timeout")) {
        server.send(200,"text/html",timeOut);
        return;
      }
    }
    server.send(200, "text/html", waiting);
  }
}

void handleNotFound() {
  IPAddress clientIP = server.client().remoteIP();
  if (mon.hasAccess(clientIP)) {
    for (uint8_t i = 0; i < server.args(); i++) {
      if (server.argName(i) == String("engine")) {
        train.setEngine(server.arg(i) == String("on"));
      }
      if (server.argName(i) == String("speed")) {
        String spd = server.arg(i);
        train.setSpeed(spd.toInt());
      }
      if (server.argName(i) == String("dir")) {
        String dir = server.arg(i);
        train.setForward( dir == "for");
      }
    }
    server.send(200,"text/plain","ok");
    return;
  }  
  server.send(404, "text/html", notFound);
}

void easterEgg(){
  server.send(200, "text/html", easter);
}
#endif
