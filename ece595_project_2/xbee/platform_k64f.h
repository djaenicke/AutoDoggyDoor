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
	@file xbee/platform_hcs08.h
	Header for Freescale HCS08 platform (XBee Programmable using CodeWarrior).

	This file is automatically included by xbee/platform.h.
*/

#ifndef __XBEE_PLATFORM_K64F
#define __XBEE_PLATFORM_K64F
    #define strcmpi         strcasecmp
    #define strncmpi        strncasecmp

    // macro used to declare a packed structure (no alignment of elements)
    #define PACKED_STRUCT        struct __attribute__ ((__packed__))

    #define _f_memcpy        memcpy
    #define _f_memset        memset

    // stdint.h for int8_t, uint8_t, int16_t, etc. types
    #include <stdint.h>

	// This type isn't in stdint.h
	typedef uint8_t			bool_t;

	// the "FAR" modifier is not used
	#define FAR

	struct xbee_cbuf_t;						// defined in xbee/cbuf.h
	typedef struct xbee_serial_t {
		uint32_t					baudrate;
		uint8_t					port;
		struct xbee_cbuf_t	*rxbuf;
	} xbee_serial_t;

	// We'll use 1/1/2000 as the epoch, to match ZigBee.
	#define ZCL_TIME_EPOCH_DELTA	0

	#define XBEE_MS_TIMER_RESOLUTION 1

	extern void Init_Xbee_Timer(void);

#endif		// __XBEE_PLATFORM_K64F

