#include <errno.h>
#include <stdio.h>

#include "stm32f4xx_hal.h"

// Some define to set standard descriptor
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// Global variable
UART_HandleTypeDef guart = {0};

// Define _write required for printf
int _write(int fd, char *ptr, int len) {
    HAL_StatusTypeDef hstatus;

    if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
        hstatus = HAL_UART_Transmit(&guart, (uint8_t *)ptr, len, HAL_MAX_DELAY);
        if (hstatus == HAL_OK)
            return len;
        else
            return EIO;
    }
    errno = EBADF;
    return -1;
}

/*
 * https://www.st.com/resource/en/user_manual/dm00105879-description-of-stm32f4-hal-and-ll-drivers-stmicroelectronics.pdf
 */

int main(void) {
    // This should always be called.
    HAL_Init();

    /* RCC - Reset and Clock Control
     * https://bluetechs.wordpress.com/zothers/x/rcc/#stage2c:enablingperipheralclocks
     *
     * To dramatical lower the current draw, the rcc can be use to enable
     * master clock for only some peripheral thus disabling the others.
     */

    /* SYSCFG Peripheral - System configuration controller
     * Section 7 ~
     * https://www.st.com/resource/en/reference_manual/rm0368-stm32f401xbc-and-stm32f401xde-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
     * https://hackaday.com/2021/03/26/bare-metal-stm32-please-mind-the-interrupt-event/
     *
     * Required mostly all the time
     */
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    /* PWR - Power controller
     * Section 5 ~
     * https://www.st.com/resource/en/reference_manual/rm0368-stm32f401xbc-and-stm32f401xde-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
     *
     * Handle chip internal supply
     */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* Section 5.1.3 ~
     * https://www.st.com/resource/en/reference_manual/rm0368-stm32f401xbc-and-stm32f401xde-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
     *
     * The regular output voltage is around 1.2V, we scale it by 2. This has
     * some special behaviour (like auto scaling to SCALE3 in case of PLL off).
     *
     * Here is the reason:
     *
     * The voltage scaling allows optimizing the power consumption when the
     * device is clocked below the maximum system frequency.
     */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    // Configure Oscillator
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    // Configure Clock
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);

    /*
     * Configure UART
     */

    // Enable USART2 Clock
    __HAL_RCC_USART2_CLK_ENABLE();

    // Enable GPIOA Clock to configure uart gpio
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Configure UART GPIO
    GPIO_InitTypeDef gpio_init = {0};

    gpio_init.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    gpio_init.Mode = GPIO_MODE_AF_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init.Alternate = GPIO_AF7_USART2;
    // Apply GPIO configuration
    HAL_GPIO_Init(GPIOA, &gpio_init);

    // Configure UART on GPIO
    guart.Instance = USART2;
    guart.Init.BaudRate = 115200;
    guart.Init.WordLength = UART_WORDLENGTH_8B;
    guart.Init.StopBits = UART_STOPBITS_1;
    guart.Init.Parity = UART_PARITY_NONE;
    guart.Init.Mode = UART_MODE_TX_RX;
    guart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    guart.Init.OverSampling = UART_OVERSAMPLING_16;
    // Call UART_init from the HAL to apply configuration
    HAL_UART_Init(&guart);

    while (1) {
        printf("Ping.\r\n");
        HAL_Delay(1000);
    }
}
