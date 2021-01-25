
/*
	PA0~7 : D0~D7
	PC1: RS  PC2: RW PC3: E
	PD0~3 : stepmoter
	PB0 : dust sensor LED 
	PF0 : dust sensor ADC
*/

#define F_CPU 16000000UL
#include "avr/io.h"
#include "avr/iom128.h"
#include "util/delay.h"
 
//���� ���� ���� ������ ���� ������ ������
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

int moter_cnt=0;
int moter_stat=1;
unsigned char time=0;

int flag = 1;

// ������ ���� ȸ�� �ڵ�
int moter_start(int dat){
	for(int i=0;i<dat;i++){
	// �Ʒ� �ڵ带 50�� �ݺ��ϸ� 360�� ȸ���� ( �Ʒ��ڵ� �ѹ� ���� �� 7.2�� ȸ��)
		for(int j=0;j<50;j++){
		PORTD = 0x03;
		_delay_ms(5);
	
		PORTD = 0x06;
		_delay_ms(5);
		
		PORTD = 0x0c;
		_delay_ms(5);
		
		PORTD = 0x09 ;
		_delay_ms(5);
		}
	}
	//���� ������ ��� ���� ���ϸ� ���Ϳ� ����ȭ �ɷ��� �߿� ������
	PORTD= 0x00;
}
// ������ ���� ȸ�� �ڵ�
int moter_back(int dat){
	for(int i=0;i<dat;i++){
		for(int j=0;j<50;j++){
		PORTD = 0x09;
		_delay_ms(5);
	
		PORTD = 0x0c;
		_delay_ms(5);
		
		PORTD = 0x06;
		_delay_ms(5);
		
		PORTD = 0x03 ;
		_delay_ms(5);
		}
	}
	PORTD= 0x00;
}
// �������� ���� ������ �� ��� �ڵ�
 int get_value(void)
{
   ADMUX |= 0x40; //ADC �������� ����
    PORTB = 0x00; // �������� LED On
	_delay_us(280);
	ADCSRA = 0xc7; //adc enable, start, prescaler 128
    while(ADCSRA&(1 << ADIF) == 0); // ��ȯ�Ϸ� �� ������ ���
    _delay_us(40);
	PORTB = 0x01; // �������� LED Off
	_delay_us(9680);
    return ADC; // ADC�� ��ȯ
}

void LCD_controller(unsigned char control)
{
    _delay_ms(30);
    PORTC = 0x00; //RW clear.
    _delay_us(0.05); //RW & RS Setup time is 40ns min.
    PORTC = 0b00000100; // E set.
    _delay_us(0.1); //Data Setup time is 80ns min.
    PORTA = control; // Data input.
    _delay_us(0.3); // valid data is 130ns min.
    PORTC = 0b00000011; // RS set. RW set. E clear.
}
 
void LCD_data(unsigned char Data)
{
    _delay_ms(1);
    PORTC = 0b00000001; //RS set. RW clear. E clear.
    _delay_us(0.05); //RW & RS Setup time is 40ns min.
    PORTC = 0b00000101; // E set.
    _delay_us(0.1); //Data Setup time is 80ns min.
    PORTA = Data; // Data input.
    _delay_us(0.5); // valid data min is 130ns.
    PORTC = 0b00000010; // RS clear. RW set. E clear.
}

void LCD_string(unsigned char address, unsigned char *Str)
{
    int i=0;
    LCD_controller(address); // LCD display start position
    while(*Str != '\0')
    {
        if(address+i == 0x90)
            LCD_controller(0xc0); // second line display
        LCD_data(*Str);
        i++;
        Str++;
    }
}
 
void LCD_initialize(void)
{
    /* 8bit interface mode */
    _delay_ms(50);
    LCD_controller(0x3c); // Function set. Use 2-line, display on.
    _delay_us(40); // wait for more than 39us.
    LCD_controller(0x0f); // Display ON/OFF Control. display,cursor,blink on
    _delay_us(40); // wait for more than 39us.
    LCD_controller(0x01); // Display Clear.
    _delay_ms(1.53); // wait for more than 1.53ms.
    LCD_controller(0x06); // Entry Mode Set. I/D increment mode, entire shift off
}
 
void TX0_char(char c){ while(!(UCSR0A&0x20)); UDR0=c; } //UART TX �Լ�
void TX0_STR(char *s){ while(*s)TX0_char(*s++); } // UART TX �Լ�


int main()
{
	int dust_Average;
	int dust_sum;
	int dust_num=0;
 	int res = 0;
    unsigned char str[4];
    int i=0;
    char print_Arr[16];
	float dust;
	float dust_v;
   

	DDRB = 0xff; // Dust sensor 
    DDRC = 0xff; // Control_bit
    DDRA = 0xff; // Data_bit
	DDRD = 0xff; // setp moter
 
    LCD_initialize(); // �ʱ�ȭ �ѹ� �����ְ�
    
  
    UCSR0B=0x08; 
	UBRR0L=103;  //  9600
  	PORTB= 0xff;
        
    while(1)  
    {  

		res = get_value(); // ������ �޾ƿ��� 0~1024 ������ ��
        dust = res;
		dust_v = dust * (5.0 / 1024.0); // 5V ���� ����ϴϱ� ������ �� 1�� V�� ���
		dust = dust_v * 0.17 - 0.1; // ���������� ������ ��� ��
		res = (int)(dust * 100); // ������ �Ҽ��� �����ϱ� X100

		
		dust_sum+= res; // ��� �������� ������ ���ϱ�
		dust_num++;

		if(dust_num == 10){  // 10�� ���ϸ� ��ճ���
			dust_Average = dust_sum / dust_num;
			dust_num = 0;
			dust_sum =0; 
		}
		//��հ��� ���� ��ġ �̻��� ��� ������ ���� ����
		if(dust_Average >= 25 && moter_stat == 1){
			moter_start(10);
			moter_stat = 2;
		}
		//��հ��� ������ġ ������ ��� ������ ���� ����
		else if(dust_Average <25 && moter_stat == 2){
			moter_back(10);
			moter_stat = 1;
		}
		
		//���������� ���� �Ҽ��� ���ڸ��� ��
		if(res/10 == 0){
		sprintf(print_Arr,"dust:0.0%d  ",res);
		TX0_STR(print_Arr);
		LCD_string(0x80, print_Arr);
		}
		//���������� ���� ������ ���ڸ��� �� 
		else{
		sprintf(print_Arr,"dust:0.%d  ",res);
		TX0_STR(print_Arr);
		LCD_string(0x80, print_Arr);
		}
        
 _delay_ms(1000);
		
       
		
    }  
    return 0;
}
