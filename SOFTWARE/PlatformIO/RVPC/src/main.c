#include <ch32v00x.h>
#include <debug.h>

void NMI_Handler(void)       __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

#define BUZZER_DELAY_MS 1

int main(void) {
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	SystemCoreClockUpdate();
    
    Delay_Init();

	// GPIO configuration
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef cfg = {0};
    cfg.GPIO_Mode = GPIO_Mode_Out_PP;
   	cfg.GPIO_Pin = GPIO_Pin_4;
	cfg.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOC, &cfg);

	while (1) {
		GPIO_WriteBit(GPIOC, GPIO_Pin_4, 1);
		Delay_Ms(BUZZER_DELAY_MS);
		GPIO_WriteBit(GPIOC, GPIO_Pin_4, 0);
		Delay_Ms(BUZZER_DELAY_MS);
	}

}

void NMI_Handler(void) {
	
}

void HardFault_Handler(void) {
	
	while (1) {
		
	}
}
