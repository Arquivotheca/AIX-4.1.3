static char sccsid[] = "@(#)48  1.4  src/bos/diag/tu/swmono/c0.c, tu_swmono, bos411, 9428A410j 1/28/94 13:49:06";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: c000
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

c000()
{
  word rc;
  do
  {
    rc=c030();                        rc_msg(BRESDRAW0,rc,CPTR);  FUSE;
    rc=c031();                        rc_msg(BRESDRAW1,rc,CPTR);  FUSE;
    rc=c032();                        rc_msg(BRESDRAW2,rc,CPTR);  FUSE;
    rc=c040();                        rc_msg(STEPDRAW0,rc,CPTR);  FUSE;
    rc=c041();                        rc_msg(STEPDRAW1,rc,CPTR);  FUSE;
    rc=c042();                        rc_msg(STEPDRAW2,rc,CPTR);  FUSE;
    rc=c050();                        rc_msg(AREA_FILL,rc,CPTR);  FUSE;
    rc=c061();                        rc_msg(CCC_TEST ,rc,CPTR);  FUSE;
    rc=c062();                        rc_msg(PLANEMASK,rc,CPTR);  FUSE;
    rc=c064();                        rc_msg(OCT_TEST ,rc,CPTR);  FUSE;
    rc=c065();                        rc_msg(BPP4_TEST,rc,CPTR);  FUSE;
    rc=c0b2();                        rc_msg(BMSKTEST ,rc,CPTR);  FUSE; 
    rc=c0b3();                        rc_msg(MASKTEST ,rc,CPTR);  FUSE; 
    rc=c0f1();                        rc_msg(0xC0F10000 ,rc,CPTR);  FUSE; 
 /* rc=c0d1();                        rc_msg(MEMXFER,  rc,CPTR);  FUSE; ------- ------ */
 /* rc=c0e0();                        rc_msg(INTLEVTEST,rc,CPTR); FUSE; ------- ------ */

  } while (FALSE);  /* executes only once, allowing breaks if error */
  if (rc > ERR_LEVEL) 
  { sprintf(tinfo.info.msg_buff, "%s",LASTMSG);
    sprintf(tinfo.info.err_buff, "%s",LASTERR);
    CPTR->err_count--; /* removes error counted in skytu runtu loop */
    CatErr(CoP_TEST | SUBTU_FAIL);
    return(SUBTU_FAIL);
  }
  else
  { CatErr (CoP_TEST | GOOD_OP);
    return(GOOD_OP);
  }
}
