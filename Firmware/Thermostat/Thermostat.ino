/* 
 2022 Antonio de Vincentiis
 */

#include "SevSeg.h"
#include <SimpleRotary.h>
#include "max6675.h"
#include <SPI.h>
#include <Wire.h>

// DISPLAY
SevSeg sevseg; //Instantiate a seven segment controller object
int type_moved = 0; // 0= no move, 1= move time, 2= move temp
int pot_mov_temp_h = 1;  // hysteresis temperature potentiometer move
int last_temp_set = 0;  // last temperature potentiometer set
int decisec_view = 30; // 2 sec
int timer_view = 0;

// TEMPERATURE POT
int temperaturePot = A6;
int tempset = 0;
int hysteresis = 2;  // hysteresis

// THERMOCOUPLE
int thermoCS = A0;
int thermoCLK = A1;
int thermoDO = A2;
int sensorValue = 0;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
int tempread = 0;

// ROTARY ENCODE
// Pin A, Pin B, Button Pin
SimpleRotary rotary(A3,A4,A5);
int minutes = 0;
int mode = 0; // 0= top + bottom yesfan, 1= bottom yesfan, 2= top yesfan, 3= top + bottom nofan, 4= bottom nofan, 5= top nofan;
int decisec = 0;
int second = 0;

// RELAIS
int RESUP = 1;
int RESDW = 5;
int FAN = 0;

void setup() {
  byte numDigits = 3;
  byte digitPins[] = {2, 3, 4};
  byte segmentPins[] = {6, 7, 8, 9, 10, 11, 12, 13};
  bool resistorsOnSegments = false; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_ANODE; // See README.md for options
  bool updateWithDelays = false; // Default 'false' is Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = false; // Use 'true' if your decimal point doesn't exist or isn't connected

// ROTARY ENCODE
  // Set the trigger to be either a HIGH or LOW pin (Default: HIGH)
  // Note this sets all three pins to use the same state.
  rotary.setTrigger(HIGH);
  // Set the debounce delay in ms  (Default: 2)
  rotary.setDebounceDelay(1);
  // Set the error correction delay in ms  (Default: 200)
  rotary.setErrorDelay(200);

// DISPLAY
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
  updateWithDelays, leadingZeros, disableDecPoint);
  sevseg.setBrightness(90);
  sevseg.setNumber(minutes, 0);
  pinMode(RESUP, OUTPUT);
  pinMode(RESDW, OUTPUT);
  digitalWrite(RESUP, LOW);    
  digitalWrite(RESDW, LOW);    
}

void loop() {
  static unsigned long timer = millis();
  sensorValue = analogRead(temperaturePot);
  if (millis() - timer >= 100) {
    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
    tempset = sensorValue * (290.0 / 1023.0) + 40;
    if (tempset - last_temp_set > pot_mov_temp_h || 
        last_temp_set - tempset > pot_mov_temp_h ){ // trigger move temperature set
        timer_view = 0;
        type_moved = 2;
    }
    second++; // 100 milliSeconds is equal to 0,1 Second
    if (second >= 10) { //  trigger 1 second
      tempread = int(thermocouple.readCelsius());
      second=0;
    }
    if (type_moved >= 1){
      timer_view++;
      if (timer_view >= decisec_view) { //  trigger view move
       type_moved = 0;
       timer_view = 0;
      } 
      if (type_moved == 1){
        sevseg.setNumber(minutes, 0);
      } else if (type_moved == 2){
        sevseg.setNumber(tempset, 0);
      }
    } else {
      sevseg.setNumber(tempread, 0);
    }
    decisec++; // 100 milliSeconds is equal to 0,1 Second
    if (decisec >= 600 && minutes > 0) { // Reset to 0 after counting for 1000 seconds.
      decisec=0;
      minutes--;
    }
    if (tempset - hysteresis > tempread && minutes > 0 ){ 
      switch (mode) { // 0= top + bottom yesfan, 1= bottom yesfan, 2= top yesfan, 3= top + bottom nofan, 4= bottom nofan, 5= top nofan;
        case 0:
          digitalWrite(RESDW, HIGH);
          digitalWrite(RESUP, HIGH);
          digitalWrite(FAN, HIGH);
          break;
        case 1:
          digitalWrite(RESDW, HIGH);
          digitalWrite(RESUP, LOW);
          digitalWrite(FAN, HIGH);
          break;
        case 2:
          digitalWrite(RESDW, LOW);
          digitalWrite(RESUP, HIGH);
          digitalWrite(FAN, HIGH);
          break;
        case 3:
          digitalWrite(RESDW, HIGH);
          digitalWrite(RESUP, HIGH);
          digitalWrite(FAN, LOW);
          break;
        case 4:
          digitalWrite(RESDW, HIGH);
          digitalWrite(RESUP, LOW);
          digitalWrite(FAN, LOW);
          break;
        case 5:
          digitalWrite(RESDW, LOW);
          digitalWrite(RESUP, HIGH);
          digitalWrite(FAN, LOW);
          break;
        default:
          digitalWrite(RESDW, LOW);
          digitalWrite(RESUP, LOW);
          digitalWrite(FAN, LOW);
          break;
      }      
    } else if ( tempset + hysteresis < tempread || minutes == 0) { 
      digitalWrite(RESDW, LOW);
      digitalWrite(RESUP, LOW);
      if (minutes == 0) {
        digitalWrite(FAN, LOW);
      }
    }
    last_temp_set = tempset; 
    timer = millis();
  }
  sevseg.refreshDisplay(); // Must run repeatedly
  // ROTARY ENCODE
  byte i;
  i = rotary.rotate();
  if ( i == 2 && minutes < 120) {
    minutes++;
    timer_view = 0;
    type_moved = 1;
  } else if ( i == 1 && minutes > 0) {
    minutes--;
    timer_view = 0;
    type_moved = 1;
  }
  byte l;
  // Check to see if button is pressed for 1 second.
  l = rotary.pushLong(800);
  if ( l == 1 ) {
    mode++;
    if ( mode >= 6 ) {
      mode = 0;
    }
  }

}

/// END ///
