#ifndef COLOTL_CONFIG_H
#define COLOTL_CONFIG_H

#include "esp_camera.h"

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

// ==========================================
// 3. SERVOS (Pan & Tilt)
// ==========================================
// NOTA: La ESP32-CAM tiene pocos pines libres si usas la tarjeta SD.
// Los pines 12, 13, 14, 15 suelen usarse para la SD.
// Si NO usas SD, puedes usarlos para servos.
#define SERVO_PAN_PIN         12  // Ejemplo (GPIO 12)
#define SERVO_TILT_PIN        13  // Ejemplo (GPIO 13)

// Límites de los servos (para evitar golpes mecánicos)
#define SERVO_PAN_MIN         0
#define SERVO_PAN_MAX         180
#define SERVO_PAN_CENTER      90

#define SERVO_TILT_MIN        45
#define SERVO_TILT_MAX        135
#define SERVO_TILT_CENTER     90

// ==========================================
// 4. GPS (UART)
// ==========================================
// El GPS necesita puerto Serial (RX/TX).
// Puedes usar Serial2 o SoftwareSerial en pines libres.
#define GPS_RX_PIN            14  // Ejemplo (GPIO 14)
#define GPS_TX_PIN            15  // Ejemplo (GPIO 15)
#define GPS_BAUD_RATE         9600

// ==========================================
// 5. MOTORES (H-Bridge L298N / Driver)
// ==========================================
// Si necesitas muchos pines, considera usar un expansor I2C (PCA9685)
// aquí definimos pines directos como ejemplo:
#define MOTOR_LEFT_FWD        2   // Cuidado: GPIO 2 es pin de boot
#define MOTOR_LEFT_BWD        4   // Cuidado: GPIO 4 es el Flash LED
// ...etc

#endif // COLOTL_CONFIG_H