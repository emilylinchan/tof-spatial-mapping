#include <stdint.h>
#include "tm4c1294ncpdt.h"
#include "SysTick_Emily.h"		

#define I2C_MCS_ACK             0x00000008  // Data Acknowledge Enable
#define I2C_MCS_DATACK          0x00000008  // Acknowledge Data
#define I2C_MCS_ADRACK          0x00000004  // Acknowledge Address
#define I2C_MCS_STOP            0x00000004  // Generate STOP
#define I2C_MCS_START           0x00000002  // Generate START
#define I2C_MCS_ERROR           0x00000002  // Error
#define I2C_MCS_RUN             0x00000001  // I2C Master Enable
#define I2C_MCS_BUSY            0x00000001  // I2C Busy
#define I2C_MCR_MFE             0x00000010  // I2C Master Function Enable

#define MAXRETRIES  5  // number of receive attempts before giving up
void I2C_Init(void){
  SYSCTL_RCGCI2C_R |= SYSCTL_RCGCI2C_R0;         
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;          
  while((SYSCTL_PRGPIO_R&0x0002) == 0){};
	GPIO_PORTB_AFSEL_R |= 0x0C;        
  GPIO_PORTB_ODR_R |= 0x08;         
  GPIO_PORTB_DEN_R |= 0x0C;      
	GPIO_PORTB_AMSEL_R &= ~0x0C;       
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFF00FF)+0x00002200;	//TED		
  I2C0_MCR_R = I2C_MCR_MFE;      				
  I2C0_MTPR_R = 0x0D;										
}

// Receive one byte from specified slave
uint8_t I2C_Recv(int8_t slave){
  int retryCounter = 1;
  do{
    while(I2C0_MCS_R&I2C_MCS_BUSY){};
    I2C0_MSA_R = (slave<<1)&0xFE;    
    I2C0_MSA_R |= 0x01;            
    I2C0_MCS_R = (0 | I2C_MCS_STOP | I2C_MCS_START | I2C_MCS_RUN);   
		SysTick_Wait(1200);			
    while(I2C0_MCS_R&I2C_MCS_BUSY){};
    retryCounter = retryCounter + 1;      
  }                                

  while(((I2C0_MCS_R&(I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0) && (retryCounter <= MAXRETRIES));
  return (I2C0_MDR_R&0xFF);         
}
// Receives two bytes from specified slave
uint16_t I2C_Recv2(int8_t slave){
  uint8_t data1,data2;
  int retryCounter = 1;
  do {
    while(I2C0_MCS_R&I2C_MCS_BUSY){};
    I2C0_MSA_R = (slave<<1)&0xFE;  
    I2C0_MSA_R |= 0x01;            
    I2C0_MCS_R = (0 | I2C_MCS_ACK | I2C_MCS_START | I2C_MCS_RUN);    
    while(I2C0_MCS_R&I2C_MCS_BUSY){};
    data1 = (I2C0_MDR_R&0xFF);     
    I2C0_MCS_R = (0 | I2C_MCS_STOP | I2C_MCS_RUN); 
    while(I2C0_MCS_R&I2C_MCS_BUSY){};
    data2 = (I2C0_MDR_R&0xFF);      
    retryCounter = retryCounter + 1;      
  }             
  while(((I2C0_MCS_R&(I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0) && (retryCounter <= MAXRETRIES));
  return (data1<<8)+data2;          
}

// Send one byte to specified slave
uint32_t I2C_Send1(int8_t slave, uint8_t data1){
  while(I2C0_MCS_R&I2C_MCS_BUSY){};
  I2C0_MSA_R = (slave<<1)&0xFE;   
  I2C0_MSA_R &= ~0x01;           
  I2C0_MDR_R = data1&0xFF;     
	I2C0_MCS_R = (0 | I2C_MCS_STOP | I2C_MCS_START | I2C_MCS_RUN); 
  while(I2C0_MCS_R&I2C_MCS_BUSY){};

  return (I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
}

// Send two bytes to specified slave
uint32_t I2C_Send2(int8_t slave, uint8_t data1, uint8_t data2){
  while(I2C0_MCS_R&I2C_MCS_BUSY){};
  I2C0_MSA_R = (slave<<1)&0xFE;    
  I2C0_MSA_R &= ~0x01;            
  I2C0_MDR_R = data1&0xFF;       
  I2C0_MCS_R = (0 | I2C_MCS_START | I2C_MCS_RUN); 
	SysTick_Wait(1200);		

  while(I2C0_MCS_R&I2C_MCS_BUSY){};

  if((I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0){
    I2C0_MCS_R = (0 | I2C_MCS_STOP );   

    return (I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
  }
  I2C0_MDR_R = data2&0xFF;      
  I2C0_MCS_R = (0 | I2C_MCS_STOP | I2C_MCS_RUN);  
  while(I2C0_MCS_R&I2C_MCS_BUSY){}
  return (I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
}

// Send three bytes to specified slave
uint32_t I2C_Send3(int8_t slave, uint8_t data1, uint8_t data2, uint8_t data3){
  while(I2C0_MCS_R&I2C_MCS_BUSY){};
  I2C0_MSA_R = (slave<<1)&0xFE; 
  I2C0_MSA_R &= ~0x01;           
  I2C0_MDR_R = data1&0xFF;     
  I2C0_MCS_R = (0 | I2C_MCS_START | I2C_MCS_RUN);  
 
  while(I2C0_MCS_R&I2C_MCS_BUSY){};
  if((I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0){
    I2C0_MCS_R = (0 | I2C_MCS_STOP);
    return (I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
  }

  I2C0_MDR_R = data2&0xFF;   
  I2C0_MCS_R = (0 | I2C_MCS_RUN);   

  while(I2C0_MCS_R&I2C_MCS_BUSY){};                         
  if((I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0){
    I2C0_MCS_R = (0 | I2C_MCS_STOP);
    return (I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
  }

  I2C0_MDR_R = data3&0xFF;     
  I2C0_MCS_R = (0 | I2C_MCS_STOP | I2C_MCS_RUN);  
  while(I2C0_MCS_R&I2C_MCS_BUSY){};

  return (I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
}
