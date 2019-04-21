/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "board.h"
#include "fsl_device_registers.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "app_supervisor.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
* Prototypes
******************************************************************************/


/*******************************************************************************
* Variables
******************************************************************************/


/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
int main(void)
{
    SYSMPU_Type *base = SYSMPU;
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitBootPeripherals();
    BOARD_InitDebugConsole();
    BOARD_Enable_SW_Interrupts();

    /* Disable SYSMPU. */
    base->CESR &= ~SYSMPU_CESR_VLD_MASK;

    Start_App_Supervisor();

    return 0;
}

