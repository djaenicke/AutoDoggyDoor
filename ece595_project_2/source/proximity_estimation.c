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
static uint8_t Xbee_AT_Resp[RESP_BUF_SIZE];


void Run_Proximity_Estimation(void)
{

}

void Init_Xbee_Interface(void)
{
    Init_Xbee_Timer();
    Xbee_Serial_Open(&Xbee_Serial_Port);

    Send_Xbee_AT_Cmd_Blocking(GET_API_MODE, Xbee_AT_Resp, sizeof(Xbee_AT_Resp));
    if ('2' != Xbee_AT_Resp[0])
    {
        /* Default to API mode */
        Enter_Xbee_API_Mode();
    }

    Send_Xbee_AT_Cmd_Blocking(GET_POWER_LEVEL, Xbee_AT_Resp, sizeof(Xbee_AT_Resp));
    if (LOWEST != Xbee_AT_Resp[0])
    {
        /* Default to lowest power mode */
        Change_Xbee_Power_Level_Blocking(LOWEST);
    }
}

