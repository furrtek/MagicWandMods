// "Europe Magic Wand" Hitachi clone firmware with analog-in (CV) input
// Compile with SDCC for STM8S003F3
// See README for programming instructions
// Furrtek - 2016

// PD5: Analog input (see schematic)
// PC3: + button
// PC4: - button
// PC5: Motor control
	
#include "stm8sdefs.h"

#define MODE_NORMAL 0
#define MODE_AIN	1

unsigned char volatile timer_div = 0;		// Timing counter
unsigned char volatile adc_trig = 0;		// Conversion request flag

void TIM2_UPD_OVF_IRQHandler(void) __interrupt(13) {
	timer_div++;
	adc_trig = 1;
	TIM2_SR1 = 0;
}

main() {
	unsigned char mode = MODE_NORMAL;		// Start in normal mode
	unsigned char new_input, active_input;
	unsigned char prev_input = 0;
	unsigned char duty_index = 0;
	signed int adc_value = 0;
	unsigned int adc_mean = 0;
	unsigned char adc_count = 0;
	unsigned char pwm_value = 0;
	unsigned char ccr_value = 0;
	unsigned char released = 0;
	
	// Duty cycle LUT, starts at 50% and goes up to 100%
	const unsigned char duty_lut[6] = { 0, 0x80, 0xA0, 0xC0, 0xE0, 0xFF };
    
	CLK_CKDIVR = 1 << 3;	// fCPU = 16MHz/2 = 8MHz

	PC_CR1 |= 0x18;			// Pull-ups for buttons
	
	ADC_CR1 = 0x41;			// f/8, ADON
	ADC_CSR = 5;

	// TIM1 is used for PWM generation
	TIM1_ARRH = 0x01;		// Reload = 0x100
	TIM1_ARRL = 0x00;
	TIM1_CCR1H = 0x00;		// Compare = 0x00
	TIM1_CCR1L = 0x00;
	TIM1_CCER1 = 1;			// Compare mode
	TIM1_CCMR1 = 6 << 4;	// PWM1 mode
	TIM1_BKR = 1 << 7;		// Enable output
	TIM1_CR1 = 1;			// Enable timer
	
	// TIM2 is used for timing ADC and input debounce
	TIM2_PSCR = 6;
	TIM2_ARRH = 0;
	TIM2_ARRL = 0x80;		// 8MHz/64/128 = ~977Hz
	TIM2_IER = 1;			// Enable overflow interrupt
	TIM2_CR1 = 1;			// Enable timer
	
	__asm;					// Enable interrupts
	rim
	__endasm;

	for(;;) {
		if (adc_trig) {
			adc_trig = 0;					// Clear flag
			
			ADC_CR1 |= 1;					// Start conversion
			while(!(ADC_CSR & 0x80)) {};
			ADC_CSR &= ~0x80;
			
			adc_value = (ADC_DRH - 127);	// Cancel out DC offset
			if (adc_value < 0) adc_value = -adc_value;
			adc_mean += adc_value;
			
			if (adc_count >= 31) {
				adc_count = 0;
				
				adc_mean = adc_mean >> 1;	// /32*16 = /2
				
				if (adc_mean > 0xFF) adc_mean = 0xFF;			// Cap to 255
				
				if (adc_mean >= 0x20) {
					pwm_value = adc_mean;	// Noise threshold
				} else {
					if (pwm_value >= 0x10) pwm_value -= 0x10;	// Auto decay
				}
				
				adc_mean = 0;
			} else {
				adc_count++;
			}
		}
		
		if (timer_div >= 32) {	// 977/32 = 31Hz
			timer_div = 0;
			
			// Check for falling edge
			new_input = PC_IDR ^ 0xFF;
			active_input = (prev_input ^ new_input) & new_input;
			prev_input = new_input;
			
			// Detect both buttons released
			if ((new_input & 0x18) == 0x00) released = 1;
			
			if (((new_input & 0x18) == 0x18) && (released)) {
				// Toggle mode
				released = 0;
				mode ^= 1;
			} else {
				// Select speed
				if (((active_input & 0x18) == 0x08) && (duty_index < 5))
					duty_index++;
				else if (((active_input & 0x18) == 0x10) && (duty_index > 0))
					duty_index--;
			}
			
			if (mode == MODE_NORMAL) {
				TIM1_CCR1L = duty_lut[duty_index];
			} else {
				// Multiply analog input by selected speed
				ccr_value = ((unsigned int)pwm_value * (unsigned int)duty_lut[duty_index]) >> 8;
				
				// Avoid stalling motor with value too low
				if (ccr_value < 0x20) ccr_value = 0;
				
				TIM1_CCR1L = ccr_value;
			}
		}
	}
}
