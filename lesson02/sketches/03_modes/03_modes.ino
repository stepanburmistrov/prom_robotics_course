
#define LED_R 2
#define LED_Y 3
#define LED_G 4
#define BTN   5

uint8_t LEDS[3] = { LED_R, LED_Y, LED_G };

#define MODE_FAST 0
#define MODE_SLOW 1
#define MODE_ALL  2
#define MODE_OFF  3

uint8_t current = MODE_FAST;

const unsigned long FAST_STEP = 150;   // было 100
const unsigned long SLOW_STEP = 600;   // было 500
const unsigned long ALL_STEP  = 400;   // было 250

unsigned long lastStep = 0, btnLast = 0, pressStart = 0;
uint8_t idx = 0;

bool btnStable = HIGH, btnPrev = HIGH, longFired = false;
const unsigned long DEBOUNCE = 30;
const unsigned long LONG_MS  = 1500; 

void setAll(uint8_t s){ for (uint8_t i=0;i<3;i++) digitalWrite(LEDS[i], s); }

const char* nameOf(uint8_t m){
  switch (m){
    case MODE_FAST: return "FAST";
    case MODE_SLOW: return "SLOW";
    case MODE_ALL:  return "ALL";
    default:        return "OFF";
  }
}

void switchToMode(uint8_t m){
  current = m; lastStep = millis(); idx = 0;
  if (current == MODE_OFF) setAll(LOW);
  Serial.print("Mode => "); Serial.println(nameOf(current));
}

void handleButton(){
  unsigned long now = millis();
  bool raw = digitalRead(BTN);        // INPUT_PULLUP: не нажато=HIGH, нажато=LOW

  // антидребезг
  if (raw != btnStable && (now - btnLast) > DEBOUNCE){
    btnPrev = btnStable; btnStable = raw; btnLast = now;

    if (btnPrev == HIGH && btnStable == LOW){    // press
      pressStart = now; longFired = false;
      Serial.println("PRESS");
    }
    if (btnPrev == LOW && btnStable == HIGH){    // release
      Serial.println("RELEASE");
      if (!longFired){
        if      (current == MODE_FAST) switchToMode(MODE_SLOW);
        else if (current == MODE_SLOW) switchToMode(MODE_ALL);
        else if (current == MODE_ALL)  switchToMode(MODE_FAST);
        else if (current == MODE_OFF)  switchToMode(MODE_FAST);
      }
    }
  }

  // длинное удержание
  if (btnStable == LOW && !longFired && (now - pressStart) >= LONG_MS){
    longFired = true;
    switchToMode(MODE_OFF);
    Serial.println("LONG >= 1.5s -> OFF");
  }
}

void doBlink(){
  if (current == MODE_OFF) return;

  unsigned long now = millis();
  if (current == MODE_FAST || current == MODE_SLOW){
    unsigned long step = (current == MODE_FAST) ? FAST_STEP : SLOW_STEP;
    if (now - lastStep >= step){
      lastStep = now;
      setAll(LOW);
      digitalWrite(LEDS[idx], HIGH);
      idx = (idx + 1) % 3;
    }
  } else if (current == MODE_ALL){
    if (now - lastStep >= ALL_STEP){
      lastStep = now;
      static bool on = false; on = !on;
      setAll(on ? HIGH : LOW);
    }
  }
}

void setup(){
  Serial.begin(9600);                  // 9600 — стабильнее для TinkerCad
  for (uint8_t i=0;i<3;i++){ pinMode(LEDS[i], OUTPUT); digitalWrite(LEDS[i], LOW); }
  pinMode(BTN, INPUT_PULLUP);          // кнопка к GND!
  switchToMode(current);

  setAll(HIGH); delay(100); setAll(LOW);
}

void loop(){
  handleButton();
  doBlink();
}
