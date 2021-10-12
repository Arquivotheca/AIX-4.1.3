static char sccsid[] = "@(#)98  1.14.1.4  src/bos/usr/lib/methods/cfgbus/busquery.c, cmdbuscf, bos411, 9428A410j 3/29/94 13:19:57";
/*
 *   COMPONENT_NAME: (CMDBUSCF) Bus query module
 *
 *   FUNCTIONS: busquery
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <fcntl.h>
#include <sys/mdio.h>
#include <sys/types.h>

#include "bcm.h"

/*
 * NAME: busquery 
 *
 * FUNCTION: Walk Micro-channel bus. 
 *
 * NOTES: 
 *
 * RETURN VALUE DESCRIPTION: -1 : Failure 0 : Success 
 *
 */

busquery(cardtbl, busize, bus, phase)

ushort	*cardtbl;
int	busize;
char	*bus;
int	phase;

{
	MACH_DD_IO	mddRecord;
	uchar		pos[2];
	int		fd;
	int		i;


	if ((fd = open(bus, O_RDWR)) == -1)
		return(-1);

	mddRecord.md_size = 2;	/* build mdd record */
	mddRecord.md_incr = MV_BYTE;
	mddRecord.md_data = pos;/* addr of data */

	/* Discover what slots have cards */
	for (i = 0; i < busize; i++) {

		/* Initialize table entry for no adapter */
		cardtbl[i] = EMPTYSLOT;

		/* Set LEDs for current slot */
		if (phase)
			setleds(0x500 + i);

		/* Read card id from slot */
		mddRecord.md_addr = POSREG(0, (0 == i ? 15 : i - 1));
		if (ioctl(fd, MIOCCGET, &mddRecord) == -1) {
			continue;
		}
		cardtbl[i] = (pos[0] << 8) | pos[1];

		/* Write to trace file */
		if (prntflag) {
			fprintf(trace_file,"slot %2d = %04x\n",i,cardtbl[i]);
		}
	}

	close(fd);
	return(0);
}
