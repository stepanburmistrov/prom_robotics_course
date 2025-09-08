
/*
  PWM demo with two buttons (INPUT_PULLUP).
  - BTN1 (D5): brightness up
  - BTN2 (D6): brightness down
  - Hold for continuous change, non‑blocking.
  - Works on PWM pin D9 (UNO PWM pins: 3,5,6,9,10,11).

  Wiring:
    D9 -> LED (through 220–330 Ω) -> GND
    D5 ↔ GND (INPUT_PULLUP)
    D6 ↔ GND (INPUT_PULLUP)
*/
#include <Arduino.h>

const uint8_t PWM_LED = 9;
const uint8_t BTN_UP  = 5;
const uint8_t BTN_DN  = 6;

const unsigned long REPEAT_MS = 60;   // repeat while holding
const uint8_t STEP = 8;               // change per step

// Debounce
const unsigned long DEBOUNCE = 15;
unsigned long lastUpMs=0, lastDnMs=0;
bool upStable=HIGH, dnStable=HIGH;
unsigned long upChangeMs=0, dnChangeMs=0;

uint8_t level = 0; // 0..255

void apply() {
  analogWrite(PWM_LED, level);
  Serial.print("PWM="); Serial.println(level);
}

void setup() {
  Serial.begin(115200);
  pinMode(PWM_LED, OUTPUT);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DN, INPUT_PULLUP);
  apply();
  Serial.println("PWM fade with two buttons");
}

void loop() {
  unsigned long now = millis();
  // debounce: UP
  bool ru = digitalRead(BTN_UP);
  if (ru != upStable && (now - upChangeMs) > DEBOUNCE) { upStable = ru; upChangeMs = now; if (upStable==LOW){ level = (level + STEP > 255) ? 255 : level + STEP; apply(); lastUpMs = now; } }
  // debounce: DN
  bool rd = digitalRead(BTN_DN);
  if (rd != dnStable && (now - dnChangeMs) > DEBOUNCE) { dnStable = rd; dnChangeMs = now; if (dnStable==LOW){ level = (level >= STEP) ? level - STEP : 0; apply(); lastDnMs = now; } }

  // repeat while holding
  if (upStable==LOW && (now - lastUpMs) >= REPEAT_MS) { lastUpMs = now; level = (level + STEP > 255) ? 255 : level + STEP; apply(); }
  if (dnStable==LOW && (now - lastDnMs) >= REPEAT_MS) { lastDnMs = now; level = (level >= STEP) ? level - STEP : 0; apply(); }
}
