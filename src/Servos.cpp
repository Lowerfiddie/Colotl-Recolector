#include "Colotl_config.h"
#include <ESP32Servo.h>

// Objetos definidos globalmente en Config.h
Servo servoIzq;
Servo servoDer;

const int SERVOI_ANGULO_INICIAL = 0;
const int SERVOD_ANGULO_INICIAL = 180;

void setupServos() {
  servoIzq.attach(myservoI);
  servoDer.attach(myservoD);

  servoIzq.write(SERVOI_ANGULO_INICIAL);
  servoDer.write(SERVOD_ANGULO_INICIAL);
}

void moverServos(int angle) {
  for (int pos = 0; pos <= angle; pos++) {
    int posInv = map(pos, 0, angle, 180, 180 - angle);
    servoIzq.write(pos);
    servoDer.write(posInv);
    delay(120);
  }
}

void abrirServos() {
  servoIzq.write(SERVOI_ANGULO_INICIAL);
  servoDer.write(SERVOD_ANGULO_INICIAL);
}