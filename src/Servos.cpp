#ifndef COLOTL_CONFIG_H
#define COLOTL_CONFIG_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// =========================================================
// 1. DEFINICIONES DE TIPOS (Deben ir SIEMPRE al principio)
// =========================================================

// Máquina de Estados
enum Estado {
  INICIALIZACION,
  ESPERAR_BASURA,
  ACERCARSE,
  SUJETAR_BASURA,
  IR_A_ZONA_DESCARGA,
  DESCARGAR_BASURA,
  REGRESAR_A_HOME
};

// Estructura de datos de visión
struct DatosVision {
    bool detectado;     
    float x;            
    float y;            
    float h;            
    float w;            
    String label;       
};

// =========================================================
// 2. VARIABLES GLOBALES (EXTERN)
// =========================================================
// Aquí declaramos que existen, pero se crean en los .cpp

extern DatosVision visionData; 
extern Estado estadoActual;

// TUS NOMBRES DE SERVOS:
extern Servo servoIzq;  // (Lo usaremos para el eje X / Pan)
extern Servo servoDer;  // (Lo usaremos para el eje Y / Tilt)
extern Servo servoGarra; 

extern bool tieneBasura; 

// Variables de Navegación
extern float dumpLat; 
extern float dumpLon;
extern float homeLat; 
extern float homeLon;

// =========================================================
// 3. CONFIGURACIÓN DE PINES Y RED
// =========================================================

#define WIFI_SSID "Totalplay-2.4G-4c08" 
#define WIFI_PASS "B4B45C4C08"

// --- PINES ---
#define PIN_ENABLE_PUENTE_H  16 
#define MOTOR_IZQ_PIN   14
#define MOTOR_DER_PIN   15
#define PIN_SERVO_GARRA 13  

// Pines para tus servos (Izq = Pan, Der = Tilt)
// ADVERTENCIA: El pin 12 puede causar problemas de arranque (Boot). 
// Si el robot no enciende con batería, desconecta este servo al prenderlo.
#define PIN_SERVO_IZQ  12 
#define PIN_SERVO_DER  2   

#define RX_PIN_GPS 3 
#define TX_PIN_GPS 1

// --- CONFIGURACIÓN VISIÓN ---
#define H_LEJOS       20  
#define H_CAPTURA     80  
#define CENTRO_IMG_X  48.0f 
#define ZONA_MUERTA   10.0f 

#define EI_CAMERA_RAW_FRAME_BUFFER_COLS     320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS     240
#define EI_CAMERA_FRAME_BYTE_SIZE           3
#define EI_CLASSIFIER_INPUT_WIDTH           96
#define EI_CLASSIFIER_INPUT_HEIGHT          96

// =========================================================
// 4. FUNCIONES
// =========================================================
void webLog(String mensaje);
void motores_avanzar();
void motores_detener();
void motores_girarIzquierda();
void motores_girarDerecha();
void servo_setAngle(int angle);
void garra_abrir();
void moverServosIA(int x_detectado, int y_detectado); // Función para mover la cabeza
bool getCurrentLocation(float &lat, float &lon);
float distanciaEnMetros(float lat1, float lon1, float lat2, float lon2);

#endif