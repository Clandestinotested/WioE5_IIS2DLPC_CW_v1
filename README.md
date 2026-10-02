# WioE5_IIS2DLPC_CW_v1

Complete STM32CubeIDE project template for Seeed Wio-E5 mini with:
- **IIS2DLPC** accelerometer on I2C2 (PA15 = SDA, PB15 = SCL)
- **INT1** on PA9 / EXTI9 for any-motion wake-up interrupt
- **USART1** debug on PB6 (TX) / PB7 (RX) via USB-UART bridge
- **Direct SUBGHZ** radio control for CW pulse pattern (600ms ON / 1000ms OFF / 15s total)

## Hardware Setup

### Pin Configuration

```text
IIS2DLPC Connections:
  SDA  → PA15 (I2C2_SDA, AF4)
  SCL  → PB15 (I2C2_SCL, AF4)
  INT1 → PA9  (EXTI9, Rising Edge)
  VDD  → 3.3V
  GND  → GND

USART1 Debug (USB-UART Bridge):
  TX   → PB6 (USART1_TX, AF7)
  RX   → PB7 (USART1_RX, AF7)
  Baud → 115200

Pull-ups:
  Board may have 4.7kΩ pull-ups already populated on PA15/PB15.
  If not present, add external 4.7kΩ pull-ups from SDA→3.3V and SCL→3.3V.
```

### I2C2 Configuration

- **Speed**: 100 kHz (standard mode)
- **Timing**: 0x00707CBBU (for 32 MHz clock)
- **Address Mode**: 7-bit
- **Analog Filter**: Enabled
- **Digital Filter**: Disabled

### USART1 Configuration

- **Baud Rate**: 115200
- **Data Bits**: 8
- **Stop Bits**: 1
- **Parity**: None
- **Flow Control**: None

## Software Architecture

### Main Application (`Core/Src/main.c`)

- Initialization of HAL, clock tree, GPIO, I2C2, USART1
- Main loop calls `App_Motion_Task()` and `App_CW_Task()` every 10 ms
- UART debug output (115200 baud)

### Motion Sensor (`Core/Src/app_motion.c`)

- Initializes IIS2DLPC with any-motion wake-up enabled
- 50 Hz ODR, wake-up threshold ~0.08g (5 LSB)
- INT1 routed to MD1_CFG for wake-up detection
- On motion interrupt, reads WAKE_UP_SRC and decodes X/Y/Z axis
- Triggers CW pulse sequence when motion detected

### CW Transmitter (`Core/Src/app_cw.c`)

- 15-second pulse sequence: 600 ms ON, 1000 ms OFF
- State machine with HAL_GetTick() timing
- Placeholder functions for actual SUBGHZ driver integration:
  - `SubGHz_TX_Start_CW()` — Enable radio TX (replace with STM32WLE5 driver call)
  - `SubGHz_TX_Stop_CW()` — Disable radio TX (replace with STM32WLE5 driver call)
- Debug output logs ON/OFF transitions

## SUBGHZ Integration

The CW transmitter includes two placeholder functions in `app_cw.c`:

```c
static void SubGHz_TX_Start_CW(void)
{
    /* TODO: Replace with actual SUBGHZ driver call.
     *
     * Example (pseudo-code):
     *   SubGHz_TX_SetFrequency(868000000);  // 868 MHz
     *   SubGHz_TX_SetPower(14);             // 14 dBm
     *   SubGHz_TX_StartContinuousWave();
     */
}

static void SubGHz_TX_Stop_CW(void)
{
    /* TODO: Replace with actual SUBGHZ driver call.
     *
     * Example (pseudo-code):
     *   SubGHz_TX_Stop();
     */
}
```

### Integrating Your SUBGHZ Driver

1. Include your SUBGHZ driver header in `app_cw.c`:
   ```c
   #include "subghz_driver.h"  // or whatever your driver is called
   ```

2. Replace the placeholder functions with actual driver calls:
   ```c
   static void SubGHz_TX_Start_CW(void)
   {
       SUBGHZ_Init();
       SUBGHZ_SetFrequency(868000000);
       SUBGHZ_SetPower(14);
       SUBGHZ_StartTxCW();
   }
   ```

3. Recompile and test.

## Operation Flow

```
1. System boots → UART debug output
2. IIS2DLPC initialized (WHO_AM_I check, wake-up config)
3. Idle waiting for motion on INT1 (PA9)
4. Motion detected → INT1 rises → EXTI9_5_IRQHandler() → App_Motion_Task() reads WAKE_UP_SRC
5. Motion axis decoded (X/Y/Z) → UART debug output
6. App_CW_Start() triggered → Radio TX starts CW pulse
7. 15-second pulse loop (600ms ON / 1000ms OFF) with state transitions logged to UART
8. After 15s → CW stops, return to idle
9. Next motion triggers another sequence
```

## Debug Output Example

```
=== Wio-E5 Mini IIS2DLPC + CW System ===
Initializing motion sensor...
IIS2DLPC detected (WHO_AM_I = 0x44)
  CTRL1 set to ODR_50Hz
  WAKE_UP_THS configured
  WAKE_UP_DUR configured
  MD1_CFG configured (INT1=wake-up)
IIS2DLPC any-motion setup complete
CW transmitter ready (motion-triggered)
Waiting for motion on PA9 INT1...

MOTION DETECTED: axis mask=0x07 (X=1 Y=1 Z=1)
CW sequence started (15s total: 600ms ON / 1000ms OFF)
  [SUBGHZ] TX CW START
  [00000ms] CW ON
  [00601ms] CW OFF
  [01601ms] CW ON
  [02201ms] CW OFF
  ...
  [14401ms] CW OFF
CW sequence completed (elapsed=15000ms)
  [SUBGHZ] TX CW STOP
```

## Compilation & Flashing

1. **STM32CubeIDE Project Structure**
   - `Core/Inc/` — Header files
   - `Core/Src/` — Implementation files
   - `Drivers/STM32WLE5xx_HAL_Driver/` — HAL library (generated or external)
   - `.project`, `.cproject` — IDE configuration files

2. **Build**
   ```bash
   cd /path/to/project
   make clean
   make all
   ```

3. **Flash (via OpenOCD or STM32CubeProgrammer)**
   ```bash
   openocd -f stm32wle5.cfg -c "program build/WioE5_IIS2DLPC_CW.elf verify reset exit"
   ```

## Troubleshooting

### IIS2DLPC Not Detected (WHO_AM_I fails)
- Check I2C wiring: PA15 (SDA), PB15 (SCL)
- Verify pull-up resistors (4.7 kΩ) on both lines
- Check I2C speed: 100 kHz timing is configured to 0x00707CBBU
- Measure I2C bus with oscilloscope to confirm clock/data signals

### No UART Output
- Check USB connection (should appear as COM port)
- Verify baud rate: 115200
- Check PB6 (TX) and PB7 (RX) connections
- Ensure USART1 is enabled in system clock config

### Motion Not Triggering
- Verify PA9 connected to INT1 on sensor
- Check EXTI9_5_IRQn priority and enabled status
- Increase wake-up threshold if sensor is too sensitive (WAKE_UP_THS register)
- Confirm sensor power supply (3.3V)

### CW Not Transmitting
- Implement actual SUBGHZ driver calls in `SubGHz_TX_Start_CW()` and `SubGHz_TX_Stop_CW()`
- Verify radio PA is enabled and configured
- Check frequency and power settings match your region regulations
- Confirm RF front-end (antenna, impedance matching) is correct

## Notes

- This project is a **complete template** ready for integration with your STM32WLE5 SUBGHZ driver.
- All HAL calls are standard and compatible with STM32CubeMX-generated code.
- Clock tree example assumes 32 MHz HSE; adjust `SystemClock_Config()` if different.
- Replace SUBGHZ placeholder functions with your actual driver API.

## License

Open source. Modify as needed for your application.
