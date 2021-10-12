static char sccsid[] = "@(#)41  1.5  src/bos/kernext/disp/ped/diag/miderr.c, peddd, bos411, 9428A410j 5/13/94 14:55:02";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: miderr
 *		
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*------------
 * FUNCTIONAL DESCRIPTION:
 *
  MIDERR error logging routine.

	midddf_t        *ddf            middd device dependent structure.
	ulong           status_parm     DSP Commo Register status.

  ------------*/

#include <sys/types.h>
#include <sys/intr.h>
#include <sys/errids.h>
#include <sys/syspest.h>

#include "hw_dsp.h"
#include "midddf.h"
#include "mid_dd_trace.h"

BUGVDEF(dbg_miderr,0);                          /* debug flag for peddd */
ERR_REC(256) ER;

int
miderr(ddf,status_parm,res_name,dmodule,fmodule,return_code,err_indicator,ras_unique)
		midddf_t                *ddf;
		ulong                   status_parm;
		char                    *res_name;
		char                    *dmodule;
		char                    *fmodule;
		int                     return_code;
		int                     err_indicator;
		char                    *ras_unique;

{
	BUGLPR(dbg_miderr, 1, ("Entering miderr \n"));

	if (status_parm)
	{
	    if ( ( status_parm & 0xF0000000 ) == DATA_STREAM_ERROR )
	    {
		    BUGLPR(dbg_miderr, 1,
		    ("DATA STREAM ERROR REPORTED BY ADAPTER 0x%8X !!\n\n",
		    status_parm ));

		    ER.error_id = ERRID_MID_SW;     /* Template Id # */
	    }
	    else
	    {
		    BUGLPR(dbg_miderr, 1,
		    ("HARDWARE ERROR REPORTED BY ADAPTER 0x%8X !!!\n\n",
		    status_parm ));

		    if ( ( status_parm & 0xFFFF0000 ) == MICROCODE_CRC_FAILED )
			    ER.error_id = ERRID_MID_UCODE;
		    else if ( ( status_parm & 0xFFFF0000 ) == BLAST_ERROR )
			    ER.error_id = ERRID_MID_BLASTB;
		    else if ( ( status_parm & 0xFFFF0000 ) == BLAST_PROC_ERROR )
			    ER.error_id = ERRID_MID_BLAST;
		    else if ( ( status_parm & 0xFFFF0000 ) == PIPE_PROC1_ERROR )
			    ER.error_id = ERRID_MID_PIPE1;
		    else if ( ( status_parm & 0xFFFF0000 ) == PIPE_PROC2_ERROR )
			    ER.error_id = ERRID_MID_PIPE2;
		    else if ( ( status_parm & 0xFFFF0000 ) == PIPE_PROC3_ERROR )
			    ER.error_id = ERRID_MID_PIPE3;
		    else if ( ( status_parm & 0xFFFF0000 ) == PIPE_PROC4_ERROR )
			    ER.error_id = ERRID_MID_PIPE4;
	    }

	    strcpy( ER.resource_name, "MIDDD");

	    sprintf( ER.detail_data, " %d ", ddf->slot );

	} else
	{
	    ER.error_id = ERRID_GRAPHICS;

	    sprintf(ER.resource_name,"%8s",res_name);

	    sprintf(ER.detail_data,"%8s  %8s  %4d  %4d  %s",
		   dmodule,fmodule,return_code,err_indicator,ras_unique);
	}

	/* Call system error logging routine */
	errsave(&ER, ERR_REC_SIZE + (strlen(ER.detail_data)+1));

	/* For debug only */
	BUGLPR(dbg_miderr, 1, ("Leaving miderr \n"));

	return(0);
}
