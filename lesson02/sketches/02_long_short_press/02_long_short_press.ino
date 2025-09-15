const byte LEDS[3] = {2,3,4};
const byte BTN = 5;

enum Mode {OFF=0, ON=1, BLINK=2};
Mode mode = OFF;

const unsigned long DEBOUNCE=15, LONG_MS=3000, BLINK_STEP=250;
unsigned long btnLastChange=0, pressStart=0, lastBlink=0;
bool btnStable=HIGH, btnPrevStable=HIGH, longFired=false;

void setAll(byte s){ for(byte i=0;i<3;i++) digitalWrite(LEDS[i], s); }

void setup(){
  for(byte i=0;i<3;i++){ pinMode(LEDS[i],OUTPUT); digitalWrite(LEDS[i],LOW); }
  pinMode(BTN, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println("V2: short=toggle, long=blink all");
}

void loop(){
  unsigned long now=millis();
  // --- debounce ---
  bool raw=digitalRead(BTN);
  if(raw!=btnStable && (now-btnLastChange)>DEBOUNCE){
    btnPrevStable=btnStable; btnStable=raw; btnLastChange=now;

    if(btnPrevStable==HIGH && btnStable==LOW){ // press
      pressStart=now; longFired=false;
    }
    if(btnPrevStable==LOW && btnStable==HIGH){ // release
      if(!longFired){ // short
        mode = (mode==OFF)? ON : OFF;
        Serial.println(mode==ON? "ON" : "OFF");
      }
    }
  }
  // long press while holding
  if(btnStable==LOW && !longFired && (now-pressStart)>=LONG_MS){
    longFired=true; mode=BLINK; Serial.println("BLINK (long press)");
  }

  // --- outputs ---
  if(mode==OFF){ setAll(LOW); }
  else if(mode==ON){ setAll(HIGH); }
  else { // BLINK
    if(now-lastBlink>=BLINK_STEP){ lastBlink=now; static bool on=false; on=!on; setAll(on?HIGH:LOW); }
  }
}
