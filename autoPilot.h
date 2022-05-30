#ifndef AUTOPILOT_H
#define AUTOPILOT_H
//#define TRAIN_DBG
#include <ESP8266WiFi.h>
#include <Scheduler.h>
#include "train.h"
#include "monitor.h"

class autoPilote : public Task {
    Monitor* mon = NULL;
    Train* mozdony = NULL;
    bool isset = false;
    bool state = false;
    const IPAddress local = IPAddress(192, 168, 0, 1);
    int ciklusido=3500;
    int maxSpeed=170;

  public:
    autoPilote() {}

    void set(Monitor& moni, Train& tr) {
      isset = (&moni != NULL && &tr != NULL);
      mon = &moni;
      mozdony = &tr;
    }

    bool start() {
      if (!isset) return false;
      state = mon->tryAccess(local);
      if (state) mozdony->reset();
      return state;
    }
    void stop() {
      state = false;
      mon->release(local);
    }

    void loop() {
      if (!state || !isset) {
        delay(1000);
        return;
      }
      mozgasCiklus();
      mozgasCiklus();
      if (state && mon->hasAccess(local)) mozdony->reset();
      delay(240000);
    }

  private:
    void mozgasCiklus() {
      if (state && mon->hasAccess(local)) {
        mozdony->setEngine(true);
        mozdony->setForward(false);
        mozdony->setSpeed(maxSpeed);
        delay(ciklusido);
      } else return;

      if (state && mon->hasAccess(local)) {
        mozdony->setSpeed(0);
        delay(ciklusido + 500);
      } else return;

      if (state && mon->hasAccess(local)) {
        mozdony->setForward(true);
        mozdony->setSpeed(maxSpeed);
        delay(ciklusido);
      } else return;

      if (state && mon->hasAccess(local)) {
        mozdony->setSpeed(0);
        delay(ciklusido + 500);
      } else return;
    }
};

#endif
