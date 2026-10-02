# WioE5_IIS2DLPC_CW_v1

STM32CubeIDE project for Seeed Wio-E5 mini with:
- IIS2DLPC accelerometer on I2C2 (PA15 SDA, PB15 SCL)
- INT1 on PA9 / EXTI9 for any-motion wake-up
- USB-UART debug bridge on PB6/PB7 via ST-LINK/USB-UART converter
- Direct SUBGHZ radio control for CW pulses, no AT commands

This repository contains the code skeleton and project structure for the motion sensor + CW pulse workflow.

## Hardware wiring

- IIS2DLPC SDA  -> PA15
- IIS2DLPC SCL  -> PB15
- IIS2DLPC INT1 -> PA9
- Sensor VDD    -> 3.3V
- Sensor GND    -> GND
- 4.7k pull-ups are already present on the board if populated; otherwise add one on SDA and one on SCL to 3.3V

## Notes

The generated code targets the STM32WLE5JC on the Wio-E5 mini and is intended as a starting point for:
- motion detection and axis wake-up decoding
- 15 s CW pulse burst control
- UART debug output via the onboard USB-UART bridge
