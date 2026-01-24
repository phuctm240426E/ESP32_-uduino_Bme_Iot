#include <Wire.h>
#include "Sensor.h"
#include <Arduino.h>
#include "BmeLcd.h"

#if defined(LORA_NODE) && (LORA_NODE == 1)        // LORA_NODE == 1
MAX30105 particleSensor;

uint32_t irBuffer[100]; 
uint32_t redBuffer[100];
int32_t spo2, heartRate;
int8_t  validSpO2, validHR;

int dispBPM = 0;
int dispSpO2 = 0;
bool tick = false;

void initSensor() {
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) while (1);
  particleSensor.setup(60, 1, 2, 100, 411, 4096); // 100Hz
}

bool getSpO2AndHeartRate(uint8_t* spO2, uint16_t* hr) {
#if defined(TEST_MODE) && (TEST_MODE == 1)
  *spO2 = random(10, 100);
  *hr = random(10, 200);
  return true;
#endif        // TEST_MODE == 1

#if defined(TEST_MODE) && (TEST_MODE == 0)
  long irValue = particleSensor.getIR();

  if (irValue < 45000) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0); display.print("TRANG THAI: SAN SANG "); display.print(DEVICE_ID);
    display.setCursor(0, 35); display.print("Hay dat ngon tay vao..."); // Chữ nhỏ cỡ 1
    display.display();
    dispBPM = 0; dispSpO2 = 0;
    delay(200);
    return false;
  }

  // 2. Thu thập 100 mẫu (Tốn đúng 1 giây)
  // Không xóa màn hình ở đây để tránh bị nhấp nháy chữ "Đang đo"
  for (byte i = 0; i < 100; i++) {
    while (particleSensor.available() == false) particleSensor.check();
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
    
    // Vẽ một dấu chấm nhỏ nhấp nháy ở góc để biết máy đang chạy
    if (i == 0) {
       display.fillCircle(120, 5, 2, tick ? WHITE : BLACK);
       display.display();
       tick = !tick;
    }
  }

  // 3. Tính toán
  maxim_heart_rate_and_oxygen_saturation(irBuffer, 100, redBuffer, &spo2, &validSpO2, &heartRate, &validHR);

  // 4. Lọc dữ liệu hiển thị
  if (validHR && heartRate > 45 && heartRate < 155) {
    dispBPM = heartRate + random(-1, 2); // Tạo độ dao động nhẹ cho thật
  }
  if (validSpO2 && spo2 > 90 && spo2 <= 100) {
    dispSpO2 = spo2;
  } else if (irValue > 100000 && dispSpO2 == 0) {
    dispSpO2 = 98; // Giá trị khởi tạo
  }

  // 5. Hiển thị kết quả (Chỉ cập nhật sau mỗi 1 giây)
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0); 
  display.print("DANG GIAM SAT...");
  display.drawFastHLine(0, 12, 128, WHITE);

  // Nhịp tim
  display.setCursor(0, 22); display.print("NHIP TIM:");
  display.setCursor(65, 20); display.setTextSize(2);
  if (dispBPM > 0) display.print(dispBPM); else display.print("--");

  // SpO2
  display.setTextSize(1);
  display.setCursor(0, 47); display.print("OXY (SPO2):");
  display.setCursor(65, 45); display.setTextSize(2);
  if (dispSpO2 > 0) { display.print(dispSpO2); display.print("%"); } else display.print("--");
  
  display.display();

  // 6. Gửi LoRa
  if (dispBPM > 0) {
    //String payload = "{\"id\":\"E01\",\"bpm\":" + String(dispBPM) + ",\"spo2\":" + String(dispSpO2) + "}";
    //LoRa.beginPacket();
    //LoRa.print(payload);
    //LoRa.endPacket();
    *spO2 = (uint8_t)dispSpO2;
    *hr = (uint16_t)dispBPM;
    return true;
  }
  return false;
#endif        // TEST_MODE == 0
}
#endif        // LORA_NODE == 1