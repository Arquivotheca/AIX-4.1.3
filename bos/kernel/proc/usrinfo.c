static char sccsid[] = "@(#)48	1.7.1.2  src/bos/kernel/proc/usrinfo.c, sysproc, bos411, 9428A410j 2/4/94 15:01:30";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: getuinfo
 *		setuinfo
 *		usrinfo
 *		
 *
 *   ORIGINS: 27,3
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/errno.h>      /* definitions of system error codes            */
#include <uinfo.h>          /* user info system call definitions            */
#include <sys/user.h>       /* process private U structure                  */
#include <sys/priv.h>       /* priviledges stuff			    */
#include <sys/trchkid.h>    /* trace hooks				    */
#include <sys/malloc.h> 
#include <sys/audit.h>    

/*
 *
 * NAME:        usrinfo()
 *
 * PURPOSE:     Common entry point for setting and getting the process'
 *              user info string
 *
 * INPUT:       command code, buffer pointer, and xfer count
 *
 * RETURNS:     0=Good, -1=u_error set on error.
 *
 * ALGORITHM:
 *
 *      switch on command code
 *
 *              case SETUINFO: call setuinfo()
 *
 *              case GETUINFO: call getuinfo()
 *
 *              default: set u_error = EINVAL
 */

usrinfo(int cmd, char *buf, int count)
/* int cmd;		Get/Set user information*/
/* char *buf;		buffer to use		*/
/* int count;		# bytes to return or set*/
{
	int retval;
	static int svcnum = 0;

	TRCHKT_SYSC(USRINFO);
	switch(cmd)
	{
	case SETUINFO:
		if(audit_flag && audit_svcstart("PROC_Environ", &svcnum, 0)){
			if(buf){
	                        char *ptr;
	                        int len;
	
	                        if((ptr = malloc(count)) == NULL){
	                                u.u_error = ENOMEM;
					retval = -1;
					return(retval);
	                        }
	                        if(copyinstr(buf, ptr, count, &len)){
	                                u.u_error = EFAULT;
					retval = -1;
					return(retval);
	                        }
                        	audit_svcbcopy(ptr, len);
                        	free(ptr);
			}
			audit_svcfinis();
		}
		retval = setuinfo(buf,count);
		break;

	case GETUINFO:
		retval = getuinfo(buf,count);
		break;

	default:
		u.u_error = EINVAL;       /* illegal command code         */
		retval = -1;
		break;
	}
	return(retval);
}

/*
 *
 * NAME:        setuinfo()
 *
 * FUNCTION:    Set user information in user structure.
 *
 * INPUT:       command code, buffer address, and xfer count
 *
 * RETURNS:     u.u_error set on error.
 *
 * ALGORITHM:
 *
 *      check that current user has the correct priviledge
 *      check that count is not larger than UINFOSIZ;
 *      check accessability of user buffer;
 *      extract user name from user info buffer;
 *      copy info from user buffer to U structure;
 *
 */
int
setuinfo(char *buf,int count)
{
	if (privcheck(SET_PROC_ENV) == EPERM)    /* check for permission */
	{
		u.u_error = EPERM;
		return(-1);
	}

	if (count > UINFOSIZ)
	{
		u.u_error = EINVAL;
		return(-1);
	}

	bzero(u.u_uinfo, UINFOSIZ);
	/* copy uinfo string to U block                                 */
	if (copyin((caddr_t)buf, (caddr_t)u.u_uinfo, count) != 0)
	{
		u.u_error = EFAULT;
		return(-1);
	}

	return(count);
}

/*
 *
 * NAME:        getuinfo()
 *
 * function:    Get user information from user structure.
 *
 * INPUT:       command code, buffer address, and xfer count
 *
 * OUTPUT:      u_error set on error.
 *
 * ALGORITHM:
 *
 *      if count larger than UINFOSIZ
 *              set it to UINFOSIZ
 *      count to end of uinfo string
 *      check accessability of user buffer
 *      copy info from U structure to user buffer
 */
int
getuinfo(char *buf, int count)
{
	register char  *cp;             /* pointer to current character */
	register int    n;              /* uinfo string char count      */
	register int    prevnull;       /* null char previously encountered */
	register int    ucount;         /* number of chars to move      */

	if ( (ucount = count) > UINFOSIZ)
		ucount = UINFOSIZ;

	/* count to end of uinfo string                                 */
	for ( prevnull = FALSE, n = 1, cp = u.u_uinfo;
	      n < ucount; n++ )
	{
		if ( *cp++ == '\0' )
		{
			if ( prevnull )         /* two successive nulls */
				break;
			else
				prevnull = TRUE;
		}
		else
			prevnull = FALSE;
	}

	if (copyout((caddr_t)u.u_uinfo, (caddr_t)buf, n) != 0)
	{
		u.u_error = EFAULT;
		return(-1);
	}
	return(n);
}

