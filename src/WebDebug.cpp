#include "Colotl_config.h"
#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);
String consolaWeb = "--- COPITL (MODO CLIENTE) ---\n";

// HTML (Se mantiene igual, solo cambié el título visual)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <title>COPITL CONTROL</title>
  <style>
    body { background: #111; color: #0f0; font-family: monospace; padding: 20px; text-align: center; }
    h2 { border-bottom: 1px solid #444; padding-bottom: 10px; }
    #console { border: 1px solid #444; height: 200px; overflow-y: scroll; padding: 10px; background: #000; white-space: pre-wrap; text-align: left; margin-bottom: 20px;}
    .btn { display: block; width: 100%; padding: 15px; margin: 10px 0; font-size: 18px; font-weight: bold; border: 2px solid #0f0; cursor: pointer; background: #222; color: #fff; border-radius: 5px; }
    .btn:active { background: #0f0; color: #000; }
    .btn-dump { border-color: #f00; color: #faa; }
    .btn-home { border-color: #0af; color: #aff; }
  </style>
</head>
<body>
  <h2>COPITL V3 - EN LÍNEA</h2>
  <div id="console">Conectando...</div>
  <button class="btn btn-home" onclick="guardarPunto('setHome')">📍 FIJAR CASA (HOME)</button>
  <button class="btn btn-dump" onclick="guardarPunto('setDump')">🗑️ FIJAR BASURERO</button>
  <br>
  <button class="btn" onclick="location.reload()">Refrescar</button>
  <button class="btn" onclick="fetch('/clear')">Limpiar Log</button>
  
  <script>
    function guardarPunto(endpoint) {
      fetch("/" + endpoint).then(r => r.text()).then(t => alert(t));
    }
    setInterval(() => {
      fetch("/readlog").then(r => r.text()).then(d => {
        const c = document.getElementById("console");
        c.innerHTML = d;
        c.scrollTop = c.scrollHeight;
      });
    }, 1000);
  </script>
</body>
</html>
)rawliteral";

void setupWebDebug() {
  // --- CAMBIO A MODO ESTACIÓN (CLIENTE) ---
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  Serial.print("Conectando a WiFi: ");
  Serial.println(WIFI_SSID);

  // Esperar conexión (Máximo 20 intentos para no bloquear el robot eternamente)
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConectado!");
      Serial.print("IP del Robot: ");
      Serial.println(WiFi.localIP()); // <--- IMPORTANTE: Ver esta IP en el Monitor Serial
      consolaWeb += "WiFi OK. IP: " + WiFi.localIP().toString() + "\n";
  } else {
      Serial.println("\nError: No se pudo conectar al WiFi.");
      consolaWeb += "Error WiFi. Trabajando Offline.\n";
  }

  // Rutas del servidor
  server.on("/", HTTP_GET, []() { server.send(200, "text/html", index_html); });
  server.on("/readlog", HTTP_GET, []() { server.send(200, "text/plain", consolaWeb); });
  server.on("/clear", HTTP_GET, []() { consolaWeb = "Log limpio.\n"; server.send(200, "text/plain", "OK"); });

  server.on("/setHome", HTTP_GET, []() {
      float lat, lon;
      if (getCurrentLocation(lat, lon)) {
          homeLat = lat; homeLon = lon;
          webLog("HOME: " + String(lat, 6) + ", " + String(lon, 6));
          server.send(200, "text/plain", "Home Guardado");
      } else {
          server.send(500, "text/plain", "Sin GPS");
      }
  });

  server.on("/setDump", HTTP_GET, []() {
      float lat, lon;
      if (getCurrentLocation(lat, lon)) {
          dumpLat = lat; dumpLon = lon;
          webLog("DUMP: " + String(lat, 6) + ", " + String(lon, 6));
          server.send(200, "text/plain", "Basurero Guardado");
      } else {
          server.send(500, "text/plain", "Sin GPS");
      }
  });

  server.begin();
}

void loopWebDebug() {
  server.handleClient();
}

void webLog(String mensaje) {
  String linea = String(millis()/1000) + "s: " + mensaje + "\n";
  consolaWeb += linea;
  if (consolaWeb.length() > 1500) consolaWeb = consolaWeb.substring(consolaWeb.length() - 1500);
}