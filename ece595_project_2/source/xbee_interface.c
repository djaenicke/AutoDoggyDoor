#include <errno.h>

#include "fsl_ftm.h"
#include "clock_config.h"
#include "xbee_interface.h"
#include "xbee_cbuf.h"

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK (CLOCK_GetFreq(kCLOCK_BusClk)/4)

#define RX_BUFF_SIZE  255
#define TX_BUFF_SIZE  RX_BUFF_SIZE

static Xbee_Serial_T Xbee_Serial;
static uart_config_t UART_Cfg;
static uart_handle_t UART_Handle;

static uart_transfer_t Write_Xfer;
static uart_transfer_t Read_Xfer;

static xbee_cbuf_t *Tx_Buffer;
static uint8_t Internal_Tx_Buffer[TX_BUFF_SIZE + XBEE_CBUF_OVERHEAD];

static uint8_t Working_Tx_Buffer[TX_BUFF_SIZE];
static uint8_t Working_Rx_Buffer[RX_BUFF_SIZE];

static uint32_t Xbee_Milliseconds_Timer = 0;
static uint32_t Xbee_Seconds_Timer = 0;

static uint32_t Silence_Start = 0;
static uint32_t Xbee_Reboot_Start = 0;

void UART_User_Callback(UART_Type *base, uart_handle_t *handle, status_t status, void *userData);

void Init_Xbee_Timer(void)
{
    ftm_config_t ftm_info;

    FTM_GetDefaultConfig(&ftm_info);

    /* Divide FTM clock by 4 */
    ftm_info.prescale = kFTM_Prescale_Divide_4;

    FTM_Init(FTM0, &ftm_info);

    /*
     * Set timer period.
    */
    FTM_SetTimerPeriod(FTM0, USEC_TO_COUNT(1000U, FTM_SOURCE_CLOCK));

    FTM_EnableInterrupts(FTM0, kFTM_TimeOverflowInterruptEnable);

    EnableIRQ(FTM0_IRQn);

    FTM_StartTimer(FTM0, kFTM_SystemClock);
}

void FTM0_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    FTM_ClearStatusFlags(FTM0, kFTM_TimeOverflowFlag);
    Xbee_Milliseconds_Timer++;

    if (Xbee_Milliseconds_Timer == 1000)
    {
        Xbee_Seconds_Timer++;
    }

    __DSB();
}

extern void Init_Xbee_Timer(void);

uint32_t Get_Xbee_Seconds_Timer(void)
{
    uint32_t ret_val = 0;

    /* Need to disable interrupts to ensure data consistency */
    /* TODO - Devin this will need to be protected from context switches */
    FTM_DisableInterrupts(FTM0, kFTM_TimeOverflowInterruptEnable);
    ret_val = Xbee_Seconds_Timer;
    FTM_EnableInterrupts(FTM0, kFTM_TimeOverflowInterruptEnable);

    return(ret_val);
}

uint32_t Get_Xbee_Milliseconds_Timer(void)
{
    uint32_t ret_val = 0;

    /* Need to disable interrupts to ensure data consistency */
    FTM_DisableInterrupts(FTM0, kFTM_TimeOverflowInterruptEnable);
    ret_val = Xbee_Milliseconds_Timer;
    FTM_EnableInterrupts(FTM0, kFTM_TimeOverflowInterruptEnable);

    return(ret_val);
}

int Xbee_Serial_Write(const void *buffer, int length)
{
    int ret_val;

    if (length < 0)
        return -EIO;

    if (length > TX_BUFF_SIZE)
        length = TX_BUFF_SIZE;

    /* Add the new data to the circular buffer */
    ret_val = xbee_cbuf_put(Tx_Buffer, buffer, length);

    /* Check if the UART driver is already busy sending data */
    if (kUART_TxDataRegEmptyFlag & UART_GetStatusFlags(Xbee_Serial.base))
    {
        /* Send the new data since the driver isn't busy */
        Write_Xfer.dataSize = Xbee_Serial_Tx_Used();
        Write_Xfer.data = Working_Tx_Buffer;
        xbee_cbuf_get(Tx_Buffer, Working_Tx_Buffer, Write_Xfer.dataSize);
        UART_TransferSendNonBlocking(Xbee_Serial.base, &UART_Handle, &Write_Xfer);
    }

    return (ret_val);
}

int Xbee_Serial_Read(void *buffer, int bufsize)
{
    int avail = Xbee_Serial_Rx_Used();
    size_t bytes_read = 0;

    if (bufsize < 0)
        return -EIO;

    if (avail)
    {
        if (bufsize > avail)
        {
            bufsize = avail;
        }

        Read_Xfer.data = buffer;
        Read_Xfer.dataSize = bufsize;

        UART_TransferReceiveNonBlocking(Xbee_Serial.base, &UART_Handle, &Read_Xfer, &bytes_read);
    }

    return bytes_read;
}

int Xbee_Serial_Tx_Free(void)
{
    return xbee_cbuf_free(Tx_Buffer);
}

int Xbee_Serial_Tx_Used(void)
{
    return xbee_cbuf_used(Tx_Buffer);
}

int Xbee_Serial_Tx_Flush(void)
{
    xbee_cbuf_flush(Tx_Buffer);
    return(0);
}

int Xbee_Serial_Rx_Free(void)
{
    return ((UART_Handle.rxRingBufferSize - 1U) - Xbee_Serial_Rx_Used());
}

int Xbee_Serial_Rx_Used(void)
{
    int size;

    if (UART_Handle.rxRingBufferTail > UART_Handle.rxRingBufferHead)
    {
        size = (UART_Handle.rxRingBufferHead + UART_Handle.rxRingBufferSize - UART_Handle.rxRingBufferTail);
    }
    else
    {
        size = (UART_Handle.rxRingBufferHead - UART_Handle.rxRingBufferTail);
    }

    return size;
}

int Xbee_Serial_Rx_Flush(void)
{
    /* TODO - add check to make sure data isn't being received */
    UART_Handle.rxRingBufferHead = UART_Handle.rxRingBufferTail;

    return(0);
}

int Xbee_Serial_Open(Xbee_Serial_T *serial)
{
    status_t status;

    if (serial == NULL)
    {
        assert(FALSE);
    }

    Xbee_Serial.base = serial->base;
    Xbee_Serial.baudrate = serial->baudrate;

    UART_GetDefaultConfig(&UART_Cfg);
    UART_Cfg.baudRate_Bps = Xbee_Serial.baudrate;
    UART_Cfg.enableTx = true;
    UART_Cfg.enableRx = true;

    Tx_Buffer = (xbee_cbuf_t *) Internal_Tx_Buffer;
    xbee_cbuf_init(Tx_Buffer, TX_BUFF_SIZE);

    status = UART_Init(Xbee_Serial.base, &UART_Cfg, CLOCK_GetFreq(kCLOCK_BusClk));
    UART_TransferCreateHandle(Xbee_Serial.base, &UART_Handle, UART_User_Callback, NULL);
    UART_TransferStartRingBuffer(Xbee_Serial.base, &UART_Handle, Working_Rx_Buffer, RX_BUFF_SIZE);

    if (kStatus_Success == status)
    {
        return 0;
    }
    else
    {
        return -EINVAL;
    }
}

void Xbee_Serial_Close(void)
{
    if (Xbee_Serial.base == NULL)
    {
        assert(FALSE);
    }

    UART_Deinit(Xbee_Serial.base);
    UART_TransferStopRingBuffer(Xbee_Serial.base, &UART_Handle);
}

void UART_User_Callback(UART_Type *base, uart_handle_t *handle, status_t status, void *userData)
{
    userData = userData;

    if (kStatus_UART_TxIdle == status)
    {
        Write_Xfer.dataSize = Xbee_Serial_Tx_Used();
        Write_Xfer.data = Working_Tx_Buffer;

        if (Write_Xfer.dataSize)
        {
            xbee_cbuf_get(Tx_Buffer, Working_Tx_Buffer, Write_Xfer.dataSize);
            UART_TransferSendNonBlocking(Xbee_Serial.base, &UART_Handle, &Write_Xfer);
        }
        else
        {
            Silence_Start = Get_Xbee_Milliseconds_Timer();
        }
    }

    if (kStatus_UART_RxIdle == status)
    {

    }
}

uint32_t Get_Time_Since_Last_Tx(void)
{
    return (Get_Xbee_Milliseconds_Timer() - Silence_Start);
}

void Start_Xbee_Reboot_Wait(void)
{
    Xbee_Reboot_Start = Get_Xbee_Milliseconds_Timer();
}

uint32_t Get_Time_Since_Xbee_Reboot(void)
{
    return (Get_Xbee_Milliseconds_Timer() - Xbee_Reboot_Start);
}

