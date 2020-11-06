# Encoder-driver

---

[TOC]

---



适用于F103C8T6的增量式旋转编码器驱动（本来是为Arduino设计的一个模块,型号KY-040），如下图：
<br/>![KY-040](https://ss1.bdstatic.com/70cFvXSh_Q1YnxGkpoWK1HF6hhy/it/u=4007454106,1722868969&fm=26&gp=0.jpg)</br>
第一次和重新构写编码器驱动的调试过程都花了很长时间，但实际原理真的不难，简单到找不到相应的手册，只有商家的简介
<br/>[编码器原理介绍]（https://detail.tmall.com/item.htm?spm=a230r.1.14.24.2157211a2JGcFs&id=610333515676&ns=1&abbucket=2&skuId=4446450308834）</br>

驱动用于自制冰箱温度阈值调节，可自行修改变量

---



## Background

大一暑假和同学一起做了个可以自动控温的小冰箱，用作社团招新活动使用的，现在让师弟重新做起来，然后我也想重新写一下编码器的代码，就有了这部分

相比于以前编码器部分，有如下改变：

1.使用外部中断，响应更快（甚至于有点太快了），消抖不好写

2.使用宏定义，易读性增加一点

3.函数模块化

## Install

使用MDK5编译，安装st公司f1系列安装包即可

## Usage

**1.根据编码器原理和模块简介和改变温度阈值需求，在encoder.h中宏定义函数及声明变量，如下所示：**

```C
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
```



**2.在encoder.c中编写模块的GPIO、EXTI、NVIC等初始化函数，并封装成模块初始化函数Encoder_GPIO_Init(void)**

```c
void Encoder_Init(void)
{		
		Encoder_GPIO_Init();
		Encoder_CLK_EXTI_Init();
		Encoder_SW_EXTI_Init();
		Encoder_NVIC_Config();
}

void Encoder_GPIO_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;		//定义句柄变量
		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//先使能外设IO PORTC时钟 
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//注意要打开复用功能时钟
	
		//旋转编码器引脚定义，PA7为CLK,PA6为DT
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6;	 // 端口配置		
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 //浮空输入
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
		GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIO 
		
		//旋转编码器引脚定义，PA5用于编码器按键SW
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	 // 端口配置	用于SW
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //下拉输入
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
		GPIO_Init(GPIOA, &GPIO_InitStructure);	
}

void Encoder_CLK_EXTI_Init(void)
{
		EXTI_InitTypeDef EXTI_InitStructure;		//定义句柄变量
	
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource7); //选择EXTI信号源
		EXTI_InitStructure.EXTI_Line = EXTI_Line7;               //中断线选择
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;      //EXTI为中断模式
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  //双边沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;                //使能中断
    EXTI_Init(&EXTI_InitStructure);
}

void Encoder_SW_EXTI_Init(void)
{
		EXTI_InitTypeDef EXTI_InitStructure;		//定义句柄变量
	
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource5); //选择EXTI信号源
		EXTI_InitStructure.EXTI_Line = EXTI_Line5;               //中断线选择
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;      //EXTI为中断模式
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //下降沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;                //使能中断
    EXTI_Init(&EXTI_InitStructure);
}

void Encoder_NVIC_Config(void)
{
		NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);           //配置NVIC优先级分组为1

    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;        //中断源：[9:5]，位于“stm32f10x.h”中
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //抢占优先级：1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        //子优先级：1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           //使能中断通道
    NVIC_Init(&NVIC_InitStructure);
}
```



**3.编写外部中断服务函数（stm32f103_it.c）：其中CLK引脚因为模块特性，所以设置了双边沿触发，消抖目前还没想到比延时更好的方法，现在的消抖方法基本满足正常速度的方向识别**

```C
void EXTI9_5_IRQHandler(void)
{
		static char i = 0;
	
		if(EXTI_GetITStatus(EXTI_Line7) != RESET)			//编码器CLK发生跳变
		{
				static uint8_t temp=0;
				temp = Encoder_GetCLK;
				delay_ms(7);															//调试得出的最佳消抖延时，之后再看看能不能根据转速调整
				if(Encoder_GetCLK == Encoder_GetCLK)	Encoder_Read_dir();
				EXTI_ClearITPendingBit(EXTI_Line7);
		}
		else if(EXTI_GetITStatus(EXTI_Line5) != RESET)		//编码器按键按下
		{
				EXTI_ClearITPendingBit(EXTI_Line5);
			
				delay_ms(10);
				if(Encoder_GetSW)
				{
					printf("按下了按键\r\n");
				}
		}
}
```



**4.消抖后调用Encoder_Read_dir()完成控制**

```C
void Encoder_Read_dir(void)
{
		char encoder_dir;
					
		encoder_dir = Encoder_GetCLK ^ Encoder_GetDT;	//使用异或，减少赋值步骤
		if(encoder_dir && Goal_Temp < 30)				//限制阈值上限为30摄氏度
		{
				Goal_Temp += Delta_Temp;			//Goal_Temp为设置的温度阈值变量
		}
		else if (!encoder_dir && Goal_Temp > -10)		//限制阈值下限为-10摄氏度
		{
				Goal_Temp -= Delta_Temp;
		}
				
}
```



**5.人手正常速度转动编码器，功能正常，但转速大的时候还是会出现方向反馈错误的问题**



## Badge

readme.md标准模板：

```
[![standard-readme compliant](https://img.shields.io/badge/readme%20style-standard-brightgreen.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)
```



