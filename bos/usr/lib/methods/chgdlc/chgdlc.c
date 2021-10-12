static char sccsid[] = "@(#)02  1.14  src/bos/usr/lib/methods/chgdlc/chgdlc.c, dlccfg, bos411, 9428A410j 2/4/94 11:05:47";
/*
 * COMPONENT_NAME: (DLCCFG) chgdlc.c - Change Data Link Control Method Code
 *
 * FUNCTIONS: main(), update_dev(), meth_exit()
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

/*
   Interface:

	chgdlc -l <logical name>
		 [-a attr=val [attr=val...]]

*/


/* header files needed for compilation */
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/devinfo.h>
#include <sys/device.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <nl_types.h>
#include <locale.h>
#include "pparms.h"
#include "cfgdebug.h"

/* searchstring templates */
#define SSTRING1 "uniquetype = '%s'"

/* miscellaneous defines */



/* external functions */
extern  char  *malloc();
extern  char  *strtok();
extern  char  *realloc();
extern  int   check_parms();

extern int              odm_initialize();
extern int              odm_terminate();
extern struct Class    *odm_open_class();
extern int              odm_close_class();
extern void            *odm_get_obj();
extern int              odm_rm_obj();
extern int              odm_add_obj();


/* global variables */
int no_atts;
char    allattrs[2048];         /* complete list of attr=value parameters */
int     Pflag;                  /* flag for -P param (i.e, "only change db")*/


/*****                                                                  */
/***** Main Function                                                    */
/*****                                                                  */

main(argc,argv)
int     argc;
char    *argv[];
{
extern  int     optind;         /* for getopt function */
extern  char    *optarg;        /* for getopt function */

	struct  attr *attrs;            /* list of changed attributes */

	struct  Class   *cusdev,        /* customized devices class pointer */
			*predev;        /* predefined devices class pointer */

	struct  CuDv    cusobj;         /* device customized device object  */
	struct  PdDv    preobj;         /* predefined device object         */

	char    *lname;                 /* logical name    */
	char    *badattr;       /* pointer to list of invalid attributes */
	char    *badattlist[512]; /* Space for check_parms to report badattr */

	char    sstring[256];           /* searchstring space */

	int     errflg,c;               /* used in parsing parameters */
	int     Tflag;                  /* flags for -T parameters */
	int     rc;                     /* return code */

	long    majorno, minorno, *minor_list;
				       /* Major/minor for device */
	int     how_many;               /* required for getminor */
	int     oldstatus;              /* Status of device at program commencement */
	char    *out_p;                 /* Message from config method */
	int     attrno;                 /* Used when restoring attributes */

	struct  CuAt    *old_attrs;     /* Attributes for device before being changed*/
	struct  objlistinfo list_info;  /* Info about old_attrs */


	/* set up initial variable values */
	attrs = (struct attr *) NULL;
	allattrs[0] = '\0';
	lname = NULL;
	no_atts = 0;
	errflg = 0;
	Pflag = 0;
	Tflag = 0;


	/* parse command line parameters */
	while ((c = getopt(argc,argv,"l:a:PT")) != EOF) {
		switch (c) {
		case 'l':
			if (lname != NULL)
				errflg++;
			lname = optarg;
			break;
		case 'a':
			strcat(allattrs,optarg);
			strcat(allattrs," ");
			if (add_attr(optarg,&attrs))
				errflg++;
			break;
		case 'P':
			if (Tflag)
				errflg++;
			else
				Pflag++;
			break;
		case 'T':
			if (Pflag)
				errflg++;
			else
				Tflag++;
			break;
		default:
			errflg++;
		}
	}
	if (errflg) {
		/* error parsing parameters */
		DEBUG_0("chgdlc: command line error\n")
		exit(E_ARGS);
	}


	/* logical name must be specified */
	if (lname==NULL)
	{
		DEBUG_0("chgdlc: logical name must be specified\n")
		exit(E_LNAME);
	}

	/* The -T flag not supported by any device using this code */
	if (Tflag) {
		DEBUG_0("chgdlc: -T is not supported for this device\n")
		exit(E_TFLAG);
	}

	/* initialize the ODM */
	rc = odm_initialize();
	if (rc == -1)
	{
		DEBUG_0("chgdlc: odm_initialize failed\n")
		exit(E_ODMINIT);
	}

	/* Lock the database */
	if (odm_lock("/etc/objrepos/config_lock",0) == -1)
	{
		DEBUG_0("chgdlc: odm_lock() failed\n")
		err_exit(E_ODMLOCK);
	}

	/*******************************************************************
	   Begin by validating the logical name passed in from the command
	   line. This is done by searching the customized devices object
	   class for this logical name.
	 *******************************************************************/


	/* open customized devices object class (CusDevices) */
	if ((int) (cusdev=odm_open_class(CuDv_CLASS))== -1)
	{
		/* open failed */
		DEBUG_0("chgdlc: open class CuDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/* find customized object */
	sprintf(sstring,"name = '%s'",lname);


	rc =(int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0)
	{
		DEBUG_0("chgdlc: Device does not exist\n")
		err_exit(E_NOCuDv);
	}
	else if (rc == -1)
	{
		/* odm error occurred */
		DEBUG_0("chgdlc: ODM error getting CuDv object\n")
		err_exit(E_ODMGET);
	}

	/* Read the current attributes for the device */
	if((int)(old_attrs=odm_get_list(CuAt_CLASS,sstring,&list_info,20,1))
		==-1) {
		DEBUG_1("Error reading CuAt with %s",sstring)
		err_exit(E_ODMGET);
	}

	/* if attributes are being changed, call device specific routine
	   to make sure attributes are correct */
	if (attrs!=NULL)
	{
		badattlist[0] = '\0';

		rc = check_parms(attrs,Pflag,(int)0,lname,
			badattlist);

		if (rc > 0)
		{
			DEBUG_0("chgdlc: check_parms() error\n")
			if( badattlist[0] != '\0' )
				list_badattr( cusobj.PdDvLn_Lvalue,badattlist);
			err_exit(rc);
		}
	}
	else    /* Nothing to do */
		exit (0);

	/******************************************
	  Validate attributes, if any were passed.
	 *******************************************/
	if (allattrs[0] != '\0')
	{
		DEBUG_1 ("chgdlc: Attributes to validate: %s\n",allattrs)
		if (attrval(cusobj.PdDvLn_Lvalue,allattrs,&badattr) > 0)
		{
			list_badattr( cusobj.PdDvLn_Lvalue, badattr );
			err_exit(E_ATTRVAL);
		}
	}

	/* get device's predefined object */
	sprintf(sstring,"uniquetype = '%s'",cusobj.PdDvLn_Lvalue);
	rc = (int)odm_get_first(PdDv_CLASS,sstring,&preobj);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("chgdlc: No PdDv object\n")
		err_exit(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("chgdlc: ODM error getting PdDv object\n")
		err_exit(E_ODMGET);
	}


	/* If the device is configured and the -P flag was not specified, */
	/* then need to unconfigure the device before updating database.  */
	/* One exception is the sys device which must NOT be unconfigured.*/

	if ( ((oldstatus=cusobj.status) != DEFINED) && (!Pflag) )
	{
		/* In this case we WILL be unconfiguring,       */
		/* & reconfiguring the device.                  */

		sprintf( sstring, "-l %s", cusobj.name );
		rc = odm_run_method(preobj.Unconfigure, sstring, &out_p,NULL);
		if (rc == -1) {
			DEBUG_0("chgdlc: odm_run_method failure\n")
			err_exit(E_ODMRUNMETHOD);
		}
		else if (rc != 0) {
			DEBUG_0("chgdlc: device unconfigure failed\n")
			err_exit(rc);
		}
	}


	/* update the device's attributes with new the new values */
	if (attrs!=NULL)
	{
		rc = update_dev(&cusobj,attrs);
		if (rc != 0) {
			DEBUG_0("chgdlc: Failed in updating attributes\n")
			err_exit(rc);
		}
	}


	/*****************************************************************
	 If device was configured when beginning the change and the Pflag
	 was not specified, then need to re-configure the device.
	 *****************************************************************/

	if ( (oldstatus != DEFINED) && (!Pflag) )
	{
		sprintf( sstring, "-l %s", cusobj.name );
		rc = odm_run_method(preobj.Configure, sstring, &out_p, NULL);
		if (rc == -1) {
			DEBUG_0("chgdlc: odm_run_method failure\n")
			err_exit(E_ODMRUNMETHOD);
		}
		else if (rc != 0) {
			DEBUG_0("chgdlc: device reconfigure failed\n")
			DEBUG_0("chgdlc: restoring attributes\n")

			/* Restore attributes to previous state:*/
			sprintf(sstring,"name = '%s'", lname );
			if (odm_rm_obj(CuAt_CLASS,sstring) == -1) {
				DEBUG_0("chgdlc: ODM error while restoring attributes\n")
				err_exit(E_ODMDELETE);
			}

			for( attrno=0; attrno<list_info.num; attrno++ ) {
				if (odm_add_obj(CuAt_CLASS,&old_attrs[attrno]) == -1) {
					DEBUG_0("chgdlc: ODM error while restoring attributes\n")
					err_exit(E_ODMADD);
				}
			}
			err_exit(rc);   /* Exit with configure method's error */
		}
	}   /* end if device is configured */


	/* clean up and return successful */
	if (odm_close_class(cusdev) == -1)
	{
		/* error closing class */
		DEBUG_0("chgdlc: Failure closing CuDv object class\n");
		err_exit(E_ODMCLOSE);

	}


	/***********************************************************
	  config method is finished at this point. Terminate the
	  ODM, and exit with a good return code.
	  ***********************************************************/
	odm_terminate();
	exit(0);

}



/*****                                                                  */
/***** Subroutines and Functions                                        */
/*****                                                                  */
/* error exit routine */
err_exit(exit_code)
int     exit_code;              /* Error exit code */
{
	odm_close_class(CuDv_CLASS);
	odm_close_class(CuAt_CLASS);
	odm_close_class(PdAt_CLASS);
	odm_terminate();
	exit(exit_code);
}

int
add_attr(attrstr,attrlist)
char    *attrstr;
struct  attr **attrlist;
{
struct  attr *attrl;
char    *name,*val,*str;

	str = attrstr;
	attrl = *attrlist;

	/* parse up attribute string */
	while ((name = strtok(str,"=")) != NULL) {

		/* get space */
		if ((attrl = (struct attr *)
			realloc(attrl,(no_atts+2)*sizeof(struct attr)))==NULL)
			return(E_MALLOC);

		/* set up attribute name field */
		attrl[no_atts].attribute = name;

		/* set up value string */
		if ((val = strtok(NULL," "))==NULL)
			attrl[no_atts].value = (char *)(strchr(name,'\0')+1);
		else
			attrl[no_atts].value = val;

		/* finish off tail of array */
		no_atts++;
		attrl[no_atts].attribute  = NULL;
		attrl[no_atts].value      = NULL;

		/* so we keep going from where we left off */
		str = NULL;

	}

	*attrlist = attrl;

	return(0);

}




int
update_dev(cusobj,attrs)
struct  CuDv *cusobj;
struct  attr *attrs;
{
	struct  Class   *preatt,   /* customized attribute object class   */
			*cusatt;   /* customized attribute object class */
	struct  CuAt    cuatobj;   /* customized attribute object */
	struct  PdAt    pdatobj;   /* predefined attribute object */
	char    sstring[256];      /* search string space  */
	int     rc;


	/***************************************************
	   Update changed attributes and connection/parent
	   information the proper object classes.
	 ***************************************************/


	/* open CuAt object class */
	if ((int) (cusatt=odm_open_class(CuAt_CLASS))==-1)
	{
		/* error: open failed */
		DEBUG_0("Error Opening CuAt\n")
		return(E_ODMOPEN);
	}

	/* open PdAt object class */
	if ((int) (preatt=odm_open_class(PdAt_CLASS))==-1)
	{
		/* error: open failed */
		DEBUG_0("Error Opening PdAt\n")
		return(E_ODMOPEN);
	}


	/* update customized attribute object class */
	do {
		/* get predefined attribute */
		sprintf(sstring,"uniquetype = '%s' AND attribute = '%s'",
				cusobj->PdDvLn_Lvalue,attrs->attribute);

		rc = (int)odm_get_first(preatt,sstring,&pdatobj);
		if (rc == -1) {
			DEBUG_1("Error reading PdAt where %s\n",sstring)
			return(E_ODMGET);
		}
		else if (rc ==0 ) { /* No predefined */
			DEBUG_1("No records in PdAt where %s\n",sstring)
			return(E_NOATTR);
		}

		/* get customized attribute (if there is one) */
		sprintf(sstring,"name = '%s' AND attribute = '%s'",
				cusobj->name ,attrs->attribute);

		rc = (int)odm_get_first(cusatt,sstring,&cuatobj);
		if (rc == -1) {
			DEBUG_1("Error reading CuAt where %s\n",sstring)
			return(E_ODMGET);
		}


		/* decide what to do */
		if ( rc == 0 ) {        /* There is NO customized attribute */
			if( strcmp(attrs->value,pdatobj.deflt) == 0) {

/* CASE 1: No CuAt and new value = default, so do nothing.                  */
				DEBUG_1("No customized %s, and new value = default\n",
					attrs->attribute );
			}
			else {
/* CASE 2: No CuAt and new value != default, so add new CuAt object.        */
				DEBUG_1("No customized %s, and new value != default\n",
					attrs->attribute );

				strcpy(cuatobj.name,cusobj->name);
				strcpy(cuatobj.type,pdatobj.type);
				strcpy(cuatobj.generic,
						pdatobj.generic);
				strcpy(cuatobj.rep,pdatobj.rep);
				cuatobj.nls_index = pdatobj.nls_index;
				strcpy(cuatobj.attribute,
						pdatobj.attribute);
				strcpy(cuatobj.value,attrs->value);

				/*add this object to CuAt object class*/
				if (odm_add_obj(cusatt,&cuatobj) == -1) {
					return(E_ODMADD);
				}
			}
		}

		else {          /* There IS a customized attribute */

/* CASE 3: Is a CuAt and new value = default, so delete CuAt object.       */
			if( strcmp( attrs->value, pdatobj.deflt )==0) {
				DEBUG_1("There was customized %s, and new value = default\n",
					attrs->attribute );

				/* delete CuAt object */
				if (odm_rm_obj(cusatt,sstring) == -1) {
					return(E_ODMDELETE);
				}
			}
			else {

/* CASE 4: Is a CuAt and new value != default, so update CuAt object.      */
				DEBUG_1("There was customized %s, and new value != default\n",
					attrs->attribute );

				/* update CuAt object */
				strcpy(cuatobj.value,attrs->value);
				if (odm_change_obj(cusatt,&cuatobj) == -1) {
					return(E_ODMUPDATE);
				}
			}
		}

		attrs++;

	} while (attrs->attribute!=NULL);


	/* close object classes */
	if (odm_close_class(cusatt) == -1) {
		/* error: close failed */
		DEBUG_0("chgdlc: Failure closing CuAt object class\n")
		return(E_ODMCLOSE);
	}
	if (odm_close_class(preatt) == -1) {
		/* error: close failed */
		DEBUG_0("chgdlc: Failure closing PdAt object class\n")
		return(E_ODMCLOSE);
	}


	return(0);

}

list_badattr( uniquetype, badattr )
char    *uniquetype;
char    *badattr;
{
	char    *attrname;
	char    sstring[100];
	struct  PdDv    PdDvobj;
	struct  PdAt    PdAtobj;
	int     rc;
	char    *msgptr;
	nl_catd catd;

	sprintf( sstring, "uniquetype = %s", uniquetype );
	rc = (int)odm_get_first(PdDv_CLASS,sstring,&PdDvobj);
	if (rc==0) {
		DEBUG_1("No PdDv entry with %s\n",sstring);
		err_exit(E_NOCuDv);
	}
	else if (rc == -1) {
		DEBUG_1("odm_get_first error using %s\n",sstring);
		err_exit(E_ODMGET);
	}

	/* Loop through bad attribute names: */

	for( attrname = strtok(badattr,", ");
		/* Check not at end */
		attrname != (char *)NULL;
		/* Find next */
		attrname = strtok( (char *)NULL, ", ") ) {

		/* Find the PdAt entry for the attribute */
		sprintf( sstring, "uniquetype = %s AND attribute = %s",
			uniquetype, attrname );
		rc = (int)odm_get_first(PdAt_CLASS,sstring,&PdAtobj);
		if (rc==0) {
			DEBUG_1("No PdAt entry with %s\n",sstring);
			fprintf(stderr,"     %s\n");
			err_exit(E_INVATTR);
		}
		else if (rc == -1) {
			DEBUG_1("odm_get_first error using %s\n",sstring);
			err_exit(E_ODMGET);
		}
		else {
		DEBUG_3( "Printing NLS version of cat %s, set %d, msg %d\n",
					PdDvobj.catalog, PdDvobj.setno,
					PdAtobj.nls_index )

			(void) setlocale (LC_ALL, "");
/* <<< defect 116118 >>> */
			catd = catopen (PdDvobj.catalog, NL_CAT_LOCALE);
/* <<< end defect 116118 >>> */
			if (catd == -1)
			   err_exit(E_NOPdDv);

			msgptr = catgets (catd, PdDvobj.setno,
					  PdAtobj.nls_index, "");
			/* the message returned may be an error message */

			fprintf (stderr, "     %s     %s\n",
				 attrname, msgptr);
			catclose (catd);
		}
	}
}
