static char sccsid[] = "@(#)80	1.1  src/bos/kernext/jfsc/comp.kext.c, sysxjfsc, bos411, 9428A410j 3/29/94 17:31:22";
/* 
 * COMPONENT_NAME: SYSXJFSC (JFS Compression)
 *
 * FUNCTIONS:  	compress_config
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/device.h>
#include <sys/jfsc.h>

#define COMPRESS_UNCONFIGURED	0
#define COMPRESS_CONFIGURED	1
#define COMPRESS_INITIALIZING	2

extern int encode_decode(int, int, caddr_t, size_t, caddr_t, size_t);

int compress_state = COMPRESS_UNCONFIGURED;

/*
 * NAME: 	compress_config
 *
 * FUNCTION: 	Compression kernel extension configuration entry point.
 *		This routine is called by the config routine to pin 
 *		itself, and set kernel function	pointer to the 
 *		compress/decompress function.	
 *
 * RETURNS:	0 	- success
 *		EBUSY	- operation unsuccessful
 */
int
compress_config(int cmd,	/* command to config routine    */
	 struct uio *uiop)	/* ignored			*/
{
	int	old_state;      /* compress state before swap */
	int	rc;

	if (cmd == CFG_INIT)
	{
		old_state = COMPRESS_UNCONFIGURED;
		if (!compare_and_swap(&compress_state,
			&old_state, COMPRESS_INITIALIZING))
				return EBUSY; /* state in use or configuring */

		if (rc = pincode(encode_decode))
		{
			old_state = COMPRESS_INITIALIZING;
			while (!compare_and_swap(&compress_state,
				&old_state, COMPRESS_UNCONFIGURED));
			return(rc);
		}
		compp = encode_decode;	/* Compression function for kernel */

		old_state = COMPRESS_INITIALIZING;
		while (!compare_and_swap(&compress_state,
			&old_state, COMPRESS_CONFIGURED));
		return 0;
	}
	else
		return EBUSY;	/* Cannot unconfigure reliably */
}
