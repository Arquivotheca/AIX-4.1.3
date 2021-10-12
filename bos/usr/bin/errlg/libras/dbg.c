static char sccsid[] = "@(#)85	1.1  src/bos/usr/bin/errlg/libras/dbg.c, cmderrlg, bos411, 9428A410j 3/2/93 08:58:21";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: _Debug, debuginit
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Debug printf routines to file "Btrace"
 */

#include <stdio.h>
#include <libras.h>

int Debugflg;
FILE *Debugfp;

static linecount;
static wrapcount;

debuginit(filename)
char *filename;
{

	if(filename == 0)
		filename = "Btrace";
	if(Debugfp) {
		fclose(Debugfp);
		Debugfp = 0;
	}
	if((Debugfp = fopen(filename,"w+")) == NULL) {
		perror(filename);
		return(-1);
	}
	linecount = 0;
	wrapcount = 0;
	Debugflg++;
	return(1);
}

_Debug(s,a,b,c,d,e,f,g)
char *s;
{

	if(Debugfp) {
		if(wrapcount)
			fprintf(Debugfp,"%d: ",wrapcount);
		fprintf(Debugfp,s,a,b,c,d,e,f,g);
		fflush(Debugfp);
		if(++linecount > 8000) {
			linecount = 0;
			wrapcount++;
			fseek(Debugfp,0,0);
		}
	}
}

