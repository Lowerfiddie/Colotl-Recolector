#ifndef INFERENCE_HANDLER_H
#define INFERENCE_HANDLER_H

#include <Arduino.h>
#include <stdint.h>

void print_model_info();

// Esta función ahora encapsula todo el ciclo: captura -> resize -> inferencia
void run_inference_cycle(uint8_t* snapshot_buf);

#endif