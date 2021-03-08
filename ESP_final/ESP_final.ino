//*
 * UNIVERSIDAD DEL VALLE DE GUATEMALA
 * DEPARTAMENTO DE ELECTRÓNICA
 * ELECTRÓNICA DIGITAL II
 * PROFESOR: KURT KELLNER
 * MINI PROYECTO 2: INTERCONEXIÓN ESP CON PIC
 * AUTOR : JUAN PABLO VALENZUELA
 * 
 */

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"

/************************ Example Starts Here *******************************/

// this int will hold the current count for our sketch
int count = 0;
int TEMP_LSB = 0; // for incoming serial data
String led_0="a";
char envio = 0;


// set up the 'counter' feed
AdafruitIO_Feed *counter = io.feed("LED");
AdafruitIO_Feed *temp = io.feed("TEMP");
AdafruitIO_Feed *LED0 = io.feed("LED0");
AdafruitIO_Feed *LED1 = io.feed("LED1");
AdafruitIO_Feed *LED2 = io.feed("LED2");

//MESSAGE HANDLER
void handleMessage(AdafruitIO_Data *data) {

  //Serial.print("received <- Pushbutton ");
  led_0 = data->value();
  //Serial.println(led_0);

}

void setup() {

  // start the serial connection
  Serial.begin(9600);

  // wait for serial monitor to open
  while(! Serial);

  //Serial.print("Connecting to Adafruit IO");

  // connect to io.adafruit.com
  io.connect();

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    //Serial.print(".");
    delay(500);
  }

  // we are connected
  //Serial.println();
  //Serial.println(io.statusText());
  pinMode(2, OUTPUT);

  LED0->onMessage(handleMessage);  
  LED0->get();
  LED1->onMessage(handleMessage);
  LED1->get();
  LED2->onMessage(handleMessage);
  LED2->get();
}

void loop() {

  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();

  if (Serial.available() > 0) {
    // read the incoming byte:
    TEMP_LSB = Serial.read();
    TEMP_LSB = TEMP_LSB/4;//Procesamos el dato

    // say what you got:
    //Serial.print("I received: ");
    //Serial.println(TEMP_LSB, HEX);
  }
  
  // save count to the 'counter' feed on Adafruit IO
  //Serial.print("sending -> ");
  //Serial.println(count);
  counter->save(count);
  
  
  // increment the count by 1
  count++;
  if (count==151){ //RESETING THE COUNTER
    count=0;
  }
  // Adafruit IO is rate limited for publishing, so a delay is required in
  // between feed->save events. In this example, we will wait three seconds
  // (1000 milliseconds == 1 second) during each loop.
  digitalWrite(2,LOW);
  delay(6000);
  temp->save(TEMP_LSB);
  digitalWrite(2, HIGH);
  delay(6000);

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
  //Serial.println(led_0);
  //Serial.println(envio, HEX);
  Serial.write(envio);
  
}
