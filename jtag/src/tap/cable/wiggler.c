/*
 * $Id$
 *
 * Macraigor Wiggler JTAG Cable Driver
 * Copyright (C) 2002, 2003 ETC s.r.o.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 * Documentation:
 * [1] http://www.ocdemon.net/
 * [2] http://jtag-arm9.sourceforge.net/hardware.html
 *
 */

#include <sys/io.h>

#include "cable.h"
#include "state.h"

/*
 * data D[7:0] (pins 9:2)
 */
#define	TDI	3
#define	TCK	2
#define	TMS	1
#define	TRST	0

/*
 * 7 - BUSY (pin 11)
 * 6 - ACK (pin 10)
 * 5 - PE (pin 12)
 * 4 - SEL (pin 13)
 * 3 - ERROR (pin 15)
 */
#define	TDO 	7	

static unsigned int port;

static int
wiggler_init( unsigned int aport )
{
	tap_state_init();
	port = aport;
	if (((port + 2 <= 0x400) && ioperm( port, 2, 1 )) || ((port + 2 > 0x400) && iopl( 3 )))
		return 0;
	tap_state_set_trst( (inb( port ) >> TRST) & 1 );

	return 1;
}

static void
wiggler_done( void )
{
	if (port + 2 <= 0x400)
		ioperm( port, 2, 0 );
	else
		iopl( 0 );

	tap_state_done();
}

static void
wiggler_clock( int tms, int tdi )
{
	int trst = tap_state_get_trst();

	tms &= 1;
	tdi &= 1;

	outb( (trst << TRST) | (0 << TCK) | (tms << TMS) | (tdi << TDI), port );
	cable_wait();
	outb( (trst << TRST) | (1 << TCK) | (tms << TMS) | (tdi << TDI), port );
	cable_wait();

	tap_state_clock( tms );
}

static int
wiggler_get_tdo( void )
{
	outb( (tap_state_get_trst() << TRST) | (0 << TCK), port );
	cable_wait();
	return ((inb( port + 1 ) ^ 0x80) >> TDO) & 1;		/* BUSY is inverted */
}

static void
wiggler_set_trst( int new_trst )
{
	tap_state_set_trst( new_trst );
	outb( (new_trst & 1) << TRST, port );
}

cable_driver_t wiggler_cable_driver = {
	"WIGGLER",
	"Macraigor Wiggler JTAG Cable",
	wiggler_init,
	wiggler_done,
	wiggler_clock,
	wiggler_get_tdo,
	wiggler_set_trst
};