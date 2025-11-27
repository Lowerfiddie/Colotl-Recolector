#include "inference_handler.h"

#include <Proyecto_Copitl_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"

static uint8_t *static_snapshot_buf = nullptr;

static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr)
{
    if (!static_snapshot_buf) return -1;
    size_t pixel_ix = offset * 3;
    size_t pixels_left = length;
    size_t out_ptr_ix = 0;

    while (pixels_left != 0) {
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
    // Ahora podemos usar estas constantes aquí dentro sin problemas
    ei_printf("Modelo Input: %d x %d\n", EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT);
}

// Ya no recibe width/height, los toma de la librería
void run_inference(uint8_t* buffer) {
    
    static_snapshot_buf = buffer;

    ei::signal_t signal;
    // Usamos las constantes internas de la librería
    signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
    signal.get_data = &ei_camera_get_data;

    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);

    if (err != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", err);
        return;
    }

    ei_printf("Timing -> DSP: %d ms, Class: %d ms, Anomaly: %d ms\n",
              result.timing.dsp, result.timing.classification, result.timing.anomaly);

#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    bool found = false;
    for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
        if (bb.value == 0) continue;
        
        ei_printf("  OBJETO: %s (%f) [ x: %u, y: %u, w: %u, h: %u ]\r\n",
                bb.label, bb.value, bb.x, bb.y, bb.width, bb.height);
        found = true;
    }
    if (!found) ei_printf("  No objects detected\n");
#else
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        ei_printf("  %s: %.5f\r\n", ei_classifier_inferencing_categories[i], result.classification[i].value);
    }
#endif
}