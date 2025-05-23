/*
PRESSFLOWino by ADVisor Antonio De Vincentiis - Pescara - Italy 
https://github.com/devincentiis
firmware for PRESSFLOWino schematic version 2.0
*/

// include the library code:
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <ADVcapacitiveSensor.h>

ADVcapacitiveSensor touchbtnUP;
bool prevBtnUP = LOW;
bool btnStateUP = LOW;
ADVcapacitiveSensor touchbtnDWN;
bool prevBtnDWN = LOW;
bool btnStateDWN = LOW;
ADVcapacitiveSensor touchbtnENT;
bool prevBtnENT = LOW;
bool btnStateENT = LOW;
int btnENT_pressed_counter;
#define LONGPRESS_LEN    4  // Min nr of loops for a long press
ADVcapacitiveSensor touchbtnBCK;
bool prevBtnBCK = LOW;
bool btnStateBCK = LOW;

// touch buttons
#define BTN_UP A0  
#define BTN_DWN A1
#define BTN_ENT A7
#define BTN_BACK A6

#define BTNR_UP 0
#define BTNR_DWN A5
#define BTNR_ENT 1
#define BTNR_BACK 3 

// output devices
#define BUZZER A2
#define BACKLIGHT 6
bool lightStatus = true;

#define MOTOR 5
bool motor_status = false;

#define EVALVE 4
bool valveStatus = false;

int MAXPRESS = 80;
const int MINPRESS = 5;

#define ALARM_TIME          40  // alarm time (seconds)
#define ATTEMPTS            10  // attempts before alarm
#define TIME_FLUSH_CONTROL  25   // flow sensor: max seconds without flush
int flush_control_counter;
#define MOTOR_START_DELAY   4   // 4 sec delay (ex. solenoid valve)

#define FLOWSENS 2 
byte flowsensorInterrupt = 0;  

const int zeroCalibration = 140;
int pressValue = 0;
int pressRead     = 0;  // value coming from the sensor
#define PRESSENS A3     // input pin for the press sensor

volatile byte pulseCount;
float flowRate;
// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float pulseFactor = 1.00;

float pressFactor = 1.30;

// Compressor Mode: control variables
int attempts_rest = ATTEMPTS;         // rest of attempts before alarm
int start_delay = MOTOR_START_DELAY;  // delay electrovalve closing (seconds)
volatile uint16_t max_on_time = 900;  // max time motor run admitted (900=15min)
volatile uint16_t operating_time = 0; // operating time (seconds)

char buttonPressed = ' ';
byte menuLevel = 0;     // Level 0: display actual values: pressure, flux, pump status
                        // Level 1: display main menu
byte option = 1;

int minPress_val = MINPRESS;
int maxPress_val = MAXPRESS;
int pressSensorType_val = 0; // 0=Custom, 1=500Kpa, 2=700Kpa, 3=800Kpa, 4=1Mpa, 5=1.2Mpa, 6=1.5Mpa, 7=2Mpa, 8=3Mpa, 9=4Mpa 
float pf0 = 1.10;  // 0: Custom
float pf1 = 0.69;  // 1: 500Kpa
float pf2 = 0.96;  // 2: 700Kpa
float pf3 = 1.10;  // 3: 800Kpa
float pf4 = 1.37;  // 4: 1Mpa
float pf5 = 1.65;  // 5: 1.2Mpa
float pf6 = 2.06;  // 6: 1.5Mpa
float pf7 = 2.75;  // 7: 2Mpa
float pf8 = 4.13;  // 8: 3Mpa
float pf9 = 5.50;  // 9: 4Mpa
float pressFactorData[10] = {pf0,pf1,pf2,pf3,pf4,pf5,pf6,pf7,pf8,pf9};

int mp0 = 80;  // 0: Custom
int mp1 = 50;  // 1: 500Kpa
int mp2 = 70;  // 2: 700Kpa
int mp3 = 80;  // 3: 800Kpa
int mp4 = 100;  // 4: 1Mpa
int mp5 = 120;  // 5: 1.2Mpa
int mp6 = 150;  // 6: 1.5Mpa
int mp7 = 200;  // 7: 2Mpa
int mp8 = 300;  // 8: 3Mpa
int mp9 = 400;  // 9: 4Mpa
int maxPressData[10] = {mp0,mp1,mp2,mp3,mp4,mp5,mp6,mp7,mp8,mp9};

int fluxSensorType_val = 0;  // 0=none, 1=switch, 2=YFG1, 3=YFDN32, 4=YFDN40 , 5=YFDN50 , 6=FS300A, 7=YFS401, 8=YFS201, 9=custom
float ff0 = 0.00;  // 0: none    
float ff1 = 0.00;  // 1: switch
float ff2 = 9.72;  // 2: YFG1
float ff3 = 23.33;  // 3: YFDN32
float ff4 = 23.33;  // 4: YFDN40
float ff5 = 52.50;  // 5: YFDN50  
float ff6 = 1.75;  // 6: FS300A
float ff7 = 0.11;  // 7: YFS401
float ff8 = 1.40;  // 8: YFS201
float flowFactorData[9] = {ff0,ff1,ff2,ff3,ff4,ff5,ff6,ff7,ff8};

int ctrlMode_val = 0;
float print_val = 0.0;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int LCDRS = 12 , LCDEN =11  , LCDD4 = 10, LCDD5 = 9, LCDD6 = 8, LCDD7 = 7;
LiquidCrystal lcd(LCDRS, LCDEN, LCDD4, LCDD5, LCDD6, LCDD7);



unsigned long currentMillis;
unsigned long startOneSecMillis;
const unsigned long OneSec = 1000; // 1000 = 1 sec
unsigned long startDeciSecMillis;
const unsigned long DeciSec = 250; // 1/4 sec

char ShowSpinn[5] = {" |/-"};
byte charUp[8] = {
  B00100,
  B01110,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};
byte charDown[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B01110,
  B00100
};
byte charUpDown[8] = {
  B00100,
  B01110,
  B11111,
  B00000,
  B00000,
  B11111,
  B01110,
  B00100
};

byte charBackSlash [8]= { 
  0x00, 
  0x10, 
  0x08, 
  0x04, 
  0x02, 
  0x01, 
  0x00, 
  0x00 
};

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

int displaySpinn = 1;

void setup() {
  // touch buttons:
  pinMode(BTN_UP,INPUT);
  pinMode(BTN_DWN,INPUT);
  pinMode(BTN_BACK,INPUT);
  pinMode(BTN_ENT,INPUT);
  pinMode(BTNR_UP,OUTPUT);
  pinMode(BTNR_DWN,OUTPUT);
  pinMode(BTNR_ENT,OUTPUT);
  pinMode(BTNR_BACK,OUTPUT);

  // outputs
  pinMode(MOTOR,OUTPUT);
  pinMode(EVALVE,OUTPUT);
  pinMode(BACKLIGHT,OUTPUT);
  pinMode(BUZZER,OUTPUT);
  
  minPress_val = EEPROM.read(0);
  if (minPress_val < MINPRESS || minPress_val > MAXPRESS ){ // minPress limit
    minPress_val = MINPRESS;
    EEPROM.write(0, minPress_val);
  }
  ctrlMode_val = EEPROM.read(2);
  if (ctrlMode_val > 1 ){ // mode (pupressSensorType_val=0, compressor=1)
    ctrlMode_val = 0;
    EEPROM.write(2, 0);
  }  
  fluxSensorType_val = EEPROM.read(3);
  if (fluxSensorType_val > 9 ){ 
    fluxSensorType_val = 0;
    EEPROM.write(3, 0);
  }  
  EEPROM.get(4, pulseFactor);
  if (pulseFactor < 0.01 ){ 
    pulseFactor = 1;
    EEPROM.put(4,pulseFactor);
  }
  pressSensorType_val = EEPROM.read(5);
  if (pressSensorType_val > 9 ){ 
    pressSensorType_val = 0;
    EEPROM.write(5, 0);
  }
  MAXPRESS = maxPressData[pressSensorType_val];
  maxPress_val = EEPROM.read(1);
  if (maxPress_val < MINPRESS || maxPress_val > MAXPRESS ){ // maxPress limit
    maxPress_val = MAXPRESS;
    EEPROM.write(1, maxPress_val);
  }
  EEPROM.get(6, pressFactor);
  if (pressFactor < 0.01 ){ 
    pressFactor = 1;
    EEPROM.put(6,pressFactor);
  }

  pulseCount = 0;  
  flowRate = 0.0;
  btnENT_pressed_counter = 0;
  flush_control_counter = 0;

  if (fluxSensorType_val <= 1 ) {  // switch or no flux sensor
    // The flow-switch sensor (ON-OFF) is connected to pin 2 which
    pinMode(FLOWSENS,INPUT_PULLUP);
  } else {
    // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
    // Configured to trigger on a FALLING state change (transition from HIGH
    // state to LOW state)
    pinMode(FLOWSENS,INPUT);
    attachInterrupt(flowsensorInterrupt, pulseCounter, FALLING);
  }
      
  // set up the LCD's number of columns and rows:
  digitalWrite(BACKLIGHT,HIGH);
  lcd.begin(16, 2);
  lcd.createChar(0, charBackSlash);
  lcd.createChar(1, charUp);
  lcd.createChar(2, charDown);
  lcd.createChar(3, charUpDown);
  lcd.createChar(4, charRight);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("U:");
  lcd.print(touchbtnUP.begin(BTNR_UP, BTN_UP, 5.0, 5, 10, 0.2));
  lcd.print(" D:");
  lcd.print(touchbtnDWN.begin(BTNR_DWN, BTN_DWN, 5.0, 5, 10, 0.2));
  lcd.setCursor(0, 1);
  lcd.print("E:");
  lcd.print(touchbtnENT.begin(BTNR_ENT, BTN_ENT, 5.0, 5, 10, 0.2));
  lcd.print(" B:");
  lcd.print(touchbtnBCK.begin(BTNR_BACK, BTN_BACK, 5.0, 5, 10, 0.2));
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
}

void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}

void processButton(char buttonPressed) {
  switch(menuLevel) {
    case 0:                     // menuLevel 0, home screen
      switch ( buttonPressed ) {
        case 'E':               // Enter
          option = 1;
          menuLevel = 1;        // go to main menu
          break;
        case 'U': // Up
          break;
        case 'D': // Down
          break;
        case 'B': // Back
          menuLevel = 0;        // go to home screen
          break;
        default:
          break;
      }
      break;
    // end menuLevel 0
   
    case 1:                     // menuLevel 1, main menu
      switch ( buttonPressed ) {
        case 'E':               // Enter
          menuLevel = 2;        // go to menu second level
          break;
        case 'U':               // Up (view inverse)
          if ( option > 1 ) {
            option--;
          }
          break;
        case 'D':               // Down (view inverse)
          if ( option < 5 ) {
            option++;
          }
          break;
        case 'B': // Back
          menuLevel = 0;        // hide menu, go back to level 0
          break;
        default:
          break;
      } 
      break;
     // end menuLevel 1
     
     case 2:                     // menuLevel 2
      switch ( buttonPressed ) {
        case 'E':               // Enter
          if ((option == 3 && fluxSensorType_val == 9) || (option == 5 && pressSensorType_val == 0)) { // goto menù level 3 only for sensors type customizeble (9)
            menuLevel = 3;        // go to menu third level
          } 
          break;
        case 'U':               // Up
          if (option == 1 && minPress_val < ( maxPress_val - 1 )) { // minPress
            minPress_val = minPress_val + 1;
            EEPROM.write(0, minPress_val);
          } else if (option == 2 && maxPress_val < ( MAXPRESS - 1 )){ // maxPress
            maxPress_val = maxPress_val + 1;
            EEPROM.write(1, maxPress_val);
          } else if (option == 3 && fluxSensorType_val < 9 ){ // fluxSensorType_val
            fluxSensorType_val++;
            EEPROM.write(3, fluxSensorType_val);
          } else if (option == 4 && ctrlMode_val < 1) { // mode (pump=0, compressor=1)
            ctrlMode_val++;
            EEPROM.write(2, ctrlMode_val);
          } else if (option == 5 && pressSensorType_val < 9) { // pressSensorType_val
            pressSensorType_val++;
            MAXPRESS = maxPressData[pressSensorType_val];
            EEPROM.write(5, pressSensorType_val);
          }
          break;
        case 'D':               // Down
          if (option == 1 && minPress_val > ( MINPRESS + 1 ) ) { // minPress
            minPress_val = minPress_val - 1;
            EEPROM.write(0, minPress_val);
          } else if (option == 2 && maxPress_val > ( minPress_val + 1 )){ // maxPress
            maxPress_val = maxPress_val - 1;
            EEPROM.write(1, maxPress_val);
          } else if (option == 3 && fluxSensorType_val >= 1){ // fluxSensorType_val
            fluxSensorType_val--;
            EEPROM.write(3, fluxSensorType_val);
          } else if (option == 4 && ctrlMode_val >= 1) { // mode (pump=0, compressor=1)
            ctrlMode_val=0;
            EEPROM.write(2, ctrlMode_val);
          } else if (option == 5 && pressSensorType_val >= 1) { // pressSensorType_val
            pressSensorType_val--;
            MAXPRESS = maxPressData[pressSensorType_val];
            EEPROM.write(5, pressSensorType_val);
          }
          break;
        case 'B':               // Back
          menuLevel = 1;        // go back to level 1
          break;
        default:
          break;
      } 
      break;
     // end menuLevel 2 
     
     case 3:                     // menuLevel 3
      switch ( buttonPressed ) {
        case 'U':               // Up
          if (option == 3 && fluxSensorType_val == 9 && pulseFactor < 100){ // Flux sensor type custom
            if (pulseFactor < 10 ){ 
              pulseFactor += 0.01;
            } else {
              pulseFactor += 0.1;
            }
            EEPROM.put(4, pulseFactor);
          }
          if (option == 5 && pressSensorType_val == 0 && pressFactor < 10 ){ // Press sensor type custom
            pressFactor += 0.01;
            EEPROM.put(6, pressFactor);
          }        
          break;
        case 'D':               // Down
          if (option == 3 && fluxSensorType_val == 9 && pulseFactor > 0.01){ // Flux Sensor type custom
            if (pulseFactor < 10){
              pulseFactor -= 0.01;
            } else {
              pulseFactor -= 0.1;
            }
            EEPROM.put(4, pulseFactor);
          }        
          if (option == 5 && pressSensorType_val == 0 && pressFactor > 0.01){ // Press sensor type custom
            pressFactor -= 0.01;
            EEPROM.put(6, pressFactor);
          }        
          break;
        case 'B':               // Back
          menuLevel = 2;        // go back to level 2
          break;
        default:
          break;
      } 
      break;      
    // end menuLevel 3
    default:
      break;
  }
}

void viewHome() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (ctrlMode_val == 1){ 
  // MODE COMPRESSOR
    if (fluxSensorType_val >= 1 ) {  // switch or hall effect flux sensor
      if (fluxSensorType_val == 1 ) {  // switch ON-OFF
        if (pulseCount==0){
          if (motor_status) { 
            lcd.print("Flow: YES  ");
          } else {
            lcd.print("Refill oil!");
          }
        } else {
          lcd.print("Flow: NO   ");
        }
        if (operating_time > 0){
          lcd.print(operating_time);
        }
      } else { // hall effect sensor
        lcd.print("Flow: ");
        lcd.print(flowRate,1);
        lcd.print(" L/m    ");
      }
    } else { // no flow sensor
      if (operating_time > 0){
        lcd.print("ON for ");
        lcd.print(operating_time);
        lcd.print("s   ");
      } else {
        lcd.print("  PRESSFLOWino ");
      }
    }
  } else {
  // MODE PUMP
    if (fluxSensorType_val >= 1 ) {  // switch or hall effect flux sensor
      if (fluxSensorType_val == 1 ) {  // switch ON-OFF
        if (pulseCount==0){
        } else {
          lcd.print("Flow: NO   ");
        }
        if (operating_time > 0){
          lcd.print(operating_time);
        }
      } else { // hall effect sensor
        lcd.print("Flow: ");
        lcd.print(flowRate,1);
        lcd.print(" L/m    ");
      }
    } else { // no flow sensor
      if (operating_time > 0){
        lcd.print("ON for ");
        lcd.print(operating_time);
        lcd.print("s   ");
      } else {
        lcd.print("  PRESSFLOWino ");
      }
    }
  }
  lcd.setCursor(0, 1);
  lcd.print("Press: ");
  print_val = pressValue;
  lcd.print((print_val/10),1);
  lcd.print(" bar    ");
}

void viewLevel_1 () { 
  switch (option) {
    case 1:
      lcd.clear();
      // first row
      lcd.print(">Min press:");
      print_val = minPress_val;
      lcd.print((print_val/10),1);
      // second row
      lcd.setCursor(0, 1);
      lcd.print(" Max press:");
      print_val = maxPress_val;
      lcd.print((print_val/10),1);
      lcd.setCursor(15,1);
      lcd.write((byte)2);     // down arrow
      break;
    case 2:
      lcd.clear();
      // first row
      lcd.print(" Min press:");
      print_val = minPress_val;
      lcd.print((print_val/10),1);
      // second row
      lcd.setCursor(0, 1);
      lcd.print(">Max press:");
      print_val = maxPress_val;
      lcd.print((print_val/10),1);
      lcd.setCursor(15,1);
      lcd.write((byte)3);     // up and down arrow
      break;
    case 3:
      lcd.clear();
      // first row
      lcd.print(" Max press:");
      print_val = maxPress_val;
      lcd.print((print_val/10),1);
      // second row
      lcd.setCursor(0, 1);
      lcd.print(">Fsen: ");
      if (fluxSensorType_val==1){
        lcd.print("Switch  ");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up & down arrow
      } else if (fluxSensorType_val==2){
        lcd.print("YF-B1   ");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up & down arrow
      } else if (fluxSensorType_val==3){
        lcd.print("YFDN32  ");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up & down arrow
      } else if (fluxSensorType_val==4){
        lcd.print("YFDN40  ");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up & down arrow
      } else if (fluxSensorType_val==5){
        lcd.print("YFDN50  ");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up & down arrow
      } else if (fluxSensorType_val==6){
        lcd.print("FS300A  ");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up & down arrow
      } else if (fluxSensorType_val==7){
        lcd.print("YF-S401 ");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up & down arrow
      } else if (fluxSensorType_val==8){
        lcd.print("YF-S201 ");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up & down arrow
      } else if (fluxSensorType_val==9){
        lcd.print("Custom  ");
        lcd.setCursor(15,1);
        lcd.write((byte)2);     // down arrow
      } else { // none
        lcd.print("None    ");
        lcd.setCursor(15,1);
        lcd.write((byte)1);     // up arrow
      }
      break;
    case 4:
      lcd.clear();
      lcd.print(" Fsen: ");
      if (fluxSensorType_val==1){
        lcd.print("Switch  ");
      } else if (fluxSensorType_val==2){
        lcd.print("YF-B1   ");
      } else if (fluxSensorType_val==3){
        lcd.print("YFDN32  ");
      } else if (fluxSensorType_val==4){
        lcd.print("YFDN40  ");
      } else if (fluxSensorType_val==5){
        lcd.print("YFDN50   ");
      } else if (fluxSensorType_val==6){
        lcd.print("FS300A ");
      } else if (fluxSensorType_val==7){
        lcd.print("YF-S401 ");
      } else if (fluxSensorType_val==8){
        lcd.print("YF-S201 ");
      } else if (fluxSensorType_val==9){
        lcd.print("Custom  ");
      } else { // none
        lcd.print("None   ");
      }
      // second row
      lcd.setCursor(0, 1);
      if (ctrlMode_val==1){
        lcd.print(">Mode compress ");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      } else {
        lcd.print(">Mode pump     ");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      }
      break;
    case 5:
      lcd.clear();
      if (ctrlMode_val==1){
        lcd.print(" Mode compress ");
        lcd.setCursor(15,1);
        lcd.write((byte)2);     // down arrow
      } else {
        lcd.print(" Mode pump     ");
        lcd.setCursor(15,1);
        lcd.write((byte)1);     // up arrow
      }
      // second row
      lcd.setCursor(0, 1);
      lcd.print(">Psen: ");
      // 0=Custom, 1=500Kpa, 2=700Kpa, 3=800Kpa, 4=1Mpa, 5=1.2Mpa, 6=1.5Mpa, 7=2Mpa, 8=3Mpa, 9=4Mpa 
      if (pressSensorType_val==1){
        lcd.print("500Kpa  ");
      } else if (pressSensorType_val==2){
        lcd.print("700Kpa  ");
      } else if (pressSensorType_val==3){
        lcd.print("800Kpa  ");
      } else if (pressSensorType_val==4){
        lcd.print("1Mpa    ");
      } else if (pressSensorType_val==5){
        lcd.print("1.2Mpa  ");
      } else if (pressSensorType_val==6){
        lcd.print("1.5Mpa  ");
      } else if (pressSensorType_val==7){
        lcd.print("2Mpa    ");
      } else if (pressSensorType_val==8){
        lcd.print("3Mpa    ");
      } else if (pressSensorType_val==9){
        lcd.print("4Mpa    ");
      } else { // Custom
        lcd.print("Custom  ");
      }
      break;
  }
}

void viewLevel_2 () {
  switch (option) {
    case 0:
      break;
    case 1:                                 // Min press
      lcd.clear();
      lcd.print(" Min press:");
      // second row
      lcd.setCursor(0, 1);
      lcd.print("  ");
      print_val = minPress_val;
      lcd.print((print_val/10),1);
      lcd.print("  bar");
      lcd.setCursor(15,1);
      if ( minPress_val < ( maxPress_val - 1 ) &&  minPress_val > MINPRESS ) { // up and down arrows
        lcd.write((byte)3);     
      } else if (minPress_val < ( maxPress_val - 1 )){ // up arrow
        lcd.write((byte)1);     
      } else if (minPress_val > MINPRESS){ // down arrow
        lcd.write((byte)2);     
      }
      break;
    case 2:                                 // Max press
      lcd.clear();
      lcd.print(" Max press:");
      // second row
      lcd.setCursor(0, 1);
      lcd.print("  ");
      print_val = maxPress_val;
      lcd.print((print_val/10),1);
      lcd.print("  bar");
      lcd.setCursor(15,1);
      if ( maxPress_val > ( minPress_val + 1 ) &&  maxPress_val < MAXPRESS ) { // up and down arrows
        lcd.write((byte)3);     
      } else if (maxPress_val > ( minPress_val + 1 )){ // down arrow
        lcd.write((byte)2);     
      } else if (maxPress_val < MAXPRESS){ // up arrow
        lcd.write((byte)1);     
      }
      break;
    case 3: // Flux sensor type 0=none, 1=switch, 2=YFG1, 3=YFDN32, 4=YFDN40 , 5=YFDN50 , 6=FS300A, 7=YFS401, 8=YFS201, 9=custom
      lcd.clear();
      lcd.print("Flux Sensor:");
      // second row
      lcd.setCursor(0, 1);
      if ( fluxSensorType_val == 1 ) { // switch
        lcd.print("Switch");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      } else if ( fluxSensorType_val == 2 ) { // YFG1
        lcd.print("YF-B1");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      } else if ( fluxSensorType_val == 3 ) { // YFDN32
        lcd.print("YFDN32");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      } else if ( fluxSensorType_val == 4 ) { // YFDN40 
        lcd.print("YFDN40");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      } else if ( fluxSensorType_val == 5 ) { // YFDN50  
        lcd.print("YFDN50");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      } else if ( fluxSensorType_val == 6 ) { // FS300A
        lcd.print("FS300A");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      } else if ( fluxSensorType_val == 7 ) { // YFS401
        lcd.print("YF-S401");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      } else if ( fluxSensorType_val == 8 ) { // YFS201
        lcd.print("YF-S201");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      } else if ( fluxSensorType_val == 9 ) { // custom
        lcd.print("> Custom ");
        if (pulseFactor < 10){
          lcd.print(pulseFactor,2);
        } else {
          lcd.print(pulseFactor,1);
        }
        lcd.print("");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // down arrow
      } else { // none
        lcd.print("None");
        lcd.setCursor(15,1);
        lcd.write((byte)1);     // up arrow
      }
      break;
    case 4:                               // Mode
      lcd.clear();
      lcd.print(" Mode:");
      // second row
      lcd.setCursor(0, 1);
      if ( ctrlMode_val == 0 ) { // mode pump
        lcd.print("Pump");
        lcd.setCursor(15,1);
        lcd.write((byte)1);     // up arrow
      } else if (ctrlMode_val == 1){ // mode compressor
        lcd.print("Compressor");
        lcd.setCursor(15,1);
        lcd.write((byte)2);     // down arrow
      }
      break;
    case 5: // Pressure sensor type 0=Custom, 1=500Kpa, 2=700Kpa, 3=800Kpa, 4=1Mpa, 5=1.2Mpa, 6=1.5Mpa, 7=2Mpa, 8=3Mpa, 9=4Mpa
      lcd.clear();
      lcd.print("Pressure sensor:");
      // second row
      lcd.setCursor(0, 1);
      if ( pressSensorType_val == 1 ) { 
        lcd.print("500 Kpa");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      } else if ( pressSensorType_val == 2 ) { 
        lcd.print("700 Kpa");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      } else if ( pressSensorType_val == 3 ) { 
        lcd.print("800 Kpa");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      } else if ( pressSensorType_val == 4 ) { 
        lcd.print("1.0 Mpa");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      } else if ( pressSensorType_val == 5 ) { 
        lcd.print("1.2 Mpa");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      } else if ( pressSensorType_val == 6 ) { 
        lcd.print("1.5 Mpa");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      } else if ( pressSensorType_val == 7 ) { 
        lcd.print("2.0 Mpa");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      } else if ( pressSensorType_val == 8 ) { 
        lcd.print("3.0 Mpa");
        lcd.setCursor(15,1);
        lcd.write((byte)3);     // up and down arrow
      } else if ( pressSensorType_val == 9 ) { 
        lcd.print("4.0 Mpa");
        lcd.setCursor(15,1);
        lcd.write((byte)2);     // down arrow
      } else { // Custom
        lcd.print("> Custom: ");
        lcd.print(pressFactor,2);
        lcd.setCursor(15,1);
        lcd.write((byte)1);     // up arrow
      }
      break;
  }
}

void viewLevel_3 () {
  switch (option) {
    case 0:
      break;
    case 3:                                 // Flux sensor type custom
      lcd.clear();
      lcd.print("Set pulse rate ");
      // second row
      lcd.setCursor(0, 1);
      if (pulseFactor < 10){
        lcd.print(pulseFactor,2);
      } else {
        lcd.print(pulseFactor,1);
      }
      lcd.print(" K (vol/pulse)");
      lcd.setCursor(15,1);
      if ( pulseFactor > 0.00 && pulseFactor < 10.0) { // up and down arrows
        lcd.write((byte)3);     
      } else if (pulseFactor == 0.00 ){ // up arrow
        lcd.write((byte)1);     
      } else { // down arrow
        lcd.write((byte)2);     
      }
      break;
    case 5:                                 // Pressure sensor type custom
      lcd.clear();
      lcd.print("Pressure factor");
      // second row
      lcd.setCursor(0, 1);
      lcd.print(pressFactor,2);
      lcd.print(" Bar/Volts");
      lcd.setCursor(15,1);
      if ( pressFactor > 0.00 && pressFactor < 10.0) { // up and down arrows
        lcd.write((byte)3);     
      } else if (pressFactor == 0.00 ){ // up arrow
        lcd.write((byte)1);     
      } else { // down arrow
        lcd.write((byte)2);     
      }
      break;
  }
}

void viewAlarm (int optalarm) {
  digitalWrite(MOTOR, LOW );
  digitalWrite(EVALVE, LOW );
  motor_status = false; // motor OFF
  switch (optalarm) {
    case 1:                                 // Over max time run
      lcd.clear();
      lcd.print("! ALARM ! ");
      // second row
      lcd.setCursor(0, 1);
      lcd.print("Run over ");
      lcd.print(max_on_time);
      lcd.print(" sec");
      break;
    case 2:                                 // Motor run but no flux 
      lcd.clear();
      lcd.print("! ALARM !");
      // second row
      lcd.setCursor(0, 1);
      lcd.print("Motor ON no flux");
      break;
    default:
      break;      
  }
  volatile uint8_t alarm_time = ALARM_TIME;     // set alarm time 
  // alarm loop
  while (1) {
    if (optalarm==1){  // Over max time run no return
    }  else {
      if (alarm_time <= 0 && attempts_rest > 0) {
        alarm_time = ALARM_TIME;
        attempts_rest --; // decrement
        operating_time = 0; // reset alarm time
        flush_control_counter=0;
        motor_status = true; // motor ON
        digitalWrite(MOTOR, HIGH );
        return; //exit
      } 
      if (optalarm==2){ // Motor run but no flux 
        lcd.setCursor(14,0);
        lcd.print(attempts_rest);
      }
      lcd.display();
      _delay_ms(750);
      lcd.noDisplay();
      _delay_ms(250);
      alarm_time --;
    }
    btnStateBCK = touchbtnBCK.booltouch();
    if (btnStateBCK == HIGH ) { // push BACK 
      flush_control_counter=0;
      alarm_time = ALARM_TIME;
      attempts_rest = ATTEMPTS; //reset rest attempts
      operating_time = 0; // reset alarm timer
      motor_status = true; // accendo il motore
      return;
    }
  }
  return;
}

void onesec(){
  if (currentMillis - startOneSecMillis >= OneSec)  //test whether the period has elapsed
  {
    // read the value from the pressure sensor:
    pressRead = analogRead(PRESSENS)-zeroCalibration;
    if (pressRead <= 0){
      pressRead = 0;
    }
    if (pressSensorType_val >= 1 ) {  // preset Pressure sensor
      pressFactor = pressFactorData[pressSensorType_val];
    } else {
       
    }
    pressValue = pressRead*pressFactor/10;    
    if (fluxSensorType_val <= 1 ) {  // switch or no flux sensor
      pulseCount = digitalRead(FLOWSENS);
    } else if (fluxSensorType_val >= 9 ) {  // pulseFactor was taken from the EEPROM 
      pulseFactor = digitalRead(FLOWSENS);
    } else { // hall effect sensor
      pulseFactor = flowFactorData[fluxSensorType_val];
      // Disable the interrupt while calculating flow rate 
      detachInterrupt(flowsensorInterrupt);
      flowRate = pulseCount*pulseFactor/10;
      if (motor_status == true){
        // TIME_FLUSH_CONTROL 
        if (flush_control_counter > TIME_FLUSH_CONTROL ) {
          viewAlarm(2);
        } else if (pulseCount <= 1 ) {
          flush_control_counter++;
        } else  { 
          flush_control_counter=0;
          attempts_rest = ATTEMPTS; //reset rest attempts
        }
      }
    }
    if(menuLevel==1){
      viewLevel_1();
    } else if(menuLevel==2){
      viewLevel_2();
    } else if(menuLevel==3){
      viewLevel_3();
    } else {
      viewHome();
      if ( btnENT_pressed_counter <= LONGPRESS_LEN ) {
        btnENT_pressed_counter++;
      }
    }
    if(pressValue >= maxPress_val){
      if (menuLevel == 0){
        lightStatus = false;
      }
      motor_status=false;
    }
    if(pressValue < minPress_val){
      motor_status = true;
    }

    if (motor_status == true){
      operating_time++;
      lightStatus = true;
      digitalWrite(MOTOR, HIGH );
    } else {
      operating_time = 0;
      digitalWrite(MOTOR, LOW );
      digitalWrite(EVALVE, LOW );
      valveStatus = false;    
    }
    if (lightStatus == true){
      digitalWrite(BACKLIGHT, HIGH );
    } else {
      digitalWrite(BACKLIGHT, LOW );
    }

    if (fluxSensorType_val <= 1 ) {  // switch or no flux sensor

    } else { // hall effect sensor
      // Reset the pulse counter so we can start incrementing again
      pulseCount = 0;    
      // Enable the interrupt again now that we've finished 
      attachInterrupt(flowsensorInterrupt, pulseCounter, FALLING);
    }
    startOneSecMillis = currentMillis;  
  }
}

void decisec(){
  if (currentMillis - startDeciSecMillis >= DeciSec)  //test whether the period has elapsed
  {
    if(motor_status){
      lcd.setCursor(15,0);
      if (displaySpinn > 3){
        lcd.print(char(0));
        displaySpinn = 0;
      } else {
        lcd.print(ShowSpinn[displaySpinn]);       
      }
      displaySpinn++;
      lcd.display();
      startDeciSecMillis = currentMillis; 
    }

    if (ctrlMode_val == 1){ 
    // MODE COMPRESSOR
      if (fluxSensorType_val <= 1 ) {  // switch or no flux sensor
        pulseCount = digitalRead(FLOWSENS);
      } 
      if (operating_time >= max_on_time) {
          //  STOP ALL!!!
          //  Motor has been running for too long: THERE MAY BE A LEAK!
          motor_status = false; // stop motor
          viewAlarm(1); // leak error
      } else if (operating_time >= (start_delay + 1)) {
          digitalWrite(EVALVE, HIGH);
          valveStatus = true;
          if ( pulseCount == HIGH && fluxSensorType_val == 1 ) { // switch as sensor but no flux
            //  STOP ALL!!!
            //  Motor has been running for too long: THERE MAY BE A LEAK!
            //  Flow control appears that the level switch-flow is open therefore at 1, 
            //  i.e. the float at the top, it means that there is no air flow, 
            //  so I stop everything and write the error on display (flow error)        
            motor_status = false; // stop motor
            if ( menuLevel == 0 ) { // switch as sensor but no flux
              viewAlarm(2); // flow error
            }
          }
      }
    } else {
    // MODE PUMP
      if (fluxSensorType_val <= 1 ) {  // switch or no flux sensor
        pulseCount = digitalRead(FLOWSENS);
      }   
      if (operating_time >= max_on_time) {
          //  STOP ALL!!!
          //  Motor has been running for too long: THERE MAY BE A LEAK!
          motor_status = false; // stop motor
          viewAlarm(1); // leak error
      }
    }
  }
}

void loop() {
  currentMillis = millis();  //get the current time
  decisec();
  onesec();
  buttonPressed = ' ';

  // sensors DEBUG ;
  /*
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(touchbtnUP.touch());
  lcd.print(' ');
  lcd.print(touchbtnDWN.touch());
  lcd.setCursor(0, 1);
  lcd.print(touchbtnENT.touch());
  lcd.print(' ');
  lcd.print(touchbtnBCK.touch());
  */

  // process buttons
  btnStateUP = touchbtnUP.booltouch();
  btnStateDWN = touchbtnDWN.booltouch();
  btnStateBCK = touchbtnBCK.booltouch();
  btnStateENT = touchbtnENT.booltouch();
  if ( btnStateUP == HIGH && prevBtnUP == LOW ) {
    buttonPressed = 'U';
    lightStatus = true;
    processButton(buttonPressed);  
  } else if (btnStateDWN == HIGH && prevBtnDWN == LOW ){
    buttonPressed = 'D';
    lightStatus = true;
    processButton(buttonPressed);  
  } else if (btnStateBCK == HIGH && prevBtnBCK == LOW ){
    buttonPressed = 'B';
    lightStatus = true;
    processButton(buttonPressed);  
  } else if (btnStateENT == HIGH && prevBtnENT == LOW) {
    if (menuLevel == 0) {
      btnENT_pressed_counter = 0;
      buttonPressed = 'E';
      lightStatus = true;
    } else {
      buttonPressed = 'E';
      lightStatus = true;
      processButton(buttonPressed);
    }
  } else if (menuLevel == 0 && btnStateENT == LOW && prevBtnENT == HIGH) {
    btnENT_pressed_counter = 0;
  } else if (menuLevel == 0 && btnStateENT == HIGH && prevBtnENT == HIGH ){
    if (btnENT_pressed_counter >= LONGPRESS_LEN ) {
      buttonPressed = 'E';
      btnENT_pressed_counter = 0;
      processButton(buttonPressed);
    }
  }
  prevBtnUP  = btnStateUP;   
  prevBtnDWN = btnStateDWN; 
  prevBtnBCK = btnStateBCK; 
  prevBtnENT = btnStateENT;
}