static char sccsid[] = "@(#)05	1.6  src/bos/usr/ccs/lib/libIN/AOdesc.c, libIN, bos411, 9428A410j 6/10/91 10:14:02";
/*
 * LIBIN: AOdesc
 *
 * ORIGIN: 9
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: Determine attributes of cpu's given cpu id.
 *
 * RETURN VALUE DESCRIPTION: 
 */

#include <gpoff.h>

static AOdesc cpuchar[] = {
#ifdef A_I8086
	{"i8086/88",A_I8086,AO_sep,       2,2,16,  0,      0x10000},
#endif
#ifdef    A_Z8K2
	{"z8002",   A_Z8K2, AO_sep,       2,2,2048,0,      0},
#endif
#ifdef     A_AIWS
	{"aiws",    A_AIWS, AO_shr,       4,4,2048,0x10000000,0},
#endif
#ifdef   A_I80186
	{"i80186",  A_I80186, AO_sep,     2,2,16,  0,      0x10000},
#endif
#ifdef   A_I80286
	{"i80286",  A_I80286, AO_sep,     2,2,16,  0,      0x10000},
#endif
#ifdef   A_I80386
	{"i80386",  A_I80386, AO_sep,     2,2,16,  0,      0x10000},
#endif
	{0,         0,      0,            0,0,0,   0,      0}};

AOdesc *AOgetdesc(cpu) register cpu; {
    register AOdesc *cp;
    for (cp=cpuchar; cp->id!=0; cp++) if (cpu==cp->id) return(cp);
    return(0);}

