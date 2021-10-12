static char sccsid[] = "@(#)34	1.2  src/bos/diag/tu/sky/rcmsg.c, tu_sky, bos411, 9428A410j 10/29/93 13:41:53";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: rc_msg
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
extern struct cntlparm cptr;
/************************************************************
*                          RC_MSG                           *
* Write a message commenting on the return code to err      *
************************************************************/
rc_msg(tu_name, ret_code, jnkcptr) 
lword  tu_name;
lword ret_code;
struct cntlparm jnkcptr;
{
  if ( (tu_name & 0xFFFF0000) == tu_name) tu_name /= 0x10000; 

    if ( ((word)ret_code) > ERR_LEVEL)
    { 
       fprintf(hist,"\n\n\nError in run_tu: %X", tu_name);
       fprintf(hist,"\n%s",tinfo.info.msg_buff);
       fprintf(hist,"\n%s",tinfo.info.err_buff);
       cptr.err_count++;
    }
    else if (cptr.verbose)
    {  fprintf(hist,"\n\nNo Error in run_tu: %X", tu_name);
       fprintf(hist,"\n%s",tinfo.info.msg_buff);
       fprintf(hist,"\n%s",tinfo.info.err_buff);
    }

  strcpy(LASTERR,tinfo.info.err_buff);
  strcpy(LASTMSG,tinfo.info.msg_buff);
  strcpy(tinfo.info.err_buff,"Error Trace");
  strcpy(tinfo.info.msg_buff,":");
}
