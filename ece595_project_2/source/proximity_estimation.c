/*
 * proximity_estimation.c
 *
 *  Created on: Mar 28, 2019
 *      Author: Devin
 */
#include <stdio.h>
#include "proximity_estimation.h"
#include "xbee_interface.h"
#include "xbee_atcmd.h"
#include "assert.h"


Xbee_Serial_T Xbee_Serial_Port = {UART2, 9600};

static void Init_Xbee_Interface(void);

void Run_Proximity_Estimation(void)
{
    static uint8_t init_xbee_if = FALSE;
    static uint8_t print = FALSE;

    if (!init_xbee_if)
    {
        Init_Xbee_Interface();
        init_xbee_if = TRUE;
    }

    /* This should be called periodically */
    Process_Xbee_AT_Cmd();

    if (Xbee_AT_Cmd_Response_Rxed() && FALSE == print)
    {
        printf("\nAPI MODE = %s", Get_Xbee_AT_Cmd_Response());
        print = TRUE;
    }

}

void Init_Xbee_Interface(void)
{
    Init_Xbee_Timer();
    Xbee_Serial_Open(&Xbee_Serial_Port);
    Queue_Xbee_AT_Cmd(GET_API_MODE);
}

