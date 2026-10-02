# WioE5_IIS2DLPC_CW_v1

STM32CubeIDE project template for Seeed Wio-E5 mini with:
- IIS2DLPC on I2C2 (PA15 = SDA, PB15 = SCL)
- INT1 on PA9 / EXTI9 for any-motion wake-up
- USB-UART debug bridge on PB6/PB7 via the board bridge
- Direct SUBGHZ radio control for CW pulse pattern (no AT commands)

## Wiring

- IIS2DLPC SDA  -> PA15
- IIS2DLPC SCL  -> PB15
- IIS2DLPC INT1 -> PA9
- IIS2DLPC VDD  -> 3.3V
- IIS2DLPC GND  -> GND

Board pull-ups are already populated on some variants. If not, add one 4.7 kΩ pull-up from SDA to 3.3V and one from SCL to 3.3V.

## Operation

- Upon motion, INT1 triggers EXTI9.
- The ISR sets a flag.
- The main loop reads WAKE_UP_SRC and decodes X/Y/Z.
- The app then executes a CW pulse pattern of 600 ms ON / 1000 ms OFF for a total of 15 s.

## Notes

This project is set up as a proper HAL-based application skeleton. For the final RF stage, the exact `SUBGHZ` driver API depends on the STM32CubeWL version used by your project.
