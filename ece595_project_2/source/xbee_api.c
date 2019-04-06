/*
 * xbee_api.c
 *
 *  Created on: Mar 31, 2019
 *      Author: Devin
 */
#include <string.h>
#include "xbee_api.h"
#include "xbee_interface.h"
#include "assert.h"

#define START_DELIMETER ((uint8_t) 0x7E)
#define TX_16_BIT_ADDR  ((uint8_t) 0x01)
#define TX_HEADER_SIZE  ((uint8_t) 0x08)
#define CHECKSUM_SIZE   ((uint8_t) 0x01)

#define MAX_RF_DATA_SIZE 256
#define TX_BUFFER_SIZE 265
#define RX_BUFFER_SIZE 265

#define TX_STATUS_FRAME_SIZE (7)
#define TX_STATUS_INDEX (5)

#define RSSI_INDEX (6)
#define START_RF_DATA_INDEX (8)

typedef union {
    struct {
        uint8_t delimeter;
        union {
            uint16_t u16;
            uint8_t u8[2];
        } length;
        uint8_t frame_type;
        uint8_t frame_id;
        union {
            uint16_t u16;
            uint8_t u8[2];
        } dest_addr;
        uint8_t options;
    } s;
    uint8_t bytes[8];
} Xbee_API_Tx_Frame_Header_T;

static uint8_t Tx_Buffer[TX_BUFFER_SIZE];
static uint8_t Rx_Buffer[RX_BUFFER_SIZE];

static uint8_t Compute_Checksum(uint8_t checksum, uint8_t *data, uint8_t size);

Tx_Status_T Xbee_Send_Msg_Blocking(char *rf_data, uint8_t rf_data_len)
{
    Xbee_API_Tx_Frame_Header_T header;
    uint8_t checksum = 0xFF;

    if (rf_data_len > MAX_RF_DATA_SIZE)
    {
        assert(0);
    }

    /* Populate the header */
    header.s.delimeter = START_DELIMETER;
    header.s.length.u16 = 5 + rf_data_len;
    header.s.frame_type = TX_16_BIT_ADDR;
    header.s.frame_id = 0x01;
    header.s.dest_addr.u8[0] = 0xFF;
    header.s.dest_addr.u8[1] = 0xFF;
    header.s.options = 0x00;

    /* Compute the checksum */
    checksum -= header.s.frame_type + header.s.frame_id + \
                header.s.dest_addr.u8[0] + header.s.dest_addr.u8[1] + header.s.options;
    checksum = Compute_Checksum(checksum, (uint8_t *) rf_data, rf_data_len);

    /* Construct the frame */
    Tx_Buffer[0] = header.s.delimeter;
    Tx_Buffer[1] = header.s.length.u8[1];
    Tx_Buffer[2] = header.s.length.u8[0];
    Tx_Buffer[3] = header.s.frame_type;
    Tx_Buffer[4] = header.s.frame_type;
    Tx_Buffer[5] = header.s.dest_addr.u8[1];
    Tx_Buffer[6] = header.s.dest_addr.u8[0];
    Tx_Buffer[7] = header.s.options;
    memcpy(&(Tx_Buffer[8]), (uint8_t *) rf_data, rf_data_len);
    Tx_Buffer[8+rf_data_len] = checksum;

    Xbee_Serial_Write(Tx_Buffer, 9+rf_data_len);

    /* Wait for TX status */
    while(Xbee_Serial_Rx_Used() != TX_STATUS_FRAME_SIZE);
    Xbee_Serial_Read(Rx_Buffer, TX_STATUS_FRAME_SIZE);

    return ((Tx_Status_T)Rx_Buffer[TX_STATUS_INDEX]);

}

uint8_t Xbee_Receive_Msg_Blocking(char *rf_data, uint8_t rf_data_len, uint8_t exp_size)
{
    uint8_t avail = 0;
    int16_t rf_data_size;
    int bytes_read;

    avail = Xbee_Serial_Rx_Used();

    if ((avail - START_RF_DATA_INDEX - 1) != exp_size)
    {
        bytes_read = Xbee_Serial_Read(Rx_Buffer, avail);
        return (0);
    }

    rf_data_size = (avail - START_RF_DATA_INDEX - 1);

    if (rf_data_size < 0)
    {
        /* This should not happen */
        assert(0);
    }

    if (rf_data_size > rf_data_len)
    {
        /* receive buffer is not large enough */
        assert(0);
    }

    memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
    bytes_read = Xbee_Serial_Read(Rx_Buffer, avail);
    memcpy(rf_data, &(Rx_Buffer[START_RF_DATA_INDEX]), rf_data_size);

    return(rf_data_size);
}

uint8_t Get_Last_RSSI(void)
{
    return(Rx_Buffer[RSSI_INDEX]);
}

static uint8_t Compute_Checksum(uint8_t checksum, uint8_t *data, uint8_t size)
{
    uint16_t i;

    for (i=0; i<size; i++)
    {
        checksum -= data[i];
    }

    return checksum;
}

