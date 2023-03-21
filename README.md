# 3 Digit 7 Segments Arduino Nano Thermostat 

This is an Arduino Nano 3-digit 7-segment thermostat design with Kicad schematic diagram and PCB.

### Use instructions

* Select type of output combine UP, DOWN and FAN: push 3 seconds enconder switch
* Set the temperature: turn the potentiometer (while viewing the set value)
* Set th timer: turn the encoder (while viewing the set value)
* When timer turn to zero: OFF all 
* Without moving any knobs: display temperature

### Firmware library for [Arduino IDE 1.8.19](https://github.com/arduino/Arduino) 

* SevSeg 3.7.0 by Dean Reading [Github Project Page](https://github.com/DeanIsMe/SevSeg/)
* Simple Rotary 1.1.3 by MPrograms [Github Project Page](https://github.com/mprograms/SimpleRotary/)
* MAX6675 1.1.0 by Adafruit [Github Project Page](https://github.com/adafruit/MAX6675-library/)



<img alt="Thermostatino Top" src="https://github.com/devincentiis/thermostatino/blob/main/ThermostatinoTop.jpg?raw=true" width="100%">


|Ref         |Qnty|Value|Cmp name         |Footprint                      |Description                                                                                    |
|------------|----|-----|-----------------|-------------------------------|-----------------------------------------------------------------------------------------------|
|A1,         |1   |Arduino_Nano_v3.x|Arduino_Nano_v3.x|Module:Arduino_Nano            |Arduino Nano v3.x                                                                              |
|C1, C4,     |2   |10u  |C_Polarized      |Capacitor_THT:CP_Radial_D6.3mm_P2.50mm|Polarized capacitor                                                                            |
|C2, C3,     |2   |100n |C_Small          |Capacitor_SMD:C_1206_3216Metric_Pad1.33x1.80mm_HandSolder|Unpolarized capacitor, small symbol                                                            |
|D1, D2, D5, |3   |LED  |LED              |LED_THT:LED_D5.0mm             |Light emitting diode                                                                           |
|D3, D4, D6, |3   |SS14 |D_Schottky       |Diode_SMD:D_SMA                |Schottky diode                                                                                 |
|J1,         |1   |Terminalx02|Screw_Terminal_01x02|TerminalBlock:TerminalBlock 02_P7.50mm_Horizontal|Generic screw terminal                                                                         |
|J2,         |1   |Terminalx05|Screw_Terminal_01x05|TerminalBlock:TerminalBlock 5_P5.08mm|Generic screw terminal                                                                         |
|Q1, Q2, Q3, |3   |BC817|BC817            |Package_TO_SOT_SMD:SOT-23      |0.8A Ic, 45V Vce, NPN Transistor, SOT-23                                                       |
|R1, R2, R3, |3   |150  |R                |Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder|Resistor                                                                                       |
|R4, R5, R6, |3   |220  |R                |Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder|Resistor                                                                                       |
|RV1,        |1   |10K  |R_Potentiometer  |Potentiometer_THT:RK09K_Single_Vertical|Potentiometer                                                                                  |
|SW1,        |1   |RotaryEncoder|RotaryEncoder_Switch|Rotary_Encoder:RotaryEncoder_and_Switch_Vertical|Rotary encoder, dual channel, incremental quadrate outputs, with switch                        |
|U1,         |1   |MAX6675|MAX6675     |Package_SO:SOIC-8_3.9x4.9mm_P1.27mm|Cold Junction K-type Termocouple Interface, SPI, SO8                                           |
|U3,         |1   |LTD040BUE-101A|LTD040BUE-101A      |Display3bit:LTD040BUE-101A     |3 digit 7 segment super bright red LED, common anode                                           |

<img alt="Schematic" src="https://github.com/devincentiis/thermostatino/blob/main/schema.jpg?raw=true" width="100%">


<img alt="Thermostatino Bottom" src="https://github.com/devincentiis/thermostatino/blob/main/ThermostatinoBottom.jpg?raw=true" width="100%">

### Youtube video (old prototype)
This video is related to the [first version (prototype) of the "Thermostatino" PCB](https://youtu.be/hwenW8Aa0GM)

## License

MIT License

Copyright (c) 2023 - Antonio de Vincentiis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


