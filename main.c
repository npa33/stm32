#include "setup.h"
#include "I2C.h"
#include "MPU6050.h"
#include "LCD_I2C.h"
#include <stdio.h>

extern uint32_t volatile State;
extern uint32_t volatile Tick;

LCD_Name LCD1;

int main(void) {
	IO_Init();
	Timer_Init();
	I2C_Init();
	MPU6050_Init();
	LCD_Init(&LCD1,SLAVE_ADDRESS_LCD,16,2);
	
	LCD_Clear(&LCD1);
	LCD_Cursor(&LCD1,1,0);
	LCD_WriteString(&LCD1, "Step:");	
	
	LCD_Cursor(&LCD1,6,1);
	LCD_WriteString(&LCD1, "0");
	
	while(1) {		
		if(State % 2 == 0) {
			TIM2->CR1 |= TIM_CR1_CEN;  									//Dem buoc chan
			
			GPIOA->ODR &= ~(1u<<6); 										//Tat led do
			
			if(Tick >= 1000) {
				GPIO_TogglePIN(GPIOC, 13); 								//Nhay led 13 o tan so 1Hz
				Tick = 0;
			}
		}
		
		else {
			TIM2->CR1 &= ~TIM_CR1_CEN; 									//Dung he thong
			GPIOC->ODR |= (1u<<13); 										//Tat led xanh
			GPIOA->ODR |= (1u<<6);  										//Bat led do
		}
	}
}
