static char sccsid[] = "@(#)34	1.2  src/bos/usr/lib/methods/chggen/chggen.c, cfgmethods, bos411, 9428A410j 6/15/90 16:51:59";
/* 
 * COMPONENT_NAME : (CFGMETH) Generic check_parms routine for most devices
 *
 * FUNCTIONS : 	check_parms
 * 
 * ORIGINS : 27 
 * 
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights -  Use, Duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "pparms.h"

/*
 * NAME     : check_parms 
 *
 * FUNCTION : This function checks the validity of the attributes. 
 *
 * EXECUTION ENVIRONMENT :
 *	This function is called by the change_device function to  
 *	check the validity of the attributes of the device
 * 
 * NOTES :
 *	For most devices this generic "always succeed" check will suffice.
 *	
 * RETURNS : Returns  0 on success ALWAYS.
 */

int check_parms(attrs, Pflag, Tflag, logical_name, parent, location, badattr)
struct attr *attrs ;
int	Pflag;		/* if Pflag == 1 then the -P flag was passed */
			/*             0 then the -P flag was NOT passed */
int	Tflag;		/* if Tflag == 1 then the -T flag was passed */
			/*             0 then the -P flag was NOT passed */
char	*logical_name;	/* logical name of device being changed */
char	*parent;	/* parent(logical name) of device, NULL if not changed*/
char	*location;	/* location of device, NULL if not changed*/
char	*badattr;	/* Place to report list of bad attributes */
{
	return(0);
}
