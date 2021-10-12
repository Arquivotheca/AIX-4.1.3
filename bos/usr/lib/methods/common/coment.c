static char sccsid[] = "@(#)54  1.15  src/bos/usr/lib/methods/common/coment.c, cfgmethods, bos411, 9428A410j 11/8/93 10:39:27";
/*
 * COMPONENT_NAME: (CFGMETHODS) Common functions for ethernet adpaters
 *
 * FUNCTIONS:   get_bnc_select
 *              put_wire_type
 *              check_magic_num
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <cf.h>		/* Error codes */

#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <fcntl.h>
#include <sys/mdio.h>
#include <sys/types.h>
#include <sys/comio.h>

#include "pparms.h"
#include "cfgdebug.h"


#define         RM_BOOT_BASE_ADDR       0x0a00100
#define         RM_BOOT_COMMO_AREA      RM_BOOT_BASE_ADDR+2
#define         RM_BOOT_VAL_ID          0x7273
#define         SET_BNC         0x01
#define         SET_DIX         0x02
#define         SET_TWP         0x03


/*
 *  Name:  get_bnc_select()
 *
 *  Function:
 *    Gets the wire type setting out of NVRAM based upon the
 *    slot number and the bus ID and convert to attribute bnc_select.
 *    The value of bnc_select is:
 *              = 0  if wire type = "dix"
 *              = 1  if wire type = "bnc"
 *              = 2  if wire type = "twisted pair"
 *
 *  Returns:
 *    Returns a 0 upon success and one of the following error
 *    codes:  E_OPEN or E_DEVACCESS
 */
int
get_bnc_select(bus_num, slot, bnc_select)
	int                             bus_num, slot;
	int                             *bnc_select;
{
	MACH_DD_IO              mdd;
	int                     fd;
	uchar                   wire_type=0;

	DEBUG_2("get_bnc_select: bus_num = %d, slot = %d\n",bus_num,slot)
	/*
	 *  Open nvram for reading/writing.
	 */
	if((fd = open("/dev/nvram",O_RDWR,0)) == -1)
		return(E_OPEN);

	/*
	 *  Get the ethernet wire type.
	 */
	mdd.md_addr = RM_BOOT_COMMO_AREA+slot;
	mdd.md_size = 1;
	mdd.md_data = &wire_type;
	mdd.md_incr = MV_BYTE;

	if(ioctl(fd,MIONVGET,&mdd) == -1) {
		close(fd);
		return(E_DEVACCESS);
	}
	DEBUG_1("get_bnc_select(): NVRAM wire type is %x\n", wire_type)

	/*
	 *  Get the wire type for the specified bus number.
	 */
	if (bus_num == 0)
		wire_type = ((wire_type >> 2) & 0x03);
	else
		wire_type = ((wire_type >> 6) & 0x03);


	/*
	 *  Set bnc_select to the correct value based on wire_type.
	 */
	switch(wire_type)
	{
		case SET_BNC:
			*bnc_select = 1;
			break;
		case SET_DIX:
			*bnc_select = 0;
			break;
		case SET_TWP:
			*bnc_select = 2;
			break;
		default:
			*bnc_select = -1;
	}

	close(fd);
	return(0);

} /* End of get_bnc_select() */

/*
 *  Name:  put_wire_type()
 *
 *  Function:
 *    Sets the wire type in NVRAM based upon the
 *    slot number, the bus ID, and the input bnc_select value.
 *    If the input bnc_select is 0 then the wire type will be
 *    set to DIX and if the input bnc_select is 1 then the wire
 *    type is set to BNC.
 *    If input bnc_select = 2, then wire type will be set to Twisted Pair
 *
 *  Returns:
 *    Returns a 0 upon success and one of the following error
 *    codes:  E_OPEN or E_DEVACCESS or E_ARGS
 */
int
put_wire_type(bus_num, slot, bnc_select)
	int                             bus_num, slot;
	int                             bnc_select;
{
	MACH_DD_IO              mdd;
	int                     fd;
	uchar                   wire_type=0;
	uchar                   old_wire_type=0;

	DEBUG_3("put_wire_type(): bus_num = %x, slot = %d, bnc_select = %d\n",
		bus_num, slot,bnc_select)
	/*
	 *  Set wire type value based off of the input bnc_select value.
	 */
	switch(bnc_select)
	{
		case 0:
			wire_type |= SET_DIX;
			break;
		case 1:
			wire_type |= SET_BNC;
			break;
		case 2:
			wire_type |= SET_TWP;
			break;
		default:
			return(E_ARGS);
	}

	/*
	 *  Open nvram for read/write access
	 */
	if ((fd = open("/dev/nvram",O_RDWR,0)) == -1)
		return(E_OPEN);

	/*
	 *  Get the current value of the the wire_type for that slot
	 *  for both bus id's.
	 */
	mdd.md_addr = RM_BOOT_COMMO_AREA + slot;
	mdd.md_size = 1;
	mdd.md_data = &old_wire_type;
	mdd.md_incr = MV_BYTE;

	if(ioctl(fd,MIONVGET,&mdd) == -1) {
		close(fd);
		return(E_DEVACCESS);
	}
	DEBUG_1("put_wire_type(): old_wire_type is %x\n", old_wire_type)

	/*
	 *  Check to see if the wire type being put is different then what
	 *  is already in NVRAM.
	 */
	if (bus_num == 0) {
		if (wire_type == (old_wire_type >> 2) & 0x03)
			return(0);
	} else
		if (wire_type == (old_wire_type >> 6) & 0x03)
			return(0);

	/*
	 *  Merge the new wire type into information for the old wire type
	 */
	/* mask off old information for this bus preserving info
	 * for other bus
	 */
	if (bus_num == 0) {
		wire_type <<= 2;
		old_wire_type &= ~0x0c;
	}
	else {
		wire_type <<= 6;
		old_wire_type &= ~0xc0;
	}

	wire_type |= old_wire_type;
	DEBUG_1("put_wire_type(): wire type is %x\n", wire_type)

	/*
	 *  Write new ethernet wire type.
	 */
	mdd.md_data = &wire_type;

	if(ioctl(fd,MIONVPUT,&mdd) == -1) {
		close(fd);
		return(E_DEVACCESS);
	}

	close(fd);
	return(0);

} /* End of put_wire_type() */

/*
 *  Name:  check_magic_num()
 *
 *  Function:
 *    Checks to see if the magic number for the Ethernet NVRAM area has
 *    been written to the first two bytes of the Ethernet NVRAM area.
 *
 *  Returns:
 *    Returns a 0 upon success and one of the following error
 *    codes:  E_OPEN, E_DEVACCESS, and -1 if the magic number doesn't match
 */
int
check_magic_num()
{
	MACH_DD_IO              mdd;
	int                     fd;
	short int               magic_no;

	DEBUG_0("check_magic_num(): start of routine\n")
	/*
	 *  Open nvram for reading/writing.
	 */
	if ((fd = open("/dev/nvram",O_RDWR,0)) == -1)
		return(E_OPEN);

	/*
	 *  Get upper half-word of magic number
	 */
	mdd.md_addr = RM_BOOT_BASE_ADDR;
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
	if (magic_no != RM_BOOT_VAL_ID) {
		close(fd);
		return(-1);
	}

	close(fd);
	return(0);

} /* End of check_magic_num() */
