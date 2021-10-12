/* @(#)72	1.3  src/bos/usr/bin/capture/vtif.h, cmdsh, bos411, 9428A410j 2/11/94 17:05:41 */
/* COMPONENT_NAME: (CMDSH) Shell related commands 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 10  26  27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define DISCARD		301     /* unrecognized/unsupported esc seq */

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
#define SIGNAL		362	/* caught a signal */
#define SM		363	/* set mode */
#define SRM		364	/* sm/rm send-receive */
#define TBC    		365	/* tabulation clear */

#define DIAGN    	399	/* diagnostic dump */

#define	SG0		401	/* Select G0 character set */
#define	SG1		402 	/* Select G1 character set */
#define UK		403 	/* Select UK Ascii */
#define US		404 	/* Select US Ascii */
#define GRAPHICS	405	/* Select Graphics character set */


/* working constants */

#define NPARAMS         20     /* maximum number of parameters */
#define ERROR		-1
#define	ESC		0x1b


/* Define the package interface */

#define	VTC_SRESPF	1		/* set the response write function */
#define	VTC_STCTLF	2		/* set the comm line ioctl function */
#define	VTC_RRESPF	3		/* reset the response write function */
#define	VTC_RTCTLF	4		/* reset the comm line ioctl function */
#define VTC_SLINES	5		/* set the Lines variable */
#define VTC_SCOLS	6		/* set the Cols variable */
#define VTC_SNL		7		/* set the Newline variable */

#define VTC_LINES	15		/* get the Lines variable */
#define VTC_COLS	16		/* get the Cols variable */
#define VTC_NL		17		/* get the Newline variable */
#define	VTC_KAPPL       21 		/* get keypad status: application mode */
#define	VTC_KCURSOR     22		/* get keypad status: cursor mode */
#define	VTC_VT52	23		/* get VT52 mode status */


/* Keypad application modes */

#define VTK_CURSOR	0x02		/* in Cursor mode */
#define VTK_APPL	0x01		/* in application mode */

/* end */
