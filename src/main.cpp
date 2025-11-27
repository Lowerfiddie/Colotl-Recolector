#include <Arduino.h>
#include "esp_camera.h"

// Nuestros módulos
#include "colotl_config.h"
#include "inference_handler.h"

// --- VARIABLES GLOBALES DEFINIDAS AQUÍ ---
// (En el .h son 'extern', aquí las creamos de verdad)
DatosVision visionData; 
Estado estadoActual = INICIALIZACION;
bool tieneBasura = false;
uint8_t *snapshot_buf; 
static bool is_camera_init = false;

// Configuración de imagen
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS  320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS  240
#define EI_CAMERA_FRAME_BYTE_SIZE        3

// Configuración de Cámara
static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sccb_sda = SIOD_GPIO_NUM,
    .pin_sccb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG, 
    .frame_size = FRAMESIZE_QVGA,    
    .jpeg_quality = 8, 
    .fb_count = 2,       
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

// --- FUNCIONES AUXILIARES ---

// Función "webLog" simulada (Solo Serial por ahora)
void webLog(String mensaje) {
    Serial.println("[ESTADO]: " + mensaje);
}

// Captura de imagen
bool capture_and_convert_image() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) return false;
    bool converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, snapshot_buf);
    esp_camera_fb_return(fb);
    return converted;
}

// Lógica de actualización de IA
void updateIA() {
    if (is_camera_init && capture_and_convert_image()) {
        run_inference(snapshot_buf);
        // Al terminar, 'visionData' ya tiene los datos nuevos gracias a inference_handler.cpp
    }
}

// --- SETUP ---
void setup() {
    Serial.begin(115200);
    
    // 1. Memoria PSRAM
    if(psramFound()){
        Serial.println("\n--- PSRAM DETECTADA ---");
        heap_caps_malloc_extmem_enable(0); 
    } else {
        Serial.println("ERROR: No hay PSRAM.");
        return;
    }

    // 2. Hardware (Motores y Servos)
    // NOTA: Asegúrate que EN en colotl_config.h NO sea el pin 16 si usas PSRAM
    setupMotores();
    setupServos();

    // 3. Cámara
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        Serial.printf("Error camara 0x%x\n", err);
        return;
    }
    
    sensor_t * s = esp_camera_sensor_get();
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1); 
        s->set_brightness(s, 1); 
        s->set_saturation(s, 0); 
    }
    is_camera_init = true;
    s->set_brightness(s, 1);  // Sube brillo (-2 a 2), 0 es el predeterminado de todos, 2 es el valor maximo
    s->set_contrast(s, 1);    // Sube contraste (-2 a 2) -> Ayuda mucho a la IA a ver bordes
    s->set_saturation(s, 2);  // Sube saturación (-2 a 2) -> Ayuda si tu IA depende del color (ej. lata roja)

    // 4. Buffer de Foto
    snapshot_buf = (uint8_t*)malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE);
    
    print_model_info();
    delay(1000);
}

// --- LOOP PRINCIPAL (MÁQUINA DE ESTADOS) ---
void loop() {
    
    // Solo ejecutamos la IA si estamos buscando o acercándonos
    if (estadoActual == ESPERAR_BASURA || estadoActual == ACERCARSE) {
        updateIA(); 
    }

    switch (estadoActual) {
        
        case INICIALIZACION:
             webLog("Sistemas listos. Abriendo garra...");
             abrirServos(); 
             delay(2000); 
             estadoActual = ESPERAR_BASURA;
             break;

        case ESPERAR_BASURA:
        {
             if (visionData.detectado) {
                 String msg = "Detectado: " + visionData.label + " (Altura:" + String(visionData.h) + ")";
                 webLog(msg);

                 if (visionData.h < H_LEJOS) {
                     // Objeto muy lejos, acércate un poco recto
                     motores_avanzar();
                     delay(200); 
                     motores_detener();
                 } 
                 else if (visionData.h < H_CAPTURA) {
                     // Objeto en rango visual bueno, iniciar centrado fino
                     estadoActual = ACERCARSE;
                 } 
                 else {
                     // Objeto muy cerca (llenó la pantalla), ¡agarrar!
                     estadoActual = SUJETAR_BASURA;
                 }
             } else {
                 // No veo nada. Podrías hacer que gire lento para buscar.
                 // webLog("Buscando...");
                 //Serial.println("No hay nada");
             }
             break;
        }

        case ACERCARSE:
        {
             if (!visionData.detectado) {
                 webLog("Perdí el objetivo. Stop.");
                 motores_detener();
                 estadoActual = ESPERAR_BASURA;
                 break;
             }
             
             // Lógica de centrado (Izquierda/Derecha)
             if (visionData.x < (CENTRO_IMG_X - ZONA_MUERTA)) {
                 motores_girarIzquierda();
                 delay(80); // Toques cortos
                 motores_detener();
             } else if (visionData.x > (CENTRO_IMG_X + ZONA_MUERTA)) {
                 motores_girarDerecha();
                 delay(80);
                 motores_detener();
             } else {
                 // Está centrado, avanza hacia él
                 motores_avanzar();
                 delay(150);
                 motores_detener();
             }

             // Verificamos si ya llegamos
             if (visionData.h >= H_CAPTURA) {
                 motores_detener();
                 webLog("Objetivo en rango de garra.");
                 estadoActual = SUJETAR_BASURA;
             }
             break;
        }

        case SUJETAR_BASURA:
        {
             webLog("Cerrando garra...");
             motores_detener();
             // Ajusta '180' o el ángulo que cierre tu garra
             moverServos(60); 
             delay(1500); // Tiempo para que el servo llegue        
             
             tieneBasura = true;
             
             // Como no hay GPS, pasamos directo a simular la descarga
             // O podrías poner un estado de "Girar 180 grados" aquí
             webLog("Basura capturada. Simulando viaje a descarga...");
             delay(1000);
             estadoActual = DESCARGAR_BASURA; 
             break;
        }

        case IR_A_ZONA_DESCARGA:
        {
             // ESTADO OMITIDO POR AHORA (Sin GPS)
             // Simplemente pasamos al siguiente
             estadoActual = DESCARGAR_BASURA;
             break;
        }

        case DESCARGAR_BASURA:
        {
             webLog("Soltando basura...");
             abrirServos();
             delay(1500);
             
             // Retroceder un poco para no chocar con la basura dejada
             motores_avanzar(); // O retroceder si tienes esa función
             delay(500);
             motores_detener();

             tieneBasura = false;
             webLog("Ciclo completado. Reiniciando...");
             estadoActual = ESPERAR_BASURA; 
             break;
        }

        case REGRESAR_A_HOME:
        {
             // ESTADO OMITIDO POR AHORA (Sin GPS)
             estadoActual = ESPERAR_BASURA;
             break;
        }
    }
    
    // Pequeño delay para estabilidad del loop
    delay(10);
}