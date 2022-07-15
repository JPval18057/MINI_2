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
        __delay_us(10);
        TXREG = TEMP_MSB;
        __delay_us(10);
        TXREG = TEMP_XLSB;

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
        //----------------------------------------------------------------------
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
        
        I2C_Master_Start();         //Start condition
        I2C_Master_Write(0b11101100);     //7 bit address + Write
        I2C_Master_Write(0xFA);    //Write register adress
        I2C_Master_Stop();          //Stop condition
        __delay_ms(200);
        I2C_Master_Start();         //Start condition
        I2C_Master_Write(0b11101101);     //7 bit address + Read
        TEMP_MSB = I2C_Master_Read(0); //Read + Acknowledge
        I2C_Master_Stop();          //Stop condition
        __delay_ms(200);
        
        I2C_Master_Start();         //Start condition
        I2C_Master_Write(0b11101100);     //7 bit address + Write
        I2C_Master_Write(0xFC);    //Write register adress
        I2C_Master_Stop();          //Stop condition
        __delay_ms(200);
        I2C_Master_Start();         //Start condition
        I2C_Master_Write(0b11101101);     //7 bit address + Read
        TEMP_XLSB = I2C_Master_Read(0); //Read + Acknowledge
        I2C_Master_Stop();          //Stop condition
        __delay_ms(200);

        //ACTUALIZAR PUERTO B
        //----------------------------------------------------------------------
        
        PORTB = led0;
        //----------------------------------------------------------------------
    }
    return;
}






















