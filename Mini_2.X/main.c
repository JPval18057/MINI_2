/*
 * UNIVERSIDAD DEL VALLE DE GUATEMALA
 * DEPARTAMENTO DE ELECTRÓNICA
 * ELECTRÓNICA DIGITAL II
 * PROFESOR: KURT KELLNER
 * MINI PROYECTO 2: INTERCONEXIÓN ESP CON PIC
 * AUTOR : JUAN PABLO VALENZUELA
 * 
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



//LIBRERIAS
#define _XTAL_FREQ 4000000 //frecuencia interna de 8MHz
#include <xc.h>
#include "I2CLIB.h"

//PROTOTIPOS
void IIC(void);
void config_sensor(void); //CONFIGURACION DEL SENSROR
void enviar(char dato, char reg);
unsigned int leer(char dir);

//VARIABLES
//VARIABLES DE PRUEBAS DE SISTEMA
char constant = 1;
char bit0 = 1;
char bit1 = 0;
char bit2 = 0;
//VARIABLES DE CONTROL
char sensor_dir = 0b11101100;//ID DEL SENSOR
unsigned int period = 0; //nos sirve para contar 100ms
unsigned int count = 0; //CONTADOR DE INTERRUPCIONES
char data_in = 0; //COMANDOS RECIBIDOS DEL ESP
//VARIABLES QUE ENCIENDEN LAS LUCES CON EUSART
char led0 = 0;
char led1 = 0;
char led2 = 0;
//VARIABLES DE LECTURA
char PRES_XLSB=0;
char PRES_LSB=0;
char PRES_MSB=0;
char TEMP_XLSB=0;
char TEMP_LSB=0;
char TEMP_MSB=0;


//INTERRUPCIONES
void __interrupt() ISR(void) {
    if (INTCONbits.T0IF==1){
        INTCONbits.T0IF=0;//apagar bandera
        TMR0 = 6; //periodo de 100ms con 25 interrupciones
        
        count++;
        if (count==25){
            count = 0;
            period = 1;
        }
    }
    if (PIR1bits.RCIF==1){
        PIR1bits.RCIF=0;//APAGAR LA BANDERA
        data_in = RCREG; //LEEMOS EL REGISTRO INMEDIATAMENTE
        //PORTB = constant;
        if (data_in==0x0A){
            led0 = 1;
        }
        if (data_in==0x0B){
            led1 = 1;
            led0 = 2;
        }
        if (data_in==0x0C){
            led2 = 1;
            led0 = 4;
        }
    }
    if (PIR1bits.SSPIF==1){
        PIR1bits.SSPIF=0;
    }
    return;
}

//CONFIGURACION
void setup(void){
    //CONFIGURACION DE I/O
    TRISA = 255; //PUERTO A COMO ENTRADA
    TRISB = 0b11111000;//RB0,1 Y 2 SON SALIDAS PARA LOS LEDS
    TRISC = 255; //ENTRADA
    TRISD = 255; //ENTRADA
    TRISE = 0b00001111;
    ANSEL = 0;
    ANSELH = 0;
    TRISCbits.TRISC3 = 0; //RC3 COMO SALIDA POR EL CLK OUT
    
    //COMO PRUEBA TODOS LOS PUERTOS SON ENTRADAS PARA NO ARRUINAR NADA
    //CONFIGURACION DE I2C (FUNCION)
    //IIC();
    I2C_Master_Init(9);
    
    //CONFIGURACION DE RELOJ
    OSCCON = 0b01100101; //RELOJ INTERNO A 4MHz
    
    //CONFIGURACION DE TMR0
    TMR0 = 6; //periodo de 100ms con 25 interrupciones
    OPTION_REG = 0b00000011; //PRESCALER DE 16 EN TMR0
    INTCONbits.T0IE = 1; //interrpucion encendida
    INTCONbits.T0IF = 0; //bandera apagada
    
    //CONFIGURACION DE EUSART
    //BAUD RATE FOR 9600 WITH 0.16 ERROR
    TXSTAbits.BRGH = 1;
    BAUDCTLbits.BRG16 = 0;
    SPBRG = 25;
    
    //TRANSMITTER
    TXSTAbits.TXEN = 1;
    TXSTAbits.SYNC = 0; //modo asíncrono
    RCSTAbits.SPEN = 1;
    TXSTAbits.TX9 = 0; //transmision de 8 bits
    
    //RECEIVER
    RCSTAbits.CREN = 1;
    RCSTAbits.RX9 = 0; //recibe 8 bits nada mas
    
    //ENCENDEMOS INTERRUPCIONES
    PIE1bits.RCIE=1;
    PIE1bits.TXIE=0;//NO QUEREMOS INTERRUPCIONES PARA ENVIAR
    PIR1bits.RCIF=0;//APAGAMOS LA INTERRUPCION
    
    /*
     * 0X0A ENCIENDE LED0
     * 0X0B ENCIENDE LED1
     * 0X0C ENCIENDE LED2
     * 0XFA APAGA LED0
     * 0XFB APAGA LED1
     * 0XFC APAGA LED2
     * 0XFF APAGA TODO
     * CUALQUIER OTRA COSA NO HACE NADA
     */
    
    //INTERRUPCIONES
    INTCONbits.GIE = 1; //Encendemos solo las del timer
    //ACTIVAR INTERRUPCIONES PARA EUSART ÚNICAMENTE
    
    return;
}

void main(void) {
    setup();
    while (1){
        //CODIGO PRINCIPAL DE PRUEBA
        //----------------------------------------------------------------------
        if (period==1){
            period=0;
            bit0=bit0<<1;//CORRIMIENTO DE BITS
            if (bit0==0){
                bit0=1;
            }
        //----------------------------------------------------------------------
            //MANDAR POR EUSART CADA 100ms
        TXREG = TEMP_LSB;

        //----------------------------------------------------------------------
        
        //HACER FUNCION PARA LEER EL SENSOR
            
        
        
        //----------------------------------------------------------------------            
            
        }
        //----------------------------------------------------------------------
        
        //DECODIFICACION DEL COMANDO DE ENTRADA
        //----------------------------------------------------------------------
        if (data_in==0x0A){
            led0 = 1;
        }
        if (data_in==0x0B){
            led1 = 1;
            led0 = 2;
        }
        if (data_in==0x0C){
            led2 = 1;
            led0 = 4;
        }
        if (data_in==0xFA){
            led0 = 0;
        }
        if (data_in==0xFB){
            led1 = 0;
        }
        if (data_in==0xFC){
            led2 = 0;
        }
        if (data_in==0xFF){
            led0 = 0;
            led1 = 0;
            led2 = 0;
        }
        //tests
        I2C_Master_Start();         //Start condition
        I2C_Master_Write(0b11101100);     //7 bit address + Write
        I2C_Master_Write(0xD0);    //Write register adress
        I2C_Master_Stop();          //Stop condition
        __delay_ms(200);
        I2C_Master_Start();         //Start condition
        I2C_Master_Write(0b11101101);     //7 bit address + Read
        TEMP_LSB = I2C_Master_Read(0); //Read + Acknowledge
        I2C_Master_Stop();          //Stop condition
        __delay_ms(200);

        //ACTUALIZAR PUERTO B
        //----------------------------------------------------------------------
        /*
        PORTBbits.RB0 = led0;
        PORTBbits.RB1 = led1;
        PORTBbits.RB2 = led2;
         */
        PORTB = led0;
        //----------------------------------------------------------------------
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
    //while (PIR1bits.SSPIF==0);
    //PIR1bits.SSPIF = 0; //Esperamos a que la bandera se encienda
    //CONFIGURACION MAESTRO
    config_sensor(); //configuración del sensor
    //DESACTIVAR INTERRUPCIONES
    PIE1bits.SSPIE=1;
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
    //enviamos la configuración para modo estándar
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
    //while (PIR1bits.SSPIF==0);
    //PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    
    //PRIMERO MANDAMOS LA DIRECCION
    temp = sensor_dir & 0b11111110;//FORZAMOS EL BIT0 A 0
    SSPBUF = temp;
    //ESPERAMOS EL ACKNOWLEDGE
    while (SSPCON2bits.ACKSTAT==1);
    //ESPERAMOS LA BANDERA
    //while (PIR1bits.SSPIF==0);
    //PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    
    //DESPUÉS ESCRIBIMOS EL REGISTRO AL CUAL QUEREMOS ESCRIBIR
    SSPBUF = reg;
    //ESPERAMOS EL ACKNOWLEDGE
    while (SSPCON2bits.ACKSTAT==1);
    //ESPERAMOS LA BANDERA
    //while (PIR1bits.SSPIF==0);
    //PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    
    //LUEGO ESCRIBIMOS EL DATO
    SSPBUF = dato;
    //ESPERAMOS EL ACKNOWLEGDE
    while (SSPCON2bits.ACKSTAT==1);
    //ESPERAMOS LA BANDERA
    //while (PIR1bits.SSPIF==0);
    //PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    
    //TERMINAMOS LA COMUNICACION
    SSPCON2bits.PEN = 1; //INICIA SECUENCIA DE FINAL
    //ESPERAMOS LA BANDERA
    //while (PIR1bits.SSPIF==0);
    //PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    
    return ;
}

unsigned int leer(char dir){            //LEER UN SOLO BYTE A LA VEZ
    //LEEMOS BYTE POR BYTE
    char dato;
    char temp;
    //INICIAMOS LA SECUENCIA DE INICIO
    SSPCON2bits.SEN = 1;
    //ESPERAMOS LA BANDERA
    //while (PIR1bits.SSPIF==0);
    //PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    
    //PRIMERO MANDAMOS LA DIRECCION
    temp = sensor_dir & 0b11111110;//FORZAMOS EL BIT0 A 0
    SSPBUF = temp;
    //ESPERAMOS EL ACKNOWLEDGE
    while (SSPCON2bits.ACKSTAT==1);
    //ESPERAMOS LA BANDERA
    //while (PIR1bits.SSPIF==0);
    //PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    
    //ESCRIBIMOS LA DIRECCIÓN DEL REGISTRO QUE QUEREMOS LEER
    while (SSPSTATbits.READ_WRITE==1); //ESPERAMOS A QUE LA TRANSMISIÓN TERMINE
    SSPBUF = dir;
    
    //ESPERAMOS EL ACKNOWLEDGE
    while (SSPCON2bits.ACKSTAT==1);
    //ESPERAMOS LA BANDERA
    //while (PIR1bits.SSPIF==0);
    //PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
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
    //while (PIR1bits.SSPIF==0);
    //PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    while (SSPSTATbits.READ_WRITE==1); //ESPERAMOS A QUE LA TRANSMISIÓN TERMINE
    
    
    //LEEMOS EL REGISTRO
    while (SSPSTATbits.BF==1); //ESPERAMOS A QUE ENTRE EL BYTE
    dato = SSPBUF;
    //CANCELAMOS LA TRANSMISION
    SSPCON2bits.ACKEN = 0; //NO LE MANDAMOS ACKNOWLEDGE
    SSPCON2bits.PEN = 1; //INICIAMOS STOP CONDITION
    
    //ESPERAMOS LA BANDERA
    //while (PIR1bits.SSPIF==0);
    //PIR1bits.SSPIF = 0; //APAGAMOS LA BANDERA
    
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
