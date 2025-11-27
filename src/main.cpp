#include <Arduino.h>
#include "esp_camera.h"

// Nuestros nuevos módulos
#include "colotl_config.h"
#include "inference_handler.h"

// Configuraciones de imagen para la IA
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS           320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS           240
#define EI_CAMERA_FRAME_BYTE_SIZE                 3

// Variables Globales
uint8_t *snapshot_buf; 
static bool is_camera_init = false;

// --- CONFIGURACIÓN DE CÁMARA ---
// (Mantenemos esto aquí porque es inicialización de hardware)
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
    .jpeg_quality = 12, 
    .fb_count = 1,       
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

// Función auxiliar para capturar y convertir foto
bool capture_and_convert_image() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Error: Camera capture failed");
        return false;
    }

    // Convertir JPEG a RGB888 (El formato que necesita la IA)
    bool converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, snapshot_buf);
    esp_camera_fb_return(fb);

    if(!converted){
        Serial.println("Error: Conversion failed");
        return false;
    }
    
    // (Opcional) Aquí podrías agregar lógica de Resize si tu modelo no es 320x240
    // pero como usas crop, Edge Impulse lo maneja internamente en el signal.
    return true;
}

void setup() {
    Serial.begin(115200);
    
    // 1. Gestión de Memoria (PSRAM) - ¡CRÍTICO!
    if(psramFound()){
        Serial.println("\n--- PSRAM DETECTADA ---");
        heap_caps_malloc_extmem_enable(0); 
        Serial.println("Memoria redirigida: Todo va a PSRAM.");
    } else {
        Serial.println("ERROR CRÍTICO: No hay PSRAM.");
        return;
    }

    // 2. Inicializar Cámara
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed 0x%x\n", err);
        return;
    }
    
    // Ajustes de sensor (Flip, brillo, etc)
    sensor_t * s = esp_camera_sensor_get();
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1); 
        s->set_brightness(s, 1); 
        s->set_saturation(s, 0); 
    }
    is_camera_init = true;

    // 3. Asignar memoria para la foto una sola vez
    snapshot_buf = (uint8_t*)malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE);
    if(snapshot_buf == nullptr) {
        Serial.println("ERR: No memory for snapshot buffer");
    }

    print_model_info();
    Serial.println("Iniciando inferencia continua en 2 segundos...");
    delay(2000);
}

void loop() {
    if (!is_camera_init || snapshot_buf == nullptr) return;

    // A. Capturar Foto
    if (capture_and_convert_image()) {
        
        // B. Ejecutar IA (Ahora es solo una línea limpia)
        run_inference(snapshot_buf);
    }

    // Pequeña pausa para no saturar el log si es muy rápido
    delay(5); 
}