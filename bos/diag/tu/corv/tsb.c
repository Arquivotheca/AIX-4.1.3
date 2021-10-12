static char sccsid[] = "@(#)34  1.1  src/bos/diag/tu/corv/tsb.c, tu_corv, bos411, 9428A410j 7/22/93 18:58:47";
/*
 *   COMPONENT_NAME: TU_CORV
 *
 *   FUNCTIONS: format_TSB_data
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
#include "libadapters.h"
#include "CorvetteLib.h"
#include "OhioLib.h"

/******************************************************************************
*
* NAME:  format_TSB_data
*
* FUNCTION:  Formats TSB data into readable text.
*
* INPUT PARAMETERS:     TSB_data = sense string values.
*                       return_string = pointer to return string.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: none.
*
* EXTERNAL PROCEDURES CALLED:
*
******************************************************************************/
unsigned char format_TSB_data(ADAPTER_STRUCT *handle, char *TSB_data, char *return_string)
{
        char stat_word2[300];
        int search_pos, word1, word2;

        if (strcmp(handle->environment.device_name,"corvette")==0) {
                if (handle->environment.error_format=VERBOSE)
                        sprintf(return_string,  "     SCSI Status Byte = 0x%02x, %s\n" \
                                                "     Command Status Byte = 0x%02x, %s\n" \
                                                                "     Device Error Byte = 0x%02x, %s\n" \
                                                                "     Command Error Byte = 0x%02x, %s\n" \
                                                                "     Error Modifier = 0x%02x%02x\n",
                                                                TSB_data[14], TSB_SCSI_Status[TSB_data[14]],
                                                                TSB_data[15], TSB_Cmd_Status[TSB_data[15]],
                                                                TSB_data[16], TSB_Device_Code[TSB_data[16]],
                                                                TSB_data[17], TSB_Cmd_Code[TSB_data[17]],
                                                                TSB_data[18], TSB_data[19]);
         else
                        sprintf(return_string,  "%02x%02x%02x%02x%02x%02x%02x%02x",
                                                                TSB_data[14], TSB_data[15], TSB_data[16], TSB_data[17],
                                                                TSB_data[18], TSB_data[19], TSB_data[20], TSB_data[21]);
        }
        else if (strcmp(handle->environment.device_name,"ohio")==0) {
                        word1 = *(unsigned short *)TSB_data[0];
                if (handle->environment.error_format=VERBOSE) {
                        sprintf(return_string, "     End Status Word 1 = 0x%04, ",word1);
                        for(search_pos=0; search_pos < STAT_ENTRIES; search_pos++)
                                if (word1=TSB_Status_Word1[search_pos].word_value)
                                        strcat(return_string,TSB_Status_Word1[search_pos].description);
                        word2 = *(unsigned short *)TSB_data[2] & 0x00ff;

                        sprintf(stat_word2,     "\n     End Status Word 2 = 0x%02x%02s, %s\n" \
                                                "     Residual Byte Count = 0x%02x%02x%02x%02x\n", \
                                                "     Residual Buffer Address = 0x%02x%02x%02x%02x\n",
                                                TSB_data[2], TSB_data[3], TSB_Status_Word2[word2],
                                                TSB_data[7], TSB_data[6], TSB_data[5], TSB_data[4],
                                                TSB_data[12], TSB_data[11], TSB_data[10], TSB_data[9]);
                        strcat(return_string, stat_word2);
                }
         else
                        sprintf(return_string,  "%04x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                                                                word1, TSB_data[2], TSB_data[3],
                                                                TSB_data[7], TSB_data[6], TSB_data[5], TSB_data[4],
                                                                TSB_data[12], TSB_data[11], TSB_data[10], TSB_data[9] );
    }
}



