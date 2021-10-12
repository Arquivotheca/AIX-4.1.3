static char sccsid[] = "@(#)02	1.18  src/bos/usr/ccs/lib/libc/getlogin.c, libcs, bos411, 9428A410j 4/20/94 17:48:10";
/*
 * COMPONENT_NAME: (LIBCS) Standard C Library Security Functions 
 *
 * FUNCTIONS: getlogin 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <utmp.h>
#include <usersec.h>
#include <userpw.h>

#ifdef	_THREAD_SAFE
#include "rec_mutex.h"
extern	struct	rec_mutex	_utmp_rmutex;
extern	struct	rec_mutex	_environ_rmutex;
#define	GETFROMCRED	(getfromcred_r(answer))
static	int		getfromcred_r(char *answer);
#define POP_N_LEAVE(val) { leave = 1; rc = (val); goto pop_n_leave; }
#else
#define	GETFROMCRED	(getfromcred(answer))
static	char		*getfromcred(char *answer);
#define POP_N_LEAVE(val) return(val)
#endif	/* _THREAD_SAFE */

#include "ts_supp.h"
#include "push_pop.h"


#ifndef	_THREAD_SAFE
char *
getlogin(void)
#else
int
getlogin_r(char *answer, size_t namelen)
#endif	/* _THREAD_SAFE */

{
register	me, uf;
register char	*name;
struct	utmp	ubuf;

#ifndef	_THREAD_SAFE
static	char	answer[PW_NAMELEN];
#endif	/* _THREAD_SAFE */

register int	siz;
	int rc, leave = 0;

#ifdef	_THREAD_SAFE
	if ((answer == NULL) || (namelen <= 0))
	{
		errno=EINVAL;
		return (errno);
	}
#endif	/* _THREAD_SAFE */

	/*
	 * get ttyslot #
	 */

	if ((me = ttyslot()) < 0)
		return (GETFROMCRED);

	/*
	 * open /etc/utmp
	 */
	TS_LOCK(&_utmp_rmutex);
	TS_PUSH_CLNUP(&_utmp_rmutex);

	if((uf = open(UTMP_FILE, 0)) < 0)
	{
		POP_N_LEAVE(GETFROMCRED);
	}

	/*
	 * seek to the point were this user's information is
	 */

	(void) lseek(uf, (long)(me * sizeof(ubuf)), 0);

	/*
	 * read from the file
	 */

	if(read(uf, (char*)&ubuf, sizeof(ubuf)) != sizeof(ubuf))
	{
		(void) close(uf);
		POP_N_LEAVE(GETFROMCRED);
	}

	(void) close(uf);

pop_n_leave:
	TS_POP_CLNUP(0);
	TS_UNLOCK(&_utmp_rmutex);
	if (leave)		/* leave is set by POP_N_LEAVE(x) to 1        */
		return (rc);	/* rc is set by POP_N_LEAVE(x) to the value x */

	/*
	 * see if user entry is NULL or type is DEAD_PROCESS
	 */
	if(ubuf.ut_user[0] == '\0' || ubuf.ut_type == DEAD_PROCESS)
		return (GETFROMCRED);

#ifdef	_THREAD_SAFE
        if (((siz = strlen (ubuf.ut_user)) + 1) > namelen)
	{
		errno=ERANGE;
		return (errno);
	}
#endif	/* _THREAD_SAFE */

	if (((siz = strlen (ubuf.ut_user)) + 1) > PW_NAMELEN)
	{
		strncpy(&answer[0],&ubuf.ut_user[0],PW_NAMELEN) ;
		siz = PW_NAMELEN - 1;
	}
	else
		strncpy(&answer[0],&ubuf.ut_user[0],siz) ;

	answer[siz] = '\0' ;

#ifndef	_THREAD_SAFE
	return(&answer[0]);
#else
	return (0);
#endif	/* _THREAD_SAFE */

}

/*
 * NAME: getfromcred()
 *                                                                    
 * FUNCTION: get LOGNAME from protected env 
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *                                                                   
 * RETURNS: NULL if not found or pointer to static character string
 *
 */  

#ifndef	_THREAD_SAFE
static char *
getfromcred(char *answer)
#else
static int
getfromcred_r(char *answer)
#endif	/* _THREAD_SAFE */
{
register char	**env;	/* return from getpenv() */
register char	*ptr;	/* temporary pointer */
register char	*val;	/* temporary pointer */
register int	i;	/* counter */
register int	siz;

	/*
	 * if the login name cannot be retrieved from 
	 * /etc/utmp get it from the protected user env
	 */
	TS_LOCK(&_environ_rmutex);

	TS_PUSH_CLNUP(&_environ_rmutex);
	env = (char **)getpenv(PENV_SYS);
	TS_POP_CLNUP(0);

	if (env == NULL) 
	{ 
		TS_UNLOCK(&_environ_rmutex);

#ifdef	_THREAD_SAFE
		return(errno);
#else
		return(NULL);
#endif	/*_THREAD_SAFE*/
	}

	TS_UNLOCK(&_environ_rmutex);

	/* 
	 * parse the LOGNAME=name string
	 */

	for (i = 0; env[i]; i++)
	{
		ptr = env[i];
		val = ptr;
		while ((*ptr != '=') && (*ptr != '\0'))
			ptr++;
		*ptr++ = '\0';
		if (!strcmp(val,"LOGNAME"))
		{
			if (((siz = strlen (ptr)) + 1) > PW_NAMELEN)
			{
				strncpy(answer,ptr,PW_NAMELEN) ;
				siz = PW_NAMELEN - 1;
			}
			else
				strncpy(answer,ptr,siz) ;

			answer[siz] = '\0';
			free(env);
#ifndef	_THREAD_SAFE
			return(answer);
#else
			return(0);
#endif	/*_THREAD_SAFE*/

		}
	}

	free(env);
#ifndef	_THREAD_SAFE
	return(NULL);
#else
	return(-1);
#endif	/*_THREAD_SAFE*/

}





