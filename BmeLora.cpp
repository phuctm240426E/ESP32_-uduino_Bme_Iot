#include "BmeLora.h"
#include "Debug.h"

void initBmeLora() {
  DEBUG("LoRa Receiver");

#if defined(ESP32) && (ESP32 == 0)
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
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
#if defined(ESP32_S3) && (ESP32_S3 == 0)
  LoRa.setTxPower(20);   // SX1278: max ~17 dBm
#endif
  LoRa.setSpreadingFactor(9);
  LoRa.setSignalBandwidth(250E3);
  LoRa.setCodingRate4(6);
  LoRa.enableCrc();
  //LoRa.explicitHeaderMode();

  LoRa.setSyncWord(0x2A);
  LoRa.setGain(0); 
  DEBUG("LoRa Initializing OK!");
}

#if defined(ESP32_S3) && (ESP32_S3 == 0)
bool setLoraPacket(lora_data_packet_t* data) {
  data->deviceId = DEVICE_ID;
  bool hasData = getSpO2AndHeartRate(&(data->spO2), &(data->heartRate));
  //data->spO2 = getSpO2();
  //data->heartRate = getHeartRate();
  data->bloodPressure = getBloodPressure();
  data->CRC = 6;
  return hasData;
}
#endif

bool getLoraPacket(lora_data_packet_t* data) {
  uint8_t buffer[50];
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
}

#if defined(ESP32_S3) && (ESP32_S3 == 0)
void setAndSendLoraPacket() {
  lora_data_packet_t data;
  char dataSend[LORA_DATA_PACKET_SIZE];
  
  if(setLoraPacket(&data)) {
    memcpy(dataSend, &data, LORA_DATA_PACKET_SIZE);
    
    LoRa.beginPacket();
    LoRa.print(dataSend);
    LoRa.endPacket();
  }
}
#endif