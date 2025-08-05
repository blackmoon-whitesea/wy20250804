//TIM3 Slave Mode配置示例
#include "timer.h"
#include "stm32f4xx.h"
#include "misc.h"

// 示例1: 配置TIM3为触发模式 - 从TIM2触发启动
void TIM3_Slave_Trigger_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 使能TIM2和TIM3时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);
    
    // 配置TIM2作为主定时器 (Master)
    TIM_TimeBaseInitStructure.TIM_Prescaler = 8399;        // 84MHz/8400 = 10KHz
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 9999;           // 1秒周期
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
    
    // 配置TIM2输出触发信号 (TRGO)
    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);  // 更新事件作为触发输出
    
    // 配置TIM3作为从定时器 (Slave)
    TIM_TimeBaseInitStructure.TIM_Prescaler = 83;          // 84MHz/84 = 1MHz
    TIM_TimeBaseInitStructure.TIM_Period = 999;            // 1ms周期
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
    
    // 设置TIM3触发源为TIM2 (ITR1)
    TIM_SelectInputTrigger(TIM3, TIM_TS_ITR1);  // TIM2对应ITR1
    
    // 设置TIM3为触发模式 - TIM2触发时启动TIM3
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Trigger);
    
    // 配置TIM3中断
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 启动TIM2 (主定时器)
    TIM_Cmd(TIM2, ENABLE);
    // TIM3会在TIM2触发时自动启动
}

// 示例2: 配置TIM3为门控模式 - 外部信号控制
void TIM3_Slave_Gated_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    // 配置PA6为TIM3_CH1 (外部触发输入)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
    
    // 基本定时器配置
    TIM_TimeBaseInitStructure.TIM_Prescaler = 83;          // 84MHz/84 = 1MHz
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 999;            // 1ms基准
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
    
    // 配置输入捕获 (用作触发输入)
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x0;
    TIM_ICInit(TIM3, &TIM_ICInitStructure);
    
    // 设置触发源为TI1FP1 (CH1输入)
    TIM_SelectInputTrigger(TIM3, TIM_TS_TI1FP1);
    
    // 设置为门控模式 - 高电平时计数，低电平时停止
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Gated);
    
    // 配置中断
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 启动定时器
    TIM_Cmd(TIM3, ENABLE);
}

// 示例3: 配置TIM3为复位模式 - 外部信号复位计数器
void TIM3_Slave_Reset_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    // 配置PA7为TIM3_CH2 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_TIM3);
    
    // 基本定时器配置
    TIM_TimeBaseInitStructure.TIM_Prescaler = 83;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 9999;           // 10ms周期
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
    
    // 设置触发源为TI2FP2 (CH2输入)
    TIM_SelectInputTrigger(TIM3, TIM_TS_TI2FP2);
    
    // 设置为复位模式 - 触发信号时复位计数器
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);
    
    // 配置中断
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    TIM_Cmd(TIM3, ENABLE);
}

// 示例4: 修改现有TIM3为从模式(保持1ms基准功能)
void TIM3_Add_SlaveMode_To_Existing(void)
{
    // 假设当前TIM3已经初始化并运行
    // 这个函数演示如何在不影响现有功能的情况下添加slave mode
    
    // 暂停TIM3
    TIM_Cmd(TIM3, DISABLE);
    
    // 配置GPIO用于外部触发 (PA6 = TIM3_CH1)
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
    
    // 设置触发源 (可选择不同的触发源)
    TIM_SelectInputTrigger(TIM3, TIM_TS_TI1FP1);  // 使用CH1作为触发
    
    // 选择slave mode
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Gated); // 门控模式示例
    
    // 重新启动TIM3
    TIM_Cmd(TIM3, ENABLE);
}
