/**
  ******************************************************************************
  * @file    usb_bsp.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-January-2014
  * @brief   This file Provides Device Core configuration Functions
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usb_bsp.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
#if defined USB_CLOCK_SOURCE_CRS
 static void CRS_Config(void);
#endif /* USB_CLOCK_SOURCE_CRS */

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize BSP configurations
  * @param  None
  * @retval None
  */

void USB_BSP_Init(USB_CORE_HANDLE *pdev)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
#if defined USB_DEVICE_LOW_PWR_MGMT_SUPPORT
  EXTI_InitTypeDef EXTI_InitStructure;
#endif  
  /* Enable all GPIO clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC |
                        RCC_AHBPeriph_GPIOD | RCC_AHBPeriph_GPIOE | RCC_AHBPeriph_GPIOF , ENABLE);
  
  /* Configure all GPIOs in AN mode to disable current to flow into or out 
  of GPIOs when in suspend mode  */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_Init(GPIOE, &GPIO_InitStructure);
  GPIO_Init(GPIOF, &GPIO_InitStructure);
  
  /* Disable all GPIO clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC | RCC_AHBPeriph_GPIOD 
                        | RCC_AHBPeriph_GPIOE | RCC_AHBPeriph_GPIOF, DISABLE);
  /* Enable USB clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
  
#if defined USB_CLOCK_SOURCE_CRS
  
  /*For using CRS, you need to do the following:
  - Enable HSI48 (managed by the SystemInit() function at the application startup)
  - Select HSI48 as USB clock
  - Enable CRS clock
  - Set AUTOTRIMEN
  - Set CEN
  */
  
  /* Select HSI48 as USB clock */
  RCC_USBCLKConfig(RCC_USBCLK_HSI48);
  
  /* Configure the Clock Recovery System */
  CRS_Config();  
#else 
  /* Configure PLL to be used as USB clock:
     - Enable HSE external clock (for this example the system is clocked by HSI48
       managed by the SystemInit() function at the application startup)
     - Enable PLL
     - Select PLL as USB clock */
  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);
  
  /* Wait till HSE is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
  {}
  
  /* Enable PLL */
  RCC_PLLCmd(ENABLE);
  
  /* Wait till PLL is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
  {}
  
  /* Configure USBCLK from PLL clock */
  RCC_USBCLKConfig(RCC_USBCLK_PLLCLK);
   
#endif /*USB_CLOCK_SOURCE_CRS */ 
  
  /*JoyStick IOs Configuration*/  
//  STM_EVAL_LEDInit(LED1);
//  STM_EVAL_PBInit(BUTTON_LEFT, BUTTON_MODE_GPIO);
//  STM_EVAL_PBInit(BUTTON_UP, BUTTON_MODE_GPIO);
//  STM_EVAL_PBInit(BUTTON_DOWN, BUTTON_MODE_GPIO);
  
  /* Configure the Tamper button in EXTI mode */
//  STM_EVAL_PBInit(BUTTON_TAMPER, Mode_EXTI);
  
  /* Clear the Tamper EXTI line pending bit */
//  EXTI_ClearITPendingBit(TAMPER_BUTTON_EXTI_LINE);
  
#ifdef USB_DEVICE_LOW_PWR_MGMT_SUPPORT  
  
//  /* Enable the PWR clock */
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  
//  /* EXTI line 18 is connected to the USB Wakeup from suspend event   */
//  EXTI_ClearITPendingBit(EXTI_Line18);
//  EXTI_InitStructure.EXTI_Line = EXTI_Line18;
//  /*Must Configure the EXTI Line 18 to be sensitive to rising edge*/
//  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
//  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//  EXTI_Init(&EXTI_InitStructure);
#endif /*USB_DEVICE_LOW_PWR_MGMT_SUPPORT */
  
}

/**
  * @brief  Enable USB Global interrupt
  * @param  None
  * @retval None
  */
void USB_BSP_EnableInterrupt(USB_CORE_HANDLE *pdev)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Enable the USB interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USB_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

#if defined USB_CLOCK_SOURCE_CRS
/**
  * @brief  Configure CRS peripheral to automatically trim the HSI 
  *         oscillator according to USB SOF
  * @param  None
  * @retval None
  */
static void CRS_Config(void)
{
  /*Enable CRS Clock*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CRS, ENABLE);
  
  /* Select USB SOF as synchronization source */
  CRS_SynchronizationSourceConfig(CRS_SYNCSource_USB);
  
  /*Enables the automatic hardware adjustment of TRIM bits: AUTOTRIMEN:*/
  CRS_AutomaticCalibrationCmd(ENABLE);
  
  /*Enables the oscillator clock for frequency error counter CEN*/
  CRS_FrequencyErrorCounterCmd(ENABLE);
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
