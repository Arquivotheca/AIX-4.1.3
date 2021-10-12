static char sccsid[] = "@(#)64	1.1  src/bos/diag/dctrl/convert.c, dctrl, bos411, 9433A411a 8/10/94 13:32:14";
/*
 * COMPONENT_NAME: (DCTRL) Diagnostic Controller
 *
 * FUNCTIONS:   convert_ffc
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* ^L */
/* NAME: convert_ffc
 *
 * FUNCTION: Convert any character 'A' in the given failing function code
 *           to a 'C' to match the value being displayed in the led.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: the new failing function code value.
 *
 */
#include <diag/diag.h>

short convert_ffc(short ffc)
   {
   char    str[16], buff[16];
   int     i;
   char    **temp_ptr=NULL;

   sprintf(str, "%04X", ffc);
   if(!strrchr(str, 'A') || !strrchr(str, 'a'))
   {
	   for(i=0; i<strlen(str);i++)
		   if((str[i] == 'A') || (str[i] == 'a'))
			   buff[i] = 'C';
		   else
			   buff[i] = str[i];
											   buff[i]='\0';
	   return((short)strtoul(buff,temp_ptr,16));
   } else
	   return(ffc);

}
