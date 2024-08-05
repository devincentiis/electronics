/*
 * ADVisor Antonio De Vincentiis https://github.com/devincentiis
 * MONTESILVANO (PESCARA) ITALY https://devincentiis.it 
 * thanks to Paul Newland (microPaul) https://github.com/microPaul/
 * for his "ATtiny412 Sleeping Lighthouse" from which the part of the code related 
 * to the sleep_mode needed to reduce battery consumption was taken
 * compile with megaTinyCore at 4MHz internal  clock 
 * https://github.com/SpenceKonde/megaTinyCore
*/
#include <avr/sleep.h>

#define  F_CPU  4000000 // 4 MHz

#define powerVoltPin 0    // input pin 2 (PA6) for control battery voltage
#define speakerPin 1      // speaker to pin 3 (PA7)
#define ledPin 3          // led to pin (PA2) 5
#define setupDataPin  4  // pin (PA3) 7, only at setup enable potentiometer and resistor divider to read analog values    
#define timetoalarmPin 2  // time to alarm set pot to pin (PA1) 4

volatile uint8_t rtcIntSemaphore;  // flag from RTS interrupt that may be used by polled function

const int minPowerVoltage = 870; // alarm if below about 2.6 volts
 
bool alarmStatus = false;

// variables to store the value coming from the analog pins
int powerVoltValue = 0;
int timeToAlarm = 0;   
int secToAlarm = 0;

int secToFlush = 0;

void setup()
{  
  enableOutPeripherals();  
  // read analog values, toggle 
  pinMode(setupDataPin,OUTPUT);
  digitalWrite(setupDataPin,HIGH);
  analogReference(DEFAULT);
  timeToAlarm = analogRead(timetoalarmPin);    
  secToAlarm = 10 + (timeToAlarm/1.7);
  analogReference(INTERNAL1V1);
  powerVoltValue = analogRead(powerVoltPin);
  //low battery alarm, only one beep at boot 
  if (powerVoltValue < minPowerVoltage ){ 
    digitalWrite(speakerPin,HIGH);
    delay(500);
    digitalWrite(speakerPin,LOW);
    digitalWrite(ledPin,HIGH);
    delay(500);
    digitalWrite(ledPin,LOW);
  }
  // led monitor (debug)
  //oneLedByte(secToAlarm);
  //delay(5000);
  //oneLedByte(powerVoltValue/10);
  //delay(5000);
  disableOutPeripherals();  
  init32kOscRTCPIT(); // init the 32K internal Osc and RTC-PIT for interrupts
  initSleepMode();  // set up the sleep mode
}

void loop()
{
   while(1) {
    sleepNCycles(2); // cycles are about 500ms each
    secToFlush++;
    if (secToFlush > secToAlarm){
      secToFlush=secToAlarm/3; //restart alarm ay half time
      digitalWrite(speakerPin,HIGH);
      delay(1000);
      digitalWrite(speakerPin,LOW);
      digitalWrite(ledPin,HIGH);
      delay(1000);
      digitalWrite(ledPin,LOW);
    }
  }
}

//////////////////////////////////////////////////////////////////
//  ISR(RTC_PIT_vect)
//
//  Interrupt Service Routine for the RTC PIT interrupt
//
ISR(RTC_PIT_vect) {
  RTC.PITINTFLAGS = RTC_PI_bm;  // clear the interrupt flag   
  rtcIntSemaphore = 1; // mark to polled function that interrupt has occurred

}

//////////////////////////////////////////////////////////////////
//  init32kOscRTCPIT()
//
//  initialize the internal ultra low power 32 kHz osc and periodic Interrupt timer
//
//  these two peripherals are interconnected in that the internal 32 kHz
//  osc will not start until a peripheral (PIT in this case) calls for it,
//  and the PIT interrupt should not be enabled unit it is confirmed that
//  the 32 kHz osc is running and stable
//
//  Note that there is ERRATA on the RTC counter,  see:
//  https://ww1.microchip.com/downloads/en/DeviceDoc/ATtiny212-214-412-414-416-SilConErrataClarif-DS80000933A.pdf
//
//  That document currently states "Any write to the RTC.CTRLA register resets the 15-bit prescaler
//  resulting in a longer period on the current count or period".  So if you load the prescaler
//  value and then later enable the RTC (both on the same register) you get a very long
//  (max?) time period.  My solution to this problem is to always enable the RTC in the same
//  write that sets up the prescaler.  That seems to be an effective work-around. 
//
void init32kOscRTCPIT(void) {
  _PROTECTED_WRITE(CLKCTRL.OSC32KCTRLA, CLKCTRL_RUNSTDBY_bm);  // enable internal 32K osc   
  RTC.CLKSEL = RTC_CLKSEL_INT1K_gc; //Select 1.024 kHz from 32KHz Low Power Oscillator (OSCULP32K) as clock source
  RTC.PITCTRLA = RTC_PERIOD_CYC512_gc | RTC_PITEN_bm; //  Enable RTC-PIT with divisor of 512 (~500 milliseconds)
  while (!(CLKCTRL.MCLKSTATUS & CLKCTRL_OSC32KS_bm));  // wait until 32k osc clock has stabilized
  while (RTC.PITSTATUS > 0); // wait for RTC.PITCTRLA synchronization to be achieved
  RTC.PITINTCTRL = RTC_PI_bm;  // allow interrupts from the PIT device  
}

//////////////////////////////////////////////////////////////////
//  initSleepMode()
//
//  this doesn't invoke sleep, it just sets up the type of sleep mode
//  to enter when the MCU is put to sleep by calling "sleep_cpu()"
//
void initSleepMode(void) {
  //SLPCTRL.CTRLA = SLPCTRL_SMODE_IDLE_gc; // set sleep mode to "idle"
  //SLPCTRL.CTRLA = SLPCTRL_SMODE_STDBY_gc; // set sleep mode to "standby"  
  SLPCTRL.CTRLA = SLPCTRL_SMODE_PDOWN_gc; // set sleep mode to "power down"
  SLPCTRL.CTRLA |= SLPCTRL_SEN_bm;  // enable sleep mode
}


//////////////////////////////////////////////////////////////////
//  sleepNCycles(uint16_t val)
//
//  cause system to go to sleep for N cycles, where each cycle
//  is about 500mS.  Note that processor does wakeup every 500 ms
//  but goes back to sleep almost immediately if it has not yet
//  done the desired number of cycles.
//
//  At 4 MHz MCU speed, the awake duration of each iternation in the
//  for loop below is approximately 1.5 microseconds.
//
void sleepNCycles(uint8_t val) {
  // first cycle may not be full cycle
  disableOutPeripherals(); // all off
  for (uint8_t i = 0; i < val ; i++) {
    //diagnosticPinLow(); // set low to show sleeping
    sleep_cpu();  // put MCU to sleep
    //diagnosticPinHigh(); // set high to show awake
  }
  // now awake, sleep cycles complete, continue on
  enableOutPeripherals(); // initialize GPIO 
}

void enableOutPeripherals(void) {
  pinMode(speakerPin,OUTPUT);
  digitalWrite(speakerPin,LOW);
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin,LOW);
}

void disableOutPeripherals(void) {
  pinMode(speakerPin,INPUT);
  digitalWrite(speakerPin,LOW);
  pinMode(ledPin,INPUT);
  digitalWrite(ledPin,LOW);
  pinMode(ledPin,INPUT);
  digitalWrite(ledPin,LOW);
  pinMode(setupDataPin,INPUT);
  digitalWrite(setupDataPin,LOW);
     
  PORTA.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc; // disable input buffers
  PORTA.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;
  PORTA.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
  PORTA.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;
  PORTA.PIN6CTRL = PORT_ISC_INPUT_DISABLE_gc;
  PORTA.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;
}

void oneLedByte(uint8_t val) {
  uint8_t i;
  for (i = 0; i < 8 ; i++) {
    digitalWrite(ledPin,HIGH); // turn on LED
    (val & 1) ? delay(700) : delay (100);  // long delay for 1, short if zero
    digitalWrite(ledPin,LOW); // turn off LED
    delay(1000);
    val >>= 1; // right shift val by one bit
  }
}
