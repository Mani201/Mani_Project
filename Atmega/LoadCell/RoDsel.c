#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL //atmega128 CPU 클락설정


#define DIN    (PINA&1)  // PA0 
#define SCK_1  PORTA|=2  // PA1
#define SCK_0  PORTA&=~2 // PA1

unsigned long weigth =0,offset_data; //로드셀 데이터 저장용 변수
unsigned long data1=0,data2=0,count=0; 

int offset_flag=0;

void TX0_char(char c){ while(!(UCSR0A&0x20)); UDR0=c; } //UART TX 함수
void TX0_STR(char *s){ while(*s)TX0_char(*s++); } // UART TX 함수

//hx711에서 데이터 읽는 함수
unsigned long ReadCout(){


	PORTA |= (1<<0); //hx711 DT핀 high
	PORTA &= ~(1<<1); //hx711 sck low
	
	count =0;

	while((PINA & 0x01) == 0x01); //hx711에서 데이터 전송 준비될때까지 대기
	
	//hx711의 데이터는 총 25bit데이터로 이루어짐
	for(int i=0;i<24;i++){
		PORTA |= (1<<1); // SCK high
		count = count << 1; // 1만큼 비트 이동시켜 데이터 저장
		PORTA &= ~(1<<1); // SCK low
		if((PINA & 0x01) == 0x01){ //DT핀이 High일 경우 count에 +1
		 count++;
		 }
	}
	PORTA |= (1<<1); // SCK high
	count = count^0x800000; 
	PORTA &= ~(1<<1); // SCK low

	data1 = count;

	if(offset_flag == 0){ // 처음 실행 시 offset 측정하여 제거하기위한 코드
		offset_data = data1;
		offset_flag = 1;		
	}

	if(data1 > offset_data){
		data2 = data1 - offset_data; //data2에 측정된 데이터 저장됨
	}
	else{
		data2 = 0;
	}
	data1=0;
	
	return data2;

}
void HX711init(){
  DDRA=0x02;                   // SCK // PA1
  SCK_1; _delay_us(100); SCK_0; // 60us 이상 POWER DOWN 후 Nomal Mode
  
}
//UART tx 함수 (48을 더하는 이유는 아스키 코드값때문)
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
	
	//UART 레지스터 설정
	UCSR0B=0x08; 
	UBRR0L=103;  //  9600

	HX711init();

	while(1){

	weigth = ReadCout()/4; //로드셀 측정값 저장 ( 4를 나눈건 로드셀 스케일때문 )

	tx_init(weigth);
	TX0_char('\n');
	TX0_char('\r');
	
	//아래 if문에서 설정값 변경가능 ( ex : 3kg일경우 weigth의 값은 30000이 됨 )
	if((weigth/1000) >= 3 ){
		PORTB = 0x01;
	}
	else PORTB = 0x02;

	}
}
