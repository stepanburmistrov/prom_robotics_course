
/*
  Lesson 2: C++. Button. Non‑blocking code with millis().
  Short press: cycle blink modes (FAST → SLOW → ALL).
  Long press (>= 3000 ms): immediately switch to OFF (all LEDs off).
  Next any press returns to the last working mode.
  Debounce: edge detection with a time guard.
  Serial is used for debugging.

  Wiring (UNO):
    LEDs: D2 (RED), D3 (YELLOW), D4 (GREEN) — through 220–330 Ω to GND.
    Button1: D5 ↔ GND (INPUT_PULLUP).

  Author: your course
*/
#include <Arduino.h>

// ---------------- Pins
const uint8_t LEDS[] = {2, 3, 4};   // RED, YELLOW, GREEN
const uint8_t BTN1   = 5;           // single button

// ---------------- Modes
enum Mode : uint8_t { MODE_FAST = 0, MODE_SLOW, MODE_ALL, MODE_OFF };
const char* modeName(Mode m) {
  switch (m) {
    case MODE_FAST: return "FAST (sequential)";
    case MODE_SLOW: return "SLOW (sequential)";
    case MODE_ALL:  return "ALL (sync blink)";
    case MODE_OFF:  return "OFF";
  }
  return "?";
}

Mode  currentMode = MODE_FAST;
Mode  lastWorkMode = MODE_FAST;      // to restore after MODE_OFF

// ---------------- Timing
unsigned long lastStepMs = 0;
const unsigned long FAST_STEP = 100;
const unsigned long SLOW_STEP = 400;
const unsigned long ALL_STEP  = 250;

// sequence index
uint8_t idx = 0;

// ---------------- Button handling (debounce + long press)
bool          btnStable       = HIGH; // INPUT_PULLUP idle = HIGH
bool          btnPrevStable   = HIGH;
unsigned long btnLastChangeMs = 0;
const unsigned long DEBOUNCE  = 15;

bool          isLockedOff     = false;      // set by long press
unsigned long pressStartMs    = 0;
bool          longTriggered   = false;

// helper
void setAll(uint8_t state) {
  for (uint8_t i = 0; i < 3; ++i) digitalWrite(LEDS[i], state);
}

void switchToMode(Mode m) {
  if (m != MODE_OFF) lastWorkMode = m;
  currentMode = m;
  Serial.print("Mode => ");
  Serial.println(modeName(currentMode));

  // reset sequence state
  lastStepMs = millis();
  idx = 0;
  if (currentMode == MODE_OFF) setAll(LOW);
}

void handleButton() {
  // read and debounce
  bool raw = digitalRead(BTN1);
  unsigned long now = millis();
  if (raw != btnStable && (now - btnLastChangeMs) > DEBOUNCE) {
    btnPrevStable   = btnStable;
    btnStable       = raw;
    btnLastChangeMs = now;

    // Edge: pressed (HIGH -> LOW)
    if (btnPrevStable == HIGH && btnStable == LOW) {
      pressStartMs  = now;
      longTriggered = false;

      if (!isLockedOff) {
        // "Immediate" reaction to a short press: cycle mode right away
        if      (currentMode == MODE_FAST) switchToMode(MODE_SLOW);
        else if (currentMode == MODE_SLOW) switchToMode(MODE_ALL);
        else if (currentMode == MODE_ALL)  switchToMode(MODE_FAST);
        else if (currentMode == MODE_OFF)  switchToMode(lastWorkMode);
      } else {
        // If OFF was locked, any press returns to last working mode
        isLockedOff = false;
        switchToMode(lastWorkMode);
      }
    }
  }

  // Long press timer (do not wait for release)
  if (btnStable == LOW) {
    unsigned long held = now - pressStartMs;
    if (!longTriggered && held >= 3000) {
      longTriggered = true;
      isLockedOff   = true;
      switchToMode(MODE_OFF);
      Serial.println("Long press >=3s → MODE_OFF");
    }
  }
}

void doBlinkLogic() {
  unsigned long now = millis();

  if (currentMode == MODE_OFF) return;

  if (currentMode == MODE_FAST || currentMode == MODE_SLOW) {
    unsigned long step = (currentMode == MODE_FAST) ? FAST_STEP : SLOW_STEP;
    if (now - lastStepMs >= step) {
      lastStepMs = now;
      // turn all off, then current on
      setAll(LOW);
      digitalWrite(LEDS[idx], HIGH);
      idx = (idx + 1) % 3;
    }
  } else if (currentMode == MODE_ALL) {
    if (now - lastStepMs >= ALL_STEP) {
      lastStepMs = now;
      // toggle all together
      static bool on = false;
      on = !on;
      for (uint8_t i = 0; i < 3; ++i) digitalWrite(LEDS[i], on ? HIGH : LOW);
    }
  }
}

void setup() {
  Serial.begin(115200);
  for (uint8_t i = 0; i < 3; ++i) pinMode(LEDS[i], OUTPUT);
  pinMode(BTN1, INPUT_PULLUP);
  setAll(LOW);
  switchToMode(currentMode);
  Serial.println("Lesson 2: modes_button_longpress started");
}

void loop() {
  handleButton();
  doBlinkLogic();
}
