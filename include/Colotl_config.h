#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// --- CREDENCIALES WIFI (MODO AP) ---
// El robot creará esta red. Conéctate a ella con tu celular.
#define AP_SSID "COLOTL_ROBOT"
#define AP_PASS "12345678"

// --- ESTRUCTURA PARA COMPARTIR DATOS ---
// La cámara escribe aquí, el Main lee de aquí
struct DatosVision {
    bool detectado;     // ¿Vio algo?
    float x;            // Centro X
    float y;            // Centro Y
    float h;            // Altura (para saber distancia)
    float w;            // Ancho
    String label;       // Qué es (botella, lata, etc)
};

// --- MÁQUINA DE ESTADOS ---
enum Estado {
  INICIALIZACION,
  ESPERAR_BASURA,
  ACERCARSE,
  SUJETAR_BASURA,
  IR_A_ZONA_DESCARGA,
  DESCARGAR_BASURA,
  REGRESAR_A_HOME
};

// Variables globales compartidas (externas)
extern Estado estadoActual;
extern Servo servoIzq;
extern Servo servoDer;

extern DatosVision visionData; 
extern Estado estadoActual;
extern Servo servoGarra; 
extern bool tieneBasura; // Variable de estado para la lógica

// --- CONSTANTES DE NAVEGACIÓN Y VISIÓN ---
#define H_LEJOS    20  // Altura del objeto en pixeles (lejos)
#define H_CAPTURA  80  // Altura del objeto en pixeles (listo para agarrar)

#define H_LEJOS    20  
#define H_CAPTURA  80  
#define CENTRO_IMG_X 48.0f // Mitad de 96
#define ZONA_MUERTA  10.0f // Tolerancia para centrar

extern float dumpLat; 
extern float dumpLon;
extern float homeLat; 
extern float homeLon;

// --- PINES MOTORES (PUENTE H) ---
// ADVERTENCIA: En ESP32-CAM, el Pin 16 se usa a veces para la PSRAM.
// Si la cámara falla o se reinicia, cambia este pin a 12 o 2.
#define EN  16 

#define IN1 12
#define IN2 13
#define IN3 14
#define IN4 15

#define PWM_CHANNEL 0
#define PWM_FREQUENCY 5000
#define PWM_RESOLUTION 8  // 0–255

// --- PINES SERVOS ---
// Usamos pines que suelen estar libres si no usas la tarjeta SD
#define myservoI 2
#define myservoD 4

// --- PINES GPS ---
#define RX_PIN_GPS 3 
#define TX_PIN_GPS 1
#define BTN_GUARDAR_UBI 2 // Botón opcional (Pin 2 es LED interno también)

// --- CONFIGURACIÓN IA ---
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS     320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS     240
#define EI_CAMERA_FRAME_BYTE_SIZE           3
#define EI_CLASSIFIER_INPUT_WIDTH           96
#define EI_CLASSIFIER_INPUT_HEIGHT          96

// --- DECLARACIÓN DE FUNCIONES GLOBALES ---
void webLog(String mensaje); // Para enviar texto a la web

// Motores
void motores_avanzar();
void motores_detener();
void motores_girarIzquierda();
void motores_girarDerecha();

// Servos
void moverServos(int angle);
void abrirServos();

// GPS
bool getCurrentLocation(float &lat, float &lon);
float distanciaEnMetros(float lat1, float lon1, float lat2, float lon2);

#endif