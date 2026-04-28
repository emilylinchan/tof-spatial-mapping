#include <stdint.h>

extern volatile uint8_t runMotorWheel;  // 1=on, 0=off
extern volatile uint8_t runMotorSensor; // 1=on, 0=off

extern volatile int totalStepsSensor; // used to track pos so that can return to home

// API functions
void motorsInit(void);
void stepMotorWheel(int dir);
void stepMotorSensor(int dir);
void clearMotorWheel(void);
void clearMotorSensor(void);
