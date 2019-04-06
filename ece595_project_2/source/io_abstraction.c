/*
 * io_abstraction.cpp
 *
 *  Created on: Feb 9, 2019
 *      Author: djaenicke
 */

#include "io_abstraction.h"
#include "assert.h"

const Pin_Cfg_T Pin_Cfgs[NUM_IO] =
{
    {PORTB, GPIOB, 21, kPORT_MuxAsGpio,           kGPIO_DigitalOutput, HIGH}, /* BLUE_LED        - Active Low       */
    {PORTC, GPIOC,  6, kPORT_MuxAsGpio,           kGPIO_DigitalInput,  NA},   /* SW_2            */
    {PORTA, GPIOA,  4, kPORT_MuxAsGpio,           kGPIO_DigitalInput,  NA},   /* SW_3            */
    {PORTD, GPIOD,  2, kPORT_MuxAlt3,             kGPIO_DigitalInput,  NA},   /* UART2_RX        */
    {PORTD, GPIOD,  3, kPORT_MuxAlt3,             kGPIO_DigitalOutput, NA},   /* UART2_TX        */
};

void Set_GPIO(IO_Map_T gpio, GPIO_State_T state)
{
    if (gpio < NUM_IO)
    {
        if ((kPORT_MuxAsGpio == Pin_Cfgs[gpio].mux) &&
            (kGPIO_DigitalOutput == Pin_Cfgs[gpio].dir))
            GPIO_PinWrite(Pin_Cfgs[gpio].gbase, Pin_Cfgs[gpio].pin, (uint8_t) state);
        else
            assert(false);
    }
    else
        assert(false);
}

uint32_t Read_GPIO(IO_Map_T gpio)
{
    uint32_t ret_val;

    if (gpio < NUM_IO)
    {
        if ((kPORT_MuxAsGpio == Pin_Cfgs[gpio].mux) &&
            (kGPIO_DigitalInput == Pin_Cfgs[gpio].dir))
            ret_val = GPIO_PinRead(Pin_Cfgs[gpio].gbase, Pin_Cfgs[gpio].pin);
        else
            assert(false);
    }
    else
        assert(false);

    return (ret_val);
}

