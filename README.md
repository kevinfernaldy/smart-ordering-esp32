# Smart Ordering System
Part of IF4051 Pengembangan Sistem IoT Mid-Term Exam in Institut Teknologi Bandung<br>
By : 13516109 Kevin Fernaldy, and 13516141 Ilham Wahabi

### Dependencies
The ESP32 board used in this project is a ESP32 Devkit V1 DOIT board. The code **MAY NOT WORK** with other boards.

The following library is required.
1. [**Keypad**](https://playground.arduino.cc/Code/Keypad) by Mark Stanley, Alexander Brevig
2. [**EspMQTTCleint**](https://github.com/plapointe6/EspMQTTClient) by Patrick Lapointe
3. [**TimeOut**](https://github.com/NitrofMtl/TimeOut) by Nitrof
4. [**analogWrite**](https://github.com/ERROPiX/ESP32_AnalogWrite) **Polyfill** by ERROPiX
5. [**ArduinoJSON**](https://arduinojson.org) by Benoit Blanchon

The following additional hardware is required in order to use the full capability of the program.
1. 16x2 LCD Screen 1602A
2. 4x4 Matrix Keypad
3. **Common Cathode** RGB LED 4-pins
4. 10K Ohm Potensiometer

ESP32 boards are not natively supported in Arduino. You can install Arduino ESP32 core from Espressif. The instructions can be found in this [repository](https://github.com/espressif/arduino-esp32).

### Hardware Schematic
<img src="img/hardware_schematic.png" alt="Hardware Schematic" width="700"/>

### Uploading the Code
Before uploading the code, make sure the pin connections follow the schematic above. If you want to use different pinouts, make sure to edit the pin variables inside the code.
1. Open the Arduino IDE and select the ESP32 Devkit V1 DOIT board.
2. Open the esp32_code.ino file in your Arduino IDE.
3. Connect your ESP32 board into your computer.
4. Select the correct COM gate.
5. Upload the code.
