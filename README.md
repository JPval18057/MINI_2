# MINI_2
Repositorio del mini proyecto I2C

Se utilizó un microcontrolador PIC16F887, un ESP32, luces LED azules de 3mm, resistencias varias, cable calibre 24 de núcleo sólido, un sensor de presión y temperatura BMP280 y un Pickit 3.

La función del proyecto es leer los datos del sensor con el PIC, luego mandarlos al ESP32 y el ESP32 los envía al servidor de adafruit. En el servidor hay una interfaz que el usuario modifica mediante la página web de adafruit. El usuario puede encender luces desde la página web y ver las mediciones de los sensores.

En esta ocasión se configuró el PIC para funcionar a 3V para evitar usar el conversor bidireccional.
