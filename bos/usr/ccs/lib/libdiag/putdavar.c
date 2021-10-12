static char sccsid[] = "@(#)49	1.3  src/bos/usr/ccs/lib/libdiag/putdavar.c, libdiag, bos41B, bai4 1/9/95 13:54:40";
/*
 * COMPONENT_NAME: (LIBDIAG) DIAGNOSTIC LIBRARY
 *
 * FUNCTIONS: 	putdavar
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

#include "diag/class_def.h"		/* object class data structures	*/ 
#include "diag/tmdefs.h"

/* NAME: putdavar
 *
 * FUNCTION: This unit adds the specified variable to the Diagnostic
 * 		Application Attribute Object Class.
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *	0  - put successful
 *	-1 - put unsuccessful
 */

putdavar(dname, vname, vtype, val)
char	*dname;
char	*vname;
unsigned short	vtype;
char	*val;
{

	int 	rc;
	char 	crit[132];
	struct DAVars *T_DAVars;

	/* open the object class */
	if (((int) diag_open_class(DAVars_CLASS)) == -1)
		return(-1);

	/* remove any old variable */
	sprintf(crit, "dname = '%s' and vname = '%s'", dname, vname);
	if ( (rc = diag_rm_obj( DAVars_CLASS, crit )) == -1 ) 
		return ( rc );
	
	/* allocate some space for the data */
	T_DAVars = (struct DAVars *) calloc(1, sizeof(struct DAVars));
	if ( T_DAVars == (struct DAVars *) -1 )
		return (-1);

	/*  copy over input parameters to structure	*/
	strncpy (T_DAVars->dname, dname, sizeof(T_DAVars->dname));
	strncpy (T_DAVars->vname, vname, sizeof(T_DAVars->vname));
	T_DAVars->vtype = vtype;

	switch ( vtype ) {
		case DIAG_STRING:
			strncpy(T_DAVars->vvalue,val,sizeof(T_DAVars->vvalue));
			break;
		case DIAG_INT:
			memcpy (&T_DAVars->ivalue,(char *)val,sizeof(int));
			break;
		case DIAG_SHORT:
			memcpy (&T_DAVars->ivalue,(char *)val,sizeof(short));
			break;

	}

	/* add the structure to the object class	*/
	rc = diag_add_obj(DAVars_CLASS, T_DAVars);
	diag_close_class(DAVars_CLASS);

	free (T_DAVars);
	return( (rc == -1) ? -1 : 0 );
}
