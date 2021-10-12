static char sccsid[] = "@(#)90	1.2  src/bos/diag/tu/swmono/rw.c, tu_swmono, bos411, 9428A410j 10/29/93 14:21:17";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: readbyte
 *		readlword
 *		readword
 *		writebyte
 *		writelword
 *		writeword
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "skytu.h"
/************************************************************
* Name     : writebyte                                      *
************************************************************/
writebyte(data, addrp)
byte data;
byte *addrp;
{ 
  *addrp = data;
}
/************************************************************
* Name     : readbyte                                       *
************************************************************/
readbyte(addrp)
byte *addrp;
{ 
  byte data;

  data = *addrp;
  return(data);
}
/************************************************************
* Name     : writeword                                      *
************************************************************/
writeword(data, addrp)
word data;
word *addrp;
{ 
  *addrp = data;
}
/************************************************************
* Name     : readword                                       *
************************************************************/
readword(addrp)
word *addrp;
{ word data;

  data = *addrp;
  return(data);
}
/************************************************************
* Name     : writelword                                     *
************************************************************/
writelword(data, addrp)
lword data;
lword *addrp;
{ 
  *addrp = data;
}
/************************************************************
* Name     : readlword                                      *
************************************************************/
readlword(addrp)
lword *addrp;
{ lword data;
 
  data = *addrp;
  return(data);
}
