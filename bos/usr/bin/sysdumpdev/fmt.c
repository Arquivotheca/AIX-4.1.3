static char sccsid[] = "@(#)81	1.7  src/bos/usr/bin/sysdumpdev/fmt.c, cmddump, bos411, 9428A410j 7/16/91 11:42:56";
/*
 * COMPONENT_NAME: CMDDUMP    system dump control and formatting
 *
 * FUNCTIONS: jdumpinit, jdump, jdumpterm, jdump_paged
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * jdumpinit, jdump, jdumpterm
 *
 * Format the data from a dump file from calls from xtr.c
 * These routines take raw buffers/length as input and make no
 *   assumption as to where the data comes from.
 * Note that jdumpinit() does take a 'vaddr' parameter to set the starting
 *   offset. It is not used for anything except printing.
 *
 * These routines use a 'dmp_diplayopts' structure to describe the display
 *   options. This structure is filled in by opts.c but is only used by the
 *   jdump routines. These display attributes are programmable:
 *     fieldsize   = number of bytes in a field (1 - 16)     dflt=4
 *     nfields     = number of fields in a line              dflt=4
 *     align       = alignment of the start of a line        dflt=16
 *     wrapwidth   = width of screen.                        dflt=0 (don't care)
 *     asciiflg    = display ascii equivalent on right       dflt=0
 *     compactflg  = don't display duplicate line (like od)  dflt=0
 *

Example:

fieldsize = 4 (bytes)
Bincol    = 3
WBincol   = 3
Fieldc    = 2

XXXXXXXX XXXXXXXX XXXXXXXX XXXX31XX XXXXXXXX XXXXXXXX    .............1.........
|<---->|                   ^   ^                      ^  ^            ^
^                          |   |                      |  |            |
|                          |   |                      |  |            |
+----------------+         |   |                      |  |            |
                 |         |   |                      |  |            |
fieldsize = 4  --+         |   |                      |  |            |
WBincol   = 3  ------------+   |                      |  |            |
Bincol    = 3                  |                      |  |            |
Fieldc    = 2  ----------------+                      |  |            |
Wrapcol   = 6  ---------------------------------------+  |            |
Charoff0  = 58 ------------------------------------------+            |
Charcol   = 14                                           |<---------->|

 */

#include <stdio.h>
#include "dmpfmt.h"

extern struct dmp_displayopts Opt;	/* contains screen display options */

static Fieldc;				/* in-field byte index */
static Vaddr0;				/* address field at beginning of line */
static Bincol;				/* field index */
static WBincol;				/* screen-oriented field index */
static Charcol;				/* ascii expansion character index */
static Charoff0;			/* start in Line[] of ascii expansion */
static Wrapcol;				/* max screen-oriented field index (WBincol) */

#define BC_SEP     2		/* separation between binary and ascii expansion */
#define ADDR_WIDTH 10		/* width of the address field */
#define LINEWIDTH 2048		/* max width of screen */

static char Line[LINEWIDTH+1];		/* line gets printed into this buffer */
static char Prevline[LINEWIDTH+1];	/* previous line gets copied to here */
int Prevlen;						/* length of previous line */
int skipflg;						/* previous line was compacted out */

static spaceflg;					/* override with spaces (' ') */

static binprint();
static charprint();
static lineflush();
static lineflush_clr();
/*
 * NAME:     jdumpinit
 * FUNCTION: Set up the formatting control variables and indent output
 *           for proper starting alignment for the data area
 * INPUTS:   vaddr      Starting logical virtual address for this data area
 * RETURNS:  none
 */
jdumpinit(vaddr,inmemflg)
{
	int n;
	int i;

	if(Opt.opt_align == 0 || Opt.opt_align > 16)
		Opt.opt_align = 16;
	if(Opt.opt_fieldsize == 0)
		Opt.opt_fieldsize = 4;
	if(Opt.opt_fieldsize > 16)
		Opt.opt_fieldsize = 16;
	if(Opt.opt_nfields == 0)
		Opt.opt_nfields = 4;
	if(Opt.opt_wrapwidth == 0) {
		Wrapcol = Opt.opt_nfields;
	} else {
		n = ADDR_WIDTH;
		if(Opt.opt_asciiflg)
			n += (Opt.opt_nfields * Opt.opt_fieldsize) + BC_SEP;
		Wrapcol = (Opt.opt_wrapwidth - n) / (2 * Opt.opt_fieldsize+1);
		if(Wrapcol <= 0)
			Wrapcol = 1;
		if(Wrapcol > Opt.opt_nfields)
			Wrapcol = Opt.opt_nfields;
	}
	lineflush_clr();
	WBincol  = 0;
	Bincol   = 0;
	Charoff0 = (2 * Opt.opt_fieldsize + 1) * Wrapcol + BC_SEP;
	Charcol  = 0;
	Fieldc   = 0;
	spaceflg = 0;
	n = vaddr % Opt.opt_align;
	Vaddr0 = vaddr - n;
	Debug("Wrapcol=%d Charoff0=%d Vaddr0=%x vaddr=%x\n",
		Wrapcol,Charoff0,Vaddr0,vaddr);
	if(n > 0) {
		if(inmemflg) {
			spaceflg = 1;
			jdump(0,n);
			spaceflg = 0;
		} else {
			Vaddr0 += n;
		}
	}
}

/*
 * NAME:     jdumpterm
 * FUNCTION: flush the Line buffer as filled in by jdump()
 * INPUTS:   none
 * RETURNS:  none
 */
jdumpterm()
{
	int i;
	int n;

	n = 0;
	if(Fieldc > 0)
		n += Opt.opt_fieldsize - Fieldc;
	if(WBincol > 0)
		n += Opt.opt_fieldsize * (Wrapcol - WBincol);
	if(n > 0) {
		spaceflg = 1;
		jdump(0,n);
		spaceflg = 0;
	}
	lineflush();
	if(skipflg) {
		printf("*\n");
		printf("%08x:\n",Vaddr0);
	}
}

/*
 * NAME:     jdump
 * FUNCTION: Handle the binary dump buffer output line-ny-line.
 * INPUTS:   buf    pointer to character buffer
 *           count  size of 'buf' in bytes
 * RETURNS:  none
 *
 * The jdump() routine does not know about data areas or the format of the
 * dump file. That part is handled by disp_da(). jdump works with a character
 * buffer and byte count passed as function parameters.
 * Note: The lineflush() routine does maintain a variable 'Vaddr0' which
 *    is the virtual address of the start of the line it is printing. Its
 *    starting value is calculated by jdumpinit(). The only thing it is
 *    used for is to keep track of the virtual address of start of the line,
 *    as is incremented by the number of bytes output on the previous line.
 *    So in this sense it is not related to virtual addresses within the
 *    dump file.
 */
jdump(buf,count)
unsigned char *buf;
{
	int i;
	int x;

	Debug("jdump(%x,%x)\n",buf,count);
	for(i = 0; i < count; i++) {
		x = buf ? buf[i] : 0;
		binprint(x);
		if(Opt.opt_asciiflg)
			charprint(x);
		if(Bincol >= Opt.opt_nfields) {
			Bincol = 0;
			lineflush();
		} else if(WBincol >= Wrapcol) {
			lineflush();
		}
	}
}

/*
 * NAME:     jdump_paged
 * FUNCTION: Called when page in not in memory.
 * INPUTS:   count  size of 'buf' in bytes. Used to update Vaddr0.
 * RETURNS:  none
 */
jdump_paged(count)
{

	Debug("jdump_paged(%x) Vaddr=%x\n",count,Vaddr0);
	printf("%08x: not in memory\n",Vaddr0);
	lineflush_clr();
	Vaddr0 += count;
}

/*
 * hex to ascii conversion macros
 */
#define N(x)       ( (x) & 0x0F )
#define ITOC(x)    ( N(x) >= 0x0A ? N(x) - 0x0A + 'A' : N(x) + '0' )
#define ISPRINT(c) (' ' < (c) && (c) < 0x7F)

/*
 * NAME:     binprint
 * FUNCTION: print the binary byte value 'x' into the
 *           internal ascii line buffer 'Line'
 * INPUTS:   x      binary byte value to print
 * RETURNS:  none
 *
 * If the global flag 'spaceflg' is set, print spaces instead. This
 * lets jdumpinit() position the starting indentation easily.
 */
static binprint(x)
{
	char *cp;

	cp = Line + WBincol * (2*Opt.opt_fieldsize+1) + Fieldc * 2;
	if(spaceflg) {
		cp[0] = ' ';
		cp[1] = ' ';
	} else {
		cp[0] = ITOC(x / 16);
		cp[1] = ITOC(x % 16);
	}
	if(++Fieldc >= Opt.opt_fieldsize) {
		Bincol++;
		WBincol++;
		Fieldc = 0;
	}
}

/*
 * NAME:     charprint
 * FUNCTION: print the ascii byte value 'x' into the
 *           internal ascii line buffer 'Line'
 * INPUTS:   x      binary byte value to print
 * RETURNS:  none
 *
 * If the global flag 'spaceflg' is set, print spaces instead. This
 *  lets jdumpinit() position the starting indentation easily.
 * If the value 'x' is not straight ascii, a '.' will be placed in
 *  the Line buffer instead.
 */
static charprint(x)
{
	char *cp;
	char c;

	c = x;
	cp = Line + Charoff0 + Charcol;
	if(spaceflg)
		cp[0] = ' ';
	else
		cp[0] = ISPRINT(c) ? c : '.';
	Charcol++;
}

/*
 * NAME:     lineflush
 * FUNCTION: flush the 'Line' buffer and update Vaddr0 address pointer.
 * INPUTS:   initflg   When set, just clear col counters and Line[].
 * RETURNS:  none
 *
 * Output "compaction" is done in this routine.
 * When Line[] is identical to the previous line Prevline[], output is
 * suppressed. The variable 'skipflg' is used to print a '*' once, to
 * denote that an area is compacted.
 */
static lineflush()
{
	char *cp;
	int currlen;

	if(WBincol > 0) {
		cp = Line + Charoff0 + Opt.opt_nfields + 11;
		cp[1] = '\0';
		while(*cp == ' ' && cp > Line)
			*cp-- = '\0';
		if(Opt.opt_compactflg) {
			currlen = cp - Line + 1;
			if(!(currlen == Prevlen && strncmp(Prevline,Line,currlen) == 0)) {
				memcpy(Prevline,Line,currlen);
				Prevlen = currlen;
				if(skipflg)
					printf("*\n");
				printf("%08x: %s\n",Vaddr0,Line);		/* ADDR_WIDTH */
				skipflg = 0;
			} else {
				skipflg++;
			}
		} else {
			skipflg = 0;
			printf("%08x: %s\n",Vaddr0,Line);		/* ADDR_WIDTH */
		}
		WBincol = 0;
		Charcol = 0;
	}
	Vaddr0 += (Opt.opt_fieldsize * Wrapcol);
}

static lineflush_clr()
{

	memset(Line,' ',LINEWIDTH);
	WBincol = 0;
	Charcol = 0;
	Prevlen = 0;
	skipflg = 0;
}

