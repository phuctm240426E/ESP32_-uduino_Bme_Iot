#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "BmeLora.h"


// WiFi
const char* WIFI_SSID     = "DoanDong";
const char* WIFI_PASSWORD = "dong1958";

// ThingsBoard
const char* THINGSBOARD_HOST = "thingsboard.cloud";
const char* ACCESS_TOKEN    = "ef2ZhoEBy8MISx0OqMWJ";

// Topics
const char* TELEMETRY_TOPIC = "v1/devices/me/telemetry";
const char* RPC_SUB_TOPIC   = "v1/devices/me/rpc/request/+";

// Timing
const unsigned long SEND_INTERVAL = 5000;

QueueHandle_t loraQueue;
QueueHandle_t alarmQueue;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastSend = 0;


void setupWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}


void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to ThingsBoard...");
    if (client.connect("ESP32_HEALTH_DEVICE", ACCESS_TOKEN, NULL)) {
      Serial.println("connected");
      client.subscribe(RPC_SUB_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(3000);
    }
  }
}


void rpcCallback(char* topic, byte* payload, unsigned int length) {
  Serial.println("\n=== RPC RECEIVED ===");

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.println("JSON parse failed");
    return;
  }

  const char* method = doc["method"];
  JsonObject params = doc["params"];

  if (strcmp(method, "healthAlert") == 0) {
    int id          = params["id"];
    float heartRate = params["heartRate"];
    float spo2      = params["spo2"];

    Serial.printf("ALERT for ID %d\n", id);
    Serial.printf("HeartRate: %.1f\n", heartRate);
    Serial.printf("SpO2: %.1f\n", spo2);

    JsonArray alerts = params["alerts"];
    for (JsonVariant a : alerts) {
      Serial.print("Condition: ");
      Serial.println(a.as<const char*>());
    }

    // 🔴 HERE is where you can add:
    // - buzzer
    // - LED
    // - relay
    // - display update
  }

  // ---------- RPC Response ----------
  String topicStr = String(topic);
  String requestId = topicStr.substring(
    String("v1/devices/me/rpc/request/").length()
  );

  String responseTopic = "v1/devices/me/rpc/response/" + requestId;
  client.publish(responseTopic.c_str(), "{\"result\":\"OK\"}");
}


void sendTelemetry() {
  // Simulated sensor values
  lora_data_packet_t loraData;
  if(getLoraPacket(&loraData)) {

    

    StaticJsonDocument<128> doc;

    char key[24];

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
}

void mqttAndSendTelemetryTask(void* pvParameters) {

}

void loraAndAlarmTask(void* pvParameters) {

}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));

  setupWiFi();
  client.setServer(THINGSBOARD_HOST, 1883);
  client.setCallback(rpcCallback);

  loraQueue = xQueueCreate(10, sizeof(lora_data_packet_t));
  alarmQueue = xQueueCreate(5, sizeof(lora_data_packet_t));

  xTaskCreatePinnedToCore(
  mqttAndSendTelemetryTask,
  "MQTT",
  4096,
  NULL,
  2,
  NULL,
  0
  );

  xTaskCreatePinnedToCore(
  loraAndAlarmTask,
  "LoRa+Alarm",
  4096,
  NULL,
  3,
  NULL,
  1
  );
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - lastSend > SEND_INTERVAL) {
    lastSend = millis();
    sendTelemetry();
  }
}

/*
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "BmeLora.h"

#define WIFI_SSID     "DoanDong"
#define WIFI_PASSWORD "dong1958"

#define THINGSBOARD_HOST "thingsboard.cloud"
#define ACCESS_TOKEN    "ef2ZhoEBy8MISx0OqMWJ"

#define TELEMETRY_TOPIC "v1/devices/me/telemetry"
#define RPC_SUB_TOPIC   "v1/devices/me/rpc/request/+"

QueueHandle_t loraQueue;

WiFiClient espClient;
PubSubClient client(espClient);

void wifiTask(void *pvParameters) {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (true) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi reconnecting...");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

void mqttTask(void *pvParameters) {
  client.setServer(THINGSBOARD_HOST, 1883);
  client.setCallback(rpcCallback);

  while (true) {
    if (!client.connected()) {
      while (!client.connected()) {
        if (client.connect("ESP32_GATEWAY", ACCESS_TOKEN, NULL)) {
          client.subscribe(RPC_SUB_TOPIC);
        } else {
          vTaskDelay(pdMS_TO_TICKS(3000));
        }
      }
    }
    client.loop();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void loraTask(void *pvParameters) {
  lora_data_packet_t packet;

  while (true) {
    if (getLoraPacket(&packet)) {
      xQueueSend(loraQueue, &packet, portMAX_DELAY);
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}


void telemetryTask(void *pvParameters) {
  lora_data_packet_t packet;

  while (true) {
    if (xQueueReceive(loraQueue, &packet, portMAX_DELAY)) {

      StaticJsonDocument<128> doc;
      char key[24];

      snprintf(key, sizeof(key), "HeartRate_%u", packet.deviceId);
      doc[key] = packet.heartRate;

      snprintf(key, sizeof(key), "SpO2_%u", packet.deviceId);
      doc[key] = packet.spO2;

      char payload[128];
      serializeJson(doc, payload);

      Serial.println(payload);
      client.publish(TELEMETRY_TOPIC, payload);
    }
  }
}

void rpcCallback(char* topic, byte* payload, unsigned int length) {
  Serial.println("\n=== RPC RECEIVED ===");

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.println("JSON parse failed");
    return;
  }

  const char* method = doc["method"];
  JsonObject params = doc["params"];

  if (strcmp(method, "healthAlert") == 0) {
    int id          = params["id"];
    float heartRate = params["heartRate"];
    float spo2      = params["spo2"];

    Serial.printf("ALERT for ID %d\n", id);
    Serial.printf("HeartRate: %.1f\n", heartRate);
    Serial.printf("SpO2: %.1f\n", spo2);

    JsonArray alerts = params["alerts"];
    for (JsonVariant a : alerts) {
      Serial.print("Condition: ");
      Serial.println(a.as<const char*>());
    }

    // 🔴 HERE is where you can add:
    // - buzzer
    // - LED
    // - relay
    // - display update
  }

  // ---------- RPC Response ----------
  String topicStr = String(topic);
  String requestId = topicStr.substring(
    String("v1/devices/me/rpc/request/").length()
  );

  String responseTopic = "v1/devices/me/rpc/response/" + requestId;
  client.publish(responseTopic.c_str(), "{\"result\":\"OK\"}");
}

void setup() {
  Serial.begin(115200);

  loraQueue = xQueueCreate(10, sizeof(lora_data_packet_t));

  xTaskCreatePinnedToCore(wifiTask, "WiFi", 4096, NULL, 1, NULL, 0);
  //xTaskCreatePinnedToCore(mqttTask, "MQTT", 4096, NULL, 2, NULL, 0);
  //xTaskCreatePinnedToCore(loraTask, "LoRa", 4096, NULL, 3, NULL, 1);
  //xTaskCreatePinnedToCore(telemetryTask, "Telemetry", 4096, NULL, 2, NULL, 0);
}

void loop() {
  vTaskDelay(portMAX_DELAY); // RTOS owns everything
}*/