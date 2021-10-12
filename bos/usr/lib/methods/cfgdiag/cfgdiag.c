static char sccsid[] = "@(#)60  1.2  src/bos/usr/lib/methods/cfgdiag/cfgdiag.c, cfgmethods, bos411, 9428A410j 2/14/94 16:12:08";
/*
 * COMPONENT_NAME: (CFGMETHODS) cfgdiag.c - Diagnostic Config Method Code
 *
 * FUNCTIONS: main(), err_exit()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* interface:
   cfgdiag -l <logical_name> 
*/


/* header files needed for compilation */
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/sysconfig.h>
#include <sys/sysmacros.h>
#include <sys/device.h>

/* Local header files */
#include "cfgdebug.h"



/* main function code */
main(argc, argv, envp)
int	argc;
char	*argv[];
char	*envp[];

{
	extern  int     optind;         /* for getopt function */
	extern  char    *optarg;        /* for getopt function */

	struct cfg_dd cfg;		/* sysconfig command structure */

	char	*logical_name;		/* logical name to configure */
	char	sstring[256];		/* search criteria pointer */
	char	conflist[1024];		/* busresolve() configured devices */
	char	not_resolved[1024];	/* busresolve() not resolved devices */
	char	attr_name[9];		/* name of attribute */

	struct Class *cusdev;		/* customized devices class ptr */
	struct Class *predev;		/* predefined devices class ptr */

	struct CuDv cusobj;		/* customized device object storage */
	struct PdDv preobj;		/* predefined device object storage */
	struct CuDv parobj;		/* customized device object storage */
	struct CuDv busobj;		/* customized device object storage */
	struct CuAt cuatobj;            /* customized attribute object */
	struct PdAt pdatobj;		/* predefined attribute object */

	ushort	devid;			/* Device id - used at run-time */
	int	slot;			/* slot of adapters */
	int	rc;			/* return codes go here */
	int     errflg,c;               /* used in parsing parameters   */

	/*****                                                          */
	/***** Parse Parameters                                         */
	/*****                                                          */
	errflg = 0;
	logical_name = NULL;

	while ((c = getopt(argc,argv,"l:")) != EOF) {
		switch (c) {
		case 'l':
			if (logical_name != NULL)
				errflg++;
			logical_name = optarg;
			break;
		default:
			errflg++;
		}
	}
	if (errflg) {
		/* error parsing parameters */
		DEBUG_0("cfgdiag: command line error\n");
		exit(E_ARGS);
	}

	/*****                                                          */
	/***** Validate Parameters                                      */
	/*****                                                          */
	/* logical name must be specified */
	if (logical_name == NULL) {
		DEBUG_0("cfgdiag: logical name must be specified\n");
		exit(E_LNAME);
	}

	DEBUG_1 ("cfgdiag: Diagnosing device: %s\n",logical_name); 

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
		DEBUG_0("cfgdiag: odm_initialize() failed\n");
		exit(E_ODMINIT);
	}
	DEBUG_0 ("cfgdiag:  ODM initialized\n");

	/* open customized devices object class */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) {
		DEBUG_0("cfgdiag: open class CuDv failed\n");
		err_exit(E_ODMOPEN);
	}

	/* search for customized object with this logical name */
	sprintf(sstring, "name = '%s'", logical_name);
	rc = (int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* No CuDv object with this name */
		DEBUG_1("cfgdiag: failed to find CuDv object for %s\n", logical_name);
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_0("cfgdiag: ODM failure getting CuDv object\n");
		err_exit(E_ODMGET);
	}

	/* open predefined devices object class */
	if ((int)(predev = odm_open_class(PdDv_CLASS)) == -1) {
		DEBUG_0("cfgdiag: open class PdDv failed\n");
		err_exit(E_ODMOPEN);
	}

	/* get predefined device object for this logical name */
	sprintf(sstring, "uniquetype = '%s'", cusobj.PdDvLn_Lvalue);
	rc = (int)odm_get_first(predev, sstring, &preobj);
	if (rc==0) {
		/* No PdDv object for this device */
		DEBUG_0("cfgdiag: failed to find PdDv object for this device\n");
		err_exit(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_0("cfgdiag: ODM failure getting PdDv object\n");
		err_exit(E_ODMGET);
	}

	/* close predefined device object class */
	if (odm_close_class(predev) == -1) {
		DEBUG_0("cfgdiag: close object class PdDv failed\n");
		err_exit(E_ODMCLOSE);
	}


	/******************************************************************
	  Check to see if the device is DEFINED.  If not, error.  
	  Otherwise, check to make sure the parent device is AVAILABLE. 
 
	  If this is an mca adapter, make sure the card is in the slot.
	  and allocate resources to the adapter through busresolve.
	  ******************************************************************/

	if ( cusobj.status == DIAGNOSE )
	{
	    /* We're already in the diagnose state - exit with rc 0. */
	    /* Use the err_exit routine to close all databases.      */
	    err_exit(0);
        }

        /* any state other than DEFINED or DIAGNOSE is cause for error */
	else if ( cusobj.status != DEFINED ) 
	{
		err_exit(E_DEVSTATE);
	}
	
        else
	{

		/*******************************************************
		  The device is not available to the system yet. Now
		  check to make sure that the device's relations will
		  allow it to be configured. In particular, make sure 
		  that the parent is configured (AVAILABLE), and that
		  no other devices are configured at the same location.  
		  *******************************************************/ 

		/* get the device's parent object */ 
		sprintf(sstring, "name = '%s'", cusobj.parent); 
		rc = (int)odm_get_first(cusdev,sstring,&parobj);
		if (rc==0) 
		{
			/* Parent device not in CuDv */
			DEBUG_0("cfgdiag: no parent CuDv object\n");
			err_exit(E_NOCuDvPARENT);
		}
		else if (rc==-1) 
		{
			/* ODM failure */
			DEBUG_0("cfgdiag: ODM failure getting parent CuDv object\n");
			err_exit(E_ODMGET);
		}

		if (parobj.status != AVAILABLE && parobj.status != DIAGNOSE) 
		{
			DEBUG_0("cfgdiag: parent is not AVAILABLE\n");
			err_exit(E_PARENTSTATE);
		}

		/* make sure that no other devices are configured     */
		/* at this location                                   */
		sprintf(sstring, "parent = '%s' AND connwhere = '%s' AND status = %d",
			cusobj.parent, cusobj.connwhere, AVAILABLE);
		rc = (int)odm_get_first(cusdev,sstring,&busobj);
		if (rc == -1) 
		{
			/* odm failure */
			err_exit(E_ODMGET);
		} 
		else if (rc) 
		{
			/* Error: device config'd at this location */
			DEBUG_0("cfgdiag: device already AVAILABLE at this connection\n");
			err_exit(E_AVAILCONNECT);
		}

		if (!strcmp(preobj.subclass,"mca")) {
			/* Make sure card is in specified slot */
			slot = atoi(cusobj.connwhere);
			devid = (ushort) strtol(preobj.devid,(char **) NULL,0);
			sprintf (sstring,"/dev/%s",cusobj.parent);
			rc = chkslot(sstring,slot,devid);
			if (rc != 0) {
				DEBUG_2("cfgdiag: card %s not found in slot %d\n",
					logical_name,slot);
				err_exit(rc);
			}
		}
	       /*
		* Get the bus that this device is connected to.
		*/
		rc = Get_Parent_Bus(cusdev, cusobj.parent, &busobj) ;
		if (rc == 0)
		{
		    /* If bus type of parent is mca, then resolve bus */
		    /* resource requests.                             */
		    if (!strcmp(busobj.PdDvLn_Lvalue,"bus/sys/mca")) 
		    {
			/* Invoke Bus Resolve  */
			rc = busresolve(logical_name,(int)0,conflist,
					not_resolved, busobj.name);
			if (rc != 0) 
			{
			    DEBUG_0("cfgdiag: bus resources could not be resolved\n")
			    err_exit(rc);
			}
		    }
		}
		else if (rc == E_PARENT)
		{
		    DEBUG_0("cfgdiag: device not on bus\n")
		}
		else 
		{
		    DEBUG_0("cfgdiag: ODM err getting parent bus\n")
                            err_exit(rc) ;
		}
		

	       /* 
		* Update customized device object.  
		*/

		cusobj.status = DIAGNOSE;
		
		if (odm_change_obj(cusdev, &cusobj) == -1) 
		{
		    /* ODM failure */
		    DEBUG_0("cfgdiag: ODM failure updating CuDv object\n");
		    err_exit(E_ODMUPDATE);
		}
	} /* end if (device is DEFINED) then ... */


	/* close customized device object class */
	if (odm_close_class(cusdev) == -1) {
		DEBUG_0("cfgdiag: error closing CuDv object class\n");
		err_exit(E_ODMCLOSE);
	}

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

err_exit(exitcode)
char    exitcode;
{
	/* Close any open object class */
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	odm_close_class(CuAt_CLASS);

	/* Terminate the ODM */
	odm_terminate();
	exit(exitcode);
}

