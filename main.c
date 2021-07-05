/*
 * 4bitmode.c
 *
 * Created: 12/7/2018 5:30:05 PM
 * Author : User
 */ 
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

#define RS PD0				
#define E PD1 				
#define data_pin PC0
uint8_t c,humI,humD,tempI,tempD,CheckSum;
uint8_t receive_data();
int num;

void send_request();
void receive_response();
void lcd_init();
void send_command(unsigned char cmd);
void send_data(unsigned char data);
void lcd_print(char*str);
void lcd_gotoxy(unsigned char x, unsigned char y);


int main(void)
{
	DDRD = 0xff; //port D for LCD
		
	lcd_init();
	lcd_print("Temp.:");
	lcd_print("C");
	
	lcd_gotoxy(1,2);
	lcd_print("Humi.:");
	lcd_print("%");
	
	while (1)
	{
		send_request();
		receive_response();
		
		humI=receive_data();	// store first eight bits in humI
		humD=receive_data();	// store next eight bits in humD
		tempI=receive_data();	// store next eight bits in tempI
		tempD=receive_data();	// store next eight bits in tempD
		CheckSum=receive_data();// store next eight bits in CheckSum		
		
		
		if ((humI+humD+tempI+tempD) == CheckSum)
		{
			char str[10];
			lcd_gotoxy(8,1);
			num = (int)	tempI; //convert from binary to integer
			lcd_print(itoa(num,str,10)); //convert from integer to string to display number on the screen
			lcd_print(".");
			num = (int)	tempD;
			lcd_print(itoa(num,str,10));
			
			lcd_gotoxy(8,2);
			num = (int)	humI;
			lcd_print(itoa(num,str,10));
			lcd_print(".");
			num = (int)	humD;
			lcd_print(itoa(num,str,10));
			
		}
		
		_delay_ms(2000); //wait 2s to read second data

	}
} //end main

void send_request(){
		
	DDRC |= (1<<data_pin); //define data_pin as output
	PORTC &= ~(1<<data_pin); //set to low pin
	_delay_ms(20);
	PORTC |= (1<<data_pin); //pull up
	//_delay_us(40);
		
}

void receive_response(){
	
	DDRC &= ~(1<<data_pin);				//define data_pin as input
	while(PINC & (1<<data_pin));		//high 
	while((PINC & (1<<data_pin))==0);	//low
	while(PINC & (1<<data_pin));		//high		
}

uint8_t receive_data()			//receive data
{
	for (int q=0; q<8; q++)
	{//check whether zero or one
		while((PINC & (1<<data_pin)) == 0); //now zero wait for high
		_delay_us(30);
		if(PINC & (1<<data_pin))			// still high
		c = (c<<1)|(0x01);					// then its logic HIGH 
		else								// become low
		c = (c<<1);							// then it is logic LOW
		while(PINC & (1<<data_pin));		// wait for 0 means that this bit transmission is over
	}
	return c;
}

void lcd_init(){
	send_command(0x33);
	send_command(0x32);
	send_command(0x28);
	send_command(0x0E);
	send_command(0x01);
	_delay_ms(1000);
}

void send_command(unsigned char cmd){
	
	unsigned char i,j;
	
	i=cmd & 0xf0; //send higher nibble
	PORTD = i;
	PORTD &= ~ (1<<RS); //RS = 0 ; commmand
	PORTD |=  (1<<E); //send pulse
	_delay_ms(1000);
	PORTD &= ~(1<<E);
	
	j=cmd<<4; //send lower nibble
	PORTD = j;
	PORTD &= ~ (1<<RS); //RS = 0 ; commmand
	PORTD |=  (1<<E); //send pulse
	_delay_ms(1000);
	PORTD &= ~(1<<E);
}

void send_data(unsigned char data){
	
	unsigned char i,j;
	
	i=data & 0xf0;//send higher nibble
	PORTD =i;
	PORTD |= (1<<RS); //RS = 1 ; Data
	PORTD |=  (1<<E); //send pulse
	_delay_ms(1000);
	PORTD &= ~(1<<E);
	
	j=data<<4; //send lower nibble
	PORTD =j;
	PORTD |= (1<<RS); //RS = 1 ; Data
	PORTD |=  (1<<E); //send pulse
	_delay_ms(1000);
	PORTD &= ~(1<<E);
}

void lcd_print(char*str){
	unsigned char i=0;
	while (str[i]!=0)
	{
		send_data(str[i]);
		i++;
	}
}

void lcd_gotoxy(unsigned char x, unsigned char y){
	unsigned char firstCharAdr[]={0x80, 0xC0, 0x94, 0xD4};
	send_command(firstCharAdr[y-1]+x-1);
	_delay_us(100);	
}


