#include "BluetoothSerial.h"
#include <SPIFFS.h>

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP_BT_Rem");
  Serial.println("Bluetooth Started");
  SPIFFS.begin();
}

void loop() {
  if (SerialBT.connected()) {
    File file = SPIFFS.open("/testfile.txt", FILE_READ);
    if (file) {
      unsigned long startTime = millis();
      while (file.available()) {
        SerialBT.write(file.read());
      }
      unsigned long endTime = millis();
      Serial.print("Transfer time: ");
      Serial.print(endTime - startTime);
      Serial.println(" ms");
      file.close();
    }
    delay(60000); // Aguarde 1 minuto antes de iniciar outro teste
  }
}
