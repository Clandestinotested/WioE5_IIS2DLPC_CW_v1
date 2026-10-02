# WioE5_IIS2DLPC_CW_v1

Complete STM32CubeWL v1.5.0 project for Seeed Wio-E5 mini with:
- **IIS2DLPC** accelerometer on I2C2 (PA15 = SDA, PB15 = SCL)
- **INT1** on PA9 / EXTI9 for any-motion wake-up interrupt
- **USART1** debug on PB6 (TX) / PB7 (RX) via USB-UART bridge
- **STM32CubeWL v1.5.0 Radio API** for direct CW pulse transmission
- **600ms ON / 1000ms OFF / 15s total** pulse pattern

## Quick Start

### 1. Hardware Wiring

```text
IIS2DLPC → Wio-E5 Mini:
  SDA (pin 2) → PA15 (I2C2_SDA)
  SCL (pin 3) → PB15 (I2C2_SCL)
  INT1 (pin 4) → PA9 (EXTI9)
  VDD (pin 1) → 3.3V
  GND (pin 5) → GND
  
USB-UART (via board bridge):
  PB6 (TX) → USB bridge
  PB7 (RX) → USB bridge
  Baud: 115200
```

### 2. Software Integration

1. **Clone/download this repository**
2. **Copy STM32CubeWL v1.5.0 radio driver:**
   ```bash
   cp -r STM32Cube_FW_WL_V1.5.0/Middlewares/Third_Party/SubGHz_Phy/radio_driver/* \
       YourProject/Middlewares/Third_Party/SubGHz_Phy/
   ```

3. **Add include path in STM32CubeIDE:**
   - **Project → Properties → C/C++ General → Paths and Symbols**
   - Add: `Middlewares/Third_Party/SubGHz_Phy/radio_driver`

4. **Build and flash**

### 3. Expected Debug Output

```
=== Wio-E5 Mini IIS2DLPC + CW System (STM32CubeWL v1.5.0) ===
Initializing motion sensor...
IIS2DLPC detected (WHO_AM_I = 0x44)
  CTRL1 set to ODR_50Hz
  WAKE_UP_THS configured
  WAKE_UP_DUR configured
  MD1_CFG configured (INT1=wake-up)
IIS2DLPC any-motion setup complete
Initializing SUBGHZ radio (STM32CubeWL v1.5.0)...
[RADIO] Initializing STM32CubeWL v1.5.0 radio driver...
[RADIO] Radio initialized (freq=868000000 Hz, power=14 dBm)

System ready
Waiting for motion on PA9 INT1...

--- Motion detected ---
MOTION DETECTED: axis mask=0x07 (X=1 Y=1 Z=1)
CW sequence started (15s total: 600ms ON / 1000ms OFF)
[RADIO] CW TX started (freq=868000000 Hz, power=14 dBm)
  [00000ms] CW ON
[RADIO] Radio in standby (CW stopped)
  [00601ms] CW OFF
[RADIO] CW TX started (freq=868000000 Hz, power=14 dBm)
  [01601ms] CW ON
[RADIO] Radio in standby (CW stopped)
  [02201ms] CW OFF
...
[14401ms] CW OFF
CW sequence completed (elapsed=15000ms)
CW sequence stopped
```

## Architecture

### File Structure

```text
Core/
├── Inc/
│   ├── main.h              ← System definitions
│   ├── app_motion.h        ← IIS2DLPC interface
│   ├── app_cw.h            ← CW pulse generator
│   └── app_subghz.h        ← Radio driver wrapper
└── Src/
    ├── main.c              ← System init + main loop
    ├── app_motion.c        ← Sensor handling
    ├── app_cw.c            ← CW state machine
    └── app_subghz.c        ← STM32CubeWL v1.5.0 API
```

### Module Descriptions

#### `app_subghz.c` — STM32CubeWL v1.5.0 Radio Wrapper

Direct integration with STM32CubeWL v1.5.0 radio driver:

**Initialization:**
```c
void SubGHz_Init(void)
{
    RadioEvents_t RadioEvents;
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;
    RadioEvents.FhssChangeChannel = OnFhssChangeChannel;
    RadioEvents.CadDone = OnCadDone;

    Radio.Init(&RadioEvents);  ← Initialize radio with callbacks
    Radio.SetChannel(868000000UL);
    Radio.SetTxConfig(MODEM_FSK, 14, ...);
}
```

**CW Transmission:**
```c
void SubGHz_TX_Start_CW(uint32_t freq_hz, int8_t power_dbm)
{
    Radio.SetChannel(freq_hz);
    Radio.SetTxContinuousWave(freq_hz, power_dbm, 0xFFFFU);
}

void SubGHz_TX_Stop(void)
{
    Radio.IrqProcess();
    Radio.Standby();
}
```

#### `app_cw.c` — CW Pulse Pattern

- 15-second sequence with 600ms ON / 1000ms OFF timing
- State machine with HAL_GetTick() for accurate timing
- Calls `SubGHz_TX_Start_CW()` / `SubGHz_TX_Stop()` on transitions
- UART debug output with timestamps

#### `app_motion.c` — IIS2DLPC Any-Motion Detection

- Initializes sensor with 50 Hz ODR
- Sets wake-up threshold (~0.08g, 5 LSB)
- Routes INT1 to EXTI9
- On interrupt: reads WAKE_UP_SRC, decodes X/Y/Z axis
- Triggers CW sequence on motion

## STM32CubeWL v1.5.0 Radio API Reference

### `Radio.Init(RadioEvents_t *events)`

Initialize radio driver with event callbacks:
```c
typedef struct
{
    void (*TxDone)(void);
    void (*TxTimeout)(void);
    void (*RxDone)(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);
    void (*RxTimeout)(void);
    void (*RxError)(void);
    void (*FhssChangeChannel)(uint8_t currentChannel);
    void (*CadDone)(bool channelActivityDetected);
} RadioEvents_t;
```

### `Radio.SetChannel(uint32_t freq)`

Set operating frequency in Hz:
```c
Radio.SetChannel(868000000UL);  /* 868 MHz */
```

### `Radio.SetTxConfig(...)`

Configure TX parameters (FSK modulation):
```c
Radio.SetTxConfig(
    MODEM_FSK,              /* FSK modulation */
    14,                     /* 14 dBm TX power */
    4800,                   /* 4.8 kHz frequency deviation */
    125000,                 /* 125 kHz bandwidth */
    4800,                   /* 4.8 kbps datarate */
    0,                      /* Coding rate (N/A for FSK) */
    8,                      /* 8 bytes preamble */
    false,                  /* Variable length packets */
    true,                   /* CRC enabled */
    false,                  /* No frequency hopping */
    0,                      /* Hop period (N/A) */
    false,                  /* IQ not inverted */
    3000                    /* 3000 ms TX timeout */
);
```

### `Radio.SetTxContinuousWave(uint32_t freq, int8_t power, uint16_t time)`

Start continuous wave transmission:
```c
Radio.SetTxContinuousWave(868000000UL, 14, 0xFFFFU);
/* freq: 868 MHz
   power: 14 dBm
   time: 0xFFFF seconds (~18 hours, effectively indefinite)
*/
```

### `Radio.Standby()`

Put radio in low-power standby (stops TX/RX):
```c
Radio.Standby();
```

### `Radio.IrqProcess()`

Process pending radio interrupts:
```c
Radio.IrqProcess();
```

## Configuration

### Frequency Selection

In `app_subghz.h`:
```c
#define SUBGHZ_FREQ_868MHZ         868000000UL    /* Europe (CE) */
#define SUBGHZ_FREQ_915MHZ         915000000UL    /* USA/Australia */
#define SUBGHZ_FREQ_DEFAULT        SUBGHZ_FREQ_868MHZ
```

### TX Power

```c
#define SUBGHZ_TX_POWER_DBM        14             /* 14 dBm (max for ISM) */
```

### CW Pattern Timing

In `app_cw.h`:
```c
#define CW_ON_TIME_MS     600U      /* 600 ms ON */
#define CW_OFF_TIME_MS    1000U     /* 1000 ms OFF */
#define CW_TOTAL_TIME_MS  15000U    /* 15 seconds total */
```

## Troubleshooting

### Radio not initializing

**Check:**
- STM32CubeWL v1.5.0 radio driver files copied to `Middlewares/Third_Party/SubGHz_Phy/radio_driver/`
- Include path added in STM32CubeIDE project settings
- `radio.h` and `radio_def.h` found in compilation

### CW not transmitting

**Check:**
- UART debug shows "[RADIO] CW TX started" message
- Radio PA (power amplifier) enabled and configured
- Antenna connected and impedance matched
- Regulatory compliance for frequency/power in your region

### IIS2DLPC not detected

**Check:**
- PA15 (SDA) and PB15 (SCL) wiring correct
- 4.7 kΩ pull-ups on I2C bus
- I2C timing configured for 100 kHz (0x00707CBBU)
- Sensor has 3.3V power

### No UART output

**Check:**
- USB connection to Wio-E5 Mini
- Baud rate set to 115200
- PB6 (TX) and PB7 (RX) not used by other peripherals
- USART1 enabled in clock configuration

## Performance Notes

- **I2C Timing:** 100 kHz (0x00707CBBU for 32 MHz clock)
- **Main Loop:** 10 ms tick interval
- **CW Timing Accuracy:** ±10 ms (depends on HAL_GetTick() resolution)
- **Radio Initialization:** ~100-200 ms first call
- **State Transitions:** <10 ms standby to CW or vice versa

## Next Steps

1. ✅ Copy STM32CubeWL v1.5.0 radio driver
2. ✅ Configure include paths in STM32CubeIDE
3. ✅ Verify hardware connections
4. ✅ Compile and flash
5. ✅ Open serial terminal (115200 baud)
6. ✅ Trigger motion on IIS2DLPC sensor
7. ✅ Observe CW pulse sequence on RF analyzer

## Support

- **STM32CubeWL v1.5.0 Documentation:** [STMicroelectronics](https://www.st.com/en/embedded-software/stm32cubewl.html)
- **Wio-E5 Mini Wiki:** [Seeed](https://wiki.seeedstudio.com/Wio-E5_mini/)
- **IIS2DLPC Datasheet:** [ST](https://www.st.com/resource/en/datasheet/iis2dlpc.pdf)

## License

Open source. Modify and use freely.
