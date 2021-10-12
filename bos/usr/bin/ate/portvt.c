static char sccsid[] = "@(#)24	1.14  src/bos/usr/bin/ate/portvt.c, cmdate, bos41J, 9511A_all 3/7/95 16:48:22";
/* 
 * COMPONENT_NAME: BOS portvt.c
 * 
 * FUNCTIONS: vt2, vt6, vtclose, vtdraw, vtinit 
 *
 * ORIGINS: 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/****************************************************************************/
/*                                                                          */
/* Module name:      portvt.c                                               */
/* Description:      VT100 emulation routine.  				    */
/*		     This program is responsible for displaying incoming    */
/* 		     data when the user is in vt100 emulation mode. 	    */
/*		     It uses curses routines to maintain the screen image.  */
/*		     It calls yylex() to parse for control characters and   */
/* 		     vt100 escape characters.  The yylex source code is in  */
/* 		     lexer.   The Unix lex utility is used to create the    */
/*		     lex.yy.c module from lexer.			    */
/*                  							    */ 
/* Functions:        vt2 - convert vt100 sequences to sailboat sequences    */
/*		     vt6 - convert char to VT100 special graphics set       */
/*                   vtdraw - refresh the screen                            */
/*                   vtinit - initialize vt100 screen image variables       */
/*                   vtclose - close vt100 screen                           */
/*		     yylex - return a control char or vt100 data stream     */
/*			from the input.					    */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    curses routines provided by curses library		    */
/*   Called by:      conn in connect.c          			    */
/*                   sigrout in signal.c                                    */
/*                   main in main.c                                         */
/*                   modify in modify.c                                     */
/*   Receives:	     nothing (uses global data in modem.h)		    */
/*   Returns:	     nothing (uses global data in modem.h)		    */
/*   Abnormal exits: none                                                   */
/*   Calls:          yylex in lex.yy.c (get next token)			    */
/*                   vt3 in vt100.c (map number pad to shifted state,       */
/*                     i.e. numbers)                                        */
/*                   vt4 in vt100.c (map number pad to keypad appl mode,    */
/*                     i.e. functions)                                      */
/*                   vt7 in vt100.c (remap the numeric keypad's base state  */
/*                     to the keypad appl mode)                             */
/*                   setopt in setups.c (set terminal parameters for        */
/*                     connection)                                          */
/*                   message in message.c (display user messages)           */
/*                   cls in cls.c (clear screen)                            */
/*   Modifies:       lpath, keyappl, txti, bufr 			    */
/*                                                                          */
/****************************************************************************/


#include "modem.h"
#include "lexer.h"
#include <stdio.h>
#define HZ HZ1
#include <cur00.h> 
#include <cur01.h> 
#include <stdlib.h>

#define MAXCOL 256
#define ERROR (-1)
extern char yytext[], *yysptr, yysbuf[];
extern int yyleng, yytchar;
extern int size;		/* size of screen buffer */
/* char *sbuf;		 	   screen buffer memory pointer - modem.h */

extern char swrp;		/* screen wrap indicator */
extern char *scur;		/* screen cursor position */
extern char *sbufe;		/* end of screen buffer */
static int 
    altchar[2], 			/* special graphics set for G0,G1 */
    attributes,				/* SGR, graphics attributes */
    decom, 				/* DECOM,  origin mode */
    wrap, 				/* DECAWM, auto wrap */
    shift,				/* SCS, G0 or G1 is current char set */
    Newline,				/* line feed/new line mode */
    sr_top,				/* top of scrolling region */
    sr_end,				/* end of scrolling region */
    linsz,				/* length of lines in kapture file */
    tabs[MAXCOL],			/* tab stops */
    save_y, save_x, save_attr,		/* save variables for DECSC,DECRC */
    save_shift, save_alt0, save_alt1,
    save_wrap, save_decom,
    eatnlglitch = -1;

char escbuf[64];                        /* hold partial escape sequences */
char *eb_ptr = escbuf;			/* escape buffer pointer */
int esccnt = 0;				/* escape PUSH counter */
/* -------------------------------------------------------------------------
   vt6 - filter incoming vt100 special graphics characters (octal codes
   0137 to 0176 = hex codes 5F to 7F) to their equivalent RT-PC codes.
   This routine takes as input a character from the special character
   and line drawing character set and translates it to the corresponding
   character in hft page 0.
   ------------------------------------------------------------------------- */
char map4[] = {
	0x20,		/* blank char				      x'5f' */
	0xfe,		/* solid diamond   (vertical solid rectangle) x'60' */
	0xb2,		/* full hash			              x'61' */
	0x20,		/* horizontal tab  (blank)                    x'62' */
	0x20,		/* form feed       (blank) 		      x'63' */
	0x20,		/* carriage return (blank)	              x'64' */
	0x20,		/* line feed       (blank)                    x'65' */
	0xf8,		/* degree          (overcircle)		      x'66' */
	0xf1,		/* plus/minus			              x'67' */
	0x20,		/* new line        (blank)       	      x'68' */
	0x20,		/* vertical tab	   (blank)		      x'69' */
	0xd9,		/* lower right corner box	              x'6a' */
	0xbf,		/* upper right corner box	              x'6b' */
	0xda,		/* upper left corner box	              x'6c' */
	0xc0,		/* lower left corner box	              x'6d' */
	0xc5,		/* crossing Lines		              x'6e' */
	0x2d,		/* scan 1          (hyphen)          	      x'6f' */
	0x2d,		/* scan 3          (hyphen)          	      x'70' */
	0x2d,		/* scan 5          (hyphen)                   x'71' */
	0x2d,		/* scan 7          (hyphen)          	      x'72' */
	0x2d,		/* scan 9          (hyphen)                   x'73' */
	0xc3,		/* left 'T'              	              x'74' */
	0xb4,		/* right 'T'       		              x'75' */
	0xc1,		/* bottom 'T'        		              x'76' */
	0xc2,		/* top 'T'        		              x'77' */
	0xb3,		/* vertical bar			              x'78' */
	0xf3,		/* less than or equal to	              x'79' */
	0xf2,		/* greater than or equal to	              x'7a' */
	0xe3,		/* pi small			              x'7b' */
	0x9d,		/* not equal sign  (yen sign)                 x'7c' */
	0x9c,		/* english pound sign		              x'7d' */
	0xfa,		/* middle dot      (product dot)	      x'7e' */
	0xff,		/* all ones, non-printable	              x'7f' */
	};

wchar_t
chg(a_char)	/* convert a multibyte or byte char into a widechar */
char a_char;
{
	wchar_t wc;
	mbtowc(&wc, &a_char, mb_cur_max);
	return(wc);
}

char vt6(c)
char c;                 /* current character */
{
/*
#ifdef DEBUG
kk = sprintf(ss,"*vt6 in=%c(%x), out=%c(%x)\n",c,c,map4[c-0x5f],map4[c-0x5f]);
write(fe,ss,kk);
#endif DEBUG
*/

return(map4[c - 0x5f]);
}


/*----------------------------------------------------------------------*/
/* initialize vt100 screen image variables.				*/
/*----------------------------------------------------------------------*/
vtinit()
{
int i;

sr_top		= TOP;			/* top of scrolling reg */
sr_end          = Lines + TOP - 1;      /* end of scrolling reg */
altchar[0] 	= 0;			/* G0 char set is ascii */
altchar[1] 	= 0;			/* G1 char set is ascii */
vt52 		= 0;			/* VT52 mode is off, ansi is on */
attributes 	= 0;			/* normal attributes */
decom 		= 0;			/* origin mode is off */
/* wrap 	= awrap;		/* auto wrap is off */
shift 		= 0;			/* G0 is current char set */
Newline		= 0;			/* 0=LF only, 1=CR,LF */
for (i=8; i<MAXCOL; i+=8) tabs[i]=1;    /* set default tab stops */
save_y		= ERROR;
save_x		= ERROR;

initscr();				/* get curscr & stdscr */
raw();					/* don't wait for NL */
setscrreg(sr_top,sr_end);
scrollok(stdscr,1);			/* enable scrolling */
vmove(TOP,LFT);				/* move to corner */
#ifdef DEBUG
kk = sprintf(ss,"leaving vtinit() with stdscr= %d , curscr = %d\n",stdscr,curscr);
write(fe,ss,kk);
#endif DEBUG

}


/*----------------------------------------------------------------------*/
/* close vt100 screen.							*/
/*----------------------------------------------------------------------*/
vtclose()
{

#ifdef DEBUG
kk = sprintf(ss,"closing vt100 screen\n");
write(fe,ss,kk);
#endif DEBUG

endwin();				/* end window */
resetterm();				/* reset terminal modes */
scur=sbuf;                              /* set cursor ptr screen top */
swrp=OFF;                               /* screen wrap is OFF */
sbufe=sbuf+size-1;                      /* set ptr to screen buffer end */
cls();					/* clear the screen */
noraw();

}


/*----------------------------------------------------------------------*/
/* refresh the screen							*/
/*----------------------------------------------------------------------*/
vtdraw()
{
int rc;

#ifdef DEBUG
kk = sprintf(ss,"entering vtdraw\n");
write(fe,ss,kk);
#endif DEBUG

clearok(stdscr,TRUE);		/* everything has changed! */
#ifdef DEBUG
kk = sprintf(ss,"after touchwin with stdscr=%d\n",curscr);
write(fe,ss,kk);
#endif DEBUG
refresh();		/* write curscr to glass */

#ifdef DEBUG
kk = sprintf(ss,"leaving vtdraw\n");
write(fe,ss,kk);
#endif DEBUG
}


/*----------------------------------------------------------------------*/
/* This code is used the entire time the user in is vt100 mode.		*/
/* It calls yylex(), which lives in lex.yy.c.  yylex is a parser that   */
/* returns the id of the escape sequence (see lexer.h for a list).	*/
/* Here we keep a screen image based on what yylex() returns.		*/
/*									*/
/* The top of the screen is TOP and the left margin is LFT.  This can   */
/* be either 0 or 1.							*/
/*----------------------------------------------------------------------*/
vt2()
{
int c;					/* character */
int i,j;				/* loop variables */
int rc;					/* return code */
int y,x,new_y, new_x;			/* cursor position */
static char k=0;
#ifdef DEBUG
kk = sprintf(ss,"ENTERING vt2.  ptr=%o, lnstat=%d, lndata=", 
  ptr,lnstat);
write(fe,ss,kk);
write(fe,lndata,lnstat);
write(fe,"\n",1); 
#endif DEBUG

while (ptr < lndata+lnstat)		/* characters in buffer */
  {
  for (j=0; j<NPARAMS; j++) ps[j]=ERROR; 	/* -1 means not in use */
  i=j=yyleng=0;
  yylval=NORM;

  /*--------------------------------------------------------------------*/
  /* This is optimization.  Don't call the parser unless a control      */
  /* code is  encountered.  Otherwise, just process.			*/
  /*--------------------------------------------------------------------*/
  if (yysptr>yysbuf || eb_ptr>escbuf)   /* parser 'ungot' some chars */
     {
     yylval = yylex();			/* call it to process them */
     }
  else 					/* some chars in buffer */
     {
     for (i=0; ptr < lndata+lnstat  &&  /* ptr still in range */
	  *ptr>0x1F && *ptr<0x7F;i++) 	/* not cntl char or delete */
        {   
        yytext[i] = *ptr;		/* character from buffer */
        yyleng += 1;			/* length of string */
        ptr++;				/* advance pointer */
        }
     if (yyleng==0) 
	{
	yylval=yylex();
        }
     else 
        {
        yylval = NORM;			
	yytext[i]='\0';
        }
     }

  /*--------------------------------------------------------------------*/
  /* Some escape sequences arrive one character at a time.  If a        */
  /* lonely escape arrives, push it onto escbuf stack.	If we try to    */
  /* push esc onto stack more than 16 times, probably we're in a loop.  */
  /* Fourteen selected as longest usual vt100 data stream.		*/
  /*--------------------------------------------------------------------*/
#ifdef DEBUG
kk = sprintf(ss,"\nPROCESSING:  yytext[0]=%o, yylval=%d, yyleng=%d, yytext=", yytext[0],yylval,yyleng);
write(fe,ss,kk);
write(fe,yytext,yyleng);
write(fe,"\n",1);
#endif DEBUG
  if ((yytext[0]==033) && (yylval==NORM) && (yyleng==1) && (esccnt < 16))
     {
#ifdef DEBUG
kk = sprintf(ss,"\n     PUSHING escape onto stack\n");
write(fe,ss,kk);
#endif DEBUG
     esccnt += 1;
     *eb_ptr++=yytext[0];		/* put escape on esc seq. stack */
     yylval=DISCARD;
     }	

  /*--------------------------------------------------------------------*/
  /* process the string							*/
  /*--------------------------------------------------------------------*/
  if (yylval != DISCARD) esccnt = 0;

  switch(yylval)
     {
     /*-----------------------------------------------------------------*/
     /* normal characters (not escape sequences)			*/
     /*-----------------------------------------------------------------*/
     case NORM     :			/* normal character */
#ifdef DEBUG
kk = sprintf(ss,"     processing NORM = %s(%d)\n",yytext,yyleng);
write(fe,ss,kk);
#endif DEBUG
	getyx(stdscr,y,x);
	new_y=y; new_x=x;

        /*--------------------------------------------------------------*/
	/* control characters & DEL are returned one at a time.  Only   */
	/* normals (0x20 through 0x7E--not cntl char or delete) are     */
 	/* returned with multiple characters.				*/
        /*--------------------------------------------------------------*/
	switch(yytext[0])
           {
           /*-----------------------------------------------------------*/
	   /* backspace processing					*/
           /*-----------------------------------------------------------*/
	   case '\b' :			/* back space */
              new_x = x-1;
	      if (new_x < LFT) new_x=LFT;
              vmove(y,new_x);
	      goto paint;

           /*-----------------------------------------------------------*/
	   /* carriage return processing.				*/
           /*-----------------------------------------------------------*/
	   case 015  : 			/* carriage return */
              if (xlatcr|Newline) 
                 {
	   	 yytext[0] = '\n';	/* change CR to LF */
		 new_x = LFT;
	   	 goto newln;		/* process as a newline */
	         }
#ifdef DEBUG
kk = sprintf(ss,"in carriage return (CR-LF) y= %d\n", y);
write(fe,ss,kk);
#endif DEBUG
       	      vmove(y,LFT);
	      goto paint;
	
           /*-----------------------------------------------------------*/
	   /* Newline (linefeed) processing				*/
           /*-----------------------------------------------------------*/
	   case 013   :			/* vertical tab */
	   case 014   :			/* form feed */
           case 012   :			/* new line */
	      if (Newline) 		/* CR,LF option selected */
                 {
                 new_x=LFT;
#ifdef DEBUG
kk = sprintf(ss,"in newline mode (CR-LF) y = %d\n", y);
write(fe,ss,kk);
#endif DEBUG
                 }
newln:        new_y=y+1;
	      if (y==sr_end)
	         {
#ifdef DEBUG
kk = sprintf(ss,"NEED to scroll\n new y=%d,x=%d,sr_top=%d,sr_end=%d,rgt=%d\n", new_y,x,sr_top,sr_end,RGT);
write(fe,ss,kk);
#endif DEBUG
		scroll(stdscr);
		new_y=sr_end;
		eatnlglitch = -1 ;
	         }
		else if(new_y > sr_end) new_y=sr_end;
	      vmove(new_y,new_x);
	      goto paint;

	   case 0177   : 		/* delete */
	      goto paint;		/* ignored when received */

           default     :  	/* not backspace, newline or carriage ret */
           break;
           }				/* end switch */

        /*-----------------------------------------------------------*/
	/* only normal characters will get this far.  See if they    */
	/* need to be filtered to graphics set and add to screen.    */
        /*-----------------------------------------------------------*/
        if (altchar[shift])         		/* vt100 graphics char set */
	   {
	   for (i=0; i<yyleng; i++)
              {
              if ((yytext[i] >= 0x5f) && 
                  (yytext[i] <= 0x7f))  	/* in graphics set? */
              yytext[i] = vt6(yytext[i]);	/* filter it */
              }
	   }					/* end of 'for' loop */

        /*-----------------------------------------------------------*/
	/* curses auto wraps if x position (column) exceeds RGT.  In */
	/* INed, this causes the border to display incorrectly. This */
	/* code checks the x position and adjusts it to avoid the    */
	/* auto wrap.  Sigh.					     */
        /*-----------------------------------------------------------*/
	for(i=0; i<yyleng; i++)
	  {
		getyx(stdscr,y,x);
		if(x < RGT)
		   {
			eatnlglitch = -1 ;
			addch(yytext[i]);
		    }
		else if(awrap && y == eatnlglitch)
		    {
			eatnlglitch = -1 ;
			new_y = y+1 ;
			if(y == sr_end)
			  {
				scroll(stdscr);
				new_y = y ;
			   }
			else if(new_y > sr_end) new_y = sr_end ;
			move(new_y,LFT);
			addch(yytext[i]);
		    }
		else
		    {
			scrollok(stdscr,0);
			addch(yytext[i]);
			scrollok(stdscr,1);
			move(y,RGT);
			eatnlglitch = y ;
		     }
		 }
paint:
	  refresh();
	  break;


     /*-----------------------------------------------------------------*/
     /* screen movement codes are gathered here.			*/
     /*-----------------------------------------------------------------*/
     case CUB         : 	/* esc[PnD or escOD  -- cursor back (left) */
       	getyx(stdscr,y,x);
       	new_x = x - ps[0];
  	if (new_x < LFT) new_x = LFT;
       	vmove(y,new_x);
       	refresh();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  cub:  y=%d, x=%d, new_y=%d, new_x=%d\n", y,x,y,new_x);
write(fe,ss,kk);
#endif DEBUG
  	break;

     case CUD         : 	/* esc[PnB or escOB -- cursor down */
       	getyx(stdscr,y,x);
       	new_y = y + ps[0];
	if (new_y > Lines) new_y = Lines;
       	vmove(new_y,x);
       	refresh();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  cud:  *y=%d, x=%d, new_y=%d, new_x=%d\n", y,x,new_y,x);
write(fe,ss,kk);
#endif DEBUG
  	break;

     case CUF         : 	/* esc[PnC or escOC -- cursor fwd (right) */
       	getyx(stdscr,y,x);
	new_y = y;
       	new_x = x + ps[0];
	if (new_x > RGT) 
	   {
	   if (awrap) 
              {
	      new_x=LFT;
	      new_y = y+1;
	      if (new_y > Lines) new_y = Lines;
	      }
           else new_x = RGT;
	   }
       	vmove(new_y,new_x);
       	refresh();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  cuf:  *y=%d, x=%d, new_y=%d, new_x=%d\n", y,x,new_y,new_x);
write(fe,ss,kk);
#endif DEBUG
  	break;

     case CUU         : 	/* esc[PnA or escOA -- cursor position up */
       	getyx(stdscr,y,x);
       	new_y = y - ps[0];
  	if (new_y < TOP) new_y = TOP;
       	move(new_y,x);
       	refresh();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  cuu:  y=%d, x=%d, new_y=%d, new_x=%d\n", y,x,new_y,x);
write(fe,ss,kk);
#endif DEBUG
  	break;

     case CUP         : 	/* esc[Pn;PnH -- set cursor position */
     case HVP         : 	/* esc[Pn;Pnf -- set horiz/vert position */
  	new_x = ps[1] + LFT - 1;
	if (new_x > RGT) new_x = RGT;
        if (new_x < LFT) new_x = LFT;
	if (decom) 
	   {
	   new_y = ps[0] + sr_top + TOP - 1;
	   if (new_y > sr_end) new_y = sr_end;
	   if (new_y < sr_top) new_y = sr_top;
	   }
	else
	   {
  	   new_y = ps[0] + TOP - 1;
	   if (new_y > Lines) new_y = Lines;
	   if (new_y < TOP) new_y = TOP;
	   }
       	move(new_y,new_x);
       	refresh();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  cup/hvp:  new_y=%d, new_x=%d\n", new_y,new_x);
write(fe,ss,kk);
#endif DEBUG
  	break;

     case DECRC       : 	/* esc8 -- restore cursor */
	if (save_y != ERROR)	/* previous save was done */
	   {
	   move(save_y,save_x);
	   attributes = save_attr; colorout(attributes); 
	   shift = save_shift;
  	   altchar[0] = save_alt0;
	   altchar[1] = save_alt1;
	   awrap = save_wrap;
	   decom = save_decom;
	   }
	else			/* no previous save done */
	   {
	   move(TOP,LFT);
	   attributes = NORMAL; colorout(attributes);
	   shift = 0;
  	   altchar[0] = 0;
	   altchar[1] = 0;
	   awrap = 0;
	   decom = 0;
	   }
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  decrc:    x    y  attr shft alt0 alt1 wrap dcom\n");
write(fe,ss,kk);
kk = sprintf(ss,"                 ---- ---- ---- ---- ---- ---- ---- ----\n");
write(fe,ss,kk);
kk = sprintf(ss,"                %5d%5d%5d%5d%5d%5d%5d%5d\n",
   save_y,save_x,attributes,shift,altchar[0],altchar[1],awrap,decom);
write(fe,ss,kk);
#endif DEBUG
	refresh();
  	break;

     case DECSC       : 	/* esc7 -- save cursor */
       	getyx(stdscr,y,x);
        save_y     = y; 
	save_x     = x;
	save_attr  = attributes;
	save_shift = shift;
	save_alt0  = altchar[0];
	save_alt1  = altchar[1];
	save_wrap  = awrap;
	save_decom = decom;
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  decsc:    x    y  attr shft alt0 alt1 wrap dcom\n");
write(fe,ss,kk);
kk = sprintf(ss,"                 ---- ---- ---- ---- ---- ---- ---- ----\n");
write(fe,ss,kk);
kk = sprintf(ss,"                %5d%5d%5d%5d%5d%5d%5d%5d\n",
   save_y,save_x,save_attr,save_shift,save_alt0,save_alt1,save_wrap,save_decom);
write(fe,ss,kk);
#endif DEBUG
  	break;

     /*-----------------------------------------------------------------*/
     /* alphabetical from here on					*/
     /*-----------------------------------------------------------------*/
     case BELL        :	        /* bell */
	beep();
	break;

     case DISCARD     :		/* unrecognized or not supported */
	break;

     case END	      :		/* out of characters */
	break;

     case CPR         : 	/* esc[Pn;PnR -- cursor position report */
	   			/* VT100 to host response only	        */
				/* We should never receive this		*/
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  %s not supported\n",yytext);
write(fe,ss,kk);
#endif DEBUG
  	break;

     case DECALN      : 	/* esc#8 -- screen alignment display */
     case DECDHL      : 	/* esc#3 or esc#4 -- double height line */
     case DECDWL      : 	/* esc#6 -- double width line */
	/* not supported -- ignore */
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  %s not supported\n",yytext);
write(fe,ss,kk);
#endif DEBUG
  	break;
  	
     case DEVATTR     :		/* esc[c or esc[0c -- device attributes */
     case DECID       : 	/* escZ -- identify terminal */
	if (!vt52)
	   {
     	   write(lpath,"\033[?1;0c",7);	/* send "base VT100, no options */
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  da/decid writing %s to %d\n", "\033[?1;0c",lpath);
write(fe,ss,kk);
#endif DEBUG
	   }
	else			/* vt52 mode */
	   {
	   write(lpath,"\033/Z",3);
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  decid writing %s to %d\n", "\033\\Z.",lpath);
write(fe,ss,kk);
#endif DEBUG
	   }
  	break;

     case DECKPNM     : 	/* esc= -- keypad application mode OFF */
       	keyappl = (keyappl & 0xFFFE);	/* zero the flag bit (0) */
#ifdef HFT
        vt3();                  	/* remap keypad to shifted state */
#endif /* HFT */
	setopt();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  deckpnm setting keyappl to %d\n",keyappl);
write(fe,ss,kk);
#endif DEBUG
  	break;

     case DECKPAM     : 	/* esc> -- keypad application mode ON */
#ifdef HFT
	if (vt52) vt7();	/* remap for vt52  keypad appl */
	else vt4(); 		/* remap for vt100 keypad appl */
#endif /* HFT */
       	keyappl = (keyappl | 0x0001);   /* set flag bit (0) to 1 */

#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  deckpam setting keyappl to %d\n",keyappl);
write(fe,ss,kk);
#endif DEBUG
  	break;

     case DECLL       : 	/* esc[Psq -- load LED's */
	/* not supported -- ignore */
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  %s not supported\n",yytext);
write(fe,ss,kk);
#endif DEBUG
  	break;

     case DECREPTPARM : 	/* esc[Ps;Psx -- report of terminal params */
	/* this is a report from VT100 to host only.  Should never receive */
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  %s not supported\n",yytext);
write(fe,ss,kk);
#endif DEBUG
	break;

     case DECREQPARM  : 	/* esc[Ps;Psx -- request terminal params */
	{
	char s[20]; int kk,pp,bb,rr;
	struct termios to;

	tcgetattr(lpath,&to);   /* get port parameters */

	if (to.c_cflag & ~PARENB) pp=1;
	else if (to.c_cflag & (PARENB+PARODD)) pp=2;
	else pp=4;
	
	if (to.c_cflag & CS8) bb=1; else bb=2;

	switch(to.c_cflag&CBAUD) {
	   case    B50 : rr=  0; break;
	   case    B75 : rr=  8; break;
	   case   B110 : rr= 16; break;
	   case   B134 : rr= 24; break;
	   case   B150 : rr= 32; break;
	   case   B300 : rr= 48; break;
	   case   B600 : rr= 56; break;
	   case  B1200 : rr= 64; break;
	   case  B1800 : rr= 72; break;
	   case  B2400 : rr= 80; break;
	   case  B4800 : rr=104; break;
	   case  B9600 : rr=112; break;
	   case   EXTA : rr=120; break;
	   }

	kk = sprintf(s,"\033[3;%d;%d;%d;%d;1;0x",pp,bb,rr,rr);
	write(lpath,s,kk);
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  decreqparm writing %s to %d\n",s,lpath);
write(fe,ss,kk);
#endif DEBUG
	}
  	break;

     case DECSTBM     : 	/* esc[Pn;Pnr -- set scrolling reg */
  	sr_top = ps[0] + TOP - 1;
       	sr_end = ps[1] + TOP - 1;
	if (sr_top < TOP) sr_top = TOP;
	if (sr_end > Lines) sr_end = Lines;
	if (sr_top >= sr_end)  	/* at least a 2-line region req'd */
	   {
	   if      (sr_top <= Lines-1) sr_end=sr_top+1;
	   else if (sr_end > TOP) sr_top=sr_end-1;
	   else {sr_top=TOP; sr_end=Lines+TOP-1;}
	   }
       	setscrreg(sr_top,sr_end);
	new_x = LFT;
	if (decom) new_y = sr_top;
	else new_y = TOP;
	move(new_y,new_x);
       	refresh();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  decstbm:  sr_top=%d, sr_end=%d\n", sr_top,sr_end);
write(fe,ss,kk);
#endif DEBUG
       	break;

     case DECSWL      : 	/* esc#5 - single width line */
	/* not supported -- ignore */
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  %s not supported\n",yytext);
write(fe,ss,kk);
#endif DEBUG
  	break;

     case DECTST      : 	/* esc[2;Psy -- invoke confidence tests */
	/* not supported -- ignore */
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  %s not supported\n",yytext);
write(fe,ss,kk);
#endif DEBUG
  	break;

     case DSR         : 	/* esc[Psn -- device status request */
	switch(ps[0])
	   {
	   case 0  : /* VT100-host response only; ignore */
	   case 3  : /* VT100-host response only; ignore */
	      	     break;
	   case 5  : write(lpath,"\033[0n",4); /* send 'no malfunctions' */
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  dsr writing %s to %d\n", "\033[0n",lpath);
write(fe,ss,kk);
#endif DEBUG
		     break;
	   case 6  : {	/* rpt active cursor pos */
		     char s[20]; int k;
       		     getyx(stdscr,y,x);
		     if (decom) new_y = y - sr_top + 1;
		     else new_y = y;
		     k = sprintf(s,"\033[%d;%dR",new_y,x);
		     write(lpath,s,k);
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  dsr writing %s to %d\n", s,lpath);
write(fe,ss,kk);
#endif DEBUG
		     }
		     break;
           default : break;
           }
  	break;

     case ERASED      :		/* esc[PsJ - erase in display */ 
	scrollok(stdscr,FALSE);
       	switch(ps[0])
       	   {
  	   case 0  : getyx(stdscr,y,x);		/* erase to bottom */
       	   	     clrtobot();
   		     clrtoeol();		/* clear the line */
		     move(y,x);
    		     refresh();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  ed clear to bottom\n");
write(fe,ss,kk);
#endif DEBUG
    	  	     break;
  	   case 1  : getyx(stdscr,y,x);		/* current position */
       		     for (i=TOP; i<y; i++)	/* start from top */
       			{
  		    	move(i,LFT);		/* move to start of line */
   		    	clrtoeol();		/* clear the line */
  		    	}
  		     move(y,LFT);		/* move to curr line */
  		     for (i=LFT; i<=x; i++)
  		        delch();		/* erase each character */
  		     for (i=LFT; i<=x; i++)
			insch(' ');
  		     move(y,x);			/* move to original position */
  		     refresh();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  ed clear to top\n");
write(fe,ss,kk);
#endif DEBUG
  		     break;
  	   case 2  : getyx(stdscr,y,x);		/* current position */
       	             clear();			/* clear all of display */
		     move(y,x);
  		     refresh();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  ed clear all\n");
write(fe,ss,kk);
#endif DEBUG
  		     break;
           default : break;
           }
	scrollok(stdscr,TRUE);
  	break;

     case ERASEL      : 	/* esc[PsK -- erase in line */
	scrollok(stdscr,FALSE);
        switch(ps[0])
          {
  	  case 0  : getyx(stdscr,y,x);    	/* clear to end of line */
		    clrtoeol();
		    eatnlglitch = -1;
		    move(y,x);
  	   	    refresh();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  el clear to EOL\n");
write(fe,ss,kk);
#endif DEBUG
  		    break;
  	  case 1  : getyx(stdscr,y,x);    	/* clear to start of line */
  		    move(y,LFT);		/* move to start of curr line */
  		    for (i=LFT; i<=x; i++)
  		       delch();			/* erase each character */
  		    move(y,LFT);		/* move to start of curr line */
  		    for (i=LFT; i<=x; i++)
		       insch(' ');
		    if(x == RGT) eatnlglitch = -1 ;
  		    move(y,x);			/* move to original position */
  		    refresh();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  el clear to SOL\n");
write(fe,ss,kk);
#endif DEBUG
  		    break;
          case 2 :  getyx(stdscr,y,x); 		/* clear all of line */
  		    move(y,LFT);		/* move to start of curr line */
  		    clrtoeol();			/* erase all of line */
		    eatnlglitch = -1;
  		    move(y,x);			/* move back to original pos */
  		    refresh();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  el clear all of line\n");
write(fe,ss,kk);
#endif DEBUG
  		    break;
          default : break;
          }
	scrollok(stdscr,TRUE);
  	break;

     case HT	      :         /* ^i -- horizontal tab */
        getyx(stdscr,y,x);
	new_y = y;
	for (i=x+1; tabs[i]!=1 && i<MAXCOL; i++)
	   /* find next tab position */;
        new_x = i;
	if (new_x > RGT) new_x=RGT;
       	move(new_y,new_x);
       	refresh();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  ht moving from (%d,%d) to (%d,%d)\n",y,x,new_y,new_x);
write(fe,ss,kk);
#endif DEBUG
	break;

     case HTS         : 	/* escH */
	if (vt52) 		/* VT52 mode home sequence */
	   {
	   move(TOP,LFT);	/* move to home position */
	   refresh();
	   }
	else 			/* ansi set horizontal tab */
	   {
	   getyx(stdscr,y,x);
	   tabs[x] = 1;
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  hts setting a tab at column %d\n",x);
write(fe,ss,kk);
#endif DEBUG
	   }
  	break;

     case IND         : 	/* escD -- index (next line, same col) */
        getyx(stdscr,y,x);
	new_y = y+1;
	if (new_y > sr_end) 
	   {
	   scroll(stdscr);
	   new_y=y;
	   }
       	move(new_y,x);
	refresh();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  ind:  y=%d, x=%d, new_y=%d, new_x=%d\n", y,x,new_y,new_x);
write(fe,ss,kk);
#endif DEBUG
  	break;

     case NEL         : 	/* escE -- next line (col 0) */
        getyx(stdscr,y,x);
	new_x = LFT;
	new_y = y+1;
	if (new_y > sr_end) 
	   {
	   scroll(stdscr);
	   new_y=y;
	   }
       	move(new_y,new_x);
	refresh();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  nel: y=%d, x=%d, new_y=%d, new_x=%d\n", y,x,new_y,new_x);
write(fe,ss,kk);
#endif DEBUG
  	break;

     case RI          : 	/* escM or escI -- reverse index */

#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  In case RI ");
write(fe,ss,kk);
#endif DEBUG

        getyx(stdscr,y,x);	/* get the current location */
  	new_y = y-ps[0];	/* calculate the new line */
  	if (new_y < sr_top) 	/* new line is less than top */
	   {
  	   move(sr_end,0);	/* move to end of scrolling region */
           deleteln();          /* delete last line of scrolling region */ 
                                /* this moves last line of screen up also */
           move(sr_top,0);      /* move to top of scrolling region */
           insertln();          /* insert a blank line at top of region */
  	   new_y = sr_top;	/* move to new (blank) 1st line */
	   }
       	move(new_y,x);
	refresh();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  ri:  y=%d, x=%d, new_y=%d, new_x=%d, ps[0]=%d\n",
    y,x,new_y,x,ps[0]);
write(fe,ss,kk);
#endif DEBUG
  	break;

     case RIS         : 	/* escc -- reset to initial state */
	/* not supported -- ignore */
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  %s not supported\n",yytext);
write(fe,ss,kk);
#endif DEBUG
  	break;

     case SIGNAL         : 	/* processed a signal */
	sleep(1);
	clearok(stdscr,TRUE);
	refresh();
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  signal processed\n");
write(fe,ss,kk);
#endif DEBUG
  	break;

     case SM	      :		/* esc[?Pnh or esc[20h -- set mode */
     case RM	      :		/* esc[?Pnl or esc[20l -- reset mode */
	for (j=0; ps[j]!=ERROR && j<NPARAMS; j++)
	   {
	   switch(ps[j])
	      {
     	      case DECCKM    : 	/* esc[?1h or esc[?1l -- cursor appl mode */
     		   if (yylval == SM) keyappl = (keyappl | 0x0002);  /* ON */
     		   if (yylval == RM) keyappl = (keyappl & 0xFFFD);  /* OFF */
#ifdef DEBUG
kk = sprintf(ss,"DECCKM setting keyappl to 0%x\n",keyappl);
write(fe,ss,kk);
#endif DEBUG
  	        break;

     	      case DECANM    : 	/* esc[?2h or esc[?2l -- ANSI/VT52 mode */
		if (yylval == SM) 
		   {
		   vt52=0; 
     		   keyappl = (keyappl & 0xFFFB);  /* OFF */
		   }
     		if (yylval == RM) 
		   {
		   vt52=1; 
		   keyappl = (keyappl | 0x0004);  /* ON */
		   }
#ifdef DEBUG
kk = sprintf(ss,"DECANM setting keyappl to 0%x, vt52 to %d\n",keyappl,vt52);
write(fe,ss,kk);
#endif DEBUG
		break;

     	      case DECOM     : 	/* esc[?6h or esc[?6l -- origin mode */
		new_x = LFT;
		if (yylval == SM)
		   {
		   decom=1;
		   new_y = sr_top;
		   }
	 	else
		   {
		   decom=0;
		   new_y = TOP;
		   }
		move(new_y,new_x);
		refresh();
#ifdef DEBUG
kk = sprintf(ss,"DECOM setting decom to %d\n",decom);
write(fe,ss,kk);
#endif DEBUG
		break;

     	      case DECAWM    : 	/* esc[?7h or esc[?7l -- auto wrap */
       		if (yylval == SM) awrap=1; else awrap=0;
#ifdef DEBUG
kk = sprintf(ss,"DECAWM setting wrap to %d\n",awrap);
write(fe,ss,kk);
#endif DEBUG
  	      break;

     	      case LNM      : /* esc[20h  or esc[20l  -- LF/NL */
		if (yylval == SM) Newline=1; 	/* 1st pos of next line */
		else Newline = 0;		/* vertical down only */
#ifdef DEBUG
kk = sprintf(ss,"LNM setting Newline to %d\n",Newline);
write(fe,ss,kk);
#endif DEBUG
	      break;

     	      case DECARM   : /* esc[?8h  or esc[?8l  -- auto repeating */
              case DECCOLM  : /* esc[?3h  or esc[?3l  -- 80/132 column */
     	      case DECINLM  : /* esc[?9h  or esc[?9l  -- interlace */
	      case DECPEX   : /* esc[?19h or esc[?19l -- print extent */
              case DECPFF   : /* esc[?18h or esc[?18l -- print form feed */
     	      case DECSCLM  : /* esc[?4h  or esc[?4l  -- jump/smooth scroll */
     	      case DECSCNM  : /* esc[?5h  or esc[?5l  -- screen color */
              case DECTCEM  : /* esc[?25h or esc[?25l -- text cursor enable */
	      case IRM      : /* esc[4h   or esc[4l   -- insertion-repl */
	      case KAM      : /* esc[2h   or esc[2l   -- keyboard action */
              case SRM      : /* esc[12h  or esc[12l  -- send-receive */
		/* not supported -- ignore */
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  %s not supported\n",yytext);
write(fe,ss,kk);
#endif DEBUG
  	      break;

	      default :  
	      break;
	      } 		/* end of switch */
           }			/* end of 'for' */
        break;

     case SCS         : 	/* esc(0 or esc(B -- select char set */
	/* altchar[0]=G0; altchar[1]=G1; 0=ansi, 1=graphics */
        if (strcmp(yytext,"\033(0")==0) altchar[0] = 1;
        if (strcmp(yytext,"\033(B")==0) altchar[0] = 0;
        if (strcmp(yytext,"\033F" )==0) altchar[0] = 1;
        if (strcmp(yytext,"\033)0")==0) altchar[1] = 1;
        if (strcmp(yytext,"\033)B")==0) altchar[1] = 0;
        if (strcmp(yytext,"\033G" )==0) altchar[0] = 0;
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  scs char sets are %d %d\n",altchar[0],altchar[1]);
write(fe,ss,kk);
#endif DEBUG
  	break;

     case SGR         : 	/* esc[Ps;...;Psm -- set graphic rendition */
	for (j=0; ps[j]!=ERROR && j<NPARAMS; j++)
	   {
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  sgr ps[%d]=%d\n",j,ps[j]);
write(fe,ss,kk);
#endif DEBUG
	   switch(ps[j])
	      {
	      case 0  : attributes &= NORMAL;		/* all off */
			break;
	      case 1  : attributes |= BOLD;		/* bold */
			break;
	      case 22 : attributes &= ~BOLD;		/* normal intensity */
			break;
	      case 24 : attributes &= ~UNDERSCORE;	/* not underlined */
		        break;
	      case 25 : attributes &= ~BLINK;		/* not blinking */
			break;
	      case 27 : attributes &= ~REVERSE;		/* positive image */
			break;
	      case 4  : attributes |= UNDERSCORE;	/* underscore */
			break;
	      case 5  : attributes |= BLINK;		/* blink */
			break;
	      case 7  : attributes |= REVERSE;		/* reverse video */
			break;
	      default : break;				/* not valid */
 	      } /* end switch */
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  sgr attributes=0x%x\n",attributes);
write(fe,ss,kk);
#endif DEBUG
	   } /* end for */
        colorout(attributes); 
        refresh();
  	break;

     case SHIFT	      :         /* ^N, ^O -- shift out, shift in */
	shift = ps[0];
#ifdef DEBUG
kk = sprintf(ss,"PORTVT:  current char set is G%d\n",shift);
write(fe,ss,kk);
#endif DEBUG
	break;

     case TBC         : 	/* esc[Psg -- tabulation clear */
	if (ps[0]==0) 
	   {
 	   getyx(stdscr,y,x);
	   tabs[x]=0;
	   }
	if (ps[0]==3) for (i=0; i<MAXCOL; i++) tabs[i]=0;
#ifdef DEBUG
if (ps[0]==0) kk = sprintf(ss,"PORTVT:  tbc (%s) clearing tab at col %d\n",
   yytext,x);
else kk = sprintf(ss,"PORTVT:  tbc (%s) clearing ALL tabs\n",yytext);
write(fe,ss,kk);
#endif DEBUG
  	break;

     default          : 
#ifdef DEBUG
kk = sprintf(ss,"Undefined yylval=%d\n",yylval);
write(fe,ss,kk);
#endif DEBUG
  	break;
     }					/* end of switch */

  /*--------------------------------------------------------------------*/
  /* put EVERYTHING in the capture file.				*/
  /*--------------------------------------------------------------------*/
  if (capture)				/* user capture flag is ON */
    {
/*
#ifdef DEBUG
kk = sprintf("Putting vt100 stuff in the capture file\n");
write(fe,ss,kk);
#endif DEBUG
*/
	if (yylval != NORM){
            if ((linsz + yyleng) > 256){
                if (txti >= BUFSZ){
                    if (write(fk,bufr,txti) == ERROR)
                      message(26);
                    txti = 0;
                }
                bufr[txti++] = '\n';
                linsz = 0;
            }
        }
	if(yylval == NORM){
        for (i=0; i<yyleng; i++){
          if (linsz > 255){
              if (txti >= BUFSZ){
                 if (write(fk,bufr,txti) == ERROR)
                    message(26);
                 txti = 0;
	      }		
	      bufr[txti++] = '\n';
	      linsz = 0;
	  }
	  if (txti >= BUFSZ){
	      if (write(fk,bufr,txti) == ERROR)
		message(26);
	      txti = 0;
	  }
	  if(yytext[i] =='\n' && k == '\r') k='\n';
	  else{
	      k= yytext[i];
	      if(yytext[i] =='\r'){
		bufr[txti++]='\n';linsz=0;
	      }
	      else if(k==010){
		txti--;linsz--;
		if((int)txti<0)txti=0;
	      }
	      else if(k==025){
		for(;(int)txti>0 && bufr[txti] !='\n';txti--,linsz--);
	      }
	      else if(!(iswprint((wint_t) chg(yytext[i])) || iswspace((wint_t) chg(yytext[i]))));
	      else{
		bufr[txti++]=k; linsz++;}
	      }
	      if(linsz <0)linsz=0;
	      if (yytext[i] == '\n') linsz = 0;
	   }
	  }
	 }	/* end of capture  */
	}		/* end of while */
return;
}			/* end of vt2() */

int vmove(y,x)
  int x,y ;
{
	int vy,vx ;
	getyx(stdscr,vy,vx);
	if((y ^ vy) | (x ^ vx)) (eatnlglitch = -1, move(y,x));
	return(0);
}

#ifdef notdef
vtwrap()
{
    wrap = awrap;
}
#endif

