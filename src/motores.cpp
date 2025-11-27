#include <Arduino.h>
#include "colotl_config.h"

void setupMotores() {
  // 1. Configuramos los 4 canales PWM
  ledcSetup(PWM_CH_IN1, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_IN2, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_IN3, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_IN4, PWM_FREQ, PWM_RES);

  // 2. Conectamos los canales a los pines físicos
  ledcAttachPin(IN1, PWM_CH_IN1);
  ledcAttachPin(IN2, PWM_CH_IN2);
  ledcAttachPin(IN3, PWM_CH_IN3);
  ledcAttachPin(IN4, PWM_CH_IN4);

  motores_detener();
}

void motores_detener() {
  // Poner todo a 0 frena los motores
  ledcWrite(PWM_CH_IN1, 0);
  ledcWrite(PWM_CH_IN2, 0);
  ledcWrite(PWM_CH_IN3, 0);
  ledcWrite(PWM_CH_IN4, 0);
}

void motores_avanzar() {
  // Motor Izquierdo: Avanza
  ledcWrite(PWM_CH_IN1, VELOCIDAD_BASE);
  ledcWrite(PWM_CH_IN2, 0);

  // Motor Derecho: Avanza
  ledcWrite(PWM_CH_IN3, VELOCIDAD_BASE);
  ledcWrite(PWM_CH_IN4, 0);
}

void motores_girarIzquierda() {
  // Giro sobre su eje: Izquierda atrás, Derecha adelante
  ledcWrite(PWM_CH_IN1, 0);
  ledcWrite(PWM_CH_IN2, VELOCIDAD_BASE); // Retrocede

  ledcWrite(PWM_CH_IN3, VELOCIDAD_BASE); // Avanza
  ledcWrite(PWM_CH_IN4, 0);
}

void motores_girarDerecha() {
  // Giro sobre su eje: Izquierda adelante, Derecha atrás
  ledcWrite(PWM_CH_IN1, VELOCIDAD_BASE); // Avanza
  ledcWrite(PWM_CH_IN2, 0);

  ledcWrite(PWM_CH_IN3, 0);
  ledcWrite(PWM_CH_IN4, VELOCIDAD_BASE); // Retrocede
}