#pragma once
/*
 * Hardware configuration vor LX790 Version 1.1 (2022)
 * Model: CL_M1
 *      16 pin display cable
 */

#include "config.h"
#if HW_MODEL == LX790_V1_1

#include "LX790_util.h"

//Hardware  

// SPI pins
#define HSPI_CS 15
#define HSPI_CLK 14
#define HSPI_MOSI 13
#define HSPI_MISO 12
#define VSPI_CS 5
#define VSPI_CLK 18
#define VSPI_MOSI 23
#define VSPI_MISO 19

// (mis-)use SPI in slave mode for TM1668 data communication
#define CS_PIN_DISPLAY  HSPI_CS		// Chip Select / STB for TM1668, low active
#define DIO_PIN_DISPLAY HSPI_MOSI	// data pin
#define CLK_PIN_DISPLAY HSPI_CLK	// sample on rising edge

#define DISPLAY_CMD_MASK     0xC0
#define DISPLAY_CMD_MODE_SET 0x00	// mode settings
#define DISPLAY_CMD_DATA_SET 0x40	// data settings
#define DISPLAY_CMD_CONTROL  0x80	// display control / brightness level
#define DISPLAY_CMD_ADDRESS  0xC0	// address read / write



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
#define SEG2 0x02
#define SEG3 0x04
#define SEG4 0x08
#define SEG5 0x10
#define SEG6 0x20
#define SEG7 0x40

//Buttons @TODO
#define BTN_PIN_IO    21
#define BTN_PIN_START 22
#define BTN_PIN_HOME  23
#define BTN_PIN_OK    24
#define BTN_PIN_STOP  25

#endif