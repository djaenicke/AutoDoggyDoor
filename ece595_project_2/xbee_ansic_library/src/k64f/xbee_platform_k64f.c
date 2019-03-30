/*
 * Copyright (c) 2010-2012 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */
/**
	@addtogroup hal_hcs08
	@{
	@file xbee_platform_hcs08.c
	Platform-specific functions for use by the XBee Driver on Freescale HCS08
	platform.

	Note that on the Programmable XBee, you need to call InitRTC() from main
	and assign vRTC to the interrupt in the project's .PRM file.
*/

#include "xbee/platform.h"

// These function declarations have extra parens around the function name
// because we define them as macros referencing the global directly.
uint32_t (xbee_seconds_timer)( void)
{
	return 0;
}

uint32_t (xbee_millisecond_timer)( void)
{
	return 0;
}
