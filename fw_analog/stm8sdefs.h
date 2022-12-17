#define PC_IDR		(*(volatile unsigned char *)0x500B)
#define PC_CR1		(*(volatile unsigned char *)0x500D)

#define CLK_CKDIVR	(*(volatile unsigned char *)0x50C6)

#define TIM1_CR1	(*(volatile unsigned char *)0x5250)
#define TIM1_CCMR1	(*(volatile unsigned char *)0x5258)
#define TIM1_CCER1	(*(volatile unsigned char *)0x525C)
#define TIM1_ARRH	(*(volatile unsigned char *)0x5262)
#define TIM1_ARRL	(*(volatile unsigned char *)0x5263)
#define TIM1_CCR1H	(*(volatile unsigned char *)0x5265)
#define TIM1_CCR1L	(*(volatile unsigned char *)0x5266)
#define TIM1_BKR	(*(volatile unsigned char *)0x526D)

#define TIM2_CR1	(*(volatile unsigned char *)0x5300)
#define TIM2_IER	(*(volatile unsigned char *)0x5303)
#define TIM2_SR1	(*(volatile unsigned char *)0x5304)
#define TIM2_PSCR	(*(volatile unsigned char *)0x530E)
#define TIM2_ARRH	(*(volatile unsigned char *)0x530F)
#define TIM2_ARRL	(*(volatile unsigned char *)0x5310)

#define ADC_CSR		(*(volatile unsigned char *)0x5400)
#define ADC_CR1		(*(volatile unsigned char *)0x5401)
#define ADC_DRH		(*(volatile unsigned char *)0x5404)
