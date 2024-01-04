/**
 * This Code will receive data from USART and store it inside EEPROM and after completing reading, 
 * it will retrieve data from EEPROM byte by byte and prints back to console.
 * 
 * Register addres for USART access for ATmega328P
 * UDR0   : 0xC6 , USART I/O data register
 * UBRR0H : 0xC5 , USART baud rate register high
 * UBRR0L : 0xC4 , USART baud rate register low
 * 
 * USART control and status registers
 * UCSR0B : 0xC1
 * UCSR0C : 0xC2
 * UCSR0A : 0xC0
*/

#include<stdio.h>
#include<avr/io.h>
#include<EEPROM.h>

#define FOSC 16000000UL         // Clock Speed
#define BAUD 2400               //Required baud rate
#define MYUBRR FOSC/16/BAUD-1   //Baud rate register value

//Varibles to store data for computation
unsigned int eeprom_addr;
unsigned char x;
unsigned int initial_time,current_time;
unsigned long int bits_per_sec;
    
/**
 * Function to initialize USART
*/
static void USART_Init(unsigned int ubrr)
{
    /*Set baud rate */
    UBRR0H = (unsigned char)(ubrr>>8); // 0000 0000
    UBRR0L = (unsigned char)ubrr;      // 0110 1000
    //Enable receiver and transmitter */
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);  
    /* Set frame format: 8data, 2stop bit */
    UCSR0C = (1<<USBS0)|(3<<UCSZ00); 
    //UCSR0C =0x00;
}

/**
 * Function to receive USART data
*/
static unsigned char USART_Receive(void)
{
    /* Wait for data to be received */
    while (!(UCSR0A & (1<<RXC0)));
    /* Get and return received data from buffer */
    return UDR0;
}

/**
 * Function to send data via USART
*/
static void USART_Transmit(unsigned char data)
{
    /* Wait for empty transmit buffer */
    while (!(UCSR0A & (1<<UDRE0)));
     
   /* Put data into buffer, sends the data */
    UDR0 = data;
}

void setup()
{
     USART_Init(MYUBRR);
}
 
void loop() 
{
    //Reading data from UASRT
    initial_time=0;
    printf("start\n");
    if(UCSR0A & (1<<RXC0))
    {
      for(eeprom_addr=0;eeprom_addr<EEPROM.length();eeprom_addr++)
      {
        printf("inside read\n");
        //Reading data from USART and writing to EEPROM
        x=USART_Receive();
        current_time=micros();
        EEPROM.write(eeprom_addr,x);

        //Calculating speed of communication
        bits_per_sec=8*(pow(10,6)/(current_time-initial_time));

        //Updating the initial time
        initial_time=current_time;

        printf("%llu bits/sec\n",bits_per_sec);
     }
    }

    //Reinitialization
    x='\0';
    initial_time=0;
    
    //Writing back to USART
    if(eeprom_addr==EEPROM.length())
    {
      for(eeprom_addr=0;eeprom_addr<EEPROM.length();eeprom_addr++)
      {
        x=EEPROM.read(eeprom_addr);
        USART_Transmit(x);
      }
    }
}
