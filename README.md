# Autonomous Dual-World Robot — SLRC 2026

Autonomous physical robot built for the **Sri Lankan Robotics Challenge (SLRC) 2026 — University Category**, organized by the Electronic Club (E-Club), Department of Electronic & Telecommunication Engineering, University of Moratuwa.

This project implements a physical robot that acts as the **master controller** for a networked **virtual "slave" robot (Ares)**, coordinating maze navigation, computer-vision-based tag decoding, object manipulation, and real-time control of a simulated environment — all fully autonomously.

**Team:** PRISM
**Category:** University Category
**Competition period:** Dec 2025 – Mar 2026

---

## Table of Contents

- [Overview](#overview)
- [Mission Context](#mission-context)
- [System Architecture](#system-architecture)
- [Tasks](#tasks)
  - [Task 1 — Maze Traversal & Coordinate Extraction](#task-1--maze-traversal--coordinate-extraction)
  - [Task 2 — Inventory Collection & Coordinate Extraction](#task-2--inventory-collection--coordinate-extraction)
  - [Task 3 — Particle Laser Repair](#task-3--particle-laser-repair)
  - [Task 4 — Hidden Task & Final Challenge](#task-4--hidden-task--final-challenge)
  - [Simulation Arena Task](#simulation-arena-task)
- [Hardware](#hardware)
- [Software](#software)
- [Repository Structure](#repository-structure)
- [Getting Started](#getting-started)
- [Scoring Summary](#scoring-summary)
- [Team](#team)
- [License](#license)

---

## Overview

SLRC is one of Sri Lanka's longest-running robotics competitions, running since 2012. The 2026 University Category introduces a new dimension: teams must operate **simultaneously in a physical arena and a simulated environment**. The physical robot must autonomously:

1. Navigate a maze and decode obfuscated data hidden in AprilTags.
2. Collect objects and extract further coordinate data.
3. Repair a "Particle Laser" mechanism using collected objects.
4. Control a separate, networked virtual robot in real time — sending it through a sequence of waypoints in a simulated grid world while avoiding a patrolling hostile agent.
5. Synchronize completion between the physical and virtual robots to "bridge" the two worlds.

This repository contains the firmware, computer vision pipeline, decoding logic, and simulation-control client developed by Team PRISM for this challenge.

## Mission Context

The competition is wrapped in a narrative ("War for the Permanence") in which two rival factions — ENCOM and Dillinger Systems — compete for control of technology that allows digital entities to exist in the real world. Our robot plays the role of the field-deployed unit that must:

- Search a physical lab environment (the maze) for coordinate markers.
- Transmit those coordinates to a virtual entity ("Ares") so it can navigate the digital grid ("the Dillinger Grid") autonomously.
- Recover components to repair a "Particle Laser," the bridge mechanism between the physical and virtual worlds.
- Trigger the bridge activation once both the physical and virtual objectives are complete.

## System Architecture

```
┌─────────────────────────┐        REST API        ┌──────────────────────────┐
│   Physical Robot         │ ──────────────────────▶ │  Simulated Robot (Ares)  │
│   (Raspberry Pi)         │   waypoint commands      │  Virtual Grid Environment │
│                          │ ◀────────────────────── │                          │
│  - AprilTag detection    │   odometry / camera feed │  - 25×25 grid            │
│  - Multi-key decoding    │                          │  - Hostile patrol agent  │
│  - Maze navigation       │                          │  - Portal (goal)         │
│  - Object manipulation   │                          │                          │
│  - Slider-based insertion│                          │                          │
└─────────────────────────┘                          └──────────────────────────┘
```

The physical robot acts as the **master**, autonomously deciding when and where to send the virtual robot, while independently completing its own physical-world tasks in parallel.

## Tasks

### Task 1 — Maze Traversal & Coordinate Extraction

- Autonomously traverse the maze and detect **8 AprilTags** (family `tagStandard52h13`) mounted on maze walls.
- Each Tag ID is an obfuscated value that must be decoded using one of **5 predefined keys**, obtained by solving pre-competition puzzles.
- Decoded values follow the structure `[Key ID (1 digit)][Payload (4 digits)]`, which resolves to `[Order ID][X][Y]` — the sequence position and target grid coordinates for the virtual robot.
- An onboard LED lights up on successful tag detection.

### Task 2 — Inventory Collection & Coordinate Extraction

- After Task 1, the robot collects **6 red cubes** from platforms, each identified by a unique color + shape marker (triangle/rectangle/pentagon in blue/green/orange).
- Cubes are stored onboard.
- Removing each cube reveals another AprilTag, decoded using the same scheme as Task 1, contributing to the full set of 14 coordinate points for the virtual robot.

### Task 3 — Particle Laser Repair

- The robot navigates to a transparent tube assembly and inserts up to 3 collected cubes to trigger a proximity switch — **without making direct contact with the switch itself**.
- A custom slider-actuated mechanism (`hand_slider`) performs the precision insertion.
- Insertion status is verified via a polling API endpoint provided by the competition server.

### Task 4 — Hidden Task & Final Challenge

- A task revealed only on competition day, solvable with the robot's existing hardware.
- After completion, the robot must reach the control panel (red square) to finish the physical-world objective.

### Simulation Arena Task

- The physical robot autonomously controls the virtual robot ("Ares") via a **REST control API**, sending linear/angular velocity or displacement commands.
- Ares must visit **14 sequenced waypoints** (decoded from Tasks 1 & 2) in order and reach a **Portal** to complete the mission.
- A **hostile patrol agent** moves along a randomized path with a 1-cell detection radius; entering its detection zone at the wrong moment incurs scoring penalties.
- Real-time camera feeds (2 front-facing + 1 floor-facing) and odometry are streamed back from the simulated robot to support navigation decisions.

## Hardware

| Component | Details |
|---|---|
| Compute | Raspberry Pi (onboard vision + control) |
| Vision | Camera module — AprilTag detection (`tagStandard52h13`) |
| Manipulation | Custom slider-actuated cube insertion mechanism |
| Power | Internal supply, ≤ 24V |
| Chassis | Custom-built, ≤ 25 cm × 25 cm footprint at start |
| Indication | Onboard LED for tag/task confirmation |

## Software

- **Language(s):** Python / C++ *(update to match your actual stack)*
- **Computer Vision:** OpenCV + AprilTag detection library
- **Decoding Engine:** Custom multi-key decode functions (5 keys, obtained via pre-competition puzzles)
- **Simulation Client:** REST API integration for real-time control of the virtual robot
- **Networking:** Wi-Fi connection to competition access point + cloud-hosted simulator server

## Repository Structure

```
.
├── Pre Competition Challenges/   # Puzzle solutions and decoding key derivations
├── Raspberry Pi/                 # Onboard robot firmware, CV pipeline, task logic
├── hand_slider/
│   └── code/                     # Slider mechanism control (cube insertion)
└── README.md
```

*(Update this section with real subfolder details as the repo evolves.)*

## Getting Started

> These are placeholder setup instructions — replace with your actual build/run steps.

```bash
# Clone the repository
git clone https://github.com/denethp/autonomous-dual-world-robot.git
cd autonomous-dual-world-robot

# Install dependencies (example)
pip install -r requirements.txt

# Run the main control loop on the Raspberry Pi
python3 "Raspberry Pi/main.py"
```

## Scoring Summary

| Category | Max Score |
|---|---|
| Pre-Competition Challenges | 100 |
| Task 1 — AprilTag Read & Decode | 120 |
| Task 2 — Cube Collection & Decode | 150 |
| Task 3 — Tube Insertion & Switch Activation | 200 |
| Simulation — Waypoints, Order Bonus, Portal | 430 |
| Hidden Challenge | 150 |
| Mission Completion + Time Efficiency | 100 |
| **Total Achievable** | **1250** |

*(See the official SLRC 2026 University Category Task Document for full scoring and penalty details.)*

## Team

**Team PRISM** — University of Moratuwa

- Deneth Priyadarshana

*(Add other team members here.)*

## License

This repository is shared for portfolio and demonstration purposes only. All rights reserved — please do not reuse, redistribute, or submit this code as your own work.

---

*Built for the Sri Lankan Robotics Challenge (SLRC) 2026 — organized by the Electronic Club, Dept. of Electronic & Telecommunication Engineering, University of Moratuwa.*
