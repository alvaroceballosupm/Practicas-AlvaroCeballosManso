#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h> 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/

osThreadId_t tid_Thled2;                        // thread id
extern osThreadId_t tid_Thled3;

GPIO_InitTypeDef led_ld2 = { 
        .Pin = GPIO_PIN_7,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW
    };

		
		void Thled2 (void *argument) {
			(void)argument;
    HAL_GPIO_Init(GPIOB, &led_ld2); // Inicializamos el pin del LED
    
    while (1) {
        // Encender LED
			osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
       for (int i = 1; i <= 20; i++) {
            HAL_GPIO_WritePin(GPIOB, led_ld2.Pin, GPIO_PIN_SET);
            osDelay(137);
            HAL_GPIO_WritePin(GPIOB, led_ld2.Pin, GPIO_PIN_RESET);
            osDelay(137);
            
            // En el ciclo 15, mandamos señal al hilo 3
            if (i == 15) {
                osThreadFlagsSet(tid_Thled3, 0x01);
            }
        }
    }
}

	
