static char sccsid[] = "@(#)08  1.27  src/bos/usr/lib/methods/common/chgdevice.c, cfgmethods, bos411, 9428A410j 5/12/94 09:29:18";
/*
 * COMPONENT_NAME: (CFGMETHODS) chgdevice.c - Generic Change Method Code
 *
 * FUNCTIONS: main(), update_dev(), err_exit()
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

/*
   Interface:

	chgdevice -l <logical name> [-p <parent>] [-w <location>]
		  [-a attr=val [attr=val...]] [-P | -T]

*/


/* header files needed for compilation */
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <strings.h>
#include <cf.h>

#include <locale.h>

#include <nl_types.h>
#define DEFAULT_DEVICES_CAT_DIR		"/usr/lib/methods"

#include "pparms.h"
#include "cfgdebug.h"


/* searchstring templates */
#define	SSTRING1 "uniquetype = '%s' AND connkey = '%s' AND connwhere = '%s'"
#define	SSTRING2 "parent = '%s' AND connwhere = '%s' AND status != %d"


/* external functions */
extern	char  *malloc();
extern	char  *realloc();
extern	int   check_parms();
	

/* global variables */
int no_atts;
char    allattrs[2048];         /* complete list of attr=value parameters */
int	Pflag;			/* flag for -P param (i.e, "only change db")*/


/*****									*/
/***** Main Function							*/
/*****									*/

main(argc,argv)
int	argc;
char	*argv[];
{
extern	int	optind;		/* for getopt function */
extern	char	*optarg;	/* for getopt function */

struct	attr	*attrs;		/* list of changed attributes */

struct	Class	*cusdev,	/* customized devices class pointer */
		*precon,	/* predefined connection class pointer */
		*predev,	/* predefined devices class pointer */
		*cusdep;	/* customized dependency class hdl */

struct	CuDv	cusobj,		/* device customized device object  */
		parobj,		/* parent customized device object  */
		childobj;	/* child customized device object */
struct	PdDv	preobj;		/* predefined device object         */
struct	PdCn	pcnobj;		/* dummy predefined connection object */
struct	CuDep	cusdepobj;	/* cust. dependency object */


char	*lname,*parent,*loc;	/* logical name, parent name & location */
char	*badattr;		/* pointer to list of invalid attributes */
char	*badattlist[512];	/* Space for check_parms to report bad attr */

char	sstring[256];		/* searchstring space */

int	errflg,c;		/* used in parsing parameters */
int     Tflag;                  /* flag for -T parameter */
int	new_location;		/* Flag used to tell if should generate new
				   location code. */
int	rc;			/* return code */
int	attrs_present = FALSE;	/* flag to say attrs are being parsed */

long	majorno, minorno, *minor_list;
				/* Major/minor for device */
int	how_many;		/* required for getminor */
int	oldstatus;		/* Status of device at program commencement */
char	*out_p;			/* Message from config method */
int     attrno;                 /* Used when restoring attributes */

struct	CuAt	*old_attrs;	/* Attributes for device before being changed*/
struct	objlistinfo list_info;	/* Info about old_attrs */

	/* set up initial variable values */
	attrs = (struct attr *) NULL;
	allattrs[0] = '\0';
	lname = parent = loc = NULL;
	no_atts = 0;
	errflg = 0;
	Pflag = 0;
	Tflag = 0;

	/* parse command line parameters */
	while ((c = getopt(argc,argv,"l:a:p:w:PT")) != EOF) {
		switch (c) {
		case 'l':
			if (lname != NULL)
				errflg++;
			lname = optarg;
			break;
		case 'a':
			if (attrs_present == TRUE)
				strcat(allattrs," ");
			strcat(allattrs,optarg);
			rc = add_attr(optarg,&attrs);
			if (rc != 0)
				exit(rc);
			attrs_present = TRUE;
			break;
		case 'p':
			if (parent != NULL)
				errflg++;
			parent = optarg;
			break;
		case 'w':
			if (loc != NULL)
				errflg++;
			loc = optarg;
			break;
		case 'P':
			Pflag++;
			break;
		case 'T':
			Tflag++;
			break;
		default:
			errflg++;
		}
	}

	if (errflg) {
		/* error parsing parameters */
		DEBUG_0("chgdevice: command line error\n")
		exit(E_ARGS);
	}

	/* logical name must be specified */
	if (lname==NULL) {
		DEBUG_0("chgdevice: logical name must be specified\n")
		exit(E_LNAME);
	}

	/* The -T flag not supported by any device using this code */
	if (Tflag) {
		DEBUG_0("chgdevice: -T is not supported for this device\n")
		exit(E_TFLAG);
	}

	/* Initialize the ODM */
	if (odm_initialize() == -1) {
		DEBUG_0("chgdevice: odm_initialize failed\n")
		exit(E_ODMINIT);
	}

	/*******************************************************************
	   Begin by validating the logical name passed in from the command
	   line. This is done by searching the customized devices object
	   class for this logical name.
	 *******************************************************************/

	/* open customized devices object class (CuDv) */
	if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1) {
		/* error: open failed */
		DEBUG_0("chgdevice: open class CuDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/* find customized object */
	sprintf(sstring,"name = '%s'",lname);
	rc =(int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("chgdevice: Device does not exist\n")
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("chgdevice: ODM error getting CuDv object\n")
		err_exit(E_ODMGET);
	}

	/* Read the current attributes for the device */
	if((int)(old_attrs=odm_get_list(CuAt_CLASS,sstring,&list_info,20,1))
		==-1) {
		DEBUG_1("Error reading CuAt with %s",sstring)
		err_exit(E_ODMGET);
	}

	/* see if parent and/or connection really changed   */
	if (parent != NULL)
		if (!strcmp(parent,cusobj.parent))
			parent = NULL;

	if (loc != NULL)
		if (!strcmp(loc,cusobj.connwhere))
			loc = NULL;

	if( ( loc == NULL ) && ( parent == NULL ) )
		new_location = 0;
	else {
		/* Set Generate New Location Flag */
		new_location = 1;

		/* We need EITHER loc==NULL && parent==NULL, */
		/* OR loc!=NULL & parent!=NULL */

		if ( parent == NULL )
			parent = cusobj.parent;

		if( loc == NULL )
			loc = cusobj.connwhere;
	}
	

	/* If attributes are being changed, call device specific routine
	   to make sure attributes are correct */
	if ( ( attrs!=NULL ) || new_location) {

		badattlist[0] = '\0';

		rc = check_parms(attrs,Pflag,(int)0,lname,parent,loc,
			badattlist);

		if (rc > 0) {
			DEBUG_0("chgdevice: check_parms() error\n")
			if( badattlist[0] != '\0' )
				list_badattr( cusobj.PdDvLn_Lvalue,badattlist);
			err_exit(rc);
		}
	}
	else	/* Nothing to do */
		exit (0);


	/******************************************
	  Validate attributes, if any were passed.
	 *******************************************/

	if (allattrs[0] != '\0') {
		DEBUG_1 ("chgdevice: Attributes to validate: %s\n",allattrs)
		if (attrval(cusobj.PdDvLn_Lvalue,allattrs,&badattr) > 0) {
			list_badattr( cusobj.PdDvLn_Lvalue, badattr );
			err_exit(E_ATTRVAL);
		}
	}

	/* get device's predefined object */
	sprintf(sstring,"uniquetype = '%s'",cusobj.PdDvLn_Lvalue);
	rc = (int)odm_get_first(PdDv_CLASS,sstring,&preobj);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("chgdevice: No PdDv object\n")
		err_exit(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("chgdevice: ODM error getting PdDv object\n")
		err_exit(E_ODMGET);
	}

	/*******************************************************************
	   Validate other command line parameters.

	   If the device is being moved to a different parent and/or con-
	   nection location, then make sure that the connection location
	   exists on the current/new parent. Also, if the parent is being
	   changed, make sure that it actually exists.
	 ******************************************************************/

	if ( new_location ) {

		/* get the current/new customized parent object */
		sprintf(sstring,"name = '%s'",parent);
		rc =(int)odm_get_first(cusdev,sstring,&parobj);
		if (rc==0) {
			/* failed to get an object */
			DEBUG_0("chgdevice: No parent CuDv object\n")
			err_exit(E_NOCuDvPARENT);
		}
		else if (rc==-1) {
			/* ODM error */
			DEBUG_0("chgdevice: ODM error getting CuDv object\n")
			err_exit(E_ODMGET);
		}

		/* make sure that location exists on (new) parent */
		sprintf(sstring,SSTRING1,parobj.PdDvLn_Lvalue,preobj.subclass,
			loc);
		rc = (int)odm_get_first(PdCn_CLASS,sstring,&pcnobj);
		if (rc==0) {
			/* Not a valid connection */
			DEBUG_0("chgdevice: connection not valid\n")
			err_exit(E_INVCONNECT);
		}
		else if (rc==-1) {
			/* ODM error */
			DEBUG_0("chgdevice: ODM error getting PdCn object\n")
			err_exit(E_ODMGET);
		}


		/* If device is AVAILABLE, then make sure that no other
		   devices are configured at the new location. */

		if ((cusobj.status!=DEFINED ) &&
			( strncmp(cusobj.PdDvLn_Lvalue,"sys/node/", 9) != 0 )) {
			sprintf(sstring,SSTRING2,parent,loc,DEFINED);
			rc = (int)odm_get_first(cusdev,sstring,&childobj);
			if (rc==-1) {
				/* ODM error */
				DEBUG_0("chgdevice: ODM error getting CuDv object\n")
				err_exit(E_ODMGET);
			}
			else if (rc!=0) {
				/* There is another AVAILABLE device here */
				DEBUG_0("chgdevice: connection in use\n")
				err_exit(E_AVAILCONNECT);
			}
		}
	}

	/*******************************************************
	 Note: May add checking for dependencies here.  For now
	 it is assumed that it is OK to attempt changing a device
	 that is listed in the CuDep object class as being a
	 dependent to another device.  If this device is in use
	 by the other device then this change will fail when it tries
	 to unconfigure the device.  If the device is not in use, then
	 what does the other device care?
	 *******************************************************/


	/* If the device is configured and the -P flag was not specified, */
	/* then need to unconfigure the device before updating database.  */
	/* One exception is the sys device which must NOT be unconfigured.*/

	if ( ((oldstatus=cusobj.status) != DEFINED) && (!Pflag)
		&& (strncmp(cusobj.PdDvLn_Lvalue,"sys/node/", 9)!=0) ) {

		/* In this case we WILL be unconfiguring,	*/
		/* & reconfiguring the device.                  */

		sprintf( sstring, "-l %s", cusobj.name );
		rc = odm_run_method(preobj.Unconfigure, sstring, &out_p,NULL);
		if (rc == -1) {
			DEBUG_0("chgdevice: odm_run_method failure\n")
			err_exit(E_ODMRUNMETHOD);
		}
		else if (rc != 0) {
			DEBUG_0("chgdevice: device unconfigure failed\n")
			err_exit(rc);
		}
	}

	/* Check to see if should generate new location code */
	if (new_location) {

		/* Need to get fresh copy of customized object    */
		/* since the unconfigure method may have been run */
		sprintf(sstring,"name = '%s'",lname);
		rc =(int)odm_get_first(cusdev,sstring,&cusobj);
		if (rc==0) {
			/* failed to get an object */
			DEBUG_0("chgdevice: Device does not exist\n")
			err_exit(E_NOCuDv);
		}
		else if (rc==-1) {
			/* ODM error */
			DEBUG_0("chgdevice: ODM error getting CuDv object\n")
			err_exit(E_ODMGET);
		}

		strcpy(cusobj.parent,parent);
		strcpy(cusobj.connwhere,loc);

		location(parobj.PdDvLn_Lvalue,parobj.location,&cusobj);

		/* update device object */
		if (odm_change_obj(cusdev,&cusobj) == -1) {
			/* error: change failed */
			DEBUG_0("chgdevice: Update of CuDv object failed\n")
			err_exit(E_ODMUPDATE);
		}
	}

	/* update the device's attributes with new the new values */
	if (attrs!=NULL) {
		rc = update_dev(&cusobj,attrs);
		if (rc != 0) {
			DEBUG_0("chgdevice: Failed in updating attributes\n")
			err_exit(rc);
		}
	}


	/*****************************************************************
	 If device was configured when beginning the change and the Pflag
	 was not specified, then need to re-configure the device.
	 *****************************************************************/

	if ( (oldstatus != DEFINED) && (!Pflag) ) {
		sprintf( sstring, "-l %s", cusobj.name );
		rc = odm_run_method(preobj.Configure, sstring, &out_p, NULL);
		if (rc == -1) {
			DEBUG_0("chgdevice: odm_run_method failure\n")
			err_exit(E_ODMRUNMETHOD);
		}
		else if ((rc != 0) && (rc != E_FINDCHILD)) {
			DEBUG_0("chgdevice: device reconfigure failed\n")
			DEBUG_0("chgdevice: restoring attributes\n")

			/* Restore attributes to previous state:*/
			sprintf(sstring,"name = '%s'", lname );
			if (odm_rm_obj(CuAt_CLASS,sstring) == -1) {
				DEBUG_0("chgdevice: ODM error while restoring attributes\n")
				err_exit(E_ODMDELETE);
			}

			for( attrno=0; attrno<list_info.num; attrno++ ) {
				if (odm_add_obj(CuAt_CLASS,&old_attrs[attrno]) == -1) {
					DEBUG_0("chgdevice: ODM error while restoring attributes\n")
					err_exit(E_ODMADD);
				}
			}
			err_exit(rc);   /* Exit with configure method's error */
		}
	}   /* end if device is configured */

	/* clean up and return successful */
	if (odm_close_class(cusdev) == -1) {
		DEBUG_0("chgdevice: Failure closing CuDv object class\n");
		err_exit(E_ODMCLOSE);
	}

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
char	*attrstr;
struct	attr **attrlist;
{
struct	attr *attrl;
char	*name,*val;

	name = attrstr;
	attrl = *attrlist;

	/* parse up attribute string */
	while ((val = strchr(name,'=')) != NULL) {

		/* isolate attribute name */
		*val = '\0'; val++;

		/* get space */
		if ((attrl = (struct attr *)
			realloc(attrl,(no_atts+2)*sizeof(struct attr)))==NULL)
			return(E_MALLOC);

		/* set up attribute name field */
		attrl[no_atts].attribute = name;

		/* set up attribute value */
		if ((name = strchr(val,' ')) == NULL) {
			name = strchr(val,'\0');
		} else {
			*name = '\0'; name++;
		}

		/* save attribute value string ptr */
		attrl[no_atts].value = val;

		/* finish off tail of array */
		no_atts++;
		attrl[no_atts].attribute  = NULL;
		attrl[no_atts].value      = NULL;

	}

	*attrlist = attrl;

	return(0);

}


int
update_dev(cusobj,attrs)
struct  CuDv *cusobj;
struct  attr *attrs;
{
struct  Class   *preatt,        /* predefined attribute object class */
		*cusatt;        /* customized attribute object class */
struct  CuAt    cuatobj;        /* Customized attribute object */
struct  PdAt    pdatobj;        /* predefined attribute object */
char    sstring[256];           /* search string space  */
int     rc;


	/***************************************************
	   Update changed attributes.
	 ***************************************************/

	/* open object classes */
	if ((int)(cusatt=odm_open_class(CuAt_CLASS)) == -1) {
		/* error: open failed */
		DEBUG_0("Error Opening CuAt\n")
		return(E_ODMOPEN);
	}

	if ((int)(preatt=odm_open_class(PdAt_CLASS)) == -1) {
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
		DEBUG_0("chgdevice: Failure closing CuAt object class\n")
		return(E_ODMCLOSE);
	}
	if (odm_close_class(preatt) == -1) {
		/* error: close failed */
		DEBUG_0("chgdevice: Failure closing PdAt object class\n")
		return(E_ODMCLOSE);
	}

	return(0);

}


list_badattr( uniquetype, badattr )
char	*uniquetype;
char	*badattr;
{
	char	*attrname;
	char	sstring[100];
	struct	PdDv	PdDvobj;
	struct	PdAt	PdAtobj;
	int		rc;
	nl_catd cat_fd;
	char cat[80];
	
	setlocale(LC_ALL, "");

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
			fprintf(stderr,"     %s\n",attrname);
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

			/* attempt to open the catalog */
			if ((cat_fd = catopen( PdDvobj.catalog, NL_CAT_LOCALE )) == -1)
			{	/* can't access it - try an explicit path */
				sprintf(cat,"%s/%s",DEFAULT_DEVICES_CAT_DIR,PdDvobj.catalog);
				cat_fd = catopen( cat, NL_CAT_LOCALE );
			}
					
			fprintf( stderr, "     %s     %s\n", attrname,
				catgets( cat_fd,PdDvobj.setno,PdAtobj.nls_index,"" ));

			catclose( cat_fd );
		}
	}
}
