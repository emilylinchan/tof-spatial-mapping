#include <cstdint>

// I2C0SCL connected to PB2
// I2C0SDA connected to PB3 
// SCL and SDA lines pulled to +3.3 V with 10k pull-up resistors 
void I2C_Init(void);

// Receives one byte from specified slave
// Returns 0 if successful, nonzero if error
uint8_t I2C_Recv(int8_t slave);

// Receives two bytes from specified slave
// Returns 0 if successful, nonzero if error
uint16_t I2C_Recv2(int8_t slave);

// Sends one byte to specified slave
// Returns 0 if successful, nonzero if error
uint32_t I2C_Send1(int8_t slave, uint8_t data1);

// Sends two bytes to specified slave
// Returns 0 if successful, nonzero if error
uint32_t I2C_Send2(int8_t slave, uint8_t data1, uint8_t data2);

// Sends three bytes to specified slave
// Returns 0 if successful, nonzero if error
uint32_t I2C_Send3(int8_t slave, uint8_t data1, uint8_t data2, uint8_t data3);
