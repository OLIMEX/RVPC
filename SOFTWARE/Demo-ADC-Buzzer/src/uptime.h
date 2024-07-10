#ifndef	__UPTIME_H
#define	__UPTIME_H

#include <ch32v00x.h>

#ifdef __cplusplus
extern "C" {
#endif

void Uptime_Init();

uint32_t Uptime_S();

uint32_t Uptime_Us();

uint32_t Uptime_Ms();

void Wait_Us(uint32_t delay);

void Wait_Ms(uint32_t delay);

void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick_Handler(void);

#ifdef __cplusplus
}
#endif

#endif