static char sccsid[] = "@(#)55	1.2  src/bos/usr/bin/errlg/samples/getvfsname.c.S, cmderrlg, bos411, 9428A410j 3/26/94 09:56:50";

/*
 * COMPONENT_NAME: (CMDERRLG) Error Logging Sample 
 *
 * FUNCTIONS: Get the VFS name from the number passed
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <stdio.h>
#include <limits.h>
#include <sys/vmount.h>

static int getfsname(int, char *);

main(int argc, char *argv[])
{
	int num, i;
	char name[PATH_MAX];

	if (argc<2) {
		fprintf(stderr,"Usage:  %s <filesystem numbers>\n",argv[0]);
		exit(1);
	}

	for (i=1; i<argc; i++) {
		sscanf(argv[i],"%d",&num);
		if (getfsname(num,name) < 0) exit(2);
		printf("%s\n",name);
	}
	return(0);
}

/*
 * Get the file system name for the VFS serial number in "num".
 * It returns -1 for an error, 0=no name found, 1=success.
 */
static int
getfsname(int num, char *namep)
{
	int rv,clen;
	char *cp;
	size_t siz = sizeof(struct vmount)*10;
	struct vmount *bufp, *p;

	*namep = '\0';	/* Indicate nothing copied yet. */

	if ((p = (struct vmount *)malloc(siz)) == (struct vmount *)NULL) {
		perror("malloc");
		return(-1);
	}

	while (1) {
		rv = mntctl(MCTL_QUERY,siz,p);
		if (rv>0) break;	/* It worked */
		if (rv<0) {
			perror("mntctl");
			free(p);
			return(-1);
		}

		/* Use a bigger size */
		siz = *(size_t *)p;
		p = (struct vmount *)realloc((void *)p,siz);
		if (p == (struct vmount *)NULL) {
			perror("realloc");
			return(-1);
		}
	}

	/* Look for the file system. */
	bufp = p;

	for (; rv>0; p=(struct vmount *)((char *)p+p->vmt_length), rv--) {
		if (p->vmt_vfsnumber == num) {
			/* Found it */
			if ( ((cp = vmt2dataptr(p,VMT_OBJECT)) != NULL) && 
			     ((clen = vmt2datasize(p,VMT_OBJECT)) != 0) )
			{
				strcpy(namep,cp);
				free(bufp);
				return(1);
			}
		}
	}
	
	/* No name was found. */
	free(bufp);
	return(0);
}
