#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"

osThreadId_t tid_Control; 
osTimerId_t timer_3s; 

void Thread_Control(void *argument); 
void Timer_Callback(void *argument); 

GPIO_InitTypeDef led_verde = { 
    .Pin = GPIO_PIN_0, // LD1 Verde (PB0 según manual)
    .Mode = GPIO_MODE_OUTPUT_PP,
    .Pull = GPIO_NOPULL,
    .Speed = GPIO_SPEED_FREQ_LOW
};

GPIO_InitTypeDef led_rojo = { 
    .Pin = GPIO_PIN_14, // LD3 Rojo (PB14 según manual)
    .Mode = GPIO_MODE_OUTPUT_PP,
    .Pull = GPIO_NOPULL,
    .Speed = GPIO_SPEED_FREQ_LOW
};

int Init_Thread(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE(); 
    __HAL_RCC_GPIOC_CLK_ENABLE(); 
    
    HAL_GPIO_Init(GPIOB, &led_verde);
    HAL_GPIO_Init(GPIOB, &led_rojo);

    GPIO_InitTypeDef boton_azul = {0};
    boton_azul.Pin = GPIO_PIN_13;
    boton_azul.Mode = GPIO_MODE_IT_RISING;
    boton_azul.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &boton_azul);
    
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 10, 0); 
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
    
    tid_Control = osThreadNew(Thread_Control, NULL, NULL);
    
    // Timer One-Shot (osTimerOnce). No se repetirá automáticamente.
    timer_3s = osTimerNew(Timer_Callback, osTimerOnce, NULL, NULL);
    
    return(0);
}

void Thread_Control(void *argument) {
    (void)argument;
    uint32_t flags;
    
    // Arranque del sistema: Encender LED verde y arrancar la cuenta de 3 segundos
    HAL_GPIO_WritePin(GPIOB, led_verde.Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, led_rojo.Pin, GPIO_PIN_RESET);
    osTimerStart(timer_3s, 3000); 

    while (1) {
        // Esperamos que alguien levante la bandera 0x01 o la 0x02
        flags = osThreadFlagsWait(0x01 | 0x02, osFlagsWaitAny, osWaitForever);
        
        // Si el timer llega a cero
        if (flags & 0x02) {
            // Apagamos verde, encendemos rojo
            HAL_GPIO_WritePin(GPIOB, led_verde.Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB, led_rojo.Pin, GPIO_PIN_SET);
            
            // El sistema entra en estado de bloqueo absoluto
            while(1) {
                // Al meterlo en este bucle infinito, jamás volverá a leer el botón.
                // Solo un reset físico en la placa lo sacará de aquí.
                osDelay(1000); 
            }
        }
        
        // Si pulsamos el botón (y el timer aún no ha llegado a cero)
        if (flags & 0x01) {
            // La función osTimerStart reinicia la cuenta desde cero automáticamente si ya estaba corriendo
            osTimerStart(timer_3s, 3000); 
        }
    }
}

void Timer_Callback(void *argument) {
    (void)argument;
    // Mandamos la bandera 0x02 de "tiempo agotado" al hilo administrador
    osThreadFlagsSet(tid_Control, 0x02);
}

void EXTI15_10_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_13) {
        // Mandamos la bandera 0x01 de "botón pulsado" al hilo administrador
        osThreadFlagsSet(tid_Control, 0x01);
    }
}