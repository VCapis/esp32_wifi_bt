#include <BluetoothSerial.h>
#include <WebServer.h>
#include <SPIFFS.h>

// Replace with your Bluetooth credentials
BluetoothSerial SerialBT;
const char* bt_name = "ESP32_BT";

// Set web server port number to 80
WebServer server(80);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);

  // Initialize the SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("SPIFFS mounted successfully");

  // Initialize Bluetooth
  SerialBT.begin(bt_name);
  Serial.println("Bluetooth started");

  // Initialize web server
  server.begin();

  // Define the web server routes
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
}

void loop(){
  server.handleClient();

  if (SerialBT.available()) {
    String incoming = SerialBT.readString();
    Serial.println("Received: " + incoming);
    // Handle the incoming data (save to SPIFFS)
  }
}

void handleRoot() {
  header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  header += "<!DOCTYPE html><html><body><h1>ESP32 Bluetooth File Transfer</h1>";
  header += "<p>IP: ";
  header += WiFi.localIP().toString(); // Note: Replace with Bluetooth info if needed
  header += "</p><p>Status: ";
  // Add status here
  header += "</p></body></html>";
  server.send(200, "text/html", header);
}

void handleNotFound() {
  server.send(404, "text/plain", "Not Found");
}
