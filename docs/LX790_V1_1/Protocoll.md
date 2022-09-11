LX890 V1.1
==========

Display Cable
--------------

|                                   | |   |   |                        |
|---------------------------------:|-:|:-:|:-:|------------------------|
**3v3**                             | | 1 | 2 | **CLK** -> D14 #2 (brown)
**SW** *(I/O Btn)* -> Opto -> D27 #- (orange) | | 3 | 4 | **GND** -> GND #3 (black)
**5V** -> Vin #1 (red)              | | 5 | 6 | **DIO** *(Data)* -> D13 #4 (white)
**BUZ**                         |**[**| 7 | 8 | **GND**
**ST1** *(Stop)* --> **ST2**    |**[**| 9 | 10| **SDA** *(I2C)*
*(Stop)* -> D26 #5 (gray)           | | 11| 12| **STR** *(Start Btn)* -> D25 #6 (violet)
**OK** *(OK Btn)* -> D33 #7 (blue)  | | 13| 14| **SCL** *(I2C)*
**STB** *(CS)* -> D15 #8 (green)    | | 15| 16| **HOME** *(Home Btn)* -> D32 #9 (yellow)


Stop Button
-----------

 - JIABEN FA7-6K-2100 Trigger switch
 - 5V 1mA
 - DPNO Hall Signal


cable   | 1    | 2    | 3    | 4    
--------|:----:|:----:|:----:|:----:
color   | red  |yellow| blue |black 
signal  | 3v3  | ST1  | ST2  | GND  
release |      | 0V   | 0V   |
press   |      | 3v3  | 3v3  |



Power Off
---------

	SW: 20V
	all other: 0V

Power On
----------

	SW: 3V (released) / 0V (pressed)
	OK: 3V (released) / 0V (pressed)
	STR: 3V (released) / 0V (pressed)
	HOME: 3V (released) / 0V (pressed)
	BUZ: 0V / 3V @ 2.5kHz
	ST1 & ST2: 0V (released) / 3V (pressed)

Display Controller: TM1668 
--------------------------

**Display Protocol**

 - SPI mode: 0 / LSB
 - CLK: *Clock 100kHz*
 - DIO: *Data In/Out*
 - STB: *Chip Select*



**Segments**

	 -- 1 --
	|       |
	6       2
	|       |
	 -- 7 --
	|       |
	5       3
	|       |
	 -- 4 --
 
 - SEG1 0x01
 - SEG2 0x02
 - SEG3 0x04
 - SEG4 0x08
 - SEG5 0x10
 - SEG6 0x20
 - SEG7 0x40






**Examples**

*"-E6-"*

| Seq. | Raw | Command Description |
---|-----|--
 1.1 | 88 / 8E | Display Control: dimmed / bright
|
 2.1 | 03 | Mode Setting: 7 Grids, 11 Segments
|
 3.1 | 40 | Data Setting: write, auto increment, normal operation
 |
 4.1 | C0 | Address Setting: 0
 4.2 | 66 | 
 4.3 | 00 |
 4.4 | 60 |
 4.5 | 00 |
 4.6 | 64 |
 4.7 | 00 |
 4.8 | 26 |
 4.9 | 00 |
 4.10 | 26 |
 4.11 | 00 |
 4.12 | 26 |
 4.13 | 00 |
 4.14 | 0F |
 4.15 | 00 |

*"-E1-"*

| Seq. | Raw | Command Description |
---|-----|--
 1.1 | 8E | Display Control: bright
|
 2.1 | 03 | Mode Setting: 7 Grids, 11 Segments
|
 3.1 | 40 | Data Setting: write, auto increment, normal operation
 |
 4.1 | C0 | Address Setting: 0
 4.2 | 62 | 
 4.3 | 00 |
 4.4 | 64 |
 4.5 | 00 |
 4.6 | 64 |
 4.7 | 00 |
 4.8 | 22 |
 4.9 | 00 |
 4.10 | 22 |
 4.11 | 00 |
 4.12 | 22 |
 4.13 | 00 |
 4.14 | 0B |
 4.15 | 00 |

