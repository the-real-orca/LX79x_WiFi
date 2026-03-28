#pragma once
#include "config.h"
#include "LX790_util.h"

/**
 * @brief Performs hardware-specific setup for the selected LX790 model.
 * Initializes I2C/SPI interfaces, GPIO pins, and drivers.
 */
void HAL_setup();

/**
 * @brief Main hardware loop for communication and state updates.
 * Performs I2C/SPI transactions to read display/button status, 
 * updates the display, and sends button commands.
 * 
 * @param state Reference to LX790_State structure to be updated with:
 *   - segments: 7-segment raw pattern for each digit
 *   - point: Decimal point or colon character
 *   - clock: Clock icon visibility
 *   - lock: Lock icon visibility
 *   - battery: Battery level (0 - 3)
 *   - brightness: Display brightness level (0 - 15)
 */
void HAL_loop(LX790_State &state);

/**
 * @brief Simulates a button press on the mower.
 * @param btn The button to press (BTN_IO, BTN_START, BTN_HOME, BTN_OK, BTN_STOP).
 */
void HAL_buttonPress(BUTTONS btn);

/**
 * @brief Simulates a button release on the mower.
 * @param btn The button to release.
 */
void HAL_buttonRelease(BUTTONS btn);

#if HW_MODEL == LX790_V1_0
  #include "HAL_LX790_V1_0.h"
#elif HW_MODEL == LX790_V1_1
  #include "HAL_LX790_V1_1.h"
#else
  #error "specify your model in config.h"
#endif
