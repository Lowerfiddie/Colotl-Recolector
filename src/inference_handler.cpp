#include "inference_handler.h"
#include <Proyecto_Copitl_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "esp_camera.h"
#include "colotl_config.h"

// Buffer estático interno para el callback
static uint8_t *static_snapshot_buf = nullptr;

// Callback IDÉNTICO al original: Convierte BGR a RGB
static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr)
{
    if (!static_snapshot_buf) return -1;
    size_t pixel_ix = offset * 3;
    size_t pixels_left = length;
    size_t out_ptr_ix = 0;

    while (pixels_left != 0) {
        // Swap BGR to RGB here (Lógica original)
        out_ptr[out_ptr_ix] = (static_snapshot_buf[pixel_ix + 2] << 16) + 
                              (static_snapshot_buf[pixel_ix + 1] << 8) + 
                              static_snapshot_buf[pixel_ix];
        out_ptr_ix++;
        pixel_ix += 3;
        pixels_left--;
    }
    return 0;
}

void print_model_info() {
    ei_printf("Edge Impulse Inferencing Demo\n");
}

// Función wrapper que imita el flujo del loop() original
// Recibe el buffer, captura la foto, la recorta y corre la IA.
void run_inference_cycle(uint8_t* snapshot_buf) {
    
    // 1. CAPTURA DE IMAGEN (Lógica original ei_camera_capture)
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ei_printf("Camera capture failed\n");
        return;
    }

    // Convertir JPEG a RGB888 en el buffer
    bool converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, snapshot_buf);
    esp_camera_fb_return(fb);

    if (!converted) {
        ei_printf("Conversion failed\n");
        return;
    }

    // 2. REDIMENSIONADO (Lógica original de resize/crop)
    // Siempre se ejecuta si el tamaño de la cámara difiere del modelo
    if ((EI_CAMERA_RAW_FRAME_BUFFER_COLS != EI_CLASSIFIER_INPUT_WIDTH) || 
        (EI_CAMERA_RAW_FRAME_BUFFER_ROWS != EI_CLASSIFIER_INPUT_HEIGHT)) {
        
        ei::image::processing::crop_and_interpolate_rgb888(
            snapshot_buf,
            EI_CAMERA_RAW_FRAME_BUFFER_COLS,
            EI_CAMERA_RAW_FRAME_BUFFER_ROWS,
            snapshot_buf,
            EI_CLASSIFIER_INPUT_WIDTH,
            EI_CLASSIFIER_INPUT_HEIGHT
        );
    }

    // 3. INFERENCIA (Lógica original run_classifier)
    static_snapshot_buf = snapshot_buf;

    ei::signal_t signal;
    signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
    signal.get_data = &ei_camera_get_data;

    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false); // debug_nn = false

    if (err != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", err);
        return;
    }

    // 4. IMPRESIÓN DE RESULTADOS (Idéntico al original)
    ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
                result.timing.dsp, result.timing.classification, result.timing.anomaly);

    // Resetear datos globales
    visionData.detectado = false; 

#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    bool found = false;
    for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
        
        // FILTRO ORIGINAL: Solo ignorar si valor es 0
        if (bb.value == 0) continue; 
        
        ei_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                bb.label, bb.value, bb.x, bb.y, bb.width, bb.height);
        found = true;

        // Llenar estructura compartida (esto es lo único extra para que funcione tu robot)
        if (!visionData.detectado) {
            visionData.detectado = true;
            visionData.label = String(bb.label);
            visionData.x = bb.x + (bb.width / 2);
            visionData.y = bb.y + (bb.height / 2);
            visionData.w = bb.width;
            visionData.h = bb.height;
        }
    }
    if (!found) {
        ei_printf("    No objects detected\n");
    }
#else
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        ei_printf("  %s: %.5f\r\n", ei_classifier_inferencing_categories[i], result.classification[i].value);
    }
#endif
}