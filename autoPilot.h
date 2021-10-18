#ifndef AUTOPILOT_H
#define AUTOPILOT_H
#include <ESP8266WiFi.h>
#include <Scheduler.h>
#include "train.h"
#include "monitor.h"

class autoPilote: public Task {
    IPAddress local;
    Monitor* apmon = NULL;
    Train* mozdony = NULL;
    bool bekapcsolva;
    int ciklusido;
    int maxSpeed;
  public:
    autoPilote(const IPAddress& defaultLocalIP=IPAddress(192, 168, 0, 1)):local(defaultLocalIP), bekapcsolva(true), ciklusido(3000), maxSpeed(150) {
    }
    void set(Train* t, Monitor* m) {
      apmon = m;
      mozdony = t;
    }
    bool isset(){
      return (apmon != NULL && mozdony != NULL);
    }
    void loop() {
      if (apmon == NULL || mozdony == NULL || bekapcsolva == false) {
        delay(1000);
        return;
      }

      mozgasCiklus();
      mozgasCiklus();
      if (bekapcsolva && apmon->hasAccess(local)) mozdony->reset();
      delay(120000);
    }

    void mozgasCiklus() {
      #ifdef TRAIN_DBG 
        Serial.println("MozgasCiklus"); 
      #endif
      if (bekapcsolva && apmon->hasAccess(local)) {
        mozdony->setEngine(true);
        mozdony->setForward(false);
        mozdony->setSpeed(maxSpeed);
        delay(ciklusido);
      } else return;

      if (bekapcsolva && apmon->hasAccess(local)) {
        mozdony->setSpeed(0);
        delay(ciklusido + 500);
      } else return;

      if (bekapcsolva && apmon->hasAccess(local)) {
        mozdony->setForward(true);
        mozdony->setSpeed(maxSpeed);
        delay(ciklusido);
      } else return;

      if (bekapcsolva && apmon->hasAccess(local)) {
        mozdony->setSpeed(0);
        delay(ciklusido + 500);
      } else return;

    }

    void stop() {
      bekapcsolva = false;
      #ifdef TRAIN_DBG 
        Serial.println("Pilote stop");
      #endif
    }

    bool start() {
      if (apmon == NULL || mozdony == NULL) return false;
      IPAddress local = IPAddress(192, 168, 0, 1);
      bekapcsolva = apmon->tryAccess(local);
      if (bekapcsolva) mozdony->reset();
      #ifdef TRAIN_DBG 
        Serial.println("Pilote start"); 
      #endif
      return bekapcsolva;
    }
};

#endif
