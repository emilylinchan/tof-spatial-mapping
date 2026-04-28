#include "tm4c1294ncpdt.h"
#include "SysTick_Emily.h"
#include "onboardLEDs.h"
#include "motor_control.h"

#define SPEED 300 // Motor speed control *10us (time between steps)

volatile uint8_t runMotorWheel = 0;
volatile uint8_t runMotorSensor = 0;

volatile int totalStepsWheel = 0;
volatile int totalStepsSensor = 0;


// Initialize both motor ports
void motorsInit(){
	// Use PortH for the wheel
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R7;              
	while((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R7) == 0){};   
	GPIO_PORTH_DIR_R |= 0b00001111;       						
	GPIO_PORTH_DEN_R |= 0b00001111;											
	
	// Use PortL for the sensor
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R10;              
	while((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R10) == 0){};   
	GPIO_PORTL_DIR_R |= 0b00001111;       						
	GPIO_PORTL_DEN_R |= 0b00001111;								
		
	// Motor flags both initially off
	runMotorWheel = 0;
	runMotorSensor = 0;
		
	return;
}

// Stepper motor coil energization patterns (full-step mode) for CW direction
uint16_t coils[4] = {0b0011,  // Blue+Purple
					 0b0110,  // Purple+Yellow	
					 0b1100,  // Yellow+Orange
					 0b1001}; // Orange+Blue		

uint8_t coilIdxWheel = 0;		
uint8_t coilIdxSensor = 0;											 
										 
// Execute 1 step for wheel motor				 
void stepMotorWheel(int dir) {
	if(dir == 0){
		coilIdxWheel = (coilIdxWheel + 1) % 4;
		GPIO_PORTH_DATA_R = coils[coilIdxWheel]; 
	}
	else {
		coilIdxWheel = (coilIdxWheel - 1 + 4) % 4;
		GPIO_PORTH_DATA_R = coils[coilIdxWheel]; 
	}
	totalStepsWheel++;
	SysTick_Wait10us(SPEED);
}				
										 
// Execute 1 step for sensor motor						 
void stepMotorSensor(int dir) {
	if(dir == 0){
		coilIdxWheel = (coilIdxWheel + 1) % 4;
		GPIO_PORTL_DATA_R = coils[coilIdxWheel]; 
	}
	else {
		coilIdxWheel = (coilIdxWheel - 1 + 4) % 4;
		GPIO_PORTL_DATA_R = coils[coilIdxWheel]; 
	}
	totalStepsSensor++;
	SysTick_Wait10us(SPEED); 
}

void clearMotorWheel() {
	// Clear the coils to turn off the driver LEDs and save power
    GPIO_PORTH_DATA_R &= ~0b1111;
}

void clearMotorSensor() {
	// Clear the coils to turn off the driver LEDs and save power
    GPIO_PORTL_DATA_R &= ~0b1111;
}