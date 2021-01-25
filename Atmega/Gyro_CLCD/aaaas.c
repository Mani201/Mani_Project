//MPU-6050에 I2C방식으로 데이터를 읽어와 LCD에 출력한다. 
 
 
#include <mega128.h> 
 #include <delay.h> 
 #include <stdio.h> 
 #include <stdlib.h> 
 
#define LCD_RS      PORTA.0         // 명령어/데이터 구분 명령어      
#define LCD_RW      PORTA.1         // READ/WRITE 신호 
#define LCD_EN      PORTA.2         // ENABLE 신호 
 
#define FUNCSET       0x28          // Function Set 
#define ENTMODE       0x06          // Entry Mode Set 
#define ALLCLR        0x01          // All Clear 
#define HOME          0x02          // HOME 
#define DISPON        0x0c          // Display On 
#define LINE2         0xC0         // 2nd Line Move 
 
// MPU6050 레지스터 어드레스 및 디바이스 어드레스 
#define MPU6050_ACCEL_XOUT_H    0x3B    // R 
#define MPU6050_TEMP_OUT_H      0x41    // R    
 #define MPU6050_GYRO_XOUT_H     0x43B   // R   
 #define MPU6050_PWR_MGMT_1      0x6B    // R/W    
 #define MPU6050_I2C_ADDR        0x68    // 디바이스 어드레스 
 
typedef unsigned char uint8; 
 
uint8   mpudata[14];                    // MPU6050 센서값 저장                     
int    y_accel;      // 가속도값

char lcd_arr[2][16]; //lcd출력 배열
char lcd_full = 0xff; // 하얀색 사각형 출력 값              
char lcd_void = 0xfe; // 빈 사각형 출력 값
int lcd_cnt=-1,lcd_cnt_line=0; // 현재 사각형의 위치를 저장하는 변수
int test=0;
int lcd_cnt_temp=0,lcd_cnt_line_temp=0; // 이전 사각형의 위치를 저장하는 변수

// LCD 관련함수 
void LCD_init(void); 
void LCD_String(char flash *); 
void LCD_String1(char *);
void Busy(void); 
void Command(unsigned char);
void Data(unsigned char); 
 
// MPU6050 TWI(I2C) 관련함수 
int TWI_Write(int, uint8);              // TWI 쓰기 
int TWI_Read(int);                      // TWI 읽기 
int TWI_Comm(int);                      // TWI 공통 함수
uint8 rdata;

void main(void) {                

    int error, addr, n;      
        
    int i,j;                                          
    DDRC = 0xFF;      // 포트 C 출력 설정 (에러표시)         
    PORTC = 0xFF; 
    LCD_init();               
    // TWI 초기설정                       
    TWBR = 12;          // 16M/(16 + 2 * 12) = 400kHz(TWPS=0) 
    TWSR = 0x00;                                    // Gyro 250도/초, 가속도 2g, 클럭소스 내부 8MHz, Sleep Mode         
    error = TWI_Write(MPU6050_PWR_MGMT_1, 0);       // Sleep mode 해제        
    if(error != 0){
        PORTC = ~error;
        while(1);         
    }  
    //초기 LCD 값 저장하기 위한 반복문 코드 완료시 모든 LCD출력이 0xff상태가 됨       
    for(j=0;j<2;j++){
        for(i=0;i<16;i++){
            lcd_arr[j][i] = lcd_full;
        }                
    }
    
    while(1){            // 자이로센서 값 읽기            
     
        addr = MPU6050_ACCEL_XOUT_H;            //0x3b            
        for(n = 0;n < 14;n++){                
        error = TWI_Read(addr++);                 
        if(error != 0) {   // 에러발생              
            PORTC = ~error;              
            while(1);         
            }            
        mpudata[n] = rdata;         
        }  
               
           
        y_accel = ((int)mpudata[2] << 8) | ((int)mpudata[3] & 0x0FF);    // 자이로센서의 y축값    
      

                                         
        if(test == 0 &&  ((y_accel / 200 ) < -20 ||  (y_accel / 200 ) > 20)){
            test =1;
        }
        if(test == 1){
                                        
        lcd_cnt_temp = lcd_cnt;      // 이전 사각형의 위치를 저장
        lcd_cnt_line_temp = lcd_cnt_line;
        if(lcd_cnt != -1)
        lcd_arr[lcd_cnt_line_temp][lcd_cnt_temp] = lcd_full; // 이전에 빈사각형으로 출력된 부분을 하얀색 사각형으로 다시 출력     
        
        if( (y_accel / 200 ) < -20 ) lcd_cnt++;  //-20도 이하로 기울일 경우 앞으로 한칸 이동
        else if( (y_accel / 200 ) >= 20) lcd_cnt--; //+20도 이상으로 기울일 경우 뒤로 한칸 이동
        if(lcd_cnt <= -1 && lcd_cnt_line == 0) lcd_cnt =0; //초기 상태에서 -1 안되도록 방지
        if(lcd_cnt_line == 1 && lcd_cnt <= -1){    // 2번째줄 1번째칸 에서 -1하는 경우 1번째줄 15번쨰칸으로 이동
            lcd_cnt_line = 0;
            lcd_cnt = 15;
         }
         else if( lcd_cnt_line == 0 && lcd_cnt <=0 ){
            lcd_cnt =0;                               //1번쨰줄 1번째칸일경우 그자리 유지
         }                     
         
        if(lcd_cnt == 16){         //lcd끝까지 갈 경우
            if(lcd_cnt_line == 1){                //1번쨰 줄인 경우 그자리 유지
                lcd_cnt_line = 1;
                lcd_cnt = 15;
            }
            else{                     //그게 아니면 다음줄로 이동하고 1번째칸으로 이동
                lcd_cnt =0; 
                lcd_cnt_line++;
            } 
         }
          lcd_arr[lcd_cnt_line][lcd_cnt] = lcd_void;     // 현재 사각형 위치에 빈칸 출력

        }
        // 결과 출력                      
        Command(HOME);   
        LCD_String1(lcd_arr[0]);       //LCD 출력 
       
        Command(LINE2);   
        LCD_String1(lcd_arr[1]);        
                                
        delay_ms(1000);     
    }
} 
 
// 1바이트 전송
 int TWI_Write(int addr, uint8 data)  { 
    TWI_Comm(addr);          
 
 // Data 
    TWDR = data;  
    TWCR = 0x84;                                             
    while((TWCR & 0x80) != 0x80);         // TWINT 비트가 1이 될 떄까지 
    if((TWSR & 0xF8)!= 0x28) return(0xEF);       // STOP 조건 발생  
    TWCR = 0x94;   
    return(0); 
 } 

 // 1바이트 수신 
 int TWI_Read(int st_addr) {         
    int err;      
    err = TWI_Comm(st_addr);           
    if(err != 0) return(err); 
 
    // 반복 START 조건 발생(Read 모드로 바꿈) 
    TWCR = 0xA4;                                // TWINT, TWSTA, TWEN 셋           
    while((TWCR & 0x80) != 0x80);             // TWINT 비트가 1이 될 떄까지 
    if((TWSR & 0xF8)!= 0x10) return(0x0F); 
 
    // SLA + R 
    TWDR = (MPU6050_I2C_ADDR << 1) | 0x01;      // R/W 비트 셋 
    TWCR = 0x84;                                            
    while((TWCR & 0x80) != 0x80);             // TWINT 비트가 1이 될 때까지
    if((TWSR & 0xF8)!= 0x40) return(0x7f); 
 
    // Data Read 
    TWCR = 0x84;                                            
    while((TWCR & 0x80) != 0x80);             // TWINT 비트가 1이 될 때까지 
    if((TWSR & 0xF8)!= 0x58) return(0xF0); 
    rdata = TWDR;                   // STOP 조건 발생
    TWCR = 0x94;                                             
    return(0); 
} 
 
// 1바이트 송수신 공통 함수 
int TWI_Comm(int reg_addr)  { 
    // START 조건 발생 
    TWCR = 0xA4;                            // TWINT, TWSTA, TWEN 셋              
    while((TWCR & 0x80) != 0x80);         // TWINT 비트가 1이 될 떄까지 
    if((TWSR & 0xF8)!= 0x08) return(TWSR & 0xf8); 

 // SLA + W 
    TWDR = MPU6050_I2C_ADDR << 1; 
    TWCR = 0b10000100;                      // TWINT, TWEN 셋                     
    while((TWCR & 0x80) != 0x80);         // TWINT 비트가 1이 될 떄까지 
    if((TWSR & 0xF8)!= 0x18) return(0xFD); 
 
// Register Address
    TWDR = reg_addr;
    TWCR = 0b10000100;                                           
    while((TWCR & 0x80) != 0x80);         // TWINT 비트가 1이 될 떄까지 
    if((TWSR & 0xF8)!= 0x28) return(0xFB);        
    return(0);  
}

 
// LCD 초기화 
void LCD_init(void) {  
    DDRA = 0xFF;   // 포트 D 출력 설정  

    PORTA &= 0xFB;   // E = 0; 
 
    delay_ms(15);   
    Command(0x20);  
    delay_ms(5);  
    Command(0x20);  
    delay_us(100);  
    Command(0x20);  
    Command(FUNCSET);   
    Command(DISPON); 
    Command(ALLCLR); 
    Command(ENTMODE); 
} 
 
// 문자열 출력 함수 
void LCD_String(char flash *str) {  
    char flash *pStr=0;  
    pStr = str;  
    while(*pStr) Data(*pStr++); 
} 
                                                                                                         
void LCD_String1(char  *str) {  
    char *pStr=0;  
    pStr = str;  
    while(*pStr) Data(*pStr++); 
} 
// 인스트럭션 쓰기 함수
 void Command(unsigned char byte) {  
 Busy(); 
 
 // 인스트럭션 상위 바이트 
 PORTA = (byte & 0xF0);  // 데이터 
  PORTA &= 0xFE;   //RS = 0;  
  PORTA &= 0xFD;   //RW = 0;  
  delay_us(1);  
  PORTA |= 0x04;   //E = 1;  
  delay_us(1);  
  PORTA &= 0xFB;   //E = 0; 
 
 // 인스트럭션 하위 바이트      
 PORTA = ((byte<<4) & 0xF0); // 데이터
  PORTA &= 0xFE;   //RS = 0; 
  PORTA &= 0xFD;   //RW = 0; 
  delay_us(1);
   PORTA |= 0x04;   //E = 1; 
   delay_us(1); 
   PORTA &= 0xFB;   //E = 0;
    } 
 
//데이터 쓰기 함수
 void Data(unsigned char byte) {  
 Busy(); 
 
 // 데이터 상위 바이트  
 PORTA = (byte & 0xF0);  // 데이터  
 PORTA |= 0x01;   //RS = 1; 
  PORTA &= 0xFD;   //RW = 0; 
   delay_us(1);  
   PORTA |= 0x04;   //E = 1;  
   delay_us(1);  
   PORTA &= 0xFB;   //E = 0; 
 
 // 데이터 하위 바이트  
 PORTA = ((byte<<4) & 0xF0);   // 데이터 
  PORTA |= 0x01;   //RS = 1;  
  PORTA &= 0xFD;   //RW = 0;  
  delay_us(1); 
   PORTA |= 0x04;   //E = 1; 
 delay_us(1);  
 PORTA &= 0xFB;   //E = 0; 
 } 
 
// Busy Flag Check -> 일반적인 BF를 체크하는 것이 아니라 // 일정한 시간 지연을 이용한다.
 void Busy(void) {  delay_ms(2); } 
 
  