#include <ch32v00x.h>

#include "uptime.h"

void NMI_Handler(void)       __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

#define BUZZER_PERIPHERY   RCC_APB2Periph_GPIOC
#define BUZZER_PORT        GPIOC
#define BUZZER_PIN         GPIO_Pin_4
#define BUZZER_DELAY_US    5000

#define ADC_PERIPHERY      RCC_APB2Periph_GPIOA
#define ADC_PORT           GPIOA
#define ADC_PIN            GPIO_Pin_2
#define ADC_CHANNEL        ADC_Channel_0

/**********************************************************/
/*                          ADC                           */
/**********************************************************/

typedef enum {
	ADC_SUCCESS = 0,
	ADC_ERROR = 1
} ADC_Error_Type;

#define ADC_TIMEOUT_MS    100

static uint8_t ADC_Ready = 0;

ADC_Error_Type ADC_Calibrate(uint32_t timeoutMS) {
	ADC_Ready = 0;

	ADC_ResetCalibration(ADC1);

	uint32_t time = Uptime_Ms();
	while(ADC_GetResetCalibrationStatus(ADC1)) {
		if ((uint32_t)(Uptime_Ms() - time) >= timeoutMS) {
			// timeout
			return ADC_ERROR;
		}
	}

	ADC_StartCalibration(ADC1);

	time = Uptime_Ms();
	while(ADC_GetCalibrationStatus(ADC1)){
		if ((uint32_t)(Uptime_Ms() - time) >= timeoutMS) {
			// timeout
			return ADC_ERROR;
		}
	}

	ADC_Ready = 1;
	return ADC_SUCCESS;
}

ADC_Error_Type ADC_Channel_Init(GPIO_TypeDef *GPIO_port, uint16_t GPIO_pin, uint32_t timeoutMS) {
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	ADC_InitTypeDef ADC_InitStructure = {0};

	if (GPIO_port == GPIOA) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	} else if (GPIO_port == GPIOC) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	} else if (GPIO_port == GPIOD) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	}

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);

	GPIO_InitStructure.GPIO_Pin = GPIO_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_port, &GPIO_InitStructure);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_Calibration_Vol(ADC1, ADC_CALVOL_50PERCENT);
	ADC_Cmd(ADC1, ENABLE);

	return ADC_Calibrate(timeoutMS);
}

ADC_Error_Type Get_ADC_Val(uint8_t ADC_Channel, uint32_t timeoutMS, uint16_t *ADC_Value) {
	ADC_Calibrate(timeoutMS);
	if (!ADC_Ready) {
		return ADC_ERROR;
	}

	ADC_RegularChannelConfig(ADC1, ADC_Channel, 1, ADC_SampleTime_241Cycles);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);

	uint32_t time = Uptime_Ms();
	while(!ADC_GetFlagStatus( ADC1, ADC_FLAG_EOC )) {
		if ((uint32_t)(Uptime_Ms() - time) >= timeoutMS) {
			//timeout
			return ADC_ERROR;
		}
	}

	*ADC_Value = ADC_GetConversionValue(ADC1);

	return ADC_SUCCESS;
}

/************************** ADC ***************************/

#define ADC_AVERAGE_COUNT 10

#define DELAY_MIN         100
#define DELAY_LIMIT       105

void calcDelay(uint32_t *delay) {
	static uint16_t adc = 0;
	static uint32_t adc_sum = 0;
	static uint32_t adc_cnt = 0;
	
	if (Get_ADC_Val(ADC_CHANNEL, ADC_TIMEOUT_MS, &adc) == ADC_SUCCESS) {
		adc_sum += adc;
		adc_cnt++;
		if (adc_cnt == ADC_AVERAGE_COUNT) {
			*delay = adc_sum / adc_cnt  + DELAY_MIN;
			adc_sum = 0;
			adc_cnt = 0;
		}
	}
}

int main(void) {
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	SystemCoreClockUpdate();

	// Disable GPIO Alternate Functions
	// Othewize GPIO PORT A does not work
	RCC_HSEConfig(RCC_HSE_OFF);
	GPIO_PinRemapConfig(GPIO_Remap_PA1_2, DISABLE);

	Uptime_Init();

	RCC_APB2PeriphClockCmd(BUZZER_PERIPHERY, ENABLE);
	GPIO_InitTypeDef cfg = {0};
    cfg.GPIO_Mode = GPIO_Mode_Out_PP;
	cfg.GPIO_Pin = BUZZER_PIN;
	cfg.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BUZZER_PORT, &cfg);

	ADC_Channel_Init(ADC_PORT, ADC_PIN, ADC_TIMEOUT_MS);

	uint32_t delay = DELAY_MIN;

	while (1) {
		if (delay > DELAY_LIMIT) {
    		GPIO_WriteBit(BUZZER_PORT, BUZZER_PIN, 1);
		}
		calcDelay(&delay);
        Wait_Us(delay);
		
		if (delay > DELAY_LIMIT) {
			GPIO_WriteBit(BUZZER_PORT, BUZZER_PIN, 0);
		}
		calcDelay(&delay);
        Wait_Us(delay);
	}

}

void NMI_Handler(void) {

}

void HardFault_Handler(void) {
	while (1) {
		
	}
}
