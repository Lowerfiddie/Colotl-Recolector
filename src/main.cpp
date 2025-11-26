#include <Arduino.h>
#include "Colotl_config.h"

// Declarar funciones de setup de otros archivos
void setupWebDebug();
void loopWebDebug();
bool setupCameraIA();
void updateIA();
void setupServos();
void setupMotores();
void setupGPS();
void loopGPS();

// Variables de estado globales
Estado estadoActual = INICIALIZACION;
bool tieneBasura = false;
float lat = 0, lon = 0; // Variables temporales GPS

// --- COORDENADAS INICIALES (Valores por defecto) ---
// Se actualizarán cuando pulses los botones en la web
float homeLat = 0.0;
float homeLon = 0.0;
float dumpLat = 0.0;
float dumpLon = 0.0;

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    //Serial.begin(115200);
    
    setupWebDebug();
    setupMotores();
    setupServos();
    setupGPS();

    if (setupCameraIA()) {
        webLog("Sistema Iniciado Correctamente");
    } else {
        webLog("Error Fatal: Camara");
    }
}

void loop() {
loopWebDebug();
    loopGPS();
    
    if (estadoActual == ESPERAR_BASURA || estadoActual == ACERCARSE) {
        updateIA(); 
    }

    switch (estadoActual) {
        
        case INICIALIZACION:
             webLog("Esperando configuración GPS...");
             // Esperamos un poco antes de arrancar lógica
             delay(2000); 
             estadoActual = ESPERAR_BASURA;
             break;

        case ESPERAR_BASURA:
        {
             if (visionData.detectado) {
                 String msg = "Visto: " + visionData.label + " H:" + String(visionData.h);
                 webLog(msg);

                 if (visionData.h < H_LEJOS) {
                     motores_avanzar();
                     delay(300); 
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
                 webLog("Perdí el objetivo. Stop.");
                 motores_detener();
                 estadoActual = ESPERAR_BASURA;
                 break;
             }
             
             if (visionData.x < (CENTRO_IMG_X - ZONA_MUERTA)) {
                 motores_girarIzquierda();
             } else if (visionData.x > (CENTRO_IMG_X + ZONA_MUERTA)) {
                 motores_girarDerecha();
             } else {
                 motores_avanzar();
             }

             if (visionData.h >= H_CAPTURA) {
                 motores_detener();
                 webLog("Objetivo alcanzado.");
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
             estadoActual = IR_A_ZONA_DESCARGA;
             break;
        }

        case IR_A_ZONA_DESCARGA:
        {
             // Verificamos que el usuario haya guardado una ubicación válida
             if (dumpLat == 0.0) {
                webLog("ERROR: ¡No se ha definido Zona Descarga!");
                motores_detener();
                delay(2000);
                break;
             }

             if (getCurrentLocation(lat, lon)) {
                 float d = distanciaEnMetros(lat, lon, dumpLat, dumpLon);
                 
                 if (d > 3.0f) {
                     webLog("Yendo a Descarga (" + String(d) + "m)");
                     motores_avanzar();
                 } else {
                     motores_detener();
                     estadoActual = DESCARGAR_BASURA;
                 }
             }
             break;
        }

        case DESCARGAR_BASURA:
        {
             webLog("Soltando basura...");
             abrirServos();
             delay(1500);
             tieneBasura = false;
             estadoActual = REGRESAR_A_HOME;
             break;
        }

        case REGRESAR_A_HOME:
        {
             if (homeLat == 0.0) {
                webLog("ERROR: ¡No se ha definido HOME!");
                motores_detener();
                break;
             }

             if (getCurrentLocation(lat, lon)) {
                 float d = distanciaEnMetros(lat, lon, homeLat, homeLon);
                 
                 if (d > 3.0f) {
                     webLog("Volviendo a Casa (" + String(d) + "m)");
                     motores_avanzar();
                 } else {
                     motores_detener();
                     webLog("Llegada a Casa. Ciclo terminado.");
                     estadoActual = ESPERAR_BASURA;
                 }
             }
             break;
        }
    }
    delay(10);
}