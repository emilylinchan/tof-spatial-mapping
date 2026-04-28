#include <stdint.h>
#include "tm4c1294ncpdt.h"
#include "VL53L1X_api.h"
#include "PLL_Emily.h"
#include "SysTick_Emily.h"
#include "I2C0.h" 
#include "uart.h"
#include "onboardLEDs.h"
#include "motor_control.h"

#define REVOLUTION 2048 	// # motor steps per revolution
#define MEASUREMENTS 64     // # distance measurements taken per yz slice scan 
#define NUM_SCANS	20	    // # scans along x-axis

uint16_t dev = 0x29;	
int status = 0; 		
int start = 0;								
uint16_t data_array[NUM_SCANS]; 

// Enable interrupts
void EnableInt(void)
{    __asm("    cpsie   i\n");
}

// Disable interrupts
void DisableInt(void)
{    __asm("    cpsid   i\n");
}

// Low power wait
void WaitForInt(void)
{    __asm("    wfi\n");
}

// Activate clock for Port J and initalize PJ0 as Digital Input GPIO
void PortJ_Init(void){
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R8;					
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R8) == 0){};	
    GPIO_PORTJ_DIR_R &= ~0x01;    										
    GPIO_PORTJ_DEN_R |= 0x01;     										
	GPIO_PORTJ_PCTL_R &= ~0x0000000F;	 								
	GPIO_PORTJ_AMSEL_R &= ~0x01;										
	GPIO_PORTJ_PUR_R |= 0x01;		
}
// Interrupt initialization for GPIO Port J IRQ# 51
void PortJ_Interrupt_Init(void){
		GPIO_PORTJ_IS_R = 0;  	    // Edge sensitive
		GPIO_PORTJ_IBE_R = 0;    	// Not triggered by both edges 
		GPIO_PORTJ_IEV_R = 0;  	    // Falling edge event 
		GPIO_PORTJ_ICR_R = 0x01;    // Clear interrupt flag
		GPIO_PORTJ_IM_R = 0x01;     // Arm interrupt on PJ0 
		NVIC_EN1_R = 0x00080000;    // Enable interrupt 51 in NVIC 
		NVIC_PRI12_R = 0xA0000000;  // Set interrupt priority to 5
		EnableInt();				// Enable Global Interrupt
}

// IRQ Handler - Check if button J0 was pressed to start system
void GPIOJ_IRQHandler(void){
	GPIO_PORTJ_ICR_R = 0x01;  
	start = 1; 							
}

// Reset VL53L1X ToF sensor using XSHUT
void PortG_Init(void){
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R6;             
    while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R6) == 0){};    
    GPIO_PORTG_DIR_R &= 0x00;                           
    GPIO_PORTG_AFSEL_R &= ~0x01;                        
    GPIO_PORTG_DEN_R |= 0x01;                          
    GPIO_PORTG_AMSEL_R &= ~0x01;                
}

// XSHUT pin is an active-low shutdown input
void VL53L1X_XSHUT(void){
    GPIO_PORTG_DIR_R |= 0x01;                       
    GPIO_PORTG_DATA_R &= 0b11111110;                     
    FlashAllLEDs();
    SysTick_Wait10ms(10);
    GPIO_PORTG_DIR_R &= ~0x01;          
}


int main(void) {
	// ToF sensor variables
	uint8_t sensorState = 0;
    uint16_t wordData;
    uint16_t Distance;
	uint8_t RangeStatus;
    uint8_t dataReady;
	
	// Initialization routines
    PLL_Init();           		
	SysTick_Init();	
	PortJ_Init();						
	PortJ_Interrupt_Init();		
	I2C_Init();							
	UART_Init();						
	motorsInit();	
	onboardLEDs_Init();
	SysTick_Wait10ms(100); 
	FlashAllLEDs(); 			
	
	// Validate UART interface
	UART_printf("Program Begins\r\n");
	Flash_UART_Tx();

	// Validate I2C interface
	status = VL53L1X_GetSensorId(dev, &wordData);
	Status_Check("I2C Init Sucessful!\r\n", status);

	// Wait for device ToF booted
	while(sensorState == 0){
		status = VL53L1X_BootState(dev, &sensorState);
		SysTick_Wait10ms(10);
    }	
	FlashAllLEDs();
	UART_printf("ToF Chip Booted!\r\n Please Wait...\r\n");
	Flash_UART_Tx();
	
	status = VL53L1X_ClearInterrupt(dev);
	
    // Initialize the sensor with the default ranging settings (Long 4m, TB = 100ms)
    status = VL53L1X_SensorInit(dev);
	Status_Check("Sensor Init Successful!\r\n", status);
	Flash_UART_Tx();

	while(1){	
		// System start triggered by pressing onboard button J0
		if(start){
			status = VL53L1X_StartRanging(dev); 

			// Take specified number of scans along x-axis
			for(int scan = 1; scan <= NUM_SCANS; scan++){
				UART_printf("START\n");
				Flash_UART_Tx();
				
				// Take specified number of distance measurements per yz slice
				for(int i = 0; i < MEASUREMENTS; i++){ 		
					
					// Step motor corresponding number of times between measurements
					for(int j = 0; j < (REVOLUTION / MEASUREMENTS); j++){
						stepMotorSensor(0);
					}
					clearMotorSensor(); // Stop motor to take measurement
				
					// Take 1 distance measurement
					while (dataReady == 0){ 					
						status = VL53L1X_CheckForDataReady(dev, &dataReady);
						VL53L1_WaitMs(dev, 5);
					}
					dataReady = 0;
					
					// Read the data values from ToF sensor
					status = VL53L1X_GetRangeStatus(dev, &RangeStatus);
					status = VL53L1X_GetDistance(dev, &Distance);		
					if(status == 0) Flash_Measurement_Status();
					status = VL53L1X_ClearInterrupt(dev);
					
					// Send the data over UART
					sprintf(printf_buffer,"%u, %u\r\n", RangeStatus, Distance);
					UART_printf(printf_buffer);
					Flash_UART_Tx();
					SysTick_Wait10ms(20);		
				}
				
				// Untangle wires by doing 1 full rotation in reverse direction
				for(int i = 0; i < REVOLUTION; i++){
					stepMotorSensor(1); 
				}
				clearMotorSensor();
				
				// Move vehicle forward for next scan
				if(scan < NUM_SCANS){
					MovingLED(1);
					for(int i = 0; i < 2*REVOLUTION; i++) { 
						stepMotorWheel(1);
					}
					clearMotorWheel();
					MovingLED(0);
				}
			}
			UART_printf("END\n");
			VL53L1X_StopRanging(dev);
			start = 0;						
		}
		WaitForInt(); 
	}
}
