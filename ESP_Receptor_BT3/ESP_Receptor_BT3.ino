#include <BluetoothSerial.h>
#include <FS.h>
#include <SPIFFS.h>
#include <vector>

// Definição do objeto Bluetooth
BluetoothSerial SerialBT;
unsigned long startTime, endTime;
std::vector<unsigned long> packetTimes;
unsigned long totalLatency = 0;
unsigned long maxLatency = 0;
unsigned long minLatency = 0xFFFFFFFF;
int packetCountSuccess = 0;
int packetCountFail = 0;

void setup() {
  // Inicialização da Serial para monitoramento
  Serial.begin(115200);
  
  // Inicialização do Bluetooth
  if (!SerialBT.begin("ESP32_BT")) {
    Serial.println("Falha ao iniciar o Bluetooth");
    while (1);
  }
  Serial.println("Bluetooth iniciado com sucesso!");

  // Inicialização do SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Falha ao montar o sistema de arquivos");
    while (1);
  }
  Serial.println("SPIFFS montado com sucesso!");

  // Cria um arquivo de exemplo no SPIFFS
  createExampleFile("/example.txt", "Este é um exemplo de dados do arquivo");

  // Inicia o tempo de referência
  startTime = millis();
}

void loop() {
  // Verifica se há dados recebidos
  if (SerialBT.available()) {
    String filePath = "/received_data.txt";
    File file = SPIFFS.open(filePath, FILE_WRITE);
    if (!file) {
      Serial.println("Falha ao abrir o arquivo para escrita");
      return;
    }

    // Inicia o tempo de transmissão
    startTime = millis();
    packetTimes.clear();
    totalLatency = 0;
    maxLatency = 0;
    minLatency = 0xFFFFFFFF;
    packetCountSuccess = 0;
    packetCountFail = 0;

    while (SerialBT.available()) {
      char receivedChar = SerialBT.read();
      file.write(receivedChar);
      
      unsigned long currentTime = millis();
      unsigned long latency = currentTime - (packetTimes.empty() ? startTime : packetTimes.back());
      packetTimes.push_back(currentTime);
      totalLatency += latency;
      if (latency > maxLatency) maxLatency = latency;
      if (latency < minLatency) minLatency = latency;

      // Simula pacotes de sucesso e falha
      bool success = random(0, 2); // Simula sucesso (0) ou falha (1)
      if (success) {
        packetCountSuccess++;
      } else {
        packetCountFail++;
      }
    }
    file.close();

    // Finaliza o tempo de transmissão
    endTime = millis();

    Serial.println("Dados recebidos e salvos no arquivo");

    // Imprime informações no monitor serial e envia para o celular
    printStats(filePath);
  }

  delay(1000);
}

void createExampleFile(const char* path, const char* message) {
  File file = SPIFFS.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Falha ao abrir o arquivo para escrita");
    return;
  }
  file.print(message);
  file.close();
}

void printStats(const String& filePath) {
  // Obtém o tamanho do arquivo
  File file = SPIFFS.open(filePath, FILE_READ);
  if (!file) {
    Serial.println("Falha ao abrir o arquivo para leitura");
    return;
  }
  size_t fileSize = file.size();
  file.close();

  // Calcula o jitter
  unsigned long jitter = calculateJitter(packetTimes);

  // Formata as estatísticas
  String stats = "========== Estatísticas ==========\n";
  stats += "Filename:" + filePath + "\n";
  stats += "Size (bytes):" + String(fileSize) + "\n";
  stats += "Start Time(ms):" + String(startTime) + "\n";
  stats += "End Time (ms): " + String(endTime) + "\n";
  stats += "Duration  (ms): " + String(endTime - startTime) + "\n";
  stats += "Jitter (ms): " + String(jitter) + " \n";
  stats += "Avg Latency (ms): " + String(packetTimes.size() ? totalLatency / packetTimes.size() : 0) + " \n";
  stats += "Max Latency (ms): " + String(maxLatency) + " \n";
  stats += "Min Latency (ms): " + String(minLatency) + " \n";
  stats += "Success Packets: " + String(packetCountSuccess) + "\n";
  stats += "Error Packets: " + String(packetCountFail) + "\n";
  stats += "==================================\n";

  // Imprime as estatísticas no monitor serial
  Serial.print(stats);

  // Envia as estatísticas para o celular via Bluetooth
  SerialBT.print(stats);
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