# Group32 Operating System

This project is a small educational operating system developed as part of the IKT218 course. The system demonstrates fundamental operating system concepts through low-level programming and direct interaction with hardware components.

---

## Features

### Boot and Initialization
- Custom boot process using a multiboot-compliant loader  
- Kernel execution in protected mode  
- Basic memory initialization  

### Screen Output
- VGA text mode rendering  
- Direct memory-mapped access to the video buffer  
- Custom output and printing functions  

### Keyboard Input
- Interrupt-driven keyboard handling  
- Scan code to ASCII translation  
- Support for both blocking and non-blocking input  

### Timer (PIT)
- Programmable Interval Timer (PIT) used for delays and timing  
- Enables controlled execution speed and real-time behavior  

### Sound (PC Speaker)
- Sound generation via the PC speaker  
- Frequency-based tone control  
- Used in both the music player and game sound effects  

---

## Menu System

Upon boot, the operating system presents an interactive menu:
1. Music Player
2. Pong Game
3. System Information
4. Clear Screen
5. Halt OS


The menu allows the user to select and execute different system features using keyboard input. This provides a simple interaction layer that connects multiple subsystems within the operating system.

---

## Music Player

- Plays predefined melodies using structured note sequences  
- Utilizes the PC speaker for audio output  
- Timing controlled by the PIT  
- Demonstrates integration of sound generation and timing mechanisms  

---

## Pong Game

- Two-player Pong implemented in VGA text mode  
- Left player: `W` / `S`  
- Right player: Arrow `Up` / `Down`  
- Real-time input via keyboard interrupts  
- Continuous game loop with screen updates  
- Sound effects for paddle collisions and scoring events  

---

## System Integration

The project demonstrates integration of key operating system components:

- Memory management (basic setup)  
- Interrupt handling (keyboard input)  
- Timer-based execution (PIT)  
- Low-level screen output (VGA text mode)  
- Sound generation (PC speaker)  

These components are combined through a menu-driven interface, enabling interactive use of the operating system.

---

## Build and Run

### Build (VS Code / Dev Container)

```bash
rm -rf build
mkdir -p build/group32
cd build/group32
cmake ../../src/group32
cmake --build .
cmake --build . --target uiaos-create-image

## Run:
cd ~/2026-ikt218-osdev/build/group32
../../src/group32/scripts/start_qemu.sh kernel.iso
## Authors

Camilla H. Smith-Tønnessen  
Ingelin Birkeland Rage  

Group 32  
IKT218 – Advanced Operating Systems