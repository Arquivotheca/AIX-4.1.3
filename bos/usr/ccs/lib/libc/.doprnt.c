#ifndef LD_128
static char sccsid[] = "@(#)38  1.107  src/bos/usr/ccs/lib/libc/doprnt.c, libcprnt, bos41J 5/26/95 14:06:04";
#endif /* LD_128 */
/*
 * COMPONENT_NAME: (LIBCPRNT) Standard C Library Print Functions
 *
 * FUNCTIONS: _doprnt (or _doprnt128)
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* NOTICE: Please read all of the following before making any
 *         changes to the code.
 *
 * A Short Essay On Structured Code, Performance, and the Dreaded GOTO.
 * ===================================================================
 *
 * This code depents on the number of bits for char, short, int and 
 * long are 8, 16, 32, and 32 respectively.  It will require rework
 * when we go to 64 bit integer.
 *
 * This code is frankly structured to allow the compiler to
 * generate the best possible code to identify and process
 * as quickly as possible the most commonly used formats.
 * This has lead to a source structure with an awkward and
 * hard to follow logic flow, and a lot of gotos.  In addition
 * it is important that the actual encodings for the format
 * classification tree are chosen so that if -> else if ->
 * else if decisions that will be made together are encoded
 * in the same byte of the flag, so that the byte itself can be
 * moved to one condition register and used to resolve several
 * branches.  The Dreaded GOTO is very useful in this code 
 * because we are trying to tell the compiler exactly how to
 * generate branches at the machine level.
 *
 * We recognize that we have traded performance for much
 * more complex and hard to maintain source code.  We do not do
 * so lightly, but the fact is that this code has a serious
 * influence on the SPEC benchmarks, most likely other benchmarks, and
 * on a lot of user code.  This means that when you make a change
 * in this code, however trivial, that you MUST examine the
 * object code produced before and after the change, and  
 * re-evaluate the performance of the code in addition to running
 * functional regression test.  If you don't know enough assembly
 * language to examine the object code, you need to find someone
 * who does as part of your code review.
 *
 * Additional Notes About This Code.
 * ================================
 * This source actually builds a number of different objects.
 * If LD_128 is defined, _doprnt128 is built, which treats
 * long double objects as 128 bits wide.  If LD_128 is not defined,
 * long double objects are treated as 64 bits (the same format as
 * double precision).
 *
 * If _THREAD_SAFE is defined, the code must be thread safe.  The
 * interface is not changed, but all global data must be locked
 * when used.
 *
 * The following macros are defined in xputc.h:
 *
 * #define XPUTC_DECL  
 * #define XPUTC_INIT(p)
 * #define XPUTC_FINISH(p)
 * #define XPUTC(x,p)
 * #define PUT1(x,n,iop)
 * #define PUT0(s,n,iop)
 *
 * Be careful about defining and using any local variable with
 * a leading underscore ('_') in the name.  These macros
 * use a number of such varibles; look at xputc.h for a description.
 * Since these macro use stack variables, any function that
 * uses them must declare the variables (XPUTC_DECL), initialize
 * them (XPUT_INIT) and perform clean up processing before exit
 * (XPUTC_FINISH).  This is especially important if you add a
 * new exit point to any function; it would be easy to miss the
 * clean up.
 *
 * The following read-only static tables are defined in doprnt_pow_table.h:
 * 
 * typedef struct fp 
 * static double		table1[]
 * static char 			table2[]
 * static char			table3[]
 * static double		tablepow[]
 * static struct fp		pow1[129][2]
 * static struct fp		pow3[13][2]
 * static const int		fmt_flags[256]
 * static unsigned short	a99[100]
 * static unsigned short	a256[512]
 * static unsigned short	a64[64]
 * static char			HexTab[64]
 *
 * The following are defined in print.h:
 *
 * #define MAXDIGS
 * #define MAXECVT
 * #define MAXQECVT
 * #define max(a,b)
 * #define min(a,b)
 *
 * Floating Point Variables.
 * ========================
 *
 * The main function _doprnt (or _doprnt128) may not declare or
 * use ANY local variables of type float, double or long double.
 * There are no exceptions.  All code which actually must
 * manipulate floating point data must be done in static local
 * functions or by calls to other libc.a routines.  The actual
 * floating point arguments should be passed by va_arg() macro.
 *
 */

#define	 CVTBUFSIZE    1000
#define _ILS_MACROS
#ifdef _THREAD_SAFE
# ifndef _STDIO_UNLOCK_CHAR_IO
# define _STDIO_UNLOCK_CHAR_IO
#endif /* _THREAD_SAFE */

#include <stdio_lock.h>

#ifdef LD_128
#include "rec_mutex.h"
extern struct rec_mutex _qecvt_rmutex;
#endif /* LD_128 */

#endif /* _THREAD_SAFE */

#include "ts_supp.h"

#include <limits.h>		/* 64890: LONGLONG_MIN */
#include <stdio.h>
#include <ctype.h>
#include <langinfo.h>
#include <varargs.h>
#include <values.h>
#include <string.h>
#include <fp.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>
#include "print.h"	/* parameters & macros for doprnt: MAXDIGS, ... */
#include "xputc.h"


#ifdef LD_128
/* _qecvt() expects a QUAD struct, not a long double.  They are
 * slightly difference, since the address of the struct must be
 * passed in a gpr for the call.
 */
typedef struct {
	double	hi, lo;
}     QUAD;
extern char	*_qecvt(QUAD , int, int *, int *);
extern char	*_qfcvt(QUAD , int, int *, int *);
#endif /* LD_128 */

#define RFREE(s)	if (s) free((void *)s)
#define UNSET	(-1)
#define WCTOMB(c,x)	((rc=wctomb(c,x)) > 1 ? rc : 1)

#define MBTOWC(c,x)	((rc=mbtowc(x,c,mb_cur_max)) > 1 ? rc : 1)

#define MBLEN(x)	((mbleng=mblen(x,mb_cur_max)) > 1 ? mbleng : 1)

#define ISDIGIT(x)	( (unsigned)((x) - '0') <= 9 )

#define MAX_RADIX_LEN_IN_INT	8  /* max radix length in byte */
#define SIZE_BUF_SHORT	  ((MAXDIGS + 1)/2)


/* Constant table to decode letter in format item.
 * This is read only so it is thread safe.
 * When this array is indexed by an ascii character
 * a mask of flags indicating the properties of the character
 * is obtained.  All of the following MUST be defined BEFORE
 * including "doprnt_pow_table.h".
 */

#define _B	 0x04000000   /* % null */
#define _is_pct_null   _B
#define _set_width    0x02000000   /* width has been set */
#define _complicate   0x01000000   /* when this bit is set, the format is not a simple format,
				    * it would contain at least one option */ 

#define _P	 0x1000	 /* - + blank # B N 0 J ' */
#define _Pm	 0x801000  /* - */
#define _Pp	 0x401000  /* + */
#define _Pb	 0x201000  /* blank */
#define _P0	 0x101000  /* 0 */
#define _Ps	 0x081000  /* # */
#define _Pq	 0x041000  /* ' */
#define _Pw	 0x021000  /* w */
#define _P_	 0x011000  /* B N J */

/* the following 4 macros must not overlap with prefix flag except _is_wide */
#define _is_wide	(_Pw & ~_P)
#define _is_longdouble	0x08
#define _is_longlong	0x04
#define _is_long	0x02
#define _is_half	0x01

#define _W	0x2000	/* * 1 2 3 4 5 6 7 8 9 . */
#define _Wd	0x2200	/* 1 2 3 4 5 6 7 8 9 */
#define _Ws	0x2100	/* * */

#define _L	0x4000	/* l h L ll lL Ll LL w */

#define _F	0x8000	/* % d i u o x X f e E g G c C s S p n $ */
#define _i	0x8800	/* d i u o x X p */
#define _si	0x8880	/* d i */
#define _u	0x8840	/* u */
#define _x	0x8820	/* x X p */
#define _o	0x8810	/* o */

#define _cs	0x8400	/* c C s S  */
#define _S	0x8480	/* S */
#define _s	0x8440	/* s */
#define _C	0x8420	/* C */
#define _c	0x8410	/* c */

#define _f	0x8200	/* f e E g G */
#define _fe	0x8280	/* e E */
#define _ff	0x8240	/* f */
#define _fg	0x8220	/* g G */

#define _r	0x8100	/* % n $ */

#define	 _is_prefix	       _P
#define	 _is_minus	       (_Pm & ~_P)
#define	 _is_plus	       (_Pp & ~_P)
#define	 _is_blank	       (_Pb & ~_P)
#define	 _is_sharp	       (_Ps & ~_P)
#define	 _is_zero	       (_P0 & ~_P)
#define	 _is_quote	       (_Pq & ~_P)
#define	 _is_noop	       (_P_ & ~_P)

#define	 _is_width	       _W
#define	 _is_digit	       (_Wd & ~_W)
#define	 _is_star	       (_Ws & ~_W)

#define	 _is_length	       _L

#define	 _is_format	       _F

#define	 _is_integer	       (_i & ~_F)
#define	 _is_signed	       (_si & ~_i) 
#define	 _is_unsign	       (_u & ~_i)   
#define	 _is_xformat	       (_x & ~_i)   
#define	 _is_octal	       (_o & ~_i)   

#define	 _is_char_str	       (_cs & ~_F)
#define	 _is_S		       (_S  & ~_cs)
#define	 _is_s		       (_s  & ~_cs)
#define	 _is_C		       (_C  & ~_cs)
#define	 _is_c		       (_c  & ~_cs)

#define	 _is_float	       (_f  & ~_F)
#define	 _is_eformat	       (~_f & _fe)
#define	 _is_fformat	       (~_f & _ff)

#include "doprnt_pow_table.h"

/* table for quick classification of the formating character.
   It is used by _doprnt routine to traverse the hierachical path
   of the formating structure.
 */
static const int        fmt_flags[256] = {
        /* 00             */    _B,    0,    0,    0,    0,    0,    0,    0,
        /* 08             */     0,    0,    0,    0,    0,    0,    0,    0,
        /* 10             */     0,    0,    0,    0,    0,    0,    0,    0,
        /* 18             */     0,    0,    0,    0,    0,    0,    0,    0,
        /* 20    !"#$%&'  */   _Pb,    0,    0,  _Ps,   _r, _B | _r,    0,  _Pq,
        /* 28   ()*+,-./  */     0,    0,  _Ws,  _Pp,    0,  _Pm,   _W,    0,
        /* 30   01234567  */   _P0,  _Wd,  _Wd,  _Wd,  _Wd,  _Wd,  _Wd,  _Wd,
        /* 38   89:;<=>?  */   _Wd,  _Wd,    0,    0,    0,    0,    0,    0,
        /* 40   @ABCDEFG  */     0,    0,  _P_,   _C,    0,  _fe,    0,  _fg,
        /* 48   HIJKLMNO  */     0,    0,  _P_,    0,   _L,    0,  _P_,    0,
        /* 50   PQRSTUVW  */     0,    0,    0,   _S,    0,    0,    0,    0,
        /* 58   XYZ[\]^_  */    _x,    0,    0,    0,    0,    0,    0,    0,
        /* 60   `abcdefg  */     0,    0,    0,   _c,  _si,  _fe,  _ff,  _fg,
        /* 68   hijklmno  */    _L,  _si,    0,    0,   _L,    0,   _r,   _o,
        /* 70   pqrstuvw  */    _x,    0,    0,   _s,    0,   _u,    0,   _L | _Pw,
        /* 78   xyz{|}~   */    _x,    0,    0,    0,    0,    0,    0,    0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
};


/********************************************************
 0x08000	 0x08800	0x08880
is_format  --- _is_integer --- _is_signed  --- d i
	    |		   |	0x08840
	    |		   |_ _is_unsign  --- u
	    |		   |	0x08820
	    |		   |_ _is_xformat --- x X p
	    |		   |	0x08810
	    |		   |_ else	 --- o
	    |	 0x08400	 0x08480
	    |_ _is_char_str --- _is_string --- s S
	    |		    |	 0x08440
	    |		    |_ else	 --- c C
	    |	 0x08200      0x08280
	    |_ _is_float --- _is_e_format  ---	e E
	    |		 |    0x08240
	    |		 |_ _is_f_format  ---  f
	    |		 |    0x08220
	    |		 |_ else	 ---  g G
	    |	 0x08100
	    |_ else	 --- % n $

  0x4000
is_length --- l h L ll lL Ll LL w

  0x2000       0x2002
is_width ---  _is_digit	 --- 1 2 3 4 5 6 7 8 9
	  |    0x2001
	  |_  else	 --- * .


  0x1000       0x801000	    
is_prefix --- _is_minus --- -
	   |   0x401000 
	   |_ _is_plus	--- +
	   |   0x201000 
	   |_ _is_blank --- blank
	   |   0x101000 
	   |_ _is_zero	--- 0
	   |   0x81000 
	   |_ _is_sharp	 --- #
	   |   0x41000 
	   |_ _is_quote	 --- '
	   |   0x21000 
	   |_ _is_wide	--- w
	   |   0x11000 
	   |_ _is_noop	--- B N J
********************************************************/



/*
 * This structure contains variables that are needed in several routines and
 * used to be global and static.  For thread-safety and to reduce the number
 * of global variables, these will now be passed around via this structure.
 * #defines will be used to keep the code as unchanged as possible.  Note
 * that this data is only used in internal routines, so no new visible
 * interfaces will be added, although some internal ones will.
 */
struct arg_data {
	int	*size;	    /* size array (keep track of arg size in bytes) */
	int	*order;	    /* order array (keep track of arg variable order) */
	int	sla;	    /* last alloced index for size array */
	int	ola;	    /* last alloced index for order array */
	int	ts;	    /* total size of all arguments in bytes */
	int	ac;	    /* (argument count) total # of arguments in format */
	int	ma;	    /* (maximum arg) highest numbered argument */
};
typedef struct arg_data arg_data;

#define size			(argdata->size)
#define order			(argdata->order)
#define size_last_alloc		(argdata->sla)
#define order_last_alloc	(argdata->ola)
#define tot_size		(argdata->ts)
#define arg_count		(argdata->ac)
#define max_arg			(argdata->ma)

static int	reorder(char *, va_list, char **, va_list *, arg_data *);
static int	width(char **, char **, int *, arg_data *);
static int	argnum(char **, int *, arg_data *);
static char	*insert_thousands_sep(char *, char *);
static char	*myecvt( double val, int *ndigit, int *decpt, int *sign, 
int *ibuf, char *cvtbuf);
static char	*myfcvt( double val, int *ndigit, int *decpt, int *sign, 
int *ibuf, char *cvtbuf);

#ifdef LD_128 /* 128 bit version of doprnt */
static char	*myqecvt( QUAD ldval, int ndigit, int *decpt, int *sign,
char *cvtbuf);
static char	*myqfcvt( QUAD ldval, int ndigit, int *decpt, int *sign,
char *cvtbuf);
#endif /* LD_128 */

/*
 * FUNCTION: _doprnt: common code for printf, fprintf, sprintf
 *
 *	With AIX 4 and XPG 4, the IBM extension 'ws' has been replaced with
 *	X/Open's 'S' and 'wc' has been replaced with 'C'.  The code for 'ws'
 *	and 'wc' is left as is for binary compatibility reasons.
 * Parameters:
 *	oformat		Origianl format.
 *	oargs		Original argument list.
 *	iop		File pointer.
 */

#ifdef LD_128 /* 128 bit version of doprnt */
int	
_doprnt128(char *oformat, va_list oargs, FILE *iop)
#else /* must be 64 bit version */
int	
_doprnt(char *oformat, va_list oargs, FILE *iop)
#endif /* LD_128 */
{
	/* The count variable counts the number of output bytes. */
	int		count = 0;
	register char	*format;	/* (possibly new) format string */
	va_list		args;		/* (possibly reordered) argument list */
	char		*nformat;	/* temp. format pointer if reordering */
	va_list		nargs;		/* temp. args pointer if reordering  */
	int		reorderflag = UNSET;	/* reordering flag */
	struct arg_data argdata;
	wchar_t		*wcp, wc;	/* character pointers used in 'S' format */
	register char	*bp, *p;	/* Starting and ending points for value to be printed */
	int		width, prec;	/* Field width and precision */
	register int	fcode;		/* Format code */
	int		leadz;		/* int indicating leading zeros format */
	int		lzero;		/* Number of padding zeroes required on the left */
	int		padlen;		/* Number of padding blanks required, needed when value 
					 * string is shorter than the requested width */
	char		*mbbuf;		/* buffer to hold a multibyte string */
	wchar_t		*wcbuf;		/* buffer to hold a wchar string */
	unsigned short	buf_short[SIZE_BUF_SHORT+1];
	unsigned short	*sp;
        unsigned short  *tsp;
	char		*buf;
	char		cvtbuf[CVTBUFSIZE+MAX_RADIX_LEN_IN_INT];
	char		*prefix;	/* Pointer to sign, "0x", "0X", or empty */
	int		prefix_size;	/* size of prefix */
	char	        *hextab;	/* Pointer to "0123456789abcdef0x" 
					 * or "0123456789ABCDEF0X" */
	long		val, val_tmp;	/* The value being converted */
	long long int	llval;		/* long long value being converted */
	char		cval;		/* The value being converted */
	int		mb_cur_max;	/* The value of MB_CUR_MAX */
	char		*tab;		/* Pointer to a translate table for
					 * digits of whatever radix */
	int		k, n;		/* Work variables */
	int		i, j, nb, rc, mbleng;	/* Work variables */
	int		decpt, sign, ndigit;
	int		ibuf[6 + MAX_RADIX_LEN_IN_INT];
	int		rzero;		/* right zeros */
	int		tzero;		/* trailing zeros in integral part */
	int		radlen;		/* the length of radix char */
	char		*radchr;
	char		*default_radchr = ".";
	int		radchr0;	/* keeps  a register copy of the radix character */
	int		sys_radlen = 0;
	char		expbuf[8];
	int		explen;
	int		patchr;
	int		signlen, len_out_minus_pad, lenpad, ndigit_fraction_part;
	char		signchr;	/* to store quoted integral part of float in f format */
	char		buf_quote[400];
	unsigned int	flags;		/* bit mask indication properties of format letter */
	unsigned int	fprefix;	/* bit mask indication properties of format letter */
	char		*anchor;	/* saves location of last % in format */
	XPUTC_DECL;			/* declare temp variables used by xputc macro */

	/*** END OF LOCAL VARIABLE DECLARATIONS ***/

	XPUTC_INIT(iop);	/* prepare to use the xputc macro */
	args = oargs;		/* copy argument pointer; may need to reorder */
	nformat = nargs = 0;	/* initialize counters */
	format = oformat;	/* copy format string; may need to reorder */
	mb_cur_max = MB_CUR_MAX; /* local copy so we only evaluate macro 1 time */

	/*
	 *	The main loop -- this loop goes through one iteration
	 *	for each string of ordinary characters or format specification.
	 */

	while (*format) {
new_format:
		fcode = *format;
		if (fcode != '%') {
			/* If character we're looking at is not '%',
			 * use strchr() to see if there's another one.
			 * If not, just put out the rest of the characters
			 * and we're done.  If there is another '%', 
			 * put out all characters up to it and then start
			 * handling the format.
			 */
			if ((p = strchr(format, '%')) == NULL) {
				i = strlen(format);
				PUT0(format, i, iop);
				count += i;
				XPUTC_FINISH(iop);
				return(ferror(iop) ? EOF : count);
			} else {
				i = p - format;
				PUT0(format, i, iop);
				format += i;
				count += i;
			}
		}

		format++;
		/* save last % position for call to reorder */
		anchor = format - 1;

		/* Use table to quickly classify the format
		 * character.
		*/

		fcode = *format++;
		flags = fmt_flags[fcode];

		/* reset flags */
		fprefix = 0;
		prefix = "";
		prefix_size = 0;
		width = 0;
		prec = -1;

		/* Set two null chars at the end of buf_short
		 * so that the same buffer can be used for both
		 * short or char types.
		 */
		buf_short[SIZE_BUF_SHORT] = 0;
		lzero = -1;

		if (flags & _is_format) {
		/*
		 *	The character addressed by format must be
		 *	the format letter -- there is nothing
		 *	left for it to be.
		 *
		 *	The prefix is a sign, a blank, "0x",
		 *	"0X", or null, and is addressed by
		 *	"prefix".
		 *
		 *	The value to be printed starts at "bp"
		 *	and continues up to and not including
		 *	"p".
		 */

fmt_letter:
			if (flags & _is_integer) {
                                /* all the integer format are processed by two digits at each
                                   iteration from the least significant digit, instead of the
                                   traditional digit by digit to reduce the number of branches
                                   required to complete the conversion.  The following is the 
                                   detail about how the conversion is actually done.
                                   While ( i >= A) {  where A = 100(%u, %i, %d), 64 (%o), 256 (%x, %p)
                                      j = the value 2 least significant digits. 
                                          For u,i,d, just find the remainder of dividing i by 100,
                                          for o and x, p format, use bit and to get the least 6 and 8
                                          bits respectively.
                                      Use j as the index to a preset table to get two digit characters.
                                      i = i / 100 (for %u, %i, %d) or
                                        = i >> 6  (for %o)
                                        = i >> 8  (for %x, %p)
                                   }
                                   Get the final digit/digits.
                                   if i < 10, 16, or 8 (for u,i,d,  x,p, and o respectively)
                                   we only need to emit one digit.
                                   Adjust the character point properly.
                                 */
            
				sp = buf_short + SIZE_BUF_SHORT;
				p = (char *)sp;


				if (!(fprefix &	 _complicate)) {
				/* ---- fast path for simple integer format ---- 
				 * fast path for simple format with at most 
				 * width parameter specified.
				 */
					val = va_arg(args, long);
					if (flags & _is_signed) {  /* d i format */
						if (val < 0) {
							prefix_size = 1;
							if (val != HIBITL) 
								val = -val;
						}
						while ((unsigned)val >= 100) {
							long	val_new;
							int	i;
							val_new = (unsigned)val / 100;
							i = (unsigned)val - (unsigned)val_new * 100;
							*--sp = a99[i];
							val = (unsigned)val_new;
						}
						*--sp = a99[val];
						bp = (char *)sp + ((unsigned)val < 10);
					        /* output characters for %i and %d formats */
					        nb = p - bp;
					        if (fprefix & _set_width) {
						        if (width > (nb + prefix_size) ) {
                                                           i = width - (nb + prefix_size);
							        PUT1(' ', i, iop);
                                                           count += i;
                                                        }
					        }
                                                if(prefix_size)
							XPUTC('-', iop);
					        PUT0(bp, nb, iop);
					        count += nb + prefix_size;
					        continue;

					} else if (flags & _is_unsign) {  /* u format */
unsigned_simple:
						while ((unsigned)val >= 100) {
							long	val_new;
							int	i;
							val_new = (unsigned)val / 100;
							i = (unsigned)val - (unsigned)val_new * 100;
							*--sp = a99[i];
							val = (unsigned)val_new;
						}
						*--sp = a99[val];
						bp = (char *)sp + ((unsigned)val < 10);
						goto simple_int_output;
					} else if (flags & _is_xformat) {  /* x X p format */
						/* set up char table for x/X format
			   %x format start at a256[256]
			   %X format start at a256[0]
			 */
						tsp = &a256[(fcode & 0x20) << 3];

						while ((unsigned)val >= 256) {
							*--sp = tsp[val & 0xff];
							val = (unsigned)val >> 8;
						}
						*--sp = tsp[val];
						bp = (char *)sp + ((unsigned)val < 16);
						goto simple_int_output;
					} else {  /* must be o format */
						while ((unsigned)val >= 64) {
							*--sp = a64[val & 0x3f];
							val = (unsigned)val >> 6;
						}
						*--sp = a64[val];
						bp = (char *)sp + ((unsigned)val < 8);
						goto simple_int_output;
					}
simple_int_output:
					/* output characters for integer formats other than %d %i */
					nb = p - bp;
					if (fprefix & _set_width) {
						if (width > (nb + prefix_size) ) {
                                                   i = width - (nb + prefix_size);
							PUT1(' ', i, iop);
                                                   count += i;
                                                }
					}
					PUT0(bp, nb, iop);
					count += nb + prefix_size;
					continue;
				} /* ---- end of fast path for simple integer format ---- */
				else if (fprefix & _is_longlong) {
					if (fcode == 'p') { /* longlong pointer not supported yet */
						fprefix &= ~_is_longlong;
						val = va_arg(args, long);
					}
					else llval = (long long) va_arg(args, long long);
				} else {
					val = va_arg(args, long);
					if (fprefix & _is_half) {
						if (flags & _is_signed) 
							val = (short) val;
						else 
							val = (unsigned short) val;
					}
				}

				if (flags & _is_signed) {  /* d i format */

					if (fprefix & _is_longlong) {
						if (llval < 0) {
							prefix = "-";
							prefix_size = 1;
							if (llval != LONGLONG_MIN)
								llval = -llval;
						} else if (fprefix & _is_plus) {
							prefix = "+";
							prefix_size = 1;
						} else if (fprefix & _is_blank) {
							prefix = " ";
							prefix_size = 1;
						}
						goto unsigned_label_longlong;
					} else {
						if (val < 0) {
							prefix = "-";
							prefix_size = 1;
							if (val != HIBITL) 
								val = -val;
						} else if (fprefix & _is_plus) {
							prefix = "+";
							prefix_size = 1;
						} else if (fprefix & _is_blank) {
							prefix = " ";
							prefix_size = 1;
						}
						goto unsigned_label_long;
					}
				} 
				    else if (flags & _is_unsign) {  /* u format */
					if (fprefix & _is_longlong) {
unsigned_label_longlong:
						if (llval || prec) {
							while ((unsigned long long)llval >= 100) {
								unsigned long long int	llval_new;
								int	i;
								llval_new = (unsigned long long)llval / 100;
								i = (unsigned long long)llval - llval_new * 100;
								*--sp = a99[i];
								llval = llval_new;
							}
							*--sp = a99[(int)llval];
							bp = (char *)sp + ((unsigned long long)llval < 10);
						} else 
							bp = (char *)sp;
					} else {
unsigned_label_long:
						if (val || prec) {
							while ((unsigned)val >= 100) {
								long	val_new;
								int	i;
								val_new = (unsigned)val / 100;
								i = (unsigned)val - (unsigned)val_new * 100;
								*--sp = a99[i];
								val = (unsigned)val_new;
							}
							*--sp = a99[val];
							bp = (char *)sp + ((unsigned)val < 10);
						} else 
							bp = (char *)sp;
					}
					if (fprefix & _is_quote) 
						p = insert_thousands_sep(bp, p);
				} 
				    else if (flags & _is_xformat) {  /* x X p format */
					bp = (char *)sp;
					hextab = &(HexTab[fcode & 0x20]);

					if (fprefix & _is_longlong) {
						if ((fprefix & _is_sharp) && (fcode != 'p') ) {
							if (llval != 0) {
								prefix = hextab + 16;  /* get "0x" or "0X" */
								prefix_size = 2;
							}
						}
						if (llval || prec) {
							while ((unsigned long long)llval >= 16) {
								*--bp = hextab[llval&0xf];
								llval = (unsigned long long) llval >> 4;
							}
							*--bp = hextab[(int)llval];
						}
					} else {
						if ((fprefix & _is_sharp) && (fcode != 'p') ) {
							if (val != 0) {
								prefix = hextab + 16;  /* get "0x" or "0X" */
								prefix_size = 2;
							}
						}
						if (val || prec) {
                                                        /* The conversion for this code is different from the %x part in 
                                                           the fast path.  There is no reason for it.  It is 
                                                           just a different implementation.  May consider to 
                                                           change it to the way fast path is doing.
                                                         */
							while ((unsigned long)val >= 256) {
								*--bp = hextab[val&0xf];
								val = (unsigned) val >> 4;
								*--bp = hextab[val&0xf];
								val = (unsigned) val >> 4;
							}
							*(bp - 1) = hextab[val&0xf];
							bp -= (val > 0);
							val = (unsigned) val >> 4;
							*(bp - 1) = hextab[val];
							bp -= (val > 0);
						}

					}
				} 
				    else {  /* must be o format */
					bp = (char *)sp;
					if (fprefix & _is_longlong) {
						if (llval || prec) {
							while ((unsigned long long)llval >= 8) {
								*--bp = '0' + (llval & 0x7);
								llval = (unsigned long long) llval >> 3;
							}
							if (llval) 
								*--bp = '0' + llval;
							;
							if (fprefix & _is_sharp) 
								*--bp = '0';
						}
                                                else if(llval == 0) {
                                                        if (fprefix & _is_sharp)
                                                                *--bp = '0';
                                                }

					} else {
						if (val || prec) {
							while ((unsigned long)val >= 64) {
								*--bp = '0' + (val & 0x7);
								val = (unsigned) val >> 3;
								*--bp = '0' + (val & 0x7);
								val = (unsigned) val >> 3;
							}

							*(bp - 1) = '0' + (val & 0x7);
							bp -= (val > 0);
							val = (unsigned) val >> 3;
							*(bp - 1) = '0' + val;
							bp -= (val > 0);
							if (fprefix & _is_sharp) 
								*--bp = '0';
						}
                                                else if(val == 0) {
                                                        if (fprefix & _is_sharp)
                                                                *--bp = '0';
                                                }
					}
				}

				/* normal integer output */
				nb = p - bp;
				if (nb == 0) {
					if (prec != 0) {
						nb = 1;
						*--bp = '0';
					}
				}
				k = nb + prefix_size;
				if (prec > nb) {
					lzero = prec - nb;
					k += lzero;
				} else 
					lzero = 0;

				if (width <= k) {
					/* fast path for simple format %d,%x, ... etc. */
					if (prefix_size < 1) {
                                                /* the default case has no prefix.
                                                   Force compiler to fall thru this case by having the blank section.
                                                 */
					} else if (prefix_size == 1) {
						XPUTC(*prefix, iop);
					} else {  /* must be 0x or 0X */
						PUT0(prefix, 2, iop);
					}
					PUT1('0', lzero, iop);
					/* The value itself */
					PUT0(bp, nb, iop);
					count += k;
					continue;
				}

				count += width;
				padlen = width - k;

				if (fprefix & _is_minus) {
					if (prefix_size < 1) {
                                                /* the default case has no prefix.
                                                   Force compiler to fall thru this case by having the blank section.
                                                 */
					} else if (prefix_size == 1) {
						XPUTC(*prefix, iop);
					} else {  /* must be 0x or 0X */
						PUT0(prefix, 2, iop);
					}

					PUT1('0', lzero, iop);

					/* The value itself */
					PUT0(bp, nb, iop);

					/* Blanks on the right if required */
					PUT1(' ', padlen, iop);

				} else { /* not _is_minus */
					if ((fprefix & _is_zero) && (prec == -1)) {
						lzero += padlen;
					} else {
						PUT1(' ', padlen, iop);
					}

					/* Prefix, if any */
					if (prefix_size < 1) {
                                                /* the default case has no prefix.
                                                   Force compiler to fall thru this case by having the blank section.
                                                 */
					} else if (prefix_size == 1) {
						XPUTC(*prefix, iop);
					} else {  /* must be 0x or 0X */
						PUT0(prefix, 2, iop);
					}

					/* Zeroes on the left */
					PUT1('0', lzero, iop);

					/* The value itself */
					PUT0(bp, nb, iop);
				}

				continue;
			} /* --------------- end of integer ------------------ */
			else if (flags & _is_char_str) {
				buf = (char *)buf_short;

				if (!(fprefix &	 _complicate)) {
					/* ---- fast path for simple s/c format ----
			fast path for simple format with at most
			width parameter specified.
		      */
					if (flags & _is_s) {   /* s format */
						p = bp = va_arg(args, char * );
						n = strlen(p);
						if (width > n) {
							PUT1(' ', width - n, iop);
							count += width;
						} else 
							count += n;
						PUT0(bp, n, iop);
						continue;
					} else if (flags & _is_c) { /* c format */
						cval = va_arg(args, int);
						if (width > 1 ) {
							PUT1(' ', width - 1, iop);
							count += width;
							XPUTC(cval, iop);
						} else {
							count++;
							XPUTC(cval, iop);
						}
						continue;
					}
				}
				/* ---- end of fast path for simple s/c format ---- */

				if (flags & _is_s) {   /* s format */
					if (fprefix & _is_wide)	 
						goto S_format;
					p = bp = va_arg(args, char * );

					n = strlen(p);
					/* must use "unsigned" below, otherwise the
			   initialized prec = -1 would set n = prec.
			 */
					if ((unsigned)prec < n) 
						n = prec;
					p += n;
s_output:
					/* output characters for s and S formats */
					nb = p - bp;
					if (fprefix & _is_minus) {
						/* The value itself */
						if (nb > 0) {
							PUT0(bp, nb, iop);
						}
						/* Blanks on the right if required */
						padlen = width - nb;
						padlen = ~(padlen >> 31) & padlen;  /* zero it if negative */
						PUT1(' ', padlen, iop);
					} else { /* not _is_minus */
						/* Blanks on the left if required */
						if (fprefix & _is_zero) {
							if ((width < prec) && (width > 0)) 
								width = prec;
							padlen = width - nb;
							padlen = ~(padlen >> 31) & padlen;  /* zero it if negative */
							PUT1('0', padlen, iop);
						} else {
							padlen = width - nb;
							padlen = ~(padlen >> 31) & padlen;  /* zero it if negative */
							PUT1(' ', padlen, iop);
						}
						/* The value itself */
						if (nb > 0) {
							PUT0(bp, nb, iop);
						}
					}
					count += nb + padlen;
                                        if((flags & _is_S)  && mbbuf) {
                                           free(mbbuf);
                                           mbbuf = NULL;
                                        }
					continue;
				} else if (flags & _is_c) { /* c format */
					if (fprefix & _is_wide) 
						goto C_format;
					cval = va_arg(args, int);
c_merge:
c_output:
					/* output characters for c formats */
					padlen = width - 1;
					padlen = ~(padlen >> 31) & padlen;
					count += padlen + 1;

					if (fprefix & _is_minus) {
						XPUTC(cval, iop);
						/* Blanks on the right if required */
						PUT1(' ', padlen, iop);
					} else { /* not _is_minus */
						/* Blanks on the left if required */
						PUT1(' ', padlen, iop);
						XPUTC(cval, iop);
					}

					continue;

				} else if (flags & _is_S) {   /* format %S */
S_format:
					wcp = va_arg(args, wchar_t * );
					/* d051432 - to prevent core dump if trying to
			      determine length of a non-null terminated string
			      occurring at the end of a page.... */
					if (prec < 0)
						i = wcslen(wcp);
					else
						i = prec;
					j = mb_cur_max * (i + 1);
					if ((p = bp = mbbuf = (char *) malloc (j)) == NULL) {
						errno = ENOMEM;
						RFREE(nargs);
						RFREE(nformat);
						/* commit xputc macro vars to their home
				  location in memory
				 */
						XPUTC_FINISH(iop);
						return(-1);
					}
					/**********
			     only put prec bytes in the buffer
			   **********/
					if (prec >= 0) {
						/**********
				  loop thru the wchar array and convert characters until
				  prec bytes are in the buffer
				**********/
						n = 0;
						while (*wcp != 0x00) {
                                                   /* wcstombs() could be used instead,
                                                      for a performance gain if performance of wide character strings
                                                      becomes an issue.  
                                                    */
							if ((j = wctomb(&mbbuf[n], *wcp)) < 0) {
								free(mbbuf);
								mbbuf = NULL;
								break;
							}
							wcp++;
							if ((j + n) < prec) {
								n += j;
								continue;
							}
							if ((j + n) == prec) {
								n += j;
							}
							break;
						}


						if (mbbuf)
							p += n;
					} else if ((n = wcstombs (mbbuf, wcp, j)) > 0) {
						p += n;
					} else {
						free (mbbuf);
						mbbuf = NULL;
					}

					goto s_output;
				} else {  /* must be C format */
C_format:
					wc = va_arg(args, int);
					buf[0] = fcode;
					bp = buf; /* necessary in case WCTOMB() fails */
					p = bp + WCTOMB(buf, wc);
C_output:
					/* output characters for C formats */
					nb = p - bp;
					padlen = width - nb;
					/* same as if(padlen < 0) padlen = 0;  */
					padlen = ~(padlen >> 31) & padlen;

					count += padlen + nb;
					if (fprefix & _is_minus) {
						PUT0(bp, nb, iop);
						/* Blanks on the right if required */
						PUT1(' ', padlen, iop);
					} else { /* not _is_minus */
						/* Blanks on the left if required */
						PUT1(' ', padlen, iop);
						PUT0(bp, nb, iop);
					}

					continue;
				}
			} /* --------------- end of char_str ------------------ */
			else if (flags & _is_float) {
                        /* for all floating point conversion, we pass a buffer with a few offset from 
                           the beginning of the buffer so that we can insert radix character 
                           and shift the digits in the left of the radix character to the left.
                           We may also need this offset so that we can prefix the sign character when needed.
                         */

				if (sys_radlen == 0)  {
                                /* determine the radix character once for each call
                                    to the doprnt routine which has floating point
                                    conversion.
                                 */
					/* get radix character */
					radchr = nl_langinfo(RADIXCHAR);
					if (radchr[0] != '\0') {
                                                if(radchr[1] == '\0')
                                                   sys_radlen = 1;
						else 
                                                   sys_radlen = strlen(radchr);
					} else {
						sys_radlen = 1;
						radchr[0] = '.';
					}
				}

				if (flags & _is_fformat) {
                                /* There are two math paths in myfcvt, fast and normal.
                                   The normal path requires a larger buffer and
                                   the fast path requires a buffer aligned at 
                                   half word boundary.  These two buffer are passed 
                                   to myfcvt in ibuf and cvtbuf respectively.
                                 */ 
					if (prec < 0) { 
                                        /* set default value */
						ndigit = 6; 
						prec = 6; 
					} else 
						ndigit = prec;
#ifdef LD_128
					if (fprefix & _is_longdouble) {
						bp = myqfcvt(va_arg(args, QUAD), ndigit, &decpt, &sign, cvtbuf + MAX_RADIX_LEN_IN_INT);
						ndigit = strlen(bp);
					} else {
#endif /* LD_128 */
						bp = myfcvt(va_arg(args, double), &ndigit, &decpt, &sign,
						    ibuf + MAX_RADIX_LEN_IN_INT, cvtbuf + MAX_RADIX_LEN_IN_INT);
#ifdef LD_128
					}
#endif /* LD_128 */

					nb = ndigit;
					if (*bp > '9') 
						goto INF_NaN_output;
					goto f_output;
				} else if (flags & _is_eformat) {
 /* %e or %E format:
  * For precision less than 15 and not long double format
  * call myecvt, which is a faster implementation of double
  * to decimal conversion.  Otherwise, if long double call
  * myqecvt, which in turn calls _qecvt.  If not long double
  * call ecvt.  For all cases branch to "ecvt_merge" to actually
  * emit the converted digits.  Note:  ecvt_merge requires that
  * ndigits be set to the *actual* number of converted digits.
  * Also, for the number 0.0 ecvt will set decpt to 0, but 
  * ecvt_merge expects it to be 1, so fix that up.
  */

					if (prec < 0) {
                                        /* set default value */
						ndigit = 7;
						prec = 6;
					}
					if ((prec < 15)
#ifdef LD_128
					     && !(fprefix & _is_longdouble)
#endif /* LD_128 */
					    )  {
						ndigit = prec + 1;
						goto fast_ecvt;
					}
#ifdef LD_128
					if (fprefix & _is_longdouble) {
						if (prec < MAXQECVT)
							ndigit = prec + 1;
						else
							ndigit = MAXQECVT;
					}
					else {
#endif /* ifdef LD_128 */
						if (prec < MAXECVT)
							ndigit = prec + 1;
						else
							ndigit = MAXECVT;
#ifdef LD_128
					}
#endif /* ifdef LD_128 */
					/* for prec >= 15, call system ecvt */
#ifdef LD_128
					if (fprefix & _is_longdouble) {
						bp = myqecvt(va_arg(args, QUAD),
						    ndigit, &decpt, &sign, cvtbuf + MAX_RADIX_LEN_IN_INT);
						ndigit = strlen(bp);
					} else {
#endif
#ifdef _THREAD_SAFE
						ecvt_r(va_arg(args, double), ndigit, &decpt, &sign, 
                                                            cvtbuf + MAX_RADIX_LEN_IN_INT, CVTBUFSIZE);
                                                bp = cvtbuf + MAX_RADIX_LEN_IN_INT;
#else 
						bp = ecvt(va_arg(args, double), ndigit, &decpt, &sign);
#endif /* _THREAD_SAFE */
						ndigit = strlen(bp);
						if ((*bp == '0') && (decpt == 0))
							decpt = 1;  /* _doprnt expect *decpt=1 for value 0 */ 
#ifdef LD_128
					}
#endif
					goto ecvt_merge;
fast_ecvt:
#ifdef LD_128
					if (fprefix & _is_longdouble) {
						bp = myqecvt(va_arg(args, QUAD),
						    ndigit, &decpt, &sign, cvtbuf + MAX_RADIX_LEN_IN_INT);
					} else {
#endif
						bp = myecvt(va_arg(args, double), &ndigit, &decpt, &sign ,
						    ibuf + MAX_RADIX_LEN_IN_INT, cvtbuf);
#ifdef LD_128
					}
#endif

ecvt_merge:
					nb = ndigit;
					if (*bp > '9') 
						goto INF_NaN_output;
					goto e_output;

	/* %g or %G */
				} else {    /* must be g or G format */
                                /* for g format, we call ecvt first. 
                                   Then, based on the result, it will do the e or f formating
                                 */
					if (prec < 0) {
                                        /* set default value */
						ndigit = 6;
						prec = 6;
						goto fast_gcvt;
					} else if (prec == 0) {
						ndigit = 1;
						prec = 1;
						goto fast_gcvt;
					} else if (prec < 16) {
						ndigit = prec;
						goto fast_gcvt;
					} else if (prec <= MAXECVT) 
						ndigit = prec;
					else 
						ndigit = MAXECVT;
#ifdef LD_128
					if (fprefix & _is_longdouble) {
						bp = myqecvt(va_arg(args, QUAD),
						    min(prec, MAXQECVT), &decpt, &sign, cvtbuf + MAX_RADIX_LEN_IN_INT);
						ndigit = strlen(bp);
					} else {
#endif
#ifdef _THREAD_SAFE
						ecvt_r(va_arg(args, double), ndigit, &decpt, &sign, 
                                                            cvtbuf + MAX_RADIX_LEN_IN_INT, CVTBUFSIZE);
                                                bp = cvtbuf + MAX_RADIX_LEN_IN_INT;
#else 
						bp = ecvt(va_arg(args, double), ndigit, &decpt, &sign);
#endif /* _THREAD_SAFE */
						ndigit = strlen(bp);
						if ((*bp == '0') && (decpt == 0))
							decpt = 1;  /* _doprnt expect *decpt=1 for value 0 */ 
#ifdef LD_128
					}
#endif
					goto gcvt_merge;

fast_gcvt:
#ifdef LD_128
					if (fprefix & _is_longdouble) {
						bp = myqecvt(va_arg(args, QUAD), 
						    min(prec, MAXQECVT), &decpt, &sign, cvtbuf + MAX_RADIX_LEN_IN_INT);
						ndigit = strlen(bp);
					} else {
#endif
						bp = myecvt(va_arg(args, double), &ndigit, &decpt, &sign,
						    ibuf + MAX_RADIX_LEN_IN_INT, cvtbuf);
#ifdef LD_128
					}
#endif

gcvt_merge:

					nb = ndigit;
					if (*bp > '9') 
						goto INF_NaN_output;

					/* decpt - 1 is the value of the exponent */
					if ( (decpt < -3) || (decpt > prec) ) {
						fcode = fcode - 2;  /* set exponent symbol, (e|E) = (g|G) - 2 */
						if (fprefix & _is_sharp) {
							prec = prec - 1;  /* make its meaning same as e format */

						} else {
							/* remove trailing zeros for g format */
							for (i = nb - 1; i > 0; i--) {
								if ( *(bp + i) != '0') 
/* code folded from here */
	break;
/* unfolding */
							}
							nb = i + 1;
							prec = i;
						}
						goto e_output;

					} else {
						if (fprefix & _is_sharp) {
							/* make prec's meaning same as f format */
							/* prec >= decpt at this point */
							prec = prec - decpt;
						} else {
							/* remove trailing zeros in fractional part for g format */
							if (decpt <= 0) { /* 0.00012345000 and 0.12345000 */
								for (i = nb - 1; i > 0; i--) {
/* code folded from here */
	if ( *(bp + i) != '0') 
		break;
/* unfolding */
								}
								nb = i + 1;
								prec = nb - decpt;
							} else if (decpt < nb) { /* 123.45000 */
								for (i = nb - 1; i >= decpt; i--) {
/* code folded from here */
	if ( *(bp + i) != '0') 
		break;
/* unfolding */
								}
								nb = i + 1;
								prec = nb - decpt;
							} else {  /* 12345000000.000 and 12345.000  */
								prec = 0;
							}
						}

						goto f_output;
					}

				}

e_output:
				/* put in a decimal point */
				if (prec != 0 || (fprefix & _is_sharp) ) {
					radlen = sys_radlen;
					k = radlen;
					nb += k;
					*(bp - radlen) = *bp;
					bp = bp - radlen;
					for (k = 1; k <= radlen; k++)
						*(bp + k) = radchr[k-1];
				} else 
					radlen = 0;

				rzero = prec + 1 + radlen - nb;
				rzero = ~(rzero >> 31) & rzero; /* zero it if negative */

				/* create exponent suffix */
				/* if(dval != 0.0) */ { /* assume decpt is alread 0 */

					int	n;
					char	*p;

					expbuf[0] = fcode;  /* put e or E into buf */
					n = decpt - 1;	/* exponent value */

					if (n < 0) {
						n = -n;
						expbuf[1] = '-';
					} else 
						expbuf[1] = '+';

					if (n >= 100) {
						int	i;
						i = n / 100;
						expbuf[2] = i + '0';
						i = n - i * 100;
						p = (char *)(a99 + i);
						expbuf[3] = *p;
						expbuf[4] = *(p + 1);
						explen = 5;
					} else {
						p = (char *)(a99 + n);
						expbuf[2] = *p;
						expbuf[3] = *(p + 1);
						explen = 4;
					}
				}

				if (!(fprefix &	 _complicate)) {
					/* ---- fast path for simple e format ----
	fast path for simple format with at most  
	width parameter specified.
      */

					*(bp - 1) = '-';
					bp = bp - sign;
					nb = nb + sign;
					i = explen + nb;
					if (width > i) {
						padlen = width - i;
						PUT1(' ', padlen, iop);
						count += width;
					} else 
						count += i;
					PUT0(bp, nb, iop);
					PUT0(expbuf, explen, iop);
					continue;
				}
				/* ------- end of fast path for e format ------- */


				k = rzero + explen;
				if (fprefix & _is_minus) {
					/* create prefix sign character */
					if (sign) { 
						*--bp = '-'; 
						nb++; 
					} else if (fprefix & _is_plus) { 
						*--bp = '+'; 
						nb++; 
					} else if (fprefix & _is_blank) { 
						*--bp = ' '; 
						nb++; 
					}

					PUT0(bp, nb, iop);
					patchr = '0';
					PUT1(patchr, rzero, iop);
					PUT0(expbuf, explen, iop);  /* explen always > 1 */

					k += nb;
					width = width - k;
					/* same as if(width < 0) width = 0;  */
					width = ~(width >> 31) & width;
					count += k;
					count += width;

					/* Blanks on the right if required */
					patchr = ' ';
					PUT1(patchr, width, iop);
				} else {
					if (fprefix & _is_zero)	 {
						/* create prefix sign character */
						if (sign) { 
							XPUTC('-', iop); 
							k++; 
						} else if (fprefix & _is_plus) { 
							XPUTC('+', iop); 
							k++; 
						} else if (fprefix & _is_blank) { 
							XPUTC(' ', iop); 
							k++; 
						}
						k += nb;
						width = width - k;
						/* same as if(width < 0) width = 0;  */
						width = ~(width >> 31) & width;
						count += k;
						count += width;
						/* zeros on the left if required */
						patchr = '0';
						PUT1(patchr, width, iop);
					} else {
						/* create prefix sign character */
						if (sign) { 
							*--bp = '-'; 
							nb++; 
						} else if (fprefix & _is_plus) { 
							*--bp = '+'; 
							nb++; 
						} else if (fprefix & _is_blank) { 
							*--bp = ' '; 
							nb++; 
						}
						k += nb;
						width = width - k;
						/* same as if(width < 0) width = 0;  */
						width = ~(width >> 31) & width;
						count += k;
						count += width;
						/* Blanks on the left if required */
						patchr = ' ';
						PUT1(patchr, width, iop);
					}

					PUT0(bp, nb, iop);
					patchr = '0';
					PUT1(patchr, rzero, iop);
					PUT0(expbuf, explen, iop);

				}

				continue;

f_output:
				if (!(fprefix &	 _complicate) && ((unsigned)decpt < nb)) {
					/* ---- fast path for simple f format ----
	fast path for simple format with at most 
	width parameter specified.
	This part only handles value with radix char inside the
	value string, e.g. 123.45 
      */
					radlen = sys_radlen;
					rzero = prec - (nb - decpt);
					/* insert radchr into value string */
					nb += radlen;
					for (j = 0; j < decpt; j++) 
						*(bp + j - radlen) = *(bp + j);
					for (j = 0; j < radlen; j++) 
						*(bp + decpt - 1 + j) = radchr[j];
					bp = bp - radlen;
					if (decpt == 0) {
						*--bp = '0';
						nb++;
					}
					*(bp - 1) = '-';
					bp = bp - sign;
					nb = nb + sign;
					if (rzero > 0) {
						i = nb + rzero;
						if (width > i) {
							padlen = width - i;
							PUT1(' ', padlen, iop);
							count += width;
						} else 
							count += i;
						PUT0(bp, nb, iop);
						PUT1('0', rzero, iop);
					} else {
						if (width > nb) {
							padlen = width - nb;
							PUT1(' ', padlen, iop);
							count += width;
						} else 
							count += nb;
						PUT0(bp, nb, iop);
					}
					continue;
				}
				/* ------- end of fast path for f format ------- */

				/* get radix character */
				if (prec != 0 || (fprefix & _is_sharp) ) {
					radlen = sys_radlen;
				} else 
					radlen = 0;

				if (sign) {
					signlen = 1; 
					signchr = '-';
				} else if (fprefix & _is_plus) {
					signlen = 1; 
					signchr = '+';
				} else if (fprefix & _is_blank) {
					signlen = 1; 
					signchr = ' ';
				} else 
					signlen = 0;

				if (decpt <= 0) { /* 0.000012345 and 0.12345 */
					rzero = prec - nb + decpt;
					rzero = ~(rzero >> 31) & rzero;	 /* zero it if negative */
					len_out_minus_pad = signlen + 1 + radlen + prec;
					lenpad = width - len_out_minus_pad;
					lenpad = ~(lenpad >> 31) & lenpad;  /* zero it if negative */
					count += lenpad + len_out_minus_pad;

					if (fprefix & _is_minus) {
						if (signlen) 
							XPUTC(signchr, iop);
					} else if (fprefix & _is_zero) {
						if (signlen) 
							XPUTC(signchr, iop);
						PUT1('0', lenpad, iop);
						lenpad = 0;
					} else {
						PUT1(' ', lenpad, iop);
						if (signlen) 
							XPUTC(signchr, iop);
						lenpad = 0;
					}

					XPUTC('0', iop);
					PUT0(radchr, radlen, iop);
					PUT1('0', -decpt, iop);
					PUT0(bp, nb, iop);
				} else if (decpt < nb) {  /*  123.45  */
					ndigit_fraction_part = nb - decpt;
					rzero = prec - ndigit_fraction_part;
					rzero = ~(rzero >> 31) & rzero; /* zero it if negative */
					if (fprefix & _is_quote) {
						strncpy(buf_quote, bp, decpt);
						p = insert_thousands_sep(buf_quote, buf_quote + decpt);
						for (k = 0; k < radlen; k++) 
							*p++ = radchr[k];
						/* copy digits in fractional part */
						for (k = decpt; k < nb; k++) 
							*p++ = bp[k];
						bp = buf_quote;
						nb = p - buf_quote;
					} else {
						/* insert radchr into value string */
						nb += radlen;
						for (j = 0; j < decpt; j++) 
							*(bp + j - radlen) = *(bp + j);
						for (j = 0; j < radlen; j++) 
							*(bp + decpt - 1 + j) = radchr[j];
						bp = bp - radlen;
					}

					len_out_minus_pad = signlen + nb + rzero;
					lenpad = width - len_out_minus_pad;
					lenpad = ~(lenpad >> 31) & lenpad;  /* zero it if negative */
					count += lenpad + len_out_minus_pad;

					if (fprefix & _is_minus) {
						if (signlen) 
							XPUTC(signchr, iop);
					} else if (fprefix & _is_zero) {
						if (signlen) 
							XPUTC(signchr, iop);
						PUT1('0', lenpad, iop);
						lenpad = 0;
					} else {
						PUT1(' ', lenpad, iop);
						if (signlen) 
							XPUTC(signchr, iop);
						lenpad = 0;
					}
					PUT0(bp, nb, iop);
				} else {   /* 123450000000.000000 and 12345.000000 */

					if (fprefix & _is_quote) {
						strncpy(buf_quote, bp, nb);
						for (i = nb; i < decpt; i++) 
							buf_quote[i] = '0';
						p = insert_thousands_sep(buf_quote, buf_quote + decpt);
						bp = buf_quote;
						nb = p - buf_quote;
						tzero = 0;
					} else {
						tzero = decpt - nb;
					}
					rzero = prec;
					len_out_minus_pad = signlen + nb + tzero + radlen + rzero;
					lenpad = width - len_out_minus_pad;
					lenpad = ~(lenpad >> 31) & lenpad;  /* zero it if negative */
					count += lenpad + len_out_minus_pad;

					if (fprefix & _is_minus) {
						if (signlen) 
							XPUTC(signchr, iop);
					} else if (fprefix & _is_zero) {
						if (signlen) 
							XPUTC(signchr, iop);
						PUT1('0', lenpad, iop);
						lenpad = 0;
					} else {
						PUT1(' ', lenpad, iop);
						if (signlen) 
							XPUTC(signchr, iop);
						lenpad = 0;
					}
					PUT0(bp, nb, iop);
					PUT1('0', tzero, iop);
					PUT0(radchr, radlen, iop);
				}

				PUT1('0', rzero, iop);
				PUT1(' ', lenpad, iop);
				continue;


INF_NaN_output:
				if (sign) {
					*--bp = '-';
					nb++;
				} else if (fprefix & _is_plus) {
					*--bp = '+';
					nb++;
				} else if (fprefix & _is_blank) {
					*--bp = ' ';
					nb++;
				}

				lenpad = width - nb;
				lenpad = ~(lenpad >> 31) & lenpad; /* zero it if negative */

				if (fprefix & _is_minus) {
					PUT0(bp, nb, iop);
					PUT1(' ', lenpad, iop);
				} else {
					PUT1(' ', lenpad, iop);
					PUT0(bp, nb, iop);
				}
				count += lenpad + nb;

				continue;

			} /* --------------- end of float ------------------ */
			else {	/* must be % n $ or some undefined format */
				switch (fcode) {
				case 'n':
					if (fprefix & _is_half)
						*(va_arg(args, short * )) = (short)count;
					else if (fprefix & _is_longlong)
						*(va_arg(args, long long * )) = (long long)count;
					else if (fprefix & _is_long)
						*(va_arg(args, long * )) = (long)count;
					else
						*(va_arg(args, int * )) = (int)count;
					continue;
				case '$':
					/* try to reorder the format and arguments */
					if (reorder(anchor, oargs, &nformat, &nargs, &argdata)) {
						RFREE(nargs);
						RFREE(nformat);
						/* commit xputc macro vars to their home
			       location in memory
			      */
						XPUTC_FINISH(iop);
						return(EOF);
					}
					format = nformat;
					args = nargs;
					goto new_format;
				case '\0':  /* unexpected end of format, return error */
					RFREE(nargs);
					RFREE(nformat);
					/* commit xputc macro vars to their home
			   location in memory
			 */
					XPUTC_FINISH(iop);
					return(EOF);
					/*   case '%':	 use default to process % format  */
				default:  /* unrecognized format */
					cval = fcode;
					goto c_merge;
				}
			}
			/* --------- end of % n $ and undefinded format --------- */
		} /*   ---------- end of fmt_letter --------------------------- */

		else if (flags & _is_prefix) {
prefix_indicator:
			fprefix |= _complicate;
			do {
				fprefix |= flags;
				fcode = *format++;
				flags = fmt_flags[fcode];
			} while (flags & _is_prefix);
			if (flags & _is_format) 
				goto fmt_letter;
			else if (flags & _is_width) 
				goto width_indicator;
			else if (flags & _is_length) 
				goto length_indicator;
			else {
				/* bad format */
				if ( fcode == '\0') 
					goto error_return;
				else {
					format--;
					goto new_format;
				}
			}
		} 
		    /*	-------------  end of processing prefix	  ----------- */

		else if (flags & _is_width) {
width_indicator:
			fprefix |= _set_width;
			if (flags & _is_digit) {
				width = fcode - '0';
				while (ISDIGIT(fcode = *format++)) {
					width = fcode - '0' + width * 10;
				}
			} else if (flags & _is_star) {
				width = va_arg(args, int);
				if (width < 0) {
				        fprefix |= _complicate;
					width = -width;
					fprefix |= _is_minus;
				}
				fcode = *format++;
			}
			if ( fcode == '.') {
				fprefix |= _complicate;
				fcode = *format++;
				if (fcode == '*') {
					prec = va_arg(args, int);
					if (prec < 0) 
						prec = -1;
					fcode = *format++;
				} else if (ISDIGIT(fcode)) {
					prec = fcode - '0';
					while (ISDIGIT(fcode = *format++)) {
						prec = fcode - '0' + prec * 10;
					}
				} else 
					prec = 0;
			}
			flags = fmt_flags[fcode];
			if (flags & _is_format) 
				goto fmt_letter;
			else if (flags & _is_length) 
				goto length_indicator;
			else {
				/* bad format */
				if ( fcode == '\0') 
					goto error_return;
				else {
					format--;
					goto new_format;
				}
			}
		} 
		    /*	------------------  end of processing width  ----------------- */
		else if (flags & _is_length)  {
length_indicator:
			if (fcode == 'l') {
				fcode = *format++; /* advance past l */
				if (fcode == 'l' || fcode == 'L') {
					fprefix |= _is_longlong;
					fcode = *format++;
					fprefix |= _complicate;
				} else {
					fprefix |= _is_long;
				}
			} else if (fcode == 'L') {
				fcode = *format++; /* advance past L */
				if (fcode == 'l' || fcode == 'L') {
					fprefix |= _is_longlong;
					fcode = *format++;
					fprefix |= _complicate;

				} else {
					fprefix |= _is_long;
#ifdef LD_128
					fprefix |= _is_longdouble;
					fprefix |= _complicate;
#endif
				}
			} else if (fcode == 'w') {
				fprefix |= _is_wide;
				fcode = *format++;
				fprefix |= _complicate;
			} else if (fcode == 'h') {
				fprefix |= _is_half;
				fcode = *format++;
				fprefix |= _complicate;
			} else { /* default mode */
				fprefix |= _is_long;
			}
			flags = fmt_flags[fcode];
			if (flags & _is_format) 
				goto fmt_letter;
			else {
				/* bad format */
				if ( fcode == '\0') 
					goto error_return;
				else {
					cval = fcode;
					goto c_merge;
				}
			}
		} /*  ------------------  end of processing length  ----------------- */

		else { /* invalid format string, process a new one */
			if ( fcode == '\0') 
				goto error_return;
			format--;   /* bad format, back one char to start over */
			goto new_format;
		}
	}
	/* commit xputc macro vars to their home location in memory */
	XPUTC_FINISH(iop);
	return(ferror(iop) ? EOF : count);
error_return:
        RFREE(mbbuf);
        RFREE(nargs);
        RFREE(nformat);
	XPUTC_FINISH(iop);
	return(EOF);

}



/*	argnum: common code for reorder, width.
 *		Originally design for PTM p36289
 *		to support XPG3 standards.
 */

static int	
argnum(char **prt, int *rcode, arg_data *argdata)
{
	int	arg_index, i;
	char	fcode;

	*rcode = 0;

	for (arg_index = 0; ISDIGIT(fcode = *((*prt)++)); )
		arg_index = arg_index * 10 + fcode - '0';
	if (arg_index > max_arg)
		max_arg = arg_index;
	if (arg_index > size_last_alloc) {
		i = size_last_alloc;
		/* Want to add enough space in increments of 10 to accomodate
		 * size[arg_index].
		 * eg: size_last_alloc=20, arg_index=42
		 *     want size_last_alloc to be 50.
		 *     42 + ( 10 - (42 % 10) )
		 *     42 + ( 10 - 2 )		= 50
		 */
		size_last_alloc = arg_index + (10 - (arg_index % 10));
		size = (int *) realloc (size, size_last_alloc * sizeof(int));
		if (size == NULL) {
			errno = ENOMEM;
			*rcode = -1;
			return -1;
		}

		/* Make sure no holes are in format string
		   (checked later) */
		for (; i < size_last_alloc; i++)
			size[i] = 0;
	}

	arg_index--;	/* the arrays are zero based */
	return(arg_index);
}



/*	width: Used for reorder to handle width and precision.
 *		Originally design for PTM p36289 to support
 *		XPG3 standards.
 */

static int	
width(char **prt, char **new_prt, int *rc, arg_data *argdata)
{
	char	*sav_prt;
	int	index;

	*rc = 0;

	if (**prt == '*' && ISDIGIT( *(*prt + 1))) {
		*(++(*new_prt)) = *((*prt)++);
		sav_prt = *prt;
		while ( ISDIGIT(*((*prt)++)))
                      ;
		*((*prt)--);
		if ( **prt == '$') {
			*prt = sav_prt;
			index = argnum(prt, rc, argdata);
			if (*rc == -1)
				return;
			if (arg_count >= order_last_alloc) {
				order_last_alloc += 10;
				order = (int *) realloc (order, order_last_alloc * sizeof(int));
				if (order == NULL) {
					errno = ENOMEM;
					*rc = -1;
					return;
				}
			}
			order[arg_count++] = index;
			size[index] = sizeof(char *);
			tot_size += sizeof(char *);
		} else 
			*rc = -1;
	} else {
		if ( **prt == '*')
			*rc = -1;
		while ( ISDIGIT( **prt))
			*(++(*new_prt)) = *((*prt)++);
	}
}



/*
 *	Reorder goes through a format statement containing variably
 *	ordered arguments and reorders the arguments accordingly,
 *	stripping out the x$'s.	 Upon success, reorder() returns 0;
 *	the new format string is returned via the argument "bp_new_fmt",
 *	and the new arg pointer is returned via the argument "bp_new_args".
 *	Upon failure, reorder() returns -1; *bp_new_fmt = *bp_new_args = 0.
 */


static int	
reorder(char *format, va_list args, char **bp_new_fmt,
va_list *bp_new_args, arg_data *argdata)
{
	int	wlflag;
	/* Pargs array keeps pointers to the args in proper order */
	int	**pargs;

	/* New_args is the new args va_list returned to caller */
	char	*new_args;

	/* Arg_index counts number of args (i.e. % occurrences) */
	int	arg_index;

	/* new_fmt is the working pointer in the new format string*/
	/* bp is the beginning pointer to original format string  */
	char	*new_fmt, *bp;

	/* Format code */
	register int	fcode;

	/* Return code */
	int	rcode;

	/* Flags - nonzero if corresponding character is in format */
	int	length;		/* l */
	int	hflag;		/* 64890: h */
	int	Lflag;		/* 64890: L */
	int	llflag;		/* 64890: ll, lL, Ll, or LL */

	/* work variable(s) */
	int	i, sav_index = -1;

	/*
	 *	The format loop interogates all conversion characters to
	 *	determine the order and size of each argument, building the
	 *	size array, calculating the tot_size, and filling in new_fmt.
	 */
	bp = format;
	tot_size = arg_count = max_arg = 0;
	size = order = 0;
	*bp_new_fmt = *bp_new_args = 0;
	pargs = 0;
	rcode = 0;

	*bp_new_fmt = new_fmt = (char *) malloc (strlen (format));
        if(new_fmt == NULL) {
           errno = ENOMEM;
           rcode = -1;
           goto ret;
        }
	for (i = 0; i < strlen (format); i++)
		*(new_fmt + i) = '\0';

	size_last_alloc = 10;
	order_last_alloc = 10;
	size = (int *) malloc (10 * sizeof (int));
	order = (int *) malloc (10 * sizeof (int));
	if (new_fmt == NULL || size == NULL || order == NULL) {
		errno = ENOMEM;
		rcode = -1;
		goto ret;
	}

	/* Make sure no holes are in format string (checked later) */
	for (i = 0; i < size_last_alloc; i++)
		size[i] = 0;
	new_fmt--;
	for ( ; ; ) {
		while ((*++new_fmt = *format) != '\0') {
			if (*format == '%') {
				if (*(format + 1) == '%') {
					*++new_fmt = *++format;
				} else
					break;
			}
			format++;
		}

		fcode = *format;
		if (fcode == '\0') { /* end of format; normal return */
			break;	  /* Now do the get_args loop */
		}
		/*
		 *	% has been found.
		 *	First extract digit$ from format and compute arg_index.
		 *	Next parse the format specification.
		 */
		format++;
		arg_index = argnum(&format, &rcode, argdata);
		if (rcode == -1)
			goto ret;
		sav_index = arg_index;

		wlflag = 0;
		for ( ; ; ) { /* Scan the <flags> */
			switch (fcode = *++new_fmt = *(format++)) {
			case '+':
			case '-':
			case '\'':
			case ' ':
			case '#':
			case 'B':
			case 'N':
			case 'J':
			case '0':
				continue;
			case 'w':
				wlflag++;
				continue;
			}
			*new_fmt-- = '\0';
			format--;
			break;
		}

		/* Scan the field width */
		width(&format, &new_fmt, &rcode, argdata);
		if (rcode == -1 )
			goto ret;

		/* Scan the precision */
		if (*format == '.') {
			*++new_fmt = *format++;
			width(&format, &new_fmt, &rcode, argdata);
			if (rcode == -1 )
				goto ret;
		}

		arg_index = sav_index;

		/* Scan the length modifier */
		length = 0;
		llflag = 0;		/* 64890 long long */
		Lflag = 0;		/* 64890 long long */
		hflag = 0;		/* 64890 long long */
		switch (*format) {
		case 'l':
			*++new_fmt = *format++;	    /* copy & advance past 'l' */
			if (('l' == *format) || ('L' == *format)) { /* 64890 */
				llflag++;	/* indicate got long long */
				*++new_fmt = *format++; /* copy & advance past 'l' 'L' */
			} else
				length++;	/* indicate got 'l' */
			break;
		case 'L':		/* long double == double in this
								 * implementation */
			*++new_fmt = *format++;	    /* copy & advance past 'L' */
			if (('l' == *format) || ('L' == *format)) { /* 64890 */
				llflag++;	/* indicate got long long */
				*++new_fmt = *format++; /* copy & advance past 'l' 'L' */
			} else
				Lflag++;	/* indicate got 'L' */
			break;
		case 'h':
			hflag++;	    /* indicate got 'h' */
			*++new_fmt = *format++;	    /* copy & advance past 'h' */
			break;
		}

		switch (fcode = *++new_fmt = *format++) {

		case 'i':
		case 'd':
		case 'u':
		case 'o':
		case 'X':
		case 'x':
			/* Fetch the argument size */
			if (length || Lflag)	    /* 64890 long long */
				size[arg_index] = sizeof(long);
			else if (llflag)	    /* 64890 long long */
				size[arg_index] = sizeof(long long);
			else if (fcode == 'd' || fcode == 'i')
				size[arg_index] = sizeof(int);
			else
				size[arg_index] = sizeof(unsigned);

			break;
		case 'n':		/* p36963.  64890 long long made
								 * separate. */
			if (llflag)
				size[arg_index] = sizeof(long long int *);
			else if (length || Lflag)
				size[arg_index] = sizeof(long int *);
			else if (hflag)
				size[arg_index] = sizeof(short int *);
			else
				size[arg_index] = sizeof(int *);
			break;

		case 'p':		/* p36961.  64890 long long made
								 * separate. */
			if (llflag)
				;	 /* Support for 64-bit pointers would go
								 * here */
			size[arg_index] = sizeof(void *);
			break;

		case 'E':
		case 'e':
		case 'f':
		case 'G':
		case 'g':
			/* Fetch the size */
#ifdef LD_128
			if (Lflag)
				size [arg_index] = sizeof (long double);
			else
				size [arg_index] = sizeof (double);
#else /* must be _doprnt */
			size [arg_index] = sizeof (double);
#endif /* LD_128 */
			break;

			/* case '%':   */
		default:
			break;

		case 'c':
			if (!wlflag) {
				size [arg_index] = sizeof (int);
				break;
			}
		case 'C':
			size [arg_index] = sizeof (wint_t);
			break;
		case 's':
			if (!wlflag) {
				size [arg_index] = sizeof (char *);
				break;
			}
		case 'S':
			size [arg_index] = sizeof (wchar_t * );
			break;

		case '\0': 
			{ /* unexpected end of format; return error */
				rcode = -1;
				goto ret;
			}

		}
		if (arg_count >= order_last_alloc) {
			order_last_alloc += 10;
			order = (int *) realloc (order, order_last_alloc * sizeof(int));
			if (order == NULL) {
				errno = ENOMEM;
				rcode = -1;
				goto ret;
			}
		}
		order[arg_count++] = arg_index;
		tot_size += size[arg_index];
	}

	/* Get ARGS Loop:  fill in the pargs array...pointers to args. */
	pargs = (int **) malloc (arg_count * sizeof (int *));
	if (pargs == NULL) {
		errno = ENOMEM;
		rcode = -1;
		goto ret;
	}
	for (i = 0, arg_index = 0; i < max_arg; ) {
		/* Make sure no holes are in format string */
		if (size[i] == 0) {
			rcode = -1;
			errno = EINVAL;
			goto ret;
		}
		pargs[i] = (int *) &args[arg_index];
		arg_index += size[i++];
	}

	/* Reorder ARGS Loop:  allocate char *new_args;
			       loop through order array to index into
			       size and pargs and copy XX size bytes
			       from *pargs[i] to new_args */
	{
		int	new_index;	/* index in new_args char array */
		int	arg_no;		/* argument order used to index in size */
		/* and pargs			 */
		int	byte_no;	/* number of bytes of each argument	*/

		new_index = 0;
		*bp_new_args = new_args = (char *) malloc (tot_size);
		if (new_args == NULL) {
			errno = ENOMEM;
			rcode = -1;
			goto ret;
		}
		for (arg_no = 0; arg_no < arg_count; arg_no++) {
			memcpy ((void *)new_args, (void *) pargs[order[arg_no]],
			    (size_t)size[order[arg_no]]);
			new_args += size[order[arg_no]];
		}
ret:
		RFREE(*bp_new_fmt);
		RFREE(order);
		RFREE (size);
		RFREE (pargs);
		return (rcode);
	}
}



static
char	*insert_thousands_sep(char *bp, char *ep)
{
	/* get the thousands sep. from the current locale */
	struct lconv *locptr = localeconv();
	char	*thsnd_ptr = locptr->thousands_sep;
	int	len = strlen(thsnd_ptr);
	char	thousep = -1;	   /* Actually, it's 255 since char is unsignded */
	int	buf_index, i;
	char	*obp = bp;
	char	buf[371];
	char	*bufptr = buf;
	char	*grp_ptr = locptr->grouping;


	/* thousands sep. not use in this locale or no grouping required */
	if (!thsnd_ptr	|| !*thsnd_ptr || (*grp_ptr == -1))
		return (ep);

	buf_index = ep - bp;
	while (1) {
		if (*grp_ptr == -1) {
			for (i = 0; i < buf_index--; i++)
				*bufptr++ = *(bp + buf_index);
			break;
		}
		for (i = 0; i < *grp_ptr && buf_index-- > 0; i++)
			*bufptr++ = *(bp + buf_index);

		if (buf_index > 0) {
			*bufptr++ = thousep;	/* store place keeper */
			ep += len;   /* account for the multibyte separator */
		} else
			break;
		if (*(grp_ptr + 1) != '\0')
			++grp_ptr;
	}

	/* put the string in the caller's buffer in reverse order */
	--bufptr;
	while (buf <= bufptr)
		if (*bufptr == (unsigned char)-1) {
			strcpy(obp, thsnd_ptr);
			obp += len;
			bufptr--;
		}
		else
			*obp++ = *bufptr--;
	return (ep);
}


/* 
  FUNCTION: myecvt
  
  Parameters:
    double odval:  input value to be converted.
    int *ndigit:   number of digits to be converted, the value
                   of *ndigit might change in this routine to
                   reflect the actual number of digits emitted
                   by this function.
    int *decpt:    The byte position of radix character from
                   the beginning of the converted string. A negative
                   value of *decpt indicates the radix character is to
                   the left of the beginning of the converted string.
    int *sign:     The returned value indicating the sign of odval.
                   It is 0 if odval is positive, 1 if odval is negative.
    int *ibuf:     Buffer of converted string.  The returned string is
                   not necessary starting from the beginning position 
                   of this buffer.
                   
 Return:    A pointer to the converted string.

 Remark:  This routine requires 0 < ndigit <= 15  

 Algorithm:

  1. multiply the input float x with a factor such that the
     result float y would contain *ndigit number of digits
     in its integral part.
  2. round y according to the caller's rounding mode.
  3. do the conversion.
     (A) if(y < 2**32) 
	 a. convert y into an integer i with the same numerical value.
	 b. do the conversion just like a %u format.
     (B) if(y >= 2**32)
	 In (A), the digits are generated from the least signicant digit
	 and up.  The algorithm does it in the reverse order.
	 By doing so, we can avoid the expensive division operation for
	 obtaining quotients and remainders needed in (A).  However, it
	 requires a large table of n*10**m and a lookup method, which is
	 based on an index obtained from the exponent bits of the IEEE 
	 representation of the floating point number, to quickly determine
	 the value of n and m.
	 a. extract the 8 bit index expm3ff from the exponent bits of y.
	 b. This index would be sufficient to indicate the following
	    statement for some n and m.
	    (n-1)*10**m < y < (n+1)*10**m.
	 c. table1[table2[expm3ff]] is the value n*10*m.
	    table3 contains the information of n and m.
	    We are ready to generate the most significant digit now.
	    The digit is n and the offset of the position in the buffer
	    is m-18.
	 d. if (y >= n*10**m) y = y - n*10**m;
	    esle y = y - (n-1)*10**m; 
	 e. Now we have got the most significant digit.	 
	    Iterate a thru d until y is 0.
 */

static char	*
myecvt( double odval, int *ndigit, int *decpt, int *sign, 
int *ibuf, char *cvtbuf)
{
	short	n, i, i1, i2, i3, i4;
	double	y, z, dval;
	union { 
		int	i[2];
		double	x; 
	} u;
	short	expm3ff;
	double	log10_2 = 0.3010299956639812;  /* log10(2.0) */
	double	*dp;
	typedef struct { 
		double	h;  
		double	l; 
	} ldouble;
	ldouble * powtable1 = (ldouble * )pow1;
	ldouble * powtable3 = (ldouble * )pow3;
	ldouble q, q2;
	double	two52 = 4503599627370496.0, tmp_two52;
	double	two32 = 4294967296.0;
	double	dsign;
	int	ndigit_int_part, nd;
	char	*buf, *p, *bp;
	double	pm_one[2] = { 
		1.0, -1.0	};

	double	saverm;
	unsigned long	val;
	short	*sp;
	/* end of local variables */

	saverm = __readflm();	/* get the FPSCR to examine the rounding mode */
	buf = (char *)ibuf;	/* set up pointer to buffer to do convertsion */
	*decpt = 0;		/* set default */
	u.x = odval;		/* copy arg to structure to do bit manipulation */
	*sign = (unsigned int)u.i[0] >> 31;

	/* The fast path code will only handle round to nearest mode.
	 * Other rounding modes require manipulation of the FPSCR, which
	 * becomes expensive.
	 */
	if (0 != (0x3 & VALL(saverm)))
		goto ecvt_system_code;

	/* 10**n with n > 23 can not be represented as a double exactly */

	dsign = pm_one[*sign];
	dval = dsign * odval;
	u.i[0] &= 0x7fffffff;

	ibuf[0] = 0x30303030;
	ibuf[1] = 0x30303030;
	ibuf[2] = 0x30303030;
	ibuf[3] = 0x30303030;
	ibuf[4] = 0x30303000;  /* note: end with a null char.
			      It is required if we expect this routine
			      to return a null terminated string.
			      However, if we pass the number of converted
			      digits in *ndigit, it is not necessary to have
			      a null char at the end of the string.
			      The same applies to the following INF and NaN
				      results. */

	if ((odval - odval) != 0.0) { /* INF or NaN */
		p = buf;
		if ( dval == dval ) { /* INF */
			*p++ = 'I';
			*p++ = 'N';
			*p++ = 'F';
		} else { /* NaN */
			*p++ = 'N';
			*p++ = 'a';
			*p++ = 'N';
			if (u.i[0] & 0x00080000)
				*p++ = 'Q';
			else
				*p++ = 'S';
		}
		*ndigit = p - buf;
		return(buf);
	}


	n = (u.i[0] >> 20);
	if ( (n & 0x7ff) < 0x0ac) { /* denorm and zero */
		if (u.x == 0.0) {
			*decpt = 1;
			return(buf + 19 - *ndigit);
		}
		/* scale denorm by 10^256 */
		q2.h = powtable3[10].h;
		q2.l = powtable3[10].l;
		mf2x1(&q2, u.x);
		u.x = q2.h;
		n = u.i[0] >> 20;
		*decpt = -256;
	} else {
		q2.h = u.x;
		q2.l = 0.0;
		*decpt = 0;
	}

	/* calculate ndigit_int_part:
	 * 10**ndigit_int_part >= value >= 10**(ndigit_in_part - 1)	  
	 */
	n = n - 0x3ff;
	/* to ensure it is trucated to -INF */
	i1 = (n * log10_2 + 500.0);
	i1 = i1 - 500;
	i2 = (unsigned short)i1 & 0x003f;
	i3 = i1 >> 6;

	if (i3 == 0) {
		z = powtable1[i2+65].h;
	} else {
		z = (powtable1[i2+65].h * powtable3[6+i3].l + 
		    powtable1[i2+65].l * powtable3[6+i3].h ) + 
		    powtable1[i2+65].h * powtable3[6+i3].h;
	}

	if (u.x >= z) {
		ndigit_int_part = i1 + 2;
	} else {
		ndigit_int_part = i1 + 1;
	}

	n = *ndigit - ndigit_int_part;
	*decpt = *decpt + ndigit_int_part;

	/* multiply value by 10**n so that the result would
	 * contain exactly *ndigit digits in integral part
	 * powtable1[i+64] = 10**i
	 */
	i1 = n;
	i2 = (short)i1 & 0x003f;
	i3 = i1 >> 6;

	if (i3 == 0) {
		q.l = powtable1[i2+64].l;
		q.h = powtable1[i2+64].h;
		mf2x2(&q, &q2);

	} else if (i3 > 0) {
		q.l = (powtable1[i2+64].h * powtable3[6+i3].l  + 
		    powtable3[6+i3].h * powtable1[i2+64].l);
		q.h =  powtable3[6+i3].h * powtable1[i2+64].h + q.l;
		q.l = q.h - powtable3[6+i3].h * powtable1[i2+64].h;

		q.h = powtable1[i2+64].h;
		q.l = powtable1[i2+64].l;
		mf2x2(&q, &powtable3[6+i3]);
		mf2x2(&q, &q2);
	} else {
		i3++;
		i2 = i2 - 64;
		q.l = (powtable1[i2+64].h * powtable3[6+i3].l  + 
		    powtable3[6+i3].h * powtable1[i2+64].l);
		q.h =  powtable3[6+i3].h * powtable1[i2+64].h + q.l;
		q.l = q.h - powtable3[6+i3].h * powtable1[i2+64].h;

		q.h = powtable1[i2+64].h;
		q.l = powtable1[i2+64].l;
		mf2x2(&q, &powtable3[6+i3]);
		mf2x2(&q, &q2);

	}


	if (q.h < two32) {
		/* round to an integer according to user's rounding mode */
		tmp_two52 = dsign * two52;

		q.h = q.h * dsign + tmp_two52;
		val = VALL(q.h);
		sp = (short *)buf + 9;
		p = (char *)sp;
		while ((unsigned)val >= 100) {
			long	val_new;
			int	i;
			val_new = (unsigned)val / 100;
			i = (unsigned)val - (unsigned)val_new * 100;
			*--sp = a99[i];
			val = (unsigned)val_new;
		}
		*--sp = a99[val];
		bp = (char *)sp + ((unsigned)val < 10);
		/* 0.9 with *ndigit=1 would round to 1.0.
		 * In this case, *decpt needs to be adjusted.
		 */
		*decpt += (p - bp - *ndigit);

		return(bp);
	} 
	    else { /* q.h must be less than two52 since we limit *ndigit to less than 16 */
		/* round the value to an integral double according to caller's rounding mode */
		tmp_two52 = dsign * two52;
		q2.h = q.h * dsign + tmp_two52;
		q2.l = (q.h * dsign - (q2.h - tmp_two52)) + dsign * q.l;
		q2.h = ((q2.h + q2.l) - tmp_two52);
		u.x = dsign * q2.h;

		if (u.x == powtable1[*ndigit + 64].h) {
			buf[0] = '1';
			/* buf[1] = '\0';  not necessary */
			*decpt += 1;
			*ndigit = 1;
			return(buf);
		} else 
			expm3ff = (u.i[0] >> 17) - 0x1ff8;
	}

	while (u.x >= 1.0) {
		i3 = (unsigned short)table2[expm3ff];
		y = u.x - table1[i3];
		if (y < 0.0) {
			i3--;
			u.x = u.x - table1[i3];
			i3 = i3 << 1;
			buf[(unsigned short)table3[i3+1]] = table3[i3];
		} else {
			u.x = y;
			i3 = i3 << 1;
			buf[(unsigned short)table3[i3+1]] = table3[i3];
		}
		expm3ff = (u.i[0] >> 17) - 0x1ff8;
	}

	return (buf + 19 - *ndigit);

	/* When in the course of human events it becomes necessary to give
	 * up on the fast path code, we use the system ecvt function.  The
	 * interface to myecvt is somewhat different from ecvt in that
	 * myecvt must return *ndigit set to the actual number of 
	 * characters produced; we do this with a call to strlen.
	 * Also, for a value of 0.0 myecvt returns *decpt set to 1,
	 * ecvt returns 0, so we must fix this up.
	 */

ecvt_system_code:
#ifdef _THREAD_SAFE
        ecvt_r(odval, *ndigit, decpt, sign,
               cvtbuf + MAX_RADIX_LEN_IN_INT, CVTBUFSIZE);
        bp = cvtbuf + MAX_RADIX_LEN_IN_INT;
#else
        bp = ecvt(odval, *ndigit, decpt, sign);
#endif /* _THREAD_SAFE */

	*ndigit = strlen(bp);
	if ((*bp == '0') && (*decpt == 0))
		*decpt = 1; /* _doprnt expect *decpt=1 for value 0 */
	return bp;
}

/* 
  FUNCTION: myfcvt
  
  Parameters:
    double odval:  input value to be converted.
    int *ndigit:   number of digits to be converted, the value
                   of *ndigit might change in this routine to
                   reflect the actual number of digits emitted
                   by this function.
    int *decpt:    The byte position of radix character from
                   the beginning of the converted string. A negative
                   value of *decpt indicates the radix character is to
                   the left of the beginning of the converted string.
    int *sign:     The returned value indicating the sign of odval.
                   It is 0 if odval is positive, 1 if odval is negative.
    int *ibuf:     Buffer of converted string.  The returned string is
                   not necessary starting from the beginning position 
                   of this buffer.  This buffer is used when the input
                   value can be handled with the fast path in the routine.
                   Otherwise, the bigger buffer cvtbuf will be used.
    int *cvtbuf:   A bigger buffer for the converted string.  The buffer
                   is used when calling the system fcvt.
  Return:    A pointer to the converted string.
*/

static char	*
myfcvt( double odval, 
int *ndigit, 
int *decpt, 
int *sign, 
int *ibuf, 
char *cvtbuf
)
{
	short	n, i, i1, i2, i3, i4;
	double	y, z, dval;
	union { 
		int	i[2];
		double	x; 
	} u;
	short	expm3ff;
	double	log10_2 = 0.3010299956639812;  /* log10(2.0) */
	double	*dp;
	typedef struct { 
		double	h;  
		double	l; 
	} ldouble;
	ldouble * powtable1 = (ldouble * )pow1;
	ldouble * powtable3 = (ldouble * )pow3;
	ldouble q, q2;
	double	two52 = 4503599627370496.0, tmp_two52;
	double	two32 = 4294967296.0;
	double	dsign;
	int	ndigit_int_part, nd;
	char	*buf, *p, *bp;
	double	pm_one[2] = { 
		1.0, -1.0	};
	int	oval;

	double	saverm;
	unsigned long	val;
	short	*sp;

	buf = (char *)ibuf;
	*decpt = 0;

	u.x = odval;
	*sign = (unsigned int)u.i[0] >> 31;

	/* fast path for easy numbers */
	if (*ndigit < 16) {
		saverm = __setflm(0.0);
		/* scale by 10^(*ndigit) */
		dsign = pm_one[*sign];
		q.h = powtable1[*ndigit + 64].h;
		dval = dsign * odval;
		q.h = dval;
		q.l = 0;
		__setflm(saverm);
		/* For directed rounding modes you have to diddle with the
		 * rounding modes depending on the sign of the argument.
		 * This is very expensive to do, so for directed rounding
		 * modes we'll give up and use the system fcvt.
		 */
		if (0 != (0x2 & VALL(saverm)))
			goto fcvt_directed_rm;
		mf2x1(&q, powtable1[*ndigit + 64].h);

		if (q.h < two32) {
			tmp_two52 = dsign * two52;
			/* round to an integer according to user's rounding mode */
			q.h = q.h * dsign + tmp_two52;
			oval = val = VALL(q.h);
			sp = (short *)buf + 9;
			p = (char *)sp;
			while ( (unsigned) val >= 100) {
				long	val_new;
				int	i;
				val_new = (unsigned)val / 100;
				i = (unsigned)val - (unsigned)val_new * 100;
				*--sp = a99[i];
				val = (unsigned)val_new;
			}
			*--sp = a99[val];
			bp = (char *)sp + ((unsigned)val < 10);
			nd = p - bp;
			*decpt = nd - *ndigit;
			*ndigit = nd;
			return(bp);
		}
	}
fcvt_directed_rm:
#ifdef _THREAD_SAFE
        fcvt_r(odval, *ndigit, decpt, sign,
               cvtbuf, CVTBUFSIZE);
        bp = cvtbuf;
#else
        bp = fcvt(odval, *ndigit, decpt, sign);
        /* copy the result to our buf */
        strcpy(cvtbuf, bp);
        bp = cvtbuf; 
#endif /* _THREAD_SAFE */

	while (*bp == '0') {
		bp++;
		(*decpt)--;
	}
	*ndigit = strlen(bp);
	return bp;
}

#ifdef LD_128
/* 
  FUNCTION:     myqecvt
  
  Parameters:
    QUAD ldval:    input value to be converted.
    int *ndigit:   number of digits to be converted.
    int *decpt:    The byte position of radix character from
                   the beginning of the converted string. A negative
                   value of *decpt indicates the radix character is to
                   the left of the beginning of the converted string.
    int *sign:     The returned value indicating the sign of odval.
                   It is 0 if odval is positive, 1 if odval is negative.
    int *cvtbuf:   The buffer to store the converted string.  
  Return:    A pointer to the converted string.
*/
static char	*
myqecvt( QUAD ldval, int ndigit, int *decpt, int *sign,
char *cvtbuf)
{

	char	*bp, *p;

	/* _qecvt() doesn't really understand long double; it expects
      * a QUAD struct defined above.  The union below is used to
      * move back and forth between a long double and a QUAD.  The
      * 'd' and 'u' structures can also be used to pick part individual
      * parts of the long double if necessary.
      */
	union {
		long double	l;
		QUAD q;
		struct { 
			double	hi, lo; 
		} d;
		struct { 
			unsigned int	a, b, c, d; 
		} u;
	} ldarg;

	ldarg.q = ldval;
	TS_LOCK(&_qecvt_rmutex);
	bp = _qecvt(ldarg.q, ndigit, decpt, sign);
	strcpy(cvtbuf, bp);
	TS_UNLOCK(&_qecvt_rmutex);
	bp = &cvtbuf[0];
	if ((ldarg.l == 0.0L) && (*decpt == 0))
		*decpt = 1;      /* _doprnt expect *decpt=1 for value 0 */	
	return bp;
}

/* 
  FUNCTION: myqfcvt
  
  Parameters:
    QUAD ldval:    input value to be converted.
    int *ndigit:   number of digits to be converted.
    int *decpt:    The byte position of radix character from
                   the beginning of the converted string. A negative
                   value of *decpt indicates the radix character is to
                   the left of the beginning of the converted string.
    int *sign:     The returned value indicating the sign of odval.
                   It is 0 if odval is positive, 1 if odval is negative.
    int *cvtbuf:   The buffer to store the converted string.  
  Return:    A pointer to the converted string.
*/

static char	*
myqfcvt( QUAD ldval, int prec, int *decpt, int *sign, 
char *cvtbuf)
{

	char	*bp, *p;

	union {
		long double	l;
		QUAD q;
		struct { 
			double	hi, lo; 
		} d;
		struct { 
			unsigned int	a, b, c, d; 
		} u;
	} ldarg;
	ldarg.q = ldval;
	TS_LOCK(&_qecvt_rmutex);
	bp = _qfcvt(ldarg.q, prec, decpt, sign);
	strcpy(cvtbuf, bp);
	bp = &cvtbuf[0];
	TS_UNLOCK(&_qecvt_rmutex);

	while (*bp == '0') {
		bp++;
		(*decpt)--;
	}
	/* Hack for behavior difference between _qfcvt and fcvt --
      * for numbers less than 1.0, fcvt returns as many leading zeros
      * as is necessary for *decpt to be 0.  The formatting code
      * works OK except for the case where no digits are output.
      */
	if ((*bp == '\0')) {
		*decpt = -prec;	/* _doprnt expect *decpt=1 for value 0 */	
	}
	return bp;
}
#endif /* LD_128 */
