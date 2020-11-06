#include "encoder.h"

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

void Encoder_Read_dir(void)
{
			char encoder_dir;
					
			encoder_dir = Encoder_GetCLK ^ Encoder_GetDT;		//使用异或，减少赋值步骤
			if(encoder_dir && Goal_Temp < 30)								//限制阈值上限为30摄氏度
			{
					Goal_Temp += Delta_Temp;										//Goal_Temp为设置的温度阈值变量
			}
			else if (!encoder_dir && Goal_Temp > -10)				//限制阈值下限为-10摄氏度
			{
					Goal_Temp -= Delta_Temp;
			}
				
}
	