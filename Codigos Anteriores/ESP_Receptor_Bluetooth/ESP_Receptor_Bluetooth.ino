#include "BluetoothSerial.h"
#include <SPIFFS.h>

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP_BT_Rec");
  Serial.println("Bluetooth Started");
  SPIFFS.begin();
}

void loop() {
  if (SerialBT.available()) {
    File file = SPIFFS.open("/receivedfile.txt", FILE_WRITE);
    if (file) {
      unsigned long startTime = millis();
      while (SerialBT.available()) {
        file.write(SerialBT.read());
      }
      unsigned long endTime = millis();
      Serial.print("Received file in: ");
      Serial.print(endTime - startTime);
      Serial.println(" ms");
      file.close();
    }
  }
}
