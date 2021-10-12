static char sccsid[] = "@(#)84	1.1  src/bos/usr/bin/bterm/kbd.c, libbidi, bos411, 9428A410j 8/26/93 13:35:04";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: do_function_keys
 *		do_jump
 *		f_timeout
 *		fill_kbd_buffer
 *		kbd_input
 *		kbd_output
 *		kbd_unput
 *		parse_kbd_input
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
#include <sys/select.h>
#include <fcntl.h>
#include <termios.h>
#include <cur00.h>
#include <memory.h>
#include <string.h>
#include <term.h>
#include "global.h"
#include "trace.h"

extern unsigned char KBRD_MAP[][3];
extern unsigned char SYMETRIC_MAP[];
/*-----------------------------------------------------------------------*/
int f_timeout(time)
int time;

/* Wait "time" microseconds for input on fd = 0, the keyboard.               */
/* Return 0 if timeout occured, 1 otherwise.                                 */

{
    fd_set kbfds;
    struct timeval tmout;

   FD_ZERO(&kbfds);
   FD_SET(0, &kbfds);
   tmout.tv_sec = 0;      
   tmout.tv_usec = time;  
   return(select(1, &kbfds, NULL, NULL, &tmout));
  }

/*-----------------------------------------------------------------------*/
int fill_kbd_buffer(buff)
unsigned char *buff;
{ 
  int len;
  unsigned char ch;
int i;

  len=0;
  /* fill buffer from keyboard input */
  for (;;)
  {
   ch=get_kb_char();
   if (ch==0xff) break;    /* no input found */
   buff[len]=ch;
   len++;
   if (len==6)  /* buffer is full */ 
      break;
  }
  return(len);
}
/*-----------------------------------------------------------------------*/
/* input routine for yylex() */
int kbd_input ()
{
  int ch;
  int i;
  int len;
  unsigned char buff[6];


  /* if we have more input, consume it and advance index */
  if (buffer_index < buffer_length)
  {
     STATE=1;   /* got new input, and not yet identified */
     ch = buffer[buffer_index];
     buffer_index++;
     return (ch);
  }
  else 
 {
 if (STATE==1)  /* if we're in the middle of unrecognized input */
                /* then check if kbd has more input */
 {
  len=0;
  if(f_timeout(200000)) /* wait 2/10 sec, then read input if not timedout */
    len=fill_kbd_buffer(buff);
  if (len>0)
    {
     /* retain last 2 characters for backtracking */
     if (buffer_length<2) i=buffer_length; else i=2;  /* chars to retain */
     memmove(&buffer[0],&buffer[buffer_length-i],i);
     memmove(&buffer[i],buff,len);
     buffer_length=len+i;
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
int kbd_unput (ch)
int	ch;
{
 buffer_index--;
}
/*--------------------------------------------------------------------------*/
/* This function is used for any character not identified by yylex. 
   The character is a normal ascii character not a control.  */
int kbd_output (ch) 
int	ch;
{
  unsigned char chval[3];  /* two chars in case of lam-alef */
  int num;

 chval[0] = ch;
 chval[1]=0x00;
 chval[2]=0x00;
 num=1;
 STATE=0;   /* consumed input, not in middle of escape sequence */

 if (is_nl_kbd())
  {
    strcpy(&(chval[0]),&(KBRD_MAP[chval[0]][0]));
    if (chval[1]) num=2;
    if (is_implicit_text() && is_symetric_mode())
      chval[0]=SYMETRIC_MAP[chval[0]];
  }

 if (is_bidi_mode() && is_visual_text() && is_RTL_mode())
      chval[0]=SYMETRIC_MAP[chval[0]];

 if (is_bidi_mode()) do_bidi_check_autopush(chval[0]);
 if (is_bidi_mode() && is_visual_text() && is_push_mode())
  {
   push_char(chval[0]);
   if (num==2)  push_char(chval[1]);
  }
 else array_to_pty(&(chval[0]),num);
}
/*--------------------------------------------------------------------------*/

void parse_kbd_input()
{
  unsigned char ch;
  int i;

  buffer_length=0;
  memset(buffer,0x00,1030);
  STATE=0;
  buffer_length=fill_kbd_buffer(buffer);
  /* if there is any input then parse it */
  if (buffer_length>0)
   {
    if (status_line) remove_status();
    buffer_index=0;
    Lex_Mode=2;   /* keyboard parsing mode */
    (*yylex_function)();
    if (SCR->_changed[SCR->_logy]) update_line(SCR->_logy);
   }
  
}
/*-----------------------------------------------------------------------*/
void do_jump()
{
  STATE=0;/* identified escape sequence */
  do_bidi_end_push();
  string_to_pty(Jump);
}
/*-----------------------------------------------------------------------*/
/* Instead of the default AID code of the function key, we send the key
   stored in our array. */
void do_function_keys(key_num)
int key_num;
{
 int length;

  STATE=0;/* identified escape sequence */
  do_bidi_end_push();
  length =strlen(function_keys[key_num]);
TRACE(("function keys %d\n",key_num));
  array_to_pty(function_keys[key_num],length);
}
/*-----------------------------------------------------------------------*/
