#include "BmeLora.h"
#include "Debug.h"

int cnt = 0;
uint8_t id = 1;

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
//#if defined(LORA_NODE) && (LORA_NODE == 1)
  LoRa.setTxPower(20);   // SX1278: max ~17 dBm
//#endif        // LORA_NODE == 1
  LoRa.setSpreadingFactor(9);
  LoRa.setSignalBandwidth(250E3);
  LoRa.setCodingRate4(6);
  LoRa.enableCrc();
  //LoRa.explicitHeaderMode();

  LoRa.setSyncWord(0x2A);
  LoRa.setGain(0); 
  DEBUG("LoRa Initializing OK!");
}

uint16_t calculateCRC(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}

#if defined(LORA_NODE) && (LORA_NODE == 1)
bool setLoraPacket(lora_data_packet_t* data) {
  data->deviceId = DEVICE_ID;
  bool hasData = getSpO2AndHeartRate(&(data->spO2), &(data->heartRate));
  data->crc = calculateCRC((uint8_t*)data, 4);
  return hasData;
}
#endif        // LORA_NODE == 1

bool getLoraPacket(lora_data_packet_t* data) {
#if defined(TEST_MODE) && (TEST_MODE == 1)
  if(id == 1) {
    id = 2;
  }
  else{
   id = 1; 
  }
  data->deviceId = id;
  data->spO2 = cnt;
  cnt++;
  //data->spO2 = random(85, 100);
  data->heartRate = random(81, 90);
  data->crc = data->crc = calculateCRC((uint8_t*)data, 4);
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
  uint8_t dataSend[LORA_DATA_PACKET_SIZE];
  
  if(setLoraPacket(&data)) {
    memcpy(dataSend, &data, LORA_DATA_PACKET_SIZE);
    
    LoRa.beginPacket();
    LoRa.write(dataSend, LORA_DATA_PACKET_SIZE);
    LoRa.endPacket();
  }
}
#endif        // LORA_NODE == 1