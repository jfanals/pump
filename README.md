# Arduino Automatic Water Pump Controller

A smart Arduino-based system for automatically controlling a water pump to empty a well or tank. The system detects water flow and manages pump cycles with built-in safety features and visual feedback.

## Features

- **Automatic Operation**: Pump starts, detects flow, and stops when water runs out
- **Smart Cycle Management**: Waits for well to refill before attempting next cycle
- **Flow Detection**: Uses sensor input to detect actual water flow
- **Safety Features**: Maximum pump runtime protection and debounce filtering
- **Visual Feedback**: LED blinks to show total minutes of successful pumping
- **Serial Monitoring**: Detailed logging for debugging and monitoring

## How It Works

1. **Prime**: Pump starts for 5 seconds to establish flow
2. **Pump**: If flow detected, continues pumping while water flows
3. **Stop**: When flow stops for 5 seconds, pump turns off
4. **Wait**: Waits 1 hour for well to refill, then repeats

## Hardware Requirements

- Arduino Uno/Nano or compatible
- Relay module (active LOW) for pump control
- Flow sensor (or pressure switch/float switch)
- Water pump
- Non-return valve (recommended)

## Wiring

| Component | Arduino Pin | Notes |
|-----------|-------------|--------|
| Pump Relay | Pin 7 | Active LOW (LOW = pump ON) |
| Flow Sensor | Pin 2 | INPUT_PULLUP (LOW = flow detected) |
| Status LED | Pin 13 | Built-in LED for visual feedback |

## Configuration

Edit these constants in `pump_controller.ino` to adjust timing:

```cpp
const unsigned long PRIME_DURATION = 5000UL;      // 5 seconds
const unsigned long REFILL_WAIT = 3600000UL;      // 1 hour  
const unsigned long MAX_PUMP_TIME = 1200000UL;    // 20 minutes
const unsigned long NO_FLOW_DEBOUNCE = 5000UL;    // 5 seconds
```

## Installation

1. **Download** the code and open `pump_controller.ino` in Arduino IDE
2. **Connect** your hardware according to the wiring table
3. **Configure** timing constants for your specific setup
4. **Upload** to your Arduino
5. **Monitor** via Serial Monitor (9600 baud) for debugging

## LED Indicator

The built-in LED (Pin 13) shows total successful pumping time:
- **Each blink** = 1 minute of actual pumping
- **Blink pattern**: 500ms ON, 500ms OFF per minute
- **Pause**: 2 seconds between complete cycles
- **Example**: 5 minutes total = blink-blink-blink-blink-blink, pause, repeat

## Safety Notes

⚠️ **Important Safety Considerations:**
- Ensure proper electrical isolation between Arduino and pump power
- Use appropriate relay ratings for your pump
- Install proper fusing and circuit protection
- Test thoroughly before leaving unattended
- Monitor for dry-running conditions

## Typical Use Cases

- **Well water pumping**: Automatically empty shallow wells
- **Sump pump control**: Manage water removal from basements
- **Tank drainage**: Automated tank emptying systems
- **Irrigation**: Controlled water transfer systems

## Serial Output

Connect to Serial Monitor (9600 baud) to see:
- Current system state and timing
- Flow detection status  
- Total pumping time accumulated
- Debug information every 10 seconds

## License

Open source - feel free to modify and adapt for your needs.

## Contributing

Issues and improvements welcome! This is a practical project designed for real-world water management applications.

---

*Built for reliability and safety in automated water pumping applications.*