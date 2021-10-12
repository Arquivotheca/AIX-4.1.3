static char sccsid[] = "@(#)12	1.12  src/bos/usr/lib/methods/undefine/undefine.c, cfgmethods, bos411, 9428A410j 6/2/94 07:39:05";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: err_exit
 *		main
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/* interface:
   undefine -l <logical_name>
*/


/* header files needed for compilation */
#include <stdio.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include "cfgdebug.h"
#include "ptycfg.h"

main(argc,argv)
int argc;
char *argv[];

{
extern  int     optind;         /* for getopt function */
extern  char    *optarg;        /* for getopt function */

	char *devname;			/* device name to undefine */
	char sstring[256];		/* search criteria pointer */

	struct CuDv  cusobj;	 	/* customized device object */
	struct CuDv  cusparentobj;	/* customized parent object */
	struct CuDep cusdepobj;		/* customized dependency object */ 

	struct Class *cusdev;		/* customized device class handle */

	int     rc;                     /* return codes go here */
	int     errflg,c;               /* used in parsing parameters   */


  	/*
	 * Parse Command Line
	 */
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
		DEBUG_0("undefine: command line error\n");
		exit(E_ARGS);
	}

	/* logical name must be specified */
	if (devname == NULL) {
		DEBUG_0("undefine: logical name must be specified\n");
		exit(E_LNAME);
	}

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
		DEBUG_0("undefine: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}
	DEBUG_0 ("ODM initialized\n")

	/* open customized device object class */
	if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1) {
		/* error opening class */
		DEBUG_0("undefine: open of CuDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/* Get Customized Device Object */
	sprintf(sstring,"name = '%s'",devname);	
	rc = (int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("undefine: Device does not exist\n")
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("undefine: ODM error getting CuDv object\n")
		err_exit(E_ODMGET);
	}

	/* Device state must be DEFINED in order to be undefined */
	if (cusobj.status != DEFINED) {
		DEBUG_0("undefine: device not in defined state\n");
		err_exit(E_DEVSTATE);
	}

	/* See if this device has children.  If it does, it can  */
	/* not be undefined                                      */
	sprintf(sstring,"parent = '%s'",devname);
	rc = (int)odm_get_first(cusdev,sstring,&cusparentobj);
	if (rc==-1) {
		/* ODM error */
		DEBUG_0("undefine: ODM error looking for children\n")
		err_exit(E_ODMGET);
	}
	else if (rc!=0) {
		/* Has children!! */
		DEBUG_0("undefine: Device has children\n")
		err_exit(E_CHILDSTATE);
	}

	/* See if this device is a dependency of another device.  If */
	/* it is, then it can not be undefined                       */

	sprintf(sstring,"dependency = '%s'",devname);
	rc = (int)odm_get_first(CuDep_CLASS,sstring,&cusdepobj);
	if (rc==-1) {
		/* ODM error */
		DEBUG_0("undefine: ODM error checking dependencies\n")
		err_exit(E_ODMGET);
	}
	else if (rc!=0) {
		/* This device has dependent devices!! */
		DEBUG_0("undefine: Device has dependent devices\n")
		err_exit(E_DEPSTATE);
	}

	/* Everything is OK.  Begin deleting objects. */

	/* Delete VPD for this device */
	DEBUG_0("undefine: deleting customized VPD\n")
	sprintf(sstring,"name = '%s'",devname);
	if ((odm_rm_obj(CuVPD_CLASS,sstring)) == -1) {
		DEBUG_0("undefine: odm_rm_obj() failed for CuVPD\n")
		err_exit(E_ODMDELETE);
	}

	/* Delete all attributes for this device */
	DEBUG_0("undefine: deleting customized attributes\n")
	sprintf(sstring,"name = '%s'",devname);
	if ((odm_rm_obj(CuAt_CLASS,sstring)) == -1) {
		DEBUG_0("undefine: odm_rm_obj() failed for CuAt\n")
		err_exit(E_ODMDELETE);
	}

	/* reldevno() releases major and minor no and removes the */
	/* special files from /dev.                               */

	DEBUG_0("undefine: calling reldevno()\n");
	reldevno(devname,TRUE);

	/* Also, take care of other special file for PTY device.  */
	if (strncmp(cusobj.PdDvLn_Lvalue,PTY_UTYPE,11) == 0) {
		DEBUG_0("undefine: calling reldevno() for pty\n");
        reldevno(ATTDUMMY_MASTER, FALSE);
        relmajor(ATTDUMMY_MASTER);
        reldevno(ATTDUMMY_SLAVE, TRUE);
        reldevno(BSDDUMMY_SLAVE, TRUE);
	}

        /* Release the device instance number for this device */
        relinst(devname);

	/* Delete Cutomized Device Object for this device */
	DEBUG_0("undefine: deleting CuDv object\n")
	sprintf(sstring,"name = '%s'",devname);
	if ((odm_rm_obj(cusdev,sstring)) == -1) {
		DEBUG_0("undefine: odm_rm_obj() failed for CuDv\n")
		err_exit(E_ODMDELETE);
	}

	/* Close Customized Device Object Class */
	if (odm_close_class(CuDv_CLASS) == -1) {
		/* ODM error */
		DEBUG_0("undefine: failure closing CuDv object class\n")
		err_exit(E_ODMCLOSE);
	}

	/* Terminate ODM */
	odm_terminate();

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
