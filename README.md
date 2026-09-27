# FabLink

FabLink is a C++17/Qt-based semiconductor equipment communication simulator demonstrating a simplified SECS/GEM stack.

## What it demonstrates

- **SECS-II** — structured messages and binary data encoding
- **HSMS** — TCP-based host/equipment communication
- **GEM** — basic communication and equipment control states
- **Multithreading** — simulated measurement generation using a worker thread
- **Qt** — Widgets UI, signals/slots, and networking
- **OOP** — encapsulation, composition, abstraction, and clean class separation

## Architecture

```text
Qt UI
  |
CommunicationManager
  |
  +-- Equipment
  +-- GEM State Machine
  +-- Measurement Worker
  |
HSMS Connection
  |
SECS-II Messages
  |
TCP/IP
  |
FabLinkHost
```

## Applications

- **FabLink** — simulated semiconductor equipment with a Qt dashboard.
- **FabLinkHost** — host/MES simulator that connects to the equipment.

The demo performs a simplified startup handshake and exchanges event reports such as `S1F1`, `S1F2`, `S1F13`, `S1F14`, `S6F11`, and `S6F12`.

## Build

Requirements:

- Qt 6
- C++17 compiler
- CMake
- MinGW or another supported Qt compiler

Open `CMakeLists.txt` in Qt Creator, select the Qt 6 kit, configure CMake, and build both targets.

## Run

1. Start **FabLink**. The equipment listens on port `5000`.
2. Start **FabLinkHost**.
3. The Host connects to the Equipment.
4. The programs perform the HSMS/GEM handshake.
5. The Equipment generates simulated measurements and sends event reports.

## Note

This is a portfolio/learning implementation of a representative subset of SECS-II, HSMS, and GEM. It is **not a certified production SECS/GEM implementation** and intentionally omits many production features such as full timer/retry handling, complete standard coverage, alarms, recipes, and vendor-specific configuration.

## Author

FabLink — Semiconductor Equipment Communication Simulator
