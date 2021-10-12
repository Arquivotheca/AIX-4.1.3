static char sccsid[] = "@(#)00  1.13  src/bos/usr/lib/methods/udefdlc/udefdlc.c, dlccfg, bos411, 9428A410j 10/19/93 09:45:21";
/*
 * COMPONENT_NAME: (DLCCFG) DLC Configuration
 *
 * FUNCTIONS: main (udefdlc) err_exit
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* interface:
   udefdlc -l <logical_name>
*/

#include <sys/types.h>
#include <stdio.h>

#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>

#include "cfgdebug.h"


/* external odm manipulation functions */
extern char *malloc();

extern int              odm_initialize();
extern int              odm_terminate();
extern struct Class    *odm_open_class();
extern int              odm_close_class();
extern void            *odm_get_obj();
extern int              odm_rm_obj();


main(argc,argv)
int argc;
char *argv[];
{
extern  int     optind;         /* for getopt function */
extern  char    *optarg;        /* for getopt function */

	char *devname;                  /* device name to undefine */
	char sstring[MAX_ODMI_CRIT];    /* search criteria string */

	struct CuDv  cusobj;            /* customized device object */
	struct CuAt  cusattobj;         /* customized attribute object */

	struct Class *cusdev;           /* customized device class handle */
	struct Class *cusatt;           /* customized attribute class handle */

	int  rc;                        /* return codes go here */
	int     errflg,c;               /* used in parsing parameters   */



  /*************************************************************************
   Parse Command Line:
   *************************************************************************/

	errflg = 0;
	devname = NULL;
	while ((c = getopt(argc,argv,"l:")) != EOF) {
		switch (c) {
		case 'l':
			if (devname != NULL)
				errflg++;
			devname = optarg;
			break;
		default:
			errflg++;
		}
	}
	if (errflg) {
		/* error parsing parameters */
		DEBUG_0("udefdlc: command line error\n");
		exit(E_ARGS);
	}

	/* logical name must be specified */
	if (devname == NULL) {
		DEBUG_0("udefdlc: logical name must be specified\n");
		exit(E_LNAME);
	}


  /*************************************************************************
   Start ODM:
   *************************************************************************/

	rc = odm_initialize();
	if (rc == -1)
	{
		/* init failed */
		DEBUG_0("udefdlc: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}

	/*
	 * Lock the database
	 */
	if ((rc = odm_lock("/etc/objrepos/config_lock",0)) == -1)
	{
		DEBUG_0("udefdlc: odm_lock() failed\n")
		err_exit(E_ODMLOCK);
	}
	DEBUG_0 ("ODM initialized and locked\n")

	/*
	 * Open Customized Device Object Class
	 */
	if ((int) (cusdev = odm_open_class(CuDv_CLASS)) == -1)
	{
		DEBUG_0("udefdlc: open of CuDv failed\n")
		err_exit(E_ODMOPEN);
	}

  /************************************************************************
   Get Customized Device Object from (CuDv):
   ************************************************************************/

	sprintf(sstring,"name='%s'",devname);
	rc = (int) odm_get_obj(cusdev,sstring,&cusobj,ODM_FIRST);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("udefdlc: Device does not exist\n")
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("udefdlc: ODM error getting CuDv object\n")
		err_exit(E_ODMGET);
	}

	/* Device state must be DEFINED in order to be undefined */
	if (cusobj.status != DEFINED)
	{
		DEBUG_0("udefdlc: device not in defined state\n");
		err_exit(E_DEVSTATE);
	}

	/* this device never has parents or children */


	/* Delete all attributes for this device */
	DEBUG_0("udefdlc: deleting customized attributes\n")
	sprintf(sstring,"name='%s'",devname);

	if ((odm_rm_obj(CuAt_CLASS,sstring)) == -1)
	{
		DEBUG_0("udefdlc: odm_rm_obj() failed for CuAt\n")
		err_exit(E_ODMDELETE);
	}

	/* reldevno() releases major and minor no and removes the */
	/* special files from /dev.                               */

	DEBUG_0("udefdlc: calling reldevno()\n");
	reldevno(devname,TRUE);

	/* Delete Cutomized Device Object for this device */
	DEBUG_0("udefdlc: deleting CuDv object\n")
	sprintf(sstring,"name = '%s'",devname);
	if ((odm_rm_obj(cusdev,sstring)) == -1)
	{
		DEBUG_0("udefdlc: odm_rm_obj() failed for CuDv\n")
		err_exit(E_ODMDELETE);
	}

	/* Close Customized Device Object Class */
	if (odm_close_class(CuDv_CLASS) == -1)
	{
		/* ODM error */
		DEBUG_0("udefdlc: failure closing CuDv object class\n")
		err_exit(E_ODMCLOSE);
	}


	/* Terminate ODM */
	(void) odm_terminate();

	/* Successful exit */
	exit(0);
}


/*
 * NAME: err_exit
 *
 * FUNCTION: Exit with an error message.
 *
 * EXECUTION ENVIRONMENT: NONE
 *
 * (NOTES:) Terminate odm after closing all the classes.
 *
 * RETURNS: NONE
 */
err_exit(exit_code)
int     exit_code;              /* Error exit code */
{
	odm_close_class(CuDv_CLASS);
	odm_terminate();
	exit(exit_code);
}



