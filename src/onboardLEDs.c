#include <stdint.h>
#include "tm4c1294ncpdt.h"
#include "SysTick_Emily.h"
#include "onboardLEDs.h"

#define DELAY 1

//Flash D1
void FlashLED1(int count) {
	while(count--) {
		GPIO_PORTN_DATA_R ^= 0b00000010; 							
		SysTick_Wait10ms(DELAY);												
		GPIO_PORTN_DATA_R ^= 0b00000010;			
		SysTick_Wait10ms(DELAY);											
	}
}

//Flash D2
void FlashLED2(int count) {
	while(count--) {
		GPIO_PORTN_DATA_R ^= 0b00000001; 					
		SysTick_Wait10ms(DELAY);										
		GPIO_PORTN_DATA_R ^= 0b00000001;			
		SysTick_Wait10ms(DELAY);													
	}
}

//Flash D3
void FlashLED3(int count) {
	while(count--) {
		GPIO_PORTF_DATA_R ^= 0b00010000; 				
		SysTick_Wait10ms(DELAY);											
		GPIO_PORTF_DATA_R ^= 0b00010000;			
		SysTick_Wait10ms(DELAY);													
	}
}

//Flash D4
void FlashLED4(int count) {
	while(count--) {
		GPIO_PORTF_DATA_R ^= 0b00000001; 						
		SysTick_Wait10ms(DELAY);											
		GPIO_PORTF_DATA_R ^= 0b00000001;			
		SysTick_Wait10ms(DELAY);											
	}
}

// Flash all LEDs
void FlashAllLEDs(){
	GPIO_PORTN_DATA_R ^= 0b00000011; 								
	GPIO_PORTF_DATA_R ^= 0b00010001; 							
	SysTick_Wait10ms(25);												
	GPIO_PORTN_DATA_R ^= 0b00000011;			
	GPIO_PORTF_DATA_R ^= 0b00010001; 						
	SysTick_Wait10ms(25);														
}


// Status LEDs
void Flash_UART_Tx(){
	FlashLED3(1); // PF4
}

void Flash_Measurement_Status(){
	FlashLED4(1); // PF0 
}

void MovingLED(int on){
	if(on)   GPIO_PORTN_DATA_R |= 0b00000010; // PN1
	else	 GPIO_PORTN_DATA_R &= ~0b00000010;

}

// Initialize onboard LEDs
void onboardLEDs_Init(void){
	// Port N LEDs
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R12;				
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R12) == 0){};	
	GPIO_PORTN_DIR_R |= 0x03;        						
	GPIO_PORTN_AFSEL_R &= ~0x03;     								
	GPIO_PORTN_DEN_R |= 0x03;        								
	GPIO_PORTN_AMSEL_R &= ~0x03;  

	// Port F LEDs   								
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;				
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R5) == 0){};	
	GPIO_PORTF_DIR_R |= 0x11;        						
	GPIO_PORTF_AFSEL_R &= ~0x11;     							
	GPIO_PORTF_DEN_R |= 0x11;        								
	GPIO_PORTF_AMSEL_R &= ~0x011;     			
			
	FlashAllLEDs();
	return;
}




