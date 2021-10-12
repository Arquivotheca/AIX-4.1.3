static char sccsid[] = "@(#)92	1.1  src/bos/usr/bin/bterm/status.c, libbidi, bos411, 9428A410j 8/26/93 13:35:29";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: do_bidi_put_status
 *		remove_status
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
/*    status.c: includes all functions to handle the status line 
**    functions: do_bidi_put_status, remove_status
*/

#include "trace.h"
#include "global.h"

/*----------------------------------------------------------------*/
 void do_bidi_put_status()
 {
  char st_line[80];
  int i;
  

  /* prepare status line */
  memset(&(st_line[0]),0x20,80);

  /* keyboard layer */
  if (is_nl_kbd())
    st_line[10]=0x41;    /* A */
  else st_line[10]=0x45; /* E */

  /* autopush enabled */
  if (is_visual_text())
   if ((is_RTL_mode() && is_right_autopush())
       || (is_LTR_mode() && is_left_autopush()))
    {
      st_line[17]=0x41;   /* A */
      st_line[18]=0x50;   /* P */
    }

  /* push mode */
  if (is_visual_text() && is_push_mode())
   st_line[21]=0x70;    /* p */

  /* screen orientation */
  st_line[30]=0x53;     /* S */
  st_line[31]=0x43;     /* C */
  st_line[32]=0x52;     /* R */
  if (is_RTL_mode())
     {
       st_line[27]=0x3c; /* < */
       st_line[28]=0x2d; /* - */
     }
  else
     {
       st_line[34]=0x2d; /* - */
       st_line[35]=0x3e; /* > */
     }

  /* shaping */
  if (is_asd_on())
          st_line[45]=0xc2;   /* alef madda */
  else if (is_initial_csd()) 
          st_line[45]=0xc0;   /* initial ein */
  else if (is_middle_csd()) 
          st_line[45]=0xdb;   /* middle ein */
  else if (is_final_csd()) 
          st_line[45]=0xbe;   /* final ein */
  else if (is_isolated_csd()) 
          st_line[45]=0xd9;   /* isolated ein */

  /* text mode */
  if (is_implicit_text())
    st_line[55]=0x49;   /* I */
  else
    st_line[55]=0x56;   /* V */
 
  /* numerics */
  if (is_bilingual_nss())
    st_line[65]=0x55;   /* U */ 
  else if (is_arabic_nss())
    st_line[65]=0x41;   /* A */ 
  if (is_hindu_nss())
    st_line[65]=0x48;   /* H */ 

  /* end of string */
  st_line[79]=0;

  /* put it on screen */
  status_line = TRUE;
  put_cursor(LINES-1,0);
  string_to_hft(st_line,80);
  put_cursor(SCR->_logy,SCR->_visx);
  flush_to_hft();
  for (i=0;i<80;i++)
    SCR->_phys_image[LINES-1]._line_chars[i]=st_line[i];
 }
/*----------------------------------------------------------------*/
 void remove_status()
 {
  status_line = FALSE;
  update_line(LINES-1);
 }
/*----------------------------------------------------------------*/
