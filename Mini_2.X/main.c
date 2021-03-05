/*
 * File:   main.c
 * Author: cabal
 *
 * Created on 4 de marzo de 2021, 04:33 PM
 */

// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>

//LIBRERIAS
#define _XTAL_FREQ 4000000 //frecuencia interna de 8MHz

//PROTOTIPOS
void IIC(void);
void config_sensor(void); //CONFIGURACION DEL SENSROR
void enviar(char dato, char reg);
unsigned int leer(char dir);

//VARIABLES
char bit0 = 0;
char bit1 = 0;
char bit2 = 0;
char sensor_dir = 0b11101100;


//CONFIGURACION
void setup(void){
    //CONFIGURACION DE I/O
    TRISA = 255; //PUERTO A COMO ENTRADA
    TRISB = 0b11111000;//RB0,1 Y 2 SON SALIDAS PARA LOS LEDS
    TRISC = 255; //ENTRADA
    TRISD = 255; //ENTRADA
    TRISE = 0b00001111;
    TRISCbits.TRISC3 = 0; //RC3 COMO SALIDA POR EL CLK OUT
    //COMO PRUEBA TODOS LOS PUERTOS SON ENTRADAS PARA NO ARRUINAR NADA
    //CONFIGURACION DE I2C (FUNCION)
    
    //CONFIGURACION DE RELOJ
    OSCCON = 0b01100101; //RELOJ INTERNO A 4MHz
    
    //CONFIGURACION DE EUSART
    
    
    //INTERRUPCIONES
    INTCONbits.GIE = 0; //apagamos todas las interrupciones para mientras
    //ACTIVAR INTERRUPCIONES PARA EUSART ÚNICAMENTE
    
    return;
}

void main(void) {
    setup();
    while (1){

        bit0 = 1;
        PORTBbits.RB0 = bit0;
        __delay_ms(500);
        bit1 = 1;
        PORTBbits.RB1 = bit1;
        __delay_ms(500);
        bit2 = 1;
        PORTBbits.RB2 = bit2;
        __delay_ms(500);
        bit0 = 0;
        bit1 = 0;
        bit2 = 0;

    }
    return;
}
void IIC(void){
    //The 7-bit device address is 111011x
    //To select I2C CSB must be at 1 or 3V
    //PIC CONFIG
    sensor_dir = 0b1110110; //el último bit es para R/W
    SSPADD = 9; //PARA UN RELOJ DE 100KHz con reloj de 4MHz
    SSPSTATbits.SMP = 1 ; //ES 0 PARA 400Hz de clock y 1 para 100kHz y 1MHz
    SSPCON = 0b00101000; //IIC MASTER MODE ENABLED
    while (PIR1bits.SSPIF);
    PIR1bits.SSPIF = 0; //Esperamos a que la bandera se encienda
    //CONFIGURACION MAESTRO
    
    /*
     * TO DO LIST:
     * CONFIGURE IIC IN PIC
     * SEND CONFIGURATION TO SENSOR ONLY ONCE IN SETUP
     * READ THE SENSOR EVERY 100ms
     * FOR TESTING PURPOSES JUST READ TEMP
     */
    return;
}

void config_sensor(void){
    //ESCRIBIMOS AL REGISTRO CRTL_MEA
    //ctrl_meas = 0b00101111; para modo standard normal
    //ctrl_mea = 0xF4
    enviar(0b00101111,0xF4); //leemos presión y temperatura
    return;
}


void enviar(char dato, char reg){
    //APAGAMOS EL READ MODE POR SI ACASO
    SSPCON2bits.RCEN = 0;
    
    
    //SI RW ES 0 ENTONCES ESCRIBIMOS
    char temp;
    //INICIAMOS LA SECUENCIA DE INICIO
    SSPCON2bits.SEN = 1;
    //ESPERAMOS LA BANDERA
    while (PIR1bits.SSPIF);
    PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    
    //PRIMERO MANDAMOS LA DIRECCION
    temp = sensor_dir & 0b11111110;//FORZAMOS EL BIT0 A 0
    SSPBUF = temp;
    //ESPERAMOS EL ACKNOWLEDGE
    while (!SSPCON2bits.ACKSTAT);
    //ESPERAMOS LA BANDERA
    while (PIR1bits.SSPIF);
    PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    
    //DESPUÉS ESCRIBIMOS EL REGISTRO AL CUAL QUEREMOS ESCRIBIR
    SSPBUF = reg;
    //ESPERAMOS EL ACKNOWLEDGE
    while (!SSPCON2bits.ACKSTAT);
    //ESPERAMOS LA BANDERA
    while (PIR1bits.SSPIF);
    PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    
    //LUEGO ESCRIBIMOS EL DATO
    SSPBUF = dato;
    //ESPERAMOS EL ACKNOWLEGDE
    while (!SSPCON2bits.ACKSTAT);
    //ESPERAMOS LA BANDERA
    while (PIR1bits.SSPIF);
    PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    
    //TERMINAMOS LA COMUNICACION
    SSPCON2bits.PEN = 1; //INICIA SECUENCIA DE FINAL
    //ESPERAMOS LA BANDERA
    while (PIR1bits.SSPIF);
    PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    
    return ;
}

unsigned int leer(char dir){
    //LEEMOS BYTE POR BYTE
    char dato;
    char temp;
    //INICIAMOS LA SECUENCIA DE INICIO
    SSPCON2bits.SEN = 1;
    //ESPERAMOS LA BANDERA
    while (PIR1bits.SSPIF==0);
    PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    
    //PRIMERO MANDAMOS LA DIRECCION
    temp = sensor_dir & 0b11111110;//FORZAMOS EL BIT0 A 0
    SSPBUF = temp;
    //ESPERAMOS EL ACKNOWLEDGE
    while (SSPCON2bits.ACKSTAT==1);
    //ESPERAMOS LA BANDERA
    while (PIR1bits.SSPIF==0);
    PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    
    //ESCRIBIMOS LA DIRECCIÓN DEL REGISTRO QUE QUEREMOS LEER
    while (SSPSTATbits.READ_WRITE==1); //ESPERAMOS A QUE LA TRANSMISIÓN TERMINE
    SSPBUF = dir;
    
    //ESPERAMOS EL ACKNOWLEDGE
    while (SSPCON2bits.ACKSTAT==1);
    //ESPERAMOS LA BANDERA
    while (PIR1bits.SSPIF==0);
    PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    while (SSPSTATbits.READ_WRITE==1); //ESPERAMOS A QUE LA TRANSMISIÓN TERMINE
    
    //LEEMOS
    
    //INICIAMOS REPEATED START CONDITION
    SSPCON2bits.RSEN =1;
    while (SSPCON2bits.RSEN ==1); //ESPERAMOS A QUE TERMINE
    //DIRECCION DEL DISPOSITIVO EN READ MODE
    temp = sensor_dir | 0b00000001;//FORZAMOS EL BIT0 A 1
    SSPBUF = temp;
    
    //ESPERAMOS EL ACKNOWLEDGE
    while (SSPCON2bits.ACKSTAT==1);
    
    //MASTER READ MODE
    SSPCON2bits.RCEN = 1;
    
    //ESPERAMOS LA BANDERA
    while (PIR1bits.SSPIF==0);
    PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    while (SSPSTATbits.READ_WRITE==1); //ESPERAMOS A QUE LA TRANSMISIÓN TERMINE
    
    
    //LEEMOS EL REGISTRO
    while (SSPSTATbits.BF==0); //ESPERAMOS A QUE ENTRE EL BYTE
    dato = SSPBUF;
    //CANCELAMOS LA TRANSMISION
    SSPCON2bits.ACKEN = 0; //NO LE MANDAMOS ACKNOWLEDGE
    SSPCON2bits.PEN = 1; //INICIAMOS STOP CONDITION
    
    //APAGAMOS EL READ MODE
    SSPCON2bits.RCEN = 0;
    
    return dato;
}

/*
 * REGISTROS DEL SENSOR
 * DIRECCION - REGISTRO
 * 0xFC         TEMP_XLSB
 * 0xFB         TEMP_LSB
 * 0xFA         TEMP_MSB
 * 0xF9         PRES_XLSB
 * 0xF8         PRES_LSB
 * 0xF7         PRES_MSB
 * 0xF4         CTRL_MEAS
 * 
 * PARA LEER:
 * char PRES_XLSB;
 * char PRES_LSB;
 * char PRES_MSB;
 * PRES_XLSB = leer(0xF9);
 * PRES_LSB = leer(0xF8);
 * PRES_MSB = leer(0xF7);
 * LUEGO PROCESARLO EN EL ESP
 * 
 * CTRL MEAS SIRVE PARA CONFIGURAR LA PRESICION
 * Temperature - PRESSURE - t
 * x1               x4      11 (normal mode)
 * 001              011     11
 * ctrl_meas = 0b00101111; para modo standard normal (posiblemente no sea necesario ajustar)
 * read every 100ms to be safe, can be more
 */