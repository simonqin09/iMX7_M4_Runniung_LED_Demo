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

#include <stdio.h>
#include "board.h"
#include "gpio_pins.h"
#include "gpio_imx.h"
#include "gpio_ctrl.h"
#include "debug_console_imx.h"
#include "rpmsg/rpmsg_rtos.h"
#include "FreeRTOS.h"
#include "task.h"
#include "gpt.h"
#include "semphr.h"
#include "string.h"
#include "mu_imx.h"
#include "rdc_semaphore.h"
//#include "hw_timer.h"
#include "rpmsg_platform_porting.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
/* default task ram size */
#define APP_TASK_STACK_SIZE 256
/* minimal blinking interval time 100ms */
#define BLINKING_INTERVAL_MIN    (100)
/* APP decided interrupt priority */
#define APP_MU_IRQ_PRIORITY 3
/* define max string size */
#define MAX_STRING_SIZE 496

/* static global variables */
static volatile uint32_t blinkingInterval = BLINKING_INTERVAL_MIN;
static uint32_t sendNewData = 0;
static SemaphoreHandle_t app_sema;
static struct remote_device *rdev;
static struct rpmsg_channel *app_chnl;
static char strVar[2][MAX_STRING_SIZE+1];
static uint8_t handler_idx = 0;


/* rpmsg_rx_callback will call into this for a channel creation event*/
static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl) {
    app_chnl = rp_chnl;
    xSemaphoreGiveFromISR(app_sema, NULL);
}

static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl) {
    rpmsg_destroy_ept(rp_chnl->rp_ept);
}

static void rpmsg_read_cb(struct rpmsg_channel *rp_chnl, void *data, int len, void * priv, unsigned long src) {
    /*
     * Temporarily Disable MU Receive Interrupt to avoid master
     * sending too many messages and remote will fail to keep pace
     * to consume
     */
    MU_DisableRxFullInt(MUB, RPMSG_MU_CHANNEL);
    /*
     * Copy to next app string buffer
     */
    assert(len <= MAX_STRING_SIZE);
    //assert(len <= MAX_STRING_SIZE);
    memcpy((void*) strVar[handler_idx], data, len);
    printf("received data is %s", strVar[handler_idx]);
    /*
     * Add trailing '\0'
     */
    strVar[handler_idx][len] = 0;
    handler_idx = (handler_idx + 1) % 2;
    xSemaphoreGiveFromISR(app_sema, NULL);
}


/******************************************************************************
 *
 * Function Name: ToggleTask
 * Comments: this task is used to turn toggle on/off LED.
 *
 ******************************************************************************/
static void ToggleTask(void *pvParameters)
{
	while(true)
	{
		/* BOARD_GPIO_LED_CONFIG toggle */
		GPIO_Ctrl_ToggleLed(BOARD_GPIO_LED_CONFIG, BOARD_GPIO_LED_RDC_PDAP);
		/* Use vTaskDelay to get delay, per configTICK_RATE_HZ=1000hz, single time slot = 1ms */
		vTaskDelay(blinkingInterval);
		/* BOARD_GPIO_LED1_CONFIG toggle */
		GPIO_Ctrl_ToggleLed(BOARD_GPIO_LED1_CONFIG, BOARD_GPIO_LED1_RDC_PDAP);
		vTaskDelay(blinkingInterval);
		/* BOARD_GPIO_LED2_CONFIG toggle */
		GPIO_Ctrl_ToggleLed(BOARD_GPIO_LED2_CONFIG, BOARD_GPIO_LED1_RDC_PDAP);
		vTaskDelay(blinkingInterval);
	}
}

/******************************************************************************
*
* Function Name: SwitchTask
* Comments: this task is used to change blinking frequency.
*
******************************************************************************/
void SwitchTask(void *pvParameters)
{
	char result[50];

	while (true)
    {
        PRINTF("\n\r====== Blinking interval %dms ======\n\r", blinkingInterval);
        GPIO_Ctrl_WaitKeyPressed();
        blinkingInterval += 100;
        //sendNewData = 1;
        if (blinkingInterval > 1000)
        {
        	blinkingInterval = BLINKING_INTERVAL_MIN;
        }

        // send new interval update message to host */
        sprintf(result, "Interval=%d\n", (int) blinkingInterval);
        /* Send message... */
        rpmsg_send(app_chnl, result, strlen(result));
    }
}



/******************************************************************************
 *
 * Function Name: StrEchoTask
 * Comments: this task is used to communicate between M4 FreeRtos and A7 Linux.
 *
 ******************************************************************************/
static void StrEchoTask(void *pvParameters)
{
	char result[50];
	char * pch0;
	char * pch1;
	int pch2;

	/*
	 * Prepare for the MU Interrupt
	 * MU must be initialized before rpmsg init is called
	*/
	MU_Init(BOARD_MU_BASE_ADDR);
	NVIC_SetPriority(BOARD_MU_IRQ_NUM, APP_MU_IRQ_PRIORITY);
	NVIC_EnableIRQ(BOARD_MU_IRQ_NUM);

    /* Print the initial banner */
    PRINTF("\r\nRPMSG String Echo FreeRTOS RTOS API Demo...\r\n");

    /* create new semaphore */
    app_sema = xSemaphoreCreateBinary();

    /* RPMSG Init as REMOTE */
    PRINTF("RPMSG Init as Remote\r\n");
    rpmsg_init(0, &rdev, rpmsg_channel_created, rpmsg_channel_deleted, rpmsg_read_cb, RPMSG_MASTER);
    PRINTF("================== RPMSG DONE===================\n\r");

    /* send message flag set */
    sendNewData = 1;

    xSemaphoreTake(app_sema, portMAX_DELAY);

    //printf("initiate gpio key");

    /* initiate and enable GPIO KEY interrupt */
    RDC_SEMAPHORE_Lock(BOARD_GPIO_KEY_RDC_PDAP);
    /* Clear the interrupt state */
    GPIO_ClearStatusFlag(BOARD_GPIO_KEY_CONFIG->base, BOARD_GPIO_KEY_CONFIG->pin);
    /* Enable GPIO pin interrupt */
    GPIO_SetPinIntMode(BOARD_GPIO_KEY_CONFIG->base, BOARD_GPIO_KEY_CONFIG->pin, true);
    RDC_SEMAPHORE_Unlock(BOARD_GPIO_KEY_RDC_PDAP);
    /* Enable the IRQ. */
    NVIC_EnableIRQ(BOARD_GPIO_KEY_IRQ_NUM);

    while(true)
    {
    	xSemaphoreTake(app_sema, portMAX_DELAY);

    	//PRINTF("start to process received data\n\r");
        /*Parse MSG*/
        pch0 = strstr (strVar[0], "Interval");
        pch1 = strstr (strVar[1], "Interval");
        if(pch0)
        {
            pch0 = pch0 + 9;
            pch2 = atoi(pch0);
            //printf("pch2 = %d", pch2);
            /*determine if interval value got is no more than 1000 */
            if(pch2 <= 1000)
            {
                /* pass value to blinkingInterval */
            	blinkingInterval = pch2;
            	/* set the send flag to update new interval value to A7 */
                sendNewData = 1;
            }
            else
            {
            	blinkingInterval = BLINKING_INTERVAL_MIN;
            	sendNewData = 1;
            }
        }
        else
        {
        	if(pch1)
        	{
        	    pch1 = pch1 + 9;
        	    pch2 = atoi(pch1);
        	    printf("pch2 = %d", pch2);
        	    /*determine if interval value got is no more than 1000 */
        	    if(pch2 <= 1000)
        	    {
        	        /* pass value to blinkingInterval */
        	    	blinkingInterval = pch2;
        	    	/* set the send flag to update new interval value to A7 */
        	        sendNewData = 1;
        	    }
        	    else
        	    {
        	    	blinkingInterval = BLINKING_INTERVAL_MIN;
        	    	sendNewData = 1;
        	    }
        	}
        }
        //PRINTF("finish process received data\n\r");

        /* blinkingInterval changed */
        if(sendNewData == 1)
        {
        	//PRINTF("\r\n data change \r\n");
        	/* clear flag */
        	sendNewData = 0;
            //PRINTF("================== LED Blinking Interval = %dms ==================\n\r", blinkingInterval);

            /* Create message */
            sprintf(result, "Interval=%d\n", (int) blinkingInterval);
            /* Send message... */
            rpmsg_send(app_chnl, result, strlen(result));

        }
        MU_EnableRxFullInt(MUB, RPMSG_MU_CHANNEL);
    }
}
/*
 * MU Interrrupt ISR
 */
void BOARD_MU_HANDLER(void)
{
    /*
     * calls into rpmsg_handler provided by middleware
     */
    rpmsg_handler();
}


/******************************************************************************
 *
 * Function Name: main
 * Comments: Hello World Example with GPIO.
 *   This example include:
 *   Configure BUTTON1 as GPIO functionality
 *     and check the button's state(pressed or released). According to the Button
 *     status, copy it to the LED
 *
 ******************************************************************************/
int main(void)
{
    /* hardware initialiize, include RDC, IOMUX, Uart debug initialize */
    hardware_init();
    PRINTF("\n\r\n\r\n\r");
    PRINTF("=======================================================\n\r");
    PRINTF("============ GPIO FreeRtos and MCC Example ============\n\r");
    PRINTF("========== Play with Led and Button and MCC ===========\n\r");
    PRINTF("=======================================================\n\r");

    /* GPIO module initialize, configure "LED" as output and button as interrupt mode. */
    GPIO_Ctrl_Init();

    /* Create a demo task. */
    xTaskCreate(ToggleTask, "Toggle Task", APP_TASK_STACK_SIZE,
                    NULL, tskIDLE_PRIORITY+1, NULL);
    xTaskCreate(SwitchTask, "Switch Task", APP_TASK_STACK_SIZE,
                        NULL, tskIDLE_PRIORITY+2, NULL);
    xTaskCreate(StrEchoTask, "String Echo Task", APP_TASK_STACK_SIZE,
                NULL, tskIDLE_PRIORITY+2, NULL);

    /* Start FreeRTOS scheduler. */
    vTaskStartScheduler();

    /* Should never reach this point. */
    while (true);
}
/*******************************************************************************
 * EOF
 ******************************************************************************/
