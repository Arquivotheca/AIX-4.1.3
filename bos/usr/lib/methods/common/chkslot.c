static char sccsid[] = "@(#)79	1.8  src/bos/usr/lib/methods/common/chkslot.c, cfgmethods, bos411, 9428A410j 3/2/91 16:17:15";
/*
 * COMPONENT_NAME: (CFGMETH) chkslot
 *
 * FUNCTIONS: chkslot()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include <fcntl.h>
#include <cf.h>		/* Error codes */
#include <sys/types.h>
#include <sys/mdio.h>
#include <sys/eu.h>

#include "cfgdebug.h"

/*
 *  NAME: chkslot
 * 
 *  FUNCTION:
 *      Return zero value if cardid is in desired slot.
 * 
 *  INPUTS:
 *      parent  - The name of the parent device, i.e. bus0 or eu0.
 *      slot    - The slot number from the parent connection descriptor.
 *                It should be a value of 1 through 8, with 0 being used
 *                for the Standard I/O Planar.
 *      cardid  - The card Id composed as ((POS0<<8) || POS1).
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int
chkslot(parent, slot, cardid)
char *parent;
int slot;
ushort cardid;
{
	MACH_DD_IO mddRecord;		/* struct used for bus query */
	struct euidcb euid;		/* struct required for eu ioctl	*/
	uchar pos[2];
	int fd;
	int i;

	DEBUG_3 ("chkslot: Checking for card id %hu in slot %d of %s\n",cardid,slot,parent)
	/* Decrement slot number found in database */
	if (slot == 0)
		/* Checking Standard I/O Planar */
		slot = 15;
	else
		/* convert slot number to what is used by hardware */
		slot--;

	if (0 > (fd = open(parent, O_RDWR)))
		return E_NODETECT;

	if (strncmp(parent,"/dev/bus",8) == 0) {
	/* Adpater is on bus */

		pos[0] = 0xff;
		pos[1] = 0xff;

		mddRecord.md_size = 1; 
		mddRecord.md_incr = MV_BYTE;
		mddRecord.md_data = &pos[0];      
		mddRecord.md_addr = POSREG(0, slot);

		if (0 > ioctl(fd, MIOCCGET, &mddRecord))
			return E_NODETECT;

		mddRecord.md_data = &pos[1];      
		mddRecord.md_addr = POSREG(1, slot);

		if (0 > ioctl(fd, MIOCCGET, &mddRecord))
			return E_NODETECT;

		close(fd);

		if (cardid != ((pos[0] << 8) | pos[1]))
			return E_NODETECT;
	}
	else if (strncmp(parent,"/dev/eu",7) == 0) {
	/* Adapter in expansion unit */

		euid.slot = (uchar)slot;
		if (ioctl(fd, EU_GETID, &euid) < 0) {
			DEBUG_2("error in getting card id in slot %d of %s\n",slot,parent)
			return E_NODETECT;
		}

		close(fd);

		if (cardid != euid.id)
			return E_NODETECT;
	}
	else 
		return E_NODETECT;
return 0;
}
