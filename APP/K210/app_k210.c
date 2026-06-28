#include "app_k210.h"

// 颜色计数数据
K210_Color_Count_t g_color_count = {0, 0, 0, 0};

char buf_msg[20] = {'\0'};
uint8_t g_new_flag = 0;
uint8_t g_index = 0; 
uint8_t g_new_data = 0; //1:���ݽ������ 1: Data reception completed

//Function function: Retain the information of k210
//Pass in function: recv-msg: Information sent from serial port
// ��������:����k210����Ϣ
// ���뺯��:recv_msg:���ڷ�������Ϣ
void Deal_K210_QR(uint8_t recv_msg)
{
	if (recv_msg == '$' && g_new_flag == 0)
	{
		g_new_flag = 1;
		memset(buf_msg, 0, sizeof(buf_msg)); // Clear old data ���������
		return;
	}
	if(g_new_flag == 1)
	{
		if (recv_msg == '#')
		{
			g_new_flag = 0;
			g_index = 0;
			g_new_data = 1;
		}

		if (g_new_flag == 1 && recv_msg != '$')
		{
			buf_msg[g_index++] = recv_msg;

			if(g_index > 20) //������� Array overflow
			{
				g_index = 0;
				g_new_flag = 0;
				g_new_data = 0;
				memset(buf_msg, 0, sizeof(buf_msg)); // Clear old data ���������
			}

		}
	}
}



#define Trun_speed 137  //转向速度降低30% (原196)  //Turn speed reduced by 30%
#define Go_speed 7      //速度降低30% (原10，再降30%)  //Speed reduced by 30%

/*
 * �������ܣ�����k210�����Ĳ�ָͬ����в�ͬ�Ķ���
 *
 *Function: perform different actions according to different instructions sent by k210
 * 
*/
void Change_state_QR(void)
{
	if(g_new_data == 1)
	{
		g_new_data = 0;  
		if (strcmp("goback", buf_msg) == 0 )
		{
			//��������  Buzzer sounds
			BEEP_BEEP = 1;
			delay_time(20); //200ms
			BEEP_BEEP = 0;
			//С�����������ֹͣ The car moves back for two seconds and then stops
			Move_X = -Go_speed;
			my_delay(2);
			Move_X = 0;
		}
		else if (strcmp("goahead", buf_msg) == 0 )
		{
			//蜂鸣器响  Buzzer sounds
			BEEP_BEEP = 1;
			delay_time(20); //200ms
			BEEP_BEEP = 0;
			//小车向前两秒后停止  The car moves forward for two seconds and then stops
			Move_X = Go_speed;
			my_delay(2);
			Move_X = 0;
		}
		else if (strcmp("turnleft", buf_msg) == 0)
		{
			//蜂鸣器响  Buzzer sounds
			BEEP_BEEP = 1;
			delay_time(20); //200ms
			BEEP_BEEP = 0;
			//向左转30度  Turn left 30 degrees
			Move_Z = -Trun_speed;
			my_delay(1);  // 延时1秒实现转向  //Delay 1 second for turning
			Move_Z = 0;
		}
		else if (strcmp("turnright", buf_msg) == 0 )
		{
			//蜂鸣器响  Buzzer sounds
			BEEP_BEEP = 1;
			delay_time(20); //200ms
			BEEP_BEEP = 0;
			//向右转30度  Turn right 30 degrees
			Move_Z = Trun_speed;
			my_delay(1);  // 延时1秒实现转向  //Delay 1 second for turning
			Move_Z = 0;
		}
		else if (strcmp("buzzer", buf_msg) == 0 )
		{
			//��������3��  The buzzer sounds 3 times
			for (u8 i =0;i<3;i++)
			{
				BEEP_BEEP = 1;
				delay_time(20); //200ms
				BEEP_BEEP = 0;
				delay_time(20); //200ms
			}
			
		}
		
	}

}


//����ѧϰ
//Self directed learning
// 颜色识别数据解析
static void Parse_Color_Count(void)
{
    if (buf_msg[0] == 'C')
    {
        g_color_count.red_count = buf_msg[1] - '0';
        g_color_count.green_count = buf_msg[2] - '0';
        g_color_count.yellow_count = buf_msg[3] - '0';
        g_color_count.total_count = g_color_count.red_count +
                                    g_color_count.green_count +
                                    g_color_count.yellow_count;
    }
}

void Deal_K210_self(uint8_t recv_msg)
{
	if (recv_msg == '$' && g_new_flag == 0)
	{
		g_new_flag = 1;
		memset(buf_msg, 0, sizeof(buf_msg)); // Clear old data ���������
		return;
	}
	if(g_new_flag == 1)
	{
		if (recv_msg == '#')
		{
			g_new_flag = 0;
			g_index = 0;
			g_new_data = 1;
			Parse_Color_Count(); // 解析颜色数据
		}

		if (g_new_flag == 1 && recv_msg != '$')
		{
			buf_msg[g_index++] = recv_msg;

			if(g_index > 20) //������� Array overflow
			{
				g_index = 0;
				g_new_flag = 0;
				g_new_data = 0;
				memset(buf_msg, 0, sizeof(buf_msg)); // Clear old data ���������
			}

		}
	}
}



#define Trun_speed_self 137  //转向速度降低30% (原196)  //Turn speed reduced by 30%
#define Go_speed_self 7      //速度降低30% (原10，再降30%)  //Speed reduced by 30%
/*
 * �������ܣ�����k210�����Ĳ�ָͬ����в�ͬ�Ķ���
 *
 *Function: perform different actions according to different instructions sent by k210
 * 
*/
void Change_state_self(void)
{
	if(g_new_data == 1)
	{
		g_new_data = 0;  
		if (strcmp("1", buf_msg) == 0 )
		{
			//С��ǰ�������ֹͣ  The car moves forward for two seconds and then stops
			Move_X = Trun_speed_self;
			my_delay(2);
			Move_X = 0;
		}
		else if (strcmp("2", buf_msg) == 0)
		{
			//С����ת1sȻ��ǰ��1���ֹͣ The car turns left for 1 second and then moves forward for 1 second before stopping
			Move_Z = -Trun_speed_self;
			my_delay(1);
			
			Move_Z = 0;
			Move_X = Go_speed_self;
			
			my_delay(1);
			Move_X = 0;
		}
		else if (strcmp("3", buf_msg) == 0 )
		{
			//С����ת1sȻ��ǰ��1���ֹͣ  The car turns right for 1 second and then moves forward for 1 second before stopping
			Move_Z = Trun_speed_self;
			my_delay(1);
			
			Move_Z = 0;
			Move_X = Go_speed_self;
		
			my_delay(1);
			Move_X = 0;
		}
		
	}

}


void Deal_K210_minst(uint8_t recv_msg)
{
	if (recv_msg == '$' && g_new_flag == 0)
	{
		g_new_flag = 1;
		memset(buf_msg, 0, sizeof(buf_msg)); // Clear old data ���������
		return;
	}
	if(g_new_flag == 1)
	{
		if (recv_msg == '#')
		{
			g_new_flag = 0;
			g_index = 0;
			g_new_data = 1;
		}

		if (g_new_flag == 1 && recv_msg != '$')
		{
			buf_msg[g_index++] = recv_msg;

			if(g_index > 20) //������� Array overflow
			{
				g_index = 0;
				g_new_flag = 0;
				g_new_data = 0;
				memset(buf_msg, 0, sizeof(buf_msg)); // Clear old data ���������
			}

		}
	}
}


#define Trun_speed_minst 137  //转向速度降低30% (原196)  //Turn speed reduced by 30%

void Change_state_minst(void)
{
	if(g_new_data == 1)
	{
		g_new_data = 0;  
		if (strcmp("6", buf_msg) == 0 )
		{
			OLED_Draw_Line("num:6!  ", 3, false, true);
			//��������1s Buzzer sounds for 1s
			BEEP_BEEP = 1;
			my_delay(1);
			BEEP_BEEP = 0;
			
		}
		else if (strcmp("2", buf_msg) == 0)
		{
			OLED_Draw_Line("num:2!  ", 3, false, true);
			//С����ת2sȻ��ֹͣ  The car turns left for 2 seconds and then stops
			Move_Z = -Trun_speed_minst;
			my_delay(1);
			my_delay(1);
			Move_Z = 0;
			
		}
		else if (strcmp("3", buf_msg) == 0 )
		{
			OLED_Draw_Line("num:3!  ", 3, false, true);
			//С����ת2sȻ��ֹͣ The car turns right for 2 seconds and then stops
			Move_Z = Trun_speed_minst;
			my_delay(1);
			my_delay(1);
			Move_Z = 0;
			
		}
		
	}

}






