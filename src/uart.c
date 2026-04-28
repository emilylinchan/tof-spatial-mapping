#include "uart.h"
#include "tm4c1294ncpdt.h"
#include <stdint.h>
#include <stdio.h>

// Initialize UART0, based on textbook. Clock code modified.
void UART_Init(void) {
	SYSCTL_RCGCUART_R |= 0x0001; 
	SYSCTL_RCGCGPIO_R |= 0x0001; 

	while((SYSCTL_PRUART_R&SYSCTL_PRUART_R0) == 0){};
		
 	UART0_CTL_R &= ~UART_CTL_UARTEN;    
 	UART0_IBRD_R = 8;                    
 	UART0_FBRD_R = 44;                   
                                      
 	UART0_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
                                   
 	UART0_CC_R = (UART0_CC_R&~UART_CC_CS_M)+UART_CC_CS_PIOSC;
                                     
 	SYSCTL_ALTCLKCFG_R = (SYSCTL_ALTCLKCFG_R&~SYSCTL_ALTCLKCFG_ALTCLK_M)+SYSCTL_ALTCLKCFG_ALTCLK_PIOSC;
  	UART0_CTL_R &= ~UART_CTL_HSE;      

	UART0_LCRH_R = 0x0070;		
	UART0_CTL_R = 0x0301;		
	GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFFFFFF00)+0x00000011; 
	GPIO_PORTA_AMSEL_R &= ~0x03;	
	GPIO_PORTA_AFSEL_R |= 0x03;		
	GPIO_PORTA_DEN_R |= 0x03;			
}

// Wait for new input, then return ASCII code 
	char UART_InChar(void){
		while((UART0_FR_R&0x0010) != 0);	
		return((char)(UART0_DR_R&0xFF));
	} 
	
	// Wait for buffer to be not full, then output 
	void UART_OutChar(char data){
		while((UART0_FR_R&0x0020) != 0);	
		UART0_DR_R = data;
	} 
	void UART_printf(const char* array){
		int ptr=0;
		while(array[ptr]){
			UART_OutChar(array[ptr]);
			ptr++;
		}
	}
	
	void Status_Check(char* array, int status){
			if (status != 0){
				UART_printf(array);
				sprintf(printf_buffer," failed with (%d)\r\n",status);
				UART_printf(printf_buffer);
			}else
			{
				UART_printf(array);
				UART_printf(" Successful.\r\n");
			}
	}

	