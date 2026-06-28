#ifndef __APP_K210_H_
#define __APP_K210_H_

#include "AllHeader.h"

void Deal_K210_QR(uint8_t recv_msg);
void Change_state_QR(void);


void Deal_K210_self(uint8_t recv_msg);
void Change_state_self(void);

void Deal_K210_minst(uint8_t recv_msg);
void Change_state_minst(void);

// 颜色计数数据结构（用于K210_SelfLearn模式）
typedef struct {
	uint8_t red_count;      // 红色数量
	uint8_t green_count;    // 绿色数量
	uint8_t yellow_count;   // 黄色数量
	uint8_t total_count;    // 总数
} K210_Color_Count_t;

extern K210_Color_Count_t g_color_count;

#endif

