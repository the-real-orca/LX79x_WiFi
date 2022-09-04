LX890 V1.1
==========

Cable
-----

|   |   |   |   |
----|---|---|----
3v3 | 1 | 2 | CLK
SW *(I/O Btn)* | 3 | 4 | GND
5V  | 5 | 6 | DIO *(Data)*
BUZ | 7 | 8 | GND
ST1 *(Stop)* | 9 | 10| SDA *(I2C)*
ST2 *(Stop)* | 11| 12| STR *(Start Btn)*
OK *(OK Btn)*| 13| 14| SCL *(I2C)*
STB | 15| 16| HOME *(Home Btn)*

OFF
----

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

TM1668 Display Controller
--------------------------

 - CLK: *Clock 100kHz*
 - DIO: *Data In/Out*
 - STB: *Chip Select* 

Dsiplay Protocoll

SPI mode: 0 / LSB

**-E6-**

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

**-E1-**

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

