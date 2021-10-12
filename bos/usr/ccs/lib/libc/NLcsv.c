static char sccsid[] = "@(#)17	1.5  src/bos/usr/ccs/lib/libc/NLcsv.c, libcnls, bos411, 9428A410j 2/26/91 12:44:10";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NLcsv
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

#include <stdlib.h> /* for MB_CUR_MAX */

/*
 * EXTERNAL PROCEDURES CALLED:  None
 */


/*
 * NAME: NLcsv
 *
 * FUNCTION: Return a pointer to the character set vector.
 *
 *           The character set vector is a character string
 *           which consists of byte pairs which are the begin
 *           and end points of multibyte (16-bit) characters.
 *
 * (EXECUTION ENVIRONMENT:)
 *
 *      Environment-Specific aspects, such as -
 *      Preemptable        : Yes
 *      VMM Critical Region: No
 *      Runs on Fixed Stack: No
 *      May Page Fault     : Yes
 *      May Backtrack      : Yes
 *
 * (NOTES:) Returns a pointer to the character set vector.
 *
 * (DATA STRUCTURES:) The character set vector is defined as a static
 *                    array of type char.
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to the character set vector.
 */

/*******************************
** this v3.1 routine will work**
** with all v3.2 single byte  **
** code sets and with pc932.  **
** All other code pages are   **
** not supported.             **
********************************/

static char csv_KJI[] = { 0x81, 0x9f, 0xe0, 0xfc, 0x00, 0x00 };
static char csv_SB[] = { 0x1c, 0x1f, 0x00, 0x00 };

char *
NLcsv() {
	if (MB_CUR_MAX == 1)
		return(csv_SB);
	else
		return(csv_KJI);
}
