/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "app_tof.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "53l5a1_ranging_sensor.h"
#include <stdio.h>
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TOF_Init();
  /* USER CODE BEGIN 2 */
  //set ccr value for pwm gen
  __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1, __HAL_TIM_GET_AUTORELOAD(&htim2)*0.5);
    htim2.Instance->EGR=TIM_EGR_UG;

  int32_t calibration_data;
  calibration_data=VL53L5A1_RANGING_SENSOR_XTalkCalibration(VL53L5A1_DEV_CENTER, 0.8, 600);
  //VL53L5A1_DEV_CENTER è il parametro Instance

  uint8_t dimMatrix = 4;
  //uint8_t dimMatrix = 8;

  int index;
  long distance[4]={0,0,0,0};
  int max(long x[],int k)
  {
  	int i;
  	int index;
  	long sx=x[0]+x[1];
  	long ctr=x[1]+x[2];
  	long dx=x[2]+x[3];
  	if ((dx>ctr)&(dx>sx)){
  		index=2;
  	  	} else if((sx>ctr)&(sx>dx)){
  	  		index=0;
  	  	} else if ((ctr>dx)&(ctr>sx)){
  	  		index=1;
  	  	}
  	return(index);
  }


  void forward()
  {
	  printf("forward\r\n");
  	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  	HAL_TIM_Base_Start_IT(&htim3);
  	HAL_GPIO_WritePin(DIR1_GPIO_Port, DIR1_Pin, RESET );
  	HAL_GPIO_WritePin(DIR2_GPIO_Port, DIR2_Pin, SET ); //sx
  }

  void spin()
  {	__HAL_TIM_SET_AUTORELOAD(&htim3,2548);//leva il diviso 100
  	htim3.Instance->EGR=TIM_EGR_UG ;
  	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  	HAL_TIM_Base_Start_IT(&htim3);
  	HAL_GPIO_WritePin(DIR1_GPIO_Port, DIR1_Pin, SET );
  	HAL_GPIO_WritePin(DIR2_GPIO_Port, DIR2_Pin, SET );
  	printf("spin\r\n");
  	}

  void goRight()
  {
  	__HAL_TIM_SET_AUTORELOAD(&htim3,500);//leva il diviso 100
  	htim3.Instance->EGR=TIM_EGR_UG ;
  	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  	HAL_TIM_Base_Start_IT(&htim3);
  	printf("right\r\n");
  	HAL_GPIO_WritePin(DIR1_GPIO_Port, DIR1_Pin, SET );
  	HAL_GPIO_WritePin(DIR2_GPIO_Port, DIR2_Pin, SET );
  	//HAL_GPIO_WritePin(DIR2_GPIO_Port, DIR2_Pin, SET );
  	}

  void goLeft()
  {
  	__HAL_TIM_SET_AUTORELOAD(&htim3,500);//leva il diviso 100
  	htim3.Instance->EGR=TIM_EGR_UG ;
  	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

  	HAL_TIM_Base_Start_IT(&htim3);
  	HAL_GPIO_WritePin(DIR1_GPIO_Port, DIR1_Pin, RESET );
  	HAL_GPIO_WritePin(DIR2_GPIO_Port, DIR2_Pin, RESET );
  	//HAL_GPIO_WritePin(DIR2_GPIO_Port, DIR2_Pin, RESET );
  	printf("left\r\n");
  	}


  void check(int index, long distance[])
  {
	  HAL_GPIO_WritePin(LED_Read_GPIO_Port, LED_Read_Pin, RESET);
	  HAL_GPIO_WritePin(LED_Step_GPIO_Port, LED_Step_Pin, SET);

	  if ( distance[1]<200 & distance[2]<200)
		 {
		  spin();
		  	//printf("test\r\n");

		  	   }
		 else{
			 int n=(((distance[2]+distance[1])/2-200)/(10*3.14*6.5))*2038;
			 		  	  switch(index)
			 		  	  {
			 		  	  	  case 0:
			 		  	  		  //go left;
			 		  	  		  goLeft();
			 		  	  		  break;
			 		  	  	  case 1:

			 		  	  		__HAL_TIM_SET_AUTORELOAD(&htim3,n); //set n of steps for the position control of stepper
			 		  	  		printf("n=%d\r\n", n);
			 		  	  		htim3.Instance->EGR=TIM_EGR_UG ;
			 		  	  		forward();
			 		  	  		  break;
			 		  	  	  case 2:
			 		  	  		  //go right
			 		  	  		  goRight();
			 		  	  		  break;
		  		   }
		 }
  }


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

  //MX_TOF_Process();
    /* USER CODE BEGIN 3 */
  while(HAL_TIM_Base_GetState(&htim3)==HAL_TIM_STATE_BUSY)
  	  {

  	  	  }

TOF_Read_Distances( dimMatrix, distance);
/*for (int i = 0; i < 1; i++)
	 {
	      for (int j =(4 - 1); j >= 0; j --)
	         {printf("dist=%d\r\n", distance[j]);
	        		   }
	        }*/
  index=max(distance, 4);
  check(index, distance);


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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
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
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
