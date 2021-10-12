static char sccsid[] = "@(#)17	1.10  src/bos/usr/ccs/lib/libs/tcb.c, libs, bos411, 9428A410j 6/16/90 01:14:08";
/*
 * COMPONENT_NAME: (LIBS) security library functions
 *
 * FUNCTIONS: tcb
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

#include	"sys/stat.h"
#include	"sys/errno.h"
#include	"sys/priv.h"
#include	"sys/mode.h"
#include	"sys/tcb.h"

int 
tcb (path, cmd)
char	*path;
int	cmd;
{
	struct 	pcl 	pclbuf, *pclp = &pclbuf;
	extern	int	errno;
	int	pclsize;
	int	tcb;
	int	rc;
	int	len;		/* global length of the pcl structure	*/
	extern 	char	*malloc();

	tcb = rc = 0;
	len	= PCL_SIZ;
	if (statpriv (path, 0, pclp, len) == -1) {
		/*
		 * if length of static pcl structure is too small, then
		 * malloc more memory.
		 */

		len = pclp->pcl_len;
		if ( errno != ENOSPC )
			return (-1);
		if ( !( pclp =( struct pcl *)malloc( len )))
			return -1;
		if (statpriv (path, 0, pclp, len) == -1)
			return -1;
	}
		

	tcb = pclp->pcl_mode & (S_ITCB | S_ITP) ;
	
	switch (cmd) {
	    case TCB_ON:	
		pclp->pcl_mode |= S_ITCB;
		rc = chpriv (path, pclp, len);
		break;
	    case TP_ON:
		pclp->pcl_mode |= (S_ITP | S_ITCB);
		rc = chpriv (path, pclp, len);
		break;
	    case TCB_OFF:
		pclp->pcl_mode &= (~(S_ITCB | S_ITP));
		rc = chpriv (path, pclp, len);
		break;
	    case TCB_QUERY:
		/*
		 * enter here with rc = TCB_OFF = 0
		 */
		if ( tcb & S_ITCB )
			rc = TCB_ON;
		if ( tcb & S_ITP )
			rc = TP_ON;
		break;

	    default:
		errno = EINVAL;		/* invalid command */
		rc = -1;
		break;
	}
	if ( pclp != &pclbuf )
		free( pclp );
	return (rc);
}
