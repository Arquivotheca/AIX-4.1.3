static char sccsid[] = "@(#)81	1.1  src/bos/usr/bin/uucp/uucpadm/search.c, cmduucp, bos411, 9428A410j 8/3/93 16:04:17";
/* 
 * COMPONENT_NAME: CMDUUCP search.c
 * 
 * FUNCTIONS: search 
 *
 * ORIGINS: 10  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  Uucpadm
 *
 *  Copyright (c) 1988 IBM Corporation.  All rights reserved.
 *
 *
 */

/*
 *  search - locate a string in a series of config files.
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "defs.h"
extern void setservice();
extern char *fSystems[], *fDevices[], *fDialers[];	/* From sysfiles.c */

char *search (pattern, filetype, fieldnum)
	char	*pattern;   /* Pattern to search for */
	int	filetype;   /* Search Systems, Devices or Dialers files */
	int	fieldnum;   /* Field to be searched */
{
	char *x, *svcs[] = { "cu", "uucico"};
	int svc_ndx, fil_ndx, err;
	char **filelist;
	extern struct files Rules[FILES];
	extern char  File[FILES][MAXFILE];

	if ( filetype == Systems ) {
		filelist = fSystems;
	}
	else if ( filetype == Devices ) {
		filelist = fDevices;
	}
	else if (filetype == Dialers ) {
		filelist = fDialers;
	}
	else {
		return NULL;
	}
	/* Check all config files for all services (cu, uucico, etc); */
	for (svc_ndx=0; svc_ndx < (sizeof(svcs)/sizeof(x)); svc_ndx++) {
		setservice(svcs[svc_ndx]); /* Sets up the filelist array */

		/* Check each config file for the current service */
		for (fil_ndx=0; filelist[fil_ndx] != NULL; fil_ndx++) {
			(void) strcpy(Rules[filetype].name, filelist[fil_ndx]);

			Rules[filetype].fd=open(Rules[filetype].name,O_RDONLY);
			if (Rules[filetype].fd >= 0) {
				/* Read entire file into RAM */
				if ((err = bload(&Rules[filetype])) != EX_OK)
					continue;

				/* Search for the pattern in RAM */
				x = srch(pattern,strlen(pattern), \
					 File[filetype],0,fieldnum, \
					 &Rules[filetype]);

				/* If an entry was found, return. */
				if (x != NULL) {
					return(x);
				}
			}
		}
	}
	/* Pattern not found. Return null */
	return NULL ;
}
