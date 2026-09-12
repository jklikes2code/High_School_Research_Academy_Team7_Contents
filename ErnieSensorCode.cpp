#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Wire.h>
#include "Adafruit_VL6180X.h" // Updated library for VL6180


Adafruit_VL6180X vl = Adafruit_VL6180X();


// Target Broadcast Address to avoid missing the DOIT board
uint8_t broadcastMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


// Struct matching exactly what we will send
struct DataPacket {
 bool objectDetected;
};


void setup() {
 Serial.begin(115200);
 delay(1000);
 Serial.println("\n=== VL6180 TOF SENDER STARTING ===");


 // Initialize I2C and VL6180 Sensor
 Wire.begin();
 if (!vl.begin()) {
   Serial.println("Failed to find VL6180X sensor! Check wiring.");
   while (1);
 }
 Serial.println("VL6180X Sensor online.");


 // Initialize Radio Channel 1
 WiFi.mode(WIFI_STA);
 esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);


 if (esp_now_init() != ESP_OK) {
   Serial.println("ESP-NOW Init Failed");
   return;
 }


 // Register the Broadcast peer
 esp_now_peer_info_t peerInfo = {};
 memcpy(peerInfo.peer_addr, broadcastMAC, 6);
 peerInfo.channel = 1; 
 peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
   Serial.println("Failed to add broadcast peer");
   return;
 }
 Serial.println("Setup done. Scanning...");
}


void loop() {
 // Read distance in millimeters
 uint8_t range = vl.readRange();
 uint8_t status = vl.readRangeStatus();


 DataPacket packet;
 packet.objectDetected = false;


 // Status 0 means a valid, error-free reading was captured
 if (status == VL6180X_ERROR_NONE) {
   Serial.printf("Distance: %d mm\n", range);


   // TRIGGER THRESHOLD: The VL6180 optimized range is 0 to 100mm (about 4 inches).
   // This flags an alert if an object gets closer than 70mm.
   if (range > 5 && range < 70) {
     packet.objectDetected = true;
     Serial.println("Proximity Alert Triggered!");
   }
 }


 // Blast the updated flag package over the air
 esp_now_send(broadcastMAC, (uint8_t *) &packet, sizeof(packet));


 delay(150); // Scan roughly 6 times a second
}
