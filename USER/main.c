/**
* @par Copyright (C): 2018-2028, Shenzhen Yahboom Tech
* @file         // main.c
* @author       // lly
* @version      // V1.0
* @date         // 240628
* @brief        // ������� Program entry
* @details      
* @par History  // �޸���ʷ��¼�б��ÿ���޸ļ�¼Ӧ�����޸����ڡ��޸��߼�
*               // �޸����ݼ���  Modification history list, each modification record should include the modification date, modifier and a brief description of the modification content
*/ 

#include "AllHeader.h"
#include "intsever.h"
//ע��:������������ʱ��Ҫ�ж��Ƿ���������ѹ
//Attention: When operating the buzzer, check if it is at normal voltage

uint8_t GET_Angle_Way=2;                             //��ȡ�Ƕȵ��㷨��1����Ԫ��  2��������  3�������˲�  //Algorithm for obtaining angles, 1: Quaternion 2: Kalman 3: Complementary filtering
float Angle_Balance,Gyro_Balance,Gyro_Turn;     		//ƽ����� ƽ�������� ת�������� //Balance tilt angle balance gyroscope steering gyroscope
int Motor_Left,Motor_Right;                 	  		//���PWM���� //Motor PWM variable
int Temperature;                                		//�¶ȱ��� 		//Temperature variable
float Acceleration_Z;                           		//Z����ٶȼ�  //Z-axis accelerometer
int Mid_Angle;                          						//��е��ֵ  //Mechanical median
float Move_X,Move_Z; //Move_X:ǰ���ٶ�  Move_Z��ת���ٶ�  //Move_X: Forward speed Move_Z: Steering speed
u8 Stop_Flag = 1; //0:��ʼ 1:ֹͣ  //0: Start 1: Stop


char showbuf[20]={'\0'};
u8* CCDShowBuf = NULL;

extern u8 newLineReceived;//�������� //Bluetooth reception
extern u8 g_lidar_go_flag;
extern u8 bulettohflag;

int main(void)
{	
		
	bsp_init();//���������ʼ�� //Basic peripheral initialization
	
	Mode_select(); //���°�������ģʽѡ�� //Press the button to end mode selection
	
	bsp_mode_init();//����ģʽ��ʼ����չ���� //Initialize and expand peripherals based on the pattern
	
	
	MPU6050_EXTI_Init();		//���жϷ������ŵ����  //This interrupt service function is placed last
	
	
	OLED_Draw_Line("put down key start!", 2, false, true);

	while(!Key1_State(1) && Stop_Flag ==1 );
	Stop_Flag = 0; //开始控制  //Start controlling

	// 调试：按键后立即闪烁LED，确认程序继续运行
	LED = 0; // 点亮LED
	delay_ms(500);
	LED = 1; // 熄灭LED
	delay_ms(500);
	LED = 0; // 点亮LED

	OLED_Draw_Line("start control!        ", 2, false, true);
	


	// 51显示定时变量
	u16 display51_cnt = 0;

	while(1)
	{
		// 51显示角度 - 每100次循环发送一次（非阻塞）
		display51_cnt++;
		if(display51_cnt >= 100)
		{
			display51_cnt = 0;
			{
				u8 frame[6];
				int angle_abs, int_part, dec_part;
				frame[0] = 'A';
				if(Angle_Balance < 0) {
					frame[1] = '-';
					angle_abs = (int)(-Angle_Balance * 10);
				} else {
					frame[1] = '+';
					angle_abs = (int)(Angle_Balance * 10);
				}
				if(angle_abs > 999) angle_abs = 999;
				int_part = angle_abs / 10;
				dec_part = angle_abs % 10;
				frame[2] = '0' + (int_part / 10);
				frame[3] = '0' + (int_part % 10);
				frame[4] = '0' + dec_part;
				frame[5] = '\n';
				{
					u8 i;
					for(i=0; i<6; i++) {
						while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
						USART_SendData(USART3, frame[i]);
					}
				}
				// 调试：发送时翻转LED状态，确认数据在发送
				LED = !LED;
			}
		}


		if(mode == Normal || mode == Weight_M)//Normal模式，载重模式
		{
			if (newLineReceived) //����ң�ط���  //Bluetooth remote control service
			{
				ProtocolCpyData();
				Protocol();
			}
			if(bulettohflag == 1) 
			{
				bulettohflag = 0;
				SendAutoUp();//�����Զ��ϱ����� Bluetooth automatically reports data 
			}
					
			sprintf(showbuf,"dis =%d mm   ",g_distance);
			OLED_Draw_Line(showbuf, 3, false, true); 
		}
		
		else if(mode == U_Follow) //����������ģʽ Ultrasonic follow mode
		{
			sprintf(showbuf,"dis =%d mm   ",g_distance);
			OLED_Draw_Line(showbuf, 3, false, true); 
		}
		
		else if(mode == U_Avoid) //����ģʽ  //Obstacle avoidance mode
		{
			APP_avoid();
		}
		
		else if(mode == PS2_Control) //PS2����ģʽ   //PS2 control mode
		{
			sprintf(showbuf,"speed = %d  ",speed_flag);
			OLED_Draw_Line(showbuf, 3, false, true); 
//			PS2_Contorl_Car(); //��ƽ���жϷ������� �ӿ���Ӧ In the balance interrupt service processing to speed up the response
		}
		
		else if(mode == Line_Track || mode == Diff_Line_track) //4·Ѳ��ģʽ  //4-way patrol mode
		{
			sprintf(showbuf,"x1 = %d  x2 = %d    ",IN_X1,IN_X2);
			OLED_Draw_Line(showbuf, 2, false, true); 
			sprintf(showbuf,"x3 = %d  x4 = %d    ",IN_X3,IN_X4);
			OLED_Draw_Line(showbuf, 3, false, true); 
		}
		
		else if(mode == CCD_Mode) //CCDѲ��ģʽ  //CCD patrol mode
		{
			OLED_Show_CCD_Image(CCDShowBuf);//CCD��ʾ //CCD display
		}
		
		else if(mode == ElE_Mode) //���Ѳ��ģʽ //Electromagnetic patrol mode
		{
			EleDataDeal();//��ʾ���  //Display results
		}
		
		else if(mode == K210_QR) //ʶ���ά��ģʽ  //Identify QR code patterns
		{
			Change_state_QR();//ʶM-oM-^GM-^\  //Identify

			// M-oM-^GM-^\M-oM-^GM-^SM-oM-^GM-^\M-oM-^GM-^\M-oM-^GM-^PM-oM-^GM-^\M-oM-^GM-^\M-oM-^GM-^\M-oM-^GM-^H
			sprintf(showbuf, "R:%d G:%d Y:%d   ", g_color_count.red_count, g_color_count.green_count, g_color_count.yellow_count);
			OLED_Draw_Line(showbuf, 2, false, true);
			sprintf(showbuf, "Total:%d        ", g_color_count.total_count);
			OLED_Draw_Line(showbuf, 3, false, true);
		}
		else if(mode == K210_SelfLearn) //K210颜色识别模式
		{
			// 显示颜色计数
			sprintf(showbuf, "R:%d G:%d Y:%d   ", g_color_count.red_count, g_color_count.green_count, g_color_count.yellow_count);
			OLED_Draw_Line(showbuf, 2, false, true);
			sprintf(showbuf, "Total:%d        ", g_color_count.total_count);
			OLED_Draw_Line(showbuf, 3, false, true);
		}
		else if(mode == K210_mnist) //ʶ������ģʽ  //Identify numerical patterns
		{
			Change_state_minst();
		}
		
		
		
	}
}


