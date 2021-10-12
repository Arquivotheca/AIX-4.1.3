static char sccsid[] = "@(#)95	1.1  src/bos/usr/bin/bterm/vt220func.c, libbidi, bos411, 9428A410j 8/26/93 13:35:37";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: do_vt220_clr_lin
 *		do_vt220_clr_scr
 *		do_vt220_cursor_address
 *		do_vt220_pa_keys
 *		do_vt220_reset_function_keys
 *		do_vt220_set_attributes
 *		do_vt220_soft_reset
 *		get_vt220_param
 *		init_vt220
 *		set_vt220_atrib
 *		set_vt220_grph
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
/* vt220 specific functions */

#include "global.h"
#include "trace.h"

void set_vt220_atrib();
void set_vt220_grph();
void do_vt220_reset_function_keys();
int yylex();

/*------------------------------------------------------------------------*/
/* vt220 attribute masks */
#define VT220_NORMAL      0x20000000 /* default is green on black */
#define VT220_BOLD        0x00000001
#define VT220_UNDERLINE   0x00000002
#define VT220_BLINK       0x00000004
#define VT220_REVERSE     0x00000008
#define VT220_BOLD_OFF        0xfffffffe
#define VT220_UNDERLINE_OFF   0xfffffffd
#define VT220_BLINK_OFF       0xfffffffb
#define VT220_REVERSE_OFF     0xfffffff7
/*------------------------------------------------------------------------*/

void init_vt220()
/* to initialize vt220 functions and escapes */
{
TRACE(("initializing vt220\n"));
       TERM=VT220;
       ATRIB_NORMAL=VT220_NORMAL; 
       EraseInput="\033K";
       NoScroll="\033[?7l";
       Scroll="\033[?7h";
       /* this is the escape that select vt200 mode for the IBM
          and DEC terminals emulation cartridge of the ibm3151 terminal. */
       TERM_INIT="\033 9<e";

       set_atrib=set_vt220_atrib;
       set_grph=set_vt220_grph;
       do_reset_function_keys=do_vt220_reset_function_keys;
       yylex_function=yylex;
}
/*----------------- get vt220 parameter ----------------------------------*/
/* to get the numeric parameter from the buffer, assuming the
   index is after the terminator , 
    Format assumed is : ^[[111T, where T is the terminator. */
int get_vt220_param()
{
  int i,num;

  i=buffer_index-2;  /* point to before the escape terminator */
  while (isdigit(buffer[i]))
      i--; /* now i should be pointing at the [  */
  i++;
  num=atoi(&buffer[i]);
  if  (num==0) num=1;
TRACE(("parameter is %d\n",num));
  return(num);
}
/*------------------------------------------------------------------------*/
void do_vt220_clr_lin()
{
  STATE=0;
  switch (buffer[buffer_index-2])
    {
       case '[' :  /* no paramter */
       case '0' :  do_clr_lin(SCR->_logx,COLUMNS-1); 
                   break; /* from cursor to end of line */
       case '1' :  do_clr_lin(0,SCR->_logx); 
                   break; /* from start of line to cursor */
       case '2' :  do_clr_lin(0,COLUMNS-1); 
                   break;  /* whole of line */
    }
}
/*------------------------------------------------------------------------*/
void do_vt220_clr_scr()
{
  STATE=0;
  switch (buffer[buffer_index-2])
    {
       case '[' :/* no parameter */
       case '0' :  do_clr_eos(); break; /* from cursor to end of screen */
       case '1' :  do_clr_bos(); break; /* from start of screen to cursor */
       case '2' :  do_clear_screen(); break;  /* whole of screen */
    }
}
/*------------------------------------------------------------------------*/
/*get cursor addressing parameters and then call cursor addressing. */
void do_vt220_cursor_address()
{
 int i;
 int x,y;

 i=buffer_index-1;
 while (buffer[i]!='[') i--;
 i++; /* now i at start of y value */
 y=atoi(&buffer[i])-1;
 while (isdigit(buffer[i])) i++;  /* skip y value */ 
 i++;  /* skip the semi colon */
 x=atoi(&buffer[i])-1;
TRACE(("vt220 cursor address  x=%d y=%d\n",x,y));
 do_cursor_address(y,x); 
}
/*----------------- set screen attributes --------------------------------*/
void set_vt220_atrib(atrib)
int atrib;
{
 char atts[12];  /* longest possible is "E[0;1;4;5;7m"  */
 int i;
 int fore,back;

TRACE(("setting vt220 atrib %x\n",atrib));
 atts[0]=0x1b;   /* escape */
 atts[1]=0x5b;     /* [ */
 atts[2]='0';      /* reset all */
 i=3;
 if (atrib&VT220_BOLD)      {atts[i++]=';'; atts[i++]='1';}
 if (atrib&VT220_UNDERLINE) {atts[i++]=';'; atts[i++]='4';}
 if (atrib&VT220_BLINK)     {atts[i++]=';'; atts[i++]='5';}
 if (atrib&VT220_REVERSE)   {atts[i++]=';'; atts[i++]='7';}
 atts[i++]=0x6d;     /* m */
 array_to_hft(atts,i);

 /* set foreground and background colors */
  fore = atrib & 0xf0000000;
  fore = fore>>28;
  back = atrib & 0x0f000000;
  back = back>>24;
  string_to_hft("\033[");
  if (fore < 8)
     fore = fore + 30;
  else
     fore = fore + 82;
  sprintf(atts, "%d", fore);
  string_to_hft(atts);
  if(back < 8)
    back = back + 40;
  else
    back = back + 92;
  sprintf(atts, ";%d", back);
  string_to_hft(atts);
  char_to_hft('m');
}
/*----------------- set alternate grphics mode -----------------------------*/
void set_vt220_grph(grph,old_grph)
unsigned short grph;
unsigned short old_grph;
{
 char atts[2];

 atts[0]=0x1b;   /* escape */
 switch (grph)
  {
   case 0:           /* reset alternate graphics */
     if (old_grph==1)
     {
       atts[1]=0x28;   /* ( */
       atts[2]=0x42;   /* B */
       array_to_hft(atts,3);
     }
     else 
     {
     atts[1]=0x7d;   /* } */
     array_to_hft(atts,2);
     }
     break;
   case 1:           /* set G0 */
     atts[1]=0x28;   /* ( */
     atts[2]=0x30;   /* 0 */
     array_to_hft(atts,3);
     break;
   case 2:           /* set G1 */
     atts[1]=0x7e;   /* ~ */
     array_to_hft(atts,2);
     break;
  }
}
/*-------------- set vt220 attributes -------------------------------*/
void do_vt220_set_attributes ()
{
  int i,num;
  int fore,back;

  TRACE(("setting vt220 attributes\n"));
  STATE=0;
  i=buffer_index-1;
  while (buffer[i]!='[') i--;   /* go back to start of sequence */
  i++;   /* now standing on first digit */
  while (buffer[i]!='m') /* loop until end of sequence */
  {
    num=0;
    if (isdigit(buffer[i]))
     {
       num = atoi(&buffer[i]);
       while (isdigit(buffer[i])) i++; /* skip all digits already handled */
       if (buffer[i]!='m') i++; /* skip the semicolon */
     }
     else  i++;
TRACE(("num=%d\n",num));
    switch (num)
       {
         case 0 :  /* reset attribute mode */
                   TRACE(("reset atts\n"));
                   SCR->_cur_atrib&=0xff000000;
                   break;
         case 1 :  /* turn on bold */
                   TRACE(("set bold\n"));
                   SCR->_cur_atrib|=VT220_BOLD;
                   break;
         case 4 :  /* turn on underline */
                   TRACE(("set underline\n"));
                   SCR->_cur_atrib|=VT220_UNDERLINE;
                   break;
         case 5 :  /* turn on blink */
                   TRACE(("set blink\n"));
                   SCR->_cur_atrib|=VT220_BLINK;
                   break;
         case 7 :  /* turn on reverse */
                   TRACE(("set reverse\n"));
                   SCR->_cur_atrib|=VT220_REVERSE;
                   break;
         case 22:  /* turn off bold */
                   TRACE(("reset bold\n"));
                   SCR->_cur_atrib&=VT220_BOLD_OFF;
                   break;
         case 24:  /* turn off underline */
                   TRACE(("reset underline\n"));
                   SCR->_cur_atrib&=VT220_UNDERLINE_OFF;
                   break;
         case 25:  /* turn off blink */
                   TRACE(("reset blink\n"));
                   SCR->_cur_atrib&=VT220_BLINK_OFF;
                   break;
         case 27:  /* turn off reverse */
                   TRACE(("reset reverse\n"));
                   SCR->_cur_atrib&=VT220_REVERSE_OFF;
                   break;
       }
       /* set foreground and background colors */
       fore = SCR->_cur_atrib & 0xf0000000;
       fore = fore>>28;
       back = SCR->_cur_atrib & 0x0f000000;
       back = back>>24;
       if((num > 29) && (num < 38))          /* foreground 1 - 8  */
          fore = num - 30;
        else if((num > 39) && (num < 48))     /* background 1 - 8  */
          back = num - 40;
        else if ((num > 89) && (num < 98))    /* foreground 9 - 16 */
          fore = num - 82;
        else if((num > 99) && (num < 108))    /* background 9 - 16 */
          back = num - 92;
        SCR->_cur_atrib = SCR->_cur_atrib & 0x00ffffff;
        SCR->_cur_atrib = SCR->_cur_atrib  | (fore<<28) | (back<<24);
TRACE(("now atrib=%x\n",SCR->_cur_atrib));

  }
}
/*----------- reset all function keys to default AID value ----------*/
void do_vt220_reset_function_keys()
{
 int i,j;

 STATE=0;  /* recognized char, so reset state */
 for (i=0;i<=9;i++)
   {
     function_keys[i][0]=0x1b;     /* escape */
     function_keys[i][1]=0x5b;     /* [ */
     function_keys[i][2]=0x30+i;   /* 0..9 */
     function_keys[i][3]=0x7e;     /* ~ */
     function_keys[i][4]=0x00;
   }
 for (i=10,j=0;i<=19;i++,j++)
   {
     function_keys[i][0]=0x1b;     /* escape */
     function_keys[i][1]=0x5b;     /* [ */
     function_keys[i][2]=0x31;     /* 1 */
     function_keys[i][3]=0x30+j;   /* 0..9 */
     function_keys[i][4]=0x7e;     /* ~ */
     function_keys[i][5]=0x00;
   }
 for (i=20,j=0;i<=29;i++,j++)
   {
     function_keys[i][0]=0x1b;     /* escape */
     function_keys[i][1]=0x5b;     /* [ */
     function_keys[i][2]=0x32;     /* 2 */
     function_keys[i][3]=0x30+j;   /* 0..9 */
     function_keys[i][4]=0x7e;     /* ~ */
     function_keys[i][5]=0x00;
   }
 for (i=30,j=0;i<=36;i++,j++)
   {
     function_keys[i][0]=0x1b;     /* escape */
     function_keys[i][1]=0x5b;     /* [ */
     function_keys[i][2]=0x33;     /* 3 */
     function_keys[i][3]=0x30+j;   /* 0..6 */
     function_keys[i][4]=0x7e;     /* ~ */
     function_keys[i][5]=0x00;
   }

}
/*-----------------------------------------------------------------------*/
void do_vt220_pa_keys()
{
 unsigned char arr[4];

TRACE(("pa key %c\n",buffer[buffer_index-2]));
  STATE=0;/* identified escape sequence */
  do_bidi_end_push();
  arr[0]=0x1b;  /* escape */
  arr[1]=0x4f;  /* O */
  arr[2]=buffer[buffer_index-1];
  array_to_pty(arr,3);
}

/*--------------------------------------------------------------------------*/
void do_vt220_soft_reset()
{
 TRACE(("soft reset terminal\n"));
 STATE=0;  /* recognized char, so reset state */
 SCR->_cur_atrib=ATRIB_NORMAL;
TRACE(("curatrib set to %x\n",SCR->_cur_atrib));
 SCR->_cur_grph=0;
 WRAP=FALSE;
 SCROLL=FALSE;
 INSERT=FALSE;
 init_v_tabs();
 init_h_tabs();
 array_to_hft("033[!p",4);
}
/*--------------------------------------------------------------------------*/


