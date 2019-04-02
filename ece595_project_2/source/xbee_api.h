/*
 * xbee_api.h
 *
 *  Created on: Mar 31, 2019
 *      Author: Devin
 */

#ifndef XBEE_API_H_
#define XBEE_API_H_

#include <stdint.h>

typedef enum {
    TX_SUCCESS=0,
    NO_ACK_RXED,
    CCA_FAILURE,
    PURGED,
} Tx_Status_T;

Tx_Status_T Xbee_Send_Msg_Blocking(char *rf_data, uint8_t rf_data_len);
uint8_t Xbee_Receive_Msg_Blocking(char *rf_data, uint8_t rf_data_len, uint8_t exp_size);
uint8_t Get_Last_RSSI(void);

#endif /* XBEE_API_H_ */
