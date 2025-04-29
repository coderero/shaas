# SHAAS: Secure Home Automation and Access System

![SHAAS](https://img.shields.io/badge/IoT-Security-blue)

## Overview

SHAAS (Secure Home Automation and Access System) is a comprehensive IoT security solution designed for home automation and access control. The system offers RFID-based authentication, secure data transmission using Protocol Buffers, and remote management through MQTT.

## Features

- **Secure Authentication**: RFID-based access control with whitelist management
- **Efficient Communication**: Protocol Buffers (nanopb) for efficient, structured data serialization
- **MQTT Integration**: Lightweight publish-subscribe architecture for device communication
- **Environmental Monitoring**: Temperature, humidity, air quality, and light sensors
- **Access Control**: Secure whitelist management for authorized users
- **Remote Configuration**: Modify system parameters securely over the network
- **Relay Control**: Secure management of connected devices with state persistence
- **Modular Architecture**: Clean separation of concerns for maintainability

## Hardware Requirements

- Arduino UNO R4 WiFi (or compatible)
- MFRC522 RFID reader
- DHT22 temperature/humidity sensor
- MQ135 air quality sensor
- Light sensors (LDR)
- PIR motion sensors
- 4-channel relay module
- CD74HC4067 multiplexer (for sensor expansion)

## Project Structure

- `include`: Header files defining component interfaces

  - `/communication`: WiFi, Bluetooth, MQTT, and Serial interfaces
  - `/devices`: Relay control and other hardware interfaces
  - `/modules`: Hardware abstraction for sensors
  - `/services`: Core system services and management
  - `/utils`: Utility functions and helpers

- `src`: Implementation files

  - `/communication`: Network and protocol implementations
  - `/devices`: Hardware control implementations
  - `/modules`: Sensor drivers and interfaces
  - `/services`: Core service implementations
  - `/utils`: Utility implementations

- `lib`: External libraries
  - `/transporter`: Protocol buffer definitions and generated code

## Core Components

### SystemMonitor

The central controller that orchestrates all system activities and manages state transitions.

### Security Module

Provides RFID authentication and manages the whitelist of authorized users.

### ConfigEngine

Handles system configuration via EEPROM with support for multiple sensor types.

### RelayControl

Manages connected devices with state persistence across power cycles.

### SensorManager

Monitors environmental conditions using various sensors.

## Communication

### MQTT Topics

- `arduino/{device_uid}/rfid`: Authentication messages
- `arduino/{device_uid}/config`: Configuration commands
- `arduino/{device_uid}/relay`: Device control
- `arduino/{device_uid}/{sensor_type}`: Sensor data publication

## Getting Started

1. **Clone the repository**:

   ```
   git clone https://github.com/yourusername/shaas.git
   cd shaas
   ```

2. **Install dependencies**:

   - PlatformIO (recommended) or Arduino IDE
   - Required libraries: MQTT, RFID, DHT sensor, etc.

3. **Configure the system**:

   - Update `config.h` with your network credentials
   - Adjust pin definitions in hardware configuration files

4. **Build and upload**:

   - Using PlatformIO: `pio run -t upload`
   - Using Arduino IDE: Open the project and click Upload

5. **Initial setup**:
   - Connect via Bluetooth to configure WiFi settings
   - Register the first RFID card to serve as administrator

## Security Considerations

- The system implements a multi-layered security approach
- RFID cards can be added/removed via secure MQTT messages
- All configuration changes require authentication
- Physical access to the microcontroller should be restricted
- Communication can be encrypted using TLS for MQTT

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Nanopb team for the excellent Protocol Buffers implementation
- Arduino community for libraries and examples
- MQTT and PubSubClient developers

## Authors

- Mohit Sharma (Roll No. 38, Reg. No. 12300115)

---

_This project was developed as part of an academic course on Arduino For Beginners. The goal was to create a secure and efficient home automation system that can be easily extended and maintained._
