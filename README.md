
# Projeto de Transferência de Arquivos com ESP32

Este projeto demonstra a transferência de arquivos entre dois microcontroladores ESP32 utilizando Wi-Fi e Bluetooth. Ele inclui dois pares de scripts: um para configuração de servidor e cliente Wi-Fi, e outro para configuração de receptor e transmissor Bluetooth. Os arquivos são armazenados no sistema de arquivos SPIFFS (SPI Flash File System) do ESP32.


## Componentes

- 2x ESP32
- Ambiente de desenvolvimento Arduino IDE


## Dependências

- Biblioteca WiFi para ESP32
- Biblioteca SPIFFS para ESP32
- Biblioteca BluetoothSerial para ESP32


## Instalação

Clone ou faça o download deste repositório.

```bash
git clone https://github.com/VCapis/esp32_wifi_bt.git
```

Abra o Arduino IDE e instale as bibliotecas necessárias:

+ WiFi.h
+ SPIFFS.h
+ BluetoothSerial.h


Configure a placa ESP32 no Arduino IDE:

- Vá para File > Preferences.
- Adicione https://dl.espressif.com/dl/package_esp32_index.json no campo Additional Boards Manager URLs.
- Vá para Tools > Board > Board Manager e instale esp32.


## Configuração

### Wi-Fi

#### ESP_Receptor_Wifi.ino
Este código configura o ESP32 como um servidor Wi-Fi que recebe arquivos enviados por um cliente Wi-Fi.


+ Abra ESP_Receptor_Wifi.ino no Arduino IDE.
+ Modifique as credenciais do Wi-Fi se necessário:

```bash
const char* ssid = "ESP_WIFI";
const char* password = "12345678";
```

+ Carregue o código no primeiro ESP32.

#### ESP_Remetente_Wifi.ino
Este código configura o ESP32 como um cliente Wi-Fi que envia um arquivo para o servidor Wi-Fi.

+ Abra ESP_Remetente_Wifi.ino no Arduino IDE.
+ Modifique as credenciais do Wi-Fi e o endereço IP do servidor:

```bash
const char* ssid = "ESP_WIFI";
const char* password = "12345678";
const char* serverIP = "192.168.1.100"; // Substitua pelo IP do servidor
```

+ Carregue o código no segundo ESP32.

### Bluetooth

#### ESP_Receptor_Bluetooth.ino
Este código configura o ESP32 como um receptor Bluetooth que recebe arquivos enviados por um dispositivo Bluetooth.

+ Abra ESP_Receptor_Bluetooth.ino no Arduino IDE.
+ Carregue o código no primeiro ESP32.

### ESP_Remetente_Bluetooth.ino
Este código configura o ESP32 como um transmissor Bluetooth que envia um arquivo para um dispositivo receptor Bluetooth.

+ Abra ESP_Remetente_Bluetooth.ino no Arduino IDE.
+ Carregue o código no segundo ESP32.
## Uso

### Transferência via Wi-Fi
+ Inicialize ambos os ESP32 com os códigos ESP_Receptor_Wifi.ino e ESP_Remetente_Wifi.ino carregados.
+ O ESP32 com o código ESP_Receptor_Wifi.ino funcionará como servidor e aguardará a conexão do cliente.
+ O ESP32 com o código ESP_Remetente_Wifi.ino conectará ao servidor e enviará o arquivo /testfile.txt.

### Transferência via Bluetooth
+ Inicialize ambos os ESP32 com os códigos ESP_Receptor_Bluetooth.ino e ESP_Remetente_Bluetooth.ino carregados.
+ O ESP32 com o código ESP_Receptor_Bluetooth.ino funcionará como receptor e aguardará a conexão do transmissor.
+ O ESP32 com o código ESP_Remetente_Bluetooth.ino conectará ao receptor e enviará o arquivo /testfile.txt.




## Notas
- As transferências de arquivos são realizadas utilizando o sistema de arquivos SPIFFS. Certifique-se de que o arquivo testfile.txt existe no SPIFFS antes de iniciar a transferência.
- A velocidade e a eficiência da transferência podem variar conforme o ambiente e a qualidade do sinal Wi-Fi ou Bluetooth.
- Modifique os tempos de delay conforme necessário para ajustar a frequência das transferências.