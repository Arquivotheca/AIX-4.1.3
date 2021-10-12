static char sccsid[] = "@(#)86	1.1  src/bos/usr/bin/errlg/libras/dircheck.c, cmderrlg, bos411, 9428A410j 3/2/93 08:58:49";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: dircheck
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libras.h>

/*
 * Ensure that the directories for 'pathname' exist.
 */

dircheck(pathname)
char *pathname;
{
	char dir[512];
	char *cp;
	int status,pid;

	strcpy(dir,pathname);
	if((cp = strrchr(dir,'/')) == 0)
		return(0);
	*cp = '\0';
	Debug("dircheck: pathame=%s\n",dir);
	if((pid = fork()) == 0) {
		if(jmkdir(dir))
			exit(1);
		exit(0);
	}
	while(wait(&status) != pid)
		{;}
	return(status != 0 ? -1 : 0);
}

static jmkdir(dirname)
char *dirname;
{
	char *cp;
	struct stat statbuf;
	int depth;
	char cmd[256];
	char currdir[256];

	currdir[0] = '\0';
	if(dirname[0] == '/')
		chdir("/");
	Debug("jmkdir(%s)\n",dirname);
	depth = 0;
	for(cp = strtok(dirname,"/"); cp; cp = strtok(0,"/")) {
		strcat(currdir,"/");
		strcat(currdir,cp);
		Debug("currdir='%s'\n",currdir);
		if( stat(currdir,&statbuf) == 0 &&
		   (statbuf.st_mode & S_IFMT) != S_IFDIR) {
			Debug("st_mode=%x\n",statbuf.st_mode);
			return(-1);
		}
		if(chdir(currdir)) {
			sprintf(cmd,"mkdir %s",currdir);
			if(shell(cmd)) {
				Debug("cannot mkdir '%s'\n",currdir);
				return(-1);
			}
		}
		if(++depth > 8) {
			Debug("depth exceeds 8\n");
			return(-1);
		}
	}
	Debug("mkdir successful\n");
	return(0);
}

