#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#define SPEAKER_PIN 5  // Set to GPIO 5
#define BUZZ_FREQ   1200 // Tone frequency in Hz

struct DataPacket {
  bool objectDetected;
};

// Modern callback signature required by ESP32 Core compiler
void receiveData(const esp_now_recv_info_t *recvInfo, const uint8_t *incomingData, int len) {
  DataPacket packet;
  
  // Protection against potential array mismatch sizes
  if (len == sizeof(DataPacket)) {
    memcpy(&packet, incomingData, sizeof(packet));

    if (packet.objectDetected) {
      Serial.println("Alarm received! Playing audio alert...");
      
      // Plays dual-beep alert pattern
      tone(SPEAKER_PIN, BUZZ_FREQ, 150);
      delay(200);
      tone(SPEAKER_PIN, BUZZ_FREQ, 150);
    } else {
      // Keep speaker quiet if nothing is near the sensor
      noTone(SPEAKER_PIN); 
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(4000); // Power stability buffer for DOIT board
  Serial.println("\n=== ALARM AUDIO RECEIVER STARTING ===");

  pinMode(SPEAKER_PIN, OUTPUT);
  noTone(SPEAKER_PIN); // Ensure speaker starts dead silent

  // Force Channel 1
  WiFi.mode(WIFI_STA);
  esp_wifi_set_max_tx_power(WIFI_POWER_8_5dBm); // Suppress brownout loops
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }

  // CRITICAL FIX: Cast the function callback to meet the strict compiler rules
  esp_now_register_recv_cb(esp_now_recv_cb_t(receiveData));
  Serial.println("Audio base unit online. Ready for sensor inputs...");
}

void loop() {
  // Free up processor time for backend radio processing
  delay(100);
}
