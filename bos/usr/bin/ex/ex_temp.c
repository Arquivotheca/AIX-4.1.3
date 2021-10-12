static char sccsid [] = "@(#)27  1.20  src/bos/usr/bin/ex/ex_temp.c, cmdedit, bos41B, 9504A 12/19/94 11:46:48";
/*
 * COMPONENT_NAME: (CMDEDIT) ex_temp.c
 *
 * FUNCTIONS: KILLreg, REGblk, TSYNC, YANKline, YANKreg, blkio, cleanup,
 * fileinit, getREG, getblock, getline, kshift, mapreg, notpart, partreg,
 * putline, putreg, rbflush, regbuf, regio, shread, synctmp, tflush, tlaste
 *
 * ORIGINS: 3, 10, 13, 18, 26, 27
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
 * 
 * Copyright (c) 1981 Regents of the University of California
 * 
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

/* AIX security enhancement */
#if defined(TVI)
#include <sys/stat.h>
#include <memory.h>
#include <sys/shm.h>
#endif
/* TCSEC Division C Class C2 */
#include "ex.h"
#include "ex_temp.h"
#include "ex_vis.h"
#include "ex_tty.h"
#include <time.h>
#include <sys/file.h>

/*
 * Editor temporary file routines.
 * Very similar to those of ed, except uses 2 input buffers.
 */
#define READ	0
#define WRITE	1

/* AIX security enhancement */
#if defined(TVI)
char	*tfname;
char	*rfname;
int	havetmp;
#endif
/* TCSEC Division C Class C2 */

static void blkio(short, wchar_t *, int (*)(int, char *, unsigned int));
static int shread(void);
static int getREG(void);
static int REGblk(void);
static void kshift(void);
static void rbflush(void);

static	wchar_t	*getblock(line, int);

/* AIX security enhancement */
#if !defined(TVI)
/* size of pathnames set to _POSIX_PATH_MAX */
static	char	tfname[256];
static	char	rfname[256];
static	int	havetmp;
short tfile = -1;
#endif
/* TCSEC Division C Class C2 */

/* AIX security enhancement */
#if defined(TVI)
int	tfile = -1;
int	rfile = -1;
int	tfile_size = 128*BUFBYTES;
int	rfile_size = 128*BUFBYTES;
struct	shmid_ds tf_struct,*tfbuf = &tf_struct;
struct	shmid_ds rf_struct,*rfbuf = &rf_struct;
#endif
/* TCSEC Division C Class C2 */

/* AIX security enhancement */
#if !defined(TVI)
static	short	rfile = -1;
/*
 * Vi insists on putting one 'NULL' line in the buffer (see fixzero()).
 * This means that for the most common case (entering vi with a filename),
 * the tempfile gets created, the NULL line is added, and then the file
 * you're editing gets loaded.  Since the tmpfile is not 'empty', it gets
 * deleted and recreated.  If the /tmp directory is big, this may take 
 * some time, so we make a special check for this case.
 *	if (tline == INCRMT * (HBLKS+2))
 *		return;
 */

void fileinit(void)
{
	register char *p;
	register int i, j;
	struct stat stbuf;

	if (tline == INCRMT * (HBLKS+2))
		return;
	cleanup((short)0);
	close(tfile);
	tline = INCRMT * (HBLKS+2);
	blocks[0] = HBLKS;
	blocks[1] = HBLKS+1;
	blocks[2] = -1;
	dirtcnt = 0;
	iblock = -1;
	iblock2 = -1;
	oblock = -1;
	strcpy(tfname, svalue(DIRECTORY));
	if (stat(tfname, &stbuf)) {
dumbness:
		if (setexit() == 0)
			filioerr(tfname);
		else
			putNFL();
		cleanup((short)1);
		catclose(catd);
		exit(1);
	}
	if ((stbuf.st_mode & S_IFMT) != S_IFDIR) {
		errno = ENOTDIR;
		goto dumbness;
	}
	ichanged = 0;
	ichang2 = 0;
	ignore(strcat(tfname, "/ExXXXXX"));
	for (p = strend(tfname), i = 5, j = getpid(); i > 0; i--, j /= 10)
		*--p = j % 10 | '0';
	tfile = creat(tfname, 0600);
	if (tfile < 0)
		goto dumbness;
	{
		extern stilinc; 	/* see below */
		stilinc = 0;
	}
	havetmp = 1;
	close(tfile);
	tfile = open(tfname, 2);
	if (tfile < 0)
		goto dumbness;
}
#endif

#if defined(TVI)
/* called from ex.c once for each file to be edited	*/
/* so that only one shmem buffer is set up at the same time.	*/
void fileinit(void)
{




	if (tline == INCRMT * (HBLKS+2))
		return;
	cleanup((short)0);
	tline = INCRMT * (HBLKS+2);
	blocks[0] = HBLKS;
	blocks[1] = HBLKS+1;
	blocks[2] = -1;
	dirtcnt = 0;
	iblock = -1;
	iblock2 = -1;
	oblock = -1;
	ichanged = 0;
	ichang2 = 0;
	
	if ((tfile = shmget(IPC_PRIVATE,tfile_size,IPC_EXCL|IPC_CREAT|S_IRUSR|S_IWUSR)) == -1)
		perror("1tvi");
	else {	/* Get a base address for the buffer */
		if ((tfname = shmat(tfile,0,0)) == (char *) -1)
			perror("2tvi");
		else	/* Put the statistics in a local statbuf */
			if ((shmctl(tfile, IPC_STAT,tfbuf)) == -1)
				perror("3tvi");
	}
	{
		extern stilinc; 	/* see below */
		stilinc = 0;
	}
	havetmp = 1;
}
#endif

#if !defined(TVI)
void cleanup(short all)
{
	extern char termtype[];
	if (all) {
		putpad(exit_ca_mode);
		flush();
		resetterm();
		normtty--;
	}
	if (havetmp)
		unlink(tfname);
	havetmp = 0;
	if (all && rfile >= 0) {
		unlink(rfname);
		close(rfile);
		rfile = -1;
	}
	if (all == 1)
		{
		/* close the message catalog */
		catclose(catd);
		exit(exit_status);
		}
}
#endif

#if defined(TVI)
/* Be sure present buffer is detached then removed prior to next call to fileinit */
void cleanup(short all)
{
	extern char termtype[];
	if (all) {
		putpad(exit_ca_mode);
		flush();
		resetterm();
		normtty--;
	}
	if (tfname != NULL) {
		if (shmdt(tfname) == -1)
			perror("9tvi");
		if ((shmctl(tfile,IPC_RMID,(struct shmid_ds *)tfname)) == -1)
			perror("10tvi");
	}
	if (all && rfile != -1) {
		if (shmdt(rfname) == -1) {
			perror("11tvi");
			if ((shmctl(rfile, IPC_RMID, (struct shmid_ds *)rfname)) == -1)
				perror("12tvi");
			else
				rfile = -1;
		}
	}
	if (all == 1)
		{
		/* close the message catalog */
		catclose(catd);
		exit(exit_status);
		}
}
#endif
/* TCSEC Division C class C2 */

void getline(line tl)
{
	register wchar_t *bp, *lp;
	register int nl;

	lp = linebuf;
	bp = getblock(tl, READ);
	nl = nleft;
	tl &= ~OFFMSK;
	while (*lp++ = *bp++)
		if (--nl == 0) {
			bp = getblock(tl += INCRMT, READ);
			nl = nleft;
		}
}

line putline(void)
{
	register wchar_t *bp, *lp;
	register int nl;
	line tl;

	dirtcnt++;
	lp = linebuf;
	change();
	tl = tline;
	bp = getblock(tl, WRITE);
	nl = nleft;
	tl &= ~OFFMSK;
	while (*bp = *lp++) {
		if (*bp++ == '\n') {
			*--bp = 0;
			linebp = lp;
			break;
		}
		if (--nl == 0) {
			bp = getblock(tl += INCRMT, WRITE);
			nl = nleft;
		}
	}
	tl = tline;
	tline += (((lp - linebuf) + BNDRY - 1) >> SHFT) & 077776;
	return (tl);
}


/* 
 * atl is in terms of characters, not bytes.
 */
static wchar_t * getblock(line atl, int iof)
{
	register int bno, off;    /* off is in terms of characters */

	bno = (atl >> OFFBTS) & BLKMSK;
	off = (atl << SHFT) & LBTMSK;
	if (bno >= NMBLKS)
		error(MSGSTR(M_194, " Tmp file too large"), DUMMY_INT);
	nleft = INCRMT - off;
	if (bno == iblock) {
		ichanged |= iof;
		hitin2 = 0;
		return (ibuff + off);
	}
	if (bno == iblock2) {
		ichang2 |= iof;
		hitin2 = 1;
		return (ibuff2 + off);
	}
	if (bno == oblock)
		return (obuff + off);
	if (iof == READ) {
		if (hitin2 == 0) {
			if (ichang2) {
     	                   blkio(iblock2, ibuff2, write);
			}
			ichang2 = 0;
			iblock2 = bno;
			blkio((short)bno, ibuff2, read);
			hitin2 = 1;
			return (ibuff2 + off);
		}
		hitin2 = 0;
		if (ichanged) {
			blkio(iblock, ibuff, write);
		}
		ichanged = 0;
		iblock = bno;
		blkio((short)bno, ibuff, read);
		return (ibuff + off);
	}
	if (oblock >= 0) {
			blkio(oblock, obuff, write);
	}
	oblock = bno;
	return (obuff + off);
}

#define INCORB	64
static wchar_t	incorb[INCORB+1][INCRMT];
#define pagrnd(a)	((char *)(((int)a)&~(BUFBYTES-1)))
static int	stilinc = 0;	/* up to here not written yet */

/* AIX security enhancement */
#if !defined(TVI)
static void blkio(short b, wchar_t *buf, int (*iofcn)(int, char *, unsigned int))
{

	if (b < INCORB) {
		if (iofcn == read) {
			copy(buf, pagrnd(incorb[b+1]), BUFBYTES);
			return;
		}
		copy(pagrnd(incorb[b+1]), buf, BUFBYTES);
		if (laste) {
			if (b >= stilinc)
				stilinc = b + 1;
			return;
		}
	} else if (stilinc)
		tflush();
	lseek(tfile, (long) (unsigned) b * BUFBYTES, 0);
	if ((*iofcn)(tfile, (char *)buf, BUFBYTES) != BUFBYTES)
		filioerr(tfname);
}
#else

static void blkio(short b, wchar_t *buf, int (*iofcn)(int, char *, unsigned int))
{
	/* Care for old style memory buffers used to reduce file access */

	if (b < INCORB) {
		if (iofcn == read) {
			copy(buf, pagrnd(incorb[b+1]), BUFBYTES);
			return;
		}
		copy(pagrnd(incorb[b+1]), buf, BUFBYTES);
		if (laste) {
			if (b >= stilinc)
				stilinc = b + 1;
			return;
		}
	} else if (stilinc)	/* clear old style memory buffers and reset stilinc */
		tflush();
	{
		/* Data to and from shared memory in BUFBYTE chunks */
		/* Defect 47973 - tvi will stop source processing when the */
		/* ^M is encountered.  tvi uses shared memory versus temp  */
		/* files for security reasons, and that is why the memccpy */
		/* is used here versus the memcpy.  Because memccpy requires*/
		/* an end character, and the \r was causing 47973, the char  */
		/* ^O was selected (0x0f) in an attempt to choose a char that*/
		/* is the least likely to be encountered.		    */  
		char 	*p;
		int endchar = 0x0f;

		p = tfname + (long)(unsigned) b * BUFBYTES;
		if (iofcn == read)
			tvicopy(buf, p, endchar, BUFBYTES);
		if (iofcn == write) {
			if ((b * BUFBYTES) < (tfile_size - BUFBYTES))
				tvicopy(p, buf, endchar, BUFBYTES);
			else {
				tfbuf->shm_segsz = tfile_size * 2;
				if ((shmctl(tfile, SHM_SIZE, tfbuf)) == -1)
					perror("4tvi");
				else {
					tfile_size *= 2;
					tvicopy(p, buf, endchar, BUFBYTES);
				}
			}
		}
	}
}
#endif
/* TCSEC Division C Class C2 */

void tlaste(void)
{

	if (stilinc)
		dirtcnt = 0;
}

/* AIX security enhancement */
#if defined(TVI)
void	tflush(void)
{
	/* This used to move the 64 2K incore buffers to shared memory as a block */

	stilinc = 0;
}

void TSYNC(void)
{
}

void synctmp(void)
{
}
#else

void tflush(void)
{
	int i = stilinc;

	stilinc = 0;
	lseek(tfile, (long) 0, 0);
	if (write(tfile, pagrnd(incorb[1]), i * BUFBYTES) != (i * BUFBYTES))
		filioerr(tfname);
}
/*
 * Synchronize the state of the temporary file in case
 * a crash occurs.
 */
void synctmp(void)
{
	register int cnt;
	register line *a;
	register short *bp;
	if (stilinc)
		return;
	if (dol == zero)
		return;
	/*
	 * In theory, we need to encrypt iblock and iblock2 before writing
	 * them out, as well as oblock, but in practice ichanged and ichang2
	 * can never be set, so this isn't really needed.  Likewise, the
	 * code in getblock above for iblock+iblock2 isn't needed.
	 */
	if (ichanged)
		blkio(iblock, ibuff, write);

	ichanged = 0;
	if (ichang2)
		blkio(iblock2, ibuff2, write);
	ichang2 = 0;
	if (oblock != -1)
		blkio(oblock, obuff, write);
	time(&H.Time);
	uid = getuid();
	*zero = (line) H.Time;
	for (a = zero, bp = blocks; a <= dol; a += BUFBYTES / sizeof *a, bp++) {
		if (*bp < 0) {
			tline = (tline + OFFMSK) &~ OFFMSK;
			*bp = ((tline >> OFFBTS) & BLKMSK);
			if (*bp > NMBLKS)
				error(MSGSTR(M_194, " Tmp file too large"), DUMMY_INT);
			tline += INCRMT;
			oblock = *bp + 1;
			bp[1] = -1;
		}
		lseek(tfile, (long) (unsigned) *bp * BUFBYTES, 0);
		cnt = ((dol - a) + 2) * sizeof (line);
		if (cnt > BUFBYTES)
			cnt = BUFBYTES;
		if (write(tfile, (char *) a, cnt) != cnt) {
oops:
			*zero = 0;
			filioerr(tfname);
		}
		*zero = 0;
	}
	flines = lineDOL();
	lseek(tfile, 0l, 0);
	if (write(tfile, (char *) &H, sizeof H) != sizeof H)
		goto oops;
}

void TSYNC(void)
{

	if (dirtcnt > MAXDIRT) {	/* mjm: 12 --> MAXDIRT */
		if (stilinc)
			tflush();
		dirtcnt = 0;
		synctmp();
	}
}
#endif
/* TCSEC Division C Class C2 */

/*
 * Named buffer routines.
 * These are implemented differently than the main buffer.
 * Each named buffer has a chain of blocks in the register file.
 * Each read and write operation is in BUFBYTES chunks.
 * Each block contains roughly
 *		    BUFBYTES - sizeof(short) - sizeof(short)
 * wchar_t of text
 *		[8190 wchar_t with BUFBYTES == 16384 and sizeof(short) == 2],
 * and a previous and next block number.  We also have information
 * about which blocks came from deletes of multiple partial lines,
 * e.g. deleting a sentence or a LISP object.
 *
 * We maintain a free map for the temp file.  To free the blocks
 * in a register we must read the blocks to find how they are chained
 * together.
 *
 * The register routines take an wchar_t as argument but support only
 * 'a'-'z' and '0'-'9' indices.  See cmdreg() in ex_cmds2.c which restricts
 * the range of a register name to ascii, alphabetic characters.
 *
 * BUG: 	The default savind of deleted lines in numbered
 *		buffers may be rather inefficient; it hasn't been profiled.
*unsure re above
 */
static struct	strreg {
	short	rg_flags;
	short	rg_nleft;
	short	rg_first;
	short	rg_last;
} strregs[('z'-'a'+1) + ('9'-'0'+1)], *strp;

/* sizeof(rbuf) == BUFBYTES, as used in regio() for read and write */
static struct	rbuf {
	short	rb_prev;
	short	rb_next;
	/* sizeof(rb_text) == sizeof(rbuf) - sizeof(first two fields) */
	wchar_t	rb_text[INCRMT
		- ((sizeof (short) + sizeof(short)) / sizeof(wchar_t))];
} *rbuf, KILLrbuf, putrbuf, YANKrbuf, regrbuf;
static short	rused[256];
static short	rnleft;
static short	rblock;
static short	rnext;
static wchar_t	*rbufcp;

/* AIX security enhancement */
#if !defined(TVI)
static void regio(short b, int (*iofcn)(int, char *, unsigned int))
{

	if (rfile == -1) {
		strcpy(rfname, tfname);
		*(strend(rfname) - 7) = 'R';
		rfile = creat(rfname, 0600);
		if (rfile < 0)
oops:
			filioerr(rfname);
		close(rfile);
		rfile = open(rfname, 2);
		if (rfile < 0)
			goto oops;
	}
	lseek(rfile, (long) b * BUFBYTES, 0);
	if ((*iofcn)(rfile, (char *)rbuf, BUFBYTES) != BUFBYTES)
		goto oops;
	rblock = b;
}

#else
void regio(short b, int (*iofcn)(int, char *, unsigned int))
{
	static int rctr = 0;	/* The number of shared memory bytes used up */
	char	*p;	/* pointer for buffer copying which is always done */

	if (rfile == -1) {	/* Get a new set of registers */
		if ((rfile = shmget(IPC_PRIVATE, rfile_size, IPC_EXCL|IPC_CREAT|S_IRUSR|S_IWUSR)) == -1)
			perror("5tvi");
		else {
			if ((rfname = shmat(rfile, 0, 0)) == (char *)-1)
				perror("6tvi");
			else {
				if ((shmctl(rfile, IPC_STAT, rfbuf)) == -1)
					perror("7tvi");
			}
		}
	}
	p = rfname + b * BUFBYTES;

	/* The intent is to shift the whole buffer to or from shared memory */

	if (iofcn == read)
		memcpy(rbuf, p, BUFBYTES);
	if (iofcn == write) {
		if (rctr < (rfile_size - BUFBYTES))
			memcpy(p, rbuf, BUFBYTES);
		else {
			rfbuf->shm_segsz = rfile_size * 2;
			if ((shmctl(rfile, SHM_SIZE, rfbuf)) == -1)
				perror("8tvi");
			else {
				rfile_size *= 2;
				memcpy(p, rbuf, BUFBYTES);
			}
		}
		rctr += BUFBYTES;
	}
	rblock = b;
}
#endif
/* TCSEC Division C Class C2 */

static int REGblk(void)
{
	register int i, j, m;

	for (i = 0; i < sizeof rused / sizeof rused[0]; i++) {
		m = (rused[i] ^ 0177777) & 0177777;
		if (i == 0)
			m &= ~1;
		if (m != 0) {
			j = 0;
			while ((m & 1) == 0)
				j++, m >>= 1;
			rused[i] |= (1 << j);
#ifdef RDEBUG
			printf("allocating block %d\n", i * 16 + j);
#endif
			return (i * 16 + j);
		}
	}
	error(MSGSTR(M_196, "Out of register space (ugh)"), DUMMY_INT);
	return(0);	/* NOTREACHED */
}

static struct	strreg * mapreg(register wchar_t c)
{
	if (iswupper(c))
		c = towlower(c);
	return (iswdigit(c) ? &strregs[('z'-'a'+1)+(c-'0')] : &strregs[c-'a']);
}

static KILLreg(register wchar_t c)
{
	register struct strreg *sp;

	rbuf = &KILLrbuf;
	sp = mapreg(c);
	rblock = sp->rg_first;
	sp->rg_first = sp->rg_last = 0;
	sp->rg_flags = sp->rg_nleft = 0;
	while (rblock != 0) {
#ifdef RDEBUG
		printf("freeing block %d\n", rblock);
#endif
		rused[rblock / 16] &= ~(1 << (rblock % 16));
		regio(rblock, (int (*)(int, char *, unsigned int))shread);
		rblock = rbuf->rb_next;
	}
}

/* AIX security enhancement */
#if !defined(TVI)
static int shread(void)
{
	struct front { short a; short b; };

	if (read(rfile, (char *) rbuf, sizeof (struct front)) == sizeof (struct front))
		return (sizeof (struct rbuf));
	return (0);
}
#endif

#if defined(TVI)
/* This is only called once from KILLreg() */
static int shread(void)
{
	return (0);
}
#endif
/* TCSEC Division C Class C2 */

int	getREG(void);
void putreg(wchar_t c)
{
	register line *odot = dot;
	register line *odol = dol;
	register int cnt;

	deletenone();
	appendnone();
	rbuf = &putrbuf;
	rnleft = 0;
	rblock = 0;
	rnext = mapreg(c)->rg_first;
	if (rnext == 0) {
		if (inopen) {
			splitw++;
			vclean();
			vgoto(WECHO, 0);
		}
		vreg = -1;
		error(MSGSTR(M_197, "Nothing in register %c"), c);
	}
	if (inopen && partreg(c)) {
		if (!FIXUNDO) {
			splitw++; vclean(); vgoto(WECHO, 0); vreg = -1;
			error(MSGSTR(M_198, "Can't put partial line inside macro"), DUMMY_INT);
		}
		squish();
		addr1 = addr2 = dol;
	}
	cnt = append(getREG, addr2);
	if (inopen && partreg(c)) {
		unddol = dol;
		dol = odol;
		dot = odot;
		pragged((short)0);
	}
	killcnt(cnt);
	notecnt = cnt;
}

int partreg(wchar_t c)
{

	return (mapreg(c)->rg_flags);
}

void notpart(register wchar_t c)
{

	if (c)
		mapreg(c)->rg_flags = 0;
}

static int getREG(void)
{
	register wchar_t *lp = linebuf;
	register wchar_t c;

	for (;;) {
		if (rnleft == 0) {
			if (rnext == 0)
				return (EOF);
			regio(rnext, read);
			rnext = rbuf->rb_next;
			rbufcp = rbuf->rb_text;
			rnleft = sizeof rbuf->rb_text / sizeof(wchar_t);
		}
		c = *rbufcp;
		if (c == 0)
			return (EOF);
		rbufcp++, --rnleft;
		if (c == '\n') {
			*lp++ = 0;
			return (0);
		}
		*lp++ = c;
	}
}

void YANKreg(register wchar_t c)
{
	register line *addr;
	register struct strreg *sp;
	wchar_t savelb[LBSIZE];

	if (iswdigit(c))
		kshift();
	if (iswlower(c))
		KILLreg(c);
	strp = sp = mapreg(c);
	sp->rg_flags = inopen && cursor && wcursor;
	rbuf = &YANKrbuf;
	if (sp->rg_last) {
		regio(sp->rg_last, read);
		rnleft = sp->rg_nleft;
		rbufcp = &rbuf->rb_text[sizeof(rbuf->rb_text) / sizeof(wchar_t)
					 - rnleft];
	} else {
		rblock = 0;
		rnleft = 0;
	}
	CP(savelb,linebuf);
	for (addr = addr1; addr <= addr2; addr++) {
		getline(*addr);
		if (sp->rg_flags) {
			if (addr == addr2)
				*wcursor = 0;
			if (addr == addr1)
				wcscpy(linebuf, cursor);
		}
		YANKline();
	}
	rbflush();
	if (strcmp(Command, "delete") != 0)
		killed();
	CP(linebuf,savelb);
}

static void kshift(void)
{
	register int i;

	KILLreg('9');
	for (i = '8'; i >= '0'; i--)
		copy(mapreg(i+1), mapreg(i), sizeof (struct strreg));
}

static void YANKline(void)
{
	register wchar_t *lp = linebuf;
	register struct rbuf *rp = rbuf;
	register int c;

	do {
		c = *lp++;
		if (c == 0)
			c = '\n';
		if (rnleft == 0) {
			rp->rb_next = REGblk();
			rbflush();
			rblock = rp->rb_next;
			rp->rb_next = 0;
			rp->rb_prev = rblock;
			rnleft = sizeof rp->rb_text / sizeof (wchar_t);
			rbufcp = rp->rb_text;
		}
		*rbufcp++ = c;
		--rnleft;
	} while (c != '\n');
	if (rnleft)
		*rbufcp = 0;
}

static void rbflush(void)
{
	register struct strreg *sp = strp;

	if (rblock == 0)
		return;
	regio(rblock, write);
	if (sp->rg_first == 0)
		sp->rg_first = rblock;
	sp->rg_last = rblock;
	sp->rg_nleft = rnleft;
}

/* Register c to wchar_t buffer buf of size buflen wchar_t's */
void regbuf(wchar_t c, wchar_t *buf, int buflen)
{
	register wchar_t *p, *lp;

	rbuf = &regrbuf;
	rnleft = 0;
	rblock = 0;
	rnext = mapreg(c)->rg_first;
	if (rnext==0) {
		*buf = 0;
		error(MSGSTR(M_199, "Nothing in register %c"),c);
	}
	p = buf;
	while (getREG()==0) {
		for (lp=linebuf; *lp;) {
			if (p >= &buf[buflen])
				error(MSGSTR(M_200, "Register too long@to fit in memory"), DUMMY_INT);
			*p++ = *lp++;
		}
		*p++ = '\n';
	}
	if (partreg(c)) p--;
	*p = '\0';
	getDOT();
}

