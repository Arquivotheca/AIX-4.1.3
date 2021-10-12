static char sccsid[] = "@(#)91	1.1  src/bos/usr/bin/bterm/screen.c, libbidi, bos411, 9428A410j 8/26/93 13:35:27";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: do_backspace
 *		do_bell
 *		do_buffer_address
 *		do_carriage_return
 *		do_clear_screen
 *		do_clr_bos
 *		do_clr_eol
 *		do_clr_eos
 *		do_clr_lin
 *		do_cursor_address
 *		do_cursor_back_tab
 *		do_cursor_clear_tab
 *		do_cursor_down
 *		do_cursor_home
 *		do_cursor_left
 *		do_cursor_right
 *		do_cursor_tab_control
 *		do_cursor_up
 *		do_cursor_vertical_tab
 *		do_delete_character
 *		do_delete_line
 *		do_enter_alt2_charset_mode
 *		do_enter_alt_charset_mode
 *		do_erase_input
 *		do_exit_alt2_charset_mode
 *		do_exit_alt_charset_mode
 *		do_exit_attribute_mode
 *		do_insert_character
 *		do_insert_cursor
 *		do_insert_line
 *		do_line_feed
 *		do_next_line
 *		do_power_on_reset
 *		do_previous_line
 *		do_reset_buffer_address
 *		do_scroll_down
 *		do_scroll_up
 *		do_set_horizontal_tab
 *		do_set_vertical_tab
 *		do_tab
 *		init_h_tabs
 *		init_term
 *		init_v_tabs
 *		scr_input
 *		scr_output
 *		scr_unput
 *		scroll
 *		send_to_screen
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
/*	Documentation:  screen.c
**
**		This is the "important" part of the program.
**		The function send_to_screen parses the input stream   
**		using yylex, and performs the required screen functions.
**		Escape sequences and special controls are automatically 
**		mapped to their functions, any unidentified input is handled
**              in function output1().
**
*/

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <curses.h>
#include <term.h>

#include "global.h"
#include "trace.h"

#undef delch()
#undef insch()

void do_line_feed();
void do_carriage_return();
void scroll();
void do_delete_line();
void do_insert_character();
void do_bell();
void init_v_tabs();
void init_h_tabs();

/*===========================================================*/

void send_to_screen(chbuf,len)
unsigned char *chbuf;
int len;
{
  int i,j;
  unsigned char chval;

int temp;

  STATE=0;  /* initialize lex parsing state */
  Lex_Mode=1;  /* screen input parsing */
  memmove(&buffer[0],chbuf,len); /* set variables needed by yylex */
  buffer_index = 0;
  buffer_length= len;
  (*yylex_function)();              /* parse input stream */
  if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
  return;
}
/*----------------------------------------------------------------*/
/* input routine for yylex() */
int scr_input ()
{
  int ch;
  int i;

int temp;

  /* if we have more input, consume it and advance index */
  if (buffer_index < buffer_length)
  {
     ch = buffer[buffer_index];
     buffer_index++;
     STATE=1;  /* new, not yet recognized input */
     return (ch);
  }
  else 
 {
 /* if we're out of input, then check if pty has more input */
 if (STATE==1)  /* if we're in the middle of unrecognized input */
 {
 if ((jdx=get_from_pty(new_jdx))>0)
    {
     /* retain last 3 characters for backtracking */
     if (buffer_length<3) i=buffer_length; else i=3;  /* chars to retain */
     memmove(&buffer[0],&buffer[buffer_length-i],i);
     memmove(&buffer[i],new_jdx,jdx);
     buffer_length=jdx+i;
     buffer_index=i;
     ch = buffer[buffer_index]; /* now consume 1 char */
     buffer_index++;
     return(ch);
    }
 else
    { /* no more input, return null */
     buffer_index++;
     return (0x00);
    }
 }
 else
    { /* not in middle of escape. so don't get more input */
     buffer_index++;
     return (0x00);
    }
 }
}
/*--------------------------------------------------------------------------*/
/* to put a character back into lex input stream */
int scr_unput (ch)
int	ch;
{
 buffer_index--;
}
/*--------------------------------------------------------------------------*/
/* This function is used for any character not identified by yylex. 
   The character is either a normal ascii character or an unidentified
   escape sequence. It is left to the screen to handle those */
int scr_output (ch) 
int	ch;
{
  unsigned char chval;
 STATE=0;  /* recognized char, so reset state */
 chval = ch;
 /* check insert mode */
 if (INSERT) do_insert_character(1);
 /* put the character in the screen buffer */
 SCR->_scr_image[SCR->_logy]._line_chars[SCR->_logx]=chval;
 SCR->_scr_image[SCR->_logy]._line_atrib[SCR->_logx]=SCR->_cur_atrib;
 SCR->_scr_image[SCR->_logy]._line_grph[SCR->_logx]=SCR->_cur_grph;
 SCR->_changed[SCR->_logy]=TRUE; /* set MDT of current line */
 if ((SCR->_logy==(LINES-1)) && (SCR->_logx==(COLUMNS-1)) && SCROLL && WRAP)
              {
               if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
               scroll();
              }
 else if (SCR->_logx < (COLUMNS-1))   /* increment x position */
     SCR->_logx++;
 else if ((SCR->_logy < (LINES-1)) && WRAP)   /* wrap to next line */
      {
       if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
       SCR->_logx=0;
       SCR->_logy++;
       if (is_RTL_mode()) SCR->_visx=COLUMNS-1; else SCR->_visx=0;
       put_cursor(SCR->_logy,SCR->_visx);
      }
}
/*--------------------------------------------------------------------------*/
void scroll()
/* the screen should scroll because we have writen a character at the last
   position, so we need to adjust our buffer accordingly. */ 
{
int i;
TRACE(("scrolling \n"));
  put_cursor(0,0);
  SCR->_logy=0;
  do_delete_line(1);
  SCR->_logy=LINES-1;
  SCR->_logx=0;
  if (is_RTL_mode()) SCR->_visx=COLUMNS-1; else SCR->_visx=0;
  put_cursor(SCR->_logy,SCR->_visx);
}
/*-------------------------------------------------------------------------*/

void do_carriage_return()
/* cursor moves to beginning of same line */
{
 TRACE(("carriage return\n"));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)   /* keyboard mode */
  {
   do_bidi_end_push(); /* is push active, end it */
   string_to_pty(carriage_return);
   return;
  }
 if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
 SCR->_logx = 0;
 if (is_RTL_mode()) SCR->_visx=COLUMNS-1; else SCR->_visx=0;
 put_cursor(SCR->_logy,SCR->_visx);
 if (AUTOLF) do_line_feed();  
}

/*--------------------------------------------------------------------------*/
void do_delete_line(count)
int count;
/*deletes current line, moves all lines upwards and
  inserts a null line at the bottom of the screen */
{
 int i;

 TRACE(("delete line %d times \n",count));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)  /* keyboard mode */
 {
  do_bidi_end_push(); /* is push active, end it */
  string_to_pty(key_dl);
  return; 
 }
if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
if (SCR->_logy+count>(LINES-1)) count=LINES-1-SCR->_logy;
memmove(&(SCR->_scr_image[SCR->_logy]._line_chars[0]),
        &(SCR->_scr_image[SCR->_logy+count]._line_chars[0]),
        (LINES-SCR->_logy-count)*sizeof(line_type)); 
memmove(&(SCR->_phys_image[SCR->_logy]._line_chars[0]),
        &(SCR->_phys_image[SCR->_logy+count]._line_chars[0]),
        (LINES-SCR->_logy-count)*sizeof(line_type)); 
for (i=0;i<count;i++)
{
  init_line(&(SCR->_scr_image[LINES-1-i]),0,COLUMNS-1);
  init_line(&(SCR->_phys_image[LINES-1-i]),0,COLUMNS-1);
  string_to_hft(delete_line);
}
}

/*--------------------------------------------------------------------------*/
void do_line_feed()
/* cursor moves to same position in next line */
{
 int y;

 TRACE(("line feed\n"));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)  /* keyboard mode */
 {
  do_bidi_end_push(); /* is push active, end it */
  string_to_pty(scroll_forward);
  return; 
 }
 do_bidi_end_push(); /* is push active, end it */
 if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
 if (SCR->_logy<(LINES-1)) 
   {
    SCR->_logy++;
    string_to_hft(cursor_down);
   }
 else  /* scroll 1 line */
   {
    scroll();
   }
}

/*--------------------------------------------------------------------------*/
void do_clear_screen()
/* clears the screen and sets the cursor to home */
{
 TRACE(("clear screen\n"));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)  /* keyboard mode */
 {
  do_bidi_end_push(); /* is push active, end it */
  string_to_pty(key_clear);
  return; 
 }
 (*set_atrib)(active_atrib);
 string_to_hft(clear_screen);
 init_screen();
 SCR->_logx = 0;
 SCR->_logy = 0;
 if (is_RTL_mode()) SCR->_visx=COLUMNS-1; else SCR->_visx=0;
 put_cursor(SCR->_logy,SCR->_visx);
}
/*-----------------------------------------------------------------------*/
void do_erase_input()
/* clears the screen and sets the cursor to home */
{
  TRACE(("erase input\n"));
  STATE=0;  /* recognized char, so reset state */
  if (Lex_Mode==2)  /* keyboard mode */
  {
   do_bidi_end_push();
   string_to_pty(EraseInput);
   return;
  }
  do_clear_screen();
}
/*--------------------------------------------------------------------------*/
void do_exit_alt_charset_mode()
/* exit alternate graphics characters mode */
{
 TRACE(("exit altcharset\n"));
 STATE=0;  /* recognized char, so reset state */
 SCR->_cur_grph = 0;
}

/*--------------------------------------------------------------------------*/
void do_enter_alt_charset_mode()
/* enter alternate graphics characters mode */
{
 TRACE(("enter altcharset\n"));
 STATE=0;  /* recognized char, so reset state */
 SCR->_cur_grph = 1;
}

/*--------------------------------------------------------------------------*/
void do_enter_alt2_charset_mode()
/* enter alternate graphics characters mode */
{

 TRACE(("enter altcharset\n"));
 STATE=0;  /* recognized char, so reset state */
 SCR->_cur_grph = 2;
}

/*--------------------------------------------------------------------------*/
void do_exit_alt2_charset_mode()
/* exit alternate graphics characters mode */
{

 TRACE(("exit altcharset\n"));
 STATE=0;  /* recognized char, so reset state */
 SCR->_cur_grph = 0;
}

/*--------------------------------------------------------------------------*/
void do_exit_attribute_mode()
{
 TRACE(("exit attribute mode\n"));
 STATE=0;  /* recognized char, so reset state */
 SCR->_cur_atrib=(SCR->_cur_atrib & 0xff000000)   /* retain colours */
              | (ATRIB_NORMAL & 0x00ffffff);
}

/*--------------------------------------------------------------------------*/
void do_power_on_reset()
{
 TRACE(("reset terminal\n"));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)  /* keyboard mode */
   {
     string_to_pty(reset_2string);
     return;
   }
 SCR->_cur_atrib=ATRIB_NORMAL;
 SCR->_cur_grph=0;
 do_clear_screen();
 init_v_tabs();
 init_h_tabs();
}

/*--------------------------------------------------------------------------*/
void do_cursor_up(count)
int count;
{
 TRACE(("cursor up\n"));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)  /* keyboard mode */
   {
    do_bidi_end_push(); /* is push active, end it */
    /* ignore count, from kbd it must be one */
    string_to_pty(key_up);
    return;
   }
 if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
 if ((SCR->_logy-count)>=0) SCR->_logy-=count;
       else SCR->_logy=0;
 SCR->_changed[SCR->_logy]=TRUE;
 update_line(SCR->_logy);
}

/*--------------------------------------------------------------------------*/
void do_cursor_down(count)
int count;
{
 TRACE(("cursor down\n"));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)  /* keyboard mode */
   {
    do_bidi_end_push(); /* is push active, end it */
    /* ignore count, from kbd it must be one */
    string_to_pty(key_down);
    return;
   }
 if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
 if ((SCR->_logy+count)<LINES) SCR->_logy+=count;
       else SCR->_logy=LINES-1;
 SCR->_changed[SCR->_logy]=TRUE;
 update_line(SCR->_logy);
}

/*--------------------------------------------------------------------------*/
void do_cursor_right(count)
int count;
{
 TRACE(("cursor right\n"));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)  /* keyboard mode */
   {
    /* ignore count, from kbd it must be one */
    if (is_RTL_mode())   /* reverse arrow keys */
      if (is_bidi_mode() && is_visual_text() && is_push_mode())
        push_cursor_left();
      else string_to_pty(key_left);
    else 
      if (is_bidi_mode() && is_visual_text() && is_push_mode())
        push_cursor_right();
      else string_to_pty(key_right);
    return;
   }
 if ((SCR->_logx+count)<COLUMNS) SCR->_logx+=count;
        else SCR->_logx=COLUMNS-1;
 SCR->_changed[SCR->_logy]=TRUE;
 update_line(SCR->_logy);
}

/*--------------------------------------------------------------------------*/
void do_cursor_left(count)
int count;
{
 TRACE(("cursor left\n"));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)  /* keyboard mode */
   {
    /* ignore count, from kbd it must be one */
    if (is_RTL_mode())   /* reverse arrow keys */
      if (is_bidi_mode() && is_visual_text() && is_push_mode())
        push_cursor_right();
      else string_to_pty(key_right);
    else 
      if (is_bidi_mode() && is_visual_text() && is_push_mode())
        push_cursor_left();
      else string_to_pty(key_left);
    return;
   }
 if ((SCR->_logx-count)>=0) SCR->_logx-=count;
       else SCR->_logx=0;
 SCR->_changed[SCR->_logy]=TRUE;
 update_line(SCR->_logy);
}

/*--------------------------------------------------------------------------*/
void do_cursor_home()
{
 TRACE(("cursor home\n"));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)  /* keyboard mode */
   {
    do_bidi_end_push(); /* is push active, end it */
    string_to_pty(key_home);
    return;
   }
 if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
 SCR->_logy=0;
 SCR->_logx=0;
 SCR->_changed[SCR->_logy]=TRUE;
 update_line(SCR->_logy);
 put_cursor(SCR->_logy,SCR->_visx);
}
/*--------------------------------------------------------------------------*/
void do_clr_lin(x1,x2)
int x1,x2;
/*clears current line from x1 position to x2 position */
{
 int i;
 TRACE(("clrlin y=%d from %d to %d\n",SCR->_logy,x1,x2));
 STATE=0;  /* recognized char, so reset state */
 init_line(&(SCR->_scr_image[SCR->_logy]),x1,x2);
 SCR->_changed[SCR->_logy]=TRUE;
 update_line(SCR->_logy);
}
/*--------------------------------------------------------------------------*/
void do_clr_eol()
/*clears current line from cursor position to end of line */
{
 int i;
 TRACE(("clreol\n"));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)  /* keyboard mode */
   {
    do_bidi_end_push(); /* is push active, end it */
    string_to_pty(key_eol);
    return;
   }
 do_clr_lin(SCR->_logx,COLUMNS-1);
}
/*--------------------------------------------------------------------------*/
void do_clr_eos()
/* clears from cursor position to end of screen */
{
 int i;

 TRACE(("clreos\n"));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)  /* keyboard mode */
   {
    do_bidi_end_push(); /* is push active, end it */
    string_to_pty(key_eos);
    return;
   }
if ((SCR->_logy==0) && (SCR->_logx==0))
  do_clear_screen();
else
 { 
  if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
  for (i=SCR->_logy+1;i<LINES;i++)
    {
     init_line(&(SCR->_scr_image[i]),0,COLUMNS-1);
     init_line(&(SCR->_phys_image[i]),0,COLUMNS-1);
    }
  do_clr_lin(SCR->_logx,COLUMNS-1);
  if (SCR->_logy<(LINES-1)) 
      {
       put_cursor(SCR->_logy+1,0);
       string_to_hft(clr_eos);
      }
 }
}

/*--------------------------------------------------------------------------*/
void do_clr_bos()
/* clears from begiining of screen to cursor position */
{
 int i;
 int oldy;

  TRACE(("clrbos\n"));
  STATE=0;  /* recognized char, so reset state */
  if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
  oldy=SCR->_logy;
  for (i=0;i<oldy;i++)
  {
   SCR->_logy=i;
   do_clr_lin(0,COLUMNS-1);
  }
 SCR->_logy=oldy;
 do_clr_lin(0,SCR->_logx);
}

/*--------------------------------------------------------------------------*/
void do_insert_line(count)
int count;
/*inserts a null line at current cursor address and
  shifts all lines on screen downwards */
{
 int i;

 TRACE(("insert line %d times \n",count));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)  /* keyboard mode */
 {
  /* ignore count, from kbd it must be one */
  do_bidi_end_push(); /* is push active, end it */
  string_to_pty(key_il);
  return; 
 }
if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
if (SCR->_logy+count>LINES)
        count=LINES-SCR->_logy; /* maximum allowed */
memmove(&(SCR->_scr_image[SCR->_logy+count]._line_chars[0]),
        &(SCR->_scr_image[SCR->_logy]._line_chars[0]),
        (LINES-SCR->_logy-count)*sizeof(line_type)); 
memmove(&(SCR->_phys_image[SCR->_logy+count]._line_chars[0]),
        &(SCR->_phys_image[SCR->_logy]._line_chars[0]),
        (LINES-SCR->_logy-count)*sizeof(line_type)); 
for (i=0;i<count;i++)
{
  string_to_hft(insert_line);
  init_line(&(SCR->_scr_image[SCR->_logy+i]),0,COLUMNS-1);
  init_line(&(SCR->_phys_image[SCR->_logy+i]),0,COLUMNS-1);
}
}

/*--------------------------------------------------------------------------*/
void do_insert_character(count)
int count;
/* inserts null characters at current position and shifts whole line */
{
 TRACE(("insert %d char\n",count));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)  /* keyboard mode */
 {
  string_to_pty(key_ic);
  return; 
 }
 memmove(&(SCR->_scr_image[SCR->_logy]._line_chars[SCR->_logx+count]),
         &(SCR->_scr_image[SCR->_logy]._line_chars[SCR->_logx]),
         (COLUMNS-SCR->_logx-count)*sizeof(char));
 memmove(&(SCR->_scr_image[SCR->_logy]._line_atrib[SCR->_logx+count]),
         &(SCR->_scr_image[SCR->_logy]._line_atrib[SCR->_logx]),
         (COLUMNS-SCR->_logx-count)*sizeof(int));
 memmove(&(SCR->_scr_image[SCR->_logy]._line_grph[SCR->_logx+count]),
         &(SCR->_scr_image[SCR->_logy]._line_grph[SCR->_logx]),
         (COLUMNS-SCR->_logx-count)*sizeof(char));
 init_line(&(SCR->_scr_image[SCR->_logy]),SCR->_logx,SCR->_logx+count);
 SCR->_changed[SCR->_logy]=TRUE;
 update_line(SCR->_logy);
}

/*--------------------------------------------------------------------------*/
void do_delete_character(count)
int count;
/* deletes characters and shifts the whole line and
   inserts null at the end of line */
{
 TRACE(("delete char %d\n",count));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)  /* keyboard mode */
 {
  if (is_bidi_mode() && is_visual_text() && is_push_mode())
   push_delete();
  else string_to_pty(key_dc);
  return; 
 }
 memmove(&(SCR->_scr_image[SCR->_logy]._line_chars[SCR->_logx]),
         &(SCR->_scr_image[SCR->_logy]._line_chars[SCR->_logx+count]),
         (COLUMNS-SCR->_logx-count)*sizeof(char));
 memmove(&(SCR->_scr_image[SCR->_logy]._line_atrib[SCR->_logx]),
         &(SCR->_scr_image[SCR->_logy]._line_atrib[SCR->_logx+count]),
         (COLUMNS-SCR->_logx-count)*sizeof(int));
 memmove(&(SCR->_scr_image[SCR->_logy]._line_grph[SCR->_logx]),
         &(SCR->_scr_image[SCR->_logy]._line_grph[SCR->_logx+count]),
         (COLUMNS-SCR->_logx-count)*sizeof(char));
 init_line(&(SCR->_scr_image[SCR->_logy]),COLUMNS-1-count,COLUMNS-1);
 SCR->_changed[SCR->_logy]=TRUE;
 update_line(SCR->_logy);
}

/*--------------------------------------------------------------------------*/
void do_insert_cursor()
/* buffer address mode is not implemented */
{
}
/*--------------------------------------------------------------------------*/
void do_cursor_address(y,x)
/* sets cursor address */
int y,x;
{
 TRACE(("cursor address\n"));
 STATE=0;  /* recognized char, so reset state */
 if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
 if (x < 0) x=0;
 if (x > (COLUMNS-1)) x=COLUMNS-1;
 if (y < 0) y=0;
 if (y > (LINES-1)) y=LINES-1;
 SCR->_logy=y;
 SCR->_logx=x;
 SCR->_changed[SCR->_logy]=TRUE;
 update_line(SCR->_logy);
}

/*--------------------------------------------------------------------------*/
void do_reset_buffer_address()
/* buffer address mode is not implemented */
{
}
/*--------------------------------------------------------------------------*/
void do_buffer_address(y,x)
int y,x;
/* buffer address mode is not implemented, 
   so we use this command to set cursor address */
{
 do_cursor_address(y,x);
}
/*--------------------------------------------------------------------------*/
void do_bell()
/* sounds the audible bell */
{
 TRACE(("bell\n"));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)  /* keyboard mode */
 {
  string_to_pty(bell);
  return; 
 }
 if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
 string_to_hft(bell);
}

/*--------------------------------------------------------------------------*/
void do_tab()
{
 TRACE(("tab\n"));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)  /* keyboard mode */
 {
  do_bidi_end_push(); /* is push active, end it */
  string_to_pty(key_tab);
  return; 
 }
 do_bidi_end_push(); /* is push active, end it */
 if (SCR->_logx+1<COLUMNS)
 {
    SCR->_logx++;
    while (!H_tabs[SCR->_logx]) SCR->_logx++;
 }
 SCR->_changed[SCR->_logy]=TRUE;
 update_line(SCR->_logy);
}
/*--------------------------------------------------------------------------*/
void do_cursor_vertical_tab()
{
 TRACE(("vertical tab\n"));
 STATE=0;  /* recognized char, so reset state */
 do_bidi_end_push(); /* is push active, end it */
 if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
 if (SCR->_logy+1<LINES)
 {
    SCR->_logy++;
    while (!V_tabs[SCR->_logy]) SCR->_logy++;
 }
 SCR->_changed[SCR->_logy]=TRUE;
 update_line(SCR->_logy);
}
/*--------------------------------------------------------------------------*/
void do_cursor_back_tab()
{
 TRACE(("back_tab\n"));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)  /* keyboard mode */
 {
  do_bidi_end_push(); /* is push active, end it */
  string_to_pty(key_back_tab);
  return; 
 }
 do_bidi_end_push(); /* is push active, end it */
 if (SCR->_logx-1<=0)
 {
    SCR->_logx--;
    while (!H_tabs[SCR->_logx]) SCR->_logx--;
 }
 SCR->_changed[SCR->_logy]=TRUE;
 update_line(SCR->_logy);
}
/*--------------------------------------------------------------------------*/
void do_backspace()
{
 TRACE(("backspace\n"));
 STATE=0;  /* recognized char, so reset state */
 if (Lex_Mode==2)   /* keyboard mode */
  {
   if (is_bidi_mode() && is_visual_text() && is_push_mode())
    do_cursor_left();
   else string_to_pty(key_backspace);
   return;
  }
 do_cursor_left();
}
/*--------------------------------------------------------------------------*/
void do_next_line()
/* move cursor to first position on next line */
{
  STATE=0;
  if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
  SCR->_logx=0;
  if (SCR->_logy<(LINES-1)) SCR->_logy++;
  SCR->_changed[SCR->_logy]=TRUE;
}
/*--------------------------------------------------------------------------*/
void do_previous_line()
/* move cursor to first position on previous line */
{
  STATE=0;
  if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
  SCR->_logx=0;
  if (SCR->_logy>0) SCR->_logy--;
  SCR->_changed[SCR->_logy]=TRUE;
}
/*-------------------------------------------------------------------------*/
void do_scroll_up(count)
int count;
{
 int oldy;

TRACE(("scrolling up %d lines\n",count));
 STATE=0;
 if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
 oldy=SCR->_logy;
 put_cursor(0,0);
 SCR->_logy=0;
 do_delete_line(count);
 SCR->_logy=oldy;
 put_cursor(SCR->_logy,SCR->_visx);
}
/*-------------------------------------------------------------------------*/
void do_scroll_down(count)
int count;
{
 int oldy;

 STATE=0;
 if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
 oldy=SCR->_logy;
 SCR->_logy=0;
 put_cursor(0,0);
 do_insert_line(count);
 SCR->_logy=oldy;
 put_cursor(SCR->_logy,SCR->_visx);
}
/*--------------------------------------------------------------------------*/
void init_v_tabs()
{
 int i;

 TRACE(("initilizing all vertical tabs 	\n"));
 for (i=0;i<LINES;i++) V_tabs[i]=0;
 V_tabs[0]=1;
 V_tabs[LINES-1]=1;
}
/*--------------------------------------------------------------------------*/
void init_h_tabs()
{
 int i;

 TRACE(("initilizing all horizontal tabs \n"));
 for (i=0;i<COLUMNS;i++) H_tabs[i]=0;
 for(i = 0; i < COLUMNS/8; i++) H_tabs[8*i] = 1;
 H_tabs[0]=1;
 H_tabs[COLUMNS-1]=1;
}
/*--------------------------------------------------------------------------*/
void do_set_horizontal_tab()
{
 STATE=0;
 TRACE(("set horizontal tab at %d\n",SCR->_logx));
 H_tabs[SCR->_logx]=1;
}
/*--------------------------------------------------------------------------*/
void do_set_vertical_tab()
{
 STATE=0;
 TRACE(("set vertical tab at %d\n",SCR->_logy));
 V_tabs[SCR->_logy]=1;
}
/*--------------------------------------------------------------------------*/
void do_cursor_tab_control(count)
int count;
{
 int i;

 STATE=0;
 TRACE(("control tab with param %d\n",count));
 switch (count)
    {
      case 0:   /* set horizontal tab stop */
        H_tabs[SCR->_logx] = 1;
        break;

      case 1:   /* set vertical tab stop   */
        V_tabs[SCR->_logy] = 1;
        break;

      case 2:   /* clear horizontal tab stop */
        H_tabs[SCR->_logx] = 0;
        H_tabs[0]=1;
        H_tabs[LINES-1]=1;
        break;

      case 3:   /* clear vertical tab stop   */
        V_tabs[SCR->_logy] = 0;
        V_tabs[0]=1;
        V_tabs[LINES-1]=1;
        break;

      case 4:   /* clear all horizontal tabs on line */
      case 5:   /* clear all horizontal tabs */
        for (i=0;i<COLUMNS;i++) H_tabs[i]=0;
        H_tabs[0]=1;
        H_tabs[COLUMNS-1]=1;
        break;

      case 6:   /* clear all vertical tabs */
        for (i=0;i<LINES;i++) V_tabs[i]=0;
        V_tabs[0]=1;
        V_tabs[LINES-1]=1;
        break;
    }
}
/*--------------------------------------------------------------------------*/
void do_cursor_clear_tab(count)
int count;
{
 int i;

 STATE=0;
 TRACE(("clear tab with param %d\n",count));
 switch (count)
    {
      case 0:   /* clear horizontal tab stop */
        H_tabs[SCR->_logx] = 0;
        H_tabs[0]=1;
        H_tabs[LINES-1]=1;
        break;

      case 1:   /* clear vertical tab stop   */
        V_tabs[SCR->_logy] = 0;
        V_tabs[0]=1;
        V_tabs[LINES-1]=1;
        break;

      case 2:   /* clear all horizontal tabs on line */
      case 3:   /* clear all horizontal tabs */
        for (i=0;i<COLUMNS;i++) H_tabs[i]=0;
        H_tabs[0]=1;
        H_tabs[COLUMNS-1]=1;
        break;

      case 4:   /* clear all vertical tabs */
        for (i=0;i<LINES;i++) V_tabs[i]=0;
        V_tabs[0]=1;
        V_tabs[LINES-1]=1;
        break;
    }
}
/*--------------------------------------------------------------------------*/
/* to initialize the terminal functions and escapes */
/* Returns 0 if terminal not supported. */
int init_term(term)
char *term;
{
     if (strcmp(term,"ibm3151")==0)
         {
             init_3151();
             return (1);
         }
    if ((strcmp(term,"vt320")==0)
     || (strcmp(term,"vt220")==0)
     || (strcmp(term,"vt100")==0))
         {
             init_vt220();
             return (1);
         }
    if (strcmp(term,"hft")==0)
         {
             init_hft();
             return (1);
         }
    if (strcmp(term,"aixterm")==0)
         {
             init_aixterm();
             return (1);
         }
     else  return (0); /* not recognized */
}
