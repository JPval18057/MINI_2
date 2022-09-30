# Weather Project
I2C Mini Project Repository

A PIC16F887 microcontroller, an ESP32, 3mm blue LED lights, a variety of resistors, 24 gauge solid core wire, a BMP280 pressure and temperature sensor and a Pickit 3 were used.

The function of the project is to read the data from the sensor with the PIC microcontroller, next send it to the ESP32. Then the ESP32 sends it to the adafruit server. On the server there is an interface that the user modifies through the adafruit website. The user can turn on lights from the web page and view the measurements from the sensors.

On this occasion the PIC was configured to work at 3V to avoid using the bidirectional converter. The PIC datasheet states that the PIC runs at that voltage if you work with frequencies less than 8MHz, which is the absolute maximum. In this project we worked at 4MHz to be in a safe margin.
