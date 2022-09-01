#pragma once
/*
 * Hardware configuration vor LX790 Version 1.1 (2022)
 */

#include "config.h"
#if HW_MODEL == LX790_V1_1

#include "LX790_util.h"

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

//Buttons
// @TODO Button Pins

void decodeTM1668(const uint8_t raw[7], LX790_State &out);

#endif