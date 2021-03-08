//*
 * UNIVERSIDAD DEL VALLE DE GUATEMALA
 * DEPARTAMENTO DE ELECTRÓNICA
 * ELECTRÓNICA DIGITAL II
 * PROFESOR: KURT KELLNER
 * MINI PROYECTO 2: INTERCONEXIÓN ESP CON PIC
 * AUTOR : JUAN PABLO VALENZUELA
 * 
 */
//Algunas líneas de código son originales porque este código se basó en el ejemplo de arduino
/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"

/************************ Example Starts Here *******************************/

//estas son las variables de trabajo
int count = 0;
int TEMP_LSB = 0; // for incoming serial data
String led_0="a";
char envio = 0;


// configuramos los feeds
AdafruitIO_Feed *counter = io.feed("LED");
AdafruitIO_Feed *temp = io.feed("TEMP");
AdafruitIO_Feed *LED0 = io.feed("LED0");
AdafruitIO_Feed *LED1 = io.feed("LED1");
AdafruitIO_Feed *LED2 = io.feed("LED2");

//MESSAGE HANDLER, interrupciones del dashboard
void handleMessage(AdafruitIO_Data *data) {

  //Serial.print("received <- Pushbutton ");
  led_0 = data->value();
  //Serial.println(led_0);

}

void setup() {

  // iniciamos la interfaz serial
  Serial.begin(9600);

  // esperamos a que se ponga lista la comunicación
  while(! Serial);

  

  // conectamos a io.adafruit.com
  io.connect();

  // esperamos a que se conecte
  while(io.status() < AIO_CONNECTED) {
    //Serial.print(".");
    delay(500);
  }

  //Acá se ya está conectado y solo configuramos el led integrado para verificar visualmente
  pinMode(2, OUTPUT);

  //configuramos las salidas de los mensajes que entran en el dashboard
  LED0->onMessage(handleMessage);  
  LED0->get();
  LED1->onMessage(handleMessage);
  LED1->get();
  LED2->onMessage(handleMessage);
  LED2->get();
}

void loop() {

  //iniciamos la conexión al servidor para enviar datos
  io.run();

  if (Serial.available() > 0) {
    // read the incoming byte:
    TEMP_LSB = Serial.read();
    TEMP_LSB = TEMP_LSB/4;//Procesamos el dato

    //guardamos los datos que entran por comunicación serial, las interrupciones son automáticas
  }
  
  // enviamos el valor del contador al feed counter
  counter->save(count);
  
  
  // aumentamos el contador
  count++;
  if (count==151){ //se reinicia el contador
    count=0;
  }
  // Adafruit IO no puede recibir datos tan rápido y es necesario ponerle un delay de por lo menos 2 segundos
  // entre cada lectura/escritura
  digitalWrite(2,LOW);
  delay(6000);
  temp->save(TEMP_LSB);
  digitalWrite(2, HIGH);
  delay(6000);

  //seleccionamos que caracter vamos a enviar para que el pic encienda o apague una led
  if (led_0=="ON0"){
    envio = 0x0A;
  }
  if (led_0=="OFF0"){
    envio = 0xFA;
  }
  if (led_0=="ON1"){
    envio = 0x0B;
  }
  if (led_0=="OFF1"){
    envio = 0xFB;
  }
  if (led_0=="ON2"){
    envio = 0x0C;
  }
  if (led_0=="OFF2"){
    envio = 0xFC;
  }
  
  //Enviamos el dato al PIC por eusart
  //Serial.println(led_0);
  //Serial.println(envio, HEX);
  Serial.write(envio);
  
}
