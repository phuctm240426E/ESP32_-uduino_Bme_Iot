#include "BmeLora.h"
#include "Debug.h"

extern Adafruit_SSD1306 display;

void initBmeLora() {
  DEBUG("LoRa Receiver");

#if defined(ESP32_S3) && (ESP32_S3 == 1)
  SPI.begin(L_SCK, L_MISO, L_MOSI, SS);
#endif

  LoRa.setPins(SS, RST, DIO0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //868E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(433E6)) {
    DEBUG(".");
    delay(500);
  }
  display.println("LoRa: Ready");
  display.display();
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  DEBUG("LoRa Initializing OK!");
}

void setLoraPacket(lora_data_packet_t* data) {
  data->deviceId = 1;
  data->spO2 = 2;
  data->heartRate = 3;
  data->bloodPressure.diastolic = 5;
  data->bloodPressure.systolic = 4;
  data->CRC = 6;
}

bool getLoraPacket(lora_data_packet_t* data) {
  uint8_t buffer[50];
  uint8_t cnt = 0;
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    DEBUG("Received packet '");

    while (LoRa.available()) {
      buffer[cnt] = LoRa.read();
      cnt++;
    }
    memcpy(data, buffer, LORA_DATA_PACKET_SIZE);
    return 1;
  }
  
  return 0;
}

void setAndSendLoraPacket() {
  lora_data_packet_t data;
  char dataSend[LORA_DATA_PACKET_SIZE];
  
  setLoraPacket(&data);
  memcpy(dataSend, &data, LORA_DATA_PACKET_SIZE);
  
  LoRa.beginPacket();
  LoRa.print(dataSend);
  LoRa.endPacket();
}