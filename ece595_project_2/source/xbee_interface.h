#ifndef __Xbee_INTERFACE
#define __Xbee_SERIAL


#include "fsl_uart.h"

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (1)
#endif

typedef struct Xbee_Serial_Tag
{
    UART_Type *base;
    uint32_t baudrate;
} Xbee_Serial_T;

extern void Init_Xbee_Timer(void);
extern uint32_t Get_Xbee_Seconds_Timer(void);
extern uint32_t Get_Xbee_Milliseconds_Timer(void);

extern int Xbee_Serial_Open(Xbee_Serial_T *serial);
extern void Xbee_Serial_Close(void);

extern int Xbee_Serial_Write(const void *buffer,int length);
extern int Xbee_Serial_Read(void *buffer, int bufsize);

extern int Xbee_Serial_Tx_Free(void);
extern int Xbee_Serial_Tx_Used(void);
extern int Xbee_Serial_Tx_Flush(void);
extern int Xbee_Serial_Rx_Free(void);
extern int Xbee_Serial_Rx_Used(void);
extern int Xbee_Serial_Rx_Flush(void);

#endif
