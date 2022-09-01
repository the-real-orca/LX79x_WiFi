#pragma once
/*
 * Hardware configuration vor LX790 Version 1.0
 */

#include "config.h"
#if HW_MODEL == LX790_V1_0

#include "LX790_util.h"


//Hardware  
#define SDA_PIN_MAINBOARD    33  /*default 21*/
#define SCL_PIN_MAINBOARD    25  /*default 22*/
#define SDA_PIN_DISPLAY      26
#define SCL_PIN_DISPLAY      27
#define I2C_SLAVE_ADDR        0x27
#define I2C_DISPLAY_ADDR      0x27
#define OUT_IO               13

//I2C commands
#define TYPE_BUTTONS          0x01
#define LEN_BUTTONS_RES       9
#define TYPE_DISPLAY          0x02
#define LEN_BUTTONS_REQ       4
#define LEN_DISPLAY_RES       9
#define TYPE_UNKNOWN          0x04
#define LEN_UNKNOWN_REQ       4
#define TYPE_UNKNOWN_INIT     0x05
#define LEN_UNKNOWN_INIT_REQ  5
#define LEN_MAINBOARD_MAX     9
#define LEN_CMDQUE           10

// Display
/*
 * segments:
 *
 *   -- 1 --
 *  |       |
 *  6       2
 *  |       |
 *   -- 7 --
 *  |       |
 *  5       3
 *  |       |
 *   -- 4 --
 */
#define SEG1 0x01
#define SEG2 0x04
#define SEG3 0x20
#define SEG4 0x40
#define SEG5 0x10
#define SEG6 0x02
#define SEG7 0x08

//Buttons
#define BTN_BYTE1_OK          0x01
#define BTN_BYTE1_START       0x02
#define BTN_BYTE1_HOME        0x04
#define BTN_BYTE2_STOP        0xFC

#endif

//  Landxcape LX790
//  I2C Reverse engineering
//  
//  ###### Display
//  
//  D  Z1 Z2 Z3 Z4 SY BR CS CS
//  
//  02 08 5B 24 08 E0 01 D0 CE  -> E1 gedimmt
//  02 08 5B 24 08 E0 C9 94 96  -> E1 hell
//  
//  02 08 5B 7B 08 E0 01 05 6F  -> E6 gedimmt
//  02 08 5B 7B 08 E0 C9 41 36  -> E6 hell
//  
//  D:  	Typ
//  
//  Z1-Z4:	Zahl 1-4
//
//        0x01
//         _
//  0x02 |   | 0x04
//         -   0x08
//  0x10 | _ | 0x20
//  
//        0x40
//  
//  SY:	Symbole
//  
//    Schloss    0x08
//    Uhr        0x04
//    Punkte     0x01 und 0x02
//    WiFi       0x10
//
//    Batterie Gehäuse   0x20
//    Batterie Str. re   0x60
//    Batterie Str. mi   0xE0
//  
//  BR:    Helligkeit (0x01 bis 0xC8 / 1 - 200)
//    Batterie Str. li   0x01
//
//  CS: 2 Byte Checksumme
//  
//  ###### Taster
//  
//  Master / write -> 01 01 E0 C1
//  Master / read  <- 01 01 78 00 00 00 00 5B EC	<- Taster "ok"
//  Master / read  <- 01 04 78 00 00 00 00 5A AF	<- Taster "Home / runter"
//  Master / read  <- 01 02 78 00 00 00 00 BB 22	<- Taster "Start / hoch"
//  Master / read  <- 01 00 78 00 00 00 00 FB A9	<- Taster "Power"             0x78: 01111000
//  Master / read  <- 01 00 FC 00 00 00 00 2D 02	<- Taster "Stop" (Öffner!)    0xFC: 11111100
//  
//  ###### Unbekannt
//  
//  Master / write -> 04 01 15 3E
//    -> vermutlich eine Option wie z.B. Ultraschall, welche das Mainboard auf vorhandensein abfragt..?
//  
//  ###### Unbekannt
//  
//  Master / write -> 05 01 01 83 fb
//    -> wird nach dem Einschalten ans Display geschickt.. dann nie mehr
//  
//  ###### Checksumme
//
//  https://crccalc.com/
//  CRC-16/GENIBUS
//
//  ###### Notizen
//  
//  Adresse
//  27	00100111
//  Adresse + R/W
//  4E	 01001110 write
//  4F	 01001111
//  
//  Display Beispiele
//  
//  08  0000 1000	-
//  5b	0101 1011	E
//  24  0010 0100	1
//  08	0000 1000	-
//  3F	0011 1111	A
//  7B	0111 1011	6
//  5b	0101 1011	E
//  5D	0101 1101	2
