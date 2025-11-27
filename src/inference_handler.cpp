#include "inference_handler.h"
#include <Proyecto_Copitl_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "colotl_config.h" 

static uint8_t *static_snapshot_buf = nullptr;

// Esta función es IDÉNTICA a la de tu main.cpp original
// Realiza la conversión de BGR a RGB necesaria para la ESP32-CAM
static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr)
{
    if (!static_snapshot_buf) return -1;
    size_t pixel_ix = offset * 3;
    size_t pixels_left = length;
    size_t out_ptr_ix = 0;

    while (pixels_left != 0) {
        // Swap BGR to RGB here (Copia exacta de tu repositorio)
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
    ei_printf("Modelo Input: %d x %d\n", EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT);
}

void run_inference(uint8_t* buffer) {
    static_snapshot_buf = buffer;

    ei::signal_t signal;
    signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
    signal.get_data = &ei_camera_get_data;

    ei_impulse_result_t result = { 0 };
    // debug_nn en false, igual que en tu variable estática original
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);

    if (err != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", err);
        return;
    }

    // Imprimimos tiempos para verificar que está vivo (igual que tu código original)
    ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
                result.timing.dsp, result.timing.classification, result.timing.anomaly);

    // Resetear detección
    visionData.detectado = false; 

#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    bool found_print = false; // Variable auxiliar para replicar tu lógica de impresión "No objects detected"

    for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
        
        // --- LÓGICA EXACTA DE TU REPOSITORIO ---
        // En tu main.cpp original SOLO tenías este filtro:
        if (bb.value == 0) continue; 
        // ---------------------------------------

        // Imprimimos en Serial tal cual lo hacía tu código original
        ei_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                bb.label,
                bb.value,
                bb.x,
                bb.y,
                bb.width,
                bb.height);
        
        found_print = true;

        // Llenamos la estructura para la máquina de estados
        // (Tomamos el objeto con mayor confianza o el primero que aparezca)
        if (!visionData.detectado) { // Solo guardamos el primero válido para no sobrescribir
            visionData.detectado = true;
            visionData.label = String(bb.label);
            // Calculamos el centro igual que antes
            visionData.x = bb.x + (bb.width / 2); 
            visionData.y = bb.y + (bb.height / 2); 
            visionData.w = bb.width;
            visionData.h = bb.height;
        }
    }

    // Si no encontró nada, imprimimos el mensaje clásico de tu repo
    if (!found_print) {
        ei_printf("    No objects detected\n");
    }
#endif
}