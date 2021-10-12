static char sccsid[] = "@(#)42	1.3  src/bos/usr/ccs/lib/libdiag/getdavar.c, libdiag, bos41B, bai4 1/9/95 13:54:16";
/*
 * COMPONENT_NAME: (LIBDIAG) DIAGNOSTIC LIBRARY
 *
 * FUNCTIONS: 	getdavar
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include "diag/class_def.h"		/* object class data structures	*/ 
#include "diag/tmdefs.h"

/* NAME: getdavar
 *
 * FUNCTION: This unit retrieves the variable "vname" from the
 *		Diagnostic Application Attribute Object Class.
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *	0  - get successful
 *	-1 - get unsuccessful
 */

getdavar(dname, vname, vtype, val)
char	*dname;
char	*vname;
unsigned short	vtype;
char	*val;
{

	char crit[80];
	struct DAVars *T_DAVars;
	struct listinfo v_info;
 
	sprintf(crit, "dname = '%s' and vname = '%s'", dname, vname);
	T_DAVars = (struct DAVars *)diag_get_list( DAVars_CLASS,
			crit, &v_info, 1, 1 );
	if ( T_DAVars != (struct DAVars *) -1 && v_info.num == 1 )  {
		vtype = T_DAVars->vtype;
		switch ( vtype ) {
			case DIAG_STRING:
				strncpy ( val, T_DAVars->vvalue, sizeof(T_DAVars->vvalue));
				break;
			case DIAG_INT:
				memcpy ((char *)val, &T_DAVars->ivalue, sizeof(int));
				break;
			case DIAG_SHORT:
				memcpy ((char *)val, &T_DAVars->ivalue, sizeof(short));
				break;
		}
		return(0);	
	}
	return(-1);
}
