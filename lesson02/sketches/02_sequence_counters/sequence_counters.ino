
/*
  Sequence with counters (non‑blocking):
  - LEDs sequence index advances every INTERVAL.
  - After N_STEPS_PER_CYCLE steps we count one cycle and change speed.
  - Demonstrates using variables as counters and Serial for tracing.

  Wiring: same as Lesson 1 (D2,D3,D4 to LEDs).
*/
#include <Arduino.h>

const uint8_t LEDS[] = {2, 3, 4};
const unsigned long INTERVAL_FAST = 120;
const unsigned long INTERVAL_SLOW = 400;

unsigned long lastStepMs = 0;
unsigned long intervalMs = INTERVAL_SLOW;

uint8_t stepIndex = 0;
uint16_t stepCounter = 0;
uint8_t  cycleCounter = 0;
const uint16_t N_STEPS_PER_CYCLE = 3 * 5; // 5 full loops across 3 LEDs

void setAll(uint8_t state) { for (auto p : LEDS) digitalWrite(p, state); }

void showStep(uint8_t idx) {
  setAll(LOW);
  digitalWrite(LEDS[idx], HIGH);
}

void setup() {
  Serial.begin(115200);
  for (auto p : LEDS) pinMode(p, OUTPUT);
  setAll(LOW);
  Serial.println("Sequence counters demo");
}

void loop() {
  unsigned long now = millis();
  if (now - lastStepMs >= intervalMs) {
    lastStepMs = now;
    showStep(stepIndex);
    stepIndex = (stepIndex + 1) % 3;
    stepCounter++;

    // Every full N_STEPS_PER_CYCLE → flip speed
    if (stepCounter % N_STEPS_PER_CYCLE == 0) {
      cycleCounter++;
      intervalMs = (intervalMs == INTERVAL_SLOW) ? INTERVAL_FAST : INTERVAL_SLOW;
      Serial.print("Cycle #"); Serial.print(cycleCounter);
      Serial.print(" | intervalMs="); Serial.println(intervalMs);
    }
  }
}
