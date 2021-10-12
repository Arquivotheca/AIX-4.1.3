static char sccsid[] = "@(#)27	1.1  src/bos/usr/lib/pios/piofquote.c, cmdpios, bos411, 9428A410j 10/11/89 19:48:31";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: main
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

#include <stdio.h>
main(){
  int c;
  char data_sent = 0x00;
  while ( (c = getc(stdin)) != EOF )
    {
    if (!data_sent)       /* if this is the first byte of data to arrive at   */
      {                   /* this filter then preface it with a hex(04) to    */
      putchar(0x04);      /* make sure that the printer is in its null state  */
      data_sent = 0xff;   /* when data starts arriving                        */
      }
    if (c==1 || c==3 || c==4 || c==5 || c==0x11 || c==0x13 || c==0x14 || c==0x1c)
      {
      putchar(0x01);      /* the above characters must be specially quoted in */
      putchar(c ^ 0x40);  /* order to make it past the PostScript interpreter */
      }                   /* to the emulation software                        */
    else putchar(c);
    }
  if (data_sent) putchar(0x04); /* if data was sent thru this filter then     */
}                               /* restore the printer to its null state      */
