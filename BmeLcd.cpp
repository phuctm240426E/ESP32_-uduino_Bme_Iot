#include "BmeLcd.h"

#if defined(ESP32) && (ESP32 == 0)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#endif

void initOLED() {
#if defined(ESP32) && (ESP32 == 0)
  Serial.println("OLED Init");
  while(!Wire.begin(I2C_SDA, I2C_SCL)) {
    Serial.print(".");
  }; 
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED Fail");
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("GATEWAY BOOTING...");
  display.display();
#endif
}


