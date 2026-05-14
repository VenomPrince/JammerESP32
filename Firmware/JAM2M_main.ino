/* ========================================
   JAM2M - Main Application Entry Point
   ======================================== */
   
#include "JAM2M_settings.h"
#include "JAM2M_config.h"

// ================== INTERRUPT HANDLERS ==================
void IRAM_ATTR onLeftButton() {
  unsigned long now = millis();
  if (now - lastDebounce > DEBOUNCE_MS) {
    flagPrevPressed = true;
    lastDebounce = now;
  }
}

void IRAM_ATTR onRightButton() {
  unsigned long now = millis();
  if (now - lastDebounce > DEBOUNCE_MS) {
    flagNextPressed = true;
    lastDebounce = now;
  }
}

void IRAM_ATTR onActionButton() {
  unsigned long now = millis();
  if (now - lastDebounce > DEBOUNCE_MS) {
    systemState = (systemState == STATE_IDLE) ? STATE_ACTIVE : STATE_IDLE;
    lastDebounce = now;
  }
}

// ================== RADIO CONTROL ==================
void setupRadioJammer(RF24 &radio, const byte *freqList, size_t listSize) {
  configRadio(radio);
  radio.printPrettyDetails();
  for (size_t i = 0; i < listSize; i++) {
    radio.setChannel(freqList[i]);
    radio.startConstCarrier(RF24_PA_MAX, freqList[i]);
  }
}

void startJammerMode() {
  if (radioUnitA.begin()) {
    setupRadioJammer(radioUnitA, grp1_channels, sizeof(grp1_channels));
  }
  if (radioUnitB.begin()) {
    setupRadioJammer(radioUnitB, grp2_channels, sizeof(grp2_channels));
  }
  if (radioUnitC.begin()) {
    setupRadioJammer(radioUnitC, grp3_channels, sizeof(grp3_channels));
  }
}

void updateRadioState() {
  if (systemState == STATE_ACTIVE) {
    startJammerMode();
  } else {
    radioUnitA.powerDown();
    radioUnitB.powerDown();
    radioUnitC.powerDown();
    delay(100);
  }
}

// ================== UI CONSTANTS ==================
static const uint8_t HEADER_H = 12;
static const uint8_t PADDING  = 6;
static const uint8_t CARD_H   = 36;
static const uint8_t CORNER_R = 3;
static const uint8_t DOT_Y    = 64 - 6;

static const char* MENU_ITEMS[] = {
  "WiFi", "Video TX", "RC", "BLE", 
  "Bluetooth", "USB", "Zigbee", "NRF24"
};
static const int MENU_COUNT = sizeof(MENU_ITEMS)/sizeof(MENU_ITEMS[0]);

static int getMenuIndex(RadioMode mode) {
  switch (mode) {
    case MODE_WIFI:          return 0;
    case MODE_VTX:           return 1;
    case MODE_RC:            return 2;
    case MODE_BLE:           return 3;
    case MODE_BT_CLASSIC:    return 4;
    case MODE_USB_WIRELESS:  return 5;
    case MODE_ZIGBEE:        return 6;
    case MODE_NRF24:         return 7;
    default: return 0;
  }
}

static RadioMode getModeFromIndex(int idx) {
  switch (idx) {
    case 0: return MODE_WIFI;
    case 1: return MODE_VTX;
    case 2: return MODE_RC;
    case 3: return MODE_BLE;
    case 4: return MODE_BT_CLASSIC;
    case 5: return MODE_USB_WIRELESS;
    case 6: return MODE_ZIGBEE;
    case 7: return MODE_NRF24;
    default: return MODE_WIFI;
  }
}

// ================== SPECTRUM VISUALIZER ==================
void drawSpectrum() {
  const uint8_t HDR_H = 10;
  const int topY = HDR_H + 12;
  const int bottomY = DISPLAY_HEIGHT - 2;
  const int barHeight = bottomY - topY;
  const int stride = 3;
  const int binCount = DISPLAY_WIDTH / stride;
  const int barWidth = 1;
  
  static uint8_t heights[binCount];
  static int8_t velocities[binCount];
  static bool initialized = false;
  
  if (!initialized) {
    for (int i = 0; i < binCount; ++i) {
      heights[i] = random(0, barHeight + 1);
      velocities[i] = random(-2, 3);
    }
    initialized = true;
  }
  
  for (int i = 0; i < binCount; ++i) {
    int8_t delta = (int8_t)random(-1, 2);
    velocities[i] += delta;
    if (velocities[i] > 3) velocities[i] = 3;
    if (velocities[i] < -3) velocities[i] = -3;
    
    int16_t newHeight = (int16_t)heights[i] + velocities[i];
    if (newHeight < 0) {
      newHeight = 0;
      velocities[i] = -(velocities[i] * 3) / 4;
      if (velocities[i] == 0) velocities[i] = 1;
    } else if (newHeight > barHeight) {
      newHeight = barHeight;
      velocities[i] = -(velocities[i] * 3) / 4;
      if (velocities[i] == 0) velocities[i] = -1;
    }
    heights[i] = (uint8_t)newHeight;
  }
  
  for (int i = 0; i < binCount; ++i) {
    int xPos = i * stride;
    int barH = heights[i];
    if (barH <= 0) continue;
    int yPos = bottomY - barH;
    oledScreen.drawVLine(xPos, yPos, barH);
  }
  oledScreen.sendBuffer();
}

// ================== UI DRAWING FUNCTIONS ==================
static void drawTopBar() {
  oledScreen.setDrawColor(1);
  oledScreen.drawBox(0, 0, DISPLAY_WIDTH, HEADER_H);
  oledScreen.setFont(FONT_TINY);
  oledScreen.setDrawColor(0);
  oledScreen.drawStr(PADDING, 2, "JAM2M");
  
  const char* ver = "v3.0.0";
  int vw = oledScreen.getStrWidth(ver);
  oledScreen.drawStr(DISPLAY_WIDTH - vw - 4, 2, ver);
  
  oledScreen.setDrawColor(1);
  oledScreen.drawHLine(0, HEADER_H - 1, DISPLAY_WIDTH);
}

static void drawToggleOutline(int x, int y, int w, int h) {
  if (h & 1) h--; 
  if (y & 1) y--;
  oledScreen.drawRFrame(x, y, w, h, h/2);
}

static void drawToggleKnob(int x, int y, int w, int h, float position) {
  if (h & 1) h--; 
  if (y & 1) y--;
  int radius = h/2;
  int leftPos = x + radius;
  int rightPos = x + w - radius - 1;
  int centerY = y + radius;
  int knobX = (int)lroundf(leftPos + (rightPos - leftPos) * position);
  int knobSize = max(2, radius - 3);
  oledScreen.drawDisc(knobX, centerY, knobSize);
}

static void drawToggleSwitch(int x, int y, int w, int h, bool isOn) {
  drawToggleOutline(x, y, w, h);
  drawToggleKnob(x, y, w, h, isOn ? 0.0f : 1.0f);
}

static void drawPageDots(int activeIndex) {
  int totalWidth = (MENU_COUNT * 6) - 2;
  int startX = (DISPLAY_WIDTH - totalWidth) / 2;
  for (int i = 0; i < MENU_COUNT; i++) {
    int xPos = startX + i * 6;
    if (i == activeIndex) {
      oledScreen.drawDisc(xPos, DOT_Y, 2);
    } else {
      oledScreen.drawCircle(xPos, DOT_Y, 2);
    }
  }
}

static void drawMenuCard(int centerX, int menuIdx) {
  const char* label = MENU_ITEMS[menuIdx];
  const int cardW = DISPLAY_WIDTH - PADDING * 2;
  const int cardX = centerX - cardW / 2;
  const int cardY = HEADER_H + 4;
  
  oledScreen.setDrawColor(0);
  oledScreen.drawBox(cardX + 1, cardY + 1, cardW - 2, CARD_H - 2);
  oledScreen.setDrawColor(1);
  oledScreen.drawRFrame(cardX, cardY, cardW, CARD_H, CORNER_R);
  
  oledScreen.setFont(FONT_NORMAL);
  oledScreen.setCursor(cardX + 12, cardY + 4);
  oledScreen.print(label);
  
  bool isFocused = (getModeFromIndex(menuIdx) == currentMode);
  bool isActive = (isFocused && systemState == STATE_ACTIVE);
  const char* statusText = isActive ? "ACTIVE" : "IDLE";
  
  oledScreen.setFont(FONT_TINY);
  oledScreen.setCursor(cardX + 12, cardY + 16);
  oledScreen.print(statusText);
  
  bool toggleState = (getModeFromIndex(menuIdx) == currentMode) && (systemState == STATE_ACTIVE);
  const int toggleW = 24, toggleH = 14;
  const int toggleY = (cardY + ((CARD_H - toggleH) / 2)) & ~1;
  drawToggleSwitch(cardX + cardW - toggleW - 10, toggleY, toggleW, toggleH, toggleState);
}

static void renderMenu(int focusIdx) {
  oledScreen.clearBuffer();
  drawTopBar();
  drawMenuCard(DISPLAY_WIDTH / 2, focusIdx);
  drawPageDots(focusIdx);
  oledScreen.sendBuffer();
}

static void animateMenuSwitch(int fromIdx, int toIdx) {
  const int steps = 14;
  const int delayMs = 10;
  const int direction = (toIdx > fromIdx) ? -1 : 1;
  
  for (int step = 0; step <= steps; step++) {
    float t = (float)step / (float)steps;
    float ease = (t < 0.5f) ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3) / 2.0f;
    int shift = (int)(ease * DISPLAY_WIDTH + 0.5f);
    int centerFrom = (DISPLAY_WIDTH / 2) + (-direction * shift);
    int centerTo   = (DISPLAY_WIDTH / 2) + (direction * (DISPLAY_WIDTH - shift));
    
    oledScreen.clearBuffer();
    drawTopBar();
    drawMenuCard(centerFrom, fromIdx);
    drawMenuCard(centerTo, toIdx);
    drawPageDots(toIdx);
    oledScreen.sendBuffer();
    delay(delayMs);
  }
  renderMenu(toIdx);
}

static void animateToggle(int focusIdx, bool wasActive, bool nowActive) {
  float startPos = (wasActive ? 0.0f : 1.0f);
  float endPos   = (nowActive ? 0.0f : 1.0f);
  const int steps = 10;
  const int frameDelay = 12;
  
  for (int step = 0; step <= steps; step++) {
    float t = (float)step / (float)steps;
    float ease = (t < 0.5f) ? 2.0f * t * t : 1.0f - powf(-2.0f * t + 2.0f, 2) / 2.0f;
    float pos = startPos + (endPos - startPos) * ease;
    
    oledScreen.clearBuffer();
    drawTopBar();
    
    const int cardW = DISPLAY_WIDTH - PADDING * 2;
    const int cardX = DISPLAY_WIDTH / 2 - cardW / 2;
    const int cardY = HEADER_H + 4;
    
    oledScreen.setDrawColor(0);
    oledScreen.drawBox(cardX + 1, cardY + 1, cardW - 2, CARD_H - 2);
    oledScreen.setDrawColor(1);
    oledScreen.drawRFrame(cardX, cardY, cardW, CARD_H, CORNER_R);
    
    oledScreen.setFont(FONT_NORMAL);
    oledScreen.setCursor(cardX + 12, cardY + 4);
    oledScreen.print(MENU_ITEMS[focusIdx]);
    
    oledScreen.setFont(FONT_TINY);
    oledScreen.setCursor(cardX + 12, cardY + 16);
    oledScreen.print("----");
    
    const int toggleW = 24, toggleH = 14;
    const int toggleY = (cardY + ((CARD_H - toggleH) / 2)) & ~1;
    const int toggleX = cardX + cardW - toggleW - 10;
    
    drawToggleOutline(toggleX, toggleY, toggleW, toggleH);
    drawToggleKnob(toggleX, toggleY, toggleW, toggleH, pos);
    drawPageDots(focusIdx);
    oledScreen.sendBuffer();
    delay(frameDelay);
  }
  renderMenu(focusIdx);
}

void updateDisplay() {
  int focus = getMenuIndex(currentMode);
  renderMenu(focus);
}

void navigatePrev() {
  int fromIdx = getMenuIndex(currentMode);
  int toIdx = (fromIdx == 0) ? (MENU_COUNT - 1) : (fromIdx - 1);
  currentMode = getModeFromIndex(toIdx);
  animateMenuSwitch(fromIdx, toIdx);
}

void navigateNext() {
  int fromIdx = getMenuIndex(currentMode);
  int toIdx = (fromIdx == (MENU_COUNT - 1)) ? 0 : (fromIdx + 1);
  currentMode = getModeFromIndex(toIdx);
  animateMenuSwitch(fromIdx, toIdx);
}

void toggleJammer() {
  int focus = getMenuIndex(currentMode);
  bool wasActive = (systemState == STATE_ACTIVE);
  systemState = wasActive ? STATE_IDLE : STATE_ACTIVE;
  animateToggle(focus, wasActive, !wasActive);
}

void processNavigation() {
  if (flagPrevPressed) {
    flagPrevPressed = false;
    currentMode = static_cast<RadioMode>((currentMode == 0) ? 7 : (currentMode - 1));
  } else if (flagNextPressed) {
    flagNextPressed = false;
    currentMode = static_cast<RadioMode>((currentMode + 1) % 8);
  }
}

// ================== FREQUENCY HOPPING ==================
void updateHoppingChannel() {
  if (systemState != STATE_ACTIVE) return;
  
  int channelIndex;
  byte selectedChannel;
  
  switch (currentMode) {
    case MODE_BLE:
      channelIndex = random(0, sizeof(ble_channels) / sizeof(ble_channels[0]));
      selectedChannel = ble_channels[channelIndex];
      break;
    case MODE_BT_CLASSIC:
      channelIndex = random(0, sizeof(bt_classic_channels) / sizeof(bt_classic_channels[0]));
      selectedChannel = bt_classic_channels[channelIndex];
      break;
    case MODE_WIFI:
      channelIndex = random(0, sizeof(wifi_channels) / sizeof(wifi_channels[0]));
      selectedChannel = wifi_channels[channelIndex];
      break;
    case MODE_USB_WIRELESS:
      channelIndex = random(0, sizeof(usb_channels) / sizeof(usb_channels[0]));
      selectedChannel = usb_channels[channelIndex];
      break;
    case MODE_VTX:
      channelIndex = random(0, sizeof(vtx_channels) / sizeof(vtx_channels[0]));
      selectedChannel = vtx_channels[channelIndex];
      break;
    case MODE_RC:
      channelIndex = random(0, sizeof(rc_channels) / sizeof(rc_channels[0]));
      selectedChannel = rc_channels[channelIndex];
      break;
    case MODE_ZIGBEE:
      channelIndex = random(0, sizeof(zigbee_channels) / sizeof(zigbee_channels[0]));
      selectedChannel = zigbee_channels[channelIndex];
      break;
    case MODE_NRF24:
      channelIndex = random(0, sizeof(nrf24_channels) / sizeof(nrf24_channels[0]));
      selectedChannel = nrf24_channels[channelIndex];
      break;
    default:
      return;
  }
  
  radioUnitA.setChannel(selectedChannel);
  radioUnitB.setChannel(selectedChannel);
  radioUnitC.setChannel(selectedChannel);
}

// ================== SYSTEM INITIALIZATION ==================
void setup() {
  Serial.begin(115200);
  
  // Initialize radio jammers
  startJammerMode();
  
  // Setup display
  Wire.begin();
  Wire.setClock(400000);
  oledScreen.begin();
  oledScreen.setBusClock(400000);
  oledScreen.setFont(FONT_TINY);
  oledScreen.setDrawColor(1);
  oledScreen.setFontPosTop();
  
  // Disable unused wireless features
  esp_bt_controller_deinit();
  esp_wifi_stop();
  esp_wifi_deinit();
  esp_wifi_disconnect();
  
  // Setup button inputs
  pinMode(BTN_LEFT,  INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_ACTION, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(BTN_LEFT),  onLeftButton,   FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_RIGHT), onRightButton,  FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_ACTION), onActionButton, FALLING);
  
  // Update radio state and show UI
  updateRadioState();
  showSplashScreen();
  updateDisplay();
}

// ================== MAIN LOOP ==================
void loop() {
  processNavigation();
  
  static SystemState lastState = systemState;
  static RadioMode lastMode = currentMode;
  
  if (currentMode != lastMode) {
    int fromIdx = getMenuIndex(lastMode);
    int toIdx = getMenuIndex(currentMode);
    animateMenuSwitch(fromIdx, toIdx);
    lastMode = currentMode;
    return;
  }
  
  if (systemState != lastState) {
    updateRadioState();
    int focus = getMenuIndex(currentMode);
    bool wasActive = (lastState == STATE_ACTIVE);
    bool nowActive = (systemState == STATE_ACTIVE);
    animateToggle(focus, wasActive, nowActive);
    lastState = systemState;
    return;
  }
  
  updateHoppingChannel();
}