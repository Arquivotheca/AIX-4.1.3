static char sccsid[] = "@(#)58	1.8.1.3  src/bos/usr/bin/que/libque/args.c, cmdque, bos411, 9428A410j 2/4/94 17:56:43";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: dumpargs, addflag, add2uflist, addfile1, addfile
 *
 * ORIGINS: 9, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/* args.c
	This routine handles user flags and filenames that go in the jdf
	the static struct uflags contains this junk.
*/

#include <stdio.h>
#include <IN/standard.h>
#include <sys/types.h>
#include "common.h"
#include "enq.h"
#include <ctype.h>

#include "libq_msg.h"


char *copyfile();
char *scopy();
char *fullname();


struct uflags *uflags;


/* Dump all saved filenames and unrecognized arguments to a file */
dumpargs(fil)
FILE *fil;
{
	register struct uflags  *ufp;
	for (ufp = uflags; ufp; ufp = ufp->next)
	{
		fputs(ufp->flag,fil);
		putc('\n',fil);
	}
}

/* stuff user flags into a buffer and save them.  */
/* backend options are prefaced with the Backend OPTion MARKer, (BOPTMARK)
   so that qdaemon can recognize them as being different.		
   BOPTMARK and BOPTLEN are set in common.h to some weird unusual character
   string and its length respectively(clf)
*/
addflag(str)
register char *str;
{
	char * zappa;

	zappa =  Qalloc(strlen(str)+1+BOPTLEN);
	strcpy(zappa,BOPTMARK);
	strcpy(zappa+BOPTLEN,str);
	add2uflist(zappa);
}


/*ADD To User Flag LIST*/
/* stuff user flags into a buffer and save them.  */
add2uflist(str)
register char *str;
{
	register struct uflags *thisuf, *ufp;

	thisuf =  Qalloc(sizeof(struct uflags));
	thisuf->flag =  Qalloc(strlen(str)+1);
	strcpy(thisuf->flag,str);
	thisuf->next = NULL;

	if (!uflags)
		uflags = thisuf;
	else
	{
		for (ufp = uflags; ufp->next; ufp = ufp->next);
			ufp->next = thisuf;
	}
}



/* stuff user filename into a buffer and save it.  */
addfile1(f,del)
char * f;
int del;
{
	char * newf;
	char buf[MAXLINE];

	if ( newf=fullname(f,NULL) )
	{
		sprintf(buf,"%s %d",
			newf,		/* full path name*/
			del);		/* Delete after backend dies?*/
		add2uflist(buf);
	}
	else
		syswarn(GETMESG(MSGFORM,"Filename format is bad."));
}


char * addfile(str,copying,qe)
char * str;
boolean copying;
register struct qe *qe;
{
	static char *nam=NULL;
	int del=FALSE;		/* delete after backend dies? */

	/*Copy if necessary. nam is the REAL name, relative to this directory*/

	if (copying == TRUE || str == NULL )
	{
		nam = copyfile(str,qe);		/* null str reads stdin*/

		qe->qe_tmp = TRUE;	/* turn on tmp flag */

		/* if user gave "-rm" flag, we can remove the original now. */
		if ((qe->qe_rm))
			if (str) unlink(str);
		del = CP_FLAG;	/* STDIN or -c - remove after backend dies */
	}
	else
	{
		nam = scopy(str);
		qe->qe_tmp = FALSE;	/* turn off tmp flag */
		if ((qe->qe_rm))
			del = RM_FLAG;	/* -r - remove iff backend completes OK */
	}


	addfile1(nam,del);

	return(nam);
}

