#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h> 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/

osThreadId_t tid_Thled1;                        // thread id

GPIO_InitTypeDef led_ld1 = { 
        .Pin = GPIO_PIN_0,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW
    };

		
		void Thled1 (void *argument) {
			(void)argument;
    HAL_GPIO_Init(GPIOB, &led_ld1); // Inicializamos el pin del LED
    
    while (1) {
        // Encender LED
        HAL_GPIO_WritePin(GPIOB, led_ld1.Pin, GPIO_PIN_SET); 
        osDelay(200); // El hilo se duerme 200 ticks
        
        // Apagar LED
        HAL_GPIO_WritePin(GPIOB, led_ld1.Pin, GPIO_PIN_RESET);
        osDelay(800); // El hilo se duerme 800 ticks
    }
}
int Init_Thread (void) {
 
	// Initialize LEDS
    __HAL_RCC_GPIOB_CLK_ENABLE();
  tid_Thled1 = osThreadNew(Thled1,NULL,NULL);
  if (tid_Thled1 == NULL) {
    return(-1);
  }
  return(0);
}
	
