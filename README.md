#  IoT Fire-Fighting Robot System

An IoT-based fire-fighting robot designed for real-time monitoring and automatic fire suppression.  
The system integrates embedded hardware with wireless communication, enabling both manual control and autonomous operation.

---

##  Overview

This project aims to develop a smart fire-fighting robot capable of:
- Detecting fire using sensors
- Automatically activating a water pump
- Allowing real-time remote control via web interface
- Ensuring reliable communication between microcontrollers

The system combines **ESP32**, **Arduino Uno**, and multiple sensors to create a complete embedded solution.

---

##  System Architecture

User (Web Interface)
↓
ESP32 (Main Controller)
↓ UART
Arduino Uno (Sensor + Pump Control)
↓
Actuators (Motor, Pump, Servo)


---

##  Technologies Used

- **Microcontrollers:** ESP32, Arduino Uno  
- **Programming:** C/C++ (Arduino IDE)  
- **Communication:** UART (Serial Communication)  
- **Web Communication:** WebSocket  
- **Hardware Modules:**  
  - L298N Motor Driver  
  - Flame Sensor  
  - Water Pump  
  - Servo Motors  
- **Tools:** Arduino IDE, PlatformIO  

---

##  Features

###  Automatic Mode
- Flame sensor continuously monitors the environment  
- Automatically triggers water pump when fire is detected  
- Response time: ~2 seconds  

###  Manual Mode
- Control robot via web interface  
- Move robot (forward, backward, left, right)  
- Manually turn pump ON/OFF  

###  Communication System
- UART communication between ESP32 and Arduino Uno  
- Command-based protocol (e.g., "ON", "OFF")  
- Error-checking mechanism for reliable transmission  

---

##  Hardware Setup

### Components:
- ESP32 (main controller)
- Arduino Uno (peripheral controller)
- Flame Sensor
- ESP32 Motor Driver
- Water Pump
- Servo Motors
- Power Supply

### Connections:
- ESP32 ↔ Arduino via UART (TX/RX)
- Arduino reads sensor data and controls pump
- ESP32 handles WiFi and user interface

---

##  System Workflow

1. Flame sensor detects fire  
2. Arduino processes sensor signal  
3. Sends signal to ESP32 via UART  
4. ESP32:
   - Logs data  
   - Activates pump (auto mode)  
   - Updates web interface  
5. User can override via manual control  

---

##  Performance

- Communication reliability: ~99%  
- Fire detection range: ~1 meter  
- Pump activation time: < 2 seconds  

---

##  Testing & Debugging

- Tested UART communication under continuous load  
- Debugged hardware issues (voltage mismatch, signal noise)  
- Optimized system response and stability  

---

##  Challenges

- Synchronizing UART communication between two microcontrollers  
- Handling hardware noise and unstable signals  
- Ensuring real-time responsiveness for manual control  

---

##  Future Improvements

- Add camera + computer vision for fire detection  
- Integrate cloud monitoring (MQTT / Firebase)  
- Improve battery efficiency and mobility  
- Add AI-based decision-making  

---

##  Author

**Nguyen The Tuyen**  
- Role: Embedded & System Developer  
- Developed entire system independently  

---


---

##  Notes

This project demonstrates:
- Embedded system design  
- Hardware-software integration  
- Real-time communication  
- IoT system development  
