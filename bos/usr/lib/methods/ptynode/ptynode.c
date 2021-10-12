static char sccsid[] = "@(#)96  1.10  src/bos/usr/lib/methods/ptynode/ptynode.c, cfgmethods, bos411, 9428A410j 9/19/91 14:25:57";
/*
 * COMPONENT_NAME: (CFGMETH) Pty Node Program.
 *
 * FUNCTIONS: main, err_exit
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <cf.h>		/* Error codes */

#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>

#include "cfgdebug.h"

/*
 * NAME: main
 * 
 * FUNCTION: This program detects and manages pty devicecs.
 * 
 * EXECUTION ENVIRONMENT:
 *
 *	This program is invoked by the configuration manager as a node program.
 *           
 * NOTES: A pty device that needs to be configured is detected by a Status of 
 *        "Defined" and a Previous Status of "Available".  The pty device to
 *        be configured is returned via stdout to the Configuration Manager
 *        which in turn will eventually invoke the configure method for the
 *        pty device.
 *
 * RETURNS: Exits  0 on success, >0 Error code.
 */
#define PTY_UTYPE	"pty/pty/pty"

main(argc,argv)
int argc;
char *argv[];
{
	int	rc;			/* return codes go here */
	int     how_many;               /* number of attributes returned */

	char	sstring[256];		/* search criteria */
	char	params[40];		/* parameters to configure method */
	char	autoconfig[50];		/* auto config value */
	char	*outptr;		/* stdout of pty config method */
	char	*errptr;		/* stderr of pty config method */

	struct CuDv	cusobj;		/* customized devices class */
	struct PdDv	preobj;		/* predefined devices class */
	struct CuAt     *cuattr;        /* customized attribute object */

	void	err_exit();		/* error exit routine */


	/*****************
	  Initialize ODM
	 ****************/
	if (odm_initialize() == -1) {
		DEBUG_0 ("ptynode: ODM Initialize failed\n")
		exit (E_ODMINIT);
	}

	/*****************************************************
	  Attempt to get pty device from customized database
	 *****************************************************/
	sprintf(sstring,"PdDvLn = '%s'",PTY_UTYPE);
	rc = (int)odm_get_first(CuDv_CLASS,sstring,&cusobj);
	if (rc == -1) {
		/* odm error occurred */
		DEBUG_1("ptynode: get_obj failed, crit = %s\n",sstring)
		err_exit (E_ODMGET);
	}
	else if (rc == 0) {
	/********************************************
	  No pty defined - So invoke define method
	 ********************************************/
		sprintf(sstring,"uniquetype = '%s'", PTY_UTYPE);
		rc = (int)odm_get_first(PdDv_CLASS,sstring,&preobj);
		if (rc == -1) {
			DEBUG_1 ("ptynode: get_obj failed,crit = %s\n",sstring)
			err_exit (E_ODMGET);
		}
		else if (rc == 0) {
			DEBUG_1("ptynode: PdDv crit = %s found no objects.\n",
								sstring)
			err_exit (E_NOPdDv);
		}
		else {
			sprintf(params,"-c %s -s %s -t %s",preobj.class,
					preobj.subclass,preobj.type);
			if ((odm_run_method(preobj.Define,params,
						&outptr,&errptr)) != 0)
				err_exit (E_ODMRUNMETHOD);

			/***********************************************
			  Return pty device name to Configuration
			  manager via stdout
			***********************************************/
			fprintf (stdout,"%s\n",outptr);
		}
	}
	else {
		DEBUG_0( "Found pty device.\n")
		/*********************************************
		  If pty is to be "auto configured", then ...
		 *********************************************/
		cuattr = getattr(cusobj.name,"autoconfig",FALSE,&how_many);
		if ((cuattr == NULL)) {
			/***********************************************
			  On getattr() error, default to always configure
			  pty.  So return name to Configuration manager
			***********************************************/
			fprintf (stdout,"%s\n",cusobj.name);
		}
		else if (strcmp(cuattr->value,"defined")) {
			/***********************************************
			  As long as the value of the autoconfig attribute
			  is not "defined" then also return the pty name
			  to the Configuration manager via stdout
			***********************************************/
			fprintf (stdout,"%s\n",cusobj.name);
		}
	}

	/* Terminate ODM */
	odm_terminate();

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
	/* Terminate the ODM */
	odm_terminate();
	exit(exitcode);
}
