#include <Arduino.h>
#include <stdarg.h>
#include <WiFi.h>
#include <WebServer.h>
#include "colotl_config.h"

// --- CONFIGURACIÓN DE RED ---
const char* ssid = "Colotl_Debug";  // Nombre de la red WiFi del robot
const char* password = NULL;        // Sin contraseña para acceso rápido (o pon "12345678")

// IP FIJA (192.168.4.1 es el estándar para APs de ESP32, pero la forzamos aquí)
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

// Buffer para logs (guarda los últimos mensajes)
String logBuffer = "<h3>Inicio de Logs...</h3>";

// --- HTML DE LA PÁGINA (Sencillo, con auto-refresco de logs) ---
const char* html_page = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; background-color: #222; color: #fff; }
    #log-box { 
        width: 90%; height: 400px; margin: auto; 
        background: #000; border: 1px solid #444; 
        overflow-y: scroll; text-align: left; padding: 10px; font-family: monospace;
    }
    .btn { background-color: #c0392b; border: none; color: white; padding: 10px 20px; font-size: 16px; cursor: pointer; }
    .btn:active { background-color: #e74c3c; }
  </style>
  <script>
    setInterval(function() {
      fetch('/readLogs').then(response => response.text()).then(data => {
        document.getElementById("log-box").innerHTML = data;
        // Auto scroll al final
        var objDiv = document.getElementById("log-box");
        objDiv.scrollTop = objDiv.scrollHeight;
      });
    }, 1000); // Actualiza cada 1 segundo

    function limpiar() { fetch('/clear'); }
  </script>
</head>
<body>
  <h2>Colotl Debugger</h2>
  <div id="log-box">Cargando logs...</div>
  <br>
  <button class="btn" onclick="limpiar()">Limpiar Logs</button>
</body>
</html>
)rawliteral";

// --- FUNCIONES DEL SERVIDOR ---

void handleRoot() {
  server.send(200, "text/html", html_page);
}

void handleReadLogs() {
  server.send(200, "text/plain", logBuffer);
}

void handleClear() {
  logBuffer = "<h3>Logs limpiados</h3>";
  server.send(200, "text/plain", "OK");
}

void setupWebServer() {
  // Configurar IP Fija
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);

//   webLog("AP Iniciado.");
//   webLog("IP Address: ");
//   webLogPrintf(WiFi.softAPIP());

  // Rutas
  server.on("/", handleRoot);
  server.on("/readLogs", handleReadLogs);
  server.on("/clear", handleClear);

  server.begin();
  webLog("Servidor Web Iniciado: 192.168.4.1");
}

void serverLoop() {
  server.handleClient();
}

// --- IMPLEMENTACIÓN DE TU FUNCIÓN DE LOG ---
// Esta reemplaza los Serial.println. 
// Guarda en el buffer y evita que la RAM explote limitando el tamaño.
void webLog(String mensaje) {
  // Opcional: Seguir imprimiendo en Serial por si acaso
  //Serial.println(mensaje); 
  
  // Agregar al buffer HTML
  logBuffer += "<div>[" + String(millis()/1000) + "s] " + mensaje + "</div>";

  // Limpieza preventiva de memoria: si el log es muy largo, borra el inicio
  if (logBuffer.length() > 4000) {
    logBuffer = logBuffer.substring(2000);
    logBuffer = "<div>... (Logs antiguos truncados) ...</div>" + logBuffer;
  }
}

void webLogPrintf(const char *format, ...) {
  char buffer[256]; // Buffer temporal para el texto formateado
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args); // Convierte %d, %f, etc a texto
  va_end(args);
  
  // Manda el resultado a tu función webLog existente
  webLog(String(buffer));
}