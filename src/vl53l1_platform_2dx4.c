#include "tm4c1294ncpdt.h"
#include "vl53l1_platform_2dx4.h"
#include "onboardLEDs.h"
#include "SysTick_Emily.h"
#include <string.h>
#include <time.h>
#include <math.h>

uint8_t _I2CBuffer[256];

// Configure master to write data to dev
int8_t beginTxI2C(uint8_t dev){
  I2C0_MSA_R = (dev<<1)&0xFE;   
  I2C0_MSA_R &= ~0x01;            
	return 0;
}

// Configure master to read data from dev
int8_t beginRxI2C(uint8_t dev){
		I2C0_MSA_R = (dev<<1)&0xFE;    
    I2C0_MSA_R |= 0x01;            
	return 0;
}
	
int8_t writeRegisterIndex(uint16_t index) {
	uint8_t data1, data2;

	data1 = index>>8;
	data2 = index;
	
  I2C0_MDR_R = data1&0xFF;     
  I2C0_MCS_R = 0x03&0xFF;		

  SysTick_Wait(1200);	
	
  while(I2C0_MCS_R&I2C_MCS_BUSY){};
                                    
  if((I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0){
    I2C0_MCS_R = 0x04&0xFF;
                                        
    return (I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
  }
  I2C0_MDR_R = data2&0xFF;        
  I2C0_MCS_R = 0x05&0xFF;
	
	SysTick_Wait(1200);		
	
  while(I2C0_MCS_R&I2C_MCS_BUSY){};
		
  if((I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0){
    I2C0_MCS_R = 0x04&0xFF;
                                      
    return (I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
  }	
	return 0;
}

int8_t writeRegisterIndex_nostop(uint16_t index) {
	uint8_t data1, data2;

	data1 = index>>8;
	data2 = index;
	
  I2C0_MDR_R = data1&0xFF;  
  I2C0_MCS_R =  0x03&0xFF;		
	
	SysTick_Wait(1200);	
	
  while(I2C0_MCS_R&I2C_MCS_BUSY){};
		
  if((I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0){
    I2C0_MCS_R = 0x04&0xFF;
                                      
    return (I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
  }
  I2C0_MDR_R = data2&0xFF;      
  I2C0_MCS_R = 0x01&0xFF;	
	
	SysTick_Wait(1200);		
	
  while(I2C0_MCS_R&I2C_MCS_BUSY){};
		
  if((I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0){
    I2C0_MCS_R = 0x04&0xFF;
                                         
    return (I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
  }	
	return 0;
}


//Write count bytes to dev
int8_t writeI2C(uint8_t *pdata, uint32_t count){

	for(uint32_t i = 0; i < count-1; i++){ 
		I2C0_MDR_R = (*pdata++)&0xFF;    

		if (i == 0) { I2C0_MCS_R =  0x03&0xFF; }			
		else { I2C0_MCS_R =  0x01&0xFF; }

		SysTick_Wait(1200);	
		
		while(I2C0_MCS_R&I2C_MCS_BUSY){};
			
		if((I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0){
			I2C0_MCS_R = 0x04&0xFF;
                                        
			return (I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
		}
	}

	I2C0_MDR_R = (*pdata)&0xFF;      
	I2C0_MCS_R = 0x05&0xFF; 

	SysTick_Wait(1200);			
	
  while(I2C0_MCS_R&I2C_MCS_BUSY){};
		
		if((I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0){
			I2C0_MCS_R = 0x04&0xFF;
                                       
			return (I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
		}
	return(0);
}

// Read I2C
int8_t readI2C(uint8_t *pdata){	
	I2C0_MCS_R =  0x07&0xFF; 	
	
	SysTick_Wait(1200);	
	
  while(I2C0_MCS_R&I2C_MCS_BUSY){};
		
	*pdata = (I2C0_MDR_R&0xFF);    
                                       
  if((I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0){

    I2C0_MCS_R = 0x04&0xFF;
																	
    return (I2C0_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));	
	}
	return(0);
}

/* Write Multiple Bytes to VL53L1X
		Transmit in master mode an amount of data in blocking mode using
			dev			Device Handle (address, default 0x29)
			index		The register index inside the device
			pdata		Pointer to uint8_t buffer containing the data to be written (simply, this ia an array of type uint8_t)
			count		Number of bytes in the supplied byte buffer
*/
int8_t VL53L1_WriteMulti( uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count) {
	// send address, index msb, index lsb, loop send pdata-1 (no stop), send last pdata & stop
	beginTxI2C(dev);
	writeRegisterIndex(index);
	writeI2C(pdata, count);
	return 0; 
}

/* Read Multiple Bytes from VL53L1X
		Read in master mode an amount of data in blocking mode using
			dev			Device Handle (address, default 0x29)
			index		The register index inside the device
			pdata		Pointer to uint8_t buffer containing the data to be read (simply, this ia an array of type uint8_t)
			count		Number of bytes in the supplied byte buffer
*/
int8_t VL53L1_ReadMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count){
// send address, send index, receive pdata-1, receive last pdata & stop  
	beginTxI2C(dev);
	writeRegisterIndex(index);	

	beginRxI2C(dev);
	for(uint32_t i = 0; i < count; i++) readI2C(pdata++);
	
	return 0;
}	



/* Write One Byte (8 bits) to VL53L1X
		Transmit in master mode an amount of data in blocking mode using
			dev			Device Handle (address, default 0x29)
			index		The register index inside the device
			data		Variable containing the data to be written
*/
int8_t VL53L1_WrByte(uint16_t dev, uint16_t index, uint8_t data) {
// send address, index msb, index lsb, send data & stop
	beginTxI2C(dev);
	writeRegisterIndex_nostop(index);
	
 	writeI2C(&data, 1);

	return 0; 
}

/* Write Word (16-bits) to VL53L1X
		Transmit in master mode an amount of data in blocking mode using
			dev			Device Handle (address, default 0x29)
			index		The register index inside the device
			data		variable containing the 16-bit data to be written
*/
int8_t VL53L1_WrWord(uint16_t dev, uint16_t index, uint16_t data) {

	uint8_t d1, d2;
	
	// send address, index msb, index lsb, send data msb, data lsb & stop
	beginTxI2C(dev);
	writeRegisterIndex(index);
	
	
	d1 = data>>8&0xFF; 
	d2 = data&0xFF;
	
	writeI2C(&d2, 1);
	writeI2C(&d1, 1);

	return 0;
}

/* Write Double Word (32-bits) to VL53L1X
		Transmit in master mode an amount of data in blocking mode using
			dev			Device Handle (address, default 0x29)
			index		The register index inside the device
			data		variable containing the 32-bit data to be written
*/
int8_t VL53L1_WrDWord(uint16_t dev, uint16_t index, uint32_t data) {

// send address, index msb, index lsb, send data msb, data lsb & stop
	beginTxI2C(dev);
	writeRegisterIndex(index);

	return 0;}

/* Read One Byte (8-bits) from VL53L1X
		Read in master mode an amount of data in blocking mode using
			dev			Device Handle (address, default 0x29)
			index		The register index inside the device
			pdata		Pointer to uint8_t buffer containing the data to be read (simply, this ia an array of type uint8_t)
			count		Number of bytes in the supplied byte buffer
*/
int8_t VL53L1_RdByte(uint16_t dev, uint16_t index, uint8_t *data) {
// send address, send index, receive data & stop  
	beginTxI2C(dev);
	writeRegisterIndex(index);	
	beginRxI2C(dev);
	readI2C(data);
	return 0;
}

int8_t VL53L1_RdWord(uint16_t dev, uint16_t index, uint16_t *data) {
// send address, send index, receive data & stop  
	uint8_t d1, d2;

	beginTxI2C(dev);
	writeRegisterIndex(index);
	
	beginRxI2C(dev);
		
	readI2C(&d1);
	readI2C(&d2);
	
	*data = d1<<8 | d2; 

	return 0;
}

int8_t VL53L1_RdDWord(uint16_t dev, uint16_t index, uint32_t *data) {
	// send address, send index, receive data & stop  
	uint8_t d1, d2, d3, d4;
	
	beginTxI2C(dev);
	writeRegisterIndex(index);	
	beginRxI2C(dev);

	readI2C(&d1);
	readI2C(&d2);
	readI2C(&d3);
	readI2C(&d4);
	*data = d1<<24 | d2<<16 | d3<<8 | d4; 
	return 0;
}

int8_t VL53L1_WaitMs(uint16_t dev, int32_t wait_ms){
  int32_t i;
  for(i=0; i < wait_ms; i++)
    SysTick_Wait(120000);

	return 0;
}