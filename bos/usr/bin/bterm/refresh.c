static char sccsid[] = "@(#)90	1.2  src/bos/usr/bin/bterm/refresh.c, libbidi, bos411, 9428A410j 10/5/93 18:00:01";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: init_line
 *		init_screen
 *		update_line
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
#include <fcntl.h>
#include <termios.h>
#include <cur00.h>
#include <memory.h>
#include <string.h>
#include <term.h>

#include "global.h"
#include "trace.h"

/*------------------------------------------------------------------------*/
/* compare physical screen line with screen buffer and 
   output the different characters to screen */
void update_line(y)
short y;
{
  int x,oldx,RC;
  line_type temp_line;
  int i,length;
  int NewCursorPos;
  size_t *ToOutBuf;
 
  memcpy(&(temp_line._line_chars[0]), /* copy line to temporary buffer */
         &(SCR->_scr_image[y]._line_chars[0]),COLUMNS*sizeof(char));
  memcpy(&(temp_line._line_atrib[0]), 
         &(SCR->_scr_image[y]._line_atrib[0]),COLUMNS*sizeof(int));
  memcpy(&(temp_line._line_grph[0]),
         &(SCR->_scr_image[y]._line_grph[0]),COLUMNS*sizeof(char));
    ToOutBuf=malloc(Max_Columns*sizeof(size_t));
    /* transform cannot handle the nulls at end of line, so
       do not send them */
    length=COLUMNS;
    while ((temp_line._line_chars[length-1]==0x00) 
           && (length>1)) length--;
    if (is_nonulls_mode())   /* replace nulls in middle of line with spaces */
      for (i=0;i<length;i++)
        if (temp_line._line_chars[i]==0x00)
            temp_line._line_chars[i]=0x20;
    /* replace alternate graphics characters with a definite level 0 char */
      for (i=0;i<length;i++)
        if (SCR->_scr_image[y]._line_grph[i]!=0)
            temp_line._line_chars[i]=0x0d;  /* CR */
  
    /* call the API */
    RC=layout_object_transform(plh,
                            (char *)&(temp_line._line_chars[0]),
                            &length,
                            (char *)&(temp_line._line_chars[0]),
                            &length,
                            ToOutBuf,NULL,NULL);

     if (RC!=0) TRACE(("after transform RC=%d\n",RC));
     for (i=0;i<length;i++)
        {
          /* copy atribs and grph according to ToOutBuf */
          temp_line._line_atrib[ToOutBuf[i]]=SCR->_scr_image[y]._line_atrib[i];
          temp_line._line_grph[ToOutBuf[i]]=SCR->_scr_image[y]._line_grph[i];
         /* put back the alternate graphics characters */
          if (SCR->_scr_image[y]._line_grph[i]!=0)
          {
              temp_line._line_chars[ToOutBuf[i]]=
                                         SCR->_scr_image[y]._line_chars[i];
              layout_object_shapeboxchars(plh,
                              &(temp_line._line_chars[ToOutBuf[i]]),1,
                              &(temp_line._line_chars[ToOutBuf[i]]));
          }
        }

   /* find cursor position */
  if (y==SCR->_logy)
   if (SCR->_logx<length)
     NewCursorPos=ToOutBuf[SCR->_logx];
   else 
     NewCursorPos=SCR->_logx;
   /* if the line has been reversed, we must take in consideration the nulls
      that we have ignored before sending it */
   if (is_RTL_mode())
   {
     memmove(&(temp_line._line_chars[COLUMNS-length]),
             &(temp_line._line_chars[0]),length*sizeof(char));
     memmove(&(temp_line._line_atrib[COLUMNS-length]),
             &(temp_line._line_atrib[0]),length*sizeof(int));
     memmove(&(temp_line._line_grph[COLUMNS-length]),
             &(temp_line._line_grph[0]),length*sizeof(char));
     for (i=0;i<(COLUMNS-length);i++)
       {
         temp_line._line_chars[i]=SCR->_scr_image[y]._line_chars[COLUMNS-i-1];
         temp_line._line_atrib[i]=SCR->_scr_image[y]._line_atrib[COLUMNS-i-1];
         temp_line._line_grph[i]=SCR->_scr_image[y]._line_grph[COLUMNS-i-1];
       }
     /* if the cursor is in the segment handled by transform,
        then shift it by the same value as the string (i.e. COLUMNS-length).
        Otherwise, the cursor is beyond the handled string, and it needs to 
        be mapped to its right to left value. */
    if (y==SCR->_logy)
     if (NewCursorPos<length)
        NewCursorPos+=COLUMNS-length;
     else 
      NewCursorPos=COLUMNS-SCR->_logx-1;
   }
   free(ToOutBuf);

  SCR->_changed[y]=FALSE;  /* reset MDT of line */
  if (y==SCR->_logy)
    oldx=SCR->_visx-1;      /* initial value */
  else oldx=-5;      /* dummy value to force initial cursor positioning */
  for (x=0;x<COLUMNS;x++)   /* loop on whole line  */
   /* check if character is differnet from physical screen image*/
   if( (temp_line._line_chars[x]    !=
       SCR->_phys_image[y]._line_chars[x])   ||
       (temp_line._line_atrib[x]!=
       SCR->_phys_image[y]._line_atrib[x])||
       (temp_line._line_grph[x]!=
       SCR->_phys_image[y]._line_grph[x]))
       {
         if (temp_line._line_atrib[x]!=active_atrib)/* set attribute */
           {
            active_atrib=temp_line._line_atrib[x];
            (*set_atrib)(active_atrib);      
           }
         if (temp_line._line_grph[x]!=active_grph) /* set graphics */
           {
            (*set_grph)(temp_line._line_grph[x],active_grph);      
            active_grph=temp_line._line_grph[x];
           }
         /* output character, check if we need cursor positioning or not */ 
         if (oldx==x-1)
            char_to_hft((unsigned int) temp_line._line_chars[x]);
         else  put_char(y,x,    
                 (unsigned int) temp_line._line_chars[x]);
         oldx=x;
         /*update phys screen */
         SCR->_phys_image[y]._line_chars[x]=temp_line._line_chars[x];
         SCR->_phys_image[y]._line_atrib[x]=temp_line._line_atrib[x];
         SCR->_phys_image[y]._line_grph[x]=temp_line._line_grph[x];
       }
  /* set cursor position */
  if (y==SCR->_logy) 
  {
    if (is_bidi_mode())
      SCR->_visx=NewCursorPos;
    else SCR->_visx=SCR->_logx;
  }
  put_cursor(SCR->_logy,SCR->_visx);  /* restore cursor position */
}
/*------------------------------------------------------------------------*/
void init_line(line,x1,x2)
/* to initialize a line from x1 to x2 */
line_type *line;
int x1,x2;
{
  int i;
  for (i=x1;i<=x2;i++)
  {
    line->_line_chars[i]=0x00;
    line->_line_atrib[i]=(SCR->_cur_atrib & 0xff000000)   /* retain colours */
              | (ATRIB_NORMAL & 0x00ffffff);
    line->_line_grph[i]=0;
  }
}
/*------------------------------------------------------------------------*/
void init_screen()
 /* to clear the screen buffer */
{
 int y;
 for (y=0;y<LINES;y++)
  {
   init_line(&(SCR->_scr_image[y]),0,COLUMNS-1);
   init_line(&(SCR->_phys_image[y]),0,COLUMNS-1);
   SCR->_changed[y]=FALSE;
  }
}
