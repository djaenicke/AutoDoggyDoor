/*
 * proximity_estimation.c
 *
 *  Created on: Mar 28, 2019
 *      Author: Devin
 */

#include "proximity_estimation.h"
#include "assert.h"
#include "xbee\platform.h" /* Xbee library */
#include "xbee\device.h"

xbee_serial_t Xbee_Serial_Port = {UART4, 9600};
xbee_dev_t Xbee;

static void Init_Xbee_Interface(void);

void Run_Proximity_Estimation(void)
{
    static uint8_t init_xbee_if = FALSE;

    if (!init_xbee_if)
    {
        Init_Xbee_Interface();
        init_xbee_if = TRUE;
    }
}

void Init_Xbee_Interface(void)
{
    Init_Xbee_Timer();

    /* Initialize the serial and device layer for this Xbee device */
    if (xbee_dev_init(&Xbee, &Xbee_Serial_Port, NULL, NULL))
    {
        assert(false);
    }
}

