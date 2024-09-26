/**
 * This file is part of the RVPC project 
 * https://github.com/OLIMEX/RVPC
 * https://www.olimex.com/Products/Retro-Computers/RVPC/open-source-hardware
 * 
 * Copyright (c) 2024 Olimex Ltd
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "misc.h"

void GPIO_Config(GPIO_TypeDef *GPIO_port, uint16_t GPIO_pin, GPIOMode_TypeDef GPIO_mode) {
	GPIO_InitTypeDef Config = {0};

	if (GPIO_port == GPIOA) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	} else if (GPIO_port == GPIOC) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	} else if (GPIO_port == GPIOD) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	}

	Config.GPIO_Pin = GPIO_pin;
	Config.GPIO_Mode = GPIO_mode;
	Config.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_port, &Config);
}

void GPIO_Interrupt(uint8_t GPIO_PortSource, uint8_t GPIO_PinSource, uint32_t GPIO_Line, EXTITrigger_TypeDef GPIO_Trigger) {
	EXTI_InitTypeDef kbd_exti = {0};
	GPIO_EXTILineConfig(GPIO_PortSource, GPIO_PinSource);
    kbd_exti.EXTI_Line = GPIO_Line;
    kbd_exti.EXTI_Mode = EXTI_Mode_Interrupt;
    kbd_exti.EXTI_Trigger = GPIO_Trigger;
    kbd_exti.EXTI_LineCmd = ENABLE;
    EXTI_Init(&kbd_exti);

	NVIC_InitTypeDef kbd_nvic = {0};
	kbd_nvic.NVIC_IRQChannel = EXTI7_0_IRQn;
    kbd_nvic.NVIC_IRQChannelPreemptionPriority = 0;
    kbd_nvic.NVIC_IRQChannelSubPriority = 1;
    kbd_nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&kbd_nvic);
}

void Timer_Config(TIM_TypeDef *TIM, uint16_t period, uint16_t prescaler, uint16_t mode) {
	TIM_TimeBaseInitTypeDef TimerConfig;

	if (TIM == TIM1) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	} else if (TIM == TIM2) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	}

 	TimerConfig.TIM_Period = period;
	TimerConfig.TIM_Prescaler = prescaler;
	TimerConfig.TIM_CounterMode = mode;
	TimerConfig.TIM_ClockDivision = TIM_CKD_DIV1;	
	TIM_TimeBaseInit(TIM, &TimerConfig);
}

void Slave_Timer_Config(TIM_TypeDef *TIM_Slave, TIM_TypeDef *TIM_Master, uint16_t TIM_TRGOSource, uint16_t period, uint16_t mode) {
	if (TIM_Slave == TIM_Master) {
		return;
	}

	TIM_SelectOutputTrigger(TIM_Master, TIM_TRGOSource);

	if (TIM_Slave == TIM1) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
		TIM_SelectInputTrigger(TIM_Slave, TIM_TS_ITR1);
	} else if (TIM_Slave == TIM2) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
		TIM_SelectInputTrigger(TIM_Slave, TIM_TS_ITR0);
	}

	TIM_SelectSlaveMode(TIM_Slave, TIM_SlaveMode_External1);

	TIM_CounterModeConfig(TIM_Slave, mode);
	TIM_SetAutoreload(TIM_Slave, period);
}

void Timer_Interrupt(TIM_TypeDef *TIM) {
	NVIC_InitTypeDef NVIC_InitStruct;

	if (TIM == TIM1) {
		TIM_ITConfig(TIM, TIM_IT_Update, ENABLE);
		NVIC_InitStruct.NVIC_IRQChannel = TIM1_UP_IRQn;
	} else if (TIM == TIM2) {
		TIM_ITConfig(TIM, TIM_IT_Update, ENABLE);
		NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
	}

	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

void PWM_Config(TIM_TypeDef *TIM, uint8_t channel, uint16_t pulse, uint16_t mode) {
	TIM_OCInitTypeDef TIM_OCConfig={0};
	TIM_OCConfig.TIM_Pulse = pulse;
	TIM_OCConfig.TIM_OCMode = mode;
	TIM_OCConfig.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCConfig.TIM_OCPolarity = TIM_OCPolarity_High;

	switch (channel) {
		case 1:
			TIM_OC1Init(TIM, &TIM_OCConfig );
		break;
		case 2:
			TIM_OC2Init(TIM, &TIM_OCConfig );
		break;
		case 3:
			TIM_OC3Init(TIM, &TIM_OCConfig );
		break;
		case 4:
			TIM_OC4Init(TIM, &TIM_OCConfig );
		break;
	}

	TIM_CtrlPWMOutputs(TIM, ENABLE );

	switch (channel) {
		case 1:
			TIM_OC1PreloadConfig(TIM, TIM_OCPreload_Disable);
		break;
		case 2:
			TIM_OC2PreloadConfig(TIM, TIM_OCPreload_Disable);
		break;
		case 3:
			TIM_OC3PreloadConfig(TIM, TIM_OCPreload_Disable);
		break;
		case 4:
			TIM_OC4PreloadConfig(TIM, TIM_OCPreload_Disable);
		break;
	}

	TIM_ARRPreloadConfig(TIM, ENABLE);
}
