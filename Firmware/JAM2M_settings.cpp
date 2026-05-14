/* ========================================
   JAM2M - System Configuration Implementation
   ======================================== */

#include "JAM2M_settings.h"
#include "JAM2M_config.h"

// ================== RADIO OBJECTS ==================
RF24 radioUnitA(RF24_CE_A, RF24_CSN_A);
RF24 radioUnitB(RF24_CE_B, RF24_CSN_B);
RF24 radioUnitC(RF24_CE_C, RF24_CSN_C);

// ================== RADIO CONTROL ==================
void resetAllRadios() {
  radioUnitA.stopListening();
  radioUnitA.setAutoAck(false);
  radioUnitA.setRetries(0, 0);
  radioUnitA.powerDown(); 
  digitalWrite(RF24_CE_A, LOW);

  radioUnitB.stopListening();
  radioUnitB.setAutoAck(false);
  radioUnitB.setRetries(0, 0);
  radioUnitB.powerDown(); 
  digitalWrite(RF24_CE_B, LOW);

  radioUnitC.stopListening();
  radioUnitC.setAutoAck(false);
  radioUnitC.setRetries(0, 0);
  radioUnitC.powerDown(); 
  digitalWrite(RF24_CE_C, LOW);
}

void configRadio(RF24 &radio) {
  radio.begin();
  radio.setAutoAck(false);
  radio.stopListening();
  radio.setRetries(0, 0);
  radio.setPALevel(RF24_PA_MAX, true);
  radio.setDataRate(RF24_2MBPS);
  radio.setCRCLength(RF24_CRC_DISABLED);
}

void initRadioA() { configRadio(radioUnitA); }
void initRadioB() { configRadio(radioUnitB); }
void initRadioC() { configRadio(radioUnitC); }
void initAllRadioUnits() { initRadioA(); initRadioB(); initRadioC(); }

// ================== DISPLAY UTILITIES ==================
void drawText(uint8_t x, uint8_t y, const uint8_t* asciiData, size_t length) {
  char buffer[64]; 
  for (size_t i = 0; i < length && i < sizeof(buffer) - 1; i++) {
    buffer[i] = (char)asciiData[i];
  }
  buffer[length] = '\0';
  oledScreen.drawStr(x, y, buffer);
}

void drawCentered(uint8_t screenW, uint8_t y, const uint8_t* asciiData, size_t length, const uint8_t* font) {
  char buffer[64];
  for (size_t i = 0; i < length && i < sizeof(buffer) - 1; i++) {
    buffer[i] = (char)asciiData[i];
  }
  buffer[length] = '\0';
  
  oledScreen.setFont(font);
  int16_t textWidth = oledScreen.getUTF8Width(buffer);
  oledScreen.setCursor((screenW - textWidth) / 2, y);
  oledScreen.print(buffer);
}

void showSplashScreen() {
  oledScreen.setBitmapMode(1);
  oledScreen.clearBuffer();
  drawCentered(DISPLAY_WIDTH, 15, splash_title, sizeof(splash_title), u8g2_font_ncenB14_tr);
  drawCentered(120, 35, splash_author, sizeof(splash_author), u8g2_font_ncenB08_tr);
  drawCentered(DISPLAY_WIDTH, 50, splash_version, sizeof(splash_version), u8g2_font_6x10_tf);
  oledScreen.sendBuffer();
  delay(3000);
  
  oledScreen.clearBuffer();
  oledScreen.drawXBMP(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, bootLogo);
  oledScreen.sendBuffer();
  delay(250);
}