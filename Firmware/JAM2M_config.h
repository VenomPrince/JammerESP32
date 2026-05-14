/* ========================================
   ██╗ █████╗ ███╗   ███╗██████╗ ███╗   ███╗
   ██║██╔══██╗████╗ ████║╚════██╗████╗ ████║
   ██║███████║██╔████╔██║ █████╔╝██╔████╔██║
   ██║██╔══██║██║╚██╔╝██║██╔═══╝ ██║╚██╔╝██║
   ██║██║  ██║██║ ╚═╝ ██║███████╗██║ ╚═╝ ██║
   ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝╚═╝     ╚═╝
   
   JAM2M - Wireless Spectrum Tool
   =========================================
   License: MIT
   ======================================== */

#ifndef JAM2M_CONFIG_H
#define JAM2M_CONFIG_H

// ================== DISPLAY SETUP ==================
#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  64

// ================== HARDWARE PINOUT ==================
// --- Button Interface ---
#define BTN_LEFT        27
#define BTN_RIGHT       25
#define BTN_ACTION      26

// --- nRF24 Radio Modules (Triple Setup) ---
// Radio Unit A
#define RF24_CE_A       5   
#define RF24_CSN_A      17 

// Radio Unit B  
#define RF24_CE_B       16  
#define RF24_CSN_B      4   

// Radio Unit C
#define RF24_CE_C       15  
#define RF24_CSN_C      2   

// ================== LIBRARY IMPORTS ==================
#include "JAM2M_settings.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include <Preferences.h>
#include <vector>
#include <string>
#include <math.h>

// ================== HARDWARE OBJECTS ==================
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oledScreen(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
Adafruit_NeoPixel statusLED(1, 14, NEO_GRB + NEO_KHZ800);

// ================== WIRELESS LIBRARIES ==================
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <esp_bt.h>

// ================== GLOBAL REFERENCES ==================
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C oledScreen;
extern Adafruit_NeoPixel statusLED;

// ================== TYPOGRAPHY ==================
static const uint8_t* FONT_TINY   = u8g2_font_5x8_tf;
static const uint8_t* FONT_NORMAL = u8g2_font_6x12_tf;
static const uint8_t* FONT_ICONS  = u8g2_font_open_iconic_thing_2x_t;

// ================== OPERATION MODES ==================
enum RadioMode {
  MODE_WIFI,
  MODE_VTX,
  MODE_RC,
  MODE_BLE,
  MODE_BT_CLASSIC,
  MODE_USB_WIRELESS,
  MODE_ZIGBEE,
  MODE_NRF24
};

enum SystemState {
  STATE_IDLE,
  STATE_ACTIVE
};

RadioMode currentMode = MODE_WIFI;
volatile SystemState systemState = STATE_IDLE;

// ================== FREQUENCY CHANNELS ==================
byte grp1_channels[] = {2, 5, 8, 11};
byte grp2_channels[] = {26, 29, 32, 35};
byte grp3_channels[] = {80, 83, 86, 89};

const byte bt_classic_channels[] =   {32, 34, 46, 48, 50, 52, 0, 1, 2, 4, 6, 8, 22, 24, 26, 28, 30, 74, 76, 78, 80};
const byte ble_channels[] =          {2, 26, 80};
const byte wifi_channels[] =         {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
const byte usb_channels[] =          {40, 50, 60};
const byte vtx_channels[] =          {70, 75, 80};
const byte rc_channels[] =           {1, 3, 5, 7};
const byte zigbee_channels[] =       {11, 15, 20, 25};
const byte nrf24_channels[] =        {76, 78, 79};

// ================== FLAGS & DEBOUNCE ==================
volatile bool flagPrevPressed  = false;
volatile bool flagNextPressed  = false;
volatile bool flagTogglePressed = false;

unsigned long lastDebounce = 0;
const unsigned long DEBOUNCE_MS = 200;

#endif // JAM2M_CONFIG_H