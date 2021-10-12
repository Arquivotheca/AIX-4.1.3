static char sccsid[] = "@(#)83	1.1  src/bos/usr/bin/bterm/hft_func.c, libbidi, bos411, 9428A410j 8/26/93 13:35:00";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: do_hft_reset_function_keys
 *		init_aixterm
 *		init_hft
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
/*hft  specific functions */
/* It is very similar to vt220 functions, except for the function keys. */

#include "global.h"
#include "trace.h"

void set_vt220_atrib();
void set_vt220_grph();
void do_hft_reset_function_keys();
int yylex();

#define HFT_NORMAL      0x20000000  /* default is green on black */
#define AIX_NORMAL      0x0f000000  /* default is black on white */
/*------------------------------------------------------------------------*/

void init_hft()
/* to initialize hft functions and escapes */
{
TRACE(("initializing hft\n"));
       TERM=HFT;
       ATRIB_NORMAL=HFT_NORMAL; 
       EraseInput="\033K";
       NoScroll="\033[?7l";
       Scroll="\033[?7h";
       /* this is the escape that select machine modes for the IBM
          and DEC terminals emulation cartridge of the ibm3151 terminal.
          Machine mode selected is one of the reserved. */
       TERM_INIT="\033 9<i";

       set_atrib=set_vt220_atrib;
       set_grph=set_vt220_grph;
       do_reset_function_keys=do_hft_reset_function_keys;
       yylex_function=yylex;
}
/*------------------------------------------------------------------------*/

void init_aixterm()
/* to initialize aixterm functions and escapes */
{
TRACE(("initializing aixterm\n"));
       TERM=AIXTERM;
       ATRIB_NORMAL=AIX_NORMAL; 
       EraseInput="\033K";
       NoScroll="\033[?7l";
       Scroll="\033[?7h";
       /* this is the escape that select machine modes for the IBM
          and DEC terminals emulation cartridge of the ibm3151 terminal.
          Machine mode selected is one of the reserved. */
       TERM_INIT="\033 9<i";

       set_atrib=set_vt220_atrib;
       set_grph=set_vt220_grph;
       do_reset_function_keys=do_hft_reset_function_keys;
       yylex_function=yylex;
}
/*----------- reset all function keys to default AID value ----------*/
void do_hft_reset_function_keys()
{
 int i,j;

 STATE=0;  /* recognized char, so reset state */
 for (i=0;i<=9;i++)
   {
     function_keys[i][0]=0x1b;     /* escape */
     function_keys[i][1]=0x5b;     /* [ */
     function_keys[i][2]=0x30;     /* 0 */
     function_keys[i][3]=0x30;     /* 0 */
     function_keys[i][4]=0x30+i;   /* 0..9 */
     function_keys[i][5]=0x71;     /* q */
     function_keys[i][6]=0x00;
   }
 for (i=10,j=0;i<=19;i++,j++)
   {
     function_keys[i][0]=0x1b;     /* escape */
     function_keys[i][1]=0x5b;     /* [ */
     function_keys[i][2]=0x30;     /* 0 */
     function_keys[i][3]=0x31;     /* 1 */
     function_keys[i][4]=0x30+j;   /* 0..9 */
     function_keys[i][5]=0x71;     /* q */
     function_keys[i][6]=0x00;
   }
 for (i=20,j=0;i<=29;i++,j++)
   {
     function_keys[i][0]=0x1b;     /* escape */
     function_keys[i][1]=0x5b;     /* [ */
     function_keys[i][2]=0x30;     /* 0 */
     function_keys[i][3]=0x32;     /* 2 */
     function_keys[i][4]=0x30+j;   /* 0..9 */
     function_keys[i][5]=0x71;     /* q */
     function_keys[i][6]=0x00;
   }
 for (i=30,j=0;i<=36;i++,j++)
   {
     function_keys[i][0]=0x1b;     /* escape */
     function_keys[i][1]=0x5b;     /* [ */
     function_keys[i][2]=0x30;     /* 0 */
     function_keys[i][3]=0x33;     /* 3 */
     function_keys[i][4]=0x30+j;   /* 0..6 */
     function_keys[i][5]=0x71;     /* q */
     function_keys[i][6]=0x00;
   }

}
