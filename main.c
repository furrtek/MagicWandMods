// "Europe Magic Wand" Hitachi clone firmware with analog-in (CV) input
// Compile with SDCC for STM8S003F3
// See README for programming instructions
// Furrtek - 2016

// PC3: + button
// PC4: - button
// PC5: Motor control
	
#include "stm8sdefs.h"

unsigned char volatile update = 0;

void TIM2_UPD_OVF_IRQHandler(void) __interrupt(13) {
	update = 1;
	TIM2_SR1 = 0;
}

main() {
	unsigned char new_input, active_input;
	unsigned char prev_input = 0;
	unsigned char duty_index = 0;
	
	// Duty cycle LUT, starts at 50% and goes up to 100%
	const unsigned char duty_lut[6] = { 0, 0x80, 0xA0, 0xC0, 0xE0, 0xFF };
    
	CLK_CKDIVR = 1 << 3;	// fCPU = 16MHz/2 = 8MHz

	PC_CR1 |= 0x18;			// Pull-ups for buttons

	// TIM1 is used for PWM generation
	TIM1_ARRH = 0x01;		// Reload = 0x100
	TIM1_ARRL = 0x00;
	TIM1_CCR1H = 0x00;		// Compare = 0x00
	TIM1_CCR1L = 0x00;
	TIM1_CCER1 = 1;			// Compare mode
	TIM1_CCMR1 = 6 << 4;	// PWM1 mode
	TIM1_BKR = 1 << 7;		// Enable output
	TIM1_CR1 = 1;			// Enable timer
	
	// TIM2 is used for input debounce
	TIM2_PSCR = 10;
	TIM2_ARRH = 0;
	TIM2_ARRL = 0x80;		// 8MHz/1024/128 = ~61Hz
	TIM2_IER = 1;			// Enable overflow interrupt
	TIM2_CR1 = 1;			// Enable timer
	
	__asm;					// Enable interrupts
	rim
	__endasm;

	for(;;) {
		if (update) {
			update = 0;
			
			// Check for falling edge
			new_input = PC_IDR ^ 0xFF;
			active_input = (prev_input ^ new_input) & new_input;
			prev_input = new_input;
			
			if ((active_input & 0x08) && (duty_index < 5)) duty_index++;
			if ((active_input & 0x10) && (duty_index > 0)) duty_index--;
			
			TIM1_CCR1L = duty_lut[duty_index];
		}
	}
}
