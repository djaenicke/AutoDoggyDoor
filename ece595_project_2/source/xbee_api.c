/*
 * xbee_api.c
 *
 *  Created on: Mar 31, 2019
 *      Author: Devin
 */
#include <string.h>
#include "xbee_api.h"
#include "assert.h"

#define START_DELIMETER ((uint8_t) 0x7E)
#define TX_16_BIT_ADDR  ((uint8_t) 0x01)
#define TX_HEADER_SIZE  ((uint8_t) 0x08)
#define CHECKSUM_SIZE   ((uint8_t) 0x01)

#define MAX_RF_DATA_SIZE 256
#define TX_BUFFER_SIZE 265

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

static uint8_t Compute_Checksum(uint8_t checksum, uint8_t *data, uint8_t size);

void Xbee_Send_Msg(char *rf_data, uint8_t rf_data_len)
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

    return;
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

