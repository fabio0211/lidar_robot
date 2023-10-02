/**
  ******************************************************************************
  * @file          : app_tof.c
  * @author        : IMG SW Application Team
  * @brief         : This file provides code for the configuration
  *                  of the STMicroelectronics.X-CUBE-TOF1.3.3.0 instances.
  ******************************************************************************
  *
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_tof.h"
#include "main.h"
#include <stdio.h>

#include "53l5a1_ranging_sensor.h"
#include "app_tof_pin_conf.h"
#include "stm32f4xx_nucleo.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define TIMING_BUDGET (30U) /* 5 ms < TimingBudget < 100 ms */
#define RANGING_FREQUENCY (5U) /* Ranging frequency Hz (shall be consistent with TimingBudget value) */
#define POLLING_PERIOD (1000U/RANGING_FREQUENCY) /* refresh rate for polling mode (milliseconds) */

/* Private variables ---------------------------------------------------------*/
static RANGING_SENSOR_Capabilities_t Cap;
static RANGING_SENSOR_ProfileConfig_t Profile;
static RANGING_SENSOR_Result_t Result;

static int32_t status = 0;
static volatile uint8_t PushButtonDetected = 0;
volatile uint8_t ToF_EventDetected = 0;

/* Private function prototypes -----------------------------------------------*/
static void MX_53L5A1_SimpleRanging_Init(void);
static void MX_53L5A1_SimpleRanging_Process(void);
static void print_result(RANGING_SENSOR_Result_t *Result);
static void toggle_resolution(void);
static void toggle_signal_and_ambient(void);
static void clear_screen(void);
static void display_commands_banner(void);
static void handle_cmd(uint8_t cmd);
static uint8_t get_key(void);
static uint32_t com_has_data(void);

void MX_TOF_Init(void)
{
  /* USER CODE BEGIN SV */

  /* USER CODE END SV */

  /* USER CODE BEGIN TOF_Init_PreTreatment */

  /* USER CODE END TOF_Init_PreTreatment */

  /* Initialize the peripherals and the TOF components */

  MX_53L5A1_SimpleRanging_Init();

  /* USER CODE BEGIN TOF_Init_PostTreatment */

  /* USER CODE END TOF_Init_PostTreatment */
}

/*
 * LM background task
 */
void MX_TOF_Process(void)
{
  /* USER CODE BEGIN TOF_Process_PreTreatment */

  /* USER CODE END TOF_Process_PreTreatment */

  MX_53L5A1_SimpleRanging_Process();

  /* USER CODE BEGIN TOF_Process_PostTreatment */

  /* USER CODE END TOF_Process_PostTreatment */
}

static void MX_53L5A1_SimpleRanging_Init(void)
{
  /* Initialize Virtual COM Port */
  BSP_COM_Init(COM1);

  /* Initialize button */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

  status = VL53L5A1_RANGING_SENSOR_Init(VL53L5A1_DEV_CENTER);

  if (status != BSP_ERROR_NONE)
  {
    printf("VL53L5A1_RANGING_SENSOR_Init failed\n");
    printf("Check you're using ONLY the center device soldered on the shield, NO satellite shall be connected !\n");
    while(1);
  }
}


//CODICE NOSTRO

void get_3_row(RANGING_SENSOR_Result_t *Result, long distance[]){
	//long* get_3_row(RANGING_SENSOR_Result_t *Result, long res[]){
	  VL53L5A1_RANGING_SENSOR_GetCapabilities(VL53L5A1_DEV_CENTER, &Cap);
	int8_t i, j;
	//long res[4];
	  uint8_t zones_per_line;

	  zones_per_line = ((Profile.RangingProfile == RS_PROFILE_8x8_AUTONOMOUS) ||
	                    (Profile.RangingProfile == RS_PROFILE_8x8_CONTINUOUS)) ? 8 : 4;

	  printf("Valori della terza riga:\r\n");
	  //printf("%d\r\n",Result->NumberOfZones);
	  printf("%-21s%-21s%\r\n", "Distance [mm]", "Status");

	  HAL_GPIO_WritePin(LED_Read_GPIO_Port, LED_Read_Pin, SET);
	  HAL_GPIO_WritePin(LED_Step_GPIO_Port, LED_Step_Pin, RESET);

	  for (i = 0; i < RANGING_SENSOR_NB_TARGET_PER_ZONE; i++)
	  {
		  //RANGING_SENSOR_NB_TARGET_PER_ZONE=1

		  for (j =(zones_per_line - 1); j >= 0; j --)
	    {	distance[j]=(long)Result->ZoneResult[j+8].Distance[i];
	      printf("%-20ld%-20ld\r\n", (long)Result->ZoneResult[j+8].Distance[i], (long)Result->ZoneResult[j+8].Status[i]);
	      //printf("distance=%d\r\n", distance[j]);
	   }
		   //printf("distance=%l\r\n", &distance);
	  }
	  //return res;

}


void TOF_Read_Distances(uint8_t dimMatrix, long distance[])
{
uint32_t Id;
VL53L5A1_RANGING_SENSOR_ReadID(VL53L5A1_DEV_CENTER, &Id);
VL53L5A1_RANGING_SENSOR_GetCapabilities(VL53L5A1_DEV_CENTER, &Cap);

    if (dimMatrix == 4){
        Profile.RangingProfile = RS_PROFILE_4x4_CONTINUOUS;
        Profile.TimingBudget = TIMING_BUDGET; /* 5 ms < TimingBudget < 100 ms */
        Profile.Frequency = RANGING_FREQUENCY; /* Ranging frequency Hz (shall be consistent with TimingBudget value) */
        Profile.EnableAmbient = 0; /* Enable: 1, Disable: 0 */
        Profile.EnableSignal = 0; /* Enable: 1, Disable: 0 */
    }
    else if (dimMatrix == 8){
        Profile.RangingProfile = RS_PROFILE_8x8_CONTINUOUS;
        Profile.TimingBudget = TIMING_BUDGET; /* 5 ms < TimingBudget < 100 ms */
        Profile.Frequency = RANGING_FREQUENCY; /* Ranging frequency Hz (shall be consistent with TimingBudget value) */
        Profile.EnableAmbient = 0; /* Enable: 1, Disable: 0 */
        Profile.EnableSignal = 0; /* Enable: 1, Disable: 0 */
    }

      /* set the profile if different from default one */
      VL53L5A1_RANGING_SENSOR_ConfigProfile(VL53L5A1_DEV_CENTER, &Profile);
      status = VL53L5A1_RANGING_SENSOR_Start(VL53L5A1_DEV_CENTER, RS_MODE_BLOCKING_CONTINUOUS);

      if (status != BSP_ERROR_NONE)
      {
        printf("VL53L5A1_RANGING_SENSOR_Start failed\r\n");
        while(1);
      }

      while (1)
      {
        /* polling mode */
        status = VL53L5A1_RANGING_SENSOR_GetDistance(VL53L5A1_DEV_CENTER, &Result);
        if (status == BSP_ERROR_NONE)
        {
        	//long* res=get_3_row( &Result);
        	//return res;
        	get_3_row(&Result, distance);
        	return;
        }
        if (com_has_data())
        {
          handle_cmd(get_key());
        }

        HAL_Delay(POLLING_PERIOD);
      }

}



//fine

static void MX_53L5A1_SimpleRanging_Process(void)
{
  uint32_t Id;

  VL53L5A1_RANGING_SENSOR_ReadID(VL53L5A1_DEV_CENTER, &Id);
  VL53L5A1_RANGING_SENSOR_GetCapabilities(VL53L5A1_DEV_CENTER, &Cap);

  Profile.RangingProfile = RS_PROFILE_4x4_CONTINUOUS;
  Profile.TimingBudget = TIMING_BUDGET; /* 5 ms < TimingBudget < 100 ms */
  Profile.Frequency = RANGING_FREQUENCY; /* Ranging frequency Hz (shall be consistent with TimingBudget value) */
  Profile.EnableAmbient = 0; /* Enable: 1, Disable: 0 */
  Profile.EnableSignal = 0; /* Enable: 1, Disable: 0 */

  /* set the profile if different from default one */
  VL53L5A1_RANGING_SENSOR_ConfigProfile(VL53L5A1_DEV_CENTER, &Profile);

  status = VL53L5A1_RANGING_SENSOR_Start(VL53L5A1_DEV_CENTER, RS_MODE_BLOCKING_CONTINUOUS);

  if (status != BSP_ERROR_NONE)
  {
    printf("VL53L5A1_RANGING_SENSOR_Start failed\n");
    while(1);
  }

  while (1)
  {
    /* polling mode */
    status = VL53L5A1_RANGING_SENSOR_GetDistance(VL53L5A1_DEV_CENTER, &Result);

    if (status == BSP_ERROR_NONE)
    {
     // print_result(&Result);
    }

    if (com_has_data())
    {
      handle_cmd(get_key());
    }

    HAL_Delay(POLLING_PERIOD);
  }
}

static void print_result(RANGING_SENSOR_Result_t *Result)
{
  int8_t i, j, k, l;
  uint8_t zones_per_line;

  zones_per_line = ((Profile.RangingProfile == RS_PROFILE_8x8_AUTONOMOUS) ||
         (Profile.RangingProfile == RS_PROFILE_8x8_CONTINUOUS)) ? 8 : 4;

  display_commands_banner();

  printf("Cell Format :\r\n\n");
  printf("%ld\r\n",Result->NumberOfZones);
  for (l = 0; l < RANGING_SENSOR_NB_TARGET_PER_ZONE; l++)
  {
    printf(" \033[38;5;10m%20s\033[0m : %20s\r\n", "Distance [mm]", "Status");
    if ((Profile.EnableAmbient != 0) || (Profile.EnableSignal != 0))
    {
      printf(" %20s : %20s\r\n", "Signal [kcps/spad]", "Ambient [kcps/spad]");
    }
  }

  printf("\r\n\n");

  for (j = 0; j < Result->NumberOfZones; j += zones_per_line)
  {
	 // printf("j=%d",j);
    for (i = 0; i < zones_per_line; i++) /* number of zones per line */
      printf(" -----------------");


    for (i = 0; i < zones_per_line; i++)
      printf("|                 ");
    printf("|\r\n");

    for (l = 0; l < RANGING_SENSOR_NB_TARGET_PER_ZONE; l++)
    {

      /* Print distance and status */
      for (k = (zones_per_line - 1); k >= 0; k--)
      {
        if (Result->ZoneResult[j+k].NumberOfTargets > 0)
          printf("| \033[38;5;10m%5ld\033[0m  :  %5ld ",
              (long)Result->ZoneResult[j+k].Distance[l],
              (long)Result->ZoneResult[j+k].Status[l]);
        else
          printf("| %5s  :  %5s ", "X", "X");
      }
      printf("|\r\n");

      if ((Profile.EnableAmbient != 0) || (Profile.EnableSignal != 0))
      {
        /* Print Signal and Ambient */
        for (k = (zones_per_line - 1); k >= 0; k--)
        {
          if (Result->ZoneResult[j+k].NumberOfTargets > 0)
          {
            if (Profile.EnableSignal != 0)
              printf("| %5ld  :  ", (long)Result->ZoneResult[j+k].Signal[l]);
            else
              printf("| %5s  :  ", "X");

            if (Profile.EnableAmbient != 0)
              printf("%5ld ", (long)Result->ZoneResult[j+k].Ambient[l]);
            else
              printf("%5s ", "X");
          }
          else
            printf("| %5s  :  %5s ", "X", "X");
        }
        printf("|\r\n");
      }
    }
  }

  for (i = 0; i < zones_per_line; i++)
    printf(" -----------------");
  printf("\r\n");
}



static void toggle_resolution(void)
{
  VL53L5A1_RANGING_SENSOR_Stop(VL53L5A1_DEV_CENTER);

  switch (Profile.RangingProfile)
  {
    case RS_PROFILE_4x4_AUTONOMOUS:
      Profile.RangingProfile = RS_PROFILE_8x8_AUTONOMOUS;
      break;

    case RS_PROFILE_4x4_CONTINUOUS:
      Profile.RangingProfile = RS_PROFILE_8x8_CONTINUOUS;
      break;

    case RS_PROFILE_8x8_AUTONOMOUS:
      Profile.RangingProfile = RS_PROFILE_4x4_AUTONOMOUS;
      break;

    case RS_PROFILE_8x8_CONTINUOUS:
      Profile.RangingProfile = RS_PROFILE_4x4_CONTINUOUS;
      break;

    default:
      break;
  }

  VL53L5A1_RANGING_SENSOR_ConfigProfile(VL53L5A1_DEV_CENTER, &Profile);
  VL53L5A1_RANGING_SENSOR_Start(VL53L5A1_DEV_CENTER, RS_MODE_BLOCKING_CONTINUOUS);
}

static void toggle_signal_and_ambient(void)
{
  VL53L5A1_RANGING_SENSOR_Stop(VL53L5A1_DEV_CENTER);

  Profile.EnableAmbient = (Profile.EnableAmbient) ? 0U : 1U;
  Profile.EnableSignal = (Profile.EnableSignal) ? 0U : 1U;

  VL53L5A1_RANGING_SENSOR_ConfigProfile(VL53L5A1_DEV_CENTER, &Profile);
  VL53L5A1_RANGING_SENSOR_Start(VL53L5A1_DEV_CENTER, RS_MODE_BLOCKING_CONTINUOUS);
}

static void clear_screen(void)
{
  printf("%c[2J", 27); /* 27 is ESC command */
}

static void display_commands_banner(void)
{
  /* clear screen */
  printf("%c[2H", 27);

  printf("53L5A1 Simple Ranging demo application\r\n");
  printf("--------------------------------------\r\n\n");

  printf("Use the following keys to control application\r\n");
  printf(" 'r' : change resolution\r\n");
  printf(" 's' : enable signal and ambient\r\n");
  printf(" 'c' : clear screen\r\n");
  printf("\r\n");
}

static void handle_cmd(uint8_t cmd)
{
  switch (cmd)
  {
    case 'r':
      toggle_resolution();
      clear_screen();
      break;

    case 's':
      toggle_signal_and_ambient();
      clear_screen();
      break;

    case 'c':
      clear_screen();
      break;

    default:
      break;
  }
}

static uint8_t get_key(void)
{
  uint8_t cmd = 0;

  HAL_UART_Receive(&hcom_uart[COM1], &cmd, 1, HAL_MAX_DELAY);

  return cmd;
}

static uint32_t com_has_data(void)
{
  return __HAL_UART_GET_FLAG(&hcom_uart[COM1], UART_FLAG_RXNE);;
}

void BSP_PB_Callback(Button_TypeDef Button)
{
  PushButtonDetected = 1;
}

#ifdef __cplusplus
}
#endif
