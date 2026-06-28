#include "bsp.h"
#include "intsever.h"

void bsp_init(void)
{
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//��������жϷ���  Interrupt grouping in library settings
	DIY_NVIC_PriorityGroupConfig(2);	  //�����жϷ���   //Set interrupt grouping
	delay_init();	    	            //��ʱ������ʼ��	 //Delay function initialization
	JTAG_Set(JTAG_SWD_DISABLE);     //�ر�JTAG�ӿ�    //Close JTAG interface
	JTAG_Set(SWD_ENABLE);           //��SWD�ӿ� �������������SWD�ӿڵ��� //Opening the SWD interface allows for debugging using the motherboard's SWD interface
	
	//led/beep
	init_led_gpio();								//����LED  		//Onboard LED
	init_beep();										//���ط����� //Onboard buzzer
	Key1_GPIO_Init();								//���ذ���	 //Onboard buttons
	
	
	BalanceCar_Motor_Init();     	//���GPIO��ʼ��  //Motor GPIO initialization
	BalanceCar_PWM_Init(2880,0); 	//��ʼ��PWM 25Khz ����Ӳ���ӿڣ������������   Initialize PWM 25Khz and motor hardware interface for driving the motor
	Encoder_Init_TIM3();            //��ʼ��������3  Initialize encoder 3
	Encoder_Init_TIM4();            //��ʼ��������4  Initialize encoder 4
	
	uart_init(115200);	            //����1��ʼ��  Serial port 1 initialization
	
	delay_ms(300);
	
	IIC_MPU6050_Init();							//������i2c��ʼ��   Gyroscope I2C initialization
	MPU6050_initialize();						//���������̳�ʼ��  Gyroscope range initialization
	DMP_Init();                     //DMP��ʼ��    DMP initialization
	
	OLED_I2C_Init();							 //oled��ʼ��  OLED initialization
	 
	Battery_init();									//��ص�������ʼ�� Initialization of battery level detection
}

//����ģʽѡ�����ģ��ĳ�ʼ��  Initialize modules based on mode selection
//M-oM-?M-=M-oM-?M-=M-JM-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=  Initialize modules based on mode selection
void bsp_mode_init(void)
{
	if(mode == Normal || mode == Weight_M)
	{
		bluetooth_init();								//M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=   Bluetooth initialization
		TIM2_Cap_Init(0XFFFF,72-1);    //M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=  Ultrasonic initialization
	}
	else if(mode == U_Avoid || mode == U_Follow)
	{
		TIM2_Cap_Init(0XFFFF,72-1);    //M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=  Ultrasonic initialization
	}
	else if(mode == PS2_Control)
	{
		PS2_Init();//M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=  Controller initialization
		PS2_SetInit();
	}
	else if(mode == Line_Track || mode == Diff_Line_track)
	{
		irtracking_init(); //M-oM-?M-=M-oM-?BM-?M-oM-?M-=M-oM-?M-=  Infrared patrol line
	}
	else if(mode == CCD_Mode)
	{
		ccd_Init();//CCDM-oM-?BM-?M-oM-?M-=M-oM-?M-=  CCD patrol line
	}
	else if(mode == ElE_Mode)
	{
		ele_Init(); //M-oM-?M-=M-oM-?BM-?M-oM-?M-=M-oM-?M-=  Electromagnetic patrol line
	}
	else if((mode == K210_QR) || (mode == K210_Line) || (mode == K210_Follow)|| (mode == K210_SelfLearn)|| (mode == K210_mnist))
	{
		USART2_init(115200);						//k210M-oM-?M-=M-SM-?M-oM-?M-=  K210 interface
	}

	// 51M-oM-?M-=M-oM-?M-=M-JM->M-oM-?M-=M-oM-?M-=USART3M-oM-?M-=M-oM-?M-=M-JM-<M-oM-?M-=M-oM-?M-=
	{
		GPIO_InitTypeDef GPIO_51;
		USART_InitTypeDef USART_51;

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

		GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE);

		// PC10 - USART3 TX
		GPIO_51.GPIO_Pin = GPIO_Pin_10;
		GPIO_51.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_51.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_51);

		// USART3M-oM-?M-=M-oM-?M-=M-JM-PM-: 4800M-oM-?M-=M-oM-?M-=M-JM-^QM-IM-oM-?M-=M-oM-?M-=
		USART_51.USART_BaudRate = 4800;
		USART_51.USART_WordLength = USART_WordLength_8b;
		USART_51.USART_StopBits = USART_StopBits_1;
		USART_51.USART_Parity = USART_Parity_No;
		USART_51.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_51.USART_Mode = USART_Mode_Tx;
		USART_Init(USART3, &USART_51);

		USART_Cmd(USART3, ENABLE);
	}

	TIM6_Init();									//LEDM-oM-?M-=M-oM-?M-=M-KM-8M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-QM-9M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-BM-6M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=M-oM-?M-=
}



void JTAG_Set(u8 mode)
{
	u32 temp;
	temp=mode;
	temp<<=25;
	RCC->APB2ENR|=1<<0;     //�������ʱ��	  Activate auxiliary clock  
	AFIO->MAPR&=0XF8FFFFFF; //���MAPR��[26:24] Clear MAPR [26:24]
	AFIO->MAPR|=temp;       //����jtagģʽ Set jtag mode
} 


/**************************************************************************
Function: Set NVIC group
Input   : NVIC_Group
Output  : none
�������ܣ������жϷ���
��ڲ�����NVIC_Group:NVIC���� 0~4 �ܹ�5�� 	
����  ֵ����
**************************************************************************/ 
void DIY_NVIC_PriorityGroupConfig(u8 NVIC_Group)	 
{ 
	u32 temp,temp1;	  
	temp1=(~NVIC_Group)&0x07;//ȡ����λ Take the last three
	temp1<<=8;
	temp=SCB->AIRCR;  //��ȡ��ǰ������  Read previous settings
	temp&=0X0000F8FF; //�����ǰ����   Clear previous groups
	temp|=0X05FA0000; //д��Կ��  Write the key
	temp|=temp1;	   
	SCB->AIRCR=temp;  //���÷���	  Set grouping   	  				   
}

