/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/****************************************************************************
*
* Comments:
*   This file contains the functions which write and read the SPI memories
*   using the ECSPI driver in interrupt mode.
*
****************************************************************************/

#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "gpio_pins.h"
#include "board.h"
#include "gpio_ctrl.h"
#include "gpio_imx.h"
#include "rdc_semaphore.h"
#include "debug_console_imx.h"

#define GPIO_DEBOUNCE_DELAY    (100000)
static SemaphoreHandle_t xSemaphore;

static void GPIO_Ctrl_InitKeyPin()
{
#ifdef BOARD_GPIO_KEY_CONFIG
    gpio_init_config_t keyInit = {
        .pin           = BOARD_GPIO_KEY_CONFIG->pin,
        .direction     = gpioDigitalInput,
        .interruptMode = gpioIntFallingEdge
    };

    /* Acquire RDC semaphore before access GPIO to avoid conflict, it's
     * necessary when GPIO RDC is configured as Semaphore Required */
    RDC_SEMAPHORE_Lock(BOARD_GPIO_KEY_RDC_PDAP);

    GPIO_Init(BOARD_GPIO_KEY_CONFIG->base, &keyInit);

    RDC_SEMAPHORE_Unlock(BOARD_GPIO_KEY_RDC_PDAP);

    /* Set GPT interrupt priority 3 */
    NVIC_SetPriority(BOARD_GPIO_KEY_IRQ_NUM, 5);
#endif
}

static void GPIO_Ctrl_InitLedPin()
{
#ifdef BOARD_GPIO_LED_CONFIG
    gpio_init_config_t ledInit = {
        .pin           = BOARD_GPIO_LED_CONFIG->pin,
        .direction     = gpioDigitalOutput,
        .interruptMode = gpioNoIntmode
    };

    /* Acquire RDC semaphore before access GPIO to avoid conflict, it's
     * necessary when GPIO RDC is configured as Semaphore Required */
    RDC_SEMAPHORE_Lock(BOARD_GPIO_LED_RDC_PDAP);

    GPIO_Init(BOARD_GPIO_LED_CONFIG->base, &ledInit);

    RDC_SEMAPHORE_Unlock(BOARD_GPIO_LED_RDC_PDAP);
#endif
}

void GPIO_Ctrl_InitSwitch1Pin()
{
#ifdef BOARD_GPIO_SWITCH1_CONFIG
    /* Acquire RDC semaphore before access GPIO to avoid conflict, it's
     * necessary when GPIO RDC is configured as Semaphore Required */
    RDC_SEMAPHORE_Lock(BOARD_GPIO_SWITCH1_RDC_PDAP);
    GPIO_Init(BOARD_GPIO_SWITCH1_CONFIG->base, &Switch1);
    RDC_SEMAPHORE_Unlock(BOARD_GPIO_SWITCH1_RDC_PDAP);
#endif
}

void GPIO_Ctrl_InitLed1Pin()
{
#ifdef BOARD_GPIO_LED1_CONFIG
    /* Acquire RDC semaphore before access GPIO to avoid conflict, it's
     * necessary when GPIO RDC is configured as Semaphore Required */
    RDC_SEMAPHORE_Lock(BOARD_GPIO_LED1_RDC_PDAP);
    GPIO_Init(BOARD_GPIO_LED1_CONFIG->base, &Led1);
    RDC_SEMAPHORE_Unlock(BOARD_GPIO_LED1_RDC_PDAP);
#endif
}

void GPIO_Ctrl_InitSwitch2Pin()
{
#ifdef BOARD_GPIO_SWITCH2_CONFIG
    /* Acquire RDC semaphore before access GPIO to avoid conflict, it's
     * necessary when GPIO RDC is configured as Semaphore Required */
    RDC_SEMAPHORE_Lock(BOARD_GPIO_SWITCH2_RDC_PDAP);
    GPIO_Init(BOARD_GPIO_SWITCH2_CONFIG->base, &Switch2);
    RDC_SEMAPHORE_Unlock(BOARD_GPIO_SWITCH2_RDC_PDAP);
#endif
}

void GPIO_Ctrl_InitLed2Pin()
{
#ifdef BOARD_GPIO_LED2_CONFIG
    /* Acquire RDC semaphore before access GPIO to avoid conflict, it's
     * necessary when GPIO RDC is configured as Semaphore Required */
    RDC_SEMAPHORE_Lock(BOARD_GPIO_LED2_RDC_PDAP);
    GPIO_Init(BOARD_GPIO_LED2_CONFIG->base, &Led2);
    RDC_SEMAPHORE_Unlock(BOARD_GPIO_LED2_RDC_PDAP);
#endif
}




void GPIO_Ctrl_Init()
{
	xSemaphore = xSemaphoreCreateBinary();

	GPIO_Ctrl_InitKeyPin();
	GPIO_Ctrl_InitLedPin();
	GPIO_Ctrl_InitSwitch1Pin();
    GPIO_Ctrl_InitLed1Pin();
    GPIO_Ctrl_InitSwitch2Pin();
    GPIO_Ctrl_InitLed2Pin();
}

void GPIO_Ctrl_ToggleLed(gpio_config_t *boardGpioConfig, uint32_t boardGpioRdc)
{
    static bool onLed0 = 0;
    onLed0 = GPIO_ReadPinOutput(boardGpioConfig->base, boardGpioConfig->pin);
    if(onLed0 == 0)
    {
        RDC_SEMAPHORE_Lock(boardGpioRdc);
        GPIO_WritePinOutput(boardGpioConfig->base,
                            boardGpioConfig->pin, gpioPinSet);
        RDC_SEMAPHORE_Unlock(boardGpioRdc);
    }
    else
    {
        RDC_SEMAPHORE_Lock(boardGpioRdc);
        GPIO_WritePinOutput(boardGpioConfig->base,
                            boardGpioConfig->pin, gpioPinClear);
        RDC_SEMAPHORE_Unlock(boardGpioRdc);
    }
}

void GPIO_Ctrl_ClearLed(gpio_config_t *boardGpioConfig, uint32_t boardGpioRdc)
{
    RDC_SEMAPHORE_Lock(boardGpioRdc);
    GPIO_WritePinOutput(boardGpioConfig->base,
                        boardGpioConfig->pin, gpioPinClear);
    RDC_SEMAPHORE_Unlock(boardGpioRdc);
}

void GPIO_Ctrl_SetLed(gpio_config_t *boardGpioConfig, uint32_t boardGpioRdc)
{

    RDC_SEMAPHORE_Lock(boardGpioRdc);
    GPIO_WritePinOutput(boardGpioConfig->base,
                        boardGpioConfig->pin, gpioPinSet);
    RDC_SEMAPHORE_Unlock(boardGpioRdc);
}

void GPIO_Ctrl_WriteLed(gpio_config_t *boardGpioConfig, uint32_t boardGpioRdc, uint32_t value)
{
    RDC_SEMAPHORE_Lock(boardGpioRdc);
    GPIO_WritePinOutput(boardGpioConfig->base,
                        boardGpioConfig->pin, value);
    RDC_SEMAPHORE_Unlock(boardGpioRdc);
}


uint32_t GPIO_Ctrl_GetKey(gpio_config_t *boardGpioConfig)
{
    return GPIO_ReadPinInput(boardGpioConfig->base, boardGpioConfig->pin);
}

void GPIO_Ctrl_WaitKeyPressed()
{
#ifdef BOARD_GPIO_KEY_CONFIG
    uint32_t i, debounce;

    do
    {
        debounce = 0;

        RDC_SEMAPHORE_Lock(BOARD_GPIO_KEY_RDC_PDAP);

        /* Clear the interrupt state */
        GPIO_ClearStatusFlag(BOARD_GPIO_KEY_CONFIG->base, BOARD_GPIO_KEY_CONFIG->pin);
        /* Enable GPIO pin interrupt */
        GPIO_SetPinIntMode(BOARD_GPIO_KEY_CONFIG->base, BOARD_GPIO_KEY_CONFIG->pin, true);

        RDC_SEMAPHORE_Unlock(BOARD_GPIO_KEY_RDC_PDAP);

        /* Enable the IRQ. */
        NVIC_EnableIRQ(BOARD_GPIO_KEY_IRQ_NUM);

        PRINTF("\n\rPress the (%s) key to switch the blinking frequency:\n\r", BOARD_GPIO_KEY_CONFIG->name);
        xSemaphoreTake(xSemaphore, portMAX_DELAY);

        for (i = 0; i < 3; i++)
        {
            /* Susupend Task to wait Key stable. */
            vTaskDelay(5);

            /* Check key value. */
            if (0 == GPIO_ReadPinInput(BOARD_GPIO_KEY_CONFIG->base, BOARD_GPIO_KEY_CONFIG->pin))
            {
                /* Increase debounce counter. */
                debounce ++;
            }
        }

        if (debounce >= 2)
        {
            break;
        }
    } while (1);
#else
    /* Without key on board, we return every 5 seconds */
    PRINTF("\n\rWait 5 seconds to switch blinking frequency:\n\r");
    xSemaphoreTake(xSemaphore, configTICK_RATE_HZ * 5);
#endif
}

#ifdef BOARD_GPIO_KEY_CONFIG
void BOARD_GPIO_KEY_HANDLER()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* When user input captured, we disable GPIO interrupt */
    NVIC_DisableIRQ(BOARD_GPIO_KEY_IRQ_NUM);

    RDC_SEMAPHORE_Lock(BOARD_GPIO_KEY_RDC_PDAP);

    /* Disable GPIO pin interrupt */
    GPIO_SetPinIntMode(BOARD_GPIO_KEY_CONFIG->base, BOARD_GPIO_KEY_CONFIG->pin, false);
    /* Clear the interrupt state */
    GPIO_ClearStatusFlag(BOARD_GPIO_KEY_CONFIG->base, BOARD_GPIO_KEY_CONFIG->pin);

    RDC_SEMAPHORE_Unlock(BOARD_GPIO_KEY_RDC_PDAP);

    /* Unlock the task to process the event. */
    xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);

    /* Perform a context switch to wake the higher priority task. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
#endif

/*******************************************************************************
 * EOF
 ******************************************************************************/
