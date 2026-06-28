/*==========================================================================
 * 项目: STM32平衡车角度显示 - 51单片机端 最终版本
 * 功能: 完整接收STM32的串口帧，解析并显示角度
 * 适配: 12MHz晶振、4800波特率、P1口LCD引脚
 *==========================================================================*/
#include "reg51.h"

/*--- LCD1602引脚定义 (你的硬件正确引脚) ---*/
sbit LCD_RS = P1^0;
sbit LCD_RW = P1^1;
sbit LCD_E  = P1^2;
#define LCD_Data P0

#define Busy 0x80

/*--- 协议定义 ---*/
#define FRAME_HEADER 'A'
#define FRAME_END    '\n'
#define FRAME_LEN    6

/*--- 全局变量 ---*/
unsigned char rx_buf[FRAME_LEN];
unsigned char rx_count = 0;
unsigned char rx_done = 0;

/*--- 显示字符串 ---*/
unsigned char code title[] = {"Car Angle:"};

/*--- 函数声明 ---*/
void UART_Init(void);
void WriteDataLCD(unsigned char WDLCD);
void WriteCommandLCD(unsigned char WCLCD, unsigned char BuysC);
unsigned char ReadStatusLCD(void);
void LCDInit(void);
void LCDClear(void);
void DisplayOneChar(unsigned char X, unsigned char Y, unsigned char DData);
void DisplayListChar(unsigned char X, unsigned char Y, unsigned char code *DData);
void Delay5Ms(void);
void Delay400Ms(void);

/*==========================================================================
 * 主函数
 *==========================================================================*/
void main(void)
{
    unsigned char i;

    Delay400Ms();
    LCDInit();
    LCDClear();
    UART_Init();
    EA = 1;

    DisplayListChar(0, 0, title);
    DisplayListChar(0, 1, "Waiting...");

    while(1)
    {
        if(rx_done)
        {
            rx_done = 0;
            // 显示收到的完整帧
            WriteCommandLCD(0xC0, 1);
            for(i=0; i<6; i++)
            {
                WriteDataLCD(rx_buf[i]);
            }
        }
    }
}

/*==========================================================================
 * 串口初始化 (12MHz+4800波特率，正确配置！)
 *==========================================================================*/
void UART_Init(void)
{
    TMOD = 0x20;
    TH1  = 0xF3;
    TL1  = 0xF3;
    PCON |= 0x80; // 波特率加倍，解决12MHz的误差问题
    TR1  = 1;
    SCON = 0x50;
    ES   = 1;
}

/*==========================================================================
 * 串口中断：完整接收6字节帧
 *==========================================================================*/
void UART_ISR(void) interrupt 4
{
    unsigned char tmp;
    if(RI)
    {
        RI = 0;
        tmp = SBUF;

        if(tmp == FRAME_HEADER)
        {
            rx_buf[0] = tmp;
            rx_count = 1;
        }
        else if(rx_count > 0 && rx_count < FRAME_LEN)
        {
            rx_buf[rx_count] = tmp;
            rx_count++;

            if(rx_count >= FRAME_LEN)
            {
                if(rx_buf[FRAME_LEN-1] == FRAME_END)
                {
                    rx_done = 1;
                }
                rx_count = 0;
            }
        }
    }
}

/*==========================================================================
 * LCD驱动
 *==========================================================================*/
void WriteDataLCD(unsigned char WDLCD)
{
    ReadStatusLCD();
    LCD_Data = WDLCD;
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_E = 0;
    LCD_E = 0;
    LCD_E = 1;
}

void WriteCommandLCD(unsigned char WCLCD, unsigned char BuysC)
{
    if(BuysC) ReadStatusLCD();
    LCD_Data = WCLCD;
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_E = 0;
    LCD_E = 0;
    LCD_E = 1;
}

unsigned char ReadStatusLCD(void)
{
    LCD_Data = 0xFF;
    LCD_RS = 0;
    LCD_RW = 1;
    LCD_E = 0;
    LCD_E = 0;
    LCD_E = 1;
    while(LCD_Data & Busy);
    return(LCD_Data);
}

void LCDInit(void)
{
    LCD_Data = 0;
    WriteCommandLCD(0x38, 0);
    Delay5Ms();
    WriteCommandLCD(0x38, 0);
    Delay5Ms();
    WriteCommandLCD(0x38, 0);
    Delay5Ms();
    WriteCommandLCD(0x38, 1);
    WriteCommandLCD(0x08, 1);
    WriteCommandLCD(0x01, 1);
    WriteCommandLCD(0x06, 1);
    WriteCommandLCD(0x0C, 1);
}

void LCDClear(void)
{
    WriteCommandLCD(0x01, 1);
}

void DisplayOneChar(unsigned char X, unsigned char Y, unsigned char DData)
{
    Y &= 0x1;
    X &= 0xF;
    if(Y) X |= 0x40;
    X |= 0x80;
    WriteCommandLCD(X, 0);
    WriteDataLCD(DData);
}

void DisplayListChar(unsigned char X, unsigned char Y, unsigned char code *DData)
{
    unsigned char ListLength = 0;
    Y &= 0x1;
    X &= 0xF;
    while(DData[ListLength] >= 0x20)
    {
        if(X <= 0xF)
        {
            DisplayOneChar(X, Y, DData[ListLength]);
            ListLength++;
            X++;
        }
    }
}

void Delay5Ms(void)
{
    unsigned int TempCyc = 5552;
    while(TempCyc--);
}

void Delay400Ms(void)
{
    unsigned char TempCycA = 5;
    unsigned int TempCycB;
    while(TempCycA--)
    {
        TempCycB = 7269;
        while(TempCycB--);
    }
}