#include <stdint.h>

// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void);

// Time delay using busy wait.
// The delay parameter is in units of the core clock
void SysTick_Wait(uint32_t delay);

// Time delay using busy wait.
void SysTick_Wait10ms(uint32_t delay);
void SysTick_Wait10us(uint32_t delay);
