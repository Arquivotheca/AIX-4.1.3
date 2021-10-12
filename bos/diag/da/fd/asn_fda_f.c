static char sccsid[] = "@(#)58  1.4  src/bos/diag/da/fd/asn_fda_f.c, dafd, bos411, 9428A410j 12/17/92 10:57:25";
/*
 *   COMPONENT_NAME: dafd
 *
 *   FUNCTIONS: assign_fda_frub
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "diag/da.h"
#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/diag_exit.h"
#include "diag/diago.h"
#include "diag/dcda_msg.h" 
#include "fdatuname.h"
#include "fda_rc.h"

extern struct tm_input da_input;

/*
 * NAME: void assign_fda_frub(rc,tsnum)
 *                                                                    
 * FUNCTION: High level description of what the procedure does
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: NONE
*/


struct fru_bucket frub_fda[] = {
	{ "" , FRUB1  , 0x227 , 0x501, DFD144_DISKETTE_ADAPTER_ERR , 
             {
	     {  100 , ""    , ""  , 0  , DA_NAME      , EXEMPT },
	     },
         }
};


void assign_fda_frub(rc,tsnum)
int rc,tsnum;
{
        extern void clean_up(); 
        extern void insert_frub();
        extern void addfrub();
	
	strcpy(frub_fda[0].dname,da_input.dname);

        if(tsnum == ADAPTER_TEST )
	{
		switch(rc)
		{
		case NO_ERROR:
			break;
		case ADAPTER_NOT_PRESENT:
                    	insert_frub(&da_input,&frub_fda[0]);
			addfrub(&frub_fda[ 0 ]) ;
			DA_SETRC_STATUS(DA_STATUS_BAD);
			break;
		case AIX_ERROR:
		case INVALID_CMD_ERROR:
		default:
			DA_SETRC_ERROR(DA_ERROR_OTHER);
			break;
		}

	}
	else
		DA_SETRC_ERROR(DA_ERROR_OTHER);
        if(rc != NO_ERROR)
                clean_up();
}

