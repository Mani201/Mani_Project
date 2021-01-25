#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL //atmega128 CPU Ŭ������


#define DIN    (PINA&1)  // PA0 
#define SCK_1  PORTA|=2  // PA1
#define SCK_0  PORTA&=~2 // PA1

unsigned long weigth =0,offset_data; //�ε弿 ������ ����� ����
unsigned long data1=0,data2=0,count=0; 

int offset_flag=0;

void TX0_char(char c){ while(!(UCSR0A&0x20)); UDR0=c; } //UART TX �Լ�
void TX0_STR(char *s){ while(*s)TX0_char(*s++); } // UART TX �Լ�

//hx711���� ������ �д� �Լ�
unsigned long ReadCout(){


	PORTA |= (1<<0); //hx711 DT�� high
	PORTA &= ~(1<<1); //hx711 sck low
	
	count =0;

	while((PINA & 0x01) == 0x01); //hx711���� ������ ���� �غ�ɶ����� ���
	
	//hx711�� �����ʹ� �� 25bit�����ͷ� �̷����
	for(int i=0;i<24;i++){
		PORTA |= (1<<1); // SCK high
		count = count << 1; // 1��ŭ ��Ʈ �̵����� ������ ����
		PORTA &= ~(1<<1); // SCK low
		if((PINA & 0x01) == 0x01){ //DT���� High�� ��� count�� +1
		 count++;
		 }
	}
	PORTA |= (1<<1); // SCK high
	count = count^0x800000; 
	PORTA &= ~(1<<1); // SCK low

	data1 = count;

	if(offset_flag == 0){ // ó�� ���� �� offset �����Ͽ� �����ϱ����� �ڵ�
		offset_data = data1;
		offset_flag = 1;		
	}

	if(data1 > offset_data){
		data2 = data1 - offset_data; //data2�� ������ ������ �����
	}
	else{
		data2 = 0;
	}
	data1=0;
	
	return data2;

}
void HX711init(){
  DDRA=0x02;                   // SCK // PA1
  SCK_1; _delay_us(100); SCK_0; // 60us �̻� POWER DOWN �� Nomal Mode
  
}
//UART tx �Լ� (48�� ���ϴ� ������ �ƽ�Ű �ڵ尪����)
void tx_init(unsigned long data){
	unsigned long temp;

	temp = data/10000;
	TX0_char(temp+48);
	temp = (data%10000)/1000;
	TX0_char(temp+48);
	temp = (data%1000)/100;
	TX0_char(temp+48);
	temp = (data%100)/10;
	TX0_char(temp+48);
	temp = (data%10);
	TX0_char(temp+48);
}
int main(void){

	DDRB = 0xff;
	
	//UART �������� ����
	UCSR0B=0x08; 
	UBRR0L=103;  //  9600

	HX711init();

	while(1){

	weigth = ReadCout()/4; //�ε弿 ������ ���� ( 4�� ������ �ε弿 �����϶��� )

	tx_init(weigth);
	TX0_char('\n');
	TX0_char('\r');
	
	//�Ʒ� if������ ������ ���氡�� ( ex : 3kg�ϰ�� weigth�� ���� 30000�� �� )
	if((weigth/1000) >= 3 ){
		PORTB = 0x01;
	}
	else PORTB = 0x02;

	}
}
