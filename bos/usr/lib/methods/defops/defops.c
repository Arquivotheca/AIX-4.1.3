static char sccsid[] = "@(#)34	1.6  src/bos/usr/lib/methods/defops/defops.c, cfgmethods, bos411, 9428A410j 9/19/91 14:25:32";
/*
 * COMPONENT_NAME: (CFGMETH) SOL SUBSYSTEM DEFINE PROGRAM
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include <sys/cfgodm.h>
#include "cfgdebug.h"

#define MAX_SEARCH 256
#define OPS_UTYPE "uniquetype = 'node/node/ops'"
#define OPS_NAME "ops0"
#define OPS_PdDvLn "node/node/ops"
#define PORT_PdDvLn "adapter/otp/op"

/*
 * NAME: main
 *
 * FUNCTION: Create an ops object (if it is not already present), and
 *	     return it's name to the Config Manager
 *
 * EXECUTION ENVIRONMENT: Called directly by Config-Manager, with no parameters
 *
 * RETURNS: "ops0"
 */

main(argc,argv,envp)
int	argc;
char	*argv[];
char	*envp[];
{
struct	CuDv	Cus_obj;	/* Customized object */
struct	PdDv	Pre_obj;	/* Predefined object */
int	rc;
char	*errstr, *outp;
char	sstring[MAX_SEARCH];


	/* start up odm */
	rc = odm_initialize();
	if (rc < 0)
		/* initialization failed */
		exit (E_ODMINIT);

	/* make sure at least one optical port is AVAILABLE */
	sprintf(sstring,"PdDvLn = '%s'",PORT_PdDvLn);
	rc = odm_get_first(CuDv_CLASS,sstring,&Cus_obj);
	if (rc == 0) {
		/* port not defined yet so just exit */
		DEBUG_0("defops: failed to find port CuDv\n")
		exit (E_OK);
	}
	else if (rc == -1) {
		/* ODM failure */
		DEBUG_0("defops: ODM failure getting CuDv object")
		exit (E_ODMGET);
	}
	if (Cus_obj.status != AVAILABLE) {
		/* no port available so just kick on out */
		DEBUG_0("defops: no port is available\n")
		exit (E_OK);
	}

	/* make sure OPS predefined exists */
	strcpy(sstring,OPS_UTYPE);
	rc = (int) odm_get_first(PdDv_CLASS,sstring,&Pre_obj);
	if (rc == -1) {
		DEBUG_0("defops: error reading PdDv class\n")
		exit (E_ODMGET);
	}
	if (rc == 0) {
		DEBUG_0("defops: OPS predefine non-existent\n")
		exit (E_NOPdDv);
	}

	sprintf(sstring,"PdDvLn = '%s'",OPS_PdDvLn);
	rc = (int) odm_get_first(CuDv_CLASS,sstring,&Cus_obj);
	if( rc == -1 ) {	
		DEBUG_0("defops: error reading CuDv class\n")
		exit (E_ODMGET);
	}
	if( rc == 0 ) {
		sprintf(sstring,"-c node -s node -t ops");
		rc = odm_run_method(Pre_obj.Define,sstring,&outp,NULL);
		if (rc) {
			DEBUG_0("defops: cannot run define method\n")
			return (E_ODMRUNMETHOD);
		}
		/* device successfully defined */
		fprintf(stdout,"%s\n",outp);
	} else {
		/* already defined so just print name to stdout */
		fprintf(stdout,"%s\n",Cus_obj.name);
	}
	odm_terminate();
	exit (E_OK);
}
