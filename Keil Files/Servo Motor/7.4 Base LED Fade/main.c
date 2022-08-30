#include "stm32f446xx.h"

/* Board name: NUCLEO-F446RE

 PA.5  <--> Green LED (LD2)
 PC.13 <--> Blue user button (B1)
 
 Base file for LED PWM
 
 Base Header Code by Dr. Sajid Muhaimin Choudhury, Department of EEE, BUET 22/06/2022
 
 Based on Instructor Companion of Yifeng Zhu
*/

#define SPEAKER_PORT GPIOA
#define SPEAKER_PIN  0


#define LED_PORT GPIOA
#define LED_PIN    15

#define BUTTON_PIN 13

#define VECT_TAB_OFFSET  0x00 /*!< Vector Table base offset field. 
                                   This value must be a multiple of 0x200. */

static void LED_Pin_Init(){
	  RCC->AHB1ENR 		|= RCC_AHB1ENR_GPIOAEN;             // Enable GPIOA clock
	
	  // Set mode as Alternative Function 1
		LED_PORT->MODER  	&= ~(0x03 << (2*LED_PIN));   			// Clear bits
		LED_PORT->MODER  	|=   0x02 << (2*LED_PIN);      		// Input(00), Output(01), AlterFunc(10), Analog(11)
	
		LED_PORT->AFR[1] 	&= ~(0xF << (4*(LED_PIN-8)));         // 	AF 1 = TIM2_CH1
		LED_PORT->AFR[1] 	|=   0x1 << (4*(LED_PIN-8));          // 	AF 1 = TIM2_CH1
	
		//Set I/O output speed value as very high speed
		LED_PORT->OSPEEDR  &= ~(0x03<<(2*LED_PIN)); 				// Speed mask
		LED_PORT->OSPEEDR  |=   0x03<<(2*LED_PIN); 					// Very high speed
		//Set I/O as no pull-up pull-down 
		LED_PORT->PUPDR    &= ~(0x03<<(2*LED_PIN));    			// No PUPD(00, reset), Pullup(01), Pulldown(10), Reserved (11)
		//Set I/O as push pull 
	  //LED_PORT->OTYPER   &=  ~(1<<LED_PIN) ;           	// Push-Pull(0, reset), Open-Drain(1)
}


static void TIM2_CH1_Init(){
		// tim uptade frequency = TIM_CLK/(TIM_PSC+1)/(TIM_ARR + 1)
	  // 4000000 / 40 / 1000 = 100Hz
		// Enable the timer clock
    RCC->APB1ENR 		|= RCC_APB1ENR_TIM2EN;                  // Enable TIMER clock

		// Counting direction: 0 = up-counting, 1 = down-counting
		TIM2->CR1 &= ~TIM_CR1_DIR;  
		
    TIM2->PSC = 79;//39;       // Prescaler = 23 
    TIM2->ARR = 1000-1;   // Auto-reload: Upcouting (0..ARR), Downcouting (ARR..0)
		TIM2->CCMR1 &= ~TIM_CCMR1_OC1M;  // Clear ouput compare mode bits for channel 1
    TIM2->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // OC1M = 110 for PWM Mode 1 output on ch1
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE;                     // Output 1 preload enable

		// Select output polarity: 0 = active high, 1 = active low
		TIM2->CCER |= TIM_CCER_CC1NP; // select active high
		
    // Enable output for ch1
		TIM2->CCER |= TIM_CCER_CC1E;                       
    
    // Main output enable (MOE): 0 = Disable, 1 = Enable
		TIM2->BDTR |= TIM_BDTR_MOE;  
		
		// zero angle = 300
		// -90 = 100
		// +90 = 500
		TIM2->CCR1  = 300;         // Output Compare Register for channel 1 
		TIM2->CR1  |= TIM_CR1_CEN; // Enable counter
}

//static int brightness = 0;

int main(void){

// Default system clock 4 MHz
	
	LED_Pin_Init();



	TIM2_CH1_Init(); // Timer to control LED
	int i;

	while(1){
	
			TIM2->CCR1 = 100; 	// set angle -90
		
			for(i=0;i<1000000;i++);  		// delay
			TIM2->CCR1 = 300; 	// set angle -0
			for(i=0;i<1000000;i++);  		// delay
		
		
			TIM2->CCR1 = 500; 	// set angle 90
			for(i=0;i<1000000;i++);  		// delay
		}
}

