static char sccsid[] = "@(#)99  1.11  src/bos/usr/lib/methods/defdlc/defdlc.c, dlccfg, bos411, 9428A410j 10/19/93 09:41:51";
/*
 * COMPONENT_NAME: (DLCCFG) DLC Configuration
 *
 * FUNCTIONS: main, attrfound, meth_exit
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


/******                                                                 */
/****** Define Method                                                   */
/******                                                                 */

/* interface:
   define -c <class> -s <subclass> -t <type>
*/


#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include "cfgdebug.h"

/* search string templates */
#define SSTRING1  "type = '%s' AND class = '%s' AND subclass = '%s'"


/* external odm manipulation functions */
extern  int genseq();
extern  char *malloc();

extern int              odm_initialize();
extern int              odm_terminate();
extern struct Class    *odm_open_class();
extern int              odm_close_class();
extern void            *odm_get_list();
extern void            *odm_get_obj();
extern int              odm_rm_obj();
extern int              odm_add_obj();


/*****                                                                  */
/***** Main Function                                                    */
/*****                                                                  */

main(argc,argv,envp)
int     argc;
char    *argv[];
char    *envp[];
{
extern  int     optind;         /* for getopt function */
extern  char    *optarg;        /* for getopt function */

	char    *class,*subclass,*type;         /* parameter pointers */
	char    sstring[MAX_ODMI_CRIT];         /* search string */
	char    lname[NAMESIZE];

	struct  Class   *predev,  /* handle for predefined devices class */
			*cusdev;  /* handle for customized devices class */

	struct  PdDv    PdDv;   /* predefined device object storage  */
	struct  CuDv    CuDv;   /* customized device object storage  */

	int     rc;
	int     errflg,c;       /* used in parsing parameters        */


	/*****                                                          */
	/***** Parse Parameters                                         */
	/*****                                                          */

	errflg = 0;
	class = subclass = type = NULL;

	while ((c = getopt(argc,argv,"c:s:t:")) != EOF) {
		switch (c) {
		case 'c':
			if (class != NULL)
				errflg++;
			class = optarg;
			break;
		case 's':
			if (subclass != NULL)
				errflg++;
			subclass = optarg;
			break;
		case 't':
			if (type != NULL)
				errflg++;
			type = optarg;
			break;
		default:
			errflg++;
		}
	}
	if (errflg) {
		/* error parsing parameters */
		DEBUG_0("defdlc: command line error\n");
		exit(E_ARGS);
	}

	/*****                                                          */
	/***** Validate Parameters                                      */
	/*****                                                          */

	/* class, subclass, and type should always be given */
	if (class==NULL || subclass==NULL || type==NULL)
	{
		DEBUG_0("defdlc: class, subclass, and type names must be specified\n");
		exit(E_TYPE);
	}

	DEBUG_0 ("Defining device: \n")
	DEBUG_1 ("     class      = %s\n",class)
	DEBUG_1 ("     subclass   = %s\n",subclass)
	DEBUG_1 ("     type       = %s\n",type)

	/* start up odm */
	if ((rc = odm_initialize()) == -1)
	{
		DEBUG_0("defdlc: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}


	/* lock the database */
	if ((rc = odm_lock("/etc/objrepos/config_lock",0)) == -1)
	{
		DEBUG_0("defdlc: odm_lock() failed\n")
		err_exit(E_ODMLOCK);
	}
	DEBUG_0 ("ODM initialized and locked\n")

	/* open object class */
	if ((predev=odm_open_class(PdDv_CLASS))==-1)
	{
		/* open failed */
		DEBUG_0("defdlc: open of PdDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/* get predefined object for this device */
	sprintf(sstring,SSTRING1,type,class,subclass);

	rc = (int) odm_get_obj(predev,sstring,&PdDv,ODM_FIRST);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("defdlc: No PdDv object\n")
		err_exit(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("defdlc: ODM error getting PdDv object\n")
		err_exit(E_ODMGET);
	}

	/* there are no parents or connections */

	DEBUG_0 ("base objects found\n")

	if ((int) (cusdev=odm_open_class(CuDv_CLASS))== -1)
	{
		/* open failed */
		DEBUG_0("defdlc: open of CuDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/* generate logical name for device */
	/* there is one and only one, so minor number is not used in name*/
	sprintf(lname,"%s",PdDv.prefix);

	sprintf(sstring,"name = '%s'",lname);
	rc = (int) odm_get_obj(cusdev,sstring,&CuDv,ODM_FIRST);
	if (rc == -1)
	{
		/* ODM error */
		DEBUG_0("defdlc: ODM error getting CuDv object\n")
		err_exit(E_ODMGET);
	}
	else if (rc != 0)
	{
		/* Device name already in use */
		DEBUG_0("defdlc: Specified name already in use\n")
		err_exit(E_ALREADYDEF);
	}


	/*****                                                          */
	/***** Create Customized Object(s) for this device              */
	/*****                                                          */


	/* create a new customized device object ouput in COLON fmt */
	DEBUG_1 ("creating customized object: %s\n",lname)
	strcpy(CuDv.name,lname);
	CuDv.status    = DEFINED;
	CuDv.chgstatus = PdDv.chgstatus;
	strcpy(CuDv.ddins,PdDv.DvDr);
	CuDv.parent[0]    = '\0';
	CuDv.connwhere[0] = '\0';
	strcpy(CuDv.PdDvLn_Lvalue,PdDv.uniquetype);
	CuDv.location[0]  = '\0';

	/* add customized object to customized devices object class */
	if ((rc = odm_add_obj(CuDv_CLASS,&CuDv)) == -1)
	{
		/* error: add failed */
		DEBUG_0("defdlc: ODM error creating CuDv object\n")
		err_exit(E_ODMADD);
	}
	DEBUG_1 ("created customized device: %s\n",lname)

	if (odm_close_class(CuDv_CLASS) == -1) {
		/* ODM error */
		DEBUG_0("defdlc: failure closing CuDv object class\n")
		err_exit(E_ODMCLOSE);
	}

	if (odm_close_class(PdDv_CLASS) == -1) {
		/* ODM error */
		DEBUG_0("defdlc: failure closing PdDv object class\n")
		err_exit(E_ODMCLOSE);
	}

	(void) odm_terminate();
	fprintf(stdout, "%s",lname);
	exit(E_OK);

}


/*
 * NAME: err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *           back out on an error.
 *
 * RETURNS:
 *               None
 */

err_exit(exitcode)
char    exitcode;
{
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	odm_terminate();
	exit(exitcode);
}
