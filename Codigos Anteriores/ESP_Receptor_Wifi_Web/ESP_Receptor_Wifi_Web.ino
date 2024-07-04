#include <WiFi.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include "esp_wifi.h"

// Defina o nome e a senha da rede Wi-Fi criada pelo ESP32
const char* ssid = "ESP_AP";
const char* password = "12345678";

// Crie um servidor web na porta 80
WebServer server(80);

void setup() {
  Serial.begin(115200); // Inicializa a comunicação serial
  
  // Configura o ESP32 como Access Point
  WiFi.softAP(ssid, password);
  
  Serial.println("Access Point Started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Inicializa o sistema de arquivos SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Rota para a página principal
  server.on("/", HTTP_GET, handleRoot);
  // Rota para baixar um arquivo
  server.on("/download", HTTP_GET, handleDownload);
  // Rota para lidar com uploads
  server.on("/upload", HTTP_POST, []() {
    server.send(200);
  }, handleFileUpload);
  // Rota para ver dispositivos conectados
  server.on("/connected_devices", HTTP_GET, handleConnectedDevices);

  server.begin(); // Inicia o servidor
  Serial.println("Server started");
}

void loop() {
  server.handleClient(); // Lida com as requisições do cliente
}

void handleRoot() {
  String html = "<html><body>";
  html += "<h1>ESP32 File Receiver</h1>";
  html += "<form method='POST' action='/upload' enctype='multipart/form-data'>";
  html += "<input type='file' name='upload'><br>";
  html += "<input type='submit' value='Upload'>";
  html += "</form>";
  html += "<h2>Files:</h2><ul>";

  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    html += "<li><a href='/download?file=";
    html += file.name();
    html += "'>";
    html += file.name();
    html += "</a></li>";
    file = root.openNextFile();
  }
  html += "</ul>";
  html += "<h2>Connected Devices:</h2>";
  html += "<a href='/connected_devices'>View Connected Devices</a>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleDownload() {
  if (server.args() == 0) {
    server.send(400, "text/plain", "Bad Request");
    return;
  }
  String path = server.arg("file");
  if (!SPIFFS.exists(path)) {
    server.send(404, "text/plain", "File Not Found");
    return;
  }
  File file = SPIFFS.open(path, "r");
  server.streamFile(file, "application/octet-stream");
  file.close();
}

void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = "/" + upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    File file = SPIFFS.open(filename, FILE_WRITE);
    if (!file) {
      return;
    }
    file.close();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    String filename = "/" + upload.filename;
    File file = SPIFFS.open(filename, FILE_APPEND);
    if (file) {
      file.write(upload.buf, upload.currentSize);
      file.close();
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    String filename = "/" + upload.filename;
    File file = SPIFFS.open(filename, FILE_APPEND);
    if (file) {
      file.close();
    }
  }
}

void handleConnectedDevices() {
  String html = "<html><body>";
  html += "<h1>Connected Devices</h1>";
  html += "<ul>";

  wifi_sta_list_t wifi_sta_list;
  esp_wifi_ap_get_sta_list(&wifi_sta_list);

  for (int i = 0; i < wifi_sta_list.num; i++) {
    wifi_sta_info_t sta_info = wifi_sta_list.sta[i];
    html += "<li>";
    html += "MAC: ";
    html += String(sta_info.mac[0], HEX) + ":" + String(sta_info.mac[1], HEX) + ":" + String(sta_info.mac[2], HEX) + ":" + String(sta_info.mac[3], HEX) + ":" + String(sta_info.mac[4], HEX) + ":" + String(sta_info.mac[5], HEX);
    html += " - RSSI: ";
    html += String(sta_info.rssi);
    html += "</li>";
  }

  html += "</ul>";
  html += "<a href='/'>Back to Home</a>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}
