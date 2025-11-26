#include "Colotl_config.h"
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <math.h> // Necesaria para los calculos de distancia

TinyGPSPlus gps;
SoftwareSerial ss(RX_PIN_GPS, TX_PIN_GPS);

// --- SETUP ---
void setupGPS() {
    ss.begin(9600);
    // Si usas un botón físico para debug local, configúralo aquí, 
    // sino, la configuración web es suficiente.
    // pinMode(BTN_GUARDAR_UBI, INPUT_PULLUP); 
    
    webLog("GPS Serial Iniciado");
}

// --- LOOP ---
void loopGPS() {
    // Es VITAL que esto corra rápido y seguido para no perder datos seriales
    while (ss.available() > 0) {
        gps.encode(ss.read());
    }
}

// --- FUNCIONES FALTANTES QUE CAUSABAN EL ERROR ---

// 1. Obtener ubicación actual
// Devuelve 'true' si el GPS tiene señal válida, y guarda los datos en las variables lat y lon
bool getCurrentLocation(float &lat, float &lon) {
    if (gps.location.isValid()) {
        lat = gps.location.lat();
        lon = gps.location.lng();
        return true;
    }
    return false;
}

// 2. Calcular distancia (Fórmula de Haversine)
// Calcula los metros entre dos puntos geográficos
float distanciaEnMetros(float lat1, float lon1, float lat2, float lon2) {
    // Radio de la Tierra en metros
    const float R = 6371000; 
    
    // Convertir grados a radianes
    float phi1 = lat1 * M_PI / 180.0;
    float phi2 = lat2 * M_PI / 180.0;
    float deltaPhi = (lat2 - lat1) * M_PI / 180.0;
    float deltaLambda = (lon2 - lon1) * M_PI / 180.0;

    // Fórmula matemática
    float a = sin(deltaPhi / 2) * sin(deltaPhi / 2) +
              cos(phi1) * cos(phi2) *
              sin(deltaLambda / 2) * sin(deltaLambda / 2);
              
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return R * c; // Distancia en metros
}