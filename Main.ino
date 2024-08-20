#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <WebSocketsServer.h>
#include <DHT.h>

#define DHTPIN D1       // Pin al que está conectado el DHT11
#define DHTTYPE DHT11   // Tipo de sensor
#define LED_PIN D2      // Pin al que está conectado el LED

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81); // Configura WebSocket en el puerto 81

DHT dht(DHTPIN, DHTTYPE);

// Función para servir archivos desde LittleFS
void handleFileRead(String path) {
  if (path.endsWith("/")) path += "index.html";
  String contentType = "text/plain";
  if (path.endsWith(".html")) contentType = "text/html";
  else if (path.endsWith(".css")) contentType = "text/css";
  else if (path.endsWith(".js")) contentType = "application/javascript";
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
  } else {
    server.send(404, "text/plain", "File Not Found");
  }
}

// Función para manejar la ruta /data
void handleData() {
  String param1 = server.arg("param1");
  String param2 = server.arg("param2");
  String response = "Received parameters:\n";
  response += "param1: " + param1 + "\n";
  response += "param2: " + param2;
  server.send(200, "text/plain", response);
}

// Función para manejar el estado del LED
void handleLED() {
  String state = server.arg("state");
  if (state == "on") {
    digitalWrite(LED_PIN, HIGH);
    server.send(200, "text/plain", "LED encendido");
  } else if (state == "off") {
    digitalWrite(LED_PIN, LOW);
    server.send(200, "text/plain", "LED apagado");
  } else {
    server.send(400, "text/plain", "Parámetro 'state' inválido. Usa 'on' o 'off'.");
  }
}

// Función para desconectar de la red WiFi, olvidar la configuración y reiniciar
void handleDisconnect() {
  WiFi.disconnect(); // Desconecta de la red actual
  WiFiManager wifiManager;
  wifiManager.resetSettings(); // Olvida la configuración de WiFi guardada
  server.send(200, "text/plain", "Desconectado y configuración olvidada. Reiniciando...");
  delay(1000); // Espera un momento para asegurarse de que el mensaje sea enviado
  ESP.restart(); // Reinicia el ESP8266
}

// Función de manejo de eventos del WebSocket
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  String message;

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("Cliente %u desconectado\n", num);
      break;
    case WStype_CONNECTED:
      Serial.printf("Cliente %u conectado\n", num);
      break;
    case WStype_TEXT:
      Serial.printf("Mensaje recibido de %u: %s\n", num, payload);
      {
        // Bloque para evitar el error de cruzar la etiqueta
        message = "Mensaje recibido: ";
        message += String((char*)payload);
        webSocket.sendTXT(num, message);
      }
      break;
    case WStype_ERROR:
      Serial.printf("Error en WebSocket de cliente %u\n", num);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  // Configurar el pin del LED como salida
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Apagar el LED al inicio

  // Configurar WiFiManager
  WiFiManager wifiManager;

  // Intentar conectar a la red WiFi guardada
  if (!wifiManager.autoConnect("ESP8266-AP")) {
    Serial.println("Fallo al conectar al WiFi");
    ESP.restart(); // Reinicia el ESP8266 para intentar nuevamente
  }

  Serial.println("Conectado a la red WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Montar LittleFS
  if (!LittleFS.begin()) {
    Serial.println("Fallo al montar el sistema de archivos");
    return;
  }

  // Configurar manejadores de rutas
  server.on("/", []() { handleFileRead("/index.html"); });
  server.onNotFound([]() { handleFileRead(server.uri()); });

  // Ruta para manejar datos
  server.on("/data", HTTP_GET, handleData);

  // Ruta para controlar el LED
  server.on("/led", HTTP_GET, handleLED);

  // Ruta para desconectar y olvidar la red
  server.on("/disconnect", HTTP_GET, handleDisconnect);

  // Iniciar el servidor HTTP
  server.begin();
  Serial.println("Servidor HTTP iniciado");

  // Iniciar el servidor WebSocket
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // Inicializar el DHT11
  dht.begin();
  delay(5000);
}

void loop() {
  server.handleClient();
  webSocket.loop();

  // Leer temperatura y humedad del DHT11
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Verifica si las lecturas fallaron
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Error al leer del sensor DHT11");
    Serial.print("Humedad: ");
    Serial.print(humidity);
    Serial.print(" %\t");
    Serial.print("Temperatura: ");
    Serial.println(temperature);
    delay(5000);
    return;
  }

  // Crear mensaje con los datos del sensor
  String message = "Temperatura: " + String(temperature) + " °C, Humedad: " + String(humidity) + " %";

  // Enviar mensaje a todos los clientes WebSocket conectados
  webSocket.broadcastTXT(message);

  // Espera antes de la próxima lectura
  delay(1500); // Cambia el intervalo según tus necesidades
}
