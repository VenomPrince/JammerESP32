/* ========================================
   JAM2M - LED Status Indicator
   ======================================== */
   
#include "JAM2M_settings.h"
#include "JAM2M_config.h"

extern Adafruit_NeoPixel statusLED;

void initNeoPixel() {
  EEPROM.begin(512); 
  neoEnabled = EEPROM.read(0);
  
  if (neoEnabled) {
    statusLED.begin();
    statusLED.clear();
  }
}

void setLEDColor(const std::string& color) {
  uint32_t rgbValue = 0;
  
  if (color == "red")     rgbValue = statusLED.Color(5, 0, 0);
  else if (color == "green")   rgbValue = statusLED.Color(0, 5, 0);
  else if (color == "blue")    rgbValue = statusLED.Color(0, 0, 5);
  else if (color == "yellow")  rgbValue = statusLED.Color(5, 5, 0);
  else if (color == "purple")  rgbValue = statusLED.Color(5, 0, 5);
  else if (color == "cyan")    rgbValue = statusLED.Color(0, 5, 5);
  else if (color == "white")   rgbValue = statusLED.Color(5, 5, 5);
  else if (color == "off")     rgbValue = statusLED.Color(0, 0, 0);

  statusLED.setPixelColor(0, rgbValue);
  statusLED.show();
}

void flashLED(int flashCount, const std::vector<std::string>& colorSequence, const std::string& finalColor) {
  if (flashCount <= 0 || colorSequence.empty()) {
    Serial.println("ERROR: Invalid flash parameters");
    return;
  }

  for (int i = 0; i < flashCount; ++i) {
    for (const auto& color : colorSequence) {
      setLEDColor(color);
      delay(500);
    }
  }
  setLEDColor(finalColor);
}