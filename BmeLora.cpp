#include "BmeLora.h"
#include "Debug.h"

void initBmeLora() {
  DEBUG("LoRa Receiver");

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
  LoRa.setSyncWord(0xF3);
  DEBUG("LoRa Initializing OK!");
}

uint8_t* getLoraPacket() {
  // try to parse packet
  uint8_t data[LORA_DATA_PACKET_SIZE] = {0};
  String LoRaData;
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    DEBUG("Received packet '");

    // read packet
    while (LoRa.available()) {
      LoRaData = LoRa.readString();
      DEBUG(LoRaData); 
    }

    for(int i = 0; i < LORA_DATA_PACKET_SIZE; i++) {
      data[i] = LoRaData[i];
    }
    data[LORA_DATA_PACKET_SIZE] = 0;
  }
  return data;
}