// "Europe Magic Wand" Hitachi clone firmware with UART control
// Compile with SDCC for STM8S003F3
// See README for programming instructions
// Furrtek - 2022
// TODO: "Kick" motor to get it started when low speed is requested

// PD6: UART1_RX
// PC3: + button
// PC4: - button
// PC5: Motor control (is it C6 ?)

// 9600bps 8N1 idle high
// Not higher because MCU runs off imprecise HSI oscillator
// Command format: AA BB CC
// AA: Byte, 0x01
// BB: Byte, motor PWM value
// CC: Byte, duration 0~254 in units of approx 8ms - if 255: infinite duration
	
#include "stm8sdefs.h"

#define MODE_NORMAL 0
#define MODE_SERIAL	1

unsigned char volatile timer_div = 0;		// Timing counter
unsigned char volatile duration_div = 0;	// Timing counter
unsigned char volatile rxed = 0;
unsigned char volatile rx_data[4];
unsigned char volatile rx_idx = 0;
unsigned char volatile rx_timeout = 0;

void TIM2_UPD_OVF_IRQHandler(void) __interrupt(13) {
	timer_div++;
	if (rx_timeout) rx_timeout--;
	TIM2_SR1 = 0;
	duration_div++;
}

void UART1_RX_IRQHandler(void) __interrupt(18) {
	rx_data[rx_idx & 3] = UART1_DR;
	rx_idx = (rx_idx + 1) & 3;
	rx_timeout = 10;			// Should be ~10ms
	rxed = 1;
	//PD_ODR ^= (1 << 5);		// Toggle PD5 on rx byte
}

void main() {
	unsigned char mode = MODE_NORMAL;		// Start in normal mode
	unsigned char new_input, active_input;
	unsigned char prev_input = 0;
	unsigned char duty_index = 0;
	unsigned char pwm_value = 0;
	unsigned char ccr_value = 0;
	unsigned char duration = 0;
	unsigned char released = 0;
	unsigned char beeping = 0;
	unsigned char beep_count = 0;
	
	// Duty cycle LUT for normal mode, starts at 50% and goes up to 100%
	const unsigned char duty_lut[6] = { 0, 0x80, 0xA0, 0xC0, 0xE0, 0xFF };
    
	CLK_CKDIVR = 1 << 3;	// fCPU = 16MHz/2 = 8MHz
	CLK_PCKENR = 0xFF;		// Power everything up

	PC_CR1 |= 0x18;			// Pull-ups for buttons
	
	// Debug
	//PD_DDR |= (1<<5);		// PD5 out (UART1_TX)
	//PD_CR1 |= (1<<5);		// PD5 out (UART1_TX)
	
	//UART1_DIV = 833;		// 8MHz/9600 TODO:Choose a better divisor/speed !
	UART1_BRR2 = 0x01;
	UART1_BRR1 = 0x34;		// 833 = 0x341
	UART1_CR1 = 0x00;		// 8N1
	UART1_CR2 = 0x24;		// RIEN, RIE	0x2C;		// RIEN, REN, TEN
	UART1_CR3 = 0x00;
	UART1_CR4 = 0x00;
	UART1_CR5 = 0x00;
	
	// TIM1 is used for PWM generation
	TIM1_PSCRL = 0;
	TIM1_ARRH = 0x01;		// Reload = 0x100
	TIM1_ARRL = 0x00;
	TIM1_CCR1H = 0x00;		// Compare = 0x00
	TIM1_CCR1L = 0x00;
	TIM1_CCER1 = 1;			// Compare mode
	TIM1_CCMR1 = 6 << 4;	// PWM1 mode
	TIM1_BKR = 1 << 7;		// Enable output
	TIM1_CR1 = 1;			// Enable timer
	
	// TIM2 is used for timing ADC and input debounce
	TIM2_PSCR = 6;			// 2^6
	TIM2_ARRH = 0;
	TIM2_ARRL = 0x80;		// 8MHz/64/128 = ~977Hz
	TIM2_IER = 1;			// Enable overflow interrupt
	TIM2_CR1 = 1;			// Enable timer
	
	__asm;					// Enable interrupts
	rim
	__endasm;

	for(;;) {
		if (!rx_timeout)
			rx_idx = 0;		// Reset rx buffer if more than 20 TIM2 interrupts since last byte received
		
		if (UART1_SR & 15) {
			// Clear any errors
			(void)UART1_SR;
			(void)UART1_DR;
			rx_idx = 0;
		}
		
		if (mode == MODE_SERIAL) {
			if (rxed) {
				rxed = 0;
				//if ((rx_idx & 3) == 3) {
					// Received a full frame
					//rx_idx = 0;
					
					if ((rx_data[0] == 0x01) && (!beeping)) {
						ccr_value = rx_data[1];
						duration = rx_data[2];
						duration_div = 0;
						
						// Avoid stalling motor with value too low
						if (ccr_value < 0x20) ccr_value = 0x20;
						
						TIM1_CR1 = 0;			// Disable timer
						TIM1_CNTRL = 0;
						TIM1_CNTRH = 0;
						TIM1_CR1 = 1;			// Enable timer
						TIM1_CCR1L = ccr_value;
					}
				//}
			}
		} else
			rx_idx = 0;	// Ignore received bytes if not in serial mode
		
		if (timer_div >= 32) {	// 977/32 = 31Hz
			timer_div = 0;
			
			if (beeping) {
				if (beeping == 1) {
					TIM1_PSCRL = 0;
					TIM1_CCR1L = 0;
					if (beep_count == 2) {
						beeping = 5;
						beep_count = 1;
					} else if (beep_count == 1) {
						TIM1_PSCRL = 20;
						TIM1_CCR1L = 30;
						beeping = 5;
						beep_count = 0;
					}
				}
				beeping--;
			}
			
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
				duty_index = 0;	// Make sure duty is zero when switching modes
				beeping = 5;
				beep_count = (mode == 1) ? 2 : 0;
				TIM1_PSCRL = 20;
				TIM1_CCR1L = 40;
			} else {
				// Select speed
				if (((active_input & 0x18) == 0x08) && (duty_index < 5)) {
					duty_index++;
				}
				else if (((active_input & 0x18) == 0x10) && (duty_index > 0))
					duty_index--;
			}
			
			if ((mode == MODE_NORMAL) && (!beeping)) {
				TIM1_CCR1L = duty_lut[duty_index];
			}
		}
		
		if (duration_div >= 8) {
			duration_div = 0;
			 if ((mode == MODE_SERIAL) && (!beeping)) {
				// Serial mode: handle duration parameter
				if (duration == 0) {
					// Turn off motor
					TIM1_CCR1L = 0;
				} else {
					if (duration < 255) duration--;
				}
			}
		}
	}
}

