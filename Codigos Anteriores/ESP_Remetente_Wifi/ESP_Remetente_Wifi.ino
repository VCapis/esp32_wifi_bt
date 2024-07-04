#include <WiFi.h>
#include <SPIFFS.h>

// Define as credenciais da rede Wi-Fi
const char* ssid = "ESP_WIFI";
const char* password = "12345678";
const char* serverIP = "192.168.1.100"; // IP do servidor
const int serverPort = 80; // Porta do servidor

void setup() {
  Serial.begin(115200); // Inicializa a comunicação serial
  WiFi.begin(ssid, password); // Conecta-se à rede Wi-Fi

  // Aguarda a conexão com o Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  SPIFFS.begin(); // Inicializa o sistema de arquivos SPIFFS
}

void loop() {
  WiFiClient client;
  if (client.connect(serverIP, serverPort)) { // Conecta-se ao servidor
    File file = SPIFFS.open("/testfile.txt", FILE_READ); // Abre um arquivo para leitura no SPIFFS
    if (file) {
      unsigned long startTime = millis(); // Marca o tempo de início
      while (file.available()) { // Enquanto houver dados disponíveis no arquivo
        client.write(file.read()); // Envia os dados ao servidor
      }
      unsigned long endTime = millis(); // Marca o tempo de término
      Serial.print("Transfer time: ");
      Serial.print(endTime - startTime);
      Serial.println(" ms"); // Imprime o tempo total de transferência do arquivo
      file.close(); // Fecha o arquivo
    }
    client.stop(); // Fecha a conexão com o servidor
  }
  delay(60000); // Aguarda 60 segundos antes de realizar outra transferência
}
