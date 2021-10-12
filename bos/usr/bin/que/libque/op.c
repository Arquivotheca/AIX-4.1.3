static char sccsid[] = "@(#)65	1.12  src/bos/usr/bin/que/libque/op.c, cmdque, bos411, 9428A410j 12/9/93 15:49:54";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: opmsg, getopmsg
 *
 * ORIGINS: 9, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/param.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <IN/standard.h>

#include "enq.h"
#include "common.h"

#include "libq_msg.h"


struct opm
{
	struct opm *	next;
	char *		msg;
};

struct op
{
	int		type;
	int		lines;
	union
	{
		char *		m;
		struct opm *	o;
	} u;
} op;

opmsg(type,arg)
int type;
char *arg;
{
	struct opm *new, *last;
	FILE *mfp;
	char *aline;
	int messages;

	op.type = type;
	last = NULL;

	switch( type )
	{
	case ARGM0:
		op.u.m = arg;
		op.lines = 1;
		while( *arg )
		{
			if( *arg++ == '\n' )
			{
				op.lines++;
			}
		}
		break;
	case ARGM1:
		if( (mfp = fopen(arg,"r")) == NULL)
		{
			syswarn(GETMESG(MSGOMSG,"Can't open message file %s."), arg);
			break;
		}
		for( messages=0; ; messages++ )
		{
			aline =  Qalloc(MAXLINE);
			if( fgets(aline,MAXLINE,mfp) == NULL )
			{
				free((void *)aline);
				break;
			}
			new =  Qalloc(sizeof(struct opm));
			new->next = NULL;
			new->msg = aline;

			if( last != NULL )
				last->next = new;
			else	/* empty list */
				op.u.o = new;

			last = new;
		}
		op.lines = messages;
		fclose(mfp);
	default:
		break;
	}
}

getopmsg(fp)
FILE *fp;
{
	register struct opm *opm;
	register int i;

	/* output the number of lines of operator messages first */

	fprintf(fp,"%d\n",op.lines);
	switch( op.type )
	{
	case ARGM0:
		fprintf(fp,"* %s\n", op.u.m);
		break;
	case ARGM1:
		/*
		* Since the input data was fetched with fgets
		* which retains the newline at the end of the
		* string, we don't need to put a newline when
		* we output the string.
		*/
		for( opm = op.u.o, i = 0; opm; opm = opm->next, i++ )
			fprintf(fp,"* %s", opm->msg);

		if( i != op.lines )
			syserr((int)EXITFATAL,"getopmsg");	/* sanity */

		break;
	default:
		break;
	}
}

getopmsg_pall(mbuf)
char *mbuf;
{
	register struct opm *opm;
	register int i;
	register int indx = 0;
	register int count;

	/*
	* Since the input data was fetched with fgets
	* which retains the newline at the end of the
	* string, we don't need to put a newline when
	* we output the string. Since we are writing
	* to a fixed length buffer, the message will
	* be truncated to 100 lines.
	*/
	for( opm = op.u.o, i = 0; opm && (i<100); opm = opm->next, i++ ) {
		count = sprintf(&mbuf[indx],"* %s", opm->msg);
		indx += count;
	}

	if( i != op.lines )
		syserr((int)EXITFATAL,"getopmsg");	/* sanity */

}
