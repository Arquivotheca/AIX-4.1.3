static char sccsid[] = "@(#)50	1.4  src/bos/usr/ccs/lib/libdiag/save_cdiag.c, libdiag, bos41J, 9513A_all 3/17/95 10:48:32";
/*
 * COMPONENT_NAME: (LIBDIAG) DIAGNOSTIC LIBRARY
 *
 * FUNCTIONS: save_cdiag
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include "diag/class_def.h"
#include "diag/diag.h"
#define	DIAG_CONVERSION_KEY	"DIAGS_VERSION_4"

/* NAME: save_cdiag  
 *
 * FUNCTION: This function populates the CDiagDev Object Class with objects
 * 	contained in the input structure. 
 *	Clear the present CDiagDev Object Class data
 *	Create a new entry 
 * 
 * NOTES: 
 *
 * RETURNS:
 *	0 : object class updated
 *     -1 : error opening object class or adding/updating object
 *
 */
char	*malloc();

save_cdiag  ( Top, num_devices )
diag_dev_info_t		*Top;
int			num_devices;	/* number of devices in Top */
{

	int			rc = 0; 
	int			count;
	diag_dev_info_t      	*curr_diag_dev_info;
	struct  CDiagDev        *cdiagdev_p;


	curr_diag_dev_info = Top;

	/* open the Class */
	if (((int) diag_open_class(CDiagDev_CLASS)) == -1)
		return(-1);

	/* clear all previous entries	*/
	rc = diag_rm_obj(CDiagDev_CLASS,"");

        /* Add the diagnostics key so that no conversion is performed */
	/* the next time init_diag_db() is called.                    */

	cdiagdev_p = (struct CDiagDev *)calloc(1, sizeof(struct CDiagDev));
	sprintf(cdiagdev_p->DType, "%s", DIAG_CONVERSION_KEY);
	rc = odm_add_obj( CDiagDev_CLASS, cdiagdev_p );
	free(cdiagdev_p);

	/* do for all devices in list that have a CDiagDev entry	*/
	for( count = 0; (count < num_devices) && (rc != -1); count++ )  {

		if (curr_diag_dev_info->T_CDiagDev && 
		    curr_diag_dev_info->T_CuDv->chgstatus != 3 )

			/* add the entry */
			rc = diag_add_obj( CDiagDev_CLASS, 
					  curr_diag_dev_info->T_CDiagDev );
			
		curr_diag_dev_info++;
	}

	diag_close_class( CDiagDev_CLASS );
	return( ( rc >=0 ) ? 0 : -1 );
}
