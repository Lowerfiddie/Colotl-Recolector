#ifndef INFERENCE_HANDLER_H
#define INFERENCE_HANDLER_H

#include <Arduino.h>
#include <stdint.h>

// Función para imprimir información del modelo al inicio
void print_model_info();

// Función principal para correr la IA
// Recibe el buffer de imagen, y sus dimensiones
void run_inference(uint8_t* buffer);

#endif