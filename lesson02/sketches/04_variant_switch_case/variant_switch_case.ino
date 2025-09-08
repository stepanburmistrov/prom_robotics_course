
/*
  Template variant using switch-case for modes and a struct for timers.
  Use as a starting point for your own experiments.
*/
#include <Arduino.h>
struct Led { uint8_t pin; unsigned long t; bool state; unsigned long interval; };
Led leds[] = {{2,0,false,150},{3,0,false,150},{4,0,false,150}};
enum Mode:uint8_t{FAST,SLOW,ALL,OFF}; Mode mode=FAST;
const uint8_t BTN=5; const unsigned long DEB=15; bool st=HIGH,prev=HIGH; unsigned long ch=0;
void setup(){ Serial.begin(115200); for(auto&l:leds) pinMode(l.pin,OUTPUT); pinMode(BTN,INPUT_PULLUP); }
void loop(){
  unsigned long now=millis();
  // btn
  bool r=digitalRead(BTN);
  if(r!=st && now-ch>DEB){prev=st; st=r; ch=now; if(prev==HIGH && st==LOW){ mode=(Mode)((mode+1)%3); Serial.print("mode="); Serial.println(mode);} }
  // blink
  switch(mode){
    case FAST: for(uint8_t i=0;i<3;i++){ if(now-leds[i].t>(i==0?100:100)) { leds[i].t=now; leds[i].state=(i==((now/100)%3)); digitalWrite(leds[i].pin, leds[i].state); } } break;
    case SLOW: for(uint8_t i=0;i<3;i++){ if(now-leds[i].t>(i==0?400:400)) { leds[i].t=now; leds[i].state=(i==((now/400)%3)); digitalWrite(leds[i].pin, leds[i].state); } } break;
    case ALL:  for(uint8_t i=0;i<3;i++){ if(now-leds[i].t>250){ leds[i].t=now; leds[i].state=!leds[i].state; digitalWrite(leds[i].pin, leds[i].state);} } break;
    case OFF:  for(auto&l:leds){ digitalWrite(l.pin,LOW);} break;
  }
}
