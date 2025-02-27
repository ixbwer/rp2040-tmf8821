# ToF Sensor Fishing Game for Raspberry Pi Pico

This project implements an interactive fishing game using a Raspberry Pi Pico, a TMF8821 Time-of-Flight (ToF) sensor, and an ST7789 display. The game uses hand gestures and height detection to provide an innovative gaming experience.

## Hardware Requirements

- Raspberry Pi Pico
- TMF8821/TMF8828 Time-of-Flight (ToF) sensor
- ST7789 LCD display (240x240 resolution)
- Jumper wires for connections

## Pin Configuration

### Display Connections
- SPI TX: GPIO 3
- SPI SCK: GPIO 2
- SPI DC: GPIO 1
- SPI RESET: GPIO 0

### LED
- LED: GPIO 4

## Game Description

This interactive game uses the ToF sensor to detect hand position and movements. The gameplay consists of matching your hand height with a moving target line on the screen.

### Game States

1. **Main Menu**: Select between "START" and "CONFIG" options
2. **Config**: Choose game difficulty (EASY, MEDIUM, HARD)
3. **Game**: Match your hand height with the moving target on the screen to score points
4. **Game Over**: View your score and choose to restart or return to menu

### Controls

The game uses hand gestures for navigation:
- **Up/Down**: Move selection in menus
- **Left**: Select current option
- **In-game**: Position your hand at the correct height to catch fish and score points

### Difficulty Levels

- **EASY**: Slower target movement, larger matching threshold
- **MEDIUM**: Medium target movement speed
- **HARD**: Fast target movement, small matching threshold

## Building and Running

1. Ensure you have the Raspberry Pi Pico SDK installed
2. Build the project using CMake:
   ```
   mkdir build
   cd build
   cmake ..
   make
   ```
3. Flash the resulting .uf2 file to your Raspberry Pi Pico

## Game Flow

1. Start at the Main Menu
2. Select "START" to begin playing or "CONFIG" to adjust difficulty
3. During gameplay, match your hand height with the target line
4. Score increases when you maintain the correct height
5. When the game ends, choose to restart or return to menu

## Technical Details

The game uses a dual-core approach:
- Core 0: Handles game logic, display updates, and user interface
- Core 1: Continuously reads data from the ToF sensor

The ToF sensor provides:
- Hand height measurements
- Direction detection based on hand movement

## Troubleshooting

- If the display appears black, check the SPI connections
- Make sure the TMF8821 sensor is properly connected and initialized
- Check the serial output (115200 baud rate) for debugging information

## License

This project is provided for educational purposes.