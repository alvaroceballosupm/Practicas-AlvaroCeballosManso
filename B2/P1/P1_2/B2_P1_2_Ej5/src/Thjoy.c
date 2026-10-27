#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"

// 1. Identificadores del hilo y del timer
osThreadId_t tid_Thjoy;
osTimerId_t timer_joy;

// 2. Variables para contar las veces que pulsamos cada gesto[cite: 6]
uint32_t cuenta_arriba = 0;
uint32_t cuenta_derecha = 0;
uint32_t cuenta_abajo = 0;
uint32_t cuenta_izquierda = 0;
uint32_t cuenta_centro = 0;

// Prototipos de funciones
void Thjoy(void *argument);
void TimerJoy_Callback(void *argument);

// Función de inicialización del driver
int Init_Thjoy(void) {
    // A. Encendemos el reloj de los Puertos B y E (donde están los LEDs y el Joystick)
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    // B. Configuramos los 3 LEDs (Verde=PB0, Azul=PB7, Rojo=PB14)
    GPIO_InitTypeDef config_leds = {0};
    config_leds.Pin = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14;
    config_leds.Mode = GPIO_MODE_OUTPUT_PP;
    config_leds.Pull = GPIO_NOPULL;
    config_leds.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &config_leds);

    // C. Configuramos los pines del Joystick en el Puerto B (Arriba=10, Derecha=11)
    GPIO_InitTypeDef joy_B = {0};
    joy_B.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    joy_B.Mode = GPIO_MODE_IT_RISING; // Que salte interrupción al pulsar
    joy_B.Pull = GPIO_PULLDOWN;       // Para que por defecto lea un cero (0V)
    HAL_GPIO_Init(GPIOB, &joy_B);

    // D. Configuramos los pines del Joystick en el Puerto E (Abajo=12, Izquierda=14, Centro=15)[cite: 7]
    GPIO_InitTypeDef joy_E = {0};
    joy_E.Pin = GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15;
    joy_E.Mode = GPIO_MODE_IT_RISING;
    joy_E.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOE, &joy_E);

    // E. Activamos las interrupciones en el sistema
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    // F. Creamos el hilo y el timer de un solo disparo (One-Shot)[cite: 6]
    tid_Thjoy = osThreadNew(Thjoy, NULL, NULL);
    timer_joy = osTimerNew(TimerJoy_Callback, osTimerOnce, NULL, NULL);

    return 0;
}

// Función auxiliar para codificar el valor en los LEDs (LD1 es el LSB)[cite: 7]
void Mostrar_Gesto_Leds(uint8_t valor) {
    // Primero apagamos todos
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14, GPIO_PIN_RESET);
    
    // Y encendemos solo los correspondientes mirando el código binario
    if (valor & 0x01) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);  // LD1 (Verde)[cite: 7]
    if (valor & 0x02) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);  // LD2 (Azul)
    if (valor & 0x04) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); // LD3 (Rojo)
}

// El hilo principal que gestiona la lógica
void Thjoy(void *argument) {
    (void)argument;
    uint32_t flags;

    while(1) {
        // El hilo duerme esperando la bandera del Botón (0x01) o del Timer (0x02)[cite: 6]
        flags = osThreadFlagsWait(0x01 | 0x02, osFlagsWaitAny, osWaitForever);

        if (flags & 0x01) {
            // ¡Han tocado un botón o es un rebote! Arrancamos/reiniciamos el timer a 50ms[cite: 6]
            osTimerStart(timer_joy, 50);
        }
        
        if (flags & 0x02) {
            // El timer de 50ms se ha agotado. La pulsación es estable y real[cite: 6]
            
            // Leemos los puertos para ver cuál de los 5 pines está activado y sumamos su contador[cite: 6]
            if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET) {
                cuenta_arriba++;
                Mostrar_Gesto_Leds(1); // 1 = Arriba[cite: 7]
            }
            else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET) {
                cuenta_derecha++;
                Mostrar_Gesto_Leds(2); // 2 = Derecha[cite: 7]
            }
            else if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_SET) {
                cuenta_abajo++;
                Mostrar_Gesto_Leds(3); // 3 = Abajo[cite: 7]
            }
            else if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_SET) {
                cuenta_izquierda++;
                Mostrar_Gesto_Leds(4); // 4 = Izquierda[cite: 7]
            }
            else if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_SET) {
                cuenta_centro++;
                Mostrar_Gesto_Leds(5); // 5 = Centro[cite: 7]
            }
        }
    }
}

// Función del timer cuando pasan los 50ms
void TimerJoy_Callback(void *argument) {
    (void)argument;
    osThreadFlagsSet(tid_Thjoy, 0x02); // Mandamos la bandera 0x02 al hilo[cite: 6]
}

// ------ FUNCIONES PURAS DE HARDWARE ------

// Centralita de interrupciones para los pines del 10 al 15
void EXTI15_10_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10); // Arriba
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11); // Derecha
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12); // Abajo
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14); // Izquierda
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15); // Centro
}

// Si la placa detecta electricidad en algún pin, avisa al hilo enviando la bandera 0x01
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_10 || GPIO_Pin == GPIO_PIN_11 || 
        GPIO_Pin == GPIO_PIN_12 || GPIO_Pin == GPIO_PIN_14 || GPIO_Pin == GPIO_PIN_15) {
        
        osThreadFlagsSet(tid_Thjoy, 0x01); // Mandamos bandera de "Interrupción/Rebote detectado"[cite: 6]
    }
}