#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h> 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/

osThreadId_t tid_Thled3;
extern osThreadId_t tid_Thled2;// thread id

GPIO_InitTypeDef led_ld3 = { 
        .Pin = GPIO_PIN_14,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW
    };

		
		void Thled3 (void *argument) {
			(void)argument;
    HAL_GPIO_Init(GPIOB, &led_ld3); // Inicializamos el pin del LED
    
    while (1) {
      osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
        
        for (int i = 1; i <= 30; i++) {
            HAL_GPIO_WritePin(GPIOB, led_ld3.Pin, GPIO_PIN_SET);
            osDelay(287);
            HAL_GPIO_WritePin(GPIOB, led_ld3.Pin, GPIO_PIN_RESET);
            osDelay(287);
            
            // En el ciclo 25, mandamos señal de vuelta al hilo 2
            if (i == 25) {
                osThreadFlagsSet(tid_Thled2, 0x01);
            }
        }
    }
}

	
