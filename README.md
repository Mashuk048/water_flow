# Water Flow Sensing Device

## Overview

This project involves a water flow sensing device that utilizes a 2" diameter analog sensor connected to an ESP8266 module. The device connects to a Wi-Fi network and sends real-time data to an MQTT broker, allowing for efficient monitoring and control of water flow.

## Features

- **Analog Water Flow Sensor**: Measures water flow with a 2" diameter.
- **ESP8266 Module**: Connects to Wi-Fi for data transmission.
- **MQTT Protocol**: Sends data to a lightweight, publish/subscribe MQTT broker.
- **Real-Time Monitoring**: Receive live updates on water flow metrics.

## Hardware Required

- 2" Diameter Analog Water Flow Sensor
- ESP8266 Wi-Fi Module
- Breadboard and jumper wires
- Power supply (5V for ESP8266)
- (Optional) MQTT Broker (e.g., Mosquitto)

## Software Required

- Arduino IDE (or any compatible IDE)
- Libraries:
  - PubSubClient (for MQTT communication)
  - ESP8266WiFi (for Wi-Fi connectivity)

## Installation

1. **Wiring the Hardware**:
   - Connect the water flow sensor to the ESP8266 according to the sensor's datasheet.
   - Ensure power connections are secure and the ESP8266 is powered.

2. **Setting Up the Software**:
   - Install the Arduino IDE.
   - Add the ESP8266 board manager.
   - Install the required libraries through the Library Manager.

3. **Configuring the Code**:
   - Clone this repository or download the source code.
   - Update the `config.h` file with your Wi-Fi credentials and MQTT broker details.
   - Upload the code to the ESP8266 using the Arduino IDE.

## Usage

Once everything is set up, the device will start sending water flow data to the specified MQTT broker. You can subscribe to the relevant topic to monitor the flow readings in real-time.

## MQTT Topics

- **Flow Data**: `/water/flow`(Customise as you like)
- **Status Updates**: `/water/status`(Customise as you like)

## Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue for any improvements or suggestions.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Special thanks to the open-source community for the libraries and resources that made this project possible.
- Inspiration from various IoT projects and tutorials available online.

## Contact

For any inquiries, please contact Md. Mashuk E-lahi(mail to:meer.mashuk048@gmail.com).

