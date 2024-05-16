#include <WiFi.h>
#include <SPIFFS.h>

// Define as credenciais da rede Wi-Fi
const char* ssid = "ESP_WIFI";
const char* password = "12345678";

// Cria um servidor Wi-Fi na porta 80
WiFiServer server(80);

void setup() {
  Serial.begin(115200); // Inicializa a comunicação serial
  WiFi.begin(ssid, password); // Conecta-se à rede Wi-Fi
  
  // Aguarda a conexão com o Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  server.begin(); // Inicia o servidor
  SPIFFS.begin(); // Inicializa o sistema de arquivos SPIFFS
}

void loop() {
  WiFiClient client = server.available(); // Verifica se há um cliente conectado
  if (client) {
    File file = SPIFFS.open("/receivedfile.txt", FILE_WRITE); // Abre um arquivo para escrita no SPIFFS
    if (file) {
      unsigned long startTime = millis(); // Marca o tempo de início
      while (client.connected()) { // Enquanto o cliente estiver conectado
        if (client.available()) { // Se houver dados disponíveis do cliente
          file.write(client.read()); // Escreve os dados no arquivo
        }
      }
      unsigned long endTime = millis(); // Marca o tempo de término
      Serial.print("Received file in: ");
      Serial.print(endTime - startTime);
      Serial.println(" ms"); // Imprime o tempo total de recebimento do arquivo
      file.close(); // Fecha o arquivo
    }
    client.stop(); // Fecha a conexão com o cliente
  }
}
