// V1: Serial events (start, button press/release)
int buttonPin = 5;
int ledPin = 13;
unsigned long t = 0;
const unsigned long DEBOUNCE = 20;
int stable = HIGH, prev = HIGH;
unsigned long last = 0;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  Serial.println("START");
}

void loop() {
  int raw = !digitalRead(buttonPin);
  unsigned long now = millis();
  if (raw != stable && (now - last) > DEBOUNCE) {
    prev = stable; stable = raw; last = now;
    if (prev == 0 && stable == 1) {
      Serial.println("Button pressed");
      digitalWrite(ledPin, !digitalRead(ledPin));
    }
    if (prev == 1 && stable == 0) {
      Serial.println("Button released");
    }
  }
  
}
