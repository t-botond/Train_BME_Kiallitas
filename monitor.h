#ifndef MONITOR_H
#define MONITOR_H
#include <ESP8266WiFi.h>
#include <Scheduler.h>

/**
   Ez az osztály felelős azért, hogy egyszerre csak egyetlen felhsaználó vezérelhesse a mozdonyt.
*/
class Monitor: public Task {
    IPAddress clientIP;                                 ///Az aktuálisan bérlettel rendelkező cliens ip-je
    const IPAddress local = IPAddress(192, 168, 0, 1);  ///A host IP-je
    volatile unsigned long t;                           ///Utolsó művelet időbélyege
    volatile unsigned long lastAction = 0;              ///Utolsó művelet időbélyege
    unsigned long leaseTime;                            ///Bérleti idő. A kliens minimum a megadott ideig vezérelheti a mozdonyt.
    void (*callBack)();                                 ///A callBack esemény
    const IPAddress ures = IPAddress(0, 0, 0, 255);     ///Üres IP-cím
  public:
    /**
       Konstruktor nullázza a kezdő időpontot, így a vezérlés jogát biztosan bárki megszerezheti.
       @param berletiIdo A munkamenet lejárati ideje. Alapértelmezettként 30 sec.
    */
    Monitor(const unsigned long berletiIdo = 30000) {
      t = 0;
      leaseTime = berletiIdo;
      callBack = NULL;
      clientIP = local;
    }

    /**
       A metódus hívás hatására a kliens megpróbálja megszerezni a vezérlés jogát.
       Ha az aktuális jogosult ideje lejár az újabb kliens elveszi a vezérlést a korábbitól.
       @param ip A vezérlésre igényt tartó kliens IP címe.
    */
    bool tryAccess(const IPAddress& ip) {
      if (hasAccess(ip)) return true;
      if (t + leaseTime < millis() || clientIP == local || t == 0 || clientIP == ures) {
        clientIP = ip;
        t = millis();
        lastAction = millis();
        return true;
      }
      return false;
    }

    /**
       Getter, amivel lekérdezheti a kliens, hogy van-e még vezérlési joga.
       @param ip A vezérlés jogára rákérdező kliens IP címe.
    */
    bool hasAccess(const IPAddress& ip) {
      if (clientIP == ip) lastAction = millis();
      return (clientIP == ip);
    }

    /**
     * A vezérlő kliens elveszti a vezérlés jogát.
     */
    void release() {
      t = 0;
      lastAction = millis();
      clientIP = ures;
    }

    /**
     * Lekérdezhető, hogy a monitor éppen foglalt-e.
     */
    bool isEmpty() {
      return clientIP == ures;
    }

    /**
     * Az utolsó művelet időpontja (rendszeridőben [millis()])
     */
    volatile unsigned long lastActionTime()const {
      return lastAction;
    }

    /**
     * Bérleti idő lejárta után elsüti a callBack eseményt.
     */
    void setReleaseCallBack(void (*cb)()) {
      callBack = cb;
    }

    /**
     * Elsüti a callBack függvényt, ha be van állítva, és a leaseTime-on felül nem történt esemény.
     */
    void loop() {
      if (lastAction + leaseTime < millis() && clientIP != local) {
        #ifdef TRAIN_DBG 
          Serial.println("Esemeny elsutve"); 
        #endif
        if (callBack != NULL) (*callBack)();
      }
      delay(1000);
    }

    /**
     * Az aktuális felhasználó IP-jének lekérdezése.
     */
    const IPAddress& getUserIP() {
      return clientIP;
    }

};
#endif
