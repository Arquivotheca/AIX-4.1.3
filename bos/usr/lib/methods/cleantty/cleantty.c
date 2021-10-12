static char sccsid[] = "@(#)63	1.8  src/bos/usr/lib/methods/cleantty/cleantty.c, cfgmethods, bos411, 9428A410j 7/1/94 15:33:18";
/*
 * COMPONENT_NAME: (CFGMETH) Clean Inittab File.
 *
 * FUNCTIONS: main, err_exit
 *
 * ORIGINS: 27, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */
#include <stdio.h>
#include <string.h>
#include <cf.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>

#include "cfgdebug.h"

#define TTY_CLASS  "tty/"
#define LFT_CLASS  "lft/"

/*
 * NAME: main
 * 
 * FUNCTION: This program cleans the inittab file of unconfigured ttys.
 * 
 * EXECUTION ENVIRONMENT:
 *
 *	This program is invoked by the IPL process.
 *           
 * NOTES: Any tty entry in inittab for a tty that is not configured is
 *	  removed. Also, If the current console is a tty device - the
 *	  inittab entry for that tty is removed.
 *
 * RETURNS: Exits with 0 on success, >0 Error code.
 */

main(argc,argv,envp)
int argc;
char *argv[];
char *envp[];
{
	FILE *fstr;		/* pointer to inittab file */
	char sstring[100];	/* search criteria string */
	char entry[160];		/* inittab entry string */
	struct CuAt CuAt;	/* customized attribute object */
	struct CuDv CuDv;	/* customized device object */
	struct Class *cusdev;	/* object class pointer */
	char *consname;		/* name of console device */
	char *tty;		/* name of tty device */
	char *outptr, *errptr;	/* captured stdout and stderr */
	int rc;			/* return codes */
	void	err_exit();	/* error exit routine */
	
	DEBUG_0 ("cleantty: BEGIN\n")

	/*****************
	  Initialize ODM
	 *****************/
	if (odm_initialize() < 0)
		exit (E_ODMINIT);
	DEBUG_0 ("cleantty: ODM Initialized\n")

	/********************************************************
	  Check to see if there is an inittab entry for console
	 ********************************************************/
	odm_run_method("lsitab","cons",&outptr,&errptr);
	if (!strncmp(outptr,"cons:",5)) {

		DEBUG_1 ("cleantty: Found inittab entry for console:\n%s",
								outptr)
		/********************************************
		  Get the system console device from sys0
		 ********************************************/
		strcpy (sstring,"name=sys0 AND attribute=syscons");
		if ((rc = (int)odm_get_first(CuAt_CLASS,sstring,&CuAt)) == 0)
			err_exit (E_NOCuOBJ);
		else if (rc == -1)
			err_exit (E_ODMGET);
		DEBUG_1 ("cleantty: Got syscons attribute: %s\n",CuAt.value)

		/****************************************
		  Pull console device name out of path
		 ***************************************/
		if ((consname = strrchr(CuAt.value,'/')) != NULL) {
			/******************************************************
		  	  Remove inittab entry device that is console
		 	 ******************************************************/
			consname++;
			DEBUG_1 ("cleantty: Running rmitab on %s\n",consname)
			odm_run_method("rmitab",consname,&outptr,&errptr);
		}
	}

	/*************************
	  Open /etc/inittab file
	 *************************/
	if ( (fstr = fopen ("/etc/inittab","r")) == (FILE *)NULL ) {
		DEBUG_0 ("Open of /etc/inittab failed\n")
		err_exit (E_OPEN);
	}
	DEBUG_0 ("cleanttty: Open of /etc/inittab successful.\n")

	/********************************
	  Open Customized Devices Class
	 ********************************/
	if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1)
		err_exit (E_ODMOPEN);

	/**************************************************
	  Go through inittab file looking for ttys and lft
	 **************************************************/
	while ( (fgets (entry,160,fstr)) != (char *)NULL ) {

		/* Ignore comments or lines without ":" */
		if (!strncmp(entry, ":", 1 )
		   || (tty = strtok (entry,":")) == NULL) {
			continue;
		};

		DEBUG_1 ("cleantty: Found inittab entry for %s\n",tty)
			
		/************************************************
		  See if the device is currently configured
		 ************************************************/
		sprintf(sstring,"name=%s",tty);
		if ((rc=(int)odm_get_first(cusdev,sstring,&CuDv))==-1)
			err_exit (E_ODMGET);

		else if (rc == 0) {
			/* No CuDv object found */
			/* Go ahead and remove entry if name starts with tty */
			if (!strncmp(tty,"tty",3)) {
				DEBUG_1 ("cleantty: Running rmitab on %s\n",tty)
				odm_run_method("rmitab",tty,&outptr,&errptr);
			}
			continue;
		}

		/* See if it is a TTY of an LFT */
		if (strncmp(CuDv.PdDvLn_Lvalue, TTY_CLASS, strlen(TTY_CLASS))
		   && strncmp(CuDv.PdDvLn_Lvalue,LFT_CLASS,strlen(LFT_CLASS))) {
			/* Not a tty and not lft ==> Nothing to do. */
			continue;
                }

		if (CuDv.status != (short) AVAILABLE) {
			DEBUG_1 ("cleantty: Running rmitab on %s\n",tty)
			odm_run_method("rmitab",tty,&outptr,&errptr);
		};
	} /* End while ( (fgets (entry,160,fstr)) != (char *)NULL ) */

	if (odm_close_class(cusdev) < 0)
		err_exit (E_ODMCLOSE);
	odm_terminate();
	fclose (fstr);
	exit(0);
}


/*
 * NAME: err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *           back out on an error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  The device
 *      specific routines for the various device specific config methods
 *      must not call this function.
 *
 * NOTES:
 *
 * void
 *   err_exit( exitcode )
 *      exitcode = The error exit code.
 *
 * RETURNS:
 *               None
 */
void
err_exit(exitcode)
int	exitcode;
{
	/* Close any open object class */
	odm_close_class(CuDv_CLASS);

	/* Terminate the ODM */
	odm_terminate();
	exit(exitcode);
}

