static char sccsid[] = "@(#)57	1.26  src/bos/usr/bin/tail/tail.c, cmdscan, bos412, 9446B 11/15/94 20:11:57";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27, 18
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
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 *
 * tail.c,v $ $Revision: 2.6.1.2 $ (OSF) $Date: 91/05/03 14:23:01 $
 */

/*
 *
 *	Tail writes from a file beginning at a specified point to the
 *	end of file.  Points can be specified by:
 *
 *		+/- [n]l	for n lines
 *		+/- [n]b	for n blocks
 *		+/- [n]c	for n bytes
 *		+/- [n]k	for n k-block (1024)
 *		+/- [n]m	for n multi-byte characters
 *
 *	The -f option tell you to loop forever looking for updates.  The
 *	assumption is the file is growing.
 *
 *	The -r option will copy output from end of file to specified point
 *	in reverse order.
 *
 * 	-c number 
 *		This option is in bytes.  It is an integer 
 *		number with +/- prepended to the number. + means
 * 		relative to the beginning of the file, - means
 * 		relative to the end of the file. No sign means 
 *		same as -.  This is same as +/-c obsolescent version. 
 *                                                                    
 *	-n number
 * 		number is measure in lines.  This is same as +/-l
 *		obsolescent version.
 */                                                                   

#define _ILS_MACROS

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <locale.h>
#include "tail_msg.h"
static nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd,MS_TAIL,Num, Str)

#define BLOCK   512     /* multiplier for b suffix */
#define KBLOCK  1024    /* multiplier for k suffix */
#define CHUNK   4096    /* how much we are willing to read at once */

#define DEFAULT_NUMBER "10"	/* default number used in case none specified */

#define BUFFSIZE (LINE_MAX*10)		/* POSIX.2 says this is default */

#define	NEWLINE	'\n'

typedef int     bool;
#define NO      0	
#define YES     1

#include <stdlib.h>

static void dowidechars();	/* for tail -m */
static void dowidefromstart();
static void dowidefromend();
static void setcount(char *arg);
static void usage();
static void dofollow();
static void mb_dofollow();
static void dotail();
static void dorev();
static void dobytes();
static void dolines();

static int cflag = 0;
static int nflag = 0;
static int mflag = 0;
static int bflag = 0;
static int kflag = 0;
static int mexclusive = 0;
static int no_reverse = 0;
static int multibyte = 0;
static bool explicit = NO;	/* kludge to allow Berkeley compatibility */
				/* was n_tail set explicitly? */

static bool    canseek;
static bool    bylines = YES;
static bool    follow  = NO;
static bool    fromtop = NO;
static bool	reverse = NO;
static long    n_tail  = 10L;
static struct stat     statb;
static char    bin[CHUNK + (2*MB_LEN_MAX)]; 
                        /* add X bytes so that in the case of	*
			 * the following unlikely(but possible) *
			 * scenario:				*
			 *	mblen(bin[CHUNK-1], MB_CUR_MAX)	*
			 * mblen will not attempt to read beyond*
			 * array boundaries			*/
static char	inputfile[PATH_MAX];

static FILE *fpinput;
static int mb_cur_max;

main(argc, argv)
int argc; char **argv;
{
    register int err;
    register int rc, file = 0;
    (void) setlocale(LC_ALL,"");
    catd = catopen(MF_TAIL,NL_CAT_LOCALE);

    while (--argc) {
	register char *arg = *++argv;
	switch (*arg) {
	case '+':                               /* always a number */
	    fromtop = YES;
	    setcount(arg);
	    break;
        case '-':                               /* may be number or "-f" */
nextarg: 
	   if (arg[1] == '?') {
		usage();
	   } else if (arg[1] == 'f') {
		follow = 1;
	   } else if (arg[1] == 'r') {		/* ... or even -r  or so on */
		reverse = YES;
		if (!explicit) n_tail = BUFFSIZE;
	   } else if (arg[1] == 'c') {		
		cflag++;
		no_reverse = 1;
	   } else if (arg[1] == 'n') {
		nflag++;
	   } else if (arg[1] == 'm') {
		mflag++;
		no_reverse = 1;
                /* if not in a multibyte locale then don't set this flag
                 * and let the single byte code path be used since it
                 * should be somewhat faster.
                 */
                if (MB_CUR_MAX != 1)
			multibyte++;
	   } else if (arg[1] == 'b') {
		bflag++;
		no_reverse = 1;
	   } else if (arg[1] == 'k') {
		kflag++;
		no_reverse = 1;
	   } else if ((arg[1] == '-') && (arg[2] == '\0')) { /* -- is found */
		arg = *++argv;
		argc--;
		goto file;
	   } else {
		 /* no more flag or "l" flag */
			fromtop = NO;
			setcount(arg);
			break;
	    }
	    if (arg[2] != '\0') {
		arg[1] = '-';
		arg++;
		goto nextarg;
	    }
	    if (*argv[1] == '-' || *argv[1] == '+')
	        break;
	    if (!isdigit((int)*argv[1]) && (cflag || nflag || mflag ||
		     bflag || kflag)) /* no # argument specified by user */
		{
		/* POSIX & XOPEN requires a numeric argument with the -n flag */
		if (nflag)
			usage();
		arg = DEFAULT_NUMBER;
		fromtop = NO;
		setcount(arg);
		}
	    break;
file:
	default:                                /* it's a filename */
	    if (cflag || nflag || mflag || bflag || kflag) {
		fromtop = NO;
		setcount(arg);
		}
	    else {
            	if  (argc > 1){
                	fprintf(stderr, MSGSTR(PRSFILE,
		 	"tail: Can only process one file at a time.\n"));
                	usage();
                }
		strcpy(inputfile, arg);
	    	(void)close(0);
	    	if (open(inputfile, 0) != 0) {
			perror(inputfile);
			exit(1);
	    	}
                file++;
	    }
	    break;
	} /* switch */
	if (mexclusive > 1) {
       		fprintf(stderr, MSGSTR(EXCLOPT,
			 "tail: Options are mutually exclusive.\n"));
		usage();
	}

	/* -r flag can be used with -n option only */
	if (no_reverse || follow) {
	    if (reverse)
		usage();
	}
    } /* while */

    if (!explicit && fromtop)
	n_tail--;
    fstat(0, &statb);
    errno = 0;
    rc = lseek(0, 0L, SEEK_CUR);
    err = errno;
    canseek = (rc != -1 && (statb.st_mode & S_IFMT) != S_IFCHR);
    if (reverse)
	dorev();
    else {
    	dotail();
        /* -f option works with canseek file or FIFO only */
    	if (follow && (canseek || (err==ESPIPE && file))) {
                if (multibyte)
                    mb_dofollow();
                else
		    dofollow();
        }
    }
    exit(0);
/* NOTREACHED */
}

/*
 * NAME: setcount
 *                                                                    
 * FUNCTION:  	Determine the count (+/-) and by what value to do the tail.
 *		n_tail is set to this value.  by_lines for lines else
 *		it is by character.
 *                                                                    
 * RETURN VALUE: void
 *			    
 */  

static void setcount(arg) char *arg; {

    char *ptr;
    int nopref = 0;  	/* if no + or - prefix passed */

    if (cflag || mflag || bflag || kflag || nflag)  {
	ptr = arg;
	if ( (*ptr == '+') || (*ptr == '-') )
		ptr++;
	else
		nopref++;
	while (*ptr != '\0') {
		if (!isdigit((int)*ptr)) {
			usage();
			}
		ptr++;
		}
	if (nflag)
		bylines = 1;
	else
		bylines = 0;
	}
    
    if (!nopref)
    	arg++;
    if (isdigit((int)*arg)) {
	explicit = YES;
	n_tail = 0;
	do n_tail = n_tail * 10 + (*arg++ - '0'); while (isdigit((int)*arg));
    	}
    if (!nflag && !cflag && !mflag && !bflag && !kflag) {
    	while (*arg) {
		switch (*arg++) {
		case 'l':               /* lines */
		    bylines = 1;
		    break;
		case 'b':
		    bylines = 0;        /* blocks - convert to characters */
		    n_tail *= BLOCK;
		    mexclusive++;
		    no_reverse=1;
		    break;
		case 'm':		/* tail exact number of characters */
                    /* if not in a multibyte locale then don't set this flag
                     * and let the single byte code path be used since it's
                     * should be somewhat faster.
                     */
                    if (MB_CUR_MAX != 1)
		    	multibyte = 1;	/* instead of bytes */
		    bylines = 0;	
		    mexclusive++;
		    no_reverse=1;
		    break;
		case 'k':
		    bylines = 0;        /* blocks - convert to characters */
		    n_tail *= KBLOCK;
		    mexclusive++;
		    no_reverse=1;
		    break;
		case 'c':               /* characters */
		    bylines = 0;
		    mexclusive++;
		    no_reverse=1;
		    break;
		case 'f':
		    follow = 1;         /* "-10f" same as "-10 -f" */
		    break;
		case 'r':
		    reverse = 1;	/* "-10r" same as "-10 -r" */
		    if (!explicit)	/* kludge to allow Berkeley compatibility */
			n_tail = BUFFSIZE;
		    break;
		default:
			usage();
		}
    	}
    }
    if (bflag) n_tail *= BLOCK;
    if (kflag) n_tail *= KBLOCK;
    mexclusive = mexclusive + cflag + bflag + mflag + kflag;
    nflag = cflag = bflag = kflag = mflag = 0;
    if (explicit && fromtop && n_tail > 0) --n_tail;
}

/*
 * NAME: dofollow
 *                                                                    
 * FUNCTION: every second write out any changes
 *                                                                    
 * RETURN VALUE: never returns.  Must be killed.
 */  

static void dofollow() {
    register int n;
    
    while ((n = read(0, bin, (unsigned)CHUNK)) != -1) {
        if (n == 0) {
	    sleep((unsigned)1);
	} else {
	    write(1, bin, (unsigned)n);
	}
    }
}

/*
 * NAME: mb_dofollow
 *                                                                    
 * FUNCTION: every second write out any changes
 *                                                                    
 * RETURN VALUE: never returns.  Must be killed.
 */  

static void mb_dofollow() {
    wchar_t wc;
    char *s, *tmpbin;
    int i, n, wclen, leftovers = 0;
    
    tmpbin = bin;
    while ((n = read(0, tmpbin, (unsigned)CHUNK)) != -1) {
        if (n == 0) {
	    sleep((unsigned)1);
	} else {
            s = bin;
            i = 0;
            while (s < (bin + n + leftovers)) {
                if ((wclen = mbtowc(&wc, s, mb_cur_max)) == -1) {
	            if (s + mb_cur_max < bin + n + leftovers) {
                        /* skip invalid multibyte chars */ 
                        s++;
                    } else {
                        /* save leftovers and check again after next read */
                        while (s < (bin + n + leftovers)) {
				*(bin+i) = *s;
                                s++;
                                i++;
                        }
                    }
                } else {
                    putwchar(wc);
                    s += wclen;
                }
            }
            leftovers = i;
            tmpbin = bin + leftovers;
	}
        fflush(0);
    }
}

/*
 * NAME: dotail
 *                                                                    
 * FUNCTION: 	Actually do the tail.  lseek to the starting point.
 *		skip multibyte characters if necessary, and print out the
 *		requested lines.
 *                                                                    
 * RETURN VALUE: none
 *			    
 */  

static void dotail() {
    register int nc = 0;
    char *p = bin;

    if (fromtop) {
	if (n_tail == 0) {
	    /* nothing */
	} else if (multibyte) {
	    dowidechars();
	} else if (bylines) {
	    do {
		if (nc == 0) {
		    if ((nc = read(0, bin, (unsigned)CHUNK)) <= 0) return;
		    p = bin;
		}
		--nc;
	    } while (*p++ != '\n' || --n_tail > 0);
	} else if (canseek) { 
	    lseek(0,n_tail,SEEK_CUR);
	} else {
	    do {
		if ((nc = read(0, bin, (unsigned)CHUNK)) <= 0) return;
	    } while ((n_tail -= nc) > 0); 
	    p = bin + nc + n_tail; 
	    nc = -n_tail;
	}
	if (nc > 0)  write(1, p, (unsigned)nc);
	while ((nc = read(0, bin, (unsigned)CHUNK)) > 0)
	    write(1, bin, (unsigned)nc);

/* from here down are the -number cases */
    } else if (bylines) {	
	dolines();
    } else if (multibyte) {
	dowidechars();
    } else if (canseek) { 
	lseek(0, -n_tail, SEEK_END);
	while ((nc = read(0, bin, (unsigned)CHUNK)) > 0)
            write(1, bin, (unsigned)nc);
    } else 
	    dobytes();  
}

/*
 * NAME: dorev
 *                                                                    
 * FUNCTION:   - output last n_tail lines in reverse order (last first)
 * This function faithfully emulates the Berzerkely -r operation of tail.
 * Including limitations.
 *                                                                    
 * RETURN VALUE:  none
 */  


static void dorev()
{
	static char buf[BUFFSIZE];	/* circular buffer storage area	*/

	register char *p;		/* current location in buffer	*/
	register char *bufend;		/* circular buffer end		*/

	char *last;			/* location of last newline	*/
	int partial;			/* is the buffer full or partial*/
	int lines;			/* number of lines output	*/

	int nc;				/* number of characters read	*/
	int offset = 0;			/* offset into buffer		*/
	long size = BUFFSIZE;		/* size of file/size of buffer	*/

	if (n_tail <= 0) return;

/*
 * read the file, saving the last BUFFSIZE bytes; handle wrapping
 */

	/* if input is seekable, win on large input files */
	if (canseek) {
		size = lseek(0, 0L, SEEK_END);
		if (size > BUFFSIZE) size = BUFFSIZE;
		lseek(0, -size, SEEK_CUR);
	}

	partial = YES;
	while ((nc = read(0, buf + offset, (unsigned)(BUFFSIZE - offset))) > 0) {
		if ((offset += nc) >= BUFFSIZE) {
			partial = NO;
			offset = 0;
		}
	}

	/* force a trailing newline (also ensures buffer is never empty) */
	if (buf[offset==0 ? BUFFSIZE-1 : offset-1] != NEWLINE) {
		buf[offset] = NEWLINE;
		if (++offset >= BUFFSIZE) {
			offset = 0;
			partial = NO;
		}
	}

	/* point to last character in buffer */
	if (offset > 0) bufend = buf + offset - 1;
	else		bufend = &buf[BUFFSIZE-1];

/*
 * output lines in reverse order
 */
	lines = 0;
	p = bufend;

	do {
		last = p;

		/* scan for a newline, handle wrap-around */
		do {
			if (--p < buf) {
				if (partial) {
					write(1, buf, (unsigned)(last-buf+1));
					return;
				}
				p = buf + BUFFSIZE;
			}
		} while (*p != NEWLINE && p != bufend);

		/* found a newline, output collected line */
		if (p < last)
			write(1, p+1, (unsigned)(last-p));
		else {
			write(1, p+1, (unsigned)(&buf[BUFFSIZE]-p));
			write(1, buf, (unsigned)(last-buf+1));
		}
	
	} while (++lines < n_tail && p != bufend); /* up to n_tail lines */
	return;
}
/*
 * NAME: dobytes
 *                                                                    
 * FUNCTION: 
 *
 * Last n_tail chars, no seek.  We use a circular buffer.  If the static bin
 * isn't big enough, we get the space from malloc(), rounding up to the next
 * CHUNK boundary if possible.  PORT WARNING: This routines assumes that long
 * (n_tail) and unsigned int (argument expected by malloc) and the difference
 * between two pointers (argument passed to to read/write) are all the same.
 * Not making this assumption requires a bit of finesse.
 *
 * RETURN VALUE: none
 */
static void dobytes() {
    register char *b;
    register int nc;
    register long m, size;
    register bool isfull;
    if (n_tail <= CHUNK) {
	size = CHUNK;
	b = bin;
    } else {
	size = (n_tail+CHUNK-1) / CHUNK * CHUNK;
	if ((b = malloc((size_t)size)) == NULL) {
	    size = n_tail;
	    if ((b = malloc((size_t)size)) == NULL) {
		fprintf(stderr, MSGSTR(TOOMUCH, "tail: Unable to malloc memory.\n"));
		exit(1);
	    }
	}
    }
    isfull = NO;
    m = 0;
    while ((nc = read(0, b+m, (unsigned)(size-m))) > 0) {
	if ((m += nc) >= size) { 
	    isfull = YES;
	    m = 0;
	}
    }
    if (m >= n_tail) { 
	write(1, b+m-n_tail, (unsigned)(n_tail));
    } else {
	if (isfull) write(1, b+size+m-n_tail, (unsigned)(n_tail-m));
	write(1, b, (unsigned)m);
    }
}

/*
 * Last n_tail lines: the most likely case, and also the hardest.
 * The strategy is to maintain a linked list of buffers, of which all but
 * the last are full.  If the file is seekable, the "last" (partial) buffer
 * is the first one read; its size is adjusted according to the size of the
 * file.
 */

typedef struct Buffer Buffer;
struct Buffer {
    Buffer *next;
    long    lines;
    char    buf[CHUNK];
};
static Buffer *qh = NULL;      /* head of linked list of full buffers */
static Buffer *qp;             /* partial buffer at end of file */
static int     psize;          /* size of partial buffer */

/*
 * NAME: mkbuf
 *                                                                    
 * FUNCTION:  get a buffer.
 *                                                                    
 * RETURN VALUE:  The buffer just malloc'ed is returned.
 *			    
 */  

static Buffer 
*mkbuf() 
{
    register Buffer *q;
    if ((q = (Buffer *)malloc((size_t)sizeof(Buffer))) == NULL) {
	fprintf(stderr, MSGSTR(TOOMUCH, "tail: Unable to malloc memory.\n"));
	exit(1);
    }
    q->lines = 0;
    return (q);
}

/*
 * NAME: rmbuf
 *                                                                    
 * FUNCTION:  Release a buffer.
 *                                                                    
 * RETURN VALUE: none
 */  

static void rmbuf(q) Buffer *q; {
    free((void *)q);
}

/*
 * NAME: nlines
 *                                                                    
 * FUNCTION:  Count the number of lines in a buffer.
 *                                                                    
 * RETURN VALUE:  The number of lines.
 */  

static int nlines(p, n) register char *p; register int n; {
    register int r = 0;
    while (--n >= 0) if (*p++ == '\n') ++r;
    return (r);
}

/*
 * NAME: dolines
 *                                                                    
 * FUNCTION:  	If seek-able file. seek to the tailing point.  Otherwise,
 *		read from the file, keeping a ring buffer of the amount
 *		you want to print at the end.
 *                                                                    
 * RETURN VALUE:  void
 */  

static void dolines() {
    register int nc, nl;
    register long tlines = 0;
    qp = mkbuf();
    if (canseek) {
	long e = lseek(0, 0L, SEEK_END);
	long r = lseek(0, -(e % CHUNK), SEEK_CUR);
	while (psize < e-r) {
	    if ((nc = read(0, &qp->buf[psize], (unsigned)(e-r-psize))) <= 0) {
		fprintf(stderr, MSGSTR(FSHRUNK, "tail: file shrunk!\n")); 
		perror("");
		exit(1);
	    }
	    nl = nlines(&qp->buf[psize], nc);
	    psize += nc;
	    qp->lines += nl;
	    tlines += nl;
	}
	while (r > 0 && tlines <= n_tail) {
	    register int k;
	    register Buffer *temp = mkbuf();
	    temp->next = qh;
	    qh = temp;
	    r = lseek(0, r-CHUNK, SEEK_SET);
	    for (k = 0; k < CHUNK; k += nc) {
		if ((nc = read(0, &qh->buf[k], (unsigned)(CHUNK-k))) <= 0) {
		    fprintf(stderr, MSGSTR(FSHRUNK, "tail: file shrunk!\n"));
		    perror("");
		    exit(1);
		}
	    }
	    tlines += qh->lines = nlines(&qh->buf[0], CHUNK);
	}
	lseek(0, e, SEEK_SET);
    } else {
	register Buffer **qt = &qh;
	while ((nc = read(0, &qp->buf[psize], (unsigned)(CHUNK-psize))) > 0) {
	    nl = nlines(&qp->buf[psize], nc);
	    psize += nc;
	    qp->lines += nl;
	    tlines += nl;
	    while (qh != NULL && tlines > qh->lines + n_tail) {
		register Buffer *temp = qh;
		qh = qh->next;
		if (qh == NULL) qt = &qh;
		tlines -= temp->lines;
		rmbuf(temp);
	    }
	    if (psize >= CHUNK) {
		qp->next = NULL;
		*qt = qp;
		qt = &qp->next;
		qp = mkbuf();
		psize = 0;
	    }
	}
    }
    if (qh != NULL) {
	register char *p = &qh->buf[0];
	while (tlines > n_tail) if (*p++ == '\n') --tlines;
	if (p < &qh->buf[CHUNK]) write(1, p, (unsigned)(&qh->buf[CHUNK]-p));
	while ((qh = qh->next) != NULL) write(1, &qh->buf[0], (unsigned)CHUNK);
	if (psize > 0) write(1, &qp->buf[0], (unsigned)psize);
    } else {
	register char *p = &qp->buf[0];
	while (tlines > n_tail && p - &qp->buf[0] < psize ) 
		if (*p++ == '\n') --tlines;
	psize -= p - &qp->buf[0];
	if (psize > 0) write(1, p, (unsigned)psize);
    }
}

static void
dowidechars()
{
	(void)close(0);
	if((fpinput = fopen(inputfile, "r")) == NULL) {
		perror(inputfile);
		exit(1);
	}

	mb_cur_max = MB_CUR_MAX;

	if (fromtop == YES) {
		dowidefromstart();
	} else {
		dowidefromend();
	}
}

static void
dowidefromstart()
{
	wint_t wc;
	int i;

	fseek(fpinput, 0L, SEEK_SET);
	for(i = 0; i < n_tail; i++) {
		if((wc = fgetwc(fpinput)) == WEOF) {
			/* not that many mb chars in file */
			exit(1);
		}
	}
	while((wc = fgetwc(fpinput)) != WEOF) {
		putwchar(wc);
	}
}

static void
dowidefromend()
{
	char *beginp, *endp;
	int readseg(), excess_chs;
	int bytesread = 0;	/* total # of bytes from eof */
	int sg_mb_cnt;   /* cnt of mb chars in this segment of data */
	int mb_cnt = 0;	   	/* total count of mb chars */
	int bytes_this_seg;	/* total bytes in currect segment */
	wint_t wc;

	/* start at end of file and read data backwards	*
	 * till we locate byte position corresponding to*
	 * the start of the last n_tail multibyte chars	*/
	for(;;) {
		if(bytesread >= statb.st_size)
			break;

		bytes_this_seg = readseg(&beginp, &endp);

		/* count multibyte chars in current segment	*/
		sg_mb_cnt = cnt_mbchars(beginp, endp);

		if(mb_cnt + sg_mb_cnt < n_tail) {

			/* still short of total number	*
			 * of mb chars desired, adjust	*
			 * counters and keep looping	*/

			mb_cnt += sg_mb_cnt;
			bytesread += bytes_this_seg;

		} else {

			/* this segment of data may have contained *
			 * more mb chars than needed; calculate	*
			 * the excess				*/

			excess_chs = sg_mb_cnt - (n_tail-mb_cnt);

			/* if this segment has more mb chars than *
			 * are needed to fill quota of n_tail	*
			 * chars, calculate the excess in bytes	*
			 * and subtract it from the byte counter*/

		bytes_this_seg -= cnt_bytes(beginp, endp, excess_chs);

			bytesread += bytes_this_seg;
			break;
		}
	}

	fseek(fpinput, -bytesread, SEEK_END);
	while((wc = fgetwc(fpinput)) != WEOF) {
		putwchar(wc);
	}
}

#define UNIQUE_CODE_PT		0x3f

/*
 * readseg:	read in a "segment" of data where
 *		"segment" is defined to be the data
 *		between successive occurrences of
 *		characters in the unique code point
 *		range
 */
static int
readseg(beginpp, endpp)
char **beginpp;
char **endpp;
{
	static char *beginp = bin - 1;
	static char *endp;
	static long totalbytes = 0;
	int inputblksz = 0;
	long offset = 0;
	int byteno;

	for(;;) {
		endp = beginp;

		/* if offset from eof = size of file then we're	*
		 * positioned at the beginning of the file and	*
		 * there's no need to search for a char in the	*
		 * unique code point range ... this is also ne-	*
		 * cessary because if n_tail is > the file size	*
		 * (EX. file sz=1000 mb chs & tail -1001m file)	*
		 * and if there are no chars from the uniq c pt	*
		 * range at the top of the file we'll loop for	*
		 * ever in this routine (ie. there's no EOF at	*
		 * the top of a file!)				*/
		if(offset == statb.st_size) {
			beginp = bin;
			*beginpp = beginp;
			*endpp = endp;
			beginp--;
			return(inputblksz);
		}

		for(byteno = 0; beginp >= bin; byteno++, beginp--) {
			if(*beginp <= UNIQUE_CODE_PT) {
				*beginpp = beginp;
				*endpp = endp;
				beginp--;
				totalbytes += (byteno + 1);
				return(byteno + 1);
			}
		}

		/* If there are CHUNK unread bytes left then	*
		 * read CHUNK more, otherwise set inputblksz	*
		 * to number of remaining unread bytes.		*/
		if(totalbytes + CHUNK > statb.st_size) {
			inputblksz = statb.st_size - totalbytes;
		} else {
			inputblksz = CHUNK;
		}

		/* offset from eof of next block to read */
		offset = totalbytes + inputblksz;
		fseek(fpinput, -offset, SEEK_END);
		fread(bin, 1, inputblksz, fpinput);
		beginp = bin + inputblksz - 1;
	}
}

/*
 * cnt_mbchars:	for a pair of char pointers delimiting an
 *		area of data within a buffer, count the
 *		number of legal multibyte characters
 */
static int
cnt_mbchars(beginp, endp)
char *beginp;
char *endp;
{
	int mb_cnt = 0;	/* count of multibyte chars	*/
	int len = 0;	/* length of multibyte char	*/

	while(beginp <= endp) {
		if((len = mblen(beginp, mb_cur_max)) <= 0) {
			/* Either a NULL was returned	*
			 * or valid mb char not found	*
			 * skip a byte & keep trying	*/
			len = 1;
		} else {
			/* found one, increment mb cnt	*
			 * for this segment of data	*/
			mb_cnt++;
		}
		beginp += len;
	}
	return(mb_cnt);
}

/*
 * cnt_bytes:	for a given number of multibyte characters
 *		count the number of bytes
 */
static int
cnt_bytes(beginp, endp, n_mbchars)
char *beginp;
char *endp;
int n_mbchars;	/* number of mb chars for which	we 	*
		 * need the corresponding # of bytes	*/
{
	int mb_cnt = 0;
	int byte_cnt = 0;
	int len;

	while( mb_cnt < n_mbchars && beginp <= endp ) {
		if((len = mblen(beginp, mb_cur_max)) <= 0) {
			len = 1;
		} else {
			mb_cnt++;
		}
		beginp += len;
		byte_cnt += len;
	}
	return(byte_cnt);
}

static void usage()
{
    fprintf(stderr, MSGSTR(USAGE2,"Usage: tail [-f] [-c number|-n number|-m number|-b number|-k number] [file]\n"));
    fprintf(stderr, MSGSTR(USAGE1,"Usage: tail [-r] [-n number] [file]\n"));
    fprintf(stderr, MSGSTR(USAGE,"Usage: tail [+|-[number]][l|b|c|k|m][f] [file]\n"));
    exit(2);
}

