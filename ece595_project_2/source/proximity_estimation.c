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

static void Init_Xbee_Interface(void);

void Run_Proximity_Estimation(void)
{
    static uint8_t init_xbee_if = FALSE;

    if (!init_xbee_if)
    {
        Init_Xbee_Interface();
        init_xbee_if = TRUE;

        Send_Xbee_AT_Cmd_Blocking(GET_API_MODE, Xbee_AT_Resp, sizeof(Xbee_AT_Resp));
        printf("\n\rXbee API Mode = %s", Xbee_AT_Resp);

        Send_Xbee_AT_Cmd_Blocking(GET_POWER_LEVEL, Xbee_AT_Resp, sizeof(Xbee_AT_Resp));
        printf("\n\rXbee Power Level = %s", Xbee_AT_Resp);

        Change_Xbee_Power_Level_Blocking(HIGHEST);
        Send_Xbee_AT_Cmd_Blocking(GET_POWER_LEVEL, Xbee_AT_Resp, sizeof(Xbee_AT_Resp));
        printf("\n\rXbee Power Level = %s", Xbee_AT_Resp);
    }
}

void Init_Xbee_Interface(void)
{
    Init_Xbee_Timer();
    Xbee_Serial_Open(&Xbee_Serial_Port);
}

