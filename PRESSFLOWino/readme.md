# PRESSFLOWino - Multimode, customizable press and flow controller
## for air compressor, water pump and other device

Base on Atmel ATmega328P 32 pins and compiled trought Arduino IDE and ADVcapacitiveSensor library.
The circuit is shared for the purpose of making you participate in the improvement of the firmware; for safety reasons and $\textsf{\color{orange}{to avoid risks of electric shock it is essential to power it at 48 Vac}}$ for the Triacs and at 5 Vdc for the digital section. 
Therefore, **anyone attempting to implement it in practice must be qualified and assumes all civil and criminal liability.**

**DECLARATION OF RESPONSIBILITY**

**The author therefore assumes NO RESPONSIBILITY** in civil or criminal matters and does not offer any implicit or explicit guarantee, and is not responsible for problems of any kind, damages or accidents deriving from the use of the information found on this web page , as well as from the use of the page itself or part of it. **Anyone who creates and/or uses modifications, diagrams, electrical circuits**, or mechanical parts or electronic theory information described here, **does so as their own free choice, assuming all responsibilities that may derive from them**, and will therefore not be able to claim any form of compensation from the author, in case of damage or accidents for everything described and said. In light of the above and **in consideration of the danger of electrocution deriving from the connection with voltages highter than 48V ac**, the description on this project has been deliberately omitted and **remains the responsibility of qualified personnel in the electrical/electronic sector**.


### Use instructions
By touch the 4 buttons (ENTER, UP, DOWN, BACK) you can select the parameters for the target device of the control.
* Select the type of device to control: pump or compressor.
* Select the pressure sensor or customize it.
* Select flow sensor as switch, none, preset or customize it.

### Schematic and PCB EDA [KiCad](https://www.kicad.org/download/) 

### Firmware library for [Arduino IDE](https://github.com/arduino/Arduino) 

[<img alt="PRESSFLOWino Front" src="https://github.com/devincentiis/electronics/blob/main/PRESSFLOWino/PRESSFLOWinoFront.jpg?raw=true" width="100%">](https://github.com/devincentiis/electronics/blob/main/PRESSFLOWino/PRESSFLOWinoFront.jpg)


| Reference        | Value        | Footprint                                                      | Qty |
|------------------|--------------|----------------------------------------------------------------|-----|
| BZ1              | Buzzer       | Buzzer_Beeper:Buzzer_12x9.5RM7.6                               | 1   |
| C1,C6,C7,C10,C13 | 100n         | Capacitor_SMD:C_1206_3216Metric_Pad1.33x1.80mm_HandSolder      | 5   |
| C2,C4            | 100n 400V    | Capacitor_THT:C_Rect_L19.0mm_W6.0mm_P15.00mm_MKS4              | 2   |
| C3               | 10n 400V     | Capacitor_THT:C_Rect_L13.0mm_W5.0mm_P10.00mm_FKS3_FKP3_MKS4    | 1   |
| C5               | 470u         | Capacitor_THT:CP_Radial_D12.5mm_P5.00mm                        | 1   |
| C8               | 10u          | Capacitor_THT:CP_Radial_D5.0mm_P2.50mm                         | 1   |
| C9               | 10nF         | Capacitor_THT:C_Rect_L13.0mm_W5.0mm_P10.00mm_FKS3_FKP3_MKS4    | 1   |
| D1               | LED-ORANGE   | LED_SMD:LED_1206_3216Metric_Pad1.42x1.75mm_HandSolder          | 1   |
| D2,D7            | BT137-600    | Package_TO_SOT_THT:TO-220-3_Vertical                           | 2   |
| D4               | LED-GREEN    | LED_SMD:LED_1206_3216Metric_Pad1.42x1.75mm_HandSolder          | 1   |
| D5,D8            | M4-1N4004    | Diode_SMD:D_SMA                                                | 2   |
| DS1              | 16x02        | Display:WC1602A                                                | 1   |
| F1               | FUSE         | Fuse_THT:Rect_L7.2mm_W4.5mm_P5.00mm                            | 1   |
| H1               | BCK          | MountingHole:MountingHole_3.2mm_M3_Pad_TopBottom               | 1   |
| H2               | ENT          | MountingHole:MountingHole_3.2mm_M3_Pad_TopBottom               | 1   |
| H3               | UP           | MountingHole:MountingHole_3.2mm_M3_Pad_TopBottom               | 1   |
| H4               | DWN          | MountingHole:MountingHole_3.2mm_M3_Pad_TopBottom               | 1   |
| IC1,IC2          | TLP266J      | Package_SO:SO-4_4.4x4.3mm_P2.54mm                              | 2   |
| J1               | ICSP         | Connector_PinHeader_2.54mm:PinHeader_2x03_P2.54mm_Vertical     | 1   |
| J2               | FTDI         | Connector_PinSocket_2.54mm:PinSocket_1x06_P2.54mm_Vertical     | 1   |
| P1               | E_VALVE      | ADVlibrary:Faston6.35-2x7.62                                   | 1   |
| P2               | V ac         | ADVlibrary:Faston6.35-2x7.62                                   | 1   |
| P3               | MOTOR        | ADVlibrary:Faston6.35-2x7.62                                   | 1   |
| P4               | GROUND       | ADVlibrary:Faston6.35-2x7.62                                   | 1   |
| R1,R2,R3,R4      | 4.7M         | Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder       | 4   |
| R5,R6,R7,R8,R19  | 1000         | Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder       | 5   |
| R9               | 470          | Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder       | 1   |
| R10,R16          | 330          | Resistor_SMD:R_2512_6332Metric_Pad1.40x3.35mm_HandSolder       | 2   |
| R11,R17          | 390          | Resistor_SMD:R_2512_6332Metric_Pad1.40x3.35mm_HandSolder       | 2   |
| R12,R18          | 39           | Resistor_SMD:R_2512_6332Metric_Pad1.40x3.35mm_HandSolder       | 2   |
| R14,R15          | 120          | Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder       | 2   |
| R20              | 10K          | Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder       | 1   |
| R21              | 20K          | Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder       | 1   |
| RV1              | 5K           | Potentiometer_THT:Potentiometer_Bourns_3266Y_Vertical          | 1   |
| U1               | ATmega328P   | Package_QFP:TQFP-32_7x7mm_P0.8mm                               | 1   |
| U2               | FLOW_SENS    | Connector_PinSocket_2.54mm:PinSocket_1x03_P2.54mm_Vertical     | 1   |
| U3               | PRESS_SENS   | Connector_PinSocket_2.54mm:PinSocket_1x03_P2.54mm_Vertical     | 1   |
| U4               | L7805        | Package_TO_SOT_SMD:SOT-89-3                                    | 1   |

<img alt="Schematic" src="https://github.com/devincentiis/electronics/blob/main/PRESSFLOWino/schematic.jpg?raw=true" width="100%">

<img alt="PRESSFLOWino Back" src="https://github.com/devincentiis/electronics/blob/main/PRESSFLOWino/PRESSFLOWinoBack.jpg?raw=true" width="100%">

### Video (old prototype)
[![PRESSFLOWino for Arduino Nano](http://img.youtube.com/vi/hwenW8Aa0GM/0.jpg)](http://www.youtube.com/watch?v=hwenW8Aa0GM "PRESSFLOWino for Arduino Nano")

## License

MIT License

Copyright (c) 2025 to present - Antonio de Vincentiis

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


