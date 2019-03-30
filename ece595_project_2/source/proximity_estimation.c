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
#include "xbee\atcmd.h"
#include "xbee\serial.h"

#define TEST_MSG "INIT COMPLETE!!!"
#define STATUS_MSG "\r\nrunning..."

const xbee_dispatch_table_entry_t xbee_frame_handlers[] =
{
    XBEE_FRAME_HANDLE_LOCAL_AT,
    XBEE_FRAME_MODEM_STATUS_DEBUG,
    XBEE_FRAME_TABLE_END
};

xbee_serial_t Xbee_Serial_Port = {UART2, 9600};
xbee_dev_t Xbee;

static void Init_Xbee_Interface(void);

void Run_Proximity_Estimation(void)
{
    static uint8_t init_xbee_if = FALSE;
    int status;

    if (!init_xbee_if)
    {
        Init_Xbee_Interface();
        init_xbee_if = TRUE;
    }

    xbee_dev_tick(&Xbee);

    status = xbee_cmd_query_status(&Xbee);

    if (!status)
    {
        xbee_dev_dump_settings(&Xbee, XBEE_DEV_DUMP_FLAG_DEFAULT);
    }
}

void Init_Xbee_Interface(void)
{
    Init_Xbee_Timer();

    /* Initialize the serial and device layer for this Xbee device */
    if (xbee_dev_init(&Xbee, &Xbee_Serial_Port, NULL, NULL))
        assert(false);

    /* Initialize the AT Command layer for this XBee device and have the
       driver query it for basic information (hardware version, firmware version,
       serial number, IEEE address, etc.) */
    if (xbee_cmd_init_device(&Xbee))
        assert(false);
}

