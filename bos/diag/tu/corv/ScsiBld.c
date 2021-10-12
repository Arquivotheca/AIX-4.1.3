static char sccsid[] = "@(#)24  1.1  src/bos/diag/tu/corv/ScsiBld.c, tu_corv, bos411, 9428A410j 7/22/93 18:55:08";
/*
 *   COMPONENT_NAME: TU_CORV
 *
 *   FUNCTIONS: format_sense_data
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "libscsibld.h"


/******************************************************************************
*
* NAME:  format_sense_data
*
* FUNCTION:  Formats SCSI sense data into readable text.
*
* INPUT PARAMETERS:     sense = sense string values.
*                       return_string = pointer to return string.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: none.
*
* EXTERNAL PROCEDURES CALLED: diag_pos_write
*
******************************************************************************/
unsigned char format_sense_data(char *sense, char *return_string)
{
        int search_pos = 0;

        while ( (search_pos < ASC_ENTRIES) &&
                ((sense[12] != sense_errors[search_pos].additional_sense_code) ||
                 (sense[13] != sense_errors[search_pos].additional_sense_qualifier)))
                     search_pos++;

        sprintf(return_string,"Sense Data:\n" \
                                "     Sense Key = 0x%02x, %s\n" \
                                "     Additional Sense Code = 0x%02x\n" \
                                "     Addition Sense Code Qualifier = 0x%02x\n" \
                                "     %s\n",
                                sense[2], Sense_Key[(sense[2] & 0x7)],
                                sense[12], sense[13],sense_errors[search_pos].description);

}



