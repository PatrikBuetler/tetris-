# University of Basel HS24 
# Computer Architecture
# Professor Christian Tschudin
## Authors
- Diego Gonçalves Simao diego.goncalvessimao@stud.unibas.ch
- Illia Solohub illia.solohub@stud.unibas.ch
- Patrik Buetler patrik.buetler@stud.unibas.ch
- 
# Tetris++

## Project Overview
Tetris++ is a reimagined version of the classic Tetris game, developed as part of the Computer Architecture course at the University of Basel. This project demonstrates the implementation of Tetris using Arduino hardware, featuring a unique interactive gameplay experience enhanced by gyroscopic controls and additional block types.

## Features
- **Classic Tetris Gameplay**: A faithful recreation of the original mechanics.
- **Special Blocks**:
  - Explosion Blocks: Clear entire rows upon landing.
  - Brittle Blocks: Fill gaps below to improve the player's board state.
- **Gyroscopic Controls**: Rotate and maneuver tetrominoes by tilting the device.
- **4x8x8 LED Display**: Smooth and clear visual representation of the game.

## Hardware Components
- **Arduino Mega 2560**
- **4 LED Matrices (MAX7219)**
- **Gyroscope (MP6050)**
- **5 Push Buttons**
- **Power Supply (9V)**
- **Jumper Wires and Connectors**
- **Resistors (10k OHM)**
- **2 Breadboards**

### Total Cost
- **ELEGOO Starter Kit**: 67 CHF
- **Gyroscope MP6050**: 13 CHF
- **4x LED Matrices MAX7219**: 19 CHF
- **Total**: 100 CHF

## Code Structure
The source code is organized into modular components to facilitate readability and maintenance. Key files include:
- `blockLogic.ino`: Handles the logic for tetromino behavior.
- `initFunctions.ino`: Contains initialization routines for hardware.
- `tetris.ino`: Implements the main game loop.

## Installation and Setup
1. Clone the repository:
   ```bash
   git clone https://github.com/PatrikBuetler/tetris-
   ```
2. Assemble the hardware as per the provided circuit diagram.
3. Upload the code to the Arduino Mega using the Arduino IDE.
4. Power up the system and enjoy the game!

## Gameplay Instructions
- Use the push buttons to move and drop tetrominoes.
- Rotate the blocks by tilting the device (enabled by the gyroscope).
- Watch out for special blocks that can help or surprise you!

## Challenges and Learnings
- Transitioned from a brute-force approach to a structured pixel-based system.
- Developed custom logic for special blocks and integrated gyroscopic controls.
- Learned valuable lessons in hardware-software integration and embedded programming.

## References
- [ELEGOO Starter Kit](https://www.elegoo.com/blogs/arduino-projects/elegoo-mega-2560-the-most-complete-starter-kit-tutorial?srsltid=AfmBOopupDCUx4k3GlIwDG9ThsCZb3olse-rFXBHS9r1umHbmG4WhPqe)

## Disclaimer
The content of this project stems from our work. ChatGPT was only used for improving the wording of this report.


