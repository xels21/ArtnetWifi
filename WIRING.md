# ESP32 to MAX485 Wiring for DMX512 Output

## Components Required
- ESP32 Development Board
- MAX485 Module (or SN75176 compatible RS-485 transceiver)
- XLR-3 or XLR-5 connector (for DMX output)
- 120Ω termination resistor (for end of DMX line)
- Jumper wires

## Wiring Diagram

```
ESP32                    MAX485 Module              DMX XLR Connector
┌─────────────┐         ┌──────────────┐           ┌─────────────┐
│             │         │              │           │             │
│  GPIO 17 ───┼────────►│ DI (TXD)     │           │             │
│   (TX)      │         │              │           │             │
│             │         │              │           │             │
│  GPIO 4  ───┼────────►│ DE/RE        │           │             │
│   (DE)      │         │              │           │             │
│             │         │              │           │             │
│  GND     ───┼────────►│ GND          │           │             │
│             │         │              │           │   Pin 1     │
│  5V/3.3V ───┼────────►│ VCC          │           │   (GND) ────┼──┐
│             │         │              │           │             │  │
│             │         │      B    ───┼──────────►│   Pin 2     │  │
│             │         │      (Data-) │           │   (Data-)   │  │
│             │         │              │           │             │  │
│             │         │      A    ───┼──────────►│   Pin 3     │  │
│             │         │      (Data+) │           │   (Data+)   │  │
└─────────────┘         │              │           │             │  │
                        └──────────────┘           └─────────────┘  │
                                                                     │
                                                   120Ω termination │
                                                   (between pins    │
                                                    2 and 3 at      │
                                                    end of chain)───┘
```

## Pin Connections

### ESP32 to MAX485
| ESP32 Pin | MAX485 Pin | Function                          |
|-----------|------------|-----------------------------------|
| GPIO 17   | DI (TXD)   | Data Input (UART TX)              |
| GPIO 4    | DE         | Driver Enable (HIGH = transmit)   |
| GPIO 4    | RE         | Receiver Enable (tie to DE)       |
| GND       | GND        | Ground                            |
| 5V        | VCC        | Power (3.3V also works for most)  |

### MAX485 to XLR Connector
| MAX485 Pin | XLR Pin    | DMX Signal                        |
|------------|------------|-----------------------------------|
| A (Data+)  | Pin 3      | DMX+ (positive differential)      |
| B (Data-)  | Pin 2      | DMX- (negative differential)      |
| GND        | Pin 1      | Signal Ground / Shield            |

## Important Notes

### 1. DE and RE Pins
- **DE** (Driver Enable) and **RE** (Receiver Enable) should be tied together
- Connect both to GPIO 16 on the ESP32
- HIGH = Transmit mode, LOW = Receive mode
- For DMX output, we keep it HIGH to transmit

### 2. Power Supply
- MAX485 can run on 3.3V or 5V
- Most modules work fine with 3.3V from ESP32
- For longer cable runs, 5V is more reliable (use ESP32's 5V pin if available)

### 3. XLR Pin Order
Standard DMX512 uses:
- **Pin 1**: Common/Ground
- **Pin 2**: Data- (DMX-)
- **Pin 3**: Data+ (DMX+)

Connection: A→Pin3, B→Pin2

### 4. Termination Resistor
- Place a 120Ω resistor between pins 2 and 3 **at the end** of the DMX chain
- This prevents signal reflections
- Only needed at the last device in the chain

### 5. Cable Requirements
- Use proper DMX cable (120Ω characteristic impedance)
- Cat5/Cat6 can work for short runs but is not ideal
- Maximum recommended length: 300-400 meters total chain

## Configuration in Code

Current settings in `platformio.ini`:
```ini
build_flags = 
    -DDMX_ENABLE=1
    -DDMX_TX_PIN=17
    -DDMX_DE_PIN=4
    -DDMX_REFRESH_MS=10
```

To change pins or refresh rate, modify these values:
- `DMX_TX_PIN`: UART TX pin (default: 17)
- `DMX_DE_PIN`: Driver Enable pin for MAX485 (default: 4)
- `DMX_REFRESH_MS`: Maximum time between DMX frames in milliseconds (default: 10ms = ~100Hz refresh)
  - Lower values = faster refresh, more responsive
  - Typical range: 10-25ms (40-100Hz)
  - DMX standard allows up to 1 second, but most fixtures expect 20-44 frames/sec

**Important:** DMX frames are now sent continuously at the configured refresh rate, even without incoming Art-Net data. This ensures DMX fixtures receive a stable signal and don't timeout or flicker.

## Testing

1. Upload the firmware: `build.bat`
2. Connect to WiFi (check Serial Monitor at 115200 baud)
3. Send Art-Net data from your lighting software to the ESP32's IP address
4. DMX512 data will be output on the XLR connector
5. Universe 0 by default

## Troubleshooting

**No DMX output:**
- Check MAX485 module has power (LED should be on if included)
- Verify DE pin is HIGH (measure with multimeter: should be 3.3V)
- Ensure Art-Net data is being sent to correct IP and universe

**Flickering/unstable output:**
- Check cable quality and length
- Add 120Ω termination resistor at end of chain
- Verify data rate is exactly 250,000 baud (set in code)

**Devices don't respond:**
- Verify XLR wiring (Pin 2 = Data-, Pin 3 = Data+)
- Some cheap MAX485 modules have A and B swapped
- Try swapping wires on pins 2 and 3

## Example MAX485 Module Boards

Common modules available:
- Generic MAX485 TTL to RS485 converter modules
- Waveshare RS485 Board
- DEBO RS 485 Modul

Most are pin-compatible and work identically.
