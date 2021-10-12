/* @(#)11	1.3  src/bos/usr/bin/ate/lexer.h, cmdate, bos411, 9428A410j 4/18/91 10:56:52 */
/* 
 * COMPONENT_NAME: BOS lexer.h
 * 
 * FUNCTIONS: 
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



/* parse codes returned by yylex() */
#define END		  0     /* end of file */
#define NORM  	 	300	/* any not a vt100 sequence */
#define DISCARD		301     /* unrecognized/unsupported esc seq */

#define BELL		310     /* bell */
#define CPL		311     /* cursor preceding line (hft code) */
#define CPR    		312	/* cursor position report */
#define CUB    		313	/* cursor position backward */
#define CUD    		314	/* cursor position down */
#define CUF    		315	/* cursor position forward */
#define CUP    		316	/* set cursor position to line,column */
#define CUU    		317	/* cursor position up */
#define DEVATTR       	318	/* device attributes request */
#define DECALN 		319	/* screen alignment display */
#define DECANM   	320	/* sm/rm ANSI/VT52 */
#define DECARM   	321	/* sm/rm auto repeating */
#define DECAWM 		322	/* sm/rs auto wrap */
#define DECCKM   	323	/* cursor application mode OFF */
#define DECCOLM  	324	/* sm/rm column (80 or 132 column screen) */
#define DECDHL 		325	/* double height line */
#define DECDWL 		326	/* double-width line */
#define DECID    	327	/* identify terminal.  Obsolete.  Use DA. */
#define DECINLM  	328	/* sm/rm interlace */
#define DECKPAM  	329	/* enter keypad application mode */
#define DECKPNM  	330	/* exits keypad application mode */
#define DECLL  		331	/* load LEDS */
#define DECOM    	332	/* sm/rm origin mode */
#define DECPEX		333     /* sm/rm print extent */
#define DECPFF		334	/* sm/rm print form feed */
#define DECRC    	335	/* DEC restore cursor */
#define DECREPTPARM  	336	/* report terminal parameters */
#define DECREQPARM   	337	/* req terminal parameters */
#define DECSC    	338	/* DEC save cursor */
#define DECSCLM  	339	/* sm/rm scrolling (jump vs smooth scrolling) */
#define DECSCNM  	340	/* sm/rm screen (white on black/b on w) */
#define DECSTBM  	341	/* set top and bottom margins */
#define DECSWL 		342	/* single-width line */
#define DECTCEM		343	/* sm/rm text cursor enable */
#define DECTST 		344	/* invoke confidence tests */
#define DSR      	345	/* device status request */
#define ERASED 		346	/* erase in display */
#define ERASEL 		347	/* erase in line */
#define HT              348	/* move to next horizontal tab stop */
#define HTS    		349	/* set horizontal tab */
#define HVP    		350	/* move to line,column in relation to margin */
#define IND		351	/* index (move down 1 line, same column) */
#define IRM		352	/* sm/rm insertion-replacement */
#define KAM		353	/* sm/rm keyboard action */
#define LNM    		354	/* sm/rs line feed/new line mode */
#define NEL    		355	/* move to col 1 of next line */
#define RI       	356	/* reverse index */
#define RIS    		357	/* reset to initial state  */
#define RM              358     /* reset mode */
#define SCS      	359	/* select special character set */
#define SGR    		360	/* set graphic rendition */
#define SHIFT		361     /* shift in, shift out */
#define SIGNAL		362	/* caught a signal */
#define SM		363	/* set mode */
#define SRM		364	/* sm/rm send-receive */
#define TBC    		365	/* tabulation clear */

#define NPARAMS          20     /* maximum number of parameters */
int ps[NPARAMS];		/* parameters returned by lex */
int yylval;			/* parse code defined above */
int vt52;			/* vt52 (0=ansi, 1=vt52 mode) */

#define TOP 0
#define LFT 0
#define RGT (LFT + Cols - 1)
extern int Lines,Cols;
