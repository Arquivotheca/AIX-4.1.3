static char sccsid[] = "@(#)55	1.15  src/bos/usr/lib/methods/common/comtok.c, sysxtok, bos411, 9428A410j 5/13/93 17:19:23";
/*
 *   COMPONENT_NAME: (CFGMETHODS) Configure Method for token ring adpaters
 *
 *   FUNCTIONS: check_magic_num
 *		get_ring_speed
 *		put_ring_speed
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


#include <sys/cfgodm.h>
#include <stdio.h>
#include <cf.h>
#include <fcntl.h>
#include <sys/cfgdb.h>
#include <sys/mdio.h>
#include <sys/types.h>
#include <sys/nvdd.h>

#include "cfgdebug.h"

#define		TR_MAGIC_NUM	0x7273
#define		SET_4MB		0x01
#define		SET_16MB	0x02

/*
 *  Macro to get attributes out of database.
 */
static struct attr_list	*alist=NULL;/* PdAt attribute list                 */
int		how_many;	/* Used by getattr routine.            */
int		byte_cnt;	/* Byte count of attributes retrieved  */

/*
 *  Name:  get_ring_speed()
 *
 *  Function: 
 *    Gets the ring speed setting out of NVRAM based upon the
 *    slot number and the bus ID.  The value of out_speed is
 *    set to 0 if the ring speed is 4MB and it is set to 1
 *    if the ring speed is 16MB.
 *
 *  Returns:  
 *    Returns a 0 upon success and one of the following error
 *    codes:  E_OPEN or E_DEVACCESS
 */
int
get_ring_speed(bus_num, slot, out_speed)
	int				bus_num, slot;
	int				*out_speed;
{
	MACH_DD_IO		mdd;
	int			fd;
	uchar			ring_speed=0;

	DEBUG_2("bus_num = %d, slot = %d\n",bus_num,slot)
	/*
	 *  Open nvram for reading/writing.
	 */
	if((fd = open("/dev/nvram",O_RDWR,0)) == -1)
	 	return(E_OPEN);

	/*
	 *  Get the token ring speed.
	 */
	mdd.md_addr = TR_BASE_ADDR+slot;
	mdd.md_size = 1;
	mdd.md_data = &ring_speed;
	mdd.md_incr = MV_BYTE;

	if(ioctl(fd,MIONVGET,&mdd) == -1) {
		close(fd);
		return(E_DEVACCESS);
	}
	DEBUG_1("get_ring_speed(): ring speed is %x\n", ring_speed)

	/*
	 *  Get the ring speed for the specified bus number.
	 */
	if (bus_num == 0)
		ring_speed &= 0x0f;
	else
		ring_speed >>= 4;


	/*
	 *  Set out_speed to the correct value based on ring_speed.
	 */
	switch(ring_speed)
	{
		case SET_16MB:
			*out_speed = 1;
			break;
		case SET_4MB:
			*out_speed = 0;
			break;
		default:
			*out_speed = -1;
	}

        close(fd);	
	return(0);

} /* End of get_ring_speed() */

/*
 *  Name:  put_ring_speed()
 *
 *  Function: 
 *    Sets the ring speed in NVRAM based upon the
 *    slot number, the bus ID, and the input ring speed value.  
 *    If the input ring speed is 0 then the ring speed will be
 *    set to 4MB and if the input ring speed is 1 then the ring 
 *    speed is set to 16MB.
 *
 *  Returns:  
 *    Returns a 0 upon success and one of the following error
 *    codes:  E_OPEN or E_DEVACCESS
 */
int 
put_ring_speed(bus_num, slot, in_speed)
	int				bus_num, slot;
	int				in_speed;
{
	MACH_DD_IO		mdd;
	int			fd;
	uchar			ring_speed=0;
	uchar           	old_ring_speed=0;

	DEBUG_3("bus_num = %x, slot = %d, in_speed = %d\n",bus_num, slot,
		in_speed)
	/*
	 *  Set ring speed value based off of the input speed value.
	 */
	if (in_speed == 0) 
		ring_speed |= SET_4MB;
	else
		if (in_speed == 1)
			ring_speed |= SET_16MB;

	/*
	 *  Open nvram for read/write access
	 */
	if ((fd = open("/dev/nvram",O_RDWR,0)) == -1)
	 	return(E_OPEN);

	/*
	 *  Get the current value of the the ring speeds for that slot
	 *  for both bus id's.
	 */
	mdd.md_addr = TR_BASE_ADDR + slot;
	mdd.md_size = 1;
	mdd.md_data = &old_ring_speed;
	mdd.md_incr = MV_BYTE;

	if(ioctl(fd,MIONVGET,&mdd) == -1) {
		close(fd);
		return(E_DEVACCESS);
	}
	DEBUG_1("put_ring_speed(): old_ring_speed is %x\n", old_ring_speed)

	/*
	 *  Check to see if the ring speed being put is different then what
	 *  is already in NVRAM.
	 */
	if (bus_num == 0) {
		if (ring_speed == old_ring_speed & 0x0f)
			return(0);
	} else 
		if (ring_speed == old_ring_speed >> 4)
			return(0);

	/*
	 *  Merge the new ring speed into information for the old ring speed
	 */
	if (bus_num == 0)
		old_ring_speed &= ~0x0f;
	else {
		ring_speed <<= 4;
		old_ring_speed &= 0x0f;
	}

	ring_speed |= old_ring_speed;
	DEBUG_1("put_ring_speed(): ring speed is %x\n", ring_speed)

	/*
	 *  Write new token ring speed.
	 */
	mdd.md_data = &ring_speed;

	if(ioctl(fd,MIONVPUT,&mdd) == -1) {
		close(fd);
		return(E_DEVACCESS);
	}

        close(fd);
	return(0);

} /* End of put_ring_speed() */

/*
 *  Name:  check_magic_num()
 *
 *  Function: 
 *    Checks to see if the magic number for the Token Ring NVRAM area has
 *    been written to the first two bytes of the token ring NVRAM area.
 *
 *  Returns:  
 *    Returns a 0 upon success and one of the following error
 *    codes:  E_OPEN, E_DEVACCESS, and -1 if the magic number doesn't match
 */
int
check_magic_num()
{
	MACH_DD_IO		mdd;
	int			fd;
	short int		magic_no;

	/*
	 *  Open nvram for reading/writing.
	 */
	if ((fd = open("/dev/nvram",O_RDWR,0)) == -1)
	 	return(E_OPEN);

	/*
	 *  Get upper half-word of magic number
	 */
	mdd.md_addr = TR_BASE_ADDR-2;
	mdd.md_size = 2;
	mdd.md_data = (char *) &magic_no;
	mdd.md_incr = MV_BYTE;

	if(ioctl(fd,MIONVGET,&mdd) == -1) {
		close(fd);
		return(E_DEVACCESS);
	}
	DEBUG_1("check_magic_num(): magic_no is %x\n", magic_no)

	/*
	 *  Check to verify that the magic number matches.
	 */
	if (magic_no != TR_MAGIC_NUM) {
		close(fd);
		return(-1);
	}

        close(fd);
	return(0);

} /* End of check_magic_num() */

