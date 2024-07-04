#include <WiFi.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include "esp_wifi.h"
#include <vector>

// Defina o nome e a senha da rede Wi-Fi criada pelo ESP32
const char* ssid = "ESP_AP";
const char* password = "12345678";

// Crie um servidor web na porta 80
WebServer server(80);

struct FileInfo {
  String filename;
  size_t size;
  String type;
  unsigned long startTime;
  unsigned long endTime;
  unsigned long jitter;
  int packets;
  std::vector<unsigned long> packetTimes;
};

FileInfo currentFileInfo;
std::vector<FileInfo> fileLogs;

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
  // Rota para excluir um arquivo
  server.on("/delete", HTTP_GET, handleDelete);
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

  // Informação da memória SPIFFS
  html += "<h2>SPIFFS Info:</h2>";
  html += "<ul>";
  html += "<li>Total space: " + formatBytes(SPIFFS.totalBytes()) + "</li>";
  html += "<li>Used space: " + formatBytes(SPIFFS.usedBytes()) + "</li>";
  html += "<li>Free space: " + formatBytes(SPIFFS.totalBytes() - SPIFFS.usedBytes()) + "</li>";
  html += "</ul>";

  html += "<h2>Files:</h2><ul>";

  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    String filename = file.name();
    html += "<li>";
    html += "<a href='/download?file=" + filename + "'>";
    html += filename;
    html += "</a> ";
    html += "<a href='/delete?file=" + filename + "'>";
    html += "<button>Delete</button>";
    html += "</a>";
    html += "</li>";
    file = root.openNextFile();
  }
  html += "</ul>";

  html += "<h2>Connected Devices:</h2>";
  html += "<ul id='deviceList'></ul>";

  html += "<h2>File Logs:</h2>";
  html += "<table border='1'>";
  html += "<tr><th>Filename</th><th>Size (bytes)</th><th>Type</th><th>Start Time (ms)</th><th>End Time (ms)</th><th>Jitter (ms)</th><th>Packets</th></tr>";

  for (auto& log : fileLogs) {
    html += "<tr>";
    html += "<td>" + log.filename + "</td>";
    html += "<td>" + String(log.size) + "</td>";
    html += "<td>" + log.type + "</td>";
    html += "<td>" + String(log.startTime) + "</td>";
    html += "<td>" + String(log.endTime) + "</td>";
    html += "<td>" + String(log.jitter) + "</td>";
    html += "<td>" + String(log.packets) + "</td>";
    html += "</tr>";
  }

  html += "</table>";

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

void handleDelete() {
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
  SPIFFS.remove(path);
  server.send(200, "text/plain", "File Deleted");
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
    currentFileInfo = {filename, 0, getContentType(filename), millis(), 0, 0, 0, {}};
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
      currentFileInfo.size += upload.currentSize;
      currentFileInfo.packets++;
      currentFileInfo.packetTimes.push_back(millis());
      file.close();
      Serial.print("Uploading: ");
      Serial.println(filename);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    String filename = "/" + upload.filename;
    File file = SPIFFS.open(filename, FILE_APPEND);
    if (file) {
      file.close();
      currentFileInfo.endTime = millis();
      currentFileInfo.jitter = calculateJitter(currentFileInfo.packetTimes);
      fileLogs.push_back(currentFileInfo);
      Serial.print("Upload End: ");
      Serial.println(filename);
      Serial.println("File successfully uploaded");
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    Serial.println("Upload aborted");
  }
}

unsigned long calculateJitter(const std::vector<unsigned long>& packetTimes) {
  if (packetTimes.size() < 2) return 0;

  unsigned long jitterSum = 0;
  for (size_t i = 1; i < packetTimes.size(); ++i) {
    unsigned long diff = labs(packetTimes[i] - packetTimes[i - 1]);
    if (i > 1) {
      unsigned long prevDiff = labs(packetTimes[i - 1] - packetTimes[i - 2]);
      jitterSum += labs(diff - prevDiff);
    }
  }
  return jitterSum / (packetTimes.size() - 1);
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

String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + " B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + " KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + " MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
  }
}
