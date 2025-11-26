#include "Colotl_config.h"
#include <Proyecto_Copitl_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "esp_camera.h"

// ... (Pines de cámara estándar AI Thinker igual que siempre) ...
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

static bool debug_nn = false; 
static bool is_initialised = false;
static uint8_t *snapshot_buf = nullptr;

bool ei_camera_init(void);
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf);
static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr);

// Instancia global de los datos de visión
DatosVision visionData = {false, 0, 0, 0, 0, ""}; 

bool setupCameraIA() {
    if (!ei_camera_init()) return false;
    size_t snap_size = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT * 3;
    snapshot_buf = (uint8_t*)heap_caps_malloc(snap_size, MALLOC_CAP_SPIRAM);
    return snapshot_buf != nullptr;
}

// Función renombrada: Ahora solo "Actualiza" los datos, no hace el loop de control
void updateIA() {
    // 1. Capturar
    if (!ei_camera_capture(EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, snapshot_buf)) {
        return;
    }

    // 2. Inferir
    ei::signal_t signal;
    signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
    signal.get_data = &ei_camera_get_data;
    ei_impulse_result_t result = { 0 };

    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, debug_nn);
    if (res != EI_IMPULSE_OK) return;

    // 3. Guardar resultados en la variable global "visionData"
    visionData.detectado = false; // Reset

    #if EI_CLASSIFIER_OBJECT_DETECTION == 1
        for (size_t ix = 0; ix < result.bounding_boxes_count; ix++) {
            auto bb = result.bounding_boxes[ix];
            if (bb.value > 0.6) { // Confianza > 60%
                visionData.detectado = true;
                visionData.x = bb.x + (bb.width / 2.0f); // Centro X
                visionData.y = bb.y + (bb.height / 2.0f); // Centro Y
                visionData.w = bb.width;
                visionData.h = bb.height;
                visionData.label = String(bb.label);
                
                // Opcional: Loguear solo si detecta algo interesante
                // webLog("IA ve: " + visionData.label);
                break; // Nos quedamos con el primero (mayor confianza)
            }
        }
    #endif
}

// ... (Resto de funciones boilerplate de la cámara: init, capture, get_data se mantienen igual) ...
bool ei_camera_init(void) {
    if (is_initialised) return true;
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM; // Corregido sscb -> sccb
    config.pin_sccb_scl = SIOC_GPIO_NUM; // Corregido sscb -> sccb
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    if (esp_camera_init(&config) != ESP_OK) return false;
    
    sensor_t * s = esp_camera_sensor_get();
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1);
        s->set_brightness(s, 1);
        s->set_saturation(s, 0);
    }
    is_initialised = true;
    return true;
}

bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf) {
    if (!is_initialised) return false;
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) return false;

    bool converted = fmt2rgb888(fb->buf, fb->len, fb->format, snapshot_buf);
    esp_camera_fb_return(fb);
    if (!converted) return false;

    if ((int)img_width != EI_CLASSIFIER_INPUT_WIDTH) {
        ei::image::processing::crop_and_interpolate_rgb888(
            snapshot_buf,
            EI_CAMERA_RAW_FRAME_BUFFER_COLS,
            EI_CAMERA_RAW_FRAME_BUFFER_ROWS,
            out_buf,
            img_width,
            img_height);
    }
    return true;
}

static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr) {
    size_t pixel_ix = offset * 3;
    size_t pixels_left = length;
    size_t out_ix = 0;
    while (pixels_left != 0) {
        out_ptr[out_ix] = (snapshot_buf[pixel_ix] << 16) + (snapshot_buf[pixel_ix + 1] << 8) + snapshot_buf[pixel_ix + 2];
        out_ix++;
        pixel_ix += 3;
        pixels_left--;
    }
    return 0;
}