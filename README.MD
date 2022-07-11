 # Jaika★'s Taiko Drum Interface (BlueSquidInterface)

 A feature-rich alternative to the traditional boxes seen with replica Taiko arcade drums, using Arduino as the basis for easy accessibility of parts.

## Features/Design goals
- Compatiability with all piezo-based drum controllers
- Tuned for accuracy with fast, repetitive hits while minimizing misread inputs
- Designed using off-the-shelf parts commonly found at brick-and-mortar electronics stores
- Built-in display and menu for configuring characteristics of the device
- Sensitivity adjustment for each sensor
- Adjustable delay between hits to adjust for response times of different simulators and machines
- In-built diagnostics menu with sensor readout when above sensitivity threshold
## Hardware Required
 _Each point is linked to the hardware I used (Most for Australian shops)_
- [32U4-based Arduino (tested on Leonardo)](https://store.arduino.cc/products/arduino-leonardo-with-headers)
- [SH1106-based 128x64 monochrome OLED display](https://www.jaycar.com.au/duinotech-1-3-inch-monochrome-oled-display/p/XC3728?pos=1&queryId=0f307bbf4cca92e764d02f5813176d98)
- [Digital rotary encoder module](https://www.jaycar.com.au/digital-rotation-sensor-for-arduino/p/XC3736)
- [10kR resistors x4](https://www.jaycar.com.au/10k-ohm-0-5-watt-metal-film-resistors-pack-of-8/p/RR0596)
- [100-200nF capacitor x2](https://www.jaycar.com.au/100nf-50vdc-ceramic-capacitors-pack-of-2/p/RC5360)

## Hookup 
### Rotary encoder
_NOTE: Remember to add a 100-200nF capacitor on both the CLK and DT pins of the encoder to ground. This will help prevent abnormal stuttering._
 - CLK - Pin 3
 - DT  - Pin 2
 - SW  - Pin 4
### OLED Display
- CLK - Pin 13
- DATA/MOS - Pin 11
- DC - Pin 9
- CS - Pin 10
- RESET - Pin 12
### Drum Sensors
_NOTE: Remember to add a 10kR resistor between the 2 leads of every sensor module to prevent voltage spikes from damaging your board. This also ensures that the default sensitivity configuration is likely to be reference-perfect._
 * LEFTKA Positive lead - Pin A0
 * LEFTDON Positive lead - Pin A1
 * RIGHTDON Positive lead - Pin A2
 * RIGHTKA Positive lead - Pin A3

All negative leads go to ground.