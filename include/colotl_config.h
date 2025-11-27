#ifndef COLOTL_CONFIG_H
#define COLOTL_CONFIG_H

#include "esp_camera.h"
#include <ESP32Servo.h>

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

#define CENTRO_IMG_X 48.0f // Mitad de 96
#define ZONA_MUERTA  10.0f // Tolerancia para centrar

// ==========================================
// 1. CONFIGURACIÓN DEL SISTEMA
// ==========================================
#define SERIAL_BAUD_RATE      115200

// ==========================================
// 2. CONFIGURACIÓN DE CÁMARA (AI THINKER)
// ==========================================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Configuración de imagen para la IA (Entrada del modelo)
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS     320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS     240
#define EI_CAMERA_FRAME_BYTE_SIZE           3

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

// --- DECLARACIÓN DE FUNCIONES GLOBALES ---
void webLog(String mensaje); // Para enviar texto a la web

// Motores
void motores_avanzar();
void motores_detener();
void motores_girarIzquierda();
void motores_girarDerecha();
void setupMotores();

// Servos
void moverServos(int angle);
void abrirServos();
void setupServos();

#endif // COLOTL_CONFIG_H