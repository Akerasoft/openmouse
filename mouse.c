/* Original Work:
 * Nes/Snes/N64/Gamecube to Wiimote
 * Copyright (C) 2012 Raphaël Assénat
 * Changes (C) 2023 Akerasoft
 * Programmer for Akerasoft: Robert Kolski
 * New work undecided but at least NES/SNES/SNES Mouse, other features may be cut.
 *
 * Based on earlier work:
 *
 * Nes/Snes/Genesis/SMS/Atari to USB
 * Copyright (C) 2006-2007 Raphaël Assénat
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * The author may be contacted at raph@raphnet.net
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "gamepads.h"
#include "mouse.h"

#define GAMEPAD_BYTES	4

/******** IO port definitions **************/
#define MOUSE_LATCH_DDR	DDRC
#define MOUSE_LATCH_PORT	PORTC
#define MOUSE_LATCH_BIT	(1<<1)

#define MOUSE_CLOCK_DDR	DDRC
#define MOUSE_CLOCK_PORT	PORTC
#define MOUSE_CLOCK_BIT	(1<<0)

#define MOUSE_DATA_PORT	PORTC
#define MOUSE_DATA_DDR	DDRC
#define MOUSE_DATA_PIN	PINC
#define MOUSE_DATA_BIT	(1<<2)

/********* IO port manipulation macros **********/
#define MOUSE_LATCH_LOW()	do { MOUSE_LATCH_PORT &= ~(MOUSE_LATCH_BIT); } while(0)
#define MOUSE_LATCH_HIGH()	do { MOUSE_LATCH_PORT |= MOUSE_LATCH_BIT; } while(0)
#define MOUSE_CLOCK_LOW()	do { MOUSE_CLOCK_PORT &= ~(MOUSE_CLOCK_BIT); } while(0)
#define MOUSE_CLOCK_HIGH()	do { MOUSE_CLOCK_PORT |= MOUSE_CLOCK_BIT; } while(0)

#define MOUSE_GET_DATA()	(MOUSE_DATA_PIN & MOUSE_DATA_BIT)

/*********** prototypes *************/
static char mouseInit(void);
static char mouseUpdate(void);


// the most recent bytes we fetched from the controller
static unsigned char last_read_controller_bytes[GAMEPAD_BYTES];
// the most recently reported bytes
static unsigned char last_reported_controller_bytes[GAMEPAD_BYTES];

static char nes_mode = 0;
static char mouse_mode = 0;

static char mouseInit(void)
{
	unsigned char sreg;
	sreg = SREG;
	cli();

	// clock and latch as output
	MOUSE_LATCH_DDR |= MOUSE_LATCH_BIT;
	MOUSE_CLOCK_DDR |= MOUSE_CLOCK_BIT;

	// data as input
	MOUSE_DATA_DDR &= ~(MOUSE_DATA_BIT);
	// enable pullup. This should prevent random toggling of pins
	// when no controller is connected.
	MOUSE_DATA_PORT |= MOUSE_DATA_BIT;

	// clock is normally high
	MOUSE_CLOCK_PORT |= MOUSE_CLOCK_BIT;

	// LATCH is Active HIGH
	MOUSE_LATCH_PORT &= ~(MOUSE_LATCH_BIT);

	mouseUpdate();

	SREG = sreg;

	return 0;
}


/*
 *
       Clock Cycle     Button Reported
        ===========     ===============
        1               B
        2               Y
        3               Select
        4               Start
        5               Up on joypad
        6               Down on joypad
        7               Left on joypad
        8               Right on joypad
        9               A
        10              X
        11              L
        12              R
        13              none (always high)
        14              none (always high)
        15              none (always high)
        16              none (always high) (gamepad)
		
		16              (low mouse instead of gamepad)
        17              Y direction (0=up, 1=down)
        18              Y motion bit 6
        19              Y motion bit 5
        20              Y motion bit 4
        21              Y motion bit 3
        22              Y motion bit 2
        23              Y motion bit 1
        24              Y motion bit 0
        25              X direction (0=left, 1=right)
        26              X motion bit 6
        27              X motion bit 5
        28              X motion bit 4
        29              X motion bit 3
        30              X motion bit 2
        31              X motion bit 1
        32              X motion bit 0
		
 *
 */

static char mouseUpdate(void)
{
	int i;
	unsigned char tmp=0;

	MOUSE_LATCH_HIGH();
	_delay_us(12);
	MOUSE_LATCH_LOW();

	for (i=0; i<8; i++)
	{
		_delay_us(6);
		MOUSE_CLOCK_LOW();

		tmp <<= 1;
		if (!MOUSE_GET_DATA()) { tmp |= 0x01; }

		_delay_us(6);

		MOUSE_CLOCK_HIGH();
	}
	last_read_controller_bytes[0] = tmp;
	for (i=0; i<8; i++)
	{
		_delay_us(6);

		MOUSE_CLOCK_LOW();

		tmp <<= 1;
		if (!MOUSE_GET_DATA()) { tmp |= 0x01; }

		_delay_us(6);
		MOUSE_CLOCK_HIGH();
	}
	last_read_controller_bytes[1] = tmp;
	for (i=0; i<8; i++)
	{
		_delay_us(6);

		MOUSE_CLOCK_LOW();

		tmp <<= 1;
		if (!MOUSE_GET_DATA()) { tmp |= 0x01; }

		_delay_us(6);
		MOUSE_CLOCK_HIGH();
	}
	last_read_controller_bytes[2] = tmp;
	for (i=0; i<8; i++)
	{
		_delay_us(6);

		MOUSE_CLOCK_LOW();

		tmp <<= 1;
		if (!MOUSE_GET_DATA()) { tmp |= 0x01; }

		_delay_us(6);
		MOUSE_CLOCK_HIGH();
	}
	last_read_controller_bytes[3] = tmp;


	return 0;
}

static char mouseChanged(void)
{
	return memcmp(last_read_controller_bytes,
					last_reported_controller_bytes, GAMEPAD_BYTES);
}

static void mouseGetReport(gamepad_data *dst)
{
	unsigned char h, l;

	if (dst != NULL)
	{
		l = last_read_controller_bytes[0];
		h = last_read_controller_bytes[1];

		// The 4 last bits are always high if an SNES controller
		// is connected. With a NES controller, they are low.
		// (High on the wire is a 0 here due to the mouseUpdate() implementation)
		//
		if ((h & 0x0f) == 0x0f) {
			nes_mode = 1;
			mouse_mode = 0;
		} else if ((h & 0xf) == 0x0e) {
			nes_mode = 0;
			mouse_mode = 1;		
		} else {
			nes_mode = 0;
			mouse_mode = 0;
		}

		if (nes_mode) {
			// Nes controllers send the data in this order:
			// A B Sel St U D L R
			dst->nes.pad_type = PAD_TYPE_NES;
			dst->nes.buttons = l;
			dst->nes.raw_data[0] = l;
		} else if (mouse_mode) {
			dst->mouse.pad_type = PAD_TYPE_MOUSE;
			dst->mouse.buttons = l;
			dst->mouse.buttons |= h<<8;
			dst->mouse.raw_data[0] = l;
			dst->mouse.raw_data[1] = h;
			dst->mouse.raw_data[2] = last_read_controller_bytes[2];
			dst->mouse.raw_data[3] = last_read_controller_bytes[3];
		} else {
			dst->snes.pad_type = PAD_TYPE_SNES;
			dst->snes.buttons = l;
			dst->snes.buttons |= h<<8;
			dst->snes.raw_data[0] = l;
			dst->snes.raw_data[1] = h;
		}
	}
	memcpy(last_reported_controller_bytes,
			last_read_controller_bytes,
			GAMEPAD_BYTES);
}

static Gamepad MouseGamepad = {
	.init		= mouseInit,
	.update		= mouseUpdate,
	.changed	= mouseChanged,
	.getReport	= mouseGetReport
};

Gamepad *mouseGetGamepad(void)
{
	return &MouseGamepad;
}

