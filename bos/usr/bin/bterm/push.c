static char sccsid[] = "@(#)89	1.1  src/bos/usr/bin/bterm/push.c, libbidi, bos411, 9428A410j 8/26/93 13:35:21";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: do_bidi_autopush_off
 *		do_bidi_autopush_on
 *		do_bidi_check_autopush
 *		do_bidi_end_push
 *		do_bidi_start_push
 *		do_bidi_toggle_autopush
 *		echo_push_buffer
 *		norm_cursor
 *		push_char
 *		push_cursor
 *		push_cursor_left
 *		push_cursor_right
 *		push_delete
 *		push_passthru
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
#include "trace.h"
#include "global.h"

void echo_push_buffer();
void norm_cursor();
void push_cursor();
/*------------------------ push character in push buffer -------------------*/
 void push_char(chval)
 unsigned char chval;
{
   /* check if there is still space on the line */
   if ((BDCurSeg->push_start_loc+BDCurSeg->push_count)<COLUMNS)
    {
      if (push_boundary)  /* insert before current char */
      {
       /* make place in the push buffer for the new char */
       memmove(&push_buffer[0],&(push_buffer[1]),
               COLUMNS-BDCurSeg->push_cur_loc-1); 
       BDCurSeg->push_count++;
       BDCurSeg->push_cur_loc++;
       push_buffer[COLUMNS-BDCurSeg->push_cur_loc]=chval;
      }
      else  /* not on push boundary */
      {
       /* make place in the push buffer for the new char */
       memmove(&push_buffer[0],&(push_buffer[1]),
               COLUMNS-BDCurSeg->push_cur_loc); 
       push_buffer[COLUMNS-BDCurSeg->push_cur_loc]=chval;
       BDCurSeg->push_count++;
       BDCurSeg->push_cur_loc++;
      }
      echo_push_buffer();  /* display it on screen */
    }
   else char_to_hft(0x07); /* if no space do not push and sound bell */
 }
/*------------------------ toggle autopush enable --------------------------*/
void do_bidi_toggle_autopush()
{
 if (is_bidi_mode() && is_visual_text())
 {
  if (is_RTL_mode())
    if (is_right_autopush())
       {
        reset_right_autopush();
        reset_in_autopush();
       }
    else set_right_autopush();
  else 
    if (is_left_autopush())
       {
       reset_left_autopush();
       reset_in_autopush();
       }
    else set_left_autopush();
 }
}
/*---------------------- set autopush enable on -------------------------*/
void do_bidi_autopush_on()
{
 if (is_bidi_mode() && is_visual_text())
 {
  if (is_RTL_mode())
    set_right_autopush();
  else 
    set_left_autopush();
 }
}
/*--------------------- set autopush enable off -------------------------*/
void do_bidi_autopush_off()
{
 if (is_bidi_mode() && is_visual_text())
 {
  if (is_RTL_mode())
    reset_right_autopush();
  else 
    reset_left_autopush();
  reset_in_autopush();
  }
}
/*------------------ start push mode  ----------------------------*/
void do_bidi_start_push()
{
 if (is_bidi_mode() && is_visual_text() && !is_push_mode())
  {
   BDCurSeg->push_start_loc = SCR->_logx;
   BDCurSeg->push_cur_loc   = 0;
   BDCurSeg->push_count     = 0;
   push_boundary = TRUE;
   push_cursor();
   memset(push_buffer,0x00,COLUMNS);  /* initialize push buffer */
   set_push_mode();
   if (!is_in_autopush())
     if (is_RTL_mode()) set_latin_kbd(); else set_nl_kbd(); 
  }
}
/*------------------ end push mode  ----------------------------*/
void do_bidi_end_push()
{

 if (is_bidi_mode() && is_visual_text() && is_push_mode())
  {
    /* send push buffer to terminal */
    put_cursor(SCR->_logy,BDCurSeg->push_start_loc);
    array_to_pty(&(push_buffer[COLUMNS-BDCurSeg->push_count]),
                 BDCurSeg->push_count);
    BDCurSeg->push_start_loc = 0;
    BDCurSeg->push_cur_loc   = 0;
    BDCurSeg->push_count     = 0;
    push_boundary = FALSE;
    norm_cursor();
    reset_push_mode();
    SCR->_changed[SCR->_logy]=TRUE;
    if (is_RTL_mode()) set_nl_kbd(); else set_latin_kbd(); 
    reset_in_autopush();
  }
}
/*------------------ check autopush mode  ----------------------------*/
void do_bidi_check_autopush(ch)
unsigned char ch;
{
 if (is_bidi_mode() && is_visual_text())
 {
 if (is_in_autopush())  /* check if we need to end it */
  {
    if (is_RTL_mode())
    {
    if ((is_nl_kbd() &&              /* any char from nl-kbd except: */
       (((ch<0x30) || (ch>0x39))     /* numerals,                    */ 
       && (ch!=0xac) && (ch!=0x2f))) /* comma , slash   */ 
       || (ch==0x1b))                /* or Escape */
       { 
        do_bidi_end_push();
       }  
    }
    else   /* LTR mode */
       if (is_latin_kbd()    /* any char from latin kbd */
          || (ch==0x1b))     /* or Escape */
       { 
        do_bidi_end_push();
       }  
  }
 else if (!is_push_mode())   /* check if we need to start it */
  {
    if (is_RTL_mode() && is_right_autopush())
    {
      if ((is_latin_kbd() && (ch!=0x20))  /* latin kbd and not space */ 
       || (is_nl_kbd()                    /* or nl_kbd and */
            && (((ch>=0x30) && (ch<=0x39))    /* digit */
             || ((ch>=0x41) && (ch<=0x5a))    /* or a - z */ 
             || ((ch>=0x61) && (ch<=0x7a))    /* or A - Z */
             || (ch==0x2f) || (ch==0xac))))   /* or slash or comma */
      {
       set_in_autopush();
       do_bidi_start_push();
      } 
    }
    else if (is_LTR_mode() && is_left_autopush())    /* LTR mode */
      if (is_nl_kbd() && (ch!=0x20))   /* nl_kbd and not space */
      {
       set_in_autopush();
       do_bidi_start_push();
      } 
  }
 }
}

/*---------------------------------------------------------------*/
void echo_push_buffer()
/* to echo the push buffer on the screen starting at push start location */
{
    int RC;
    int i;
    char temp_push[Max_Columns];
    size_t num;
 
    memcpy(temp_push,push_buffer,COLUMNS);
    /* call the API */
    num=BDCurSeg->push_count;
    RC=layout_object_transform(plh,
                            (char *)&(temp_push[COLUMNS-BDCurSeg->push_count]),
                            &num,
                            (char *)&(temp_push[COLUMNS-BDCurSeg->push_count]),
                            &num,NULL,NULL,NULL);  

    /* display the buffer on the screen */
     if (is_RTL_mode())
        put_cursor(SCR->_logy,
                   COLUMNS-BDCurSeg->push_start_loc-BDCurSeg->push_count);
     else put_cursor(SCR->_logy,BDCurSeg->push_start_loc);
     array_to_hft(&(temp_push[COLUMNS-BDCurSeg->push_count]),
                  BDCurSeg->push_count);
     put_cursor(SCR->_logy,SCR->_visx);
    /* update the physical screen image */
    for (i=0;i<BDCurSeg->push_count;i++)
     SCR->_phys_image[SCR->_logy]._line_chars[BDCurSeg->push_start_loc+i]=
         temp_push[COLUMNS-BDCurSeg->push_count+i];
}
/*---------------------------------------------------------------*/
void push_delete()
/* to perform a delete function on the push buffer */
{ 
 if (push_boundary)
  char_to_hft(0x07);
 else
  {
   memmove(&(push_buffer[1]),&(push_buffer[0]),COLUMNS-BDCurSeg->push_cur_loc);
   push_buffer[0]=0x00;
   BDCurSeg->push_count--;
   if ((BDCurSeg->push_cur_loc==1) &&(BDCurSeg->push_count>0))
      if (is_RTL_mode()) SCR->_visx++; else SCR->_visx--;
   else
       BDCurSeg->push_cur_loc--;
   /* put a blank in place of deleted char at end of segment */
   if (is_RTL_mode())
    put_char(SCR->_logy,COLUMNS-BDCurSeg->push_start_loc-
             BDCurSeg->push_count-1,0x20);
   else
    put_char(SCR->_logy,BDCurSeg->push_start_loc+BDCurSeg->push_count,0x20);
   if (BDCurSeg->push_count==0)   /* empty segment */
     {
       push_boundary=TRUE;
       push_cursor();
       put_cursor(SCR->_logy,SCR->_visx);
     }
   else echo_push_buffer();
  }
}
/*---------------------------------------------------------------*/
void push_cursor_right()
/* to perform cursor right on the push buffer */
{
 if (push_boundary)   /* if on a boundary */
   if (BDCurSeg->push_count==0)  /* if it is an empty segment, do not move */
   char_to_hft(0x07);
   else 
     { /* if there are chars in the segment, reset boundary but do not move */
      push_boundary=FALSE;
      norm_cursor();
     }
 else  /* not on a boundary */
   if (BDCurSeg->push_cur_loc>1)  /* there are chars to the right */
    {
     BDCurSeg->push_cur_loc--;
     if (is_RTL_mode()) SCR->_visx--; else SCR->_visx++;
     put_cursor(SCR->_logy,SCR->_visx);
    }
   else /* there are no chars in the push segment to the right */
     char_to_hft(0x07);
   }
/*---------------------------------------------------------------*/
void push_cursor_left()
/* to perform cursor left on the push buffer */
{
 if (push_boundary)   /* if on a boundary */
    char_to_hft(0x07);
 else  /* not on a boundary */
   if (BDCurSeg->push_cur_loc<BDCurSeg->push_count) /* more chars to the left*/
    {
     BDCurSeg->push_cur_loc++;
     if (is_RTL_mode()) SCR->_visx++; else SCR->_visx--;
     put_cursor(SCR->_logy,SCR->_visx);
    }
   else /* there are no chars in the push segment to the left,so set boundary*/
    {
     push_boundary=TRUE;
     push_cursor();
    }
}
/*---------------------------------------------------------------*/
void norm_cursor()
{
  TRACE(("setting normal cursor shape \n"));
}
/*---------------------------------------------------------------*/
void push_cursor()
{
  TRACE(("setting push cursor shape \n"));
}
/*---------------------------------------------------------------*/
void push_passthru()
/* save pushed buffer as is because passthru is pushed while
   in push mode */
{
    int RC;
    size_t num;

 if (is_bidi_mode() && is_visual_text() && is_push_mode())
  {
    num=BDCurSeg->push_count;
    /* call the API */
TRACE(("push passthru\n"));
    RC=layout_object_transform(plh,
                            (char *)&(push_buffer[COLUMNS-BDCurSeg->push_count]),
                            &num,
                            (char *)&(push_buffer[COLUMNS-BDCurSeg->push_count]),
                            &num,NULL,NULL,NULL);  

   }
}
/*---------------------------------------------------------------*/
