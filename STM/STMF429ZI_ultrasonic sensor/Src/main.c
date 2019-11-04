/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ETH_HandleTypeDef heth;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart3;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ETH_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);
void HCSR04_T_Sonic_Active(GPIO_TypeDef* TRIG_GPIOx, uint16_t TRIG_GPIO_Pin);
/* USER CODE BEGIN PFP */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
typedef struct{
  int                Distance;        /* Distance measured from sensor in centi meters */
  GPIO_TypeDef*      ECHO_GPIOx;      /* Pointer to GPIOx PORT for ECHO pin. Meant for private use only */
  uint16_t           ECHO_GPIO_Pin;   /* GPIO Pin for ECHO pin. Meant for private use only */
  GPIO_TypeDef*      TRIG_GPIOx;      /* Pointer to GPIOx PORT for TRIGGER pin. Meant for private use only */
  uint16_t           TRIG_GPIO_Pin;   /* GPIO Pin for ECHO pin. Meant for private use only */
  TIM_HandleTypeDef* HCSR_Timer;      /* Timer for measurement when sensor is rising edge. */
}hcsr04_t;

//초음파 센서를 사용할때 필요한 GPIO정보와 TImer 핀 정보 저장하는 구조체
hcsr04_t hcsr04;


// HCSR04
//__IO ITStatus 는 RESET일 경우 0 , SET일 경우 1로 값이 변경되는 구조체, __IO는 volatile 쉽게 말하면 volatile int FLAG_Hcsr04 =0은 __IO ITStatus FLAG_Hcsr04 = RESET / volatile int FLAG_Hcsr04 =1 은 __IO ITStatus FLAG_Hcsr04 = SET
__IO ITStatus FLAG_Hcsr04 = RESET;


char tx_data[20]= "distance : ";
uint8_t rxdata[8];

uint8_t switch_cnt=2;
int distance;

void HCSR04_T_Init(hcsr04_t* HCSR04, GPIO_TypeDef* ECHO_GPIOx, uint16_t ECHO_GPIO_Pin, GPIO_TypeDef* TRIG_GPIOx, uint16_t TRIG_GPIO_Pin, TIM_HandleTypeDef* HCSR_Timer){
    HCSR04->ECHO_GPIOx     = ECHO_GPIOx;
    HCSR04->ECHO_GPIO_Pin  = ECHO_GPIO_Pin;
    HCSR04->TRIG_GPIOx     = TRIG_GPIOx;
    HCSR04->TRIG_GPIO_Pin  = TRIG_GPIO_Pin;
    HCSR04->HCSR_Timer     = HCSR_Timer;
    HCSR04->Distance       = 0;
    
    // 처음 1회 출력
   HCSR04_T_Sonic_Active(TRIG_GPIOx, TRIG_GPIO_Pin);
}


// Measure distance
hcsr04_t HCSR04_T_Sonic_Measure(hcsr04_t hcsr04_echo, __IO ITStatus FLAG_Hcsr04){
	//상승엣지에서 카운터 동작 시작
    if(FLAG_Hcsr04 == RESET){
			__HAL_TIM_SetCounter(hcsr04_echo.HCSR_Timer,0);  // 카운트 값을 0으로 초기화
        HAL_TIM_Base_Start_IT(hcsr04_echo.HCSR_Timer);    // 타이머 동작 시작
    }
		//하강엣지에서 동작 정지 및 거리 계산
    else if(FLAG_Hcsr04 == SET){
        HAL_TIM_Base_Stop_IT(hcsr04_echo.HCSR_Timer);      // 타이머 동작 정지
        
        // tmp measure data
        int measure_distance = __HAL_TIM_GetCounter(hcsr04_echo.HCSR_Timer); // 타이머 카운터 값 얻어오는 함수
        
        // Selct cm or inch
      
        hcsr04_echo.Distance = measure_distance/58;     // cm
      
        
        return hcsr04_echo;
    }
}
//초음파 센서의 echo핀에 입력이 있을때 발생되는 인터럽트 ( 상승엣지 , 하강엣지 )
void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin)
{
  if(GPIO_Pin == GPIO_PIN_3){
		//상승엣지에서 아래 코드 동작
    if(FLAG_Hcsr04 == RESET){
      //타이머 카운터 실행및 거리 계산 함수
			HCSR04_T_Sonic_Measure(hcsr04,FLAG_Hcsr04);
      FLAG_Hcsr04 = SET;
    }
		//하강엣지에서 아래 코드 동작
    else if(FLAG_Hcsr04 == SET){
			//타이머 카운터 실행 및 거리계산 함수
      hcsr04 = HCSR04_T_Sonic_Measure(hcsr04,FLAG_Hcsr04);
      FLAG_Hcsr04 = RESET;    
			//계산된 거리값을 저장
			distance = hcsr04.Distance;
    }
  }
}

// Active Trig
void HCSR04_T_Sonic_Active(GPIO_TypeDef* TRIG_GPIOx, uint16_t TRIG_GPIO_Pin){
			//Trig핀에 high상태 출력
     HAL_GPIO_WritePin(TRIG_GPIOx, TRIG_GPIO_Pin, GPIO_PIN_SET);
  //10ms동안 유지   
	HAL_Delay(10);
	// Low 출력
     HAL_GPIO_WritePin(TRIG_GPIOx, TRIG_GPIO_Pin, GPIO_PIN_RESET);
	// 60ms 이후 다시 동작
	HAL_Delay(60);
  
}

void gpio_init(){
// LED1~8의 동작 조건을 설정
		GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | 
								 GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;	
																	// GPIO에서 사용할 PIN 설정
		GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP; 		// Output Push-Pull 모드
		GPIO_InitStruct.Pull     = GPIO_PULLUP; 						// Pull Up 모드
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; 		// 동작속도를 HIGH로 	
		
		//  GPIOLed를 GPIO_Init_Struct에 설정된 조건으로 초기화함
		HAL_GPIO_Init(GPIOD, &GPIO_InitStruct); 
	
	//초음파 센서의 GPIO 설정  초음파센서의 echo핀
	 GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING; //상승엣지와 하강엣지에서 인터럽트 발생
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	//초음파센서의 trig 핀
 GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}


uint8_t check_data=0;

//Interrupt 모드 수신이 완료되면 호출되는 callback 함수 
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandler)
{
	char comm[8]="SW "; //기본 커맨드 ( SW 1 = PC와 통신 On | SW 2 = PC와 통신 Off )
	
	//comm에 들어있는 데이터와 rx로 받은 데이터가 동일한지 확인 마지막 숫자는 비교하지 않음
	for(int i=0;i<3;i++){
		if(rxdata[i]  == comm[i]){
			check_data++;
		}
	}
	// 총 4글자 중 앞에 3글자가 동일할 경우 마지막 글자를 비교
	if(check_data == 3){
		check_data=0;
		if(rxdata[3] == 0x31) //1일경우
		{
			switch_cnt =1; // pc와 통신 on
		}
		else if( rxdata[3] == 0x32){ //2일경우
			
			switch_cnt =2; // pc와 통신 off
		}
	}
		
}
//1~6 LED / 8 LED 중 원하는 핀 번호 넣으면 해당 LED제외 다른 LED는 전부 Off만듬
void LED_Write(uint8_t pin_num){
		switch (pin_num){
			case 0:
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_4,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_5,GPIO_PIN_RESET);
			
				break;
			case 1:
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_4,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_5,GPIO_PIN_RESET);
			
				break;
			
			case 2:
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_4,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_5,GPIO_PIN_RESET);
			
				break;
			
			case 3:
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_4,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_5,GPIO_PIN_RESET);
			
				break;
			
			case 4:
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_4,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_5,GPIO_PIN_RESET);
			
				break;
			
			case 5:
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_4,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_5,GPIO_PIN_SET);
			
				break;
			case 8:
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_4,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_5,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_7,GPIO_PIN_SET);
			break;
			
		
			
		}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ETH_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
	MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
	
	//초음파센서 Init 함수
	HCSR04_T_Init(&hcsr04, GPIOC, GPIO_PIN_3, GPIOC, GPIO_PIN_2, &htim7);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
	gpio_init();
  /* USER CODE END 2 */
	
  while (1)
  {
    /* USER CODE END WHILE */
			HCSR04_T_Sonic_Active(GPIOC, GPIO_PIN_2);
			HAL_UART_Receive_IT(&huart3,(uint8_t*)rxdata,4);
		if(switch_cnt == 2){
		//led 1
		if(distance > 30 && distance <= 35){
			LED_Write(0);
		}
		//led 2
		else if(distance > 25 && distance <= 30 ){
			LED_Write(1); 
		}
		//led 3
		else if(distance > 20 && distance <= 25){
			LED_Write(2);
		}
		//led 4
		else if(distance > 15 && distance <= 20){
			LED_Write(3);
			HAL_Delay(1000);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
			HAL_Delay(1000);
		}
		//led 5
		else if(distance > 10 && distance <= 15){
			LED_Write(4);
			HAL_Delay(500);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_4,GPIO_PIN_RESET);
			HAL_Delay(500);
		}
		//led 6
		else if(distance > 5 && distance <= 10){
			LED_Write(5);
			HAL_Delay(250);
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_5,GPIO_PIN_RESET);
			HAL_Delay(250);
		}
	}
		//보드에서 PC로 데이터 전송 및 LED8번 동작
		if(switch_cnt == 1){
			
			//8번 LED On
			LED_Write(8);
			// 거리에 따라서 Delay 주는 코드
			if(distance < 10) HAL_Delay(250);
			else if(distance < 20) HAL_Delay(500);
			else if(distance < 30) HAL_Delay(1000);
			//8번 LED Off
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_7,GPIO_PIN_RESET);
			
			if(distance < 10) HAL_Delay(250);
			else if(distance < 20) HAL_Delay(500);
			else if(distance < 30) HAL_Delay(1000);
			
			int temp;
			//100의 자리까지 표현 뒤에 +48은 아스키 코드로 숫자출력을 위해 더해주는값
			tx_data[11] = (distance /100) +48;
			// 0~ 99까지의 값 temp에 저장
			temp = distance % 100;
			
			tx_data[12] = (temp /10) +48;
			tx_data[13] = (temp%10)+48;
			tx_data[14] = 'c';
			tx_data[15] = 'm';
			HAL_UART_Transmit(&huart3,(uint8_t*)tx_data,16,1000);
			//별 의미는 없는데 거리가 가까울때 전송속도가 빨라서 추가해논 코드
			HAL_Delay(500);
		}
	
		
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */

/**
  * @brief ETH Initialization Function
  * @param None
  * @retval None
  */
static void MX_ETH_Init(void)
{

  /* USER CODE BEGIN ETH_Init 0 */

  /* USER CODE END ETH_Init 0 */

   uint8_t MACAddr[6] ;

  /* USER CODE BEGIN ETH_Init 1 */

  /* USER CODE END ETH_Init 1 */
  heth.Instance = ETH;
  heth.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
  heth.Init.PhyAddress = LAN8742A_PHY_ADDRESS;
  MACAddr[0] = 0x00;
  MACAddr[1] = 0x80;
  MACAddr[2] = 0xE1;
  MACAddr[3] = 0x00;
  MACAddr[4] = 0x00;
  MACAddr[5] = 0x00;
  heth.Init.MACAddr = &MACAddr[0];
  heth.Init.RxMode = ETH_RXPOLLING_MODE;
  heth.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
  heth.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;

  /* USER CODE BEGIN MACADDRESS */
    
  /* USER CODE END MACADDRESS */

  if (HAL_ETH_Init(&heth) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ETH_Init 2 */

  /* USER CODE END ETH_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

		HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);	
	HAL_NVIC_EnableIRQ(USART3_IRQn); 
	
  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 4;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.ep0_mps = DEP0CTL_MPS_64;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2|LD1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);
	
	 GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
 GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
  /*Configure GPIO pins : USER_Btn_Pin PC3 */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC2 LD1_Pin */
  GPIO_InitStruct.Pin = LD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);
	
	 HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);


}

/* USER CODE BEGIN 4 */

/* TIM7 init function */
static void MX_TIM7_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;

  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 58;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 23200;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
void SystemClock_Config(void)
{
   RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
  }
  
  if(HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
  }
	
    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}