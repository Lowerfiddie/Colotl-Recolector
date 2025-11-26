#include "Colotl_config.h"
#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);
String consolaWeb = "--- COPITL V3 SISTEMA INICIADO ---\n";

// HTML Simple guardado en memoria Flash
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <title>COPITL DEBUG</title>
  <style>
    body { background: #111; color: #0f0; font-family: monospace; padding: 20px; }
    #console { border: 1px solid #444; height: 300px; overflow-y: scroll; padding: 10px; background: #000; white-space: pre-wrap;}
    .btn { padding: 10px 20px; background: #333; color: #fff; border: 1px solid #0f0; cursor: pointer; margin-top: 10px;}
  </style>
</head>
<body>
  <h2>COPITL TELEMETRÍA (AP MODE)</h2>
  <div id="console">Cargando datos...</div>
  <button class="btn" onclick="location.reload()">Refrescar</button>
  <button class="btn" onclick="fetch('/clear')">Limpiar</button>
  
  <script>
    setInterval(() => {
      fetch("/readlog").then(r => r.text()).then(d => {
        const c = document.getElementById("console");
        c.innerHTML = d;
        c.scrollTop = c.scrollHeight;
      });
    }, 1000); // Actualiza cada 1 segundo
  </script>
</body>
</html>
)rawliteral";

void setupWebDebug() {
  // Configurar como Access Point (El robot crea la red)
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  
  IPAddress IP = WiFi.softAPIP();
  //Serial.print("AP Iniciado. Conéctate a: ");
  //Serial.println(AP_SSID);
  //Serial.print("Entra en el navegador a: http://");
  //Serial.println(IP);

  // Rutas del servidor
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });

  server.on("/readlog", HTTP_GET, []() {
    server.send(200, "text/plain", consolaWeb);
  });
  
  server.on("/clear", HTTP_GET, []() {
    consolaWeb = "Log limpiado.\n";
    server.send(200, "text/plain", "OK");
  });

  server.begin();
}

void loopWebDebug() {
  server.handleClient();
}

void webLog(String mensaje) {
  String linea = String(millis()/1000) + "s: " + mensaje + "\n";
  consolaWeb += linea;
  
  // Limitar tamaño del log para no saturar RAM
  if (consolaWeb.length() > 1500) {
    consolaWeb = consolaWeb.substring(consolaWeb.length() - 1500);
  }
}