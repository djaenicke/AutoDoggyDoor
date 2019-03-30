#include "xbee\platform.h"
#include "xbee\cbuf.h"
#include "xbee\serial.h"

#define XBEE_UART         UART2
#define XBEE_UART_CLK_SRC UART2_CLK_SRC
#define XBEE_UART_STR     "UART2"

#define RX_BUFF_SIZE  255
#define TX_BUFF_SIZE  RX_BUFF_SIZE

static uart_config_t UART_Cfg;
static uart_handle_t UART_Handle;

static uart_transfer_t Write_Xfer;
static uart_transfer_t Read_Xfer;

static xbee_cbuf_t *Tx_Buffer;
static uint8_t Internal_Tx_Buffer[TX_BUFF_SIZE + XBEE_CBUF_OVERHEAD];

static uint8_t Working_Tx_Buffer[TX_BUFF_SIZE];
static uint8_t Working_Rx_Buffer[RX_BUFF_SIZE];

void UART_User_Callback(UART_Type *base, uart_handle_t *handle, status_t status, void *userData);

bool_t xbee_ser_invalid(xbee_serial_t *serial)
{
    bool_t ret_val = 0;

    if (serial->base != XBEE_UART || serial->baudrate == 0)
    {
        ret_val = 1;
    }

    return(ret_val);
}


const char *xbee_ser_portname(xbee_serial_t *serial)
{
    return(XBEE_UART_STR);
}


int xbee_ser_write(xbee_serial_t *serial, const void FAR *buffer, int length)
{
    int ret_val;

    if (xbee_ser_invalid(serial))
        return -EINVAL;

    if (length < 0)
        return -EIO;

    if (length > TX_BUFF_SIZE)
        length = TX_BUFF_SIZE;

    /* Add the new data to the circular buffer */
    ret_val = xbee_cbuf_put(Tx_Buffer, buffer, length);

    /* Check if the UART driver is already busy sending data */
    if (kUART_TxDataRegEmptyFlag & UART_GetStatusFlags(XBEE_UART))
    {
        /* Send the new data since the driver isn't busy */
        Write_Xfer.dataSize = xbee_ser_tx_used(serial);
        Write_Xfer.data = Working_Tx_Buffer;
        xbee_cbuf_get(Tx_Buffer, Working_Tx_Buffer, Write_Xfer.dataSize);
        UART_TransferSendNonBlocking(XBEE_UART, &UART_Handle, &Write_Xfer);
    }

    return (ret_val);
}

int xbee_ser_read(xbee_serial_t *serial, void FAR *buffer, int bufsize)
{
    int avail = xbee_ser_rx_used(serial);
    size_t bytes_read = 0;

    if (xbee_ser_invalid(serial))
        return -EINVAL;

    if (bufsize < 0)
        return -EIO;

    if (avail)
    {
        if (bufsize > avail)
            bufsize = avail;

        Read_Xfer.data = buffer;
        Read_Xfer.dataSize = bufsize;

        UART_TransferReceiveNonBlocking(serial->base, &UART_Handle, &Read_Xfer, &bytes_read);
    }

    return bytes_read;
}

int xbee_ser_putchar(xbee_serial_t *serial, uint8_t ch)
{
    if (xbee_ser_invalid(serial))
        return -EINVAL;

    if (xbee_cbuf_putch(Tx_Buffer, ch))
    {
        /* Check if the UART driver is already busy sending data */
        if (kUART_TxDataRegEmptyFlag & UART_GetStatusFlags(XBEE_UART))
        {
            /* Send the new data since the driver isn't busy */
            Write_Xfer.dataSize = xbee_ser_tx_used(serial);
            Write_Xfer.data = Working_Tx_Buffer;
            xbee_cbuf_get(Tx_Buffer, Working_Tx_Buffer, Write_Xfer.dataSize);
            UART_TransferSendNonBlocking(XBEE_UART, &UART_Handle, &Write_Xfer);
        }

        return 0;
    }

    return -ENOSPC;
}


int xbee_ser_getchar(xbee_serial_t *serial)
{
    int avail = xbee_ser_rx_used(serial);
    size_t bytes_read;
    uint8_t buffer[1];

    if (xbee_ser_invalid(serial))
        return -EINVAL;

    if (avail > 0)
    {
        Read_Xfer.data = buffer;
        Read_Xfer.dataSize = 1;
        UART_TransferReceiveNonBlocking(serial->base, &UART_Handle, &Read_Xfer, &bytes_read);
    }
    else
    {
        return -ENODATA;
    }

    return (int32_t) buffer[1];
}


int xbee_ser_tx_free(xbee_serial_t *serial)
{
    if (xbee_ser_invalid(serial))
        return -EINVAL;

    return xbee_cbuf_free(Tx_Buffer);
}


int xbee_ser_tx_used(xbee_serial_t *serial)
{
    if (xbee_ser_invalid(serial))
        return -EINVAL;

    return xbee_cbuf_used(Tx_Buffer);
}


int xbee_ser_tx_flush(xbee_serial_t *serial)
{
    if (xbee_ser_invalid(serial))
        return -EINVAL;

    xbee_cbuf_flush(Tx_Buffer);
    return(0);
}


int xbee_ser_rx_free(xbee_serial_t *serial)
{
    if (xbee_ser_invalid(serial))
        return -EINVAL;

    return ((UART_Handle.rxRingBufferSize - 1U) - xbee_ser_rx_used(serial));
}


int xbee_ser_rx_used(xbee_serial_t *serial)
{
    int size;

    if (xbee_ser_invalid(serial))
        return -EINVAL;

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


int xbee_ser_rx_flush(xbee_serial_t *serial)
{
    if (xbee_ser_invalid(serial))
        return -EINVAL;

    /* TODO - add check to make sure data isn't being received */
    UART_Handle.rxRingBufferHead = UART_Handle.rxRingBufferTail;

    return(0);
}


int xbee_ser_baudrate(xbee_serial_t *serial, uint32_t baudrate)
{
	return 0;
}

int xbee_ser_open(xbee_serial_t *serial, uint32_t baudrate)
{
    status_t status;

    if (xbee_ser_invalid(serial))
    {
        return -EINVAL;
    }

    UART_GetDefaultConfig(&UART_Cfg);
    UART_Cfg.baudRate_Bps = baudrate;
    UART_Cfg.enableTx = true;
    UART_Cfg.enableRx = true;

    status = UART_Init(XBEE_UART, &UART_Cfg, CLOCK_GetFreq(XBEE_UART_CLK_SRC));
    UART_TransferCreateHandle(XBEE_UART, &UART_Handle, UART_User_Callback, NULL);
    UART_TransferStartRingBuffer(XBEE_UART, &UART_Handle, Working_Rx_Buffer, RX_BUFF_SIZE);

    Tx_Buffer = (xbee_cbuf_t *) Internal_Tx_Buffer;
    xbee_cbuf_init(Tx_Buffer, TX_BUFF_SIZE);

    if (kStatus_Success == status)
    {
        serial->base = XBEE_UART;
        serial->baudrate = baudrate;
        return 0;
    }
    else
    {
        return -EINVAL;
    }
}


int xbee_ser_close(xbee_serial_t *serial)
{
    if (xbee_ser_invalid(serial))
    {
        return -EINVAL;
    }

    UART_Deinit(XBEE_UART);
    UART_TransferStopRingBuffer(XBEE_UART, &UART_Handle);

    return 0;
}


int xbee_ser_break(xbee_serial_t *serial, bool_t enabled)
{
    return 1;
}


int xbee_ser_flowcontrol(xbee_serial_t *serial, bool_t enabled)
{
	return 0;
}

int xbee_ser_set_rts(xbee_serial_t *serial, bool_t asserted)
{
	return 0;
}


int xbee_ser_get_cts(xbee_serial_t *serial)
{
    return 1;
}

void UART_User_Callback(UART_Type *base, uart_handle_t *handle, status_t status, void *userData)
{
    userData = userData;
    xbee_serial_t serial;

    serial.base = XBEE_UART;

    if (kStatus_UART_TxIdle == status)
    {
        Write_Xfer.dataSize = xbee_ser_tx_used(&serial);
        Write_Xfer.data = Working_Tx_Buffer;

        if (Write_Xfer.dataSize)
        {
            xbee_cbuf_get(Tx_Buffer, Working_Tx_Buffer, Write_Xfer.dataSize);
            UART_TransferSendNonBlocking(XBEE_UART, &UART_Handle, &Write_Xfer);
        }
    }

    if (kStatus_UART_RxIdle == status)
    {

    }
}
