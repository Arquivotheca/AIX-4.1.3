static char sccsid[] = "@(#)29	1.7  src/bos/usr/ccs/lib/libc/getuinfo.c, libcs, bos411, 9428A410j 11/17/93 15:13:29";
/*
 * COMPONENT_NAME: (LIBCS) Standard C Library System Security Functions 
 *
 * FUNCTIONS: getuinfo 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <uinfo.h>

#define NULL    0
#ifndef	_THREAD_SAFE
char   *INuibp;        /* pointer to user info buffer to be searched   */
#endif	/* _THREAD_SAFE */

/*
 *
 * NAME:        getuinfo()
 *
 * PURPOSE:     Get the value string that is associated with
 *              the specified name from a user information buffer.
 *              The user information buffer to be searched is pointed
 *              to by the external variable INuibp.
 *              If INuibp is null, usrinfo() is called to copy user
 *              information into a local buffer.
 *
 * PARMS:       name    pointer to the name to be found
 *
 *
 * RETURNS:     pointer to value string,
 *              or NULL if name not found
 *
 * ALGORITHM:
 *
 *      if INuibp is NULL
 *              call usrinfo
 *
 *      scan for name match
 *      if name found
 *              return pointer to value string
 *      else
 *              return NULL
 */

#ifndef _THREAD_SAFE
char *
getuinfo( name )
#else
char *
/* getuinfo_r( name, char  *INuibp) 
*/
getuinfo_r( name, INuibp)
char  *INuibp;
#endif /* _THREAD_SAFE */

register char *name;                    /* pointer to name to be found  */
{
	register char *bp;              /* pointer to user info buffer  */
	register char *np;              /* pointer into name string     */

#ifdef UINFO
#ifndef	_THREAD_SAFE
	static	char	uibuf[UINFOSIZ];	/* local buffer		*/
#endif	/* _THREAD_SAFE */

	/* if buffer pointer is null, call usrinfo to get user info     */
	if ( ( bp = INuibp ) == NULL )
#ifdef	_THREAD_SAFE
		return (NULL);
	if ( usrinfo( GETUINFO, INuibp, UINFOSIZ ) <= 0 )
#else
	if ( usrinfo( GETUINFO, uibuf, UINFOSIZ ) <= 0 )
#endif
		return( NULL );

#ifdef _THREAD_SAFE
	bp = INuibp;
#else
	INuibp = bp = uibuf;
#endif
	while ( *bp != NULL )
	{
		np = name;
		while ( *bp == *np )
			bp++, np++;
		if ( *bp == '=' && *np == '\0' )
			return ( bp + 1 );
		while ( *bp++ != '\0' );
	}
#endif	/* UINFO */
	return ( NULL );
}
