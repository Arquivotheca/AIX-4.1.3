static char sccsid[] = "@(#)08	1.5  src/bos/kernel/si/POWER/initmname.c, syssi, bos411, 9428A410j 10/26/90 10:37:55";
/*
 * COMPONENT_NAME: (SYSSI) System Initialization
 *
 * FUNCTIONS:	init_mname
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988 , 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/types.h>
#include <sys/vmker.h>
#include <sys/utsname.h>
#include <sys/limits.h>
#include <sys/syspest.h>
#include <sys/iplcb.h>

/*
 * NAME: init_mname()
 *
 * FUNCTION: machine name initialization
 *
 *	Initializes utsname machine name and xutsname nid with 
 *	processor serial number from IPL control block.
 *
 *
 * INPUT PARMETERS:
 *	NONE
 *
 * RETURN CODE:
 *	NONE
 *
 */

static	char hexnum[]="0123456789ABCDEF";
#define machmask 0x0000000f

/*
 * init_mname() initializes the utsname.machine field of
 * the utsname structure and xutsname.nid field of the 
 * xutsname structure.  The general format of the utsname.machine
 * field is the following:  XXYYYYYYMMSS
 * where XX - feature of operating system, this is 370 specific.
 *       YYYYYY - The cpu id.
 *       MM - model number.
 *       SS - submodel number.
 * For RS/6000, only cpu id and model number are obtainable from the
 * ipl control block.  The rest of the fields will be zero filled.
 */

void
init_mname()
{

	int	i,last;
	struct	ipl_cb		*iplcb_ptr;	/* ipl cb pointer   */
	struct	ipl_info	*info_ptr;	/* ipl info pointer */
	struct	ipl_directory	*dir_ptr;	/* ipl dir pointer  */
	unsigned int modelnum;
	
	/* get addressability to iplinfo structure.
	 */
	iplcb_ptr = (struct ipl_cb *) vmker.iplcbptr;
	dir_ptr = &(iplcb_ptr->s0);
	info_ptr = (struct ipl_info *) ((char *) iplcb_ptr +
		dir_ptr->ipl_info_offset);
	modelnum = info_ptr->model;
	
	/* copy processor serial number to utsname machine name.
	 */
	last = sizeof(info_ptr->vpd_processor_serial_number);
	copy_procid(info_ptr->vpd_processor_serial_number,utsname.machine,last);

	/* 
	 * zero out the 370 specific fields.
	 */
	utsname.machine[0] = '0';
	utsname.machine[1] = '0';


	/* set model number */
	utsname.machine[last+1] = hexnum[modelnum & machmask];
	modelnum >>= 4;
	utsname.machine[last] = hexnum[modelnum & machmask];


	/* get xutsname nid.
	 */
	utsname.machine[last+2] = NULL;
	xutsname.nid = getnid(&utsname.machine[2],8);

	/* no submodel number for RS/6000
	 */
	utsname.machine[last+2] = '0';
	utsname.machine[last+3] = '0';
	utsname.machine[last+4] = NULL;
}

static
copy_procid(src,dest,len)
char	*src;
char	*dest;
int	len;
{
	char *sptr, *dptr, *sstop;

	/* find first non-readable character in serial number.
	 */
	for (sptr = src; sptr < src + len; sptr++)
	{
		if (*sptr < ' ' || *sptr > '\176' )
			break;
	}

	/* copy the processor id. the processor id will be shifted
	 * down and '0' padded on the left (MSD) if a non-readable
	 * charater exists within the source string (source string
	 * ends at len or at first non-readable).
	 */
	for (dptr = dest, sstop = src + len; dptr < dest + len; dptr++)
	{
		if (sptr < sstop)
		{
			*dptr = '0';
			sptr++;
		}	
		else
		{
			*dptr = *src++;
		}

	}
}

/*
 * getnid(str,len)
 *
 * converts utsname machine (hex character string) to xutsname nid
 * (unsigned long).
 */

getnid(str,len)
char 	*str;
int 	len;
{
	char *cptr;
	unsigned nval, scale, value;
	
	nval = 0;
	scale = 1;
	
	for (cptr = str + len - 1; cptr >= str; cptr--, scale *= 16)
	{

		if (*cptr >= '0' && *cptr <= '9')
		{
			value = *cptr - '0';
		}
		else
		{
			assert(*cptr >= 'A' && *cptr <= 'F');
			value = (*cptr - 'A') + 10;
		}

		nval +=  value * scale;
	}
	return(nval);
}
