static char sccsid[] = "@(#)09  1.14  src/bos/usr/lib/methods/define/define.c, cfgmethods, bos411, 9428A410j 9/19/91 14:25:25";
/*
 * COMPONENT_NAME: (CFGMETH) define.c - Generic Define Method
 *
 * FUNCTIONS: main(), err_exit()
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

/* interface:
   define -c <class> -s <subclass> -t <type> -p <parent> -w <connection>
	  -l <device name> -u -n -o -k

   The -u flag indicates that USER specified names (via -l <device name>)
   is not allowed.
   The -n flag indicates that the device has NO parent or parent connection.
   The -o flag indicates that only ONE device of this type can be defined.
   NOTE: The -o flag also does not allow user specified names.
   The -k flag indicates that the device is only to be defined if the
   specified parent connection does not already have a device defined for it.

   The -c, -s, -t, -p, -w, and -l flags along with their corresponding
   values are supplied by the invoker of the define method.  The -u, -n,
   -o, and -k flags are picked up along with the method name from the Define
   Method descriptor in PdDv.
*/


#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include "cfgdebug.h"


/* external odm manipulation functions */
extern  int genseq();

/*****									*/
/***** Main Function							*/
/*****									*/

main(argc,argv,envp)
int	argc;
char	*argv[];
char	*envp[];

{
extern  int     optind;         /* for getopt function */
extern  char    *optarg;        /* for getopt function */


char	*class,*subclass,*type,		/* parameter pointers */
	*parent,*connect,*lname;
char	sstring[256];			/* search string */
char    parloc[LOCSIZE],                /* parents location */
	parut[UNIQUESIZE];              /* parents unique type */
char    tempname[NAMESIZE];     /* Used in generating device name */

struct  Class   *predev,        /* handle for predefined devices class    */
		*cusdev;        /* handle for customized devices class    */

struct	PdDv	PdDv;
struct	PdCn	PdCn;
struct	CuDv	CuDv;

int     seqno;
int	rc;
int     errflg,c;               /* used in parsing parameters */
int     uflag;                  /* User specified name not allowed */
int     nflag;                  /* Device does not have parent/connection */
int     oflag;                  /* Only one device of this type allowed */
int     kflag;                  /* Only one device per parent connection */

	/*****								*/
	/***** Parse Parameters 					*/
	/*****								*/
	errflg = 0;
	class = subclass = type = parent = connect = lname = NULL;
	uflag = 0;
	nflag = 0;
	oflag = 0;
	kflag = 0;

	while ((c = getopt(argc,argv,"c:s:t:p:w:l:unok")) != EOF) {
		switch (c) {
		case 'c':                       /* Device class */
			if (class != NULL)
				errflg++;
			class = optarg;
			break;
		case 's':                       /* Device subclass */
			if (subclass != NULL)
				errflg++;
			subclass = optarg;
			break;
		case 't':                       /* Device type */
			if (type != NULL)
				errflg++;
			type = optarg;
			break;
		case 'p':                       /* Parent device name */
			if (parent != NULL)
				errflg++;
			parent = optarg;
			break;
		case 'w':                       /* Connection location on */
			if (connect != NULL)    /* parent */
				errflg++;
			connect = optarg;
			break;
		case 'l':                       /* User specified device */
			if (lname != NULL)      /* name */
				errflg++;
			lname = optarg;
			break;
		case 'u':               /* User specified names not allowed */
			uflag++;
			break;
		case 'n':               /* Device does not have a parent */
			nflag++;        /* or connection */
			break;
		case 'o':               /* Only one device of this type can */
			oflag++;        /* be defined */
			uflag++;        /* User name not allowed */
			break;
		case 'k':               /* Device can not be defined if a */
			kflag++;        /* device is already defined at */
			break;          /* specified parent connection */
		default:
			errflg++;
		}
	}
	if (errflg) {
		/* error parsing parameters */
		DEBUG_0("define: command line error\n");
		exit(E_ARGS);
	}

	/*****								*/
	/***** Validate Parameters					*/
	/*****								*/
	/* class, subclass, and type must be specified */
	if (class==NULL || subclass==NULL || type==NULL) {
		DEBUG_0("define: class, subclass, and type names must be specified\n");
		exit(E_TYPE);
	}

	/* parent and connection must be specified except when -n is used */
	if (nflag) {
		if (parent != NULL || connect != NULL) {
			DEBUG_0("define: -p and -w not to be used with this device type\n");
			exit(E_PARENT2);
		}
	} else {
		if (parent == NULL || connect == NULL) {
			DEBUG_0("define: parent and connection must to be specified\n");
			exit(E_PARENT);
		}
	}

	/* User specified name not allowed in some cases */
	if (lname!=NULL && uflag!=0) {
		DEBUG_0("define: Device name cannot be specified for this type\n");
	     /*   exit(E_UNAME);   */
		exit(E_LNAME);
	}

	DEBUG_0 ("Defining device: \n")
	DEBUG_1 ("     class      = %s\n",class)
	DEBUG_1 ("     subclass   = %s\n",subclass)
	DEBUG_1 ("     type       = %s\n",type)
	DEBUG_1 ("     parent     = %s\n",parent)
	DEBUG_1 ("     connection = %s\n",connect)

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
		DEBUG_0("define: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}
	DEBUG_0 ("ODM initialized\n")

	/* get predefined object for this device */
	sprintf(sstring,"type = '%s' AND class = '%s' AND subclass = '%s'",
						type,class,subclass);
	rc = (int)odm_get_first(PdDv_CLASS,sstring,&PdDv);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("define: No PdDv object\n")
		err_exit(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("define: ODM error getting PdDv object\n")
		err_exit(E_ODMGET);
	}

	/* open customized device object class */
	if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1) {
		/* error opening class */
		DEBUG_0("define: open of CuDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/* Verify user supplied device name else generate a name */
	if (lname == NULL) {
		/* generate logical name for device */
		if ((seqno=genseq(PdDv.prefix))<0) {
			/* error making logical name */
			DEBUG_0("define: error making logical name\n");
			err_exit(E_MAKENAME);
		}
		if (oflag!=0 && seqno>0) {
			/* only one of this type device is allowed */
			DEBUG_0("define: Not allowed to add another device of this type\n");
			err_exit(E_ALREADYDEF);
		}
		sprintf(tempname,"%s%d",PdDv.prefix,seqno);
		lname = tempname;
	} else {
		/* Verify that name is not already in use */
		sprintf(sstring,"name = '%s'",lname);
		rc =(int)odm_get_first(cusdev,sstring,&CuDv);
		if (rc==-1) {
			/* ODM error */
			DEBUG_0("define: ODM error getting CuDv object\n")
			err_exit(E_ODMGET);
		}
		else if (rc != 0) {
			/* Device name already in use */
			DEBUG_0("define: Specified name already in use\n")
		    /*    err_exit(E_NAMEUSED);   */
			err_exit(E_LNAME);
		}
		/* Otherwise, name is just fine! */
	}

	/* Check parent device if device has a parent */
	if (nflag == 0) {
		/* get parent customized device object */
		sprintf(sstring,"name = '%s'",parent);
		rc =(int)odm_get_first(cusdev,sstring,&CuDv);
		if (rc==0) {
			/* failed to get an object */
			DEBUG_0("define: No parent CuDv object\n")
			err_exit(E_NOCuDvPARENT);
		}
		else if (rc==-1) {
			/* ODM error */
			DEBUG_0("define: ODM error getting CuDv object\n")
			err_exit(E_ODMGET);
		}
		strcpy(parloc,CuDv.location);
		strcpy(parut,CuDv.PdDvLn_Lvalue);

		sprintf(sstring,"uniquetype = '%s' and connkey = '%s' and connwhere = '%s'",
			CuDv.PdDvLn_Lvalue,subclass,connect);
		rc = (int)odm_get_first(PdCn_CLASS,sstring,&PdCn);
		if (rc==0) {
			/* invalid connection */
			DEBUG_0("define: Invalid connection\n")
			err_exit(E_INVCONNECT);
		}
		else if (rc==-1) {
			/* ODM error */
			DEBUG_0("define: ODM error getting PdDv object\n")
			err_exit(E_ODMGET);
		}

		if (kflag != 0) {
			/* see if a customized device already connected */
			/* at specified connection on specified parent */
			sprintf(sstring,"parent = '%s' AND connwhere = '%s'",
							parent,connect);
			rc =(int)odm_get_first(cusdev,sstring,&CuDv);
			if (rc==-1) {
				/* ODM error */
				DEBUG_0("define: ODM error getting CuDv object\n")
				err_exit(E_ODMGET);
			}
			if (rc) {
				/* already a device at connection */
				DEBUG_0("define: Already defined device at connection\n")
				err_exit(E_INVCONNECT);
			}
		}
	}
	DEBUG_0 ("base objects found\n")

	/* create a new customized device object */
	DEBUG_1 ("creating customized object: %s\n",lname)
	strcpy(CuDv.name,lname);
	CuDv.status      = DEFINED;
	CuDv.chgstatus   = PdDv.chgstatus;
	strcpy(CuDv.ddins,PdDv.DvDr);
	strcpy(CuDv.parent,parent);
	strcpy(CuDv.connwhere,connect);
	strcpy(CuDv.PdDvLn_Lvalue,PdDv.uniquetype);
	CuDv.location[0] = '\0';

	/* If device has a parent, fill in the location code */
	if (nflag==0) {
		location(parut,parloc,&CuDv);
		DEBUG_1 ("location = %s\n",CuDv.location)
	}

	/* add customized object to customized devices object class */
	if (odm_add_obj(cusdev,&CuDv) == -1) {
		/* error: add failed */
		DEBUG_0("define: ODM error creating CuDv object\n")
		err_exit(E_ODMADD);
	}
	DEBUG_1 ("created customized device: %s\n",lname)

	if (odm_close_class(CuDv_CLASS) == -1) {
		/* ODM error */
		DEBUG_0("define: failure closing CuDv object class\n")
		err_exit(E_ODMCLOSE);
	}

	odm_terminate();

	/* device defined successfully */
	fprintf(stdout,"%s ",lname);
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
