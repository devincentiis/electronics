// =========================================================================
//   ANTONIO DE VINCENTIIS ESR METER FIRMWARE VER. 20261010 FOR ATtiny412   
//   SKETCH CREATO CON ARDUINO IDE CON LIBRERIA megaTinyCore INSTALLATA
//   BODmode disabilitato, clock 20MHz interno
//   -----------------------------------------------------------------------
//   ANTONIO DE VINCENTIIS ESR METER FIRMWARE VER. 20261010 FOR ATtiny412   
//   SKETCH CREATED WITH ARDUINO IDE WITH megaTinyCore LIBRARY INSTALLED
//   BODmode disabled, internal 20MHz clock
// =========================================================================
//    PIN  | NOME_PIN  | FUNZIONI_PRINCIPALI / MAIN FUNCTIONS
// -------------------------------------------------------------------------                  
//   Pin 1 |   VDD     | ALIMENTAZIONE +5Vd / +5Vd POWER SUPPLY
//   Pin 2 |   PA6     | USCITA ANALOGICA PWM 600 HZ / ANALOG PWM OUTPUT 600 HZ (Microammeter)
//   Pin 3 |   PA7     | INGRESSO ANALOGICO ADC / ANALOG ADC INPUT (ESR Sensor)
//   Pin 4 |   PA1     | ONDA QUADRA a 100 kHz / 100 kHz SQUARE WAVE (TCA0 hardware output)
//   Pin 5 |   PA2     | PULSANTE ON/OFF/CALIBRAZIONE / BUTTON ON/OFF/1 OHM CALIBRATION
//   Pin 6 |   PA0     | LINEA UPDI / UPDI LINE (Programming)
//   Pin 7 |   PA3     | LATCH MOSFET (Mantiene acceso il circuito / Keeps the circuit powered)
//   Pin 8 |   GND     | MASSA COMUNE GNDD / COMMON GROUND GNDD
// =========================================================================

// ============= VARIANTI DI COMPILAZIONE DEL PRESENTE E SCOPI ===============================
// ============= COMPILATION VARIANTS AND PURPOSES ==========================================
// In base alla scelta effettuata con la definizione di FIRMWARE_USAGE_MODE potrete compilare
// e fare l'upload tramite UPDI a seconda delle funzione che volete vengano svolte.
// Depending on the choice made with the FIRMWARE_USAGE_MODE definition, you can compile
// and upload via UPDI depending on the functions you want to perform.
//    
// 0 = PER POTER REGOLARE LA TENSIONE DI BIAS DELLO STADIO ANALOGICO AD 1.000v CON IL TRIMMER
//     RV2 DISABILITA LA GENERAZIONE DI 100KHZ E METTE AL VALORE MASSIMO L'USCITA DAC-PWM DEL 
//     MICROMAPEROMETRO PER CONSENTIRE LA TARATURA DEL TRIMMER RV1 A FONDO SCALA ( 0 OHM ).
// 0 = TO ADJUST THE BIAS VOLTAGE OF THE ANALOG STAGE TO 1.000V WITH THE RV2 TRIMMER. IT
//     DISABLES THE 100KHZ GENERATION AND SETS THE MICROAMMETER DAC-PWM OUTPUT TO THE MAXIMUM
//     VALUE TO ALLOW THE CALIBRATION OF THE RV1 TRIMMER AT FULL SCALE ( 0 OHM ).
//
// 1 = ABILITA LA GENERAZIONE DEI 100 KHZ PER IL SENSORE E MONITORA TRAMITE SERIALE I VALORI 
//     DEL COMPONENTE POSTO TRA I PUNTALI, UTILE PER IL DEBUG E/O SI VUOLE VALORIZZARE CON
//     PRECISIONE LE COSTANTI ADC_ZERO E ADC_OPEN (VEDI SOTTO) PRIMA DI COMPILARE IN MODO 2.
// 1 = ENABLES THE 100 KHZ GENERATION FOR THE SENSOR AND MONITORS THE VALUES OF THE COMPONENT
//     PLACED BETWEEN THE PROBES VIA SERIAL. USEFUL FOR DEBUGGING AND/OR PRECISELY VALUING
//     THE ADC_ZERO AND ADC_OPEN CONSTANTS (SEE BELOW) BEFORE COMPILING IN MODE 2.
//
// 2 = COMPILA PER STRUMENTO GIA' TARATO, GESTISCE IL PULSANTE E TUTTE LE FUNZIONI PER LA VITA
//     OPERATIVA, AZZERAMENTO, MISURE, CALIBRAZIONE, AUTOSPEGNIMENTO.
// 2 = COMPILES FOR AN ALREADY CALIBRATED INSTRUMENT, MANAGES THE BUTTON AND ALL FUNCTIONS FOR
//     THE OPERATIONAL LIFETIME: ZEROING, MEASUREMENTS, CALIBRATION, AUTO-POWER OFF.
//
// Quindi è indispensabile prima caricare il firmware compilato in MODO 0 che consente di 
// tarare il trimmer di fondo scala del microamperometro e quello dell'offset 1.000V dello
// stadio analogico.
// Therefore, it is essential to first upload the firmware compiled in MODE 0, which allows
// calibrating the full-scale trimmer of the microammeter and the 1.000V offset of the analog stage.
//
// Opzionalmente compilare e fare l'upload in MODO 1 che collegando con l'apposito test point
// consente di monitorare tramite seriale il corretto funzionamento ed acquisire i valori
// esatti delle sottosegnate costanti ADC_ZERO e ADC_OPEN.
// Optionally compile and upload in MODE 1, which by connecting to the appropriate test point
// allows monitoring correct operation via serial and acquiring the exact values of the constants
// ADC_ZERO and ADC_OPEN below.
//
// Infine compilare e fare l'upload in MODO 2 che poi è il firmware che consente il funziona-
// mento operativo dello strumento.
// Finally, compile and upload in MODE 2, which is the firmware that enables the operational
// functioning of the instrument.
// =========================================================================================
#define FIRMWARE_USAGE_MODE 2

// compilando con FIRMWARE_USAGE_MODE 1 che attiva i monitor seriale potrete leggere i valori esatti
// del vostro stadio analogico che presumibilmente non si discosteranno molto dai seguenti (945-130):
// e che comunque potrete modificare prima di compilare
// By compiling with FIRMWARE_USAGE_MODE 1 which activates the serial monitors, you can read the exact
// values of your analog stage, which will presumably not differ much from the following (945-130):
// and which you can anyway modify before compiling.
#define ADC_ZERO 945
#define ADC_OPEN 130

// valorizzo per il check al boot verranno rimpiazzato dai valori reali ma servono per i controlli di congrità
// Values initialized for the boot check, they will be replaced by real values but are needed for consistency checks
int16_t adcZeroCalib = ADC_ZERO; // a puntali chiusi / with probes closed
int16_t adcOpenCalib = ADC_OPEN; // a puntali aperti / with probes open

#if FIRMWARE_USAGE_MODE == 1 
  // Modalità monitoraggio mi serve la seriale che però occupa troppa memoria e non posso lasciarla in MODO 2
  // Monitoring mode requires the serial port, which takes up too much memory to be left in MODE 2
  #include <SoftwareSerial.h>
  SoftwareSerial serialToPC(-1, PIN_PA2);
#endif

#include <EEPROM.h>

// questo valore servirà solo fintanto che non viene effettuata una calibrazione con 
// la resistenza campione da 1 Ohm, successivamente verrà sostituita da quella memorizzata sulla EEPROM
// this value will only be used until a calibration with the 1 Ohm sample resistor is performed,
// subsequently it will be replaced by the one stored in the EEPROM
#define TYPICALLY_ONEOHM 600
int16_t adcOneOhmCalib = TYPICALLY_ONEOHM;

// Utilizzati dall'ADC per fare la media di 64 campioni
// Used by the ADC to average 64 samples
volatile int32_t readingAdcSumEsr = 0;
volatile int8_t readingAdcCount = 0;
volatile bool adcReady = false;
int16_t raw10bitAdcEsr = 0;

// Gestione calibrazioni al boot avanzate
// Advanced boot calibration management
bool bootOpenCalibDone = false;
bool bootZeroCalibDone = false;
unsigned long animAzimuthMs = 0;
int16_t animDirection = 20;
int16_t currentAnimDac = 511;

// Macro per scrittura PWM a 10 bit su TCD0 (Valori ammessi: 0 - 1023)
// Utilizza il meccanismo di sincronizzazione hardware per evitare glitch
// Macro for 10-bit PWM writing on TCD0 (Allowed values: 0 - 1023)
// Uses the hardware synchronization mechanism to prevent glitches
#define SET_INSTRUMENT_LEVEL(val)   do {     TCD0.CMPACLR = (val);     TCD0.CTRLE = TCD_SYNC_bm;   } while(0)

// funzione per muovere l'indicatore
// function to move the needle indicator
void moveNeedle(int nMove, int byteOffset, int amplitude, unsigned long delayMs) {
  for (int i = 0; i < nMove; i++) {
    int ctrlAmplitude = byteOffset + amplitude;
    if (ctrlAmplitude > 1023) {
      ctrlAmplitude = 1023;
    }
    int ctrlBack = byteOffset - amplitude;
    if (ctrlBack < 0) {
      ctrlBack = 0;
    }
    
    SET_INSTRUMENT_LEVEL(byteOffset);
    delay(delayMs);
    SET_INSTRUMENT_LEVEL(ctrlAmplitude);
    delay(delayMs);
    SET_INSTRUMENT_LEVEL(ctrlBack);
    delay(delayMs);
    SET_INSTRUMENT_LEVEL(byteOffset);
    delay(delayMs);
  }
}

void setup() {
  // ==========================================================================
  // 0. LETTURA DALLA EEPROM DEL VALORE DELLLA TARATURA CON RESISTENZA DA 1 OHM
  // 0. READING THE 1 OHM RESISTOR CALIBRATION VALUE FROM EEPROM
  // ==========================================================================
  EEPROM.get(0, adcOneOhmCalib);
  
  // se non è valido uso quello congruo in attesa di una calibrazione 
  // if it's not valid, use the typical one while waiting for a calibration
  if (adcOneOhmCalib > (TYPICALLY_ONEOHM + 100) || adcOneOhmCalib < (TYPICALLY_ONEOHM - 100)) {   
    adcOneOhmCalib = TYPICALLY_ONEOHM;
  }

  cli();

  // ==========================================================================
  // 1. RIPRENDIAMOCI IL CONTROLLO DEL TCA0 DA megaTinyCore
  // 1. TAKE BACK CONTROL OF TCA0 FROM megaTinyCore
  // ==========================================================================
  takeOverTCA0();

  // ==========================================================================
  // 2. CONFIGURAZIONE PIN INPUT/OUTPUT 
  // 2. INPUT/OUTPUT PIN CONFIGURATION
  // ==========================================================================
  PORTA.DIRSET = PIN3_bm; // PA3 -> Uscita LATCH MOSFET / MOSFET LATCH Output
  PORTA.OUTSET = PIN3_bm; // Accende subito il circuito mantenendo il mosfet alto / Turns on the circuit immediately keeping mosfet high
  PORTA.DIRSET = PIN6_bm; // PA6 -> Forza come USCITA hardware (Microamperometro) / Force as hardware OUTPUT (Microammeter)
  
  #if FIRMWARE_USAGE_MODE == 2
    // Modalità strumento operativo: configurazione atomica di PA2 per il pulsante
    // Operational instrument mode: atomic configuration of PA2 for the button
    PORTA.DIRCLR = PIN2_bm;             // PA2 -> Ingresso Pulsante ON/OFF / Button Input ON/OFF
    PORTA.PIN2CTRL = PORT_PULLUPEN_bm;  // Abilita Pull-up E abilita il buffer di input digitale / Enables Pull-up AND digital input buffer
  #endif
  
  PORTA.DIRCLR = PIN7_bm; // PA7 -> Ingresso ADC (Lettura ESR) / ADC Input (ESR Reading)
  PORTA.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc; // Spegne buffer digitale su PA7 (risparmio rumore) / Disables digital buffer on PA7 (noise reduction)

  #if FIRMWARE_USAGE_MODE == 0
    PORTA.DIRCLR = PIN1_bm; // Disattivo l'uscita onda quadra a 100 KHz / Deactivate the 100 KHz square wave output
  #else
    PORTA.DIRSET = PIN1_bm; // PA1 -> Attivo uscita onda quadra a 100 KHz / PA1 -> Activate 100 KHz square wave output
  #endif  

  // ============================================================
  // 3. RIFERIMENTO ADC / ADC REFERENCE
  // ============================================================
  VREF.CTRLA = VREF_ADC0REFSEL_1V1_gc; // Solo riferimento ADC / ADC reference only
  VREF.CTRLB = VREF_ADC0REFEN_bm;

  // ============================================================
  // 4. ADC0
  // ============================================================
  ADC0.CTRLA = ADC_ENABLE_bm;
  ADC0.CTRLB = ADC_SAMPNUM_ACC1_gc;
  // Prescaler a 8: Clock ADC = 20MHz / 8 = 2.5 MHz (Il Sample & Hold dura 1.8 µs) con riferimento di tensione interna (1.1V)
  // Prescaler to 8: ADC Clock = 20MHz / 8 = 2.5 MHz (Sample & Hold lasts 1.8 µs) with internal voltage reference (1.1V)
  ADC0.CTRLC = ADC_PRESC_DIV8_gc | ADC_REFSEL_INTREF_gc; 
  ADC0.CTRLD = 0;
  ADC0.MUXPOS = ADC_MUXPOS_AIN7_gc;
  ADC0.EVCTRL = ADC_STARTEI_bm;
  ADC0.INTCTRL = ADC_RESRDY_bm;

  // ===========================================================================
  // 5. CONFIGURAZIONE TIMER D (TCD0) / TIMER D CONFIGURATION (TCD0)
  // PWM A 10 BIT a BASSA FREQUENZA (~610 Hz) PER AVERE PRECISIONE
  // 10-BIT PWM AT LOW FREQUENCY (~610 Hz) FOR PRECISION
  // ===========================================================================
  TCD0.CTRLA = 0;      // Spegne momentaneamente il timer D per configurarlo / Temporarily turns off timer D to configure it
  TCD0.CMPBCLR = 1023; // Impostiamo il fondo scala (Periodo) a 1023 per avere la risoluzione a 10 bit / Set the full scale (Period) to 1023 for 10-bit resolution
  TCD0.CMPASET = 0;    // Il fronte del PWM parte alto a inizio ciclo / The PWM edge starts high at the beginning of the cycle
  TCD0.CMPACLR = 0;    // Duty cycle iniziale (0 = strumento a zero) / Initial duty cycle (0 = instrument at zero)
  
  // Abilita l'uscita del comparatore A (WOA) sul pin fisico PA6
  // FAULTCTRL è un registro protetto da scrittura (Configuration Change Protection)
  // Enables the output of comparator A (WOA) on physical pin PA6
  // FAULTCTRL is a write-protected register (Configuration Change Protection)
  _PROTECTED_WRITE(TCD0.FAULTCTRL, TCD_CMPAEN_bm); 
  
  // Attende che il modulo sia pronto e sincronizzato / Waits for the module to be ready and synchronized
  while (!(TCD0.STATUS & TCD_CMDRDY_bm));
  
  // Configurazione Clock: Usiamo il clock di sistema applicando un prescaler DIV32. 
  // Frequenza risultante: 20 MHz / 32 / 1024 = ~610 Hz.
  // Clock Configuration: We use the system clock applying a DIV32 prescaler. 
  // Resulting frequency: 20 MHz / 32 / 1024 = ~610 Hz.
  TCD0.CTRLA = TCD_CLKSEL_SYSCLK_gc | TCD_CNTPRES_DIV32_gc | TCD_ENABLE_bm;

  // ============================================================
  // 6. TCA0
  // ============================================================
  TCA0.SINGLE.CTRLA = 0;
  TCA0.SINGLE.CTRLESET = TCA_SINGLE_CMD_RESET_gc;
  TCA0.SINGLE.CTRLD = 0;  // Assicuriamoci di essere in SINGLE mode / Ensure we are in SINGLE mode
  TCA0.SINGLE.PER = 199;  // Periodo 100 kHz 10.0 µs / Period 100 kHz 10.0 µs
  TCA0.SINGLE.CMP0 = 140;
  TCA0.SINGLE.CMP1 = 100; // Duty ≈ 50%
  
  #if FIRMWARE_USAGE_MODE == 0
    // Modalità taratura trimmer RV1 e RV2
    // Trimmer RV1 and RV2 calibration mode
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc; // spengo l'uscita di simolo del sensore per consentire la taratura del valore di bias del sensore ad 1.000V / turn off sensor simulation output to allow bias calibration to 1.000V
  #else
    // Modalità monitoraggio valori opp Modalità strumento operativo
    // Value monitoring mode OR Operational instrument mode
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP1EN_bm; // Abilita uscita WO1 generata direttamente dall'hardware TCA0 TCA_SINGLE_CMP1EN_bm Abilita la generazione dell'onda quadra sul pin fisico / Enables WO1 output generated directly by TCA0 hardware, TCA_SINGLE_CMP1EN_bm enables square wave generation on physical pin
  #endif
  
  PORTMUX.CTRLC |= PORTMUX_TCA00_ALTERNATE_gc; // PORTMUX Instrada TCA0: WO1 -> PA1 (pin fisico 4) Sul tinyAVR 412 questo è il routing alternativo. / PORTMUX routes TCA0: WO1 -> PA1 (physical pin 4) On tinyAVR 412 this is the alternate routing.
  TCA0.SINGLE.INTCTRL = 0; // nessun interrupt TCA / no TCA interrupt

  // ============================================================
  // 7. EVENT SYSTEM CON PRESCALER /64 / EVENT SYSTEM WITH PRESCALER /64
  // ============================================================
  // Selezioniamo il generatore TCA0_CMP0 (0x04) e impostiamo il divisore hardware /64 (0x06 spostato di 4 bit che non mi sembra si possa fare con megaTinyCore)
  // We select the TCA0_CMP0 generator (0x04) and set the hardware divider /64 (0x06 shifted by 4 bits, which it seems cannot be done with megaTinyCore)
  EVSYS.SYNCCH0 = EVSYS_SYNCCH0_TCA0_CMP0_gc | (0x06 << 4); 
  EVSYS.ASYNCUSER1 = EVSYS_ASYNCUSER1_SYNCCH0_gc; // Configura l'utilizzatore dell'evento (ADC0) per ascoltare il canale SYNCCH0 / Configures the event user (ADC0) to listen to channel SYNCCH0

  // ============================================================
  // 8. PULIZIA EVENTUALI FLAG PENDENTI / CLEARING ANY PENDING FLAGS
  // ============================================================
  ADC0.INTFLAGS = ADC_RESRDY_bm;

  // ============================================================
  // 9. AVVIO TCA0 / STARTING TCA0
  // ============================================================
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm;

  sei();

  #if FIRMWARE_USAGE_MODE == 1 // Modalità monitoraggio valori / Value monitoring mode
    serialToPC.begin(9600);    // invio in seriale solo quando compilo per monitoraggio / send via serial only when compiled for monitoring
  #endif

  // ==========================================================================
  // CONTROLLO OBBLIGATORIO AL BOOT DI PUNTALI UNITI E APERTI (MODE 2)
  // MANDATORY CHECK AT BOOT FOR CLOSED AND OPEN PROBES (MODE 2)
  // ==========================================================================
  #if FIRMWARE_USAGE_MODE == 2
    // Assicuriamoci che l'interrupt sul pulsante sia spento a livello atomico
    // Ensure that the button interrupt is turned off atomically
    PORTA.PIN2CTRL = PORT_PULLUPEN_bm; 
    PORTA.INTFLAGS = PIN2_bm; 

    int16_t lastValidSample = -100;
    int8_t stabilityCounter = 0;
    unsigned long lastAnimMs = 0;

    // Svuota i buffer dell'ADC prima di iniziare / Flush ADC buffers before starting
    cli(); 
    readingAdcSumEsr = 0; 
    readingAdcCount = 0; 
    adcReady = false; 
    sei();

    // Sblocca il controllo di spegnimento solo DOPO che l'utente ha rilasciato il tasto di accensione
    // Unlocks the power-off control only AFTER the user has released the power button
    bool powerOnButtonReleased = false;

    // --- FASE 1: CONTROLLO PUNTALI UNITI (ZERO CALIB) ---
    // --- PHASE 1: CLOSED PROBES CHECK (ZERO CALIB) ---
    while (!bootZeroCalibDone) {
      // Controlla se il pulsante di accensione è stato rilasciato almeno una volta
      // Checks if the power button has been released at least once
      if (!powerOnButtonReleased && (PORTA.IN & PIN2_bm)) {
        delay(20); // Minimo debounce / Minimum debounce
        if (PORTA.IN & PIN2_bm) {
          powerOnButtonReleased = true; // Da questo momento in poi, una nuova pressione spegnerà lo strumento / From this moment on, a new press will turn off the instrument
        }
      }

      // INTERRUZIONE E SPEGNIMENTO IMMEDIATO AL BOOT (Attivo solo dopo il primo rilascio)
      // INTERRUPTION AND IMMEDIATE POWER OFF AT BOOT (Active only after first release)
      if (powerOnButtonReleased && !(PORTA.IN & PIN2_bm)) {
        delay(50); // Antirimbalzo immediato / Immediate debounce
        if (!(PORTA.IN & PIN2_bm)) {
          goto force_power_off_boot;
        }
      }

      // Animazione ago in attesa: oscilla dolcemente a metà scala
      // Waiting needle animation: oscillates gently at mid-scale
      if (millis() - lastAnimMs >= 20) {
        lastAnimMs = millis();
        currentAnimDac += animDirection;
        if (currentAnimDac >= 650 || currentAnimDac <= 370) {
          animDirection = -animDirection;
        }
        SET_INSTRUMENT_LEVEL(currentAnimDac);
      }

      if (adcReady) {
        int16_t currentSample = readingAdcSumEsr >> 6;
        cli(); 
        readingAdcSumEsr = 0; 
        readingAdcCount = 0; 
        adcReady = false; 
        sei();

        // Verifica che il valore sia nei dintorni di ADC_ZERO (945)
        // Verify that the value is around ADC_ZERO (945)
        if (currentSample >= (ADC_ZERO - 60) && currentSample <= (ADC_ZERO + 60)) {
          if (abs(currentSample - lastValidSample) <= 1) {
            stabilityCounter++;
          } else {
            stabilityCounter = 0;
          }
          lastValidSample = currentSample;

          if (stabilityCounter >= 100) {
            adcZeroCalib = currentSample; 
            bootZeroCalibDone = true;
            SET_INSTRUMENT_LEVEL(767); // Feedback: Sposta l'ago a 3/4 della scala / Feedback: Move the needle to 3/4 scale
            delay(500);
          }
        } else {
          stabilityCounter = 0;
        }
      }
    }

    // Reset variabili per la seconda fase / Reset variables for the second phase
    lastValidSample = -100;
    stabilityCounter = 0;
    currentAnimDac = 100;
    animDirection = 5;

    // --- FASE 2: CONTROLLO PUNTALI APERTI (OPEN CALIB) ---
    // --- PHASE 2: OPEN PROBES CHECK (OPEN CALIB) ---
    while (!bootOpenCalibDone) {
      // Controlla se il pulsante è rilasciato (se l'utente non lo avesse ancora fatto)
      // Check if the button is released (if the user hasn't done it yet)
      if (!powerOnButtonReleased && (PORTA.IN & PIN2_bm)) {
        delay(20);
        if (PORTA.IN & PIN2_bm) {
          powerOnButtonReleased = true;
        }
      }

      // INTERRUZIONE E SPEGNIMENTO IMMEDIATO AL BOOT
      // INTERRUPTION AND IMMEDIATE POWER OFF AT BOOT
      if (powerOnButtonReleased && !(PORTA.IN & PIN2_bm)) {
        delay(50); // Antirimbalzo immediato / Immediate debounce
        if (!(PORTA.IN & PIN2_bm)) {
          goto force_power_off_boot;
        }
      }

      // Animazione ago in attesa: oscilla vicino allo zero
      // Waiting needle animation: oscillates near zero
      if (millis() - lastAnimMs >= 25) {
        lastAnimMs = millis();
        currentAnimDac += animDirection;
        if (currentAnimDac >= 160 || currentAnimDac <= 40) {
          animDirection = -animDirection;
        }
        SET_INSTRUMENT_LEVEL(currentAnimDac);
      }

      if (adcReady) {
        int16_t currentSample = readingAdcSumEsr >> 6;
        cli(); 
        readingAdcSumEsr = 0; 
        readingAdcCount = 0; 
        adcReady = false; 
        sei();

        if (currentSample >= (ADC_OPEN - 60) && currentSample <= (ADC_OPEN + 60)) {
          if (abs(currentSample - lastValidSample) <= 1) {
            stabilityCounter++;
          } else {
            stabilityCounter = 0;
          }
          lastValidSample = currentSample;

          if (stabilityCounter >= 100) {
            adcOpenCalib = currentSample; 
            bootOpenCalibDone = true;
            moveNeedle(2, 0, 40, 100); // Doppia spazzata rapida di conferma / Double rapid confirmation sweep
          }
        } else {
          stabilityCounter = 0;
        }
      }
    }

    // Punto di uscita rapido per lo spegnimento se premuto nel setup
    // Fast exit point for power-off if pressed during setup
    if (false) { 
      force_power_off_boot:
      SET_INSTRUMENT_LEVEL(0);
      
      // Attendi che rilasci il pulsante prima di spegnere per evitare rimbalzi tardivi
      // Wait for button release before powering off to avoid late bounces
      unsigned long releaseStability = millis();
      while (millis() - releaseStability < 150) { 
        if (!(PORTA.IN & PIN2_bm)) {
          releaseStability = millis(); 
        }
      }
      delay(100);
      
      PORTA.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc; // Isola il pin / Isolate the pin
      PORTA.OUTCLR = PIN3_bm;                     // Sgancia il LATCH MOSFET / Release the MOSFET LATCH
      while(1) {
        PORTA.OUTCLR = PIN3_bm;
        asm volatile("nop");
      }
    }

    // Lasciamo l'interrupt spento per evitare il reboot nel loop principale
    // Leave the interrupt off to prevent reboot in the main loop
    PORTA.PIN2CTRL = PORT_PULLUPEN_bm;
    PORTA.INTFLAGS = PIN2_bm;
  #endif
}

unsigned long dacLastUpdate = 0;
const unsigned long dacRefreshRate = 100; // Tempo in millisecondi invio a seriale e DAC/PWM / Time in milliseconds for serial and DAC/PWM output
unsigned long lastActivity = 0;
int16_t previousAdcValue = 0; // Serve per rilevare se l'ago si sta muovendo (uso attivo) / Used to detect if the needle is moving (active use)

void loop() {
  bool buttonPressed = false;
  // La calibrazione e la gestione del pulsante si attivano SOLO in modalità operativa (2)
  // Calibration and button management activate ONLY in operational mode (2)
  #if FIRMWARE_USAGE_MODE == 2
    if (!(PORTA.IN & PIN2_bm)) {
      delay(50); // Debounce iniziale alla pressione / Initial debounce on press
      if (!(PORTA.IN & PIN2_bm)) {
        buttonPressed = true; // Isola il resto del loop ESR standard / Isolates the rest of the standard ESR loop
        unsigned long pressTimeStart = millis(); 
        bool calibrationDone = false;
        int16_t previousSample = -100; 
        int8_t stabilityCounter = 0;
        
        // == CICLO DI PRESSIONE DEL PULSANTE ==
        // == BUTTON PRESS CYCLE ==
        // Il micro rimane qui dentro finché non rilasci il tasto o finché non calibra con successo
        // The micro remains here until you release the button or until it calibrates successfully
        while (!(PORTA.IN & PIN2_bm)) {
          // Se non ha ancora calibrato, tieni l'ago a metà scala come spia
          // If it hasn't calibrated yet, keep the needle at mid-scale as an indicator
          if (!calibrationDone) {
            SET_INSTRUMENT_LEVEL(511); 
          }
          
          // Se tieni premuto per più di 2 secondi AND non hai ancora salvato, avvia l'analisi
          // If you keep it pressed for more than 2 seconds AND you haven't saved yet, start the analysis
          if ((millis() - pressTimeStart >= 2000) && !calibrationDone) {
            if (adcReady) {
              int16_t calculatedSample = readingAdcSumEsr >> 6;
              cli();
              readingAdcSumEsr = 0; 
              readingAdcCount = 0; 
              adcReady = false; 
              sei(); 
              
              // Verifica se la resistenza inserita è congrua (Filtro geometrico)
              // Check if the inserted resistor is plausible (Geometric filter)
              if (calculatedSample >= (TYPICALLY_ONEOHM - 100) && calculatedSample <= (TYPICALLY_ONEOHM + 100)) { 
                // Controllo stabilità fine (±1 unità rispetto alla lettura precedente)
                // Fine stability check (±1 unit relative to the previous reading)
                if (abs(calculatedSample - previousSample) <= 1) {
                  stabilityCounter++;
                } else {
                  stabilityCounter = 0; 
                }
                previousSample = calculatedSample;
                
                // Se otteniamo 5 letture stabili consecutive MENTRE premi il tasto: SALVA!
                // If we get 5 consecutive stable readings WHILE pressing the button: SAVE!
                if (stabilityCounter >= 5) {
                  EEPROM.put(0, calculatedSample); 
                  adcOneOhmCalib = calculatedSample; // Aggiorna la variabile di lavoro / Update the working variable
                  
                  // Feedback visivo immediato sotto il tuo dito: l'ago si sposta sulla tacca di 1 Ohm
                  // Immediate visual feedback under your finger: needle moves to the 1 Ohm mark
                  SET_INSTRUMENT_LEVEL(204); 
                  delay(500);
                  moveNeedle(3, 204, 40, 150); // 3 scatti netti di conferma successo / 3 clear clicks confirming success
                  calibrationDone = true;       // Metti il flag a true per uscire/bloccare successive scritture / Set flag to true to exit/block subsequent writes
                }
              } else {
                stabilityCounter = 0; 
              }
            }
          }
          delay(10); // Pausa di sincronizzazione interna al ciclo while / Internal synchronization pause in while loop
        }
        
        // --- IL PULSANTE E' STATO RILASCIATO ---
        // --- THE BUTTON HAS BEEN RELEASED ---
        // Se la taratura NON è stata eseguita (pressione corta), spegni lo strumento
        // If calibration was NOT performed (short press), power off the instrument
        if (!calibrationDone) {
          SET_INSTRUMENT_LEVEL(0); // Riporta l'ago a zero / Bring the needle back to zero
          
          // 1. Attesa prolungata per garantire il rilascio meccanico totale
          // Il pulsante deve essere stabilmente aperto per 300 ms continui
          // 1. Prolonged wait to ensure total mechanical release
          // The button must be stably open for 300 continuous ms
          unsigned long releaseStability = millis();
          while (millis() - releaseStability < 300) { 
            if (!(PORTA.IN & PIN2_bm)) {
              releaseStability = millis(); // Resetta se c'è un rimbalzo / Reset if there's a bounce
            }
          }
          
          // 2. Pausa fissa di sicurezza ulteriore (mezzo secondo) per far calmare il tasto
          // 2. Additional fixed safety pause (half second) to let the button settle
          delay(500); 
          
          // 3. ISOLAMENTO ELETTRONICO DEL PIN PRIMA DELLO SPEGNIMENTO
          // Disabilitiamo il pull-up interno e spegniamo il buffer di ingresso su PA2.
          // In questo modo il pin diventa "cieco" e i rimbalzi non influenzeranno il micro.
          // 3. ELECTRONIC ISOLATION OF THE PIN BEFORE POWER-OFF
          // We disable the internal pull-up and turn off the input buffer on PA2.
          // This way the pin becomes "blind" and bounces won't affect the micro.
          PORTA.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc; 
          PORTA.INTFLAGS = PIN2_bm; // Pulisce eventuali interrupt residui / Clear any residual interrupts
          
          // 4. Sgancia il LATCH MOSFET per togliere l'alimentazione
          // 4. Disengage the MOSFET LATCH to remove power
          PORTA.OUTCLR = PIN3_bm; 
          
          // 5. Ciclo infinito di blocco totale aggressivo
          // Continua a forzare lo spegnimento mentre la tensione decade
          // 5. Infinite aggressive total lockdown loop
          // Continues to force shutdown while the voltage decays
          while(1) {
            PORTA.OUTCLR = PIN3_bm; 
            delay(10); 
          }
        }
        // Se invece la taratura E' STATA eseguita con successo...
        // If instead calibration WAS successfully performed...
        delay(300); // Pulisce i rimbalzi dopo la calibrazione prima di tornare al loop ESR / Clears bounces after calibration before returning to ESR loop
      }
    }
  #endif

  // Se il pulsante non è premuto, procediamo con le normali letture dello strumento
  // If the button is not pressed, we proceed with normal instrument readings
  if (!buttonPressed) {
    if (millis() - dacLastUpdate >= dacRefreshRate) { 
      dacLastUpdate = millis();
      if (adcReady) {
        raw10bitAdcEsr = readingAdcSumEsr >> 6;
        cli(); 
        
        static int16_t filteredAdcEsr = -1;
        if (filteredAdcEsr < 0) {
          filteredAdcEsr = (int16_t)raw10bitAdcEsr << 2; 
        } else {
          filteredAdcEsr = filteredAdcEsr + ((int16_t)raw10bitAdcEsr - (filteredAdcEsr >> 2));
        }
        
        int16_t current10bitAdcEsr = filteredAdcEsr >> 2;
        int16_t dacValue = 0;
        
        if (current10bitAdcEsr >= adcZeroCalib) {
          dacValue = 1023; 
        } 
        else if (current10bitAdcEsr >= adcOneOhmCalib) { 
          int32_t deltaAdc = (int32_t)(current10bitAdcEsr - adcOneOhmCalib);
          int32_t rangeAdc = (int32_t)(adcZeroCalib - adcOneOhmCalib);

          int32_t normX_1024 = (deltaAdc << 10) / rangeAdc;
          int32_t normX_2 = (normX_1024 * normX_1024) >> 10;

          int32_t logX_1024 = ((normX_2 * 870) + (normX_1024 * 154)) >> 10;      

          int32_t minDac10bit = 204; 
          int32_t maxDac10bit = 1023; 
          int32_t rangeDac10bit = maxDac10bit - minDac10bit; 
          dacValue = ((logX_1024 * rangeDac10bit) >> 10) + minDac10bit;
          
          if (dacValue > maxDac10bit) dacValue = maxDac10bit;
          if (dacValue < minDac10bit) dacValue = minDac10bit;
        }
        else if (current10bitAdcEsr > adcOpenCalib) { 
          dacValue = ((int32_t)(current10bitAdcEsr - adcOpenCalib) * 204) / (adcOneOhmCalib - adcOpenCalib);
          if (dacValue > 204) dacValue = 204;
        }
        else {
          dacValue = 0; 
        }
        
        #if FIRMWARE_USAGE_MODE == 0 
          SET_INSTRUMENT_LEVEL(1023); 
        #elif FIRMWARE_USAGE_MODE == 1 
          serialToPC.print("EEPROM_1OHM: ");
          serialToPC.print(adcOneOhmCalib); 
          serialToPC.print(" | ADC_RAW: ");
          serialToPC.print(raw10bitAdcEsr);
          serialToPC.print(" | EMAfilter: ");
          serialToPC.print(current10bitAdcEsr);
          serialToPC.print(" | DAC_PWM: ");
          serialToPC.print(dacValue);
          
          uint16_t instrumentAngle = ((uint32_t)dacValue * 900 + 511) / 1023;
          uint16_t integerPart = instrumentAngle / 10; 
          uint8_t decimalPart = instrumentAngle % 10; 
          
          serialToPC.print(" | Instrument angle: ");
          serialToPC.print(integerPart);
          serialToPC.print(".");
          serialToPC.println(decimalPart);
          SET_INSTRUMENT_LEVEL(dacValue);
        #else 
          SET_INSTRUMENT_LEVEL(dacValue);
          
          // ============================================================
          // GESTIONE AUTOSPEGNIMENTO ATTIVA (MODE 2)
          // ACTIVE AUTO-POWER OFF MANAGEMENT (MODE 2)
          // ============================================================
          // Se la lettura attuale differisce dalla precedente di almeno 100 unità,
          // significa che l'utente sta muovendo i puntali (uso attivo dello strumento)
          // If the current reading differs from the previous one by at least 100 units,
          // it means the user is moving the probes (active use of the instrument)
          if (abs(current10bitAdcEsr - previousAdcValue) > 100) {
            previousAdcValue = current10bitAdcEsr; // Aggiorna il valore di riferimento / Update reference value
            lastActivity = millis();               // Resetta il timer di inattività / Reset inactivity timer
          }

          // GESTIONE AUTOSPEGNIMENTO ATTIVA (MODE 2)
          // ACTIVE AUTO-POWER OFF MANAGEMENT (MODE 2)
          if (millis() - lastActivity >= 100000) {
            // 1. Spostiamo l'ago per feedback visivo / Move needle for visual feedback
            SET_INSTRUMENT_LEVEL(340); 
            // 2. Sostituiamo i delay di Arduino con un ciclo software puro basato su NOP
            // In questo modo siamo indipendenti dai bug del clock di millis()
            // 2. We replace Arduino delays with a pure software loop based on NOP
            // In this way we are independent of millis() clock bugs
            for(volatile long i = 0; i < 500000; i++) { 
              asm volatile("nop"); 
            }
            // 3. Forziamo il blocco degli interrupt prima di toccare i pin / Force interrupt disable before touching pins
            cli();
            // 4. DISATTIVIAMO I TIMER HARDWARE (TCD0 e TCA0)
            // Se i timer continuano a spingere corrente sui pin PA6 e PA1, 
            // il consumo totale impedisce al Latch di sganciarsi. Spegniamoli!
            // 4. WE DISABLE HARDWARE TIMERS (TCD0 and TCA0)
            // If the timers continue to push current onto pins PA6 and PA1, 
            // the total consumption prevents the Latch from releasing. Turn them off!
            TCD0.CTRLA = 0; // Spegne il PWM dell'ago (la lancetta cadrà a zero per mancanza di segnale) / Turns off needle PWM (needle drops to zero due to missing signal)
            TCA0.SINGLE.CTRLA = 0; // Spegne i 100 kHz / Turns off 100 kHz
            // 5. Mettiamo a zero i pin di uscita per evitare correnti parassite / Set output pins to zero to prevent parasitic currents
            PORTA.OUTCLR = PIN1_bm;
            PORTA.OUTCLR = PIN6_bm;
            // 6. SGANCIO AGGRESSIVO E DEFINITIVO DEL LATCH MOSFET / AGGRESSIVE AND DEFINITIVE RELEASE OF MOSFET LATCH
            PORTA.OUTCLR = PIN3_bm; // PA3 va a zero
            // 7. Ciclo infinito senza fine per far morire l'alimentazione / Endless infinite loop to let the power die
            while(1) {
              PORTA.OUTCLR = PIN3_bm;
              asm volatile("nop");
            }
          }
        #endif
      
        readingAdcSumEsr = 0; 
        readingAdcCount = 0; 
        adcReady = false; 
        sei();      
      }
    }
  }
}

ISR(ADC0_RESRDY_vect)
{
  uint16_t adc = ADC0.RES;
  if (!adcReady) {
    readingAdcSumEsr += adc; // Accumula il valore analogico per farne una media / Accumulate the analog value to average it
    readingAdcCount++;
    if (readingAdcCount >= 64) {
      adcReady = true; // Notifica che i 64 campioni sono pronti / Notifies that 64 samples are ready
    }
  }
}
