static char sccsid[] = "@(#)57	1.4  src/bos/usr/ccs/lib/libs/privilege.c, libs, bos411, 9428A410j 6/16/90 01:12:05";
/*
 * COMPONENT_NAME: (LIBS) security library functions
 *
 * FUNCTIONS: privilege
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
#include <sys/priv.h>

/*                                                                   
*
* EXTERNAL ROUTINES: getpriv, setpriv
*
*/

/*
*
* ROUTINE PROLOG
*
* ROUTINE NAME: privilege
*
* PURPOSE: Temporarily drops or regains privilege.
*
* NOTES: 
* Version 3.1 library routine.
*
*
* RETURN VALUE DESCRIPTION: 
* None.
*
*/  
void privilege(cmd)
int cmd;	/* drop/regain privilege indicator flag */
{
	priv_t	priv;	/* privilege set buffer */

	switch(cmd)
	{
		case PRIV_LAPSE:			 /* lapse privilege */
			getpriv(PRIV_INHERITED, &priv, sizeof(priv_t));
			setpriv(PRIV_SET|PRIV_EFFECTIVE|PRIV_BEQUEATH, &priv, sizeof(priv_t));
			break;

		case PRIV_ACQUIRE:			 /* reacquire privilege */
			getpriv(PRIV_MAXIMUM, &priv, sizeof(priv_t));
			setpriv(PRIV_SET|PRIV_EFFECTIVE|PRIV_BEQUEATH, &priv, sizeof(priv_t));
			break;

		case PRIV_DROP:				 /* drop privilege */
			getpriv(PRIV_INHERITED, &priv, sizeof(priv_t));
			setpriv(PRIV_SET|PRIV_EFFECTIVE|PRIV_BEQUEATH|PRIV_MAXIMUM, &priv, sizeof(priv_t));
	}
}
