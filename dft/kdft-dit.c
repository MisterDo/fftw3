/*
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* $Id: kdft-dit.c,v 1.12 2004-10-24 05:18:14 stevenj Exp $ */

#include "ct.h"

void X(kdft_dit_register)(planner *p, kdftw codelet, const ct_desc *desc)
{
     X(regsolver_ct_directw)(p, codelet, desc, DECDIT);
     X(regsolver_ct_directwbuf)(p, codelet, desc, DECDIT);
}
