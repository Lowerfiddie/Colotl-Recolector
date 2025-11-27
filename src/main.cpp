#include <Arduino.h>
#include "esp_camera.h"
#include "colotl_config.h"
#include "inference_handler.h"

// --- VARIABLES GLOBALES ---
DatosVision visionData; 
Estado estadoActual = INICIALIZACION;
bool tieneBasura = false;
uint8_t *snapshot_buf; 

// CONFIGURACIÓN DE CÁMARA
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
    // QQVGA (160x120) para velocidad, Paara fiabilidad QVGA (320x240). Variables en colotl_config.h
    .frame_size = FRAMESIZE_QVGA,    
    .jpeg_quality = 8, 
    .fb_count = 2,       
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

void webLog(String mensaje) {
    Serial.println("[ESTADO]: " + mensaje);
}

void setup() {
    // Anti-Brownout
    #include "soc/soc.h"
    #include "soc/rtc_cntl_reg.h"
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    Serial.begin(115200);
    
    if(psramFound()){
        Serial.println("\n--- PSRAM DETECTADA ---");
        heap_caps_malloc_extmem_enable(0); 
    }

    setupMotores();
    setupServos();

    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        Serial.printf("Error camara 0x%x\n", err);
        return;
    }
    
    // Ajustes del sensor
    sensor_t * s = esp_camera_sensor_get();
    s->set_vflip(s, 0); // Ajustar según orientación física
    s->set_brightness(s, 0);
    s->set_contrast(s, 1);
    s->set_saturation(s, 2);

    // Buffer de Foto (Usando constantes de colotl_config.h)
    snapshot_buf = (uint8_t*)malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE);
    
    print_model_info();
    delay(1000);
}

void loop() {
    // EJECUCIÓN DE IA:
    // Llamamos a la función única que replica el comportamiento del ejemplo original
    if (estadoActual == ESPERAR_BASURA || estadoActual == ACERCARSE) {
        run_inference_cycle(snapshot_buf);
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
                 String msg = "Detectado: " + visionData.label + " H:" + String(visionData.h);
                 webLog(msg);

                 if (visionData.h < H_LEJOS) {
                     motores_avanzar();
                     delay(200); 
                     motores_detener();
                 } 
                 else if (visionData.h < H_CAPTURA) {
                     estadoActual = ACERCARSE;
                 } 
                 else {
                     estadoActual = SUJETAR_BASURA;
                 }
             } 
             break;
        }

        case ACERCARSE:
        {
             if (!visionData.detectado) {
                 webLog("Perdí objetivo.");
                 motores_detener();
                 estadoActual = ESPERAR_BASURA;
                 break;
             }
             
             if (visionData.x < (CENTRO_IMG_X - ZONA_MUERTA)) {
                 motores_girarIzquierda();
                 delay(80);
                 motores_detener();
             } else if (visionData.x > (CENTRO_IMG_X + ZONA_MUERTA)) {
                 motores_girarDerecha();
                 delay(80);
                 motores_detener();
             } else {
                 motores_avanzar();
                 delay(150);
                 motores_detener();
             }

             if (visionData.h >= H_CAPTURA) {
                 motores_detener();
                 estadoActual = SUJETAR_BASURA;
             }
             break;
        }

        case SUJETAR_BASURA:
        {
             webLog("Cerrando garra...");
             motores_detener();
             moverServos(60); 
             delay(1500);        
             tieneBasura = true;
             estadoActual = DESCARGAR_BASURA; 
             break;
        }

        case IR_A_ZONA_DESCARGA:
             estadoActual = DESCARGAR_BASURA;
             break;

        case DESCARGAR_BASURA:
        {
             webLog("Soltando...");
             abrirServos();
             delay(1500);
             motores_avanzar(); 
             delay(500);
             motores_detener();
             tieneBasura = false;
             estadoActual = ESPERAR_BASURA; 
             break;
        }

        case REGRESAR_A_HOME:
             estadoActual = ESPERAR_BASURA;
             break;
    }
    
    delay(10);
}