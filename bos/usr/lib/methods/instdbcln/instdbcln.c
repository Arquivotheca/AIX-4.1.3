static char sccsid[] = "@(#)38	1.2  src/bos/usr/lib/methods/instdbcln/instdbcln.c, cfgmethods, bos411, 9428A410j 9/19/91 14:25:48";
/*
 * COMPONENT_NAME: CFGMETH
 *
 * FUNCTIONS: Removes Predefined objects for types with no CuDv objects.
 *	      It is only used by BOS installation scripts.
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



/* header files needed for compilation */
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <odmi.h>
#include <sys/sysmacros.h>

/* Local header files */
#include "cfgdebug.h"


/* main function code */
main(argc, argv, envp)
int	argc;
char	*argv[];
char	*envp[];

{
	char	sstring[256];		/* search criteria pointer */

	struct Class *cusdev;		/* customized devices class ptr */
	struct Class *predev;		/* predefined devices class ptr */
	struct Class *preatt;		/* predefined attributes class ptr */
	struct Class *precon;		/* predefined connections class ptr */

	struct CuDv cusobj;		/* customized device object storage */
	struct PdDv preobj;		/* predefined device object storage */
	struct PdAt pdatobj;		/* predefined attribute obj storage */

	char	utype[250][UNIQUESIZE];
	int	i;
	int	next_first;
	int	rc;			/* return codes go here */

	/*****                                                          */
	DEBUG_0 ("Starting clean up\n")

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
		DEBUG_0("odm_initialize() failed\n")
		exit(E_ODMINIT);
	}
	DEBUG_0 ("ODM initialized\n")

	/* open customized devices object class */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) {
		DEBUG_0("open class CuDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/* open predefined devices object class */
	if ((int)(predev = odm_open_class(PdDv_CLASS)) == -1) {
		DEBUG_0("open class PdDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/* open predefined attributes object class */
	if ((int)(preatt = odm_open_class(PdAt_CLASS)) == -1) {
		DEBUG_0("open class PdAt failed\n")
		err_exit(E_ODMOPEN);
	}

	/* open predefined connections object class */
	if ((int)(precon = odm_open_class(PdCn_CLASS)) == -1) {
		DEBUG_0("open class PdCn failed\n")
		err_exit(E_ODMOPEN);
	}
	/* If runtime attribute exists, get out so nothing happens to database*/
	rc = (int)odm_get_first(preatt, "uniquetype=runtime", &pdatobj);
	if (rc==-1) {
		/* ODM failure */
		DEBUG_0("ODM failure getting runtime attribute")
		err_exit(E_ODMGET);
	} else if (rc!=0) {
		DEBUG_0("invoked during runtime, exiting\n")
		err_exit(0);
	}

	/* get first PdDv object */
	rc = (int)odm_get_first(predev, "", &preobj);
	if (rc==-1) {
		/* ODM failure */
		DEBUG_0("ODM failure getting PdDv object\n")
		err_exit(E_ODMGET);
	}
	
	i = 0;
	/* find PdDv objects with no corresponding CuDv objects */
	while( rc != 0 ) {
		if (strcmp("logical_volume", preobj.class)) {
			sprintf(sstring, "PdDvLn = '%s'", preobj.uniquetype);
			rc = (int)odm_get_first(cusdev,sstring,&cusobj);
			if (rc==0) {
				strcpy(utype[i], preobj.uniquetype);
				i++;
			}
		}
		
		/* get next PdDv object */
		rc = (int)odm_get_next(predev, &preobj);
		if (rc==-1) {
			/* ODM failure */
			DEBUG_0("ODM failure getting PdDv object")
			err_exit(E_ODMGET);
		}
	}

	utype[i][0] = '\0';

	i = 0;
	while(utype[i][0] != '\0') {
		DEBUG_1("Deleting Pd objects for uniquetype = %s/n",utype[i])
		sprintf(sstring, "uniquetype='%s'", utype[i]);
		odm_rm_obj(preatt,sstring);
		odm_rm_obj(precon,sstring);
		odm_rm_obj(predev,sstring);
		i++;
	}

	/* Close any open object class */
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	odm_close_class(PdAt_CLASS);
	odm_close_class(PdCn_CLASS);

	/* Terminate the ODM */
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
