/*
 * proximity_estimation.c
 *
 *  Created on: Mar 28, 2019
 *      Author: Devin
 */

#include "proximity_estimation.h"
#include "xbee_interface.h"
#include "assert.h"


Xbee_Serial_T Xbee_Serial_Port = {UART2, 9600};

static void Init_Xbee_Interface(void);

void Run_Proximity_Estimation(void)
{
    static uint8_t init_xbee_if = FALSE;

    if (!init_xbee_if)
    {
        Init_Xbee_Interface();
        init_xbee_if = TRUE;
    }

    Xbee_Serial_Write("\r\nTest!!!", sizeof("\r\nTest!!!"));

}

void Init_Xbee_Interface(void)
{
    Init_Xbee_Timer();
    Xbee_Serial_Open(&Xbee_Serial_Port);
}

