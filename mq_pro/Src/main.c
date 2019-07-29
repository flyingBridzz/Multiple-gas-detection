/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "oled.h"
#include "com.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define		Filter		20//均值滤波的大小
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
float Concentration[3] = {0};
uint32_t BUFF_DR[3] = {0};
uint32_t SUM[3] = {0};
uint8_t Finish = 0;
volatile uint8_t Order = 0;
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
  MX_DMA_Init();
  MX_TIM6_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
	Delay_us_ON();
	OLED_Init();
	HAL_Delay(5);
	if((ADC1->CR2 & 0x01) == 0x01)//确认ADC1是否关电
	{
		ADC1->CR2 &= 0xFFFFFFFE;		//若未关电则关电ADC1
		HAL_Delay(1);
	}
	ADC1->CR2 |= 0x04;						//启动ADC1校准
	ADC1->CR2 |= 0x01;						//打开ADC1
	while((ADC1->CR2 & 0x04) == 0x04)printf("等待\r\n");//等待校准结束
	HAL_ADC_Start_DMA(&hadc1,BUFF_DR,3);//开始ADC1转换
	//OLED_Clear();
//	for(int i=10;i>0;i--)//预热两分钟，保证数据准确性
//	{
//		HAL_Delay(1000);
//		//printf("%d\r\n",i);
//		OLED_ShowNum(25,3,i,3,16);
//		OLED_ShowChar(53,3,'s');
//	}
	OLED_Clear();
	HAL_UART_Receive_IT(&huart3,&Order,1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	
		HAL_Delay(500);
		if(Finish == 1)
		{
			Finish = 0;
			Concentration[0] = ((float)(SUM[0]/Filter)/4095)*100;
			Concentration[1] = ((float)(SUM[1]/Filter)/4095)*100;
			Concentration[2] = ((float)(SUM[2]/Filter)/4095)*100;
			SUM[0] = 0;
			SUM[1] = 0;
			SUM[2] = 0;
			sendframe(Concentration[0], Concentration[1], Concentration[2]);
			printf("Con[0] = %0.2f%%  Con[1] = %0.2f%%  Con[2] = %0.2f%%\r\n",Concentration[0],Concentration[1],Concentration[2]);
			OLED_Clear();
			OLED_ShowString(2,2,"CO:");
			OLED_ShowFloat(50,2,Concentration[0],16);
			OLED_ShowString(98,2,"%");
			OLED_ShowString(2,4,"CH4:");
			OLED_ShowFloat(50,4,Concentration[1],16);
			OLED_ShowString(98,4,"%");
			OLED_ShowString(2,6,"Smog:");
			OLED_ShowFloat(50,6,Concentration[2],16);
			OLED_ShowString(98,6,"%");			
			HAL_ADC_Start_DMA(&hadc1,BUFF_DR,3);//开始ADC1转换
		}
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV8;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int fputc(int ch, FILE *f)
{
	while((USART1->SR & 0x40) == 0);//等待发送完毕
	USART1->DR = (uint8_t)ch;
	return ch;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	static uint8_t cnt = 0;
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);
  /* NOTE : This function should not be modified. When the callback is needed,
            function HAL_ADC_ConvCpltCallback must be implemented in the user file.
   */
	cnt++;
	SUM[0]+=BUFF_DR[0]&0x0000ffff;
	SUM[1]+=BUFF_DR[1]&0x0000ffff;
	SUM[2]+=BUFF_DR[2]&0x0000ffff;
	if(cnt>Filter-1)
	{
		Finish = 1;
		cnt = 0;
		HAL_ADC_Stop_DMA(&hadc1);
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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_UART_RxCpltCallback could be implemented in the user file
   */
	if(huart->Instance == USART3)
	{
		if(Order == 0xaa){
			GPIOA->BSRR = 0x00000008;
		}else{
			GPIOA->BSRR = 0x00080000;
		}
	}
	HAL_UART_Receive_IT(&huart3,&Order,1);
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
