#ifndef _IO_ABSTRACTION_H_
#define _IO_ABSTRSCTION_H_

#include <stdint.h>

#include "fsl_port.h"
#include "fsl_gpio.h"

typedef enum
{
    LOW=0,
    HIGH=1,
    NA
} GPIO_State_T;

typedef struct
{
    PORT_Type *pbase;
    GPIO_Type *gbase;
    uint32_t   pin;
    port_mux_t mux;
    gpio_pin_direction_t dir;
    GPIO_State_T init_state;
} Pin_Cfg_T;

typedef enum
{
    BLUE_LED=0,
    SW_2,
    SW_3,
    UART3_RX,
    UART3_TX,
	ACTUATOR,
	RED_LED,
	GREEN_LED,
	USS_TRIGGER,
	USS_ECHO,
    NUM_IO /* !!! Make sure this is last */
} IO_Map_T;

extern const Pin_Cfg_T Pin_Cfgs[NUM_IO];

extern void Set_GPIO(IO_Map_T gpio, GPIO_State_T state);
extern uint32_t Read_GPIO(IO_Map_T gpio);

#endif /* _IO_ABSTRACTION_H_ */
