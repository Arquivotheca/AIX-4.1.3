static char sccsid[] = "@(#)51	1.5  src/bos/usr/lib/methods/defaio/defaio.c, cfgmethods, bos411, 9428A410j 6/6/94 13:53:18";
/*
 * COMPONENT_NAME: (CFGMETH) AIO DEFINE PROGRAM
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include <sys/cfgodm.h>
/* #define CFGDEBUG */
#include "cfgdebug.h"

#define MAX_SEARCH 512
#define DEFAULT_ODMDIR "/etc/objrepos"

/* The two below must match, of course. */
#define CRITERIA "class = 'aio' AND subclass= 'node' AND type = 'aio'"
#define PdDvLINK "aio/node/aio"

extern int odmcf_errno;

char *prog;

/*
 * NAME: main
 *
 * FUNCTION: Create an aio object (if it is not already present), and
 *		return it's name to the Config-Manager
 *
 * EXECUTION ENVIRONMENT: Called directly on the command line. 
 *
 * RETURNS: "aio0"
 */

main(argc,argv,envp)
int	argc;
char	*argv[];
char	*envp[];
{
	void	err_exit();
	struct	Class	*cusdev;	/* handle for customized devices class */
	struct  Class	*predev;
	struct  Class	*preatt;
	struct	CuDv	CuDv;
	struct	PdDv	preobj;
	struct	CuAt	*cuat;
	int	rc;
	char	*autocfg ={ NULL };
	char	*errstr;
	char	search_str[MAX_SEARCH];
	char	sstring[256];
	char	logical_name[256];
	char   *odmdir = getenv("ODMDIR");
	char	odmlockfile[MAXPATHLEN+1] = { '\0' };

	prog = argv[0];
	
	/* start up odm */
	if ((rc = odm_initialize())<0) {
		/* initialization failed */
		fprintf(stderr, "%s: odm initialization failed\n", prog);
		exit(E_ODMINIT);
	}

	DEBUG_1( "odm_initialize() returned %d\n", rc )

	/*******************************************************************
	  Get the predefined object for this device. This object is located
	  searching the predefined devices object class based on the unique
	  type link descriptor in the customized device.
	  *******************************************************************/
	/* open predefined devices object class (PdDv) */
	if ((int)(predev = odm_open_class(PdDv_CLASS)) == -1) {
		fprintf(stderr, "%s: couldn't open predefined object class\n",
			prog);
		err_exit (E_ODMOPEN);
	}

	/* search Predefined devices for object with this unique type */
	sprintf(sstring, CRITERIA);

	if ((rc = (int)odm_get_obj(predev,sstring,&preobj,ODM_FIRST)) == 0) {
		/* odm objects not found */
		fprintf(stderr, "%s: PdDv crit=\"%s\" found no objects.\n",
			prog, sstring);
		err_exit (E_NOPdDv);
	} else if (rc == -1) {
		/* odm error occurred */
		fprintf(stderr, "%s: get_obj failed, crit=\"%s\".\n",
			prog, sstring);
		err_exit (E_ODMGET);
	}

	odm_close_class(PdDv_CLASS);

	/*****								*/
	/***** Create Customized Object for this device			*/
	/*****								*/

	sprintf( search_str, "name = 'aio0'" );
	rc = odm_get_first( CuDv_CLASS, search_str, &CuDv );
	if (rc == -1) {	
		fprintf(stderr, "%s: Error reading CuDv object where \"%s\"\n",
			prog, search_str);
		exit( E_ODMGET );
	}
	if (rc == 0) {
		strcpy(CuDv.name,"aio0");
		CuDv.status = DEFINED;
		CuDv.chgstatus = DONT_CARE;
		strcpy( CuDv.location, "" );
		CuDv.ddins[0] = '\0';
		CuDv.parent[0] = '\0';
		strcpy(CuDv.connwhere,"");
		strcpy(CuDv.PdDvLn_Lvalue, PdDvLINK);

		/* add customized object to customized devices object class */
		if ((rc = odm_add_obj(CuDv_CLASS,&CuDv)) < 0) {
			/* error: add failed */
			fprintf(stderr, "%s: add_obj failed", prog);
			odm_close_class(CuDv_CLASS);
			exit(E_ODMADD);
		}
		DEBUG_1( "add_obj returned %d\n", rc )
	} else {
		/* record already exists */
		DEBUG_0( "Record already exists\n")
		;
	}

	cuat = getattr("aio0","autoconfig",FALSE,&rc);
	if (cuat != NULL)
		autocfg = cuat->value;

	/*
	 * device defined successfully
	 * If the value of the autocfg attribute is "available" then
	 * we let cfgmgr know it's supposed to run the cfg method for
	 * this "device" by putting it's name to stdout.  cfgmgr does
	 * some concatenating, so we help it with a trailing space.
	 */
	if (!strcmp("available",autocfg))
		printf("%s ", CuDv.name);

	(void)odm_terminate();
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
	odm_close_class(PdDv_CLASS);
	odm_close_class(CuAt_CLASS);
	odm_close_class(PdAt_CLASS);

	/* Terminate the ODM */
	odm_terminate();
	fprintf(stderr, "%s: exiting with code %d\n", prog, exitcode);
	exit(exitcode);
}
