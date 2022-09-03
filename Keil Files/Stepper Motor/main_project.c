
#include "stm32f446xx.h"
/* Board name: NUCLEO-F446RE
 PA.5  <--> Green STEPPER_PIN_ (LD2)
 PC.13 <--> Blue user button (B1)
 
 Base Header Code by Dr. Sajid Muhaimin Choudhury, Department of EEE, BUET 22/06/2022
 
 Based on Instructor Companion of Yifeng Zhu
*/
#define STEPPER_PIN_1    4
#define STEPPER_PIN_2    5
#define STEPPER_PIN_3    3
#define STEPPER_PIN_4    2
#define STEPPER_PORT GPIOB

#define BUTTON_PIN 1
#define BUTTON_PORT GPIOC

#define VECT_TAB_OFFSET  0x00 /*!< Vector Table base offset field. 
                                   This value must be a multiple of 0x200. */
/*
 User HSI (high-speed internal) as the processor clock
 See Page 94 on Reference Manual to see the clock tree
 HSI Clock: 16 Mhz, 1% accuracy at 25 oC
 Max Freq of AHB: 84 MHz
 Max Freq of APB2: 84 MHZ
 Max Freq of APB1: 42 MHZ
 SysTick Clock = AHB Clock / 8
*/
static void enable_HSI(){
	
	/* Enable Power Control clock */
	/* RCC->APB1ENR |= RCC_APB1LPENR_PWRLPEN; */
	
	// Regulator voltage scaling output selection: Scale 2 
	// PWR->CR |= PWR_CR_VOS_1;
	
	// Enable High Speed Internal Clock (HSI = 16 MHz)
	RCC->CR |= ((uint32_t)RCC_CR_HSION);
	while ((RCC->CR & RCC_CR_HSIRDY) == 0); // Wait until HSI ready
	
	// Store calibration value
	PWR->CR |= (uint32_t)(16 << 3);
	
	// Reset CFGR register 
	RCC->CFGR = 0x00000000;
 	// Reset HSEON, CSSON and PLLON bits 
 	RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_CSSON | RCC_CR_PLLON);
	while ((RCC->CR & RCC_CR_PLLRDY) != 0); // Wait until PLL disabSTEPPER_PIN_
	
	// Programming PLLCFGR register 
	// RCC->PLLCFGR = 0x24003010; // This is the default value
	// Tip: 
	// Recommended to set VOC Input f(PLL clock input) / PLLM to 1-2MHz
	// Set VCO output between 192 and 432 MHz, 
	// f(VCO clock) = f(PLL clock input) × (PLLN / PLLM)
	// f(PLL general clock output) = f(VCO clock) / PLLP
	// f(USB OTG FS, SDIO, RNG clock output) = f(VCO clock) / PLLQ
 	
	RCC->PLLCFGR = 0;
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC); 		// PLLSRC = 0 (HSI 16 Mhz clock selected as clock source)
	RCC->PLLCFGR |= 16 << RCC_PLLCFGR_PLLN_Pos; 	// PLLM = 16, VCO input clock = 16 MHz / PLLM = 1 MHz
	RCC->PLLCFGR |= 336 << RCC_PLLCFGR_PLLN_Pos; 	// PLLN = 336, VCO output clock = 1 MHz * 336 = 336 MHz
	RCC->PLLCFGR |= 4 << RCC_PLLCFGR_PLLP_Pos; 	// PLLP = 4, PLLCLK = 336 Mhz / PLLP = 84 MHz
	RCC->PLLCFGR |= 7 << RCC_PLLCFGR_PLLQ_Pos; 	// PLLQ = 7, USB Clock = 336 MHz / PLLQ = 48 MHz
	// Enable Main PLL Clock
	RCC->CR |= RCC_CR_PLLON; 
	while ((RCC->CR & RCC_CR_PLLRDY) == 0);  // Wait until PLL ready
	
	
	// FLASH configuration block
	// enable instruction cache, enable prefetch, set latency to 2WS (3 CPU cycles)
	FLASH->ACR |= FLASH_ACR_ICEN | FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY_2WS;
	// Configure the HCLK, PCLK1 and PCLK2 clocks dividers
	// AHB clock division factor
	RCC->CFGR &= ~RCC_CFGR_HPRE; // 84 MHz, not divided
	// PPRE1: APB Low speed prescaler (APB1)
	RCC->CFGR &= ~RCC_CFGR_PPRE1; 
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; // 42 MHz, divided by 2
	// PPRE2: APB high-speed prescaler (APB2)
	RCC->CFGR &= ~RCC_CFGR_PPRE2; // 84 MHz, not divided
	
	// Select PLL as system clock source 
	// 00: HSI oscillator selected as system clock
	// 01: HSE oscillator selected as system clock
	// 10: PLL selected as system clock
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_1;
	// while ((RCC->CFGR & RCC_CFGR_SWS_PLL) != RCC_CFGR_SWS_PLL);
  	// Configure the Vector Table location add offset address 
//	VECT_TAB_OFFSET  = 0x00UL; // Vector Table base offset field. 
                                   // This value must be a multiple of 0x200. 
  	SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; // Vector Table Relocation in Internal FLASH 
}


static void configure_STEPPER_PIN__pin(){
  // Enable the clock to GPIO Port B	
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;   
		
	// GPIO Mode: Input(00), Output(01), AlterFunc(10), Analog(11, reset)
	STEPPER_PORT->MODER &= ~(3UL<<(2*STEPPER_PIN_1));  
	STEPPER_PORT->MODER |=   1UL<<(2*STEPPER_PIN_1);      // Output(01)
	STEPPER_PORT->MODER &= ~(3UL<<(2*STEPPER_PIN_2));  
	STEPPER_PORT->MODER |=   1UL<<(2*STEPPER_PIN_2);      // Output(01)
	STEPPER_PORT->MODER &= ~(3UL<<(2*STEPPER_PIN_3));  
	STEPPER_PORT->MODER |=   1UL<<(2*STEPPER_PIN_3);      // Output(01)
	STEPPER_PORT->MODER &= ~(3UL<<(2*STEPPER_PIN_4));  
	STEPPER_PORT->MODER |=   1UL<<(2*STEPPER_PIN_4);      // Output(01)
	
	// GPIO Speed: Low speed (00), Medium speed (01), Fast speed (10), High speed (11)
	STEPPER_PORT->OSPEEDR &= ~(3U<<(2*STEPPER_PIN_1));
	STEPPER_PORT->OSPEEDR |=   2U<<(2*STEPPER_PIN_1);  // Fast speed
	STEPPER_PORT->OSPEEDR &= ~(3U<<(2*STEPPER_PIN_2));
	STEPPER_PORT->OSPEEDR |=   2U<<(2*STEPPER_PIN_2);  // Fast speed
	STEPPER_PORT->OSPEEDR &= ~(3U<<(2*STEPPER_PIN_3));
	STEPPER_PORT->OSPEEDR |=   2U<<(2*STEPPER_PIN_3);  // Fast speed
	STEPPER_PORT->OSPEEDR &= ~(3U<<(2*STEPPER_PIN_4));
	STEPPER_PORT->OSPEEDR |=   2U<<(2*STEPPER_PIN_4);  // Fast speed
	
	// GPIO Output Type: Output push-pull (0, reset), Output open drain (1) 
	STEPPER_PORT->OTYPER &= ~(1U<<STEPPER_PIN_1);      // Push-pull
	STEPPER_PORT->OTYPER &= ~(1U<<STEPPER_PIN_2);      // Push-pull
	STEPPER_PORT->OTYPER &= ~(1U<<STEPPER_PIN_3);      // Push-pull
	STEPPER_PORT->OTYPER &= ~(1U<<STEPPER_PIN_4);      // Push-pull
	
	// GPIO Push-Pull: No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)
	STEPPER_PORT->PUPDR  &= ~(3U<<(2*STEPPER_PIN_1));  // No pull-up, no pull-down
	STEPPER_PORT->PUPDR  &= ~(3U<<(2*STEPPER_PIN_2));  // No pull-up, no pull-down
	STEPPER_PORT->PUPDR  &= ~(3U<<(2*STEPPER_PIN_3));  // No pull-up, no pull-down
	STEPPER_PORT->PUPDR  &= ~(3U<<(2*STEPPER_PIN_4));  // No pull-up, no pull-down
	
}
static void configure_PUSH_button(){
  // Enable the clock to GPIO Port A	
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;   
		
	// GPIO Mode: Input(00), Output(01), AlterFunc(10), Analog(11, reset)
	BUTTON_PORT->MODER &= ~(3UL<<(2*BUTTON_PIN));  
	//BUTTON_PORT->MODER |=   1UL<<(2*BUTTON_PIN);      // Output(01)
	
	// GPIO Speed: Low speed (00), Medium speed (01), Fast speed (10), High speed (11)
	BUTTON_PORT->OSPEEDR &= ~(3U<<(2*BUTTON_PIN));
	BUTTON_PORT->OSPEEDR |=   2U<<(2*BUTTON_PIN);  // Fast speed
	
	// GPIO Output Type: Output push-pull (0, reset), Output open drain (1) 
	BUTTON_PORT->OTYPER &= ~(1U<<BUTTON_PIN);      // Push-pull
	
	// GPIO Push-Pull: No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)
	BUTTON_PORT->PUPDR  &= ~(3U<<(2*BUTTON_PIN));  // No pull-up, no pull-down
	BUTTON_PORT->PUPDR  |= 1U<<(2*BUTTON_PIN);
	
}
static void turn_on_STEPPER_PIN_(){
	STEPPER_PORT->ODR |= 1U << STEPPER_PIN_1;
}
static void turn_off_STEPPER_PIN_(){
	STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_1);
}
static void toggle_STEPPER_PIN_(){
	STEPPER_PORT->ODR ^= (1 << STEPPER_PIN_1);
}
int main(void){
	uint32_t i;
	
	enable_HSI();
	configure_STEPPER_PIN__pin();
	configure_PUSH_button();

	
	int delay = 10000;
	while(1){
		STEPPER_PORT->ODR |= 1U << STEPPER_PIN_1;
		STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_2);
		STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_3);
		STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_4);
		for(i=0; i<delay; i++);
		
		STEPPER_PORT->ODR |= 1U << STEPPER_PIN_2;
		STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_1);
		STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_3);
		STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_4);
		for(i=0; i<delay; i++);
		
		STEPPER_PORT->ODR |= 1U << STEPPER_PIN_3;
		STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_1);
		STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_2);
		STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_4);
		for(i=0; i<delay; i++);
		
		STEPPER_PORT->ODR |= 1U << STEPPER_PIN_4;
		STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_1);
		STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_2);
		STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_3);
		for(i=0; i<delay; i++);
		
		while(GPIOC->IDR & GPIO_IDR_IDR_1){
			STEPPER_PORT->ODR |= 1U << STEPPER_PIN_1;
			STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_2);
			STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_3);
			STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_4);
			for(i=0; i<delay; i++);
			
			STEPPER_PORT->ODR |= 1U << STEPPER_PIN_4;
			STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_1);
			STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_2);
			STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_3);
			for(i=0; i<delay; i++);
			
			STEPPER_PORT->ODR |= 1U << STEPPER_PIN_3;
			STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_1);
			STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_2);
			STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_4);
			for(i=0; i<delay; i++);
			
			STEPPER_PORT->ODR |= 1U << STEPPER_PIN_2;
			STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_1);
			STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_3);
			STEPPER_PORT->ODR &= ~(1U << STEPPER_PIN_4);
			for(i=0; i<delay; i++);
	}
	}
}