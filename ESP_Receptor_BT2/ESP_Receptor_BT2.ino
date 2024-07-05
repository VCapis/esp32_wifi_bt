#include <BluetoothSerial.h>
#include <FS.h>
#include <SPIFFS.h>

// Definição do objeto Bluetooth
BluetoothSerial SerialBT;

unsigned long startTime, endTime;
int packetCountSuccess = 0;
int packetCountFail = 0;
String macAddress;

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

  // Captura o endereço MAC
  macAddress = getMACAddress();

  // Cria um arquivo de exemplo no SPIFFS
  createExampleFile("/example.txt", "Este é um exemplo de dados do arquivo");

  // Inicia o tempo de referência
  startTime = millis();
}

void loop() {
  // Verifica se há dados recebidos
  if (SerialBT.available()) {
    File file = SPIFFS.open("/received_data.txt", FILE_WRITE);
    if (!file) {
      Serial.println("Falha ao abrir o arquivo para escrita");
      return;
    }

    while (SerialBT.available()) {
      char receivedChar = SerialBT.read();
      file.write(receivedChar);
    }
    file.close();
    Serial.println("Dados recebidos e salvos no arquivo");

    // Atualiza contadores de pacotes recebidos
    packetCountSuccess++;
    
    // Imprime informações no monitor serial
    printStats();
    
    // Reinicia o tempo de referência
    startTime = millis();
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

void printStats() {
  Serial.println("========== Estatísticas ==========");
  Serial.println("Endereço MAC: " + macAddress);
  Serial.println("Pacotes enviados com sucesso: " + String(packetCountSuccess));
  Serial.println("Pacotes falhados: " + String(packetCountFail));
  Serial.println("==================================");
}

String getMACAddress() {
  uint8_t baseMac[6];
  esp_read_mac(baseMac, ESP_MAC_BT);
  char baseMacChr[18] = {0};
  snprintf(baseMacChr, 18, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  return String(baseMacChr);
}
