# Autonomous Dual-World Robot — SLRC 2026

Autonomous physical robot built for the **Sri Lankan Robotics Challenge (SLRC) 2026 — University Category**, organized by the Electronic Club (E-Club), Department of Electronic & Telecommunication Engineering, University of Moratuwa.

The robot acts as a **master controller** for a networked **virtual "slave" robot (Ares)**, coordinating maze navigation, computer-vision-based tag decoding, contactless object manipulation, and real-time closed-loop control of a simulated environment — entirely autonomously, with no remote control of any kind.

📺 **Demo video:** [Watch on YouTube](https://youtu.be/xlX__w1153E)

[![Watch the demo](https://img.youtube.com/vi/xlX__w1153E/maxresdefault.jpg)](https://youtu.be/xlX__w1153E)

**Team:** PRISM — University of Moratuwa
**Category:** University Category
**Competition period:** Dec 2025 – Mar 2026

---

## Table of Contents

- [Overview](#overview)
- [System Architecture](#system-architecture)
- [What Makes This Project Interesting](#what-makes-this-project-interesting)
- [Subsystems](#subsystems)
  - [Physical Robot — Arduino Mega](#physical-robot--arduino-mega)
  - [Gripper / Slider Subsystem — Arduino Slave](#gripper--slider-subsystem--arduino-slave)
  - [Vision & Decoding — Raspberry Pi](#vision--decoding--raspberry-pi)
  - [Virtual Robot Control (Ares)](#virtual-robot-control-ares)
- [Repository Structure](#repository-structure)
- [Hardware](#hardware)
- [Software Stack](#software-stack)
- [Communication Protocol](#communication-protocol)
- [Team](#team)
- [License](#license)

---

## Overview

SLRC 2026's University Category requires a physical robot to operate **simultaneously in a physical arena and a simulated environment**. The physical robot must autonomously:

1. Traverse a maze, detect 8 AprilTags, and decrypt obfuscated tag data into ordered coordinates.
2. Collect 6 cubes from marked platforms and decode a further AprilTag revealed behind each.
3. Insert cubes into a tube to trigger a proximity switch — without ever touching the switch directly.
4. **Simultaneously** control a separate, network-connected simulated robot ("Ares"), guiding it through 14 sequenced waypoints on a 25×25 grid while avoiding a randomly patrolling hostile agent, purely via a REST API and Ares's own onboard cameras.
5. Synchronize both robots reaching their end conditions to complete the mission.

This repository contains everything Team PRISM built to solve it — firmware for two microcontrollers and a full Raspberry Pi–based vision and control stack.

## System Architecture

```
┌────────────────────────┐   Serial (command byte + "DONE")   ┌──────────────────────────┐
│   Arduino Mega           │ ──────────────────────────────────▶ │  Arduino Slave            │
│   (main brain)            │ ◀────────────────────────────────  │  (hand_slider gripper)    │
│  - Line following / PID   │                                     │  - Non-blocking state     │
│  - ToF wall/box alignment │                                     │    machines (grab/release)│
│  - Encoder odometry       │                                     │  - Dual VL6180X ToF cube  │
│  - Elimination task logic │                                     │    presence detection     │
└────────────────────────┘                                     │  - Stall-detect + burst    │
                                                                    │    recovery linear actuator│
                                                                    └──────────────────────────┘

┌────────────────────────┐        REST API         ┌──────────────────────────┐
│   Raspberry Pi           │ ───────────────────────▶ │  Simulated Robot (Ares)   │
│  - AprilTag detection     │   set_velocity /          │  - 25×25 grid              │
│    (tagStandard52h13)     │   move_relative /         │  - Randomized hostile      │
│  - Multi-key decrypt       │   odometry / camera feeds │    patrol agent            │
│  - Threaded scan+move      │ ◀───────────────────────  │  - Portal (goal)            │
│    (producer/consumer)     │                            │                             │
│  - Vision-based hostile     │                            │                             │
│    detection + line PID     │                            │                             │
└────────────────────────┘                            └──────────────────────────┘
```

Three separate compute nodes — Arduino Mega, a second Arduino acting as a serial slave, and a Raspberry Pi — coordinate over serial and a REST API to run the full mission.

## What Makes This Project Interesting

A few implementation details worth calling out, straight from the codebase:

- **Contactless cube confirmation.** Instead of a mechanical switch, the gripper uses two independently address-programmed VL6180X time-of-flight sensors (`SHDN` pin sequencing to avoid I²C address collisions) to confirm a cube is actually grasped before proceeding.
- **Stall detection with burst recovery.** The slider's linear actuator monitors its own quadrature encoder; if it stalls mid-travel, the firmware cuts power, pauses, then fires a short high-PWM burst to break static friction before resuming normal closed-loop control — a small but effective robustness trick against mechanical binding.
- **Non-blocking state machines**, not `delay()`-laden sequences. Both the box-grab and cube-release routines on the slave Arduino are step-indexed state machines (`box_step`, `relese_step`) driven from the main loop, keeping the board responsive to new serial commands mid-sequence.
- **A genuinely elegant multi-key decode dispatcher.** Five distinct, reversible obfuscation functions — digit-reversal-and-multiply, halves-swap, nines'-complement, digit-rotate, and Gray-code XOR — are dispatched by a tag's leading digit, then the result is unpacked via base-625 arithmetic directly into `(order, x, y)`, with explicit range validation against the competition's documented bounds.
- **Producer/consumer multithreading** for tag scanning. One thread continuously scans and decodes AprilTags into a shared, lock-protected dictionary; a second thread drains it strictly in order (1→14) and drives Ares toward each point as soon as it's available — so the robot doesn't wait for all tags to be found before Ares starts moving.
- **Pure-vision hostile agent detection.** Ares's two front cameras are stitched into a single panorama, HSV-thresholded for the competition's specified hostile-agent green, and proximity is estimated from where the color blob's centroid crosses a fixed "warning line" in the frame — no depth sensor required.
- **Closed-loop correction layered over an open-loop API.** The Ares control API only exposes velocity and relative-move commands, so after every move or turn, a secondary PID loop reads Ares's own floor camera to re-center the robot on the grid lines, and a separate tally counts actual line crossings during a move rather than trusting elapsed time — compensating for network lag or simulator drift.
- **Chunked, obstacle-aware pathfinding.** `go_to_grid_point_chunked()` moves Ares in strides of up to 4 cells at a time, re-checking for the hostile agent and re-evaluating heading before every leap, rather than committing to one long blind path toward the target.

## Subsystems

### Physical Robot — Arduino Mega

- Encoder-based odometry (900 CPR encoders, 64.5 mm wheels, 147 mm track width) driving a full library of motion primitives: distance-based forward/backward moves, multiple line-following variants (deadband turns, "until all-white" line-loss detection, ToF-assisted distance stops), and closed-loop turns.
- An 8-sensor QTR IR array with weighted-error PID line following (`moveAlongLine`), combined with a 6-sensor ToF array for wall/box-edge alignment.
- Box-edge detection uses a rolling moving-average filter (`getAverageLeft`/`getAverageRight`) with encoder-distance debouncing, so a single sensor blip doesn't falsely trigger a box pickup.
- `eliminationTask()` encodes the full elimination-round routine as a sequential macro program built from these primitives — collect all 6 boxes, navigate to the tube, and trigger the proximity switch via cube insertion.

### Gripper / Slider Subsystem — Arduino Slave

- Receives single-character commands over serial (`H` home, `B` box sequence, `R` release sequence, `1`–`4` position presets, etc.) and acknowledges completion with `"DONE"`.
- Servo-driven base lift/lower uses an eased motion profile (`Ramp` library, quadratic in/out) rather than an instant `write()`, for smoother mechanical motion.
- Cube presence is confirmed via dual VL6180X ToF sensors before the gripper closes, and a linear actuator with stall detection handles precise slider positioning for tube insertion.

### Vision & Decoding — Raspberry Pi

- `apriltag_to_coordinates.py` implements the 5-key decryption scheme described above, shared identically between the physical-arena scanning task and the simulation-control task.
- `decoding_tag_task.py` / `main.py` run the Pi camera + AprilTag detector (`tagStandard52h13` family) to build the ordered coordinate map used to drive Ares.

### Virtual Robot Control (Ares)

- `ares_utils.py` — a thin REST client wrapping every documented Ares endpoint: `set_velocity`, `move_relative`, `/odometry`, `/camera/{id}/frame`, `/health`, `/led`, `/arena/metadata`.
- `ares_move_functions.py` — higher-level navigation built on top of the client: heading tracking, chunked grid-point navigation with hostile-agent avoidance, and vision-based alignment/correction after every move.
- `ares_cv_functions.py` — the vision pipeline: floor-line detection for alignment, panoramic hostile-agent detection, and camera-based reference calibration at the start of a run.

## Repository Structure

```
.
├── Arduino Mega/
│   └── main/                     # Main robot firmware
│       ├── main.ino              # Entry point
│       ├── elimination_task.ino  # Elimination round routine
│       ├── odometry.ino          # Motion primitives (line following, turns, ToF alignment)
│       ├── motors.ino            # Motor driver interface
│       ├── communication.ino     # Serial handshake with slave Arduino
│       ├── tof.ino               # Time-of-flight sensor array
│       ├── ir_panel.ino          # QTR IR array reading
│       ├── callibration.ino      # Sensor calibration routines
│       ├── display.ino           # OLED status display
│       ├── config.h              # Pin mappings & tunable constants
│       ├── globals.h             # Shared global state
│       └── types.h               # Shared type definitions
│
├── hand_slider/
│   └── code/
│       └── code.ino              # Slave Arduino — gripper & slider control
│
├── Raspberry Pi/
│   ├── April Tag/
│   │   ├── Apriltag_source/      # AprilTag library (v3.4.5) built for the Pi
│   │   ├── Helper Functions/     # Tag generation, visualization, SVG export
│   │   ├── apriltag_to_coordinates.py  # 5-key decryption engine
│   │   └── decoding_tag_task.py  # Physical-arena tag scanning task
│   │
│   └── Simulation Task/
│       ├── main.py               # Threaded scan + move entry point
│       ├── ares_config.py        # API/network configuration
│       ├── ares_utils.py         # Ares REST API client
│       ├── ares_move_functions.py # Navigation, chunked pathing, alignment
│       ├── ares_cv_functions.py  # Line detection & hostile-agent vision
│       └── global_states.py      # Shared simulation state
│
├── Model/                        # SOLIDWORKS/Fusion 360 CAD, STLs, gears, manufacturing files
└── README.md
```

## Hardware

| Component | Details |
|---|---|
| Main compute | Arduino Mega 2560 |
| Slave compute | Arduino (Nano/Uno-class) — gripper & slider control |
| Vision compute | Raspberry Pi + Camera Module |
| Line sensing | 8-channel QTR IR reflectance array |
| Proximity/alignment | 6× time-of-flight sensors (wall/box alignment) + 2× VL6180X (cube presence) |
| Actuation | DC gear motors (drive), N20 motors + lead screw (slider), MG995/MG90S servos (gripper base & claw) |
| Manipulation | Custom 3D-printed/CNC gripper and slider assembly (see `Model/`) |
| Tag detection | AprilTag `tagStandard52h13` family, 6 cm × 6 cm |

## Software Stack

- **Firmware:** C++ (Arduino) — motor control, odometry, line/wall following, serial protocol, state machines
- **Vision & Control:** Python — OpenCV, `apriltag` (v3.4.5), `picamera2`, `requests`, `numpy`, threading
- **CAD:** SOLIDWORKS / Fusion 360 (`Model/`)

## Communication Protocol

The Mega and the gripper/slider Arduino communicate over a simple serial link:

- Mega sends a single command character (e.g. `H` = home, `B` = start box sequence, `D`/`U` = base down/up, `1`–`4` = slider position presets).
- The slave Arduino executes the corresponding non-blocking state machine and, on completion, sends back `"DONE"\n`.
- The Mega's `waitForDone()` blocks only until this acknowledgment arrives, keeping the two boards synchronized without either side needing to poll on a fixed timer.

## Team

**Team PRISM** — University of Moratuwa

- Deneth Priyadarshana
- Rumeth Samarasinghe
- Piyumal Nuwarapaksha
- *(plus additional contributors — see repository commit history)*

## License

This repository is shared for portfolio and demonstration purposes only. All rights reserved — please do not reuse, redistribute, or submit this code as your own work.

---

*Built for the Sri Lankan Robotics Challenge (SLRC) 2026 — organized by the Electronic Club, Dept. of Electronic & Telecommunication Engineering, University of Moratuwa.*
