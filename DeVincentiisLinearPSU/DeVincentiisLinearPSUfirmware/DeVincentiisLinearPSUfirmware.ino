/*
Digital Linear Power Supply by ADVisor Antonio De Vincentiis - Pescara - Italy 
https://github.com/devincentiis
version 20250716
*/
#include "avr/io.h"
#include "util/delay.h"
#include "avr/interrupt.h"
#include "SimpleRotary.h"
#include "Wire.h"
#include "LiquidCrystal.h"
#include "EEPROM.h"

#define ROT_A PIN_PA5                   // (PA5) pin 3  from Rotary switch A pin
#define ROT_B PIN_PA7                   // (PA7) pin 5  from Rotary switch B pin
#define ROT_S PIN_PA0                   // (PA0) pin 16 from Rotary Button pin
#define CAP_DISCHARGE PIN_PC2           // (PC2) pin 14 to transistor for pulldown of the smoothing capacitors at bootstrap
#define V_READ PIN_PA1                  // (PA1) pin 17 from output voltage resistor net
#define DETECT_TRANSFO_VOLTAGE PIN_PA2  // (PA2) pin 18 from smoothing capacitors resistor net voltage detect
#define DETECT_SHUTDOWN PIN_PA3         // (PA3) pin 19 from transistor for detect when line -3,1V flotting (at shutdown)
#define A_READ PIN_PA4                  // (PA4) pin 2  from output current detector opamp 
#define A_SET PIN_PA6                   // (PA6) pin 4  to set max current 
#define RELAY12_24 PIN_PC1              // (PC1) pin 13 to relay driver transistor

unsigned long timer_decisec = millis();
int current_decisec = 0;

// ROTARY ENCODE
SimpleRotary rotary(ROT_A, ROT_B, ROT_S);


uint16_t rnetTransfoVoltage;
uint16_t max_voltage = 240;
uint16_t max_current = 150; // max current is 250 (250*20mA) = 5A 
uint16_t middle_voltage = 10;

uint16_t voltage_set = 0;
uint16_t rnetOutVoltage;
uint16_t printRealVoltage;
uint16_t current_set = 0;
uint16_t opampCurrent = 1023; 
uint16_t biasCurrent = 0;
uint16_t printRealCurrent;
int voltampere_set = 0;
int mode_set = 0; // 0 = set voltage, 1 = set current;

int shutdown_detect = 0; // 0 = exec shutdown, 1 = no shutdown
int shutdown_status = 0; // 0 = first time, 1 = netx time;

// EXTERNAL DAC
// For MCP4725A0 the address is 0x60 (default) or 0x61 (ADDR pin tied to VCC)
// For MCP4725A1 the address is 0x62 (default) or 0x63 (ADDR pin tied to VCC)
// For MCP4725A2 the address is 0x64 (default) or 0x65 (ADDR pin tied to VCC)
#define MCP4725_ADDR 0x60

unsigned long currentMillis;

// ADC/4 with 5 volt ref = 5/256 = 0.0195V / uint8
// with resistor partition +V|--- R1(33000) ---|--- R2(4700) ---| GND (K=0.12668) if Drop Voltage = 5 Volt --> 5*0.0195 = 0.6334V --> 0.6334*0.0195 = 32 uint8_t

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 10 , en =13  , d4 = 7, d5 = 6, d6 = 5, d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
byte charRight[8] = {
  B10000,
  B11000,
  B11100,
  B11110,
  B11100,
  B11000,
  B10000,
  B00000
};

// START DEFINE FUNCTION
void write12BitI2C(int x) {
  // Write a 12-bit integer out to I2C
  Wire.beginTransmission(MCP4725_ADDR);
  Wire.write(64);            // cmd to update the DAC
  Wire.write(x >> 4);        // the 8 most significant bits...
  Wire.write((x % 16) << 4); // the 4 least significant bits...
  Wire.endTransmission();
}
void writeIntIntoEEPROM(int address, int number)
{ 
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}
int readIntFromEEPROM(int address)
{
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}
void wait_cap_discharge(int halfsec) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" MIDDLE VOLTAGE");
  for (int i = 0; i <= halfsec; i++) {
    // read transfo middle voltage
    rnetTransfoVoltage = analogRead(DETECT_TRANSFO_VOLTAGE);
    middle_voltage = map(rnetTransfoVoltage, 0, 1023, 0, 440 );
    lcd.setCursor(0, 1);
    lcd.print((float)middle_voltage/10,1);
    lcd.print("V reading....");
    delay(500);
  }
}
// END DEFINE FUNCTION

void setup() {
  // EXTERNAL DAC
  Wire.begin();
  
  // INTERNAL DAC
  DACReference(INTERNAL4V3);
  analogReference(INTERNAL4V3); // ADC read real voltage and current
  pinMode(A_SET,OUTPUT); // DAC output to PA6

  //OTHER
  pinMode(RELAY12_24,OUTPUT); 
  pinMode(CAP_DISCHARGE,OUTPUT); 
  digitalWrite(RELAY12_24, HIGH); // startup at 12Vac 

  // DETECT TRANSFORMER VOLTAGE 
  pinMode(DETECT_TRANSFO_VOLTAGE,INPUT);

  // DETECT SHOTDOWN
  pinMode(DETECT_SHUTDOWN,INPUT_PULLUP);

  // ROTARY ENCODE
  pinMode(ROT_A,INPUT_PULLUP);
  pinMode(ROT_B,INPUT_PULLUP);
  pinMode(ROT_S,INPUT_PULLUP);

  // setoutput to zero
  write12BitI2C(0);
  analogWrite(A_SET,1);
  
  // Set the trigger to be either a HIGH or LOW pin (Default: HIGH)
  // Note this sets all three pins to use the same state.
  rotary.setTrigger(HIGH);
  // Set the debounce delay in ms  (Default: 2)
  rotary.setDebounceDelay(1);
  // Set the error correction delay in ms  (Default: 200)
  rotary.setErrorDelay(200);
  // Output to zero for measure transformer voltage
  lcd.begin(16, 2);
  lcd.createChar(0, charRight);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("    Digital");
  lcd.setCursor(0, 1);
  lcd.print("   Linear PS");
  delay(1000);  
  // first check if exist middle tap of transformer
  digitalWrite(CAP_DISCHARGE,HIGH); // start discharge smoothing capacitors
  wait_cap_discharge(10);
  digitalWrite(CAP_DISCHARGE,LOW); // stop discharge smoothing capacitors
  delay(2000);
  // read transfo max voltage 
  digitalWrite(RELAY12_24, LOW); 
  delay(2000);
  rnetTransfoVoltage = analogRead(DETECT_TRANSFO_VOLTAGE);
  max_voltage = map(rnetTransfoVoltage, 0, 1023, 0, 440 );
  if (middle_voltage > 220){ // dual transformer middle voltage > 22V
    lcd.clear();
    lcd.setCursor(0, 0);
    if ((max_voltage - middle_voltage) < 50) { // relay failure ex. contacts sticky
      lcd.print("Voltage min=max");
      lcd.setCursor(0, 1);
      lcd.print("RELAY STICKING?");
    } else { // over voltage on middle tap
      lcd.print("!OVER VOLTAGE!");
      lcd.setCursor(0, 1);
      lcd.print("middle V = ");
      lcd.print((float)middle_voltage/10,1);
      while(1);
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  if (max_voltage < 180){
    lcd.print(" !LOW VOLTAGE!");
    lcd.setCursor(0, 1);
    lcd.print((float)max_voltage/10,1);
    lcd.print("V ");
  } else if (max_voltage > 430){ // transformer over voltage 
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!OVER VOLTAGE!");
    lcd.setCursor(0, 1);
    lcd.print("full V = ");
    lcd.print((float)max_voltage/10,1);
    digitalWrite(RELAY12_24, HIGH); // unlink transformer 
    while(1);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Transformer OK!");
    lcd.setCursor(0, 1);
    if (middle_voltage > 50){
      lcd.print("DUAL ");
      lcd.print((float)max_voltage/10,1);
      lcd.print("-");
      lcd.print((float)middle_voltage/10,1);
      lcd.print("V");
    } else {
      lcd.print("SINGLE ");
      lcd.print((float)max_voltage/10,1);
      lcd.print(" Volt");
      middle_voltage = 0; // not switch relay
    }
  }
  delay(3000);
  // read current offset
  biasCurrent = analogRead(A_READ);
  if (biasCurrent >= 2){
    biasCurrent--;
  }
/*  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bias current ");
  lcd.print((float)biasCurrent/100,2);
  delay(2000);
*/
  // drop 1.0V from transformer max_voltage (increase stability)
  max_voltage = max_voltage*0.9;
  if (max_voltage > 348){
    max_voltage = 348;
  }
  voltampere_set = readIntFromEEPROM(3) ;
  if (voltampere_set > 500){
    voltampere_set = 500;
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Power ");
  lcd.print(voltampere_set);
  lcd.print("VA");
  lcd.setCursor(0, 1);
  lcd.print("Max current ");
  max_current = voltampere_set*500/max_voltage;
  if (max_current > 500){
    max_current=500;
  }
  lcd.print((float)max_current/50,1);
  lcd.print("A");
  delay(1000);
  lcd.clear();
  voltage_set = readIntFromEEPROM(5);
  if (voltage_set < 0 || voltage_set > 500) { // out of range
    voltage_set = 0; // 0.00V
  }  
  write12BitI2C(voltage_set*11.375);
  current_set = readIntFromEEPROM(1);
  if (current_set < 0 || current_set > 1000) { // out of range
    current_set = 5; // 5 = 100 mA
  }  
  analogWrite(A_SET,current_set);
  byte p;
  // Check to see if button is pressed for 5 seconds
  p = rotary.pushLong(500);
  if ( p == 1 || voltampere_set < 5 || voltampere_set > 250 ) {
    mode_set = 2; // goto menu set VA
  }

}

void loop() {
  currentMillis = millis();
  if (currentMillis - timer_decisec >= 100) { // 0,1 seconds
    current_decisec++;
    if (current_decisec >= 10) { // every one second
      lcd.clear();
      current_decisec=0;
    }
    // check shotdown status
    shutdown_detect = digitalRead(DETECT_SHUTDOWN);
    // read real voltage
    rnetOutVoltage = analogRead(V_READ);
    // read real current
    opampCurrent = analogRead(A_READ);
    // map it to the range of the analog out:
    printRealVoltage = map(rnetOutVoltage, 0, 1023, 0, 348);
    if (printRealVoltage >= middle_voltage || middle_voltage < 50){
      digitalWrite(RELAY12_24, LOW);   
    } else {
      if (shutdown_detect >=1 && shutdown_status == 0){ // relay energized only if no shutdown
        digitalWrite(RELAY12_24, HIGH);
      }   
    }
    if (shutdown_detect >=1){ // ok, no shutdown
      if ( mode_set == 2 ) { // view for setting VA
        lcd.setCursor(0, 0);
        lcd.print("Set transformer");
        lcd.setCursor(0, 1);
        lcd.print(" power = ");
        lcd.print(voltampere_set);
        lcd.print(" VA   ");
      } else { // view normal
        lcd.setCursor(0, 0);
        lcd.print("Out: ");
        lcd.print((float)printRealVoltage/10,1);
        lcd.print("V ");
        lcd.setCursor(10, 0);
        lcd.print(" ");
        if (opampCurrent < biasCurrent){
          opampCurrent = biasCurrent;
        }
        printRealCurrent = map(opampCurrent-biasCurrent, 0, 1023, 0, 511); // ex:biasCurrent = 5 -> about 20 mA offset current necessary at all devices powered with 5V downstream of the shunt resistor (microcontroller, display and external DAC)
        lcd.print((float)printRealCurrent/100,2);
        lcd.print("A ");
        lcd.setCursor(0, 1);
        lcd.print("Set:");
        lcd.setCursor(4, 1);
        if ( mode_set == 0 ) {
          lcd.write((byte)0);
        } else {
          lcd.print(" ");
        }
        lcd.print((float)voltage_set/10,1);
        lcd.print("V ");
        lcd.setCursor(10, 1);
        if ( mode_set == 1 ) {
          lcd.write((byte)0);
        } else {
          lcd.print(" ");
        }
        lcd.print((float)current_set/50,2);
        lcd.print("A ");
      }        
      timer_decisec = currentMillis;
    } else { // shutdown
      if (shutdown_status==0){
        digitalWrite(RELAY12_24, LOW);
        writeIntIntoEEPROM(5, voltage_set);      
        writeIntIntoEEPROM(1, current_set);      
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("    Goodbye");
        lcd.setCursor(0, 1);
        lcd.print("  enthusiast!");
        shutdown_status = 1;
      }
    }
  } 

  // ROTARY ENCODE
  byte i;
  i = rotary.rotate();
  if (mode_set==0){ // set voltage (default at startup)
    if ( i == 2 && voltage_set < max_voltage) {
      voltage_set++;
      write12BitI2C(voltage_set*11.375);
    } else if ( i == 1 && voltage_set > 0) {
      voltage_set--;
      write12BitI2C(voltage_set*11.375);
    }
  } else if (mode_set==1) { // set current limit
    if ( i == 2 && current_set < max_current) {
      current_set++;
      analogWrite(A_SET,current_set);
    } else if ( i == 1 && current_set > 1) {
      current_set--;
      analogWrite(A_SET,current_set);
    }
  } else if (mode_set==2) { // set transformer VA ( after push 5 seconds at startup)
    if ( i == 2 && voltampere_set < 500) {
      voltampere_set = ((voltampere_set/5 + (voltampere_set%5>2)) * 5) + 5;
      writeIntIntoEEPROM(3, voltampere_set);      
    } else if ( i == 1 && voltampere_set > 5) {
      voltampere_set = ((voltampere_set/5 + (voltampere_set%5>2)) * 5) - 5;
      writeIntIntoEEPROM(3, voltampere_set);      
    }
  }
  byte l;
  // Check to see if button is pressed for 1 second
  l = rotary.pushLong(100);
  if ( l == 1 ) {
    if ( mode_set == 2 ){ // update max_current without reboot
      max_current = voltampere_set*500/max_voltage;
      if (max_current > 500){
        max_current=500;
        writeIntIntoEEPROM(1, current_set);      
      }
      if (current_set > max_current){
        current_set = max_current;
      }
    }
    mode_set++;
    if ( mode_set >= 2 ) {
      mode_set = 0;
    }
  }
  
  
}
