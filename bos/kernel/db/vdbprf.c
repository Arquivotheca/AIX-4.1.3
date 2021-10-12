static char sccsid[] = "@(#)82        1.28  src/bos/kernel/db/vdbprf.c, sysdb, bos41B, 412_41B_sync 12/6/94 13:55:09";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: _doprnt, printn, sprintf, printf
 *
 * ORIGINS: 27 83
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
 * LEVEL 1,  5 Years Bull Confidential Information
*/

#include <sys/types.h>
#include <sys/lldebug.h>
#include <sys/errids.h>
#include <sys/intr.h>
#include <sys/systemcfg.h>
#include <stdarg.h>
#include "dbdebug.h"			/* dbterm defines */
#include <sys/inline.h>

#ifdef _POWER_MP
#define MP_PRINTBUFSIZE	MAXCPU * PRINTBUFSIZE
#else
#define MP_PRINTBUFSIZE	 PRINTBUFSIZE
#endif

extern unsigned char printbuf[];	/* circular buffer */
extern int dbg_avail;
extern int dbg_pinned;

ulong dbterm = 0;			/* these 2 globals help us with async */
char  debabend = OUT;

/*
 * In case console is off,
 * panicstr contains argument to last call to panic.
 */
#define PANICBUFSIZ     80
char    *panicstr,panicbuf[PANICBUFSIZ];
int     dump_rv = 0;		/* global value set when debugger wants a dump*/
				/* This is set here so that it will be pinned */

struct panicerr {                         /* error log description */
        struct  err_rec0        perr;
		char		printbuf[PANICBUFSIZ];
                char            panicstr[PANICBUFSIZ];
} panic_log = { ERRID_KERNEL_PANIC,"PANIC", 0};

/* 
 * Maximum length of string that can be printed by this printf.
 */
#define MAX_PRINTF_LEN	200

/*                                                                   
 * EXTERNAL PROCEDURES CALLED: 
 */  

extern dputchar(), d_ttyput();
extern void d_ttyclose();

static char *printn();

#define isdigit(c) (((c)>='0')&((c)<='9'))

/*
 * Scaled down version of C Library printf and sprintf.
 * Only %s %u %d (==%u) %o %x %X %D are recognized.
 * Also supports 0 or blank filling and specified length,
 * e.g., %08x means print 8 characters, 0 filled.
 */

/*
 * _doprnt external version, with no range checking.
 */

 int
_doprnt(fmt, adx, b)
 register char *fmt, *b;
 /* VARARGS */
 va_list adx;
{
	int len, c;
	char fill, *s, *b0 = b;

	for(;;) {
		while((c = *fmt++) != '%') {
			if(c == '\0') {
				*b = 0;
				return b-b0;
			}
			*b++ = c;
		}
		c = *fmt++;
		/* Setup the fill character. */
		fill = ' ';
		if ((c==' ')||(c=='0')) {
			fill = (char)c;
			c = *fmt++;
		};
		/* Setup the desired output length */
		len = 0;
		if (c == '*') {		/* dynamic length */
		    len = va_arg(adx, int);
		    c = *fmt++;
		} else
		    for (; isdigit(c); ) {
			len = (len*10)+((int)((char)c-'0'));
			c = *fmt++;
		    }
		/* Now we're at the format specifier. */
		switch(c) {
		case 'D': case 'd': case 'u': case 'o': case 'x': case 'X':
		        /* Use -1 (i.e. 0xffffffff) as ending pointer */
			b = printn(va_arg(adx, int), c, b, fill, len, (char *)-1);
			continue;
		case 'c':
			*b++ = va_arg(adx, int);
			continue;
		case 's':
			s = (char *)va_arg(adx, char *);
			for (len -= strlen(s); --len >= 0; *b++ = ' ');
			while (*b++ = *s++);
			--b;
			continue;
		}
	}
}

/*
 * _doprnt internal version (local_doprnt)
 *
 * Only allows up to MAX_PRINTF_LEN bytes in the final constructed string, 
 * although it assumes it can run past that length by at least one byte for
 * placement of a terminating null character.
 */

static int
local_doprnt(fmt, adx, b)
 register char *fmt, *b;
 /* VARARGS */
 va_list adx;
{
	int len, c;
	char fill, *s, *b0 = b, *b_end = b+MAX_PRINTF_LEN;

	for(;b < b_end;) {			/* Don't go past b_end */
		while((c = *fmt++) != '%') {
			if((c == '\0') || (b >= b_end)) {
				*b = 0;
				return b-b0;
			}
			*b++ = c;
		}
		c = *fmt++;
		/* Setup the fill character. */
		fill = ' ';
		if ((c==' ')||(c=='0')) {
			fill = (char)c;
			c = *fmt++;
		};
		/* Setup the desired output length */
		len = 0;
		if (c == '*') {		/* dynamic length */
		    len = va_arg(adx, int);
		    c = *fmt++;
		} else
		    for (; isdigit(c); ) {
			len = (len*10)+((int)((char)c-'0'));
			c = *fmt++;
		    }
		/* Now we're at the format specifier. */
		switch(c) {
		case 'D': case 'd': case 'u': case 'o': case 'x': case 'X':
			b = printn(va_arg(adx, int), c, b, fill, len, b_end);
			continue;
		case 'c':
			*b++ = va_arg(adx, int);
			continue;
		case 's':
			s = (char *)va_arg(adx, char *);
			for (len -= strlen(s); 
				(--len >= 0) && (b < b_end);
				*b++ = ' ');

			/* Note: next test lets b get equal to b_end+1 */
			/* to allow for "--b" afterwards. */
			while ((b <= b_end) && (*b++ = *s++));

			--b;
			continue;
		}
	}
	/* If we get here, we must null-terminate the string, then return */
	/* the length of the data (which should be MAX_PRINTF_LEN). */
	*b = '\0';
	return b-b0;
}

/*
 * NAME: printn
 *
 * FUNCTION: convert a number to a printable string
 *
 * RETURNS:  pointer to the end of the passed buffer
 *
 * NOTE:
 *    This routine has been modified to accept an "end-of-buffer" pointer,
 *    to allow the length of the data finally printed to be limited to avoid
 *    stepping on other memory.
 */
 
static
 char *
printn(n, ch, p, f, l, p_end)
long n;					/* Number to convert		*/
char ch;				/* Format specifier		*/
char *p;				/* buffer ptr.			*/
char f;					/* Fill character		*/
int l;					/* # of characters to output	*/
char *p_end;				/* End of buffer ptr.		*/
{
	int i=0, b, nd, c;
	int	flag;
	int	plmax;
	char d[12];

	/* Set the base */
	switch(ch) {
		case 'o':	b = 8;
				plmax = 11;
				break;
		case 'x': case 'X': b = 16;
				plmax = 8;
				break;
		default:	b=10;
				plmax = 10;
	}

	c = 1;
	flag = n < 0;
	if (flag)
		n = (-n);
	if (flag && b==10) {
		flag = 0;
		*p++ = '-';
		l--;
	}
	for (;i<plmax;i++) {
		nd = n%b;
		if (flag) {
			nd = (b - 1) - nd + c;
			if (nd >= b) {
				nd -= b;
				c = 1;
			} else
				c = 0;
		}
		d[i] = nd;
		n = n/b;
		if ((n==0) && (flag==0))
			break;
	}
	if (i==plmax)
		i--;
	/* Put in any fill characters. */
	for (l--; (l>i) && (p<p_end); l--)
		*p++ = f;
	/* Move converted # to the output. */
	for (;(i>=0) && (p<p_end);i--) {
		*p++ = "0123456789ABCDEF"[d[i]];
	}
	return p; 
}

/*
 * NAME: sprintf
 *                                                                    
 * FUNCTION: 
 *   Build a string using the supplied format.
 *   This works like the sprintf library routine using the
 *   limited Kernel printf formatting.
 *                                                                    
 * RETURN VALUE:  none
 *
 * NOTE:
 *   Since this routine uses _doprnt, there is no limit to the size of the
 *   buffer on which it will work.
 */
 void
sprintf(char *buf, char *fmt, ...)
/* VARARGS */
{
	va_list vap;

	va_start(vap, fmt);
	(void) _doprnt(fmt, vap, buf);
	va_end(vap);
}
 
static char endofbuf[] = "<- end_of_buffer        ";
static int pbufidx = 0;

/*
 * NAME: logbuf
 *                                                                    
 * FUNCTION: Writes a string to the printbuf.
 *                                                                    
 * NOTES:
 *   Saves the string into a circular buffer so that it can be examined
 *   if display data is lost (see printf() below).
 *
 * DATA STRUCTURES:  Maintains a global circular buffer in printbuf.
 *
 * RETURN VALUE:  none
 */
void
logbuf(len, msg)
int len;
char *msg;
{
	int i, j;

	/* copy into circular buffer */
	for (; len; --len)
	{
		/* when we reach the end of the buffer, wrap around */
		if (pbufidx == MP_PRINTBUFSIZE)
			pbufidx = 0;

		printbuf[pbufidx++] = *msg++;
	}

	/* terminate buffer entry with marker (so entry is visible) */
	j = pbufidx;			/* don't disturb pbufidx */
	for (i = 0; i < sizeof(endofbuf) - 1; i++)
	{
		if (j == MP_PRINTBUFSIZE)
			j = 0;

		printbuf[j++] = endofbuf[i];
	}
}

/*
 * NAME: printf
 *                                                                    
 * FUNCTION: This is the low-level debugger's printf.
 *                                                                    
 * NOTES:
 *   There is a Kernel printf also found probably in errlog.c.
 *   There is an applications' printf also.
 *   User requests (ie not debugger) are entered into a circular buffer, this
 *   allows the buffer's contents to be examined if displayed data has been
 *   lost.  The most notable example of this is the filename and linenumber
 *   displayed during an assert.  Debugger printfs are not stored to avoid
 *   cluttering up the buffer (also since the debugger starting up would
 *   trash most of the buffer).
 *   Finally, this routine will only print up to a maximum of MAX_PRINTF_LEN
 *   bytes, since the data structure it uses is static.  This limit is
 *   enforced in local_doprnt().
 *
 * DATA STRUCTURES:  Maintains a global circular buffer in printbuf.
 *
 * RETURN VALUE:  none
 */
#ifdef _POWER_MP
static volatile struct db_lock printbuff_lock = {0,0,-1};
#endif /* _POWER_MP */
 void
printf(char *fmt, ...)
/* VARARGS */
{
	static struct {
		int csmt;
		int len;
		char buf[MAX_PRINTF_LEN+4];	/* +4 allows for overflow */
	} eb;


	va_list vap;
	register int l, i_opened_it=0;
	register char *p;
	register int i, j, old_intr;

	va_start(vap, fmt);

	/* If the debugger is not available or not yet pinned, return immediately */
	if (((dbg_avail & LLDB_MASK) == NO_LLDB) || !dbg_pinned)
		return;

	old_intr = i_disable(INTMAX);

#ifdef _POWER_MP
	while (!db_lock_try(&printbuff_lock,cpunb));
#endif
	l = local_doprnt(fmt, vap, eb.buf);
	va_end(vap);

	/* If called from the kernel, log data to circular buffer */
	if (debabend == OUT)
		logbuf(l, eb.buf);

	/* print the stuff. */
#ifdef _KDB
	if (__kdb() && debabend == OUT) {
		kdb_k_printf(l, eb.buf);
#ifdef _POWER_MP
		db_unlock(&printbuff_lock);
#endif
		i_enable(old_intr);
		return;
	}
#endif /* _KDB */
	p = eb.buf;

	while(l--) {		/* use debugger version of putchar */
		if(debabend==OUT && !i_opened_it) {
			/* we were called from kernel*/
			d_ttyopen(dbterm & TTY_PORT); 	/* open port */
			i_opened_it = 1;
		}
		d_ttyput(*p++); 
	}
	if(debabend==OUT && i_opened_it) 
		d_ttyclose(dbterm & TTY_PORT); 	/* close port*/
	i_enable(old_intr);
#ifdef _POWER_MP
	db_unlock(&printbuff_lock);
#endif
}

static
void
get_last_msg(char *buf)
{
	int i,idx;

	/* If the debugger is not available or not yet pinned, give the caller */
	/* a single null character and then return immediately */
	if (((dbg_avail & LLDB_MASK) == NO_LLDB) || !dbg_pinned) {
		*buf = '\0';
		return;
	}
#ifdef _POWER_MP
	while (!db_lock_try(&printbuff_lock,cpunb));
#endif
	idx = pbufidx;

	/* Back up one string in the printbuf */
	for (i=0; i<PANICBUFSIZ-1; i++) {
		/* when we reach the beginning of the buffer, wrap around */
		if (--idx == -1)
			idx += MP_PRINTBUFSIZE;

		if (printbuf[idx] == '\n' && i != 0) {
			/* get rid of the newline; wrap if necessary */
			if (++idx == MP_PRINTBUFSIZE)
				idx = 0;
			break;
		}
	}

	/* Copy the string into the caller's buffer */
	for (i=0; idx != pbufidx; i++) {
		buf[i] = printbuf[idx];
		/* when we reach the end of the buffer, wrap around */
		if (++idx == MP_PRINTBUFSIZE)
			idx = 0;
	}
	buf[i] = '\0';
#ifdef _POWER_MP
	db_unlock(&printbuff_lock);
#endif
}

/*
 * Panic is called on unresolvable fatal errors.
 * It syncs, prints "panic: mesg" and then calls the debugger.
 * The debugger may return a value that says take a dump.
 */
char nosync = 0;
panic(char *s)
{
	char *p;
	int i;
	void dmp_do(int);
	void brkpoint(void);
	int  i_disable(int);

	/* if this is the first panic, record it in panicbuf.
	 * otherwise it is a double panic: just print string and
	 * try to call debugger (preserving first panicstr).
	 */
	if (panicstr == NULL) {
		if (s != NULL) {
			p = s;
			for(i=0;i<PANICBUFSIZ;i++) {
				panicbuf[i] = *p;
				if(*p++ == (char)NULL)
					break;
			}
		}
	}
	else
		panic_log.perr.error_id = ERRID_DOUBLE_PANIC;
	panicstr = s;

        strncpy(panic_log.panicstr,s,PANICBUFSIZ);
	get_last_msg(panic_log.printbuf);
        errsave(&panic_log, sizeof(panic_log));

	/* 
	 * print panic string 
	 */
	printf("%s\n",s);

	/* disable ints forever */
	(void) i_disable(INTMAX);

	while (1) {
		/* 
		 * if debugger loaded call it
		 */
		/* if(dbg_inited) */
			brkpoint();
		/* else dump_rv=DUMPRET;i.e.if(!dbg_inited) take a dump */

		/* 
	 	* if dump has been requested then take it
	 	* dump_rv is global - set by debugger before returning
	 	* (or above)
	 	*/
		if (dump_rv == DUMPRET) {
			/* dmp_do() disables ints,
			 * effectively pinning the stack
			 */
			dmp_do(1 /* primary dump device */);
			dump_rv = 0;
		}
	
	}

}
