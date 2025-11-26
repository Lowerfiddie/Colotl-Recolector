#ifndef COLOTL_CONFIG_H
#define COLOTL_CONFIG_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// --- CREDENCIALES WIFI (MODO ESTACIÓN) ---
// El robot se conectará a esta red:
#define WIFI_SSID "Santiago's Zimmer" 
#define WIFI_PASS "Blakiywhite2015"

// --- ESTRUCTURA PARA COMPARTIR DATOS ---
struct DatosVision {
    bool detectado;     
    float x;            
    float y;            
    float h;            
    float w;            
    String label;       
};

// Variables globales compartidas
extern DatosVision visionData; 
extern Estado estadoActual;
extern Servo servoGarra; 
extern Servo servoX; // Agregué esta referencia que faltaba
extern Servo servoY; // Agregué esta referencia que faltaba
extern bool tieneBasura; 

// --- VARIABLES DE NAVEGACIÓN (MODIFICABLES VÍA WEB) ---
extern float dumpLat; 
extern float dumpLon;
extern float homeLat; 
extern float homeLon;

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

// --- CONSTANTES FIJAS ---
#define H_LEJOS    20  
#define H_CAPTURA  80  
#define CENTRO_IMG_X 48.0f 
#define ZONA_MUERTA  10.0f 

// --- PINES ---
#define PIN_ENABLE_PUENTE_H  16 
#define MOTOR_IZQ_PIN   14
#define MOTOR_DER_PIN   15
#define PIN_SERVO_GARRA 13  

// ATENCIÓN AQUI: Si sigue fallando el arranque con batería, 
// cambia PIN_SERVO_X al pin 2 o 4.
#define myservoI  2 
#define myservoD  4   

#define RX_PIN_GPS 3 
#define TX_PIN_GPS 1

// --- CONFIGURACIÓN IA ---
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS     320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS     240
#define EI_CAMERA_FRAME_BYTE_SIZE           3
#define EI_CLASSIFIER_INPUT_WIDTH           96
#define EI_CLASSIFIER_INPUT_HEIGHT          96

// --- FUNCIONES GLOBALES ---
void webLog(String mensaje);
void motores_avanzar();
void motores_detener();
void motores_girarIzquierda();
void motores_girarDerecha();
void moverServos(int angle);
void abrirServos();
bool getCurrentLocation(float &lat, float &lon);
float distanciaEnMetros(float lat1, float lon1, float lat2, float lon2);

#endif