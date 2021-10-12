static char sccsid[] = "@(#)33  1.14  src/bos/usr/bin/que/fullname.c, cmdque, bos411, 9428A410j 4/17/91 10:14:02";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
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

#include	<IN/standard.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/mntctl.h>
#include	<sys/fullstat.h>
#include	<sys/stat.h>
#include	<string.h>
#include	<errno.h>
#include	<stdio.h>
#include	<unistd.h>
#include	<time.h>
#include	"common.h"
#include <ctype.h>

#include <nl_types.h>
#include "enq_msg.h"
#define MAXSTR 		10
#define MSGSTR(num,str)	catgets(catd,MS_ENQ,num,str)
nl_catd	catd;



/*
* fullname - translate the file name into a fully qualified file name
*	without any extraneous ".." edges.
*/

char * fullname(path,fsp)
char *path;
struct fullstat *fsp;
{
	char dirname[PATH_MAX ];
	char *newpath, *xdir(), *ptr;
	char *from, *to;
	struct fullstat fsbuf;
	int done;

	if( !fsp )
		fsp = &fsbuf;

	if( fullstat(path,FL_STAT,fsp) < 0 )
		return((char *) 0);

	if( S_ISDIR(fsp->st_mode) )
	{
		return(xdir(path));
	}
	if( *path == '/' )
	{
		newpath =  Qalloc( PATH_MAX );
		strcpy(newpath,path);
	}
	else
	{
                if ((newpath = getcwd((char *)NULL,PATH_MAX)) == NULL)
                        syserr((int)EXITFATAL,"getcwd");
 
		strcat(newpath,"/");
		strcat(newpath,path);
	}

	/* find the last "/" in the newpath */
	for( ptr = &newpath[strlen(newpath)-1]; ptr != newpath; ptr-- )
		if( *ptr == '/' )
			break;

	/*
	* The assumption that all files have names like
	* DIRNAME/FILENAME breaks down with files in the
	* root directory.  If the search for the last
	* "/" finds only the one at the front, then
	* return just the new path.
	*/
	if( ptr == newpath )
		return(newpath);

	/*
	* If the newpath contains no extraneous ".." edges
	* then, there is no need to translate further.
	*/
	for( done = 1, from = newpath; *from; from++ )
		if( *from == '.' && *(from+1) == '.' )
		{
			done = 0;
			break;
		}

	if( done )
		return(newpath);

	/* copy the newpath into the directory name buffer */

	for( from = newpath, to=dirname; from != ptr; )
		*to++ = *from++;

	*to = '\0';

	to = xdir(dirname);
	strcat(to,"/");
	strcat(to,ptr+1);	/* add the final edge */
	return(to);
}

char *
xdir(name)
char *name;
{
	char *ptr, *buf;
	int pi[2], pid, stat, i, n;

	ptr =  Qalloc( PATH_MAX );
	buf = ptr;

	if( pipe(pi) < 0 )
		syserr((int)EXITFATAL,MSGSTR(MSGPIPE,"Unable to create pipe"));

	for( i = 0; i < 8; i++ )
	{
/*		switch( pid = regfork() ) */ /* from old wait.c */
		switch( pid = fork() )
		{
		case -1:
			sleep((unsigned)(i+1));
			errno = 0;
			break;
		case 0:
			close(pi[0]);
			chdir(name);
		        ptr = getcwd((char *)NULL,PATH_MAX);
			write(pi[1],ptr,(unsigned)(strlen(ptr)+1));
			close(pi[1]);
			_exit(EXITOK);
		default:
			close(pi[1]);
			for( ; (n = read(pi[0],ptr,(unsigned)(PATH_MAX -i))) > 0; ptr += n)
				;

/*			while( pidwait(pid,&stat) != pid ) */ /* from old wait.c */
			while( waitpid(pid,&stat,0) != pid )
				;
			return(buf);
		}
	}
	return(NULL);
}
