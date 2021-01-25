//MPU-6050�� I2C������� �����͸� �о�� LCD�� ����Ѵ�. 
 
 
#include <mega128.h> 
 #include <delay.h> 
 #include <stdio.h> 
 #include <stdlib.h> 
 
#define LCD_RS      PORTA.0         // ��ɾ�/������ ���� ��ɾ�      
#define LCD_RW      PORTA.1         // READ/WRITE ��ȣ 
#define LCD_EN      PORTA.2         // ENABLE ��ȣ 
 
#define FUNCSET       0x28          // Function Set 
#define ENTMODE       0x06          // Entry Mode Set 
#define ALLCLR        0x01          // All Clear 
#define HOME          0x02          // HOME 
#define DISPON        0x0c          // Display On 
#define LINE2         0xC0         // 2nd Line Move 
 
// MPU6050 �������� ��巹�� �� ����̽� ��巹�� 
#define MPU6050_ACCEL_XOUT_H    0x3B    // R 
#define MPU6050_TEMP_OUT_H      0x41    // R    
 #define MPU6050_GYRO_XOUT_H     0x43B   // R   
 #define MPU6050_PWR_MGMT_1      0x6B    // R/W    
 #define MPU6050_I2C_ADDR        0x68    // ����̽� ��巹�� 
 
typedef unsigned char uint8; 
 
uint8   mpudata[14];                    // MPU6050 ������ ����                     
int    y_accel;      // ���ӵ���

char lcd_arr[2][16]; //lcd��� �迭
char lcd_full = 0xff; // �Ͼ�� �簢�� ��� ��              
char lcd_void = 0xfe; // �� �簢�� ��� ��
int lcd_cnt=-1,lcd_cnt_line=0; // ���� �簢���� ��ġ�� �����ϴ� ����
int test=0;
int lcd_cnt_temp=0,lcd_cnt_line_temp=0; // ���� �簢���� ��ġ�� �����ϴ� ����

// LCD �����Լ� 
void LCD_init(void); 
void LCD_String(char flash *); 
void LCD_String1(char *);
void Busy(void); 
void Command(unsigned char);
void Data(unsigned char); 
 
// MPU6050 TWI(I2C) �����Լ� 
int TWI_Write(int, uint8);              // TWI ���� 
int TWI_Read(int);                      // TWI �б� 
int TWI_Comm(int);                      // TWI ���� �Լ�
uint8 rdata;

void main(void) {                

    int error, addr, n;      
        
    int i,j;                                          
    DDRC = 0xFF;      // ��Ʈ C ��� ���� (����ǥ��)         
    PORTC = 0xFF; 
    LCD_init();               
    // TWI �ʱ⼳��                       
    TWBR = 12;          // 16M/(16 + 2 * 12) = 400kHz(TWPS=0) 
    TWSR = 0x00;                                    // Gyro 250��/��, ���ӵ� 2g, Ŭ���ҽ� ���� 8MHz, Sleep Mode         
    error = TWI_Write(MPU6050_PWR_MGMT_1, 0);       // Sleep mode ����        
    if(error != 0){
        PORTC = ~error;
        while(1);         
    }  
    //�ʱ� LCD �� �����ϱ� ���� �ݺ��� �ڵ� �Ϸ�� ��� LCD����� 0xff���°� ��       
    for(j=0;j<2;j++){
        for(i=0;i<16;i++){
            lcd_arr[j][i] = lcd_full;
        }                
    }
    
    while(1){            // ���̷μ��� �� �б�            
     
        addr = MPU6050_ACCEL_XOUT_H;            //0x3b            
        for(n = 0;n < 14;n++){                
        error = TWI_Read(addr++);                 
        if(error != 0) {   // �����߻�              
            PORTC = ~error;              
            while(1);         
            }            
        mpudata[n] = rdata;         
        }  
               
           
        y_accel = ((int)mpudata[2] << 8) | ((int)mpudata[3] & 0x0FF);    // ���̷μ����� y�ప    
      

                                         
        if(test == 0 &&  ((y_accel / 200 ) < -20 ||  (y_accel / 200 ) > 20)){
            test =1;
        }
        if(test == 1){
                                        
        lcd_cnt_temp = lcd_cnt;      // ���� �簢���� ��ġ�� ����
        lcd_cnt_line_temp = lcd_cnt_line;
        if(lcd_cnt != -1)
        lcd_arr[lcd_cnt_line_temp][lcd_cnt_temp] = lcd_full; // ������ ��簢������ ��µ� �κ��� �Ͼ�� �簢������ �ٽ� ���     
        
        if( (y_accel / 200 ) < -20 ) lcd_cnt++;  //-20�� ���Ϸ� ����� ��� ������ ��ĭ �̵�
        else if( (y_accel / 200 ) >= 20) lcd_cnt--; //+20�� �̻����� ����� ��� �ڷ� ��ĭ �̵�
        if(lcd_cnt <= -1 && lcd_cnt_line == 0) lcd_cnt =0; //�ʱ� ���¿��� -1 �ȵǵ��� ����
        if(lcd_cnt_line == 1 && lcd_cnt <= -1){    // 2��°�� 1��°ĭ ���� -1�ϴ� ��� 1��°�� 15����ĭ���� �̵�
            lcd_cnt_line = 0;
            lcd_cnt = 15;
         }
         else if( lcd_cnt_line == 0 && lcd_cnt <=0 ){
            lcd_cnt =0;                               //1������ 1��°ĭ�ϰ�� ���ڸ� ����
         }                     
         
        if(lcd_cnt == 16){         //lcd������ �� ���
            if(lcd_cnt_line == 1){                //1���� ���� ��� ���ڸ� ����
                lcd_cnt_line = 1;
                lcd_cnt = 15;
            }
            else{                     //�װ� �ƴϸ� �����ٷ� �̵��ϰ� 1��°ĭ���� �̵�
                lcd_cnt =0; 
                lcd_cnt_line++;
            } 
         }
          lcd_arr[lcd_cnt_line][lcd_cnt] = lcd_void;     // ���� �簢�� ��ġ�� ��ĭ ���

        }
        // ��� ���                      
        Command(HOME);   
        LCD_String1(lcd_arr[0]);       //LCD ��� 
       
        Command(LINE2);   
        LCD_String1(lcd_arr[1]);        
                                
        delay_ms(1000);     
    }
} 
 
// 1����Ʈ ����
 int TWI_Write(int addr, uint8 data)  { 
    TWI_Comm(addr);          
 
 // Data 
    TWDR = data;  
    TWCR = 0x84;                                             
    while((TWCR & 0x80) != 0x80);         // TWINT ��Ʈ�� 1�� �� ������ 
    if((TWSR & 0xF8)!= 0x28) return(0xEF);       // STOP ���� �߻�  
    TWCR = 0x94;   
    return(0); 
 } 

 // 1����Ʈ ���� 
 int TWI_Read(int st_addr) {         
    int err;      
    err = TWI_Comm(st_addr);           
    if(err != 0) return(err); 
 
    // �ݺ� START ���� �߻�(Read ���� �ٲ�) 
    TWCR = 0xA4;                                // TWINT, TWSTA, TWEN ��           
    while((TWCR & 0x80) != 0x80);             // TWINT ��Ʈ�� 1�� �� ������ 
    if((TWSR & 0xF8)!= 0x10) return(0x0F); 
 
    // SLA + R 
    TWDR = (MPU6050_I2C_ADDR << 1) | 0x01;      // R/W ��Ʈ �� 
    TWCR = 0x84;                                            
    while((TWCR & 0x80) != 0x80);             // TWINT ��Ʈ�� 1�� �� ������
    if((TWSR & 0xF8)!= 0x40) return(0x7f); 
 
    // Data Read 
    TWCR = 0x84;                                            
    while((TWCR & 0x80) != 0x80);             // TWINT ��Ʈ�� 1�� �� ������ 
    if((TWSR & 0xF8)!= 0x58) return(0xF0); 
    rdata = TWDR;                   // STOP ���� �߻�
    TWCR = 0x94;                                             
    return(0); 
} 
 
// 1����Ʈ �ۼ��� ���� �Լ� 
int TWI_Comm(int reg_addr)  { 
    // START ���� �߻� 
    TWCR = 0xA4;                            // TWINT, TWSTA, TWEN ��              
    while((TWCR & 0x80) != 0x80);         // TWINT ��Ʈ�� 1�� �� ������ 
    if((TWSR & 0xF8)!= 0x08) return(TWSR & 0xf8); 

 // SLA + W 
    TWDR = MPU6050_I2C_ADDR << 1; 
    TWCR = 0b10000100;                      // TWINT, TWEN ��                     
    while((TWCR & 0x80) != 0x80);         // TWINT ��Ʈ�� 1�� �� ������ 
    if((TWSR & 0xF8)!= 0x18) return(0xFD); 
 
// Register Address
    TWDR = reg_addr;
    TWCR = 0b10000100;                                           
    while((TWCR & 0x80) != 0x80);         // TWINT ��Ʈ�� 1�� �� ������ 
    if((TWSR & 0xF8)!= 0x28) return(0xFB);        
    return(0);  
}

 
// LCD �ʱ�ȭ 
void LCD_init(void) {  
    DDRA = 0xFF;   // ��Ʈ D ��� ����  

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
 
// ���ڿ� ��� �Լ� 
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
// �ν�Ʈ���� ���� �Լ�
 void Command(unsigned char byte) {  
 Busy(); 
 
 // �ν�Ʈ���� ���� ����Ʈ 
 PORTA = (byte & 0xF0);  // ������ 
  PORTA &= 0xFE;   //RS = 0;  
  PORTA &= 0xFD;   //RW = 0;  
  delay_us(1);  
  PORTA |= 0x04;   //E = 1;  
  delay_us(1);  
  PORTA &= 0xFB;   //E = 0; 
 
 // �ν�Ʈ���� ���� ����Ʈ      
 PORTA = ((byte<<4) & 0xF0); // ������
  PORTA &= 0xFE;   //RS = 0; 
  PORTA &= 0xFD;   //RW = 0; 
  delay_us(1);
   PORTA |= 0x04;   //E = 1; 
   delay_us(1); 
   PORTA &= 0xFB;   //E = 0;
    } 
 
//������ ���� �Լ�
 void Data(unsigned char byte) {  
 Busy(); 
 
 // ������ ���� ����Ʈ  
 PORTA = (byte & 0xF0);  // ������  
 PORTA |= 0x01;   //RS = 1; 
  PORTA &= 0xFD;   //RW = 0; 
   delay_us(1);  
   PORTA |= 0x04;   //E = 1;  
   delay_us(1);  
   PORTA &= 0xFB;   //E = 0; 
 
 // ������ ���� ����Ʈ  
 PORTA = ((byte<<4) & 0xF0);   // ������ 
  PORTA |= 0x01;   //RS = 1;  
  PORTA &= 0xFD;   //RW = 0;  
  delay_us(1); 
   PORTA |= 0x04;   //E = 1; 
 delay_us(1);  
 PORTA &= 0xFB;   //E = 0; 
 } 
 
// Busy Flag Check -> �Ϲ����� BF�� üũ�ϴ� ���� �ƴ϶� // ������ �ð� ������ �̿��Ѵ�.
 void Busy(void) {  delay_ms(2); } 
 
  