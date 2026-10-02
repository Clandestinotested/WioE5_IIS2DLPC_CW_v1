# WioE5_IIS2DLPC_CW_v1

Complete STM32CubeIDE project template for Seeed Wio-E5 mini with:
- **IIS2DLPC** accelerometer on I2C2 (PA15 = SDA, PB15 = SCL)
- **INT1** on PA9 / EXTI9 for any-motion wake-up interrupt
- **USART1** debug on PB6 (TX) / PB7 (RX) via USB-UART bridge
- **SUBGHZ Direct Radio Control** for CW pulse pattern (600ms ON / 1000ms OFF / 15s total)
- **STM32CubeWL v1.5.0** compatible SUBGHZ driver integration

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
- **FIFO**: Disabled

### System Clock

- **HSE**: 32 MHz (on-board crystal)
- **SYSCLK**: 32 MHz (via PLL)
- **Peripherals**: HCLK = 32 MHz, PCLK1 = 32 MHz, PCLK2 = 32 MHz

## Software Architecture

### File Structure

```text
Core/
├── Inc/
│   ├── main.h              (main definitions, IIS2DLPC registers)
│   ├── app_motion.h        (motion sensor interface)
│   ├── app_cw.h            (CW pulse generator interface)
│   └── app_subghz.h        (SUBGHZ radio driver wrapper)
└── Src/
    ├── main.c              (system initialization, main loop)
    ├── app_motion.c        (IIS2DLPC any-motion handling)
    ├── app_cw.c            (CW pulse state machine)
    └── app_subghz.c        (STM32CubeWL v1.5.0 radio wrapper)
```

### Main Application (`Core/Src/main.c`)

- Initialization of HAL, clock tree (32 MHz), GPIO, I2C2, USART1
- Main loop calls `App_Motion_Task()` and `App_CW_Task()` every 10 ms
- UART debug output (115200 baud)
- Error handler for system faults

### Motion Sensor (`Core/Src/app_motion.c`)

- Initializes IIS2DLPC with any-motion wake-up enabled
- 50 Hz ODR, wake-up threshold ~0.08g (5 LSB)
- INT1 routed to MD1_CFG for wake-up detection
- On motion interrupt, reads WAKE_UP_SRC and decodes X/Y/Z axis
- Triggers CW pulse sequence when motion detected

### CW Transmitter (`Core/Src/app_cw.c`)

- 15-second pulse sequence: 600 ms ON, 1000 ms OFF
- State machine with HAL_GetTick() timing
- Calls `SubGHz_TX_Start_CW()` for TX enable
- Calls `SubGHz_TX_Stop()` for TX disable
- Debug output logs ON/OFF transitions with timestamps

### SUBGHZ Radio Driver Wrapper (`Core/Src/app_subghz.c`)

**STM32CubeWL v1.5.0 Native API Integration**

This module wraps the STM32CubeWL radio driver and provides:

#### Initialization
```c
void SubGHz_Init(void)
{
    /* Initialize RadioEvents callbacks */
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    /* Initialize radio driver */
    Radio.Init(&RadioEvents);

    /* Set frequency and TX config */
    Radio.SetChannel(868000000UL);        /* 868 MHz (Europe) */
    Radio.SetTxConfig(MODEM_FSK, 14, ...) /* 14 dBm, FSK modulation */
}
```

#### Continuous Wave Transmission
```c
void SubGHz_TX_Start_CW(void)
{
    /* Start transmit continuous wave mode */
    Radio.SetTxContinuousWave(868000000UL, 14, 0xFFFFFFFF);
}

void SubGHz_TX_Stop(void)
{
    /* Stop TX and return to STDBY */
    Radio.IrqProcess();
    Radio.Standby();
}
```

#### Frequency/Power Configuration
```c
void SubGHz_SetFrequency(uint32_t freq_hz)
void SubGHz_SetTxPower(int8_t power_dbm)
```

### Radio Configuration Defaults

```c
#define SUBGHZ_FREQ_868MHZ         868000000UL   /* Europe */
#define SUBGHZ_FREQ_915MHZ         915000000UL   /* USA/Australia */
#define SUBGHZ_TX_POWER            14            /* 14 dBm */
#define SUBGHZ_BANDWIDTH           125000        /* 125 kHz */
#define SUBGHZ_DATARATE            4800          /* 4.8 kbps */
#define SUBGHZ_FDEV                4800          /* 4.8 kHz deviation */
```

## Integration with STM32CubeWL v1.5.0

### Required Driver Files

Your STM32CubeWL package should include:

```text
Drivers/STM32WLE5xx_HAL_Driver/Inc/
├── radio.h
├── subghz_phy.h
└── ... (other HAL headers)

Drivers/STM32WLE5xx_HAL_Driver/Src/
├── radio.c
├── subghz_phy.c
└── ... (other HAL implementations)
```

### Include Paths in STM32CubeIDE

1. **Project → Properties → C/C++ General → Paths and Symbols → Includes**
   ```
   Drivers/STM32WLE5xx_HAL_Driver/Inc
   Drivers/CMSIS/Include
   Core/Inc
   ```

2. **Symbols (Preprocessor)**
   ```
   STM32WLE5xx
   USE_HAL_DRIVER
   ```

### Linker Script

Ensure your linker script (`STM32WLE5JCIx_FLASH.ld`) includes:
- Flash memory layout for STM32WLE5JC
- RAM allocation for HAL structures
- Radio mailbox area (typically at fixed memory location)

## Radio API Reference (STM32CubeWL v1.5.0)

### struct RadioEvents_t

```c
typedef struct
{
    void ( *TxDone )( void );
    void ( *RxDone )( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );
    void ( *TxTimeout )( void );
    void ( *RxTimeout )( void );
    void ( *RxError )( void );
} RadioEvents_t;
```

### Radio.Init()

```c
void Radio.Init(RadioEvents_t *events)
```
Initialize radio driver with callback structure.

### Radio.SetChannel()

```c
void Radio.SetChannel(uint32_t freq)
```
Set operating frequency in Hz (e.g., 868000000 for 868 MHz).

### Radio.SetTxConfig()

```c
void Radio.SetTxConfig(
    RadioModems_t modem,      /* MODEM_FSK or MODEM_LORA */
    int8_t power,              /* TX power in dBm */
    uint32_t fdev,             /* FSK frequency deviation Hz */
    uint32_t bandwidth,        /* FSK bandwidth Hz */
    uint32_t datarate,         /* Datarate in bits/sec */
    uint8_t coderate,          /* LoRa coding rate (ignored for FSK) */
    uint16_t preambleLen,      /* Preamble length */
    bool fixLen,               /* Fixed or variable length */
    bool crcOn,                /* CRC enable */
    bool freqHopOn,            /* Frequency hopping enable */
    uint8_t hopPeriod,         /* Hopping period */
    bool iqInverted,           /* IQ inversion */
    uint32_t timeout           /* TX timeout in ms */
)
```

### Radio.SetTxContinuousWave()

```c
void Radio.SetTxContinuousWave(
    uint32_t freq,             /* Frequency in Hz */
    int8_t power,              /* TX power in dBm */
    uint16_t timeout           /* Duration in ms (0xFFFFFFFF = indefinite) */
)
```
**This is the key function for CW mode.**

### Radio.Standby()

```c
void Radio.Standby(void)
```
Put radio into standby (low power, no TX/RX).

### Radio.IrqProcess()

```c
void Radio.IrqProcess(void)
```
Process pending radio interrupts.

## Operation Flow

```
1. System boots → UART debug output
2. IIS2DLPC initialized (WHO_AM_I check, wake-up config)
3. SUBGHZ initialized (Radio.Init, frequency set to 868 MHz)
4. Idle, waiting for motion on INT1 (PA9)

5. Motion detected → INT1 rises → EXTI9_5_IRQHandler()
   ↓
6. HAL_GPIO_EXTI_Callback(GPIO_PIN_9) → motion_event_pending = 1
   ↓
7. App_Motion_Task() reads WAKE_UP_SRC
   ↓
8. Motion axis decoded (X/Y/Z) → UART debug output
   ↓
9. App_CW_Start() → SubGHz_TX_Start_CW()
   ↓
10. 15-second pulse loop:
    - 600 ms ON  → Radio.SetTxContinuousWave(...)
    - 1000 ms OFF → Radio.Standby()
    - State transitions logged to UART
   ↓
11. After 15s → App_CW_Stop() → Radio.Standby()
    ↓
12. Return to idle, wait for next motion
```

## Debug Output Example

```
=== Wio-E5 Mini IIS2DLPC + CW System (STM32CubeWL v1.5.0) ===
Initializing motion sensor...
IIS2DLPC detected (WHO_AM_I = 0x44)
  CTRL1 set to ODR_50Hz
  WAKE_UP_THS configured
  WAKE_UP_DUR configured
  MD1_CFG configured (INT1=wake-up)
IIS2DLPC any-motion setup complete
Initializing SUBGHZ radio...
SUBGHZ initialized (freq=868000000 Hz, power=14 dBm)

System ready
Waiting for motion on PA9 INT1...

MOTION DETECTED: axis mask=0x07 (X=1 Y=1 Z=1)
CW sequence started (15s total: 600ms ON / 1000ms OFF)
  [SUBGHZ] TX CW started (freq=868000000 Hz, power=14 dBm)
  [00000ms] CW ON
  [SUBGHZ] TX CW stopped (radio in STDBY)
  [00601ms] CW OFF
  [SUBGHZ] TX CW started (freq=868000000 Hz, power=14 dBm)
  [01601ms] CW ON
  [SUBGHZ] TX CW stopped (radio in STDBY)
  [02201ms] CW OFF
  ...
  [14401ms] CW OFF
CW sequence completed (elapsed=15000ms)
CW sequence stopped
```

## Compilation & Flashing

### Build

1. **Open in STM32CubeIDE**
   ```bash
   git clone https://github.com/Clandestinotested/WioE5_IIS2DLPC_CW_v1.git
   cd WioE5_IIS2DLPC_CW_v1
   ```

2. **Add STM32CubeWL Drivers**
   - Copy HAL and SUBGHZ drivers from your STM32CubeWL v1.5.0 package
   - Place in `Drivers/STM32WLE5xx_HAL_Driver/`

3. **Build in STM32CubeIDE**
   ```
   Project → Build Project
   ```

### Flash

**Via ST-Link/V2 and STM32CubeProgrammer:**

```bash
STM32_Programmer_CLI -c port=SWD freq=4M -w build/WioE5_IIS2DLPC_CW.elf -v
```

**Via OpenOCD:**

```bash
openocd -f interface/stlink-v2.cfg -f target/stm32wl.cfg \
    -c "program build/WioE5_IIS2DLPC_CW.elf verify reset exit"
```

## Troubleshooting

### IIS2DLPC Not Detected
- Verify PA15 (SDA) and PB15 (SCL) connections
- Check 4.7 kΩ pull-up resistors on both lines
- Measure I2C bus with oscilloscope (clock and data signals)

### No UART Output
- Check USB connection to Wio-E5 Mini (should appear as COM port)
- Verify baud rate: 115200
- Ensure PB6 (TX) and PB7 (RX) are not used by other peripherals

### Motion Not Triggering
- Verify PA9 connected to INT1 on sensor
- Confirm EXTI9_5_IRQn is enabled in NVIC
- Increase wake-up threshold if over-sensitive (WAKE_UP_THS register)
- Verify sensor has 3.3V power

### SUBGHZ Radio Not Starting
- Confirm `radio.h` and `subghz_phy.h` are in include path
- Verify linker includes `radio.c` and `subghz_phy.c` from STM32CubeWL
- Check that radio mailbox memory is correctly defined in linker script
- Verify RF PA (power amplifier) is enabled and configured
- Confirm antenna is connected and impedance matched

## Regulatory Notes

- **Europe (CE)**: 868 MHz ISM band, max 14 dBm EIRP
- **USA (FCC)**: 915 MHz ISM band, max 30 dBm EIRP
- **Australia (ACMA)**: 915-928 MHz, regulations apply

Adjust frequency and power in `app_subghz.h` according to your region.

## Next Steps

1. Verify all hardware connections match the pin configuration
2. Integrate STM32CubeWL v1.5.0 drivers from your package
3. Compile and flash the project
4. Open serial terminal at 115200 baud
5. Trigger motion on the sensor → observe CW pulse sequence

## License

Open source. Modify and use freely for your application.
