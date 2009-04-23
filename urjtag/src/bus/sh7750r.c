/*
 * $Id$
 *
 * Copyright (C) 2003 ETC s.r.o.
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
 * Written by Matan Ziv-Av <matan@svgalib.org>, 2003.
 * Modified by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#include "sysdep.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "part.h"
#include "bus.h"
#include "chain.h"
#include "bssignal.h"
#include "jtag.h"
#include "buses.h"
#include "generic_bus.h"

typedef struct
{
    urj_part_signal_t *a[26];
    urj_part_signal_t *d[64];
    urj_part_signal_t *cs[7];
    urj_part_signal_t *we[4];
    urj_part_signal_t *rdwr;
    urj_part_signal_t *rd;
    urj_part_signal_t *rdwr2;
    urj_part_signal_t *rd2;
} bus_params_t;

#define A       ((bus_params_t *) bus->params)->a
#define D       ((bus_params_t *) bus->params)->d
#define CS      ((bus_params_t *) bus->params)->cs
#define WE      ((bus_params_t *) bus->params)->we
#define RDWR    ((bus_params_t *) bus->params)->rdwr
#define RD      ((bus_params_t *) bus->params)->rd
#define RDWR2   ((bus_params_t *) bus->params)->rdwr2
#define RD2     ((bus_params_t *) bus->params)->rd2

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
sh7750r_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
                 char *cmd_params[])
{
    urj_bus_t *bus;
    urj_part_t *part;
    char buff[10];
    int i;
    int failed = 0;

    bus = calloc (1, sizeof (urj_bus_t));
    if (!bus)
        return NULL;

    bus->driver = driver;
    bus->params = calloc (1, sizeof (bus_params_t));
    if (!bus->params)
    {
        free (bus);
        return NULL;
    }

    bus->chain = chain;
    bus->part = part = chain->parts->parts[chain->active_part];

    for (i = 0; i < 26; i++)
    {
        sprintf (buff, "A%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(A[i]), buff);
    }

    for (i = 0; i < 32; i++)
    {
        sprintf (buff, "D%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(D[i]), buff);
    }

    for (i = 0; i < 7; i++)
    {
        sprintf (buff, "CS%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(CS[i]), buff);
    }

    for (i = 0; i < 8; i++)
    {
        sprintf (buff, "WE%d", i);
        failed |= urj_bus_generic_attach_sig (part, &(WE[i]), buff);
    }

    failed |= urj_bus_generic_attach_sig (part, &(RDWR), "RDWR");

    failed |= urj_bus_generic_attach_sig (part, &(RD), "RD");

    failed |= urj_bus_generic_attach_sig (part, &(RDWR2), "RDWR2");

    failed |= urj_bus_generic_attach_sig (part, &(RD2), "RD2");

    if (failed)
    {
        free (bus->params);
        free (bus);
        return NULL;
    }

    return bus;
}

/**
 * bus->driver->(*printinfo)
 *
 */
static void
sh7750r_bus_printinfo (urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    printf (_
            ("Hitachi SH7750R compatible bus driver via BSR (JTAG part No. %d)\n"),
            i);
}

/**
 * bus->driver->(*area)
 *
 */
static int
sh7750r_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    area->description = NULL;
    area->start = UINT32_C (0x00000000);
    area->length = UINT64_C (0x100000000);
    area->width = 32;

    return URJ_STATUS_OK;
}

static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < 26; i++)
        urj_part_set_signal (p, A[i], 1, (a >> i) & 1);
}

static void
set_data_in (urj_bus_t *bus)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < 32; i++)
        urj_part_set_signal (p, D[i], 0, 0);
}

static void
setup_data (urj_bus_t *bus, uint32_t d)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < 32; i++)
        urj_part_set_signal (p, D[i], 1, (d >> i) & 1);
}

/**
 * bus->driver->(*read_start)
 *
 */
static void
sh7750r_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    int cs[8];
    int i;

    for (i = 0; i < 8; i++)
        cs[i] = 1;
    cs[(adr & 0x1C000000) >> 26] = 0;

    urj_part_set_signal (p, CS[0], 1, cs[0]);
    urj_part_set_signal (p, CS[1], 1, cs[1]);
    urj_part_set_signal (p, CS[2], 1, cs[2]);
    urj_part_set_signal (p, CS[3], 1, cs[3]);
    urj_part_set_signal (p, CS[4], 1, cs[4]);
    urj_part_set_signal (p, CS[5], 1, cs[5]);
    urj_part_set_signal (p, CS[6], 1, cs[6]);
    urj_part_set_signal (p, RDWR, 1, 1);
    urj_part_set_signal (p, RDWR2, 1, 1);
    urj_part_set_signal (p, WE[0], 1, 1);
    urj_part_set_signal (p, WE[1], 1, 1);
    urj_part_set_signal (p, WE[2], 1, 1);
    urj_part_set_signal (p, WE[3], 1, 1);
    urj_part_set_signal (p, RD, 1, 0);
    urj_part_set_signal (p, RD2, 1, 0);

    setup_address (bus, adr);
    set_data_in (bus);
    urj_tap_chain_shift_data_registers (bus->chain, 0);
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
sh7750r_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    int i;
    uint32_t d = 0;

    setup_address (bus, adr);
    urj_tap_chain_shift_data_registers (bus->chain, 1);

    for (i = 0; i < 32; i++)
        d |= (uint32_t) (urj_part_get_signal (p, D[i]) << i);

    return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
sh7750r_bus_read_end (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;
    int cs[8];
    int i;
    uint32_t d = 0;

    for (i = 0; i < 8; i++)
        cs[i] = 1;

    urj_part_set_signal (p, CS[0], 1, cs[0]);
    urj_part_set_signal (p, CS[1], 1, cs[1]);
    urj_part_set_signal (p, CS[2], 1, cs[2]);
    urj_part_set_signal (p, CS[3], 1, cs[3]);
    urj_part_set_signal (p, CS[4], 1, cs[4]);
    urj_part_set_signal (p, CS[5], 1, cs[5]);
    urj_part_set_signal (p, CS[6], 1, cs[6]);

    urj_part_set_signal (p, RD, 1, 1);
    urj_part_set_signal (p, RD2, 1, 1);
    urj_tap_chain_shift_data_registers (bus->chain, 1);

    for (i = 0; i < 32; i++)
        d |= (uint32_t) (urj_part_get_signal (p, D[i]) << i);

    return d;
}

/**
 * bus->driver->(*write)
 *
 */
static void
sh7750r_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    urj_chain_t *chain = bus->chain;
    urj_part_t *p = bus->part;
    int cs[8];
    int i;

    for (i = 0; i < 8; i++)
        cs[i] = 1;
    cs[(adr & 0x1C000000) >> 26] = 0;

    urj_part_set_signal (p, CS[0], 1, cs[0]);
    urj_part_set_signal (p, CS[1], 1, cs[1]);
    urj_part_set_signal (p, CS[2], 1, cs[2]);
    urj_part_set_signal (p, CS[3], 1, cs[3]);
    urj_part_set_signal (p, CS[4], 1, cs[4]);
    urj_part_set_signal (p, CS[5], 1, cs[5]);
    urj_part_set_signal (p, CS[6], 1, cs[6]);

    urj_part_set_signal (p, RDWR, 1, 0);
    urj_part_set_signal (p, RDWR2, 1, 0);
    urj_part_set_signal (p, WE[0], 1, 1);
    urj_part_set_signal (p, WE[1], 1, 1);
    urj_part_set_signal (p, WE[2], 1, 1);
    urj_part_set_signal (p, WE[3], 1, 1);
    urj_part_set_signal (p, RD, 1, 1);
    urj_part_set_signal (p, RD2, 1, 1);

    setup_address (bus, adr);
    setup_data (bus, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal (p, WE[0], 1, 0);
    urj_part_set_signal (p, WE[1], 1, 0);
    urj_part_set_signal (p, WE[2], 1, 0);
    urj_part_set_signal (p, WE[3], 1, 0);

    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal (p, WE[0], 1, 1);
    urj_part_set_signal (p, WE[1], 1, 1);
    urj_part_set_signal (p, WE[2], 1, 1);
    urj_part_set_signal (p, WE[3], 1, 1);

    urj_tap_chain_shift_data_registers (chain, 0);
}

const urj_bus_driver_t sh7750r_bus = {
    "sh7750r",
    N_("Hitachi SH7750R compatible bus driver via BSR"),
    sh7750r_bus_new,
    urj_bus_generic_free,
    sh7750r_bus_printinfo,
    urj_bus_generic_prepare_extest,
    sh7750r_bus_area,
    sh7750r_bus_read_start,
    sh7750r_bus_read_next,
    sh7750r_bus_read_end,
    urj_bus_generic_read,
    sh7750r_bus_write,
    urj_bus_generic_no_init
};