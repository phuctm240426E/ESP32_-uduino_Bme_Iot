#include <WiFi.h>

#include "Sensor.h"
#include "BmeLora.h"
#include "BmeLcd.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 99
#endif

#include "Debug.h"

#include <Arduino_MQTT_Client.h>
#include <Server_Side_RPC.h>
#include <Attribute_Request.h>
#include <Shared_Attribute_Update.h>
#include <ThingsBoard.h>

constexpr char WIFI_SSID[] = "DoanDong";
constexpr char WIFI_PASSWORD[] = "dong1958";

constexpr char TOKEN[] = "I5hqfWK0FXG0373PC5s1";

constexpr char THINGSBOARD_SERVER[] = "demo.thingsboard.io";

constexpr uint16_t THINGSBOARD_PORT = 1883U;

constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;

constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;

constexpr size_t MAX_ATTRIBUTES = 3U;

//constexpr uint64_t REQUEST_TIMEOUT_MICROSECONDS = 5000U * 1000U;

WiFiClient wifiClient;

// Initalize the Mqtt client instance
Arduino_MQTT_Client mqttClient(wifiClient);

// Initialize used apis
Server_Side_RPC<3U, 5U> rpc;
Attribute_Request<2U, MAX_ATTRIBUTES> attr_request;
Shared_Attribute_Update<3U, MAX_ATTRIBUTES> shared_update;

const std::array<IAPI_Implementation*, 3U> apis = {
    &rpc,
    &attr_request,
    &shared_update
};

// Initialize ThingsBoard instance with the maximum needed buffer size, stack size and the apis we want to use
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE, Default_Max_Stack_Size, apis);

void InitWiFi() {
  DEBUG("Connecting to AP ...");
  // Attempting to establish a connection to the given WiFi network
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    // Delay 500ms until a connection has been succesfully established
    delay(500);
    Serial.print(".");
  }
  DEBUG("Connected to AP");
  display.println("WiFi: Connected");
  display.display();
}

const bool reconnect() {
  
  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    return true;
  }

  InitWiFi();
  return true;
}

void onRpcCall(const JsonVariantConst &data, JsonDocument &response) {
  // Method name
  const char* method = data["method"];
  Serial.println(method);
  if (strcmp(method, "alarm") == 0) {
    bool active = data["params"]["active"];

    if (active) {
      Serial.println(" ALARM ON (HeartRate_2 > 80)");
      //digitalWrite(LED_PIN, HIGH);
    } else {
      Serial.println("ALARM OFF");
      //digitalWrite(LED_PIN, LOW);
    }
  }

  // Optional response
  response["status"] = "ok";
}

const std::array<RPC_Callback, 1U> callbacks = {
  RPC_Callback{ "High_heart_rate", onRpcCall }
};

void setup() {
  Serial.begin(SERIAL_DEBUG_BAUD);
  initOLED();
  delay(1000);
  InitWiFi();
  initBmeLora(); 
}

void loop() {
  delay(10);

  if (!reconnect()) {
    return;
  }

  if (!tb.connected()) {
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    DEBUG(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) {
      DEBUG("Failed to connect");
      return;
    }

    DEBUG("Subscribing for RPC...");

    if (!rpc.RPC_Subscribe(callbacks.cbegin(), callbacks.cend())) {
      DEBUG("Failed to subscribe for RPC");
      return;
    }
  }
  
  lora_data_packet_t lora_data;
  bool hasData = getLoraPacket(&lora_data);
  if(hasData) {
    int rssi = LoRa.packetRssi();
    display.clearDisplay();
    // Vung mau vang
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("RSSI: "); display.print(rssi); display.println(" dBm");
    display.drawFastHLine(0, 12, 128, WHITE);
    

    String key;
    Serial.println("Receive packet: ");
    Serial.println(lora_data.deviceId);

    display.setCursor(0, 15);
    display.setTextSize(1); display.print("HEALTH DATA:"); display.print(lora_data.deviceId); display.println("");

    Serial.println(lora_data.spO2);
    key = "SpO2_" + String(lora_data.deviceId);
    tb.sendTelemetryData(key.c_str(), lora_data.spO2);
    display.setCursor(0, 25);
    display.print("SpO2:"); display.print(lora_data.spO2);

    Serial.println(lora_data.heartRate);
    key = "HeartRate_" + String(lora_data.deviceId);
    tb.sendTelemetryData(key.c_str(), lora_data.heartRate);
    display.setCursor(0, 35);
    display.print("HR:"); display.print(lora_data.heartRate);

    Serial.println(lora_data.bloodPressure.systolic);
    key = "Systolic_" + String(lora_data.deviceId);
    tb.sendTelemetryData(key.c_str(), lora_data.bloodPressure.systolic);
    display.setCursor(0, 45);
    display.print("systolic:"); display.print(lora_data.bloodPressure.systolic);

    Serial.println(lora_data.bloodPressure.diastolic);
    key = "Diastolic_" + String(lora_data.deviceId);
    tb.sendTelemetryData(key.c_str(), lora_data.bloodPressure.diastolic);
    display.setCursor(0, 55);
    display.print("diastolic:"); display.print(lora_data.bloodPressure.diastolic);

    Serial.println(lora_data.CRC);
    display.display();
  }
  
  tb.loop();
}
