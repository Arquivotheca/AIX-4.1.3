static char sccsid[] = "@(#)71	1.7  src/bos/usr/bin/capture/vtif.c, cmdsh, bos41B, 412_41B_sync 12/15/94 12:11:41";
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS: vtinit, vtclose, vtdraw, vtaddstr, vtattr, vtctl, vtaddch,
 *            vtesc, vmode, parmdft, vt1, vt2, vt6
 *
 * ORIGINS: 10, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <fcntl.h>
#include <errno.h>
#include <cur00.h>
#include "vtif.h"
#include "vtparse.h"

#include "capture_msg.h" 
static nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_CAPTURE,n,s) 

/* Local  constants */
#define	TOP	(stdscr->_begy)		/* top line of window */
#define	LFT	(stdscr->_begx)		/* left col of window */
#define	BMARG	(stdscr->_maxy - 1)	/* bottom line of window */
#define	RMARG	(stdscr->_maxx - 1)	/* right col of window */
#define BOT	(TOP + Lines - 1)	/* bottom actually used */
#define RGT	(LFT + Cols - 1)	/* right margin actually used */
#define MAXCOL	(256)			/* number of margin stops */
#define	XLATE0	0
#define XLATE1	1
#define XLATE2	2

static 	int 
	Lines = 0,		/* size of screen in lines */
	Cols = 0,		/* size of screen in columns */
	altchar[2], 		/* special graphics set for G0,G1 */
	attributes,		/* SGR, graphics attributes */
	keyappl,		/* keypad mode */
	vt52,			/* vt52 emulation flag */
	decom, 			/* DECOM,  origin mode */
	wrap, 			/* DECAWM, auto wrap */
	shift,			/* SCS, G0 or G1 is current char set */
	Newline,		/* line feed/new line mode */
	Init = FALSE,		/* Ecurses has been initialized */
	sr_top,			/* top of scrolling region */
	sr_end,			/* end of scrolling region */
	tabs[MAXCOL],		/* tab stops */
				/* save variables for DECSC,DECRC */
	save_y, save_x, save_attr, save_shift, save_alt0, save_alt1,
	save_wrap, save_decom,

	(*respfn)(),		/* terminal response function */
	(*termctlfn)(),		/* terminal ioctl function */

	eatnlglitch;		/* last known position after addch() */


/*----------------------------------------------------------------------*/
/* initialize vt100 screen image variables.				*/
/*----------------------------------------------------------------------*/
vtinit()
{ 	int i;

	/** Basic Init **/

	/* Create the screen */	
	if (!Init)
		initscr();		/* get curscr & stdscr */
	Init = TRUE;
	raw();				/* don't wait for NL */
	scrollok(stdscr,TRUE);		/* need to scroll */

	/* Determine screen size and set scroll regions */	
	Lines 		= (BMARG - TOP + 1);
	Cols 		= (RMARG - LFT + 1);
	sr_top		= TOP;		/* top of scrolling reg */
	sr_end          = BOT;      	/* end of scrolling reg */
	setscrreg(sr_top,sr_end);

	/* initialize emulation variables */
	altchar[0] 	= 0;		/* G0 char set is ascii */
	altchar[1] 	= 0;		/* G1 char set is ascii */
	shift 		= 0;		/* G0 is current char set */
	vt52 		= FALSE;	/* VT52 mode is off, ansi is on */
	decom 		= FALSE;	/* origin mode is off */
	wrap 		= FALSE;	/* auto wrap is off */
	Newline		= FALSE;	/* F=LF only, T=CR,LF */
	eatnlglitch	= -1;		/* disarm glitch */
	attributes 	= NORMAL;	/* normal attributes */
	vtattr(attributes);
	save_y		= ERROR;	/* no saved cursor status */
	for (i=8; i<MAXCOL; i+=8) tabs[i]=1;    /* set default tab stops */

	/** Application Init **/
	
	/* Only use a 24x80 field  */	
	if (Lines > 24) vtctl(VTC_SLINES,24);	/* vt100's have 24 lines */
	if (Cols > 80)  vtctl(VTC_SCOLS,80);	/* vt100's have 80 chars */

	/* setup keypad */
	vtesc(DECKPNM);
}

/*----------------------------------------------------------------------*/
/* close vt100 screen.							*/
/*----------------------------------------------------------------------*/
vtclose()
{ 	if (!Init)
		return(ERROR);
	endwin();		/* end window */
	noraw();
	resetterm();		/* reset terminal modes */
	Init = FALSE;
	return(0);
}

/*----------------------------------------------------------------------*/
/* refresh the screen							*/ 
/*----------------------------------------------------------------------*/
static vtdraw()
{ 	clearok(stdscr,TRUE);
	refresh();
}

/*----------------------------------------------------------------------*/
/* Reset the screen.							*/ 
/*----------------------------------------------------------------------*/
static vtclear()
{ 	move(TOP,LFT);
	touchwin(stdscr);
	refresh();
}

/*----------------------------------------------------------------------*/
/* Write a string to the screen.					*/ 
/*----------------------------------------------------------------------*/
static vtaddstr(s)
char *s;
{ 	
        while (*s != (NLSCHAR) NULL) vtaddch(*s++);
}

/*----------------------------------------------------------------------*/
/* Set the screen attibuites.						*/ 
/*----------------------------------------------------------------------*/
static vtattr(attr)
  int attr;
{	xstandout(stdscr,attr);	
}

/*----------------------------------------------------------------------*/
/* Provide an interface to internal functions and values		*/
/*----------------------------------------------------------------------*/
/* VARARGS */

static vtctl(cmd,arg)
  int cmd,		/* request code			*/
      arg;		/* argument for request		*/
{
	switch (cmd)
	{  
	   case VTC_SLINES:
		if (arg <= (BMARG-TOP+1) && arg >= 2) Lines = arg;
		if (sr_end > BOT)
		{	sr_end = BOT;
			setscrreg(sr_top,sr_end);		
		}	/* caller can shrink used-window-area */
		break;

	   case VTC_SCOLS:
		if (arg <= (RMARG - LFT + 1) && arg >= 1) Cols = arg;
		break;

	   case VTC_SRESPF:	respfn 	   = (int (*)()) arg;		break;
	   case VTC_RRESPF:	respfn 	   = 0;				break;
	   case VTC_STCTLF:	termctlfn  = (int (*)()) arg;		break;
	   case VTC_RTCTLF:	termctlfn  = 0;				break;
	   case VTC_SNL:	Newline	   = !!arg;			break;

	   case VTC_LINES:	return(Lines);
	   case VTC_COLS:	return(Cols);
	   case VTC_KCURSOR:	return((keyappl & VTK_CURSOR) == VTK_CURSOR);
	   case VTC_KAPPL:	return((keyappl & VTK_APPL)   == VTK_APPL);
	   case VTC_VT52:	return(vt52);
	   case VTC_NL:		return(Newline);
	   default:							break;
	}
	return(0);
}


/*----------------------------------------------------------------------*/
/* Write a char to the screen.						*/ 
/*----------------------------------------------------------------------*/
vtaddch(C)
  wchar_t C;		/* character to print */

{	int	i, y, x, new_y, new_x;

	getyx(stdscr,y,x);
	new_y=y; new_x=x;

	switch(C)
	{ 
	  /*-----------------------------------------------------------*/
	  /* Bell						       */
          /*-----------------------------------------------------------*/
	  case 007  :
              beep();
	      return(0);

	  /*-----------------------------------------------------------*/
	  /* backspace processing					*/
          /*-----------------------------------------------------------*/
	  case 010  :			/* back space */
              new_x = x - 1;
	      if (new_x < LFT) new_x=LFT;
              vmove(y,new_x);
	      goto paint;

	  /*-----------------------------------------------------------*/
	  /* Horizontal tab.					       */
          /*-----------------------------------------------------------*/
	  case 011  :			/* ^I -- tab */
	      /* find next tab position */
	      for (i = x+1; !tabs[i] && i<MAXCOL; i++)
	     	  ; 
              new_x = i;
	      if (new_x > RGT) new_x = RGT;
       	      vmove(y,new_x);
	      goto paint;

	  /*-----------------------------------------------------------*/
	  /* shift processing.					       */
          /*-----------------------------------------------------------*/
	  case 016  : 			/* Shift out (goto SG1)*/
              shift = 1;
	      return(0);

	  case 017  :			/* Shift in (goto SG0)*/
	      shift = 0;
	      return(0);
	
          /*-----------------------------------------------------------*/
	  /* carriage return processing.				*/
          /*-----------------------------------------------------------*/
	  case 015  : 			/* carriage return */
              if (Newline) 
              {  C = '\n';		/* change CR to LF */
		 new_x = LFT;
	   	 goto newln;		/* process as a newline */
	      }
       	      vmove(y,LFT);
	      goto paint;

          /*-----------------------------------------------------------*/
	  /* Newline (linefeed) processing				*/
          /*-----------------------------------------------------------*/
          case 012   :			/* new line */
	  case 013   :			/* vertical tab */
	  case 014   :			/* form feed */
	      if (Newline) 		/* CR,LF option selected */
                 new_x = LFT;
newln:        new_y = y+1;
	      if (y == sr_end)
	         {			/* Instead of addch()ing a newline  */
		 scroll(stdscr);	/* we have to scroll directly since */
		 new_y = sr_end;	/* addch() will clean out the line. */
	    	 eatnlglitch = -1;	/* disarm glitch since we scrolled */
	         }
	      else if (new_y > BOT) new_y = BOT;
	      vmove(new_y,new_x);
	      goto paint;

          /*-----------------------------------------------------------*/
	  /* Control codes ignored or not supported.		       */
          /*-----------------------------------------------------------*/
	  case 000    : 			/* NUL */
	  case 0177   : 			/* delete */
	      return(0);

          /*-----------------------------------------------------------*/
	  /* Normal characters and ignored control codes.	       */
          /*-----------------------------------------------------------*/
          default     :  	/* not backspace, newline or carriage ret */
              break;
	}

        /*-----------------------------------------------------------*/
	/* only normal characters will get this far.  See if they    */
	/* need to be filtered to graphics set and add to screen.    */
        /*-----------------------------------------------------------*/

        if (altchar[shift]) C = vt6(C);


        /*-----------------------------------------------------------*/
	/* curses auto wraps if the x position (column) exceeds RGT. */
	/* VT100's leave the cursor on top of the last char in the   */
	/* line and scroll *when* the next char comes in, **not**    */
	/* after the last char is written in the row.  Bother.       */
	/* This is known as the infamous  (according to Curses)      */
 	/* eat_newline[after wrap]_glitch. (Termcap boolean, XN).    */ 
        /*-----------------------------------------------------------*/
	getyx(stdscr,y,x);
        if (x <RGT) 
	{ 	eatnlglitch = -1;	/* disarm the glitch, not in RGT col*/
	     	addch(C);
	}
	else if (wrap && y == eatnlglitch) 	/* at RGT col! */
	{ 	eatnlglitch = -1;	/* glitch has struck, disable it */
	      	new_y = y+1;		/* advance to next line		*/
	      	if (y == sr_end)
	        { 	scroll(stdscr);
		 	new_y = y;
	        }
	      	else if (new_y > BOT) new_y = BOT;
	      	move(new_y, LFT);
	      	addch(C);
	}
	else				/* put char at RGT and backup cursor  */
	{ 	scrollok(stdscr,FALSE);	/* disable scrolling		*/
	      	addch(C);
	      	scrollok(stdscr,TRUE);
	      	move(y,RGT);		/* place cursor on top of char 	*/
	      	eatnlglitch = y;	/* glitch is now armed		*/
	}

paint:  refresh();
	return(0);
}


/*----------------------------------------------------------------------*/
/* vtesc uses ecurses to emulate the behavior of VT100 escape sequen-   */
/* ces.  The caller must parse the sequence himself and pass the type	*/
/* of escape sequence to emulate and any parameters as an array.	*/
/*									*/
/* In emulating a terminal, respfn and termctlfn can be set to 		*/
/* functions that, respectively, write to a communication channel and   */
/* perform the ioctl function on the communication line.		*/
/*									*/
/* (*respfn)() allows the emulator to perform terminal responses in	*/
/* which escape sequences are returned across the comm line.		*/
/* The function looks like a write() in which the fildes is assumed.	*/
/*									*/
/* (*termctlfn)() is a ioctl() function in which the fildes is assumed. */
/* This function is used to obtain the line parameters on the comm line */
/* which is then sent accross the line with (*respfn)().		*/
/*									*/	
/* If these functions are not present, their associated sequences are	*/
/* no-ops.								*/	
/*----------------------------------------------------------------------*/

/* VARARGS */

static vtesc(cmd,ps)
  int 	cmd,				/* Escape sequence command code */
 	ps[];				/* parameter array 		*/

{ int 	c,				/* character */
	i,j,				/* loop variables */
	y,x,new_y, new_x;		/* cursor position */

     switch(cmd)
     {
     /*-----------------------------------------------------------------*/
     /* screen movement codes are gathered here.			*/
     /*-----------------------------------------------------------------*/
     case CUB         : 	/* esc[PnD or escOD  -- cursor back (left) */
       	getyx(stdscr,y,x);
	parmdft(ps,1,1);
       	new_x = x - ps[0];
  	if (new_x < LFT) new_x = LFT;
       	vmove(y,new_x);
       	refresh();
  	break;

     case CUF         : 	/* esc[PnC or escOC -- cursor fwd (right) */
       	getyx(stdscr,y,x);
	parmdft(ps,1,1);
	new_y = y;
       	new_x = x + ps[0];
	if (new_x > RGT) new_x = RGT;
       	vmove(new_y,new_x);
       	refresh();
  	break;

     case CUD         : 	/* esc[PnB or escOB -- cursor down */
       	getyx(stdscr,y,x);
	parmdft(ps,1,1);
       	new_y = y + ps[0];
	if (new_y > BOT) 			new_y = BOT;
	if (y <= sr_end && new_y > sr_end)	new_y = sr_end;
       	vmove(new_y,x);
       	refresh();
  	break;

     case CUU         : 	/* esc[PnA or escOA -- cursor position up */
       	getyx(stdscr,y,x);
	parmdft(ps,1,1);
       	new_y = y - ps[0];
  	if (new_y < TOP) 			new_y = TOP;
       	if (y >= sr_top && new_y < sr_top) 	new_y = sr_top;
       	vmove(new_y,x);
       	refresh();
  	break;

     case CUP         : 	/* esc[Pn;PnH -- set cursor position */
     case HVP         : 	/* esc[Pn;Pnf -- set horiz/vert position */
	{  int deltay = (decom) ? sr_top - TOP   : 0;
	   int bottom = (decom) ? sr_end - deltay: BOT;

	   parmdft(ps,2,1,1);
  	   
	   new_x = (ps[1] - 1) + LFT;
	   if (new_x > RGT) new_x = RGT;
           if (new_x < LFT) new_x = LFT;
  
	   new_y = (ps[0] - 1) + TOP;
	   if (new_y < TOP)    new_y = TOP;
	   if (new_y > bottom) new_y = bottom;

       	   vmove(new_y + deltay, new_x);
       	   refresh();
  	   break;
	}

     case DECRC       : 	/* esc8 -- restore cursor */
	if (save_y != ERROR)	/* previous save was done */
	   {
	   vmove(save_y,save_x);
	   attributes 		= save_attr; 
	   vtattr(attributes);
	   shift 		= save_shift;
  	   altchar[0] 		= save_alt0;
	   altchar[1] 		= save_alt1;
	   wrap 		= save_wrap;
	   decom 		= save_decom;
	   }
	else			/* no previous save done */
	   {
	   vmove(TOP,LFT);
	   attributes 		= NORMAL; 
	   vtattr(attributes);
	   shift 		= 0;
  	   altchar[0] 		= 0;
	   altchar[1] 		= 0;
	   wrap 		= FALSE;
	   decom 		= FALSE;
	   }
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
	save_wrap  = wrap;
	save_decom = decom;
  	break;

     /*-----------------------------------------------------------------*/
     /* alphabetical from here on					*/
     /*-----------------------------------------------------------------*/
     case DISCARD     :		/* unrecognized or not supported */
	break;

     case CPR         : 	/* esc[Pn;PnR -- cursor position report */
	   			/* VT100 to host response only	        */
				/* We should never receive this		*/
  	break;

     case DECALN      : 	/* esc#8 -- screen alignment display */
     case DECDHL      : 	/* esc#3 or esc#4 -- double height line */
     case DECDWL      : 	/* esc#6 -- double width line */
	/* not supported -- ignore */
  	break;
  	
     case DEVATTR     :		/* esc[c or esc[0c -- device attributes */
     case DECID       : 	/* escZ -- identify terminal */
	if (respfn)
	   (!vt52) ? (*respfn)("\033[?1;0c",7)	/* "base VT100, no options */
	           : (*respfn)("\033/Z",3);	/* vt52 mode */
  	break;

     case DECKPNM     : 	/* esc= -- keypad application mode OFF */
       	keyappl &= ~VTK_APPL;	/* what about CURSOR? */
  	break;

     case DECKPAM     : 	/* esc> -- keypad application mode ON */
       	keyappl |= VTK_APPL;
  	break;

     case DECLL       : 	/* esc[Psq -- load LED's */
	/* not supported */
	break;

     case DECREPTPARM : 	/* esc[Ps;Psx -- report of terminal params */
	/* this is a report from VT100 to host only.  Should never receive */
	break;

     case DECREQPARM  : 	/* esc[Ps;Psx -- request terminal params */
	{
	char s[20]; int k,pp,bb,rr;
	struct termio to;

	if (!termctlfn) break;		/* may not be supported */
	if (!respfn)	break;		/* don't bother */
	
	(*termctlfn)(TCGETA,&to);

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

	k = sprintf(s,"\033[3;%d;%d;%d;%d;1;0x",pp,bb,rr,rr);
	(*respfn)(s,k);
	}
  	break;

     case DECSTBM     : 	/* esc[Pn;Pnr -- set scrolling reg */
  	parmdft(ps,2,1,1);	
	sr_top = ps[0] - 1 + TOP;
       	sr_end = ps[1] - 1 + TOP;
	if (sr_top < TOP) sr_top = TOP;
	if (sr_end > BOT) sr_end = BOT;
	if (sr_top >= sr_end)  	/* at least a 2-line region req'd */
	   {
	   if      (sr_top < BOT) sr_end=sr_top+1;
	   else if (sr_end > TOP) sr_top=sr_end-1;
	   else {sr_top=TOP; sr_end=BOT;}
	   }
       	setscrreg(sr_top,sr_end);
	vmove(decom ? sr_top : TOP, LFT);
       	refresh();
       	break;

     case DECSWL      : 	/* esc#5 - single width line */
	/* not supported -- ignore */
  	break;

     case DECTST      : 	/* esc[2;Psy -- invoke confidence tests */
	/* not supported -- ignore */
  	break;

     case DSR         : 	/* esc[Psn -- device status request */
	if (!respfn) break;	/* don't bother */
	parmdft(ps,1,0);
	switch(ps[0])
	{
	   case 0  : /* VT100-host response only; ignore */
	   case 3  : /* VT100-host response only; ignore */
	      	     break;
	   case 5  : (*respfn)("\033[0n",4); /* send 'no malfunctions' */
		     break;
	   case 6  : {	/* rpt active cursor pos */
		     char s[20]; int k;
		     getyx(stdscr,y,x);
		     if (decom) new_y = y - (sr_top - TOP);
		     else new_y = y;
		     k = sprintf(s,"\033[%d;%dR",new_y+1,x+1);
		     (*respfn)(s,k);
		     }
		     break;
           default : break;
        }
  	break;

     case ERASED      :		/* esc[PsJ - erase in display */ 
       	parmdft(ps,1,0);
	scrollok(stdscr,FALSE);			/* ecurses protection */
	getyx(stdscr,y,x);			/* save current pos */
	switch(ps[0])
       	{
  	   case 0  : clrtobot();
   		     clrtoeol();		/* clear the line */
		     eatnlglitch = -1;		/* screen cleared --disable xn*/
    	  	     break;
  	   case 1  : for (i=TOP; i<y; i++)	/* start from top */
       			{
  		    	move(i,LFT);		/* move to start of line */
   		    	clrtoeol();		/* clear the line */
  		    	}
  		     move(y,LFT);		/* move to curr line */
  		     for (i=LFT; i<=x; i++)
  		        delch();		/* erase each character */
  		     for (i=LFT; i<=x; i++)
			insch(' ');
		     if (x == RGT) eatnlglitch = -1;
  		     break;
  	   case 2  : for (i=TOP; i <= BOT;i++)	/* clean off screen */
			{ 
			move(i,LFT);
			clrtoeol();
			}
		     eatnlglitch = -1;		/* screen cleared --disable xn*/
  		     break;
           default : break;
        }
  	move(y,x);			/* move to original position */
  	refresh();
	scrollok(stdscr,TRUE);
  	break;

     case ERASEL      : 	/* esc[PsK -- erase in line */
       	parmdft(ps,1,0);
	scrollok(stdscr,FALSE);			/* ecurses protection */
	getyx(stdscr,y,x);
        switch(ps[0])
        {
  	  case 0  : clrtoeol();
		    eatnlglitch = -1;
  		    break;
  	  case 1  : move(y,LFT);		/* move to start of curr line */
  		    for (i=LFT; i<=x; i++)
  		       delch();			/* erase each character */
  		    move(y,LFT);		/* move to start of curr line */
  		    for (i=LFT; i<=x; i++)
		       insch(' ');
		    if (x == RGT) eatnlglitch = -1;
  		    break;
          case 2 :  move(y,LFT);		/* move to start of curr line */
  		    clrtoeol();			/* erase all of line */
		    eatnlglitch = -1;
  		    break;
          default : break;
        }
  	move(y,x);				/* move back to original pos */
  	refresh();
	scrollok(stdscr,TRUE);
  	break;

     case NEL         : 	/* escE -- next line (col 0) */
        getyx(stdscr,y,x);
	new_x = LFT;
	goto scrdn;
 
     case HTS         : 	/* escH */
	if (vt52) 		/* VT52 mode home sequence */
	   {
	   vmove(TOP,LFT);	/* move to home position */
	   refresh();
	   }
	else 			/* ansi set horizontal tab */
	   {
	   getyx(stdscr,y,x);
	   tabs[x] = 1;
	   }
  	break;

     case IND         : 	/* escD -- index (next line, same col) */
        getyx(stdscr,y,x);
	new_x = x;
scrdn:	new_y = y+1;
	{  int binding = (decom ? sr_end: BOT);
	   if (new_y > binding) new_y = binding;
	}
	if (y == sr_end) 
	   {
	   scroll(stdscr);
	   new_y=y;
	   eatnlglitch = -1;	/* disarm glitch since we scrolled */
	   }
       	vmove(new_y,new_x);
	refresh();
  	break;

     case RI          : 	/* escM or escI -- reverse index */
        getyx(stdscr,y,x);	/* get the current location */
  	new_y = y-1;		/* calculate the new line */
	{  int binding = (decom ? sr_top : TOP);
	   if (new_y < binding) new_y = binding;
	}
  	if (y == sr_top) 	/* tried to cross top margin */
	   {
  	   move(sr_end,0);	/* move to end of scrolling region */
           deleteln();          /* delete last line of scrolling region */ 
                                /* this moves last line of screen up also */
           move(sr_top,0);      /* move to top of scrolling region */
           insertln();          /* insert a blank line at top of region */
  	   new_y = sr_top;	/* move to new (blank) 1st line */
	   eatnlglitch = -1;	/* disarm glitch since we scrolled */
	   }
       	vmove(new_y,x);
	refresh();
  	break;

     case RIS         : 	/* escc -- reset to initial state */
	/* not supported -- ignore */
  	break;

     case SIGNAL         : 	/* processed a signal */
	sleep(1);
	vtdraw();
  	break;

     case SM	      :		/* esc[?Pnh or esc[20h -- set mode */
     case RM	      :		/* esc[?Pnl or esc[20l -- reset mode */
	for (j=0; ps[j]!=ERROR && j<NPARAMS; j++)
	{
	   switch(ps[j])
	   {
     	      case DECCKM    : 	/* esc[?1h or esc[?1l -- cursor appl mode */
     		   if (cmd == SM) keyappl |= VTK_CURSOR;
     		   else 	  keyappl &= ~VTK_CURSOR;
  	        break;

     	      case DECANM    : 	/* esc[?2h or esc[?2l -- ANSI/VT52 mode */
		vt52 = (cmd == SM) ? 0 : 1; 
		break;

     	      case DECOM     : 	/* esc[?6h or esc[?6l -- origin mode */
		new_x = LFT;
		if (cmd == SM)
		   {
		   decom=1;
		   new_y = sr_top;
		   }
	 	else
		   {
		   decom=0;
		   new_y = TOP;
		   }
		vmove(new_y,new_x);
		refresh();
		break;

     	      case DECAWM    : 	/* esc[?7h or esc[?7l -- auto wrap */
       		if (cmd == SM) wrap=1; else wrap=0;
  	      break;

     	      case LNM      : /* esc[20h  or esc[20l  -- LF/NL */
		if (cmd == SM) Newline = 1; 	/* 1st pos of next line */
		else           Newline = 0;	/* vertical down only */
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
  	      break;

	      default :  
	      break;
	   } 			/* end of switch */
        }			/* end of 'for' */
        break;

     case SCS         : 	/* esc(0 or esc(B -- select char set */
	/* altchar[0]=G0; altchar[1]=G1; 0=ansi, 1=graphics */
	parmdft(ps,2,0,0);
        altchar[(ps[0] == SG1) ? 1 : 0] = ((ps[1] == GRAPHICS) ? 1 : 0);
  	break;

     case SGR         : 	/* esc[Ps;...;Psm -- set graphic rendition */
	parmdft(ps,1,0);
	for (j=0; ps[j]!=ERROR && j<NPARAMS; j++)
	{
	   switch(ps[j])
	   {
	      case 0  : attributes &= NORMAL;		/* all off */
			break;
	      case 1  : attributes |= BOLD;		/* bold */
			break;
	      case 22 : attributes &= ~BOLD;		/* normal intensity */
			break;
	      case 4  : attributes |= UNDERSCORE;	/* underscore */
 			break;
 	      case 24 : attributes &= ~UNDERSCORE;	/* not underlined */
 		        break;
 	      case 5  : attributes |= BLINK;		/* blink */
			break;
	      case 25 : attributes &= ~BLINK;		/* not blinking */
			break;
	      case 27 : attributes &= ~REVERSE;		/* positive image */
			break;
	      case 7  : attributes |= REVERSE;		/* reverse video */
			break;
	      default : break;				/* not valid */
 	   }
	} 
	vtattr(attributes);
        refresh();
  	break;

     case TBC         : 	/* esc[Psg -- tabulation clear */
	parmdft(ps,1,0);
	if (ps[0]==0) 
	{
 	   getyx(stdscr,y,x);
	   tabs[x]=0;
	}
	if (ps[0]==3) for (i=0; i<MAXCOL; i++) tabs[i]=0;
  	break;

     case DIAGN	      :		/* esc[Z -- diagnostic dump */
#ifdef	NEVER	
	{
		char s[80],scum[1920];
		struct termio termio;

		scum[0] = 0;

		sprintf(s,"stdscr flags: %08X \t",stdscr->_flags);
		strcat(scum,s);
		sprintf(s,"stdscr csbp:  %08X\r\n",stdscr->_csbp);
		strcat(scum,s);
		sprintf(s,"curscr flags: %08X \t",curscr->_flags);
		strcat(scum,s);
		sprintf(s,"curscr csbp:  %08X\r\n",curscr->_csbp);
		strcat(scum,s);
	
		ioctl(0,TCGETA,&termio);
		sprintf(s,"if: %04X  of: %04X  cf: %04X  lm: %04X\r\n",
			termio.c_iflag,
			termio.c_oflag,
			termio.c_cflag,
			termio.c_lflag);
		strcat(scum,s);
		sprintf(s,"ld: %02X  cc: %02x %02x %02x %02x %02x %02x %02x %02x\r\n",
			termio.c_line,
			termio.c_cc[0],
			termio.c_cc[1],
			termio.c_cc[2],
			termio.c_cc[3],
			termio.c_cc[4],
			termio.c_cc[5],
			termio.c_cc[6],
			termio.c_cc[7]);
		strcat(scum,s);
	
		if (Init)
			vtaddstr(scum);
		else
			write(1,scum,strlen(scum));
	}
#endif NEVER
	break;

     default          : 
  	break;
     }					/* end of switch */

     return;
}

/*
 * vmove()
 *
 *	Disarm eat_newline_glitch if the cursor moves.
 */

static int vmove(y,x)
  int y,x;			/* new position */
{ 	int vy,vx; 
	getyx(stdscr,vy,vx);	/* current position */
	if ((y ^ vy)|(x ^ vx)) (eatnlglitch = -1, move(y,x));
	return(0);
}


/*
 * parmdft()
 *
 *	Set defaults for missing parameters to vtesc().
 */

/*VARARGS*/

static int parmdft(ps,n,d0,d1)
  int 	n,		/* number of parameters */
	d0,		/* default for ps[0]	*/
	d1,		/* default for ps[1]	*/
	ps[];		/* parameter array	*/

{ 	if (n<1 || n>2) return(0);

	if (ps[0] == ERROR) 
	{	ps[0] = d0;
		ps[1] = ERROR;
	}	
	if (--n && ps[1] == ERROR)
	{	ps[1] = d1;
		ps[2] = ERROR;
	}
	return(0);
}

/***************************************************************************
*                                                                          *
* NUMERIC KEYPAD     DEFAULT         REMAPPED        APPLICATION           *
* KEY / POSITION    CODE POINT      CODE POINT       CODE STRING           *
*                                                                          *
* ENTER   108           13              13             esc OM              *
*  -      105           45              45             esc Om    (dash)    *
*  .      104          196              46             esc On    (period)  *
*  /       95           47              47             esc Ol replaces "," *
*  0       99          179              48             esc Op              *
*  1       93          192              49             esc Oq              *
*  2       98          193              50             esc Or              *
*  3      103          217              51             esc Os              *
*  4       92          195              52             esc Ot              *
*  5       97          197              53             esc Ou              *
*  6      102          180              54             esc Ov              *
*  7       91          218              55             esc Ow              *
*  8       96          194              56             esc Ox              *
*  9      101          191              57             esc Oy              *
* PF-1    112        esc [001q        esc OP           esc OP              *
* PF-2    113        esc [002q        esc OQ           esc OQ              *
* PF-3    114        esc [003q        esc OR           esc OR              *
* PF-4    115        esc [004q        esc OS           esc OS              *
*                                                                          *
*        All of the above are on CODE PAGE 0x3c.  '<' in PCASCII           *
*                                                                          *
***************************************************************************/

static char map4[] = {
	0x20,		/* blank char			x'5f' */
	0xfe,		/* solid diamond, substitute vertical  
			   	solid rectangle	 	x'60' */
	0xb2,		/* full hash			x'61' */
	0x20,		/* HT				x'62' */
	0x20,		/* FF				x'63' */
	0x20,		/* CR				x'64' */
	0x20,		/* LF				x'65' */
	0xf8,		/* degree (overcircle)		x'66' */
	0xf1,		/* plus or minus		x'67' */
	0x20,		/* NL, substitute LF		x'68' */
	0x20,		/* VT				x'69' */
	0xd9,		/* lower right corner box	x'6a' */
	0xbf,		/* upper right corner box	x'6b' */
	0xda,		/* upper left corner box	x'6c' */
	0xc0,		/* lower left corner box	x'6d' */
	0xc5,		/* intersection			x'6e' */
	0x2d,		/* scan 1, substitute hyphen	x'6f' */
	0x2d,		/* scan 3, substitute hyphen	x'70' */
	0x2d,		/* scan 5, substitute hyphen	x'71' */
	0x2d,		/* scan 7, substitute hyphen	x'72' */
	0x2d,		/* scan 9, substitute hyphen	x'73' */
	0xc3,		/* left side middle		x'74' */
	0xb4,		/* right side middle		x'75' */
	0xc1,		/* bottom side middle		x'76' */
	0xc2,		/* top side middle		x'77' */
	0xb3,		/* vertical bar			x'78' */
	0xf3,		/* less than or equal to	x'79' */
	0xf2,		/* greater than or equal to	x'7a' */
	0xe3,		/* pi small			x'7b' */
	0x9d,		/* not equal sign, substitute yen sign	x'7c' */
	0x9c,		/* english pound sign		x'7d' */
	0xfa,		/* middle dot (product dot)	x'7e' */
	0xff,		/* all ones, non-printable	x'7f' */
};

/* -------------------------------------------------------------------------
   This program segment is called when an escape character is typed from
   the keyboard and when vt100 emulation is set to ON.  This
   will continue reading the keyboard until an end of sequence character
   is received (hex value greater than 0x40) and then it will write the
   equivalent VT100 sequence, modified or not, to the port (a DEC host?).
   ------------------------------------------------------------------------- */

vt1(kbdata, outfd)
  char kbdata;				/* char from keyboard */
  int outfd;				/* file descriptor to write to */
{
	static int i=0, eat_next=0;
	static char s[16],news[16], *p; /* string, vt100 string */
	static int xlate = XLATE0;
	static int sendesc = 1;


	if (eat_next) {
		eat_next = 0;
		return 0;
	}

	if (i == 0) {
		s[0] = '\0';			/* init string */
		news[0] = '\0';			/* init string */
		if (kbdata == '[') {		/* key = [ */
			s[i++] = kbdata;
			xlate = XLATE1;
			return 1;
		}
		else 	xlate = XLATE2;
	}

	s[i++] = kbdata;

	switch (xlate) {
	case XLATE1:
		if ((kbdata < 0x40) && (kbdata > 0x20) && (i < 15))
			return 1;
		
		/* sequence may be 15 characters after esc [
		 * 0x20 < kbdata < 0x40 implies kbdata is an integer 0 thru 9 or
		 * ! " # $ % & ' ( ) * + , - . / : ; < = > ?
		 * hex > 40 (a letter) is final character
		 */
		else 
		{ 	/* xlate string */
			s[i] = '\0'; 		    /* terminate the string */

			/*
			 * Stuff for RT and RISC System/6000 consoles
			 */
			if (strcmp(s,"[001q") == 0) {  /* F1 pressed */
				strcpy(news,"OP");
				break;
			}
			if (strcmp(s,"[002q") == 0) {  /* F2 pressed */
				strcpy(news,"OQ");
				break;
			}
			if (strcmp(s,"[003q") == 0) {  /* F3 pressed */
				strcpy(news,"OR");
				break;
			}
			if (strcmp(s,"[004q") == 0) {  /* F4 pressed */
				strcpy(news,"OS");
				break;
			}
			/*
			 * Old code starts here
			 */
			if (strcmp(s,"[0W") ==0) { /* HTS (horiz tab set) */
				strcpy(news,"H");
				break;
			}
			/* TBC (tabulation clear at current position */
			if (strcmp(s,"[2W") ==0) {
				strcpy(news,"[0g");
				break;
			}
			/* TBC (tabulation clear - all horizontal stops */
			if (strcmp(s,"[5W") ==0) {
				strcpy(news,"[3g");
				break;
			}
			if (strcmp(s,"[1E") ==0) {  /* NEL (next line) */
				strcpy(news,"E");
				break;
			}
			if (strcmp(s,"[u")  ==0) { /* DECRC (restore cursor) */
				strcpy(news,"8");
				break;
			}
			if (strcmp(s,"[s")  ==0) { /* DECSC (save cursor) */
				strcpy(news,"7");
				break;
			}
			if (strcmp(s,"[P")  ==0) { /* delete key */
				sendesc = 0;
				news[0] = 0x7f;
				news[1] = '\0';
				break;
			}

			/* iff cursor appl. ON, check cursor */
			if (vtctl(VTC_KCURSOR)) {
				if (strcmp(s,"[A")==0) { /* CUU (cursor up) */
					strcpy(news,"OA");
					break;
				}
				if (strcmp(s,"[B")==0) { /* CUD (cursor down) */
					strcpy(news,"OB");
					break;
				}
				if (strcmp(s,"[C")==0) { /* CUF (cursor forw */
					strcpy(news,"OC");
					break;
				}
				if (strcmp(s,"[D")==0) { /* CUB (cursor back) */
					strcpy(news,"OD");
					break;
				}
			}

			/* end of kbdata (0)  == '[' */

		}
		break;

	case XLATE2:
		if ((kbdata < 0x30) && (kbdata > 0x20) && (i<15))
			return 1;
		
		/* sequence may be 15 characters after esc
		 * 0x20 <= kbdata < 0x30 implies kbdata is a space or
		 * ! " # $ % & ' ( ) * + , - . / : ; < = > ?
		 * hex > 30 (a number, a letter, or : ; < = > ? @) is final character 
		 */
		else 
		{ 	/* xlate string */
			s[i] = '\0';                /* terminate the string */

			if (kbdata=='L' && i==1) {  /* RI (reverse index) */
				strcpy(news,"M");
				break;
			}
			/* enter KEYPAD APPLICATION MODE */
			if (kbdata=='=' && i==1) {
				vtesc(DECKPAM);
				break;
			}
			/* exit KEYPAD APPLICATION MODE */
			if (kbdata=='>' && i==1) {
				vtesc(DECKPNM);
				break;
			}
			/*
			 * Stuff for RT and RISC System/6000 consoles
			 * ibm316?
			 */
			if (kbdata=='A' && i==1) {  /* cursor up pressed */
				strcpy(news,"[A");
				break;
			}
			if (kbdata=='B' && i==1) {  /* cursor down pressed */
				strcpy(news,"[B");
				break;
			}
			if (kbdata=='C' && i==1) {  /* cursor forw pressed */
				strcpy(news,"[C");
				break;
			}
			if (kbdata=='D' && i==1) {  /* cursor back pressed */
				strcpy(news,"[D");
				break;
			}
			if (kbdata=='a' && i==1) {  /* F1 pressed */
				strcpy(news,"OP");
				eat_next = 1;
				break;
			}
			if (kbdata=='b' && i==1) {  /* F2 pressed */
				strcpy(news,"OQ");
				eat_next = 1;
				break;
			}
			if (kbdata=='c' && i==1) {  /* F3 pressed */
				strcpy(news,"OR");
				eat_next = 1;
				break;
			}
			if (kbdata=='d' && i==1) {  /* F4 pressed */
				strcpy(news,"OS");
				eat_next = 1;
				break;
			}
			/* end else sequence without [  */

		}
		break; 

		default: 
		printf(MSGSTR(VT100_XLATE, 
			"vt100(vt1): xlate invalid = %d\n"), xlate); 
		return 0;
	}


	if (sendesc == 1) {
		char c;

		c = ESC;
		write(outfd, &c, 1);		   /* send escape */
	}

	if (strcmp(news,"") != 0) {
		for (p = news; *p; p++)
			write(outfd, p, 1);
		(void) tcflush(outfd, TCOFLUSH);
	} else {
		if (strcmp (s,"") != 0) {    /* no substitution of characters */
			for (p = s; *p; p++)
				write(outfd, p, 1);
			(void) tcflush(outfd, TCOFLUSH);
		}
	}

	/* re-init static variables */
	i = 0;
	xlate = XLATE0;
	sendesc = 1;

	if (eat_next)
		return 1;
	else
		return 0;
}

vt2(C)
  int C;			/* character to process */
{
	vcmd_t	sv;		/* switch struct */
	int r,i;		/* loop variables */
	int cmd,		/* intermediate result for vtesc() */
	    args[NPARAMS];	/* parameter array to talk to vtesc() */

	r = vtparse(C,&sv);	/* parse vt100 esc sequence */
				/* if r == 0 found complete esc sequence */
				/* if r >  0 still forming esc sequence */
				/* if r <  0 found an error while parsing */

	if (r<0) /* error while parsing so spit out what we have so far */
	{	for ( i = 0; i < sv.v_len - 1; ++i)
			vtaddch(sv.v_data.string[i]);
		return(-1);
	}
	else if (r>0) return(1);	/* sequence not complete */


	/*
	 *	Dispatch esc sequence to vtesc()
 	 */

	switch(sv.v_type) 
	{ 
	   case V_CMD :		/* basic parameterized commands */
		for (i=0; i < sv.v_len; i++)	/* copy out of static */
	      	    args[i] = sv.v_data.args[i];
		args[sv.v_len] = ERROR;		/* terminate list */
		switch(sv.v_data.args[sv.v_len]) 
		{
		    case 'A':cmd = CUU;     break;/* cursor position up */
		    case 'B':cmd = CUD;     break;/* cursor down */
		    case 'C':cmd = CUF;     break;/* cursor fwd (right) */
		    case 'D':cmd = CUB;     break;/* cursor back (left) */
		    case 'H':cmd = CUP;     break;/* set cursor position */
		    case 'J':cmd = ERASED;  break;/* esc[PsJ - erase in display */ 
		    case 'K':cmd = ERASEL;  break;/* esc[PsK -- erase in line */
		    case 'R':cmd = CPR;     break;/* cursor position report */
		    case 'Z':cmd = DIAGN;   break;/*(Private)diagnostic dump */
		    case 'c':cmd = DEVATTR; break;/* device attributes */
		    case 'g':cmd = TBC;     break;/* tabulation clear */
		    case 'm':cmd = SGR;     break;/* set graphic rendition */
		    case 'n':cmd = DSR;     break;/* device status request */
		    case 'q':cmd = DECLL;   break;/* load LED's */
		    case 'r':cmd = DECSTBM; break;/* set scrolling reg */
		    case 'x':cmd = DECREQPARM; break;/* request of terminal params */
		    case 'y':cmd = DECTST;  break;/* invoke confidence tests */
		    default :cmd =      0;  break;

		    case 'h': /* set mode */
		    case 'l': /* reset mode */
				cmd = (sv.v_data.args[sv.v_len] == 'h') ? SM:RM;
				args[1] = ERROR;
				switch (sv.v_data.args[0])
				{ case  2:   args[0] = KAM;	break; 
				  case  4:   args[0] = IRM;	break;
				  case 12:   args[0] = SRM;	break;
				  case 20:   args[0] = LNM;	break;
			          default:   args[0] =   0;
				}
				if (!args[0]) cmd = 0;
				break;
		  }
		  if (cmd) vtesc(cmd,args);
		  break;

	   case V_SCMD1 : /* short commands "SCMD1" */
		  switch(sv.v_data.args[0])
		  {	/* esc<char> */
			case '7' :cmd = DECSC; break;/* save cursor */ 
			case '8' :cmd = DECRC; break;/* restore cursor */
			case 'H' :cmd = HTS;   break;/* Set Tab here */
			case 'D' :cmd = IND;   break;/* index */
			case 'E' :cmd = NEL;   break;/* next line (col 0) */
			case 'M' :
			case 'I' :cmd = RI;    break;/* reverse index */
			case 'Z' :cmd = DECID; break;/* what are you ? */;
			case 'c' :cmd = RIS;   break;/* reset to initial state */
			case '=' :cmd = DECKPAM; break;/* alt keypad */
			case '>' :cmd = DECKPNM; break;/* numeric keypad */
			default  :cmd =       0; break;

			/* handle some VT52 commands specially */		
			case '<' : 
				args[0] = DECANM;      /* Enter ANSI mode */
				args[1] = ERROR;
				vtesc(SM,args);
				cmd = 0;
				break;

			case 'F' :			/* Enter Graphics mode*/
				args[0] = SG0;
				args[1] = GRAPHICS;
				vtesc(SCS,args);
				cmd = 0;
				break;

			case 'G' :			/* Exit Graphics mode*/
				args[0] = SG0;
				args[1] = US;
				vtesc(SCS,args);
				cmd = 0;
				break;
		}
		if (cmd) vtesc(cmd);
		break;

	   case V_SCMD2 : /* die andere kleine Ordnungen "SCMD2" */ 
		switch(sv.v_data.args[0])  
		{	/* esc#Pn */
			case '3' : 
			case '4' : cmd = DECDHL; break; /* double height line */
			case '5' : cmd = DECSWL; break; /* single width line */
			case '6' : cmd = DECDWL; break; /* double width line */
			case '8' : cmd = DECALN; break; /* screen alignment display */
			default  : cmd = 0; break;
		}
		if (cmd) vtesc(cmd);
		break;

	   case V_CMDX : /* esc[?Pnh or esc[?Pnl */
		for (i=0; i<sv.v_len; i++)
			switch(sv.v_data.args[i])
			{
		  	   case 1: args[i] = DECCKM; 	break;
			   case 2: args[i] = DECANM; 	break;
		     	   case 3: args[i] = DECCOLM; 	break;
		     	   case 4: args[i] = DECSCLM; 	break;
		     	   case 5: args[i] = DECSCNM; 	break;
		  	   case 6: args[i] = DECOM; 	break;
		     	   case 7: args[i] = DECAWM; 	break;
		     	   case 8: args[i] = DECARM; 	break;
		     	   case 9: args[i] = DECINLM; 	break;
		     	   case 18:args[i] = DECPFF; 	break;
		     	   case 19:args[i] = DECPEX; 	break;
		     	   case 25:args[i] = DECTCEM; 	break;
			   default:args[i] = ERROR;	break;
			}
		args[sv.v_len] = ERROR;
		switch (sv.v_data.args[sv.v_len])
		{   case 'h':	cmd = SM; break;
		    case 'l':	cmd = RM; break;
		    default :	cmd =  0; break;
		}
		if (cmd) vtesc(cmd,args);
		break;

	   case V_SCMD3 : /* esc(0 esc(B -- select char set */
	   case V_SCMD4 : /* esc)0 esc)B -- select char set */
		switch (sv.v_data.args[0])
		{   case 'A':	args[1] = UK;	break;
		    case 'B':	args[1] = US;	break;
		    case '0':	args[1] = GRAPHICS; break;
		    default :	args[1] = 0; 	break;
		}
		if (!args[1]) break;
		args[0] = (sv.v_type  == V_SCMD3) ? SG0 : SG1;
		vtesc(SCS,args);
		break;

	   default : break;
	}
	return(0);
}

/**********************************************************************
 This routine takes as input a character from the special character
 and line drawing character set and translates it to the corresponding
 character in page 0.
 **********************************************************************/
static vt6(c)
int c;
{ 	return( (c < 0x5f || c > 0x7e) ? c : map4[c - 0x5f] );
}
