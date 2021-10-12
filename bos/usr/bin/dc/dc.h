/* @(#)54	1.14  src/bos/usr/bin/dc/dc.h, cmdcalc, bos411, 9437C411a 9/15/94 13:05:15 */
/*
 * COMPONENT_NAME: (CMDCALC) calculators
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3 26 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#define BLK sizeof(struct blk)

/* Size of a pointer--used to allocate space for arrays. */
#define PTRSZ sizeof(void *)

/* Allocation quantum for struct blk. */
#define HEADSZ (64*BLK)

/* Size of main value stack */
#define STKSZ 100

/* Size of stack used for strings being interpreted as commands. */
#define RDSKSZ 100

/* Size of symbol table for values saved in registers. */
#define TBLSZ 256

/* Maximum index for arrays */
#define MAXIND 2048

/* Posix line length for printing long numbers (the newline is not included
   in this value). */
#define LL 69

/* Two-character comparison operators: !< !> != */
#define NL 1
#define NG 2
#define NE 3

#define length(p)	((p)->wt-(p)->beg)
#define rewind(p)	(p)->rd=(p)->beg
#define create(p)	(p)->rd = (p)->wt = (p)->beg
#define fsfile(p)	(p)->rd = (p)->wt
#define truncate(p)	(p)->wt = (p)->rd
#define sfeof(p)	(((p)->rd==(p)->wt)?1:0)
#define sfbeg(p)	(((p)->rd==(p)->beg)?1:0)
#define sungetc(p,c)	*(--(p)->rd)=c
#define NEGBYTE 0200

#define MASK (-1 & ~0377)
#define sgetc(p)	( ((p)->rd==(p)->wt) ? EOF :\
				( ((*(p)->rd & NEGBYTE) != 0)\
				 ? (*(p)->rd++ | MASK)\
				 : *(p)->rd++ ))
#define slookc(p)	( ((p)->rd==(p)->wt) ? EOF :\
				( ((*(p)->rd & NEGBYTE) != 0)\
				 ? (*(p)->rd | MASK)\
				 : *(p)->rd ))
#define sbackc(p)	( ((p)->rd==(p)->beg) ? EOF :\
				( ((*(--(p)->rd) & NEGBYTE) != 0)\
				 ? (int)(*(p)->rd | MASK)\
				 : (int)*(p)->rd ))
#define sputc(p,c)	{if((p)->wt==(p)->last)more(p); *(p)->wt++ = c; }
#define salterc(p,c)	{if((p)->rd==(p)->last)more(p);\
			 *(p)->rd++ = c;\
			 if((p)->rd>(p)->wt)(p)->wt=(p)->rd;}
#define sunputc(p)	(*( (p)->rd = --(p)->wt))
#define zero(p)	memset((p)->beg, 0, (p)->last-(p)->beg)
#define EMPTY if(stkerr != 0){printf("stack empty\n"); continue; }
#define EMPTYR(x) if(stkerr!=0){pushp(x);printf("stack empty\n");continue;}
#define EMPTYS if(stkerr != 0){printf("stack empty\n"); return(1);}
#define EMPTYSR(x) if(stkerr !=0){printf("stack empty\n");pushp(x);return(1);}
#define error(p)	{printf(p); continue; }
#define errorrt(p)	{printf(p); return(1); }

/* The structure below is the basic structure used for the internal
   representation of an arbitrary-precision number in 'dc'.  Numbers are
   represented in 100's complement, base-100 notation.

   Suppose we want to represent the base 10 number n = X.Y where X is
   any sequence of 0 or more digits and Y is a sequence of d digits.

   Let m = n * 10^d.  We can write m as Z, a string of digits with no
   leading 0s.  If m == 0, Z is empty.  (Normally, Z=XY, that is, X
   contatenated with Y, unless X has leading 0s or X is empty and Y
   has leading 0s.)  Trailing 0s in Y are represented explicitly.  If
   X consists of nothing but 0s, leading 0s in Y are maintained
   implicitly by contributing to the scale factor.

   The decimal representation Z is converted to its base-100
   representation Z', which will have half as many digits as Z. (This
   conversion can be done by hand by starting at the right end side of
   Z and converting each pair of decimal digits into a single base-100
   digit.

   If n < 0, let Z'' be the 100's complement of Z'.  This is computed
   by replacing each digit i with the difference 99-i add adding 1 to
   the resulting number.  A negative number can be represented with any
   number of leading 99s.  When the 100's complement is computed, the
   result should have exactly 1 leading 99.  If n >= 0, let Z'' be Z.

   The digits in Z'' are stored in a 'struct blk'.  If x is a pointer
   to struct blk, then x->beg is a buffer that ends at x->last.  Bytes
   [x->beg .. x->wt-1] contain the number, written from
   right-to-left.  That is, byte x->beg[0] contains the least
   significant base-100 digit of Z''.  Bytes[x->wt..x->last-1] are
   unused and can be used to expand the number without reallocating
   memory. Byte[x->last] is a tag, used to differentiate between
   numbers, strings, and arrays.

   If n is negative, the leading 99 is replaced by 255, so x->wt[-2]
   will contain 255.  Finally, x->wt[-1] contains d, the scale factor
   for n.

   Examples:

   NUMBER			Internal Representation	Notes
   ======                       =======================	=====
   1				1 0
   .1				1 1
   0				0			d only
   .00				2			d only
   -1				255 0
   -.1				255 1
   _.100			0 255 3			trailing 0s represented
   -10				90 255
   -100				0 255 0
   2				2 0
   .2				2 1
   .002				2 3			leading 0s implied by
							scale factor
   20				20 0
   -.002			98 255 3

   Field x->rd is a reading pointer, used to process a number.

   Some functions expect the scale factor to be removed, so x->wt will
   be decremented before calling the function.  Other functions expect
   only positive numbers, so no '255' byte will be found at the end in
   this case. */
struct blk {
	char	*rd;
	char	*wt;
	char	*beg;
	char	*last;
};
#define TAG_NUMBER '\0'
#define TAG_STRING 'S'
#define TAG_ARRAY 'A'

static struct	blk *hfree;		/* A free list of struct blks */

static struct	blk *arg1, *arg2;
static int	svargc;
static char	**svargv;
static FILE	*curfile;
static struct	blk *inbas;		/* Internal representation of the
					   input base. */
static struct	blk *tenptr;		/* Internal representation of 10. */
static struct	blk *basptr;		/* Internal representation of obase
					   (without an explicit scale). */
static struct	blk *twoval, *chptr, *strptr;

/* The main value stack.  stack[0] is never used. */
static struct	blk *stack[STKSZ];
static struct	blk **stkptr;
#define stkbeg (&stack[0])
#define stkend (&stack[STKSZ-1])
static int	stkerr;			/* Set to 1 if too many pops are
					   attempted.  Reset by pushp(). */

struct	blk *readstk[RDSKSZ];
struct	blk **readptr;

static int	k;			/* Current scale factor. */
static struct	blk *scalptr;		/* Internal representation of k. */

/* Variables set by division and related routines. */
static struct	blk *irem;
static char	savk;
static int	skd,skr;

/* Structures used for stack elements for registers being interpreted as
   stacks.  This array is used to create a linked list of free stack
   elements. */
struct	sym {
	struct	sym *next;
	struct	blk *val;
} symlst[TBLSZ];
static struct	sym *stable[TBLSZ];
static struct	sym *sfree;		/* Head of free symbol list */

/* Variables for statistics. */
static long	rel;			/* Number of calls to release() */
static long	nbytes;
static long	all;			/* Number of calls to
					   copy() or salloc(). */
static long	headmor;		/* Number of calls to headmor(). */

static int	fw = 1;			/* Printed width of a digit in obase.
					   This will be greater than 1 when
					   obase > 16. */
#ifdef KEEP_DIGIT_GROUPS
static int	ll = LL;		/* Number of characters allowed on
					   a line when printing a number.
					   (The newline is not included in
					   this value.) */
#else
#define ll LL
#endif

static long	obase = 10;		/* Current output base. */
static int	logo;			/* floor(Log (base 2)) of obase */
static int	log10;			/* floor(Log (base 2)) of 10 */
static int	count;			/* Remaining characters in
					   output line. */
static char	*dummy;
