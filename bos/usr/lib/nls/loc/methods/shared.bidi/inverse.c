static char sccsid[] = "@(#)47	1.1  src/bos/usr/lib/nls/loc/methods/shared.bidi/inverse.c, cfgnls, bos411, 9428A410j 8/30/93 15:03:22";
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: inverse
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/types.h>
/***********************************************************************/

void inverse (buffer,num_of_elements,element_size)
/* inverse a stream of elements, where each elemnt is of size element_size */
char *buffer;
size_t num_of_elements;
int element_size;

{
 char *temp; 
 int i,j;
 int start,end;

 temp = malloc (element_size);
 memset (temp,'\0',element_size);
 for (i=0, j=num_of_elements-1; i<j; i++, j--)
 {
   memcpy(temp,&(buffer[i*element_size]),element_size);
   memcpy(&(buffer[i*element_size]),&(buffer[j*element_size]),element_size);
   memcpy(&(buffer[j*element_size]),temp,element_size);
 }
 free (temp);
}
