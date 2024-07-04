#include <BluetoothSerial.h>
#include <SPIFFS.h>
#include <vector>

// Crie uma instância do BluetoothSerial
BluetoothSerial SerialBT;

struct FileInfo {
  String filename;
  size_t size;
  String type;
  unsigned long startTime;
  unsigned long endTime;
  unsigned long jitter;
  int packets;
  std::vector<unsigned long> packetTimes;
  unsigned long totalLatency;
  unsigned long maxLatency;
  unsigned long minLatency;
  int successPackets;
  int errorPackets;
};

FileInfo currentFileInfo;
std::vector<FileInfo> fileLogs;

void setup() {
  Serial.begin(115200); // Inicializa a comunicação serial
  SerialBT.begin("ESP_BT"); // Inicializa o Bluetooth com o nome "ESP_BT"
  
  Serial.println("Bluetooth Started");

  // Inicializa o sistema de arquivos SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Informa ao usuário que o sistema está pronto
  Serial.println("ESP32 File Receiver via Bluetooth");
}

void loop() {
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n');
    command.trim();

    if (command.equals("UPLOAD")) {
      handleFileUpload();
    } else if (command.equals("DOWNLOAD")) {
      handleDownload();
    } else if (command.equals("DELETE")) {
      handleDelete();
    } else if (command.equals("LIST")) {
      listFiles();
    } else {
      SerialBT.println("Unknown command");
    }
  }
}

void handleFileUpload() {
  if (SerialBT.available()) {
    String filename = SerialBT.readStringUntil('\n');
    filename.trim();
    if (!filename.startsWith("/")) filename = "/" + filename;

    Serial.print("Upload Start: ");
    Serial.println(filename);
    currentFileInfo = {filename, 0, getContentType(filename), millis(), 0, 0, 0, {}, 0, 0, 0xFFFFFFFF, 0, 0};
    File file = SPIFFS.open(filename, FILE_WRITE);
    if (!file) {
      Serial.println("Failed to open file for writing");
      SerialBT.println("Failed to open file for writing");
      return;
    }
    file.close();

    while (SerialBT.available()) {
      file = SPIFFS.open(filename, FILE_APPEND);
      if (file) {
        String fileContent = SerialBT.readStringUntil('\n');
        file.write((uint8_t*)fileContent.c_str(), fileContent.length());
        currentFileInfo.size += fileContent.length();
        file.close();
      }
    }

    currentFileInfo.endTime = millis();
    currentFileInfo.jitter = calculateJitter(currentFileInfo.packetTimes);
    fileLogs.push_back(currentFileInfo);
    Serial.println("File successfully uploaded");
    SerialBT.println("File successfully uploaded");
  }
}

void handleDownload() {
  if (SerialBT.available()) {
    String path = SerialBT.readStringUntil('\n');
    path.trim();
    if (!path.startsWith("/")) {
      path = "/" + path;
    }
    if (!SPIFFS.exists(path)) {
      SerialBT.println("File Not Found");
      return;
    }
    File file = SPIFFS.open(path, "r");
    if (file) {
      SerialBT.println("DOWNLOAD_START");
      while (file.available()) {
        String line = file.readStringUntil('\n');
        SerialBT.println(line);
      }
      file.close();
      SerialBT.println("DOWNLOAD_END");
    } else {
      SerialBT.println("Internal Server Error");
    }
  }
}

void handleDelete() {
  if (SerialBT.available()) {
    String path = SerialBT.readStringUntil('\n');
    path.trim();
    if (!path.startsWith("/")) {
      path = "/" + path;
    }
    if (!SPIFFS.exists(path)) {
      SerialBT.println("File Not Found");
      return;
    }
    SPIFFS.remove(path);
    SerialBT.println("File Deleted");
  }
}

void listFiles() {
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    String filename = file.name();
    SerialBT.println(filename);
    file = root.openNextFile();
  }
}

String getContentType(String filename) {
  if (filename.endsWith(".htm")) return "text/html";
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
