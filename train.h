#include <Arduino.h>
#ifndef TRAIN
#define TRAIN
//#define TRAIN_DBG
#include <Scheduler.h>
#define MIN_SPEED 130   ///Mozdony minimum sebessége
#define MAX_SPEED 300   ///Mozdony megengedett maximum sebessége

/**
 * A mozdony motor védelmét megvalósító osztály
 */
class protectEngine {
    bool dir;                   ///Haladás iránya
    int lastSpeed;              ///A haladás sebessége
    unsigned long startTime;    ///Az időmérés kezdete
    unsigned long currentTime;  ///A stopperóra állása
    unsigned long maxTime;      ///Idő küszöb, amin felül tiltani kell a motort
    bool en;                    ///Engedélyező bit
  public:
    /**
     * Konstruktor
     * @mxTime - Idő küszöb, amin felül tiltani kell a motort
     */
    protectEngine(unsigned long mxTime = 12000): dir(false), lastSpeed(0), startTime(0), currentTime(0), maxTime(mxTime), en(false) {

    }

    /**
     * Motor védelem aktiválása
     */
    void start() {
      en = true;
      lastSpeed=0;
      startTime=0;
      currentTime=0;
    }

    /**
     * Motor védelem deaktiválása
     */
    void stop() {
      en = false;
    }

    /**
     * Minden egyes alkalommal elmentjük, hogy milyen irányban halad, milyen sebességgel.
     */
    void set(const bool& irany, const int& speed) {
      if (lastSpeed == 0 && startTime == 0 && currentTime == 0) {
        startTime = millis();
      } else {
        if (lastSpeed == 0 && speed > 0) {
          currentTime = 0;
          startTime = millis();
          lastSpeed = speed;
          dir = irany;
        } else {
          if (irany == dir) {
            currentTime += millis() - startTime;
            startTime = millis();
            lastSpeed = speed;
          } else {
            currentTime = 0;
            dir = irany;
            startTime = millis();
            lastSpeed = speed;
          }
        }
      }
    }

    /**
     * Időtúllépés-e
     * @return - igaz, ha a motort le kell már tiltani. Hamis, ha a motor engedélyezve maradhat.
     */
    bool isOverTime() {
      if (en) return  currentTime >= maxTime;
      return false;
    }
};

/**
 * A mozdony fizikai vezérlését végző osztály.
 */
class Train: public Task  {
    String id;
    int speed;
    int target_speed;
    bool engine;
    int outA;
    int outB;
    bool forward;
    bool target_dir;
    bool gyorsulas;
    int gyorsulas_merteke;
    protectEngine motorVedelem;

  public:
    /**
     * Konstruktor
     * @name - A mozdony neve
     * @oA - Egyik kimeneti PIN
     * @oB - Másik kimeneti PIN
     */
    Train(String  name, int oA, int oB): id(name), engine(false), outA(oA), outB(oB), forward(false),
      target_dir(false), gyorsulas(true), gyorsulas_merteke(28) {
      pinMode(outA, OUTPUT);
      pinMode(outB, OUTPUT);
      this->reset();
    }

    /**
     * A mozdony alaphelyezetbe állítása.
     */
    void reset() {
      analogWrite(outB, 0);
      analogWrite(outA, 0);
      speed = MIN_SPEED;
      target_speed = MIN_SPEED;
      engine = false;
      forward = false;
      target_dir = false;
    }

    /**
     * Előre-hátramenet állítása
     * @fw - Ha false, akkor elpőremenet.
     */
    void setForward(const bool fw) {
      target_dir = fw;
      if (!gyorsulas) {
        forward = target_dir;
        controlEngine();
      }
    }

    /**
     * Motorvédelem bekapcsolása
     */
    void startProtectEngine() {
      motorVedelem.start();
    }

    /**
     * Motorvédelem kikapcsolása.
     */
    void stopProtectEngine() {
      motorVedelem.stop();
    }

    /**
     * Sebesség állítása
     * @sebesseg - az új motor sebesség
     */
    void setSpeed(int sebesseg) {
      if (sebesseg >= 0 && sebesseg <= 255) {
        int uj_sebesseg = map(sebesseg, 0, 255, MIN_SPEED, MAX_SPEED);
        if (gyorsulas) {
          target_speed = uj_sebesseg;
        }
        else speed = uj_sebesseg;
      }
      controlEngine();
    }

    /**
     * Motor be/ki kapcsolása
     * @mode - Ha igaz, akkor a motor be van kapcsolva.
     */
    void setEngine(bool mode) {
      engine = mode;
      controlEngine();
    }

    /**
     * A mozdony nevének a lekérdezésére.
     */
    String getName() {
      return id;
    }

    /**
     * Gyorsulás mód állítása.
     * @v - Ha igaz, akkor a gyorsulás mód be van kapcsolva.
     */
    void setAcceleration(bool v) {
      gyorsulas = v;
    }

    /**
     * Ha be van kapcsolva a fokozatos gyorsulás, akkor ez a függvény végzi a sebesség léptetését.
     */
    void loop() {
      if (gyorsulas) {
        if (forward != target_dir && speed <= MIN_SPEED) forward = target_dir;
        
        int szorzo = (forward == target_dir) ? 1 : -1;
        if (speed > (szorzo * target_speed)) speed -= 1;
        else if (speed < (szorzo * target_speed)) speed += 1;

        controlEngine();
        delay(gyorsulas_merteke);
      }
      else 
        delay(1000);
    }

  private:

    /**
     * A lábkimenetek vezérlését végző osztály
     */
    void controlEngine() {
      int motorA = (forward) ? speed : 0;
      int motorB = (forward) ? 0 : speed;
      if ( speed > MIN_SPEED)
        motorVedelem.set(forward, speed);
      else
        motorVedelem.set(forward, 0);

      if(motorVedelem.isOverTime()){
        motorA=0;
        motorB=0;
      }


      if (engine && speed > MIN_SPEED) {
        analogWrite(outA, motorA);
        analogWrite(outB, motorB);
      } else {
        analogWrite(outA, 0);
        analogWrite(outB, 0);
      }
    }
};

#endif
