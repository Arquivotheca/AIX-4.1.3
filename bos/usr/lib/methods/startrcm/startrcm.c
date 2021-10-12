static char sccsid[] = "@(#)69	1.1  src/bos/usr/lib/methods/startrcm/startrcm.c, rcm, bos411, 9428A410j 11/1/93 10:57:51";
/*
 *   COMPONENT_NAME: RCM
 *
 *   FUNCTIONS: err_exit
 *		main
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*
 * Include files needed for this module follow
 */
#include <sys/types.h>
#include <sys/errno.h>
#include <stdio.h>

/*
 * odm interface include files
 */
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include "cfgdebug.h"

/*
 * NAME:  main
 *
 * FUNCTION: 
 *
 *   The purpose of this program is to start up the RCM.  RCM has a 
 *   dependency on LFT and so we check if LFT is available before
 *   trying to define and configure the RCM.
 *
 *
 */
main(argc, argv)
int	argc;
char	*argv[];
{
struct CuDv		cudv;		/* customized dependency object */
struct PdDv		pddv;		/* customized dependency object */
struct objlistinfo	list;		/* info about cudep object list */
int			rc;		/* return status */
char                    crit[256];      /* search criteria string */
char			*outp,		/* return buffer pointers for invoke */
			*errp;
int			i;		/* loop variable */

	/*
	 * Start up ODM for further use.
	 */
	if( (rc = odm_initialize()) < 0 )
	{
		DEBUG_0("Can't initialize ODM\n");
		exit(E_ODMINIT);
	}

	/*
	 * Obtain the predefined device object for the LFT type, this
	 * will be used to determine if the LFT is available.
	 */
	if( (rc = (int)odm_get_first(PdDv_CLASS, "type = 'lft'", &pddv)) == 0 )
	{
		DEBUG_0("startrcm: no PdDv object for lft\n");
		err_exit(E_NOPdDv);
	}
	else if( rc == -1 )
	{
		DEBUG_0("startrcm: fatal ODM error\n");
		err_exit(E_ODMGET);
	}

	/*
	 * Check if LFT is available.  The pddv.uniquetype will provide the
	 * string to qualify for the PdDvLn object of the CuDv class.
	 * If the status = 1 then the lft is available.
	 */
	sprintf(crit, "PdDvLn = '%s' AND status = 1", pddv.uniquetype);
	if( (rc = (int)odm_get_first(CuDv_CLASS, crit, &cudv)) < 0 )
	{
		DEBUG_0("startrcm: fatal ODM error\n");
		err_exit(E_ODMGET);
	}	
	else if( rc == 0 )
        {
		DEBUG_0("startrcm: LFT not loaded for rcm\n");
		err_exit(E_OK);
        }
	
	/*
	 * LFT is available and so we can now load RCM
	 * First get some information out of PdDv.
	 *
	 */
	if( (rc = odm_get_first(PdDv_CLASS, "type = 'rcm'", &pddv)) == 0 )
	{
		DEBUG_0("startrcm: no PdDv object for rcm\n");
		err_exit(E_NOPdDv);
	}
	else if( rc == -1 )
	{
		DEBUG_0("startrcm: fatal ODM error\n");
		err_exit(E_ODMGET);
	}

	/* ---------------------------------------------------------------
	 *
	 * Run the method to define rcm - methods/define. This will create
	 * an entry in the CuDv for this device using the PdDv.  The 
	 * following flags are passed:
	 *
	 * -c <class> -s <subclass> -t < type>
	 *
	 * --------------------------------------------------------------- */

	sprintf(crit, "PdDvLn = %s", pddv.uniquetype);

	if( (rc = (int)odm_get_first(CuDv_CLASS, crit, &cudv)) < 0 )
	{
		DEBUG_1("Error getting CuDv info: rc = %d\n",rc);
		err_exit(rc);
	}
	else if(rc == 0)
	{
		/*
	 	* There is no customized device, so define it now before
	 	*  configuring it.
	 	*/
		rc = odm_run_method( pddv.Define, "-c rcm -s node -t rcm",
				 &outp, &errp);
		if( rc != E_OK)
		{
			DEBUG_1("RCM define failed %s\n",errp);
			err_exit( rc < 0 ? E_ODMRUNMETHOD : rc );
		}

		/* echo out device name of rcm so it will be run by cfgmgr */
		fprintf(stdout, "%s\n", outp);
		
	}
	else if(cudv.status != 1) /* if rcm is defined but not available */ 
	{
		/* echo out device name of rcm so it will be run by cfgmgr */
		fprintf(stdout, "%s\n", cudv.name);
	}

	/* Terminate the ODM connection and exit with success.  */
	odm_terminate();
	exit(E_OK);
}


err_exit(exitcode)
char exitcode;
{
	odm_terminate();
	exit(exitcode);
}

