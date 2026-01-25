#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "BmeLora.h"
#include "BmeLcd.h"


// WiFi
const char* WIFI_SSID     = "DoanDong";
const char* WIFI_PASSWORD = "dong1958";

// ThingsBoard
const char* THINGSBOARD_HOST = "thingsboard.cloud";
const char* ACCESS_TOKEN    = "ef2ZhoEBy8MISx0OqMWJ";

// Topics
const char* TELEMETRY_TOPIC = "v1/devices/me/telemetry";
const char* RPC_SUB_TOPIC   = "v1/devices/me/rpc/request/+";

QueueHandle_t loraQueue;
QueueHandle_t alarmQueue;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastAlarm = 0;

unsigned long lastBlink = 0;

bool blinkState = false;

uint16_t alarmMask = 0;

uint8_t getAlarmState(uint8_t deviceId) {
  uint16_t mask = 0 | (1 << deviceId);
  if((alarmMask & mask) > 0) return 1;
  else return 0;
}

void setAlarmState(uint8_t deviceId, uint16_t state)
{
  if (state == 1) {
    // Set deviceId-th bit
    alarmMask |= (1UL << deviceId);
    } else {
    // Clear deviceId-th bit
    alarmMask &= ~(1UL << deviceId);
  }
}

void setupWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void rpcCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    //Serial.println("JSON parse failed");
    return;
  }

  const char* method = doc["method"];
  JsonObject params = doc["params"];

  if (strcmp(method, "triggerAlarm") == 0) {
    alarm_t alarm;
    alarm.deviceId = (uint8_t)doc["params"]["id"];
    alarm.isActive = 1;
    
    xQueueSend(alarmQueue, &alarm, 0);
  }
  else if (strcmp(method, "clearAlarm") == 0) {
    alarm_t alarm;
    alarm.deviceId = (uint8_t)doc["params"]["id"];
    alarm.isActive = 0;
    
    xQueueSend(alarmQueue, &alarm, 0);
  }

  // ---------- RPC Response ----------
  String topicStr = String(topic);
  String requestId = topicStr.substring(
    String("v1/devices/me/rpc/request/").length()
  );

  String responseTopic = "v1/devices/me/rpc/response/" + requestId;
  client.publish(responseTopic.c_str(), "{\"result\":\"OK\"}");
}

void mqttAndSendTelemetryTask(void* pvParameters) {
  while(true) {
    if (!client.connected()) {
      while (!client.connected()) {
      Serial.print("Connecting to ThingsBoard...");
        if (client.connect("ESP32_HEALTH_DEVICE", ACCESS_TOKEN, NULL)) {
          Serial.println("connected");
          client.subscribe(RPC_SUB_TOPIC);
        } else {
          Serial.print("failed, rc=");
          Serial.println(client.state());
          vTaskDelay(pdMS_TO_TICKS(2000));
        }
      }
    }
    client.loop();

    lora_data_packet_t loraData;
    if(xQueueReceive(loraQueue, &loraData, 0)) {
      StaticJsonDocument<128> doc;

      char key[24];

      snprintf(key, sizeof(key), "Alarm_%u", loraData.deviceId);
      doc[key] = getAlarmState(loraData.deviceId);

      snprintf(key, sizeof(key), "HeartRate_%u", loraData.deviceId);
      doc[key] = loraData.heartRate;

      snprintf(key, sizeof(key), "SpO2_%u", loraData.deviceId);
      doc[key] = loraData.spO2;

      char payload[128];
      serializeJson(doc, payload);

      Serial.print("Sending telemetry: ");
      Serial.println(payload);

      client.publish(TELEMETRY_TOPIC, payload);
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void loraAndAlarmTask(void* pvParameters) {
  lora_data_packet_t loraPkt;
  alarm_t alarm;

  while (true) {
    if (getLoraPacket(&loraPkt)) {
      if(loraPkt.crc == calculateCRC((uint8_t*)&loraPkt, 4)) {
        Serial.println("RECEIVE SUCCESS");
        uint8_t ackPacket[1];
        ackPacket[0] = loraPkt.deviceId;
        LoRa.beginPacket();
        LoRa.write(ackPacket, 1);
        LoRa.endPacket();
        xQueueSend(loraQueue, &loraPkt, 0);
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0,0);
        display.print("RSSI: "); display.print(LoRa.packetRssi()); display.println(" dBm");
        display.drawFastHLine(0, 12, 128, WHITE);
        display.setCursor(0,16);
        display.print("Device: "); display.print(loraPkt.deviceId); display.println("");
        display.print("HR:     "); display.print(loraPkt.heartRate); display.println("");
        display.print("SpO2:   "); display.print(loraPkt.spO2); display.println("");
      }
      else {
        Serial.println("RECEIVE FAIL");
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0,0);
        display.print("RSSI: "); display.print(LoRa.packetRssi()); display.println(" dBm");
        display.drawFastHLine(0, 12, 128, WHITE);
        display.setCursor(0,16);
        display.println("FAIL RECEIVE");
      }
    }

    if (xQueueReceive(alarmQueue, &alarm, 0)) {
      Serial.printf("ALARM node %d Active=%d\n",
      alarm.deviceId,
      alarm.isActive);
      setAlarmState(alarm.deviceId, alarm.isActive);
      if(alarm.isActive == 1) lastAlarm = millis();
    }
    if(alarmMask > 0) {
      display.fillRect(20, 48, 50, 16, BLACK);
      display.setCursor(28, 48);
      display.setTextSize(2);
      display.println("ALARM");
    }
    else {
      display.fillRect(20, 48, 50, 16, BLACK);
    }
    display.display();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));

  initOLED();
  setupWiFi();
  client.setServer(THINGSBOARD_HOST, 1883);
  client.setCallback(rpcCallback);
  initBmeLora();

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("DEVICE READY");
  display.display();

  loraQueue = xQueueCreate(50, sizeof(lora_data_packet_t));
  alarmQueue = xQueueCreate(50, sizeof(alarm_t));

  xTaskCreatePinnedToCore(mqttAndSendTelemetryTask, "MQTT", 4096, NULL, 2, NULL, 0);

  xTaskCreatePinnedToCore(loraAndAlarmTask, "LoRa+Alarm", 4096, NULL, 3, NULL, 1);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}