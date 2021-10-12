/* @(#)22  1.2  src/bos/usr/bin/copydump/copydump.h, cmddump, bos411, 9428A410j 2/14/94 09:09:08 */
/*
 * COMPONENT_NAME: CMDDUMP
 *
 * FUNCTIONS: copydump
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define COPY_SUCCESS	0
#define E_USAGE		1
#define E_USAGE		1
#define E_ODMINIT	2
#define E_ODMGET	3
#define E_IMPORTVG	4
#define E_VARYONVG	5
#define E_OPEN		6
#define E_LSEEK		7
#define E_READ		8
#define E_INVALID_DUMP	9
#define E_GET_DEVNAME	10
#define E_MKDEV		11
#define E_COPY		12
#define E_ODMMSG	13

#define COPY_DUMP_HELP \
"\t\tHelp for Copy a System Dump\n\n\
A system dump consists of a hexadecimal dump of the system data\n\
as it existed before the system crash.  This information can be\n\
copied to the output device that you specify.\n\n\
Select an output device from the list displayed on the screen\n\
and insert the appropriate media.  Type the number indicating\n\
the device selected and press Enter."

#define COPY_DUMP_HEADING \
"\t\tCopy a System Dump to Removable Media\n\n\
    The system dump is %d bytes and will be copied from %s\n\
    to media inserted into the device from the list below.\n\n\
    Please make sure that you have sufficient blank, formatted\n\
    media before you continue.\n\n\
    Step One:  Insert blank media into the chosen device.\n\
    Step Two:  Type the number for that device and press Enter."


