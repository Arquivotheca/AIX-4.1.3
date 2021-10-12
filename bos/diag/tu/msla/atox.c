static char sccsid[] = "@(#)25	1.3  src/bos/diag/tu/msla/atox.c, tu_msla, bos411, 9428A410j 6/26/91 08:29:43";
/*
 * COMPONENT_NAME: ( atox ) 
 *
 * FUNCTIONS:       atox
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :                                                        */
/*                                                                           */
/*          PURPOSE :  To convert an Hex-ascii string to integer             */
/*                                                                           */
/*            INPUT :  Hex character string in ascii including 0X or 0x      */
/*                                                                           */
/*           OUTPUT :  Returns 0 upon success.                               */
/*                                                                           */
/* FUNCTIONS CALLED :  None.						     */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/* EXTERNAL REFERENCES = None                                                */
/*                                                                           */
/*****************************************************************************/
unsigned long
atox(s)
char s[];
{
    int i, max, first, error;
    unsigned long n, tmp;

    error = 0;
    max = strlen(s);
    n=0;

    if ((s[1] == 'x') || (s[1] == 'X'))  
	first=2; 
    else 
	first =0;

    for (i=first; i<max; i++)
    {
        tmp = (int)s[i];
        if (tmp == ' ') 
	    continue;

        if ( (tmp > 'f') | (tmp < '0') ) 
	    error = -1;

        if ( tmp >= 'a' )  
	{
            tmp = tmp - 'a' + 10;
	}
        else
        {
            if (tmp > 'F') error = -2;
            if ( tmp >= 'A' )
                tmp = tmp - 'A' + 10;
            else
            {
                if ( tmp > '9' ) 
                    error = -3;
                else tmp = tmp - '0';
            }
        }
        n = n*16 + tmp;
    }
    if (error != 0) n=0;
    return (n);
}
