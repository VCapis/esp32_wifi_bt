# Configuração do Ambiente do Arduino 


## Instalação 

Clone ou faça o download deste repositório.

```bash
git clone https://github.com/VCapis/esp32_wifi_bt.git
```
## Resetar o EPS32 do Zero

- [Acessar](https://espressif.github.io/esptool-js/)
- Set "Baudrate: 115200"
- "Connect" e selecionar a porta
- "Erase Flash"
- Set "Flash Address: 0x0000"
- Carregar o Arquivo "File" [Download do Arquivo](https://github.com/VCapis/esp32_wifi_bt/blob/main/assets/Factory_Reset_And_Bootloader_Repair.bin)  
- "Program" e aguarde "Leaving" no CMD
- Disconnect

# Faça a instalação do Arduino IDE:
[Download] (https://www.arduino.cc/en/software)

# Instalar a Placa ESP32 no Arduino IDE:

- Abrir o Arduino IDE
- Ir em _"Arquivo">"Preferências">"URLs do Gerenciador de Placas Adicionais"_
- Colar (" https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json ") e OK
- Ir em _"Ferramentas">"Placa:">"Gerenciador de Placas"_
- Pesquisar por _"ESP32"_
- Instalar o "*esp32* _por Espressif Systems_"
- Ir em _"Ferramentas">"Placa:">"esp31">"ESP32 Dev Module"_

# Instalar as Blibiotecas 

- Abrir o Arduino IDE
- Ir em _"Rascunho">"Incluir Bibliotecas">"Gerenciar Bibliotecas"_
- Pesquisar por _"ESPAsyncWebServer"_
- Instalar o "*ESPAsyncWebServer* _by lacamera_" juntamente com as dependências solicitadas ( "AsyncTCP _by dvarrel_" e "*ESPAsyncTCP* _by dvarrel_")


Faça a instalação do Python :
https://www.python.org/downloads/








