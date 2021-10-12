static char sccsid[] = "@(#)88	1.1  src/bos/kernext/rif/rif_init.c, sysxrif, bos411, 9428A410j 2/25/91 17:22:31";
/*
 * COMPONENT_NAME: (SYSXINTF) raw interface services 
 *
 * FUNCTIONS: config_rif  
 *
 * ORIGINS: 26, 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#include <sys/types.h>
#include <sys/uio.h>		/* for uio struct */
#include <sys/device.h>
#include <sys/domain.h>		/* for struct domain */
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/socket.h>

#include <net/if.h>		/* include befure devinfo.h */
#include <net/netisr.h>
#include <net/netopt.h>

#include <sys/devinfo.h>	/* for DD_EN */

int netrif_kmid;		/* kernel module id, must be "mod name"_kmid */
int netrif_count = 0;		/* # of times CFG_INIT has been done */

extern	struct domain rifdomain;
extern	struct ifqueue rifintrq;
extern  rifintr();

/* these functions will be imported from kernel socket stuff */
extern  add_input_type(), add_domain_af();

/*
 * config_rif
 *
 * entry point for netrif kernel extension.
 *
 * RETURNS:
 *	0	no problemo.
 *	EINVAL	invalid parameter of some sort (go figure).
 */
config_rif(cmd, uio)
	int cmd;
	struct uio *uio;
{
	int		err;

	if (uio)
		uiomove((caddr_t) &netrif_kmid, sizeof(netrif_kmid), 
			UIO_WRITE, uio);

	switch (cmd) {
	case CFG_INIT:
		netrif_count++;
		
		if (netrif_count == 1) {
			add_domain_af(&rifdomain);
			
		/* pin the code	*/
		if (err = pincode(config_rif))
			return(err);
			
		}
		break;
	case CFG_TERM:
		netrif_count--;
		if (netrif_count == 0) { /* delete the table entries here ? */
			if (err = unpincode(config_rif))
				return(err);
			;
		}
		break;
	default:
		return(EINVAL);
	}
	
	return(0);
}
