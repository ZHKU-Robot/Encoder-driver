#ifndef _Encoder_H__
#define _Encoder_H__

#include "stm32f10x_gpio.h"
#include "usart.h"
#include "delay.h"

#define Encoder_GetCLK  PAin(7)		//读取时钟引脚
#define Encoder_GetDT   PAin(6)		//读取信号引脚
#define Encoder_GetSW   PAin(5)		//编码器按键输入
#define Delta_Temp			0.5				//每次温度阈值的改变值


extern float Goal_Temp,Actual_Temp;
extern uint8_t CLK_debounce,CLK_debounce_LAST,DT_debounce;

void Encoder_Init(void);
void Encoder_GPIO_Init(void);
void Encoder_CLK_EXTI_Init(void);
void Encoder_SW_EXTI_Init(void);
void Encoder_NVIC_Config(void);
void Encoder_Read_dir(void);

#endif

