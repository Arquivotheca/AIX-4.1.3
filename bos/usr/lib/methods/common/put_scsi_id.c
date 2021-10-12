static char sccsid[] = "@(#)78	1.4  src/bos/usr/lib/methods/common/put_scsi_id.c, cfgmethods, bos411, 9428A410j 5/13/93 17:21:34";
/*
 *   COMPONENT_NAME: (CFGMETHODS)
 *
 *   FUNCTIONS: put_scsi_id
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* header files needed for compilation */
#include <stdio.h>
#include <cf.h>		/* Error codes */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mdio.h>
#include <sys/nvdd.h>
#include "cfgdebug.h"

/*
 * NAME: put_scsi_id
 *                                                                    
 * FUNCTION: this functions updates nvram location for scsi_id in the
 *	specified slot if it is different from the one passed.
 *                                                                    
 * EXECUTION ENVIRONMENT: This is a library function used by scsi config
 *	and change methods to write the card_scsi_id value into the NVRAM.
 *
 * DATA STRUCTURES:
 *	mdd : structure passed to ioctl cmd.
 *
 * INPUTS: slotno, scsi_id
 *
 * RETURNS: 0 if success else >0 Error code.
 *
 */
int
put_scsi_id(slot, id, bus_num)
uchar	slot;
uchar	id;
int     bus_num;
{
uchar		tid;
MACH_DD_IO	mdd;
int		fd,old_sid;

	/* Decrement slot number found in database */
	slot--;
	if((fd = open("/dev/nvram", O_RDWR, 0)) == -1) return E_OPEN;

        DEBUG_1("bus_num = %d\n", bus_num)

	mdd.md_addr = SCSI_BASE_ADDR + slot;
	mdd.md_data = &tid;
	mdd.md_size = 1;
	mdd.md_incr = MV_BYTE;
        /* get the byte in NVRAM corresponding to the given slot */
	if(ioctl(fd,MIONVGET,&mdd) != -1) {
           old_sid = tid;
           /* store the new scsi id in either the upper or lower nibble */
           /* of the byte based on the given bus_num                    */
           if(bus_num == 0) { /* bus0 : store id in lower nibble */
              tid &= 0xf0;
           }
           else {               /* bus1 : store id in upper nibble */
              tid &= 0x0f;
              id <<= 4;
           } 
           tid |= id;
           /* only write to NVRAM if the new id is different than that */
           /* which was already in memory                              */
           if(tid != old_sid) 
              if(ioctl(fd,MIONVPUT,&mdd) == -1) {
                  close(fd);
                  return (E_DEVACCESS);
              }
           close(fd);
           return 0;
      }
      else /* ioctl failed */ {
         close(fd); 
         return (E_DEVACCESS);
      }
}
