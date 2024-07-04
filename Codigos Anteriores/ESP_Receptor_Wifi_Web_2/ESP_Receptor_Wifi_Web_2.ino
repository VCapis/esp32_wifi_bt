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
  // Rota para retornar a lista de dispositivos conectados em formato JSON
  server.on("/get_connected_devices", HTTP_GET, handleGetConnectedDevices);

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
  html += "<ul id='deviceList'></ul>";
  html += "<script>";
  html += "function updateDeviceList() {";
  html += "  fetch('/get_connected_devices').then(response => response.json()).then(data => {";
  html += "    let deviceList = document.getElementById('deviceList');";
  html += "    deviceList.innerHTML = '';"; // Clear current list
  html += "    data.forEach(device => {";
  html += "      let li = document.createElement('li');";
  html += "      li.textContent = 'MAC: ' + device.mac + ' - RSSI: ' + device.rssi;";
  html += "      deviceList.appendChild(li);";
  html += "    });";
  html += "  });";
  html += "}";
  html += "setInterval(updateDeviceList, 1000);"; // Update every second
  html += "updateDeviceList();"; // Initial call
  html += "</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleDownload() {
  if (server.args() == 0) {
    server.send(400, "text/plain", "Bad Request");
    return;
  }
  String path = server.arg("file");
  if (path[0] != '/') {
    path = "/" + path;
  }
  if (!SPIFFS.exists(path)) {
    server.send(404, "text/plain", "File Not Found");
    return;
  }
  File file = SPIFFS.open(path, "r");
  if (file) {
    server.streamFile(file, getContentType(path));
    file.close();
  } else {
    server.send(500, "text/plain", "Internal Server Error");
  }
}

String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/pdf";
  else if (filename.endsWith(".zip")) return "application/zip";
  return "text/plain";
}

void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = "/" + upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    Serial.print("Upload Start: ");
    Serial.println(filename);
    File file = SPIFFS.open(filename, FILE_WRITE);
    if (!file) {
      Serial.println("Failed to open file for writing");
      return;
    }
    file.close();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    String filename = "/" + upload.filename;
    File file = SPIFFS.open(filename, FILE_APPEND);
    if (file) {
      file.write(upload.buf, upload.currentSize);
      file.close();
      Serial.print("Uploading: ");
      Serial.println(filename);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    String filename = "/" + upload.filename;
    File file = SPIFFS.open(filename, FILE_APPEND);
    if (file) {
      file.close();
      Serial.print("Upload End: ");
      Serial.println(filename);
      Serial.println("File successfully uploaded");
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    Serial.println("Upload aborted");
  }
}

void handleConnectedDevices() {
  String html = "<html><body>";
  html += "<h1>Connected Devices</h1>";
  html += "<ul id='deviceList'>";
  
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
    // Imprime no monitor serial as informações dos dispositivos conectados
    Serial.print("Device connected - MAC: ");
    Serial.print(String(sta_info.mac[0], HEX) + ":" + String(sta_info.mac[1], HEX) + ":" + String(sta_info.mac[2], HEX) + ":" + String(sta_info.mac[3], HEX) + ":" + String(sta_info.mac[4], HEX) + ":" + String(sta_info.mac[5], HEX));
    Serial.print(" RSSI: ");
    Serial.println(String(sta_info.rssi));
  }

  html += "</ul>";
  html += "<a href='/'>Back to Home</a>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleGetConnectedDevices() {
  wifi_sta_list_t wifi_sta_list;
  esp_wifi_ap_get_sta_list(&wifi_sta_list);

  String json = "[";
  for (int i = 0; i < wifi_sta_list.num; i++) {
    wifi_sta_info_t sta_info = wifi_sta_list.sta[i];
    if (i > 0) json += ",";
    json += "{";
    json += "\"mac\":\"" + String(sta_info.mac[0], HEX) + ":" + String(sta_info.mac[1], HEX) + ":" + String(sta_info.mac[2], HEX) + ":" + String(sta_info.mac[3], HEX) + ":" + String(sta_info.mac[4], HEX) + ":" + String(sta_info.mac[5], HEX) + "\",";
    json += "\"rssi\":\"" + String(sta_info.rssi) + "\"";
    json += "}";
  }
  json += "]";

  server.send(200, "application/json", json);
}
