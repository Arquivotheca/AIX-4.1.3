static char sccsid[] = "@(#)65  1.14  src/bos/usr/bin/trcrpt/misc.c, cmdtrace, bos411, 9428A410j 5/2/94 07:08:54";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: errorstrinit, prhookid, hexstr, strip
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  @IBM_COPYRIGHT@
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 *  LEVEL 1, 5 Years Bull Confidential Information
 *
 */


/*
 * FUNCTION: errno to error-string-from-sys/errno.h conversion routine
 */  

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include "rpt.h"

extern char *malloc();

extern FILE *Tmpltfp;


#ifdef TRCRPT

errorstrinit()
{
	int i;
	FILE *fp;
	char line[128];
	char *define_cp;
	char *name_cp;
	char *value_cp;
	int n;

	if((fp = fopen(Errfile,"r")) == NULL) {
		for(i = 0; i < NERRNO; i++) {
			if(Errorstr[i] == NULL) {
				Errorstr[i] = malloc(10);
				sprintf(Errorstr[i],"ERROR_%d",i);
			}
		}
		return;
	}
	while(fgets(line,128,fp)) {
		define_cp = strtok(line," \t\n");	/* #define */
		name_cp   = strtok(NULL," \t\n");	/* EPERM */
		value_cp  = strtok(NULL," \t\n");	/* 1 */
		Debug2("DEFINE='%s' NAME='%s' VALUE='%s'\n",
			define_cp,name_cp,value_cp);
		if(define_cp == NULL || name_cp == NULL || value_cp == NULL)
			continue;
		if(strcmp(define_cp,"#define") != 0)
			continue;
		n = strtol(value_cp,0,0);
		if(n <= 0 || NERRNO <= n)
			continue;
		Errorstr[n] = malloc(strlen(name_cp)+1);
		strcpy(Errorstr[n],name_cp);
	}
	fclose(fp);
	for(i = 1; i < NERRNO; i++) {
		if(Errorstr[i] == NULL) {
			Errorstr[i] = malloc(10);
			sprintf(Errorstr[i],"ERROR_%d",i);
		}
	}
	Errorstr[0] = "0";
	Nerrorstr = NERRNO;
}

#endif



#ifdef TRCRPT

prhookid()
{
	int i;

	gettmpltinit();
	for(i = 0; i < NHOOKIDS; i++) {
		if(Tindexes[i].t_state & T_DEFINED) {
			/* printing data */
			if (printf("%s\n",hexstr(i)) <= 0) {
				perror("printf");
				genexit(1);
			}
		}
	}
}

#endif


char *hexstr(n)
{
	static char buf[8];

	sprintf(buf,"%03X",n);
	return(buf);
}
strip(line)
char *line;
{
	char *cp;

	for(cp = line + strlen(line) - 1; cp > line; cp--) {
		switch(*cp) {
		case ' ':
		case '\t':
		case '\n':
			continue;
		}
		cp++;
		break;
	}
	*cp = '\0';
	Debug("strip: '%s'\n",line);
}

