#include "BmeLora.h"
#include "Debug.h"

void initBmeLora() {
  DEBUG("Init Lora Module");

#if defined(ESP32) && (ESP32 == 0)
  SPI.begin(L_SCK, L_MISO, L_MOSI, SS);
#endif        // ESP32 == 0

  LoRa.setPins(SS, RST, DIO0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //868E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(433E6)) {
    DEBUG(".");
    delay(500);
  }
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
#if defined(LORA_NODE) && (LORA_NODE == 1)
  LoRa.setTxPower(20);   // SX1278: max ~17 dBm
#endif        // LORA_NODE == 1
  LoRa.setSpreadingFactor(9);
  LoRa.setSignalBandwidth(250E3);
  LoRa.setCodingRate4(6);
  LoRa.enableCrc();
  //LoRa.explicitHeaderMode();

  LoRa.setSyncWord(0x2A);
  LoRa.setGain(0); 
  DEBUG("LoRa Initializing OK!");
}

#if defined(LORA_NODE) && (LORA_NODE == 1)
bool setLoraPacket(lora_data_packet_t* data) {
  data->deviceId = DEVICE_ID;
  bool hasData = getSpO2AndHeartRate(&(data->spO2), &(data->heartRate));
  return hasData;
}
#endif        // LORA_NODE == 1

bool getLoraPacket(lora_data_packet_t* data) {
#if defined(TEST_MODE) && (TEST_MODE == 1)
  data->deviceId = 1;
  data->spO2 = random(70, 90);
  data->heartRate = random(81, 90);
  return true;
#endif        // TEST_MODE == 1

#if defined(TEST_MODE) && (TEST_MODE == 0)
  uint8_t buffer[10];
  uint8_t cnt = 0;
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    DEBUG("Received packet");

    while (LoRa.available()) {
      buffer[cnt] = LoRa.read();
      cnt++;
    }
    memcpy(data, buffer, LORA_DATA_PACKET_SIZE);
    return 1;
  }

  return 0;
#endif        // TEST_MODE == 0
}

#if defined(LORA_NODE) && (LORA_NODE == 1)
void setAndSendLoraPacket() {
  lora_data_packet_t data;
  char dataSend[LORA_DATA_PACKET_SIZE];
  
  if(setLoraPacket(&data)) {
    memcpy(dataSend, &data, LORA_DATA_PACKET_SIZE);
    
    LoRa.beginPacket();
    LoRa.write(dataSend, LORA_DATA_PACKET_SIZE);
    LoRa.endPacket();
  }
}
#endif        // LORA_NODE == 1