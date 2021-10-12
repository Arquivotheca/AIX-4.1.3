static char sccsid[] = "@(#)36  1.1  src/bos/usr/bin/lppchk/lppchkck.c, cmdswvpd, bos411, 9428A410j 5/29/91 16:06:04";
/*
 * COMPONENT_NAME: (CMDSWVPD) Software VPD
 *
 * FUNCTIONS: lppchk (main)
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

/*
 * NAME: lppchkck (Checksum)
 *
 * FUNCTION: Compute the checksum for the input text file.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Reads the file passed as a parameter and computes the checksum
 *      for that file.   The checksum algorithm is the same as that used
 *      by the 'sum' command (BSD option).  In addition, it will count the
 *      characters in the file and set the second parameter to reflect
 *      that value.
 *
 * RETURNS: checksum for the input file
 *
 */

#include        "lppchk.h"      /* local and standard defines           */

 /* check_fp -- checksum the bytes available for reading from the file */
 /* pointer specified, and return the checksum value */
 /* This routine does not attempt to detect or report any kinds of */
 /* errors.  The assumption is that if there is an error, the checksum */
 /* will come out wrong, and an error message will be generated due to */
 /* that. */


unsigned int lppchkck (
          FILE *fp,
          int  *sz)

  {
  int   c;                      /* data from file                       */
  int   s;                      /* local size counter                   */
  unsigned int sum ;            /* checksum being computed              */

  sum = 0;
  s   = 0 ;
  while((c = getc(fp)) != EOF)
    {
    s++ ;                       /* count file size                      */

    if(sum & 01)                /* logically rotate low 16 bits right   */
        sum = (sum >> 1) + 0x8000;
    else
        sum >>= 1;

    sum += c;                   /* include current character in sum     */
    sum &= 0xFFFF;              /* ensure only 16 bits are retained     */
    }
  *sz = s ;                     /* only set using pointer once (opt)    */
  return (sum);
  }                             /* end lppchkck                         */
