#include "xbee\platform.h"

bool_t xbee_ser_invalid(xbee_serial_t *serial)
{
	return 0;
}


const char *xbee_ser_portname(xbee_serial_t *serial)
{

}


int xbee_ser_write(xbee_serial_t *serial, const void FAR *buffer,int length)
{

}

int xbee_ser_read(xbee_serial_t *serial, void FAR *buffer, int bufsize)
{
	int retval;

	return retval;
}


int xbee_ser_putchar( xbee_serial_t *serial, uint8_t ch)
{
	int retval;

    return retval;

}


int xbee_ser_getchar( xbee_serial_t *serial)
{
	int retval;

	return retval;
}


int xbee_ser_tx_free(xbee_serial_t *serial)
{
	return INT_MAX;
}


int xbee_ser_tx_used(xbee_serial_t *serial)
{
	return 1;
}


int xbee_ser_tx_flush(xbee_serial_t *serial)
{
	return 0;
}


int xbee_ser_rx_free(xbee_serial_t *serial)
{
    return INT_MAX;
}


int xbee_ser_rx_used(xbee_serial_t *serial)
{
    return 0;
}


int xbee_ser_rx_flush(xbee_serial_t *serial)
{
    return 1;
}


int xbee_ser_baudrate(xbee_serial_t *serial, uint32_t baudrate)
{
	return 0;
}

int xbee_ser_open(xbee_serial_t *serial, uint32_t baudrate)
{

}


int xbee_ser_close(xbee_serial_t *serial)
{
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
