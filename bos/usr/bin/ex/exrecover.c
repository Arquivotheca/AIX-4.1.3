static char sccsid [] = "@(#)08  1.21  src/bos/usr/bin/ex/exrecover.c, cmdedit, bos41B, 9504A 12/19/94 11:47:03";
/*
 * COMPONENT_NAME: (CMDEDIT) exrecover.c
 *
 * FUNCTIONS: main, blkio_exr, clrstats_exr, enter, error_exr,
 * findtmp, getblock_exr, getline_exr, listfiles, preserve, putfile_exr, qucmp,
 * scrapbad, searchdir, syserror_exr, wrerror_exr, yeah
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
 * Copyright (c) 1981 Regents of the University of California
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

# define var 	/* nothing */
#include "ex.h"
#include "ex_temp.h"
#include "ex_tty.h"
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <locale.h>

nl_catd catd;

/*
 * Here we save the information about files, when
 * you ask us what files we have saved for you.
 * We buffer file name, number of lines, and the time
 * at which the file was saved.
 */
struct svfile {
	char	sf_name[FNSIZE + 1];
	int	sf_lines;
/* While OSF code was brought in AIX environment, MAX_NAME_LEN was
   replaced by MAXNAMLEN, because that's the name used under AIX */
	char	sf_entry[MAXNAMLEN + 1];
	time_t	sf_time;
};

void clrstats_exr(void);
void syserror_exr(void);

static void wrerror_exr(void);
static void error_exr(char *, int);

static void listfiles(char *);
static void enter(struct svfile *, char *, int);
static int qucmp(struct svfile *, struct svfile *);
static void findtmp(char *);
static void searchdir(char *);
static int yeah(char *);
static void scrapbad(void);
static void putfile_exr(void);
static void getline_exr(line);
static wchar_t * getblock_exr(line, int);
static void blkio_exr(short, wchar_t *, 
                      int (*)(int, char *, unsigned int));

static char xstr[1];		/* make loader happy */
short tfile = -1;	/* ditto */

/*
 *
 * This program searches through the specified directory and then
 * the directory /usr/preserve looking for an instance of the specified
 * file from a crashed editor or a crashed system.
 * If this file is found, it is unscrambled and written to
 * the standard output.
 *
 * If this program terminates without a "broken pipe" diagnostic
 * (i.e. the editor doesn't die right away) then the buffer we are
 * writing from is removed when we finish.  This is potentially a mistake
 * as there is not enough handshaking to guarantee that the file has actually
 * been recovered, but should suffice for most cases.
 */

/*
 * This directory definition also appears (obviously) in expreserve.c.
 * Change both if you change either.
 */
static	char	mydir[] =	"/var/preserve";

/*
 * Limit on the number of printed entries
 * when an, e.g. ``ex -r'' command is given.
 */
#define NENTRY  25      /* MSH:  was 50 */

static	char	nb[BUFFERSIZ];
static	int	vercnt;			/* Count number of versions of file found */
static	char	timebuf[NLTBMAX];

int main(int argc, char *argv[])
{
	register int b, i;

	setlocale(LC_ALL,"");		
	catd = catopen(MF_EX, NL_CAT_LOCALE);

	/*
	 * If given only a -r argument, then list the saved files.
	 */
	if (argc == 2 && eq(argv[1], "-r")) {
		listfiles(mydir);
		/* close the message catalog */
		catclose(catd);
		exit(0);
	}
	if (argc != 3)
		error_exr(MSGSTR(M_268, " Wrong number of arguments to exrecover"), 0);

	strcpy(file, argv[2]);

	/*
	 * Search for this file.
	 */
	findtmp(argv[1]);

	/*
	 * Got (one of the versions of) it, write it back to the editor.
	 */
	strftime(timebuf, NLTBMAX, MSGSTR(M_287, "%a %sD %T"), localtime(&H.Time));
	if (vercnt > 1)
		fprintf(stderr, MSGSTR(M_269, " [Dated: %s, newest of %d saved]"), timebuf, vercnt);
	else
		fprintf(stderr, MSGSTR(M_270, " [Dated: %s]"), timebuf);
	H.Flines++;

	/*
	 * Initialize as though the editor had just started.
	 * Allocate space for the line pointers from the temp file.
	 */
	if ((fendcore = (line *)malloc((int)H.Flines * sizeof(line))) == NULL)
		/*
		 * Good grief.
		 */
		error_exr(MSGSTR(M_271, " Not enough core for lines"), 0);
	dot = zero = dol = fendcore;
	one = zero + 1;
	iblock = oblock = -1;

#ifdef DEBUG
	fprintf(stderr, "%d lines\n", H.Flines);
#endif

	/*
	 * Now go get the blocks of seek pointers which are scattered
	 * throughout the temp file, reconstructing the incore
	 * line pointers at point of crash.
	 */
	b = 0;
	while (H.Flines > 0) {
		(void)lseek(tfile, (long) blocks[b] * BUFBYTES, 0);
		i = H.Flines < BUFBYTES / sizeof (line) ?
			H.Flines * sizeof (line) : BUFBYTES;
		if (read(tfile, (char *) dot, i) != i) {
			perror(nb);

			/* close the message catalog */
			catclose(catd);
			exit(1);
		}
		dot += i / sizeof (line);
		H.Flines -= i / sizeof (line);
		b++;
	}
	dot--; dol = dot;

	/*
	 * Sigh... due to sandbagging some lines may really not be there.
	 * Find and discard such.  This shouldn't happen much.
	 */
	scrapbad();

	/*
	 * Now if there were any lines in the recovered file
	 * write them to the standard output.
	 */
	if (dol > zero) {
		addr1 = one; addr2 = dol; io = 1;
		putfile_exr();
	}

	/*
	 * Trash the saved buffer.
	 * Hopefully the system won't crash before the editor
	 * syncs the new recovered buffer; i.e. for an instant here
	 * you may lose if the system crashes because this file
	 * is gone, but the editor hasn't completed reading the recovered
	 * file from the pipe from us to it.
	 *
	 * This doesn't work if we are coming from an non-absolute path
	 * name since we may have chdir'ed but what the hay, noone really
	 * ever edits with temporaries in "." anyways.
	 */
	if (nb[0] == '/')
		(void)(unlink(nb));

	/*
	 * Adieu.
	 */

	/* close the message catalog */
	catclose(catd);
	exit(0);
        return(0); /* return put here to keep lint happy */
}

/*
 * Print an error message (notably not in error
 * message file).  If terminal is in RAW mode, then
 * we should be writing output for "vi", so don't print
 * a newline which would screw up the screen.
 */
void
error_exr(char *str, int inf)
{
	int cook_mode;

	fprintf(stderr, str, inf);
#ifdef _POSIX_SOURCE
	tcgetattr(2, &tty);
	cook_mode = tty.c_lflag & ICANON;
#else					/* _POSIX_SOURCE */
#ifndef USG
	gtty(2, &tty);
	cook_mode = (tty.sg_flags & RAW) == 0;
#else
	ioctl(2, TCGETA, (char *)&tty);
	cook_mode = tty.c_lflag & ICANON;
#endif
#endif					/* _POSIX_SOURCE */
	if (cook_mode)
		fprintf(stderr, "\n");

	/* close the message catalog */
	catclose(catd);
	exit(1);
}

static
void listfiles(char *dirname)
{
	register DIR *dir;
	struct dirent *dirp;
	int ecount;
	register int f;
	struct svfile *fp, svbuf[NENTRY];
	struct stat st_buf;

	/*
	 * Open /usr/preserve, and go there to make things quick.
	 */
	dir = opendir(dirname);
	if (dir == NULL) {
		perror(dirname);
		return;
	}
	if (chdir(dirname) < 0) {
		perror(dirname);
		return;
	}

	/*
	 * Look at the candidate files in /usr/preserve.
	 */
	fp = &svbuf[0];
	ecount = 0;
	while ((dirp = readdir(dir)) != NULL) {
		if (dirp->d_name[0] != 'E')
			continue;
#ifdef DEBUG
		fprintf(stderr, "considering %s\n", dirp->d_name);
#endif
		stat(dirp->d_name, &st_buf);
		if (!S_ISREG(st_buf.st_mode))
			continue;
		/*
		 * Name begins with E; open it and
		 * make sure the uid in the header is our uid.
		 * If not, then don't bother with this file, it can't
		 * be ours.
		 */
		f = open(dirp->d_name, 0);
		if (f < 0) {
#ifdef DEBUG
			fprintf(stderr, "open failed\n");
#endif
			continue;
		}
		if (read(f, (char *) &H, sizeof H) != sizeof H) {
#ifdef DEBUG
			fprintf(stderr, "culdnt read hedr\n");
#endif
			(void)(close(f));
			continue;
		}
		(void)(close(f));
		if (getuid() != H.Uid) {
#ifdef DEBUG
			fprintf(stderr, "uid wrong\n");
#endif
			continue;
		}

		/*
		 * Saved the day!
		 */
		enter(fp++, dirp->d_name, ecount);
		ecount++;
#ifdef DEBUG
		fprintf(stderr, "entered file %s\n", dirp->d_name);
#endif
	}
	closedir(dir);

	/*
	 * If any files were saved, then sort them and print
	 * them out.
	 */
	if (ecount == 0) {
		fprintf(stderr, MSGSTR(M_272, "No files saved.\n"));
		return;
	}
	qsort(&svbuf[0], ecount, sizeof(svbuf[0]), qucmp);
	for (fp = &svbuf[0]; fp < &svbuf[ecount]; fp++) {
		strftime(timebuf, NLTBMAX, MSGSTR(M_288, "On %a %sD at %sT"), localtime(&fp->sf_time));
		fprintf(stderr, MSGSTR(M_273, "%s saved %d lines of file \"%s\"\n"), timebuf, fp->sf_lines, fp->sf_name);
	}
}

/*
 * Enter a new file into the saved file information.
 */
static
void enter(struct svfile *fp, char *fname, int count)
{
	register char *cp, *cp2;
	register struct svfile *f, *fl;
	time_t curtime, itol();

	f = 0;
	if (count >= NENTRY) {
		/*
		 * My god, a huge number of saved files.
		 * Would you work on a system that crashed this
		 * often?  Hope not.  So lets trash the oldest
		 * as the most useless.
		 *
		 * (unsure)
		 */
		fl = fp - count + NENTRY - 1;
		curtime = fl->sf_time;
		for (f = fl; --f > fp-count; )
			if (f->sf_time < curtime)
				curtime = f->sf_time;
		for (f = fl; --f > fp-count; )
			if (f->sf_time == curtime)
				break;
		fp = f;
	}

	/*
	 * Gotcha.
	 */
	fp->sf_time = H.Time;
	fp->sf_lines = H.Flines;
	for (cp2 = fp->sf_name, cp = savedfile; *cp;)
		*cp2++ = *cp++;
	*cp2++ = 0;
	for (cp2 = fp->sf_entry, cp = fname; *cp && cp-fname < 14;)
		*cp2++ = *cp++;
	*cp2++ = 0;
}

/*
 * Do the qsort compare to sort the entries first by file name,
 * then by modify time.
 */
static
int qucmp(struct svfile *p1, struct svfile *p2)
{
	register int t;

	if (t = strcmp(p1->sf_name, p2->sf_name))
		return(t);
	if (p1->sf_time > p2->sf_time)
		return(-1);
	return(p1->sf_time < p2->sf_time);
}

/*
 * Scratch for search.
 */
static char	bestnb[BUFFERSIZ];		/* Name of the best one */
static long	besttime;		/* Time at which the best file was saved */
static int	bestfd;			/* Keep best file open so it dont vanish */

/*
 * Look for a file, both in the users directory option value
 * (i.e. usually /tmp) and in /usr/preserve.
 * Want to find the newest so we search on and on.
 */
static
void findtmp(char *dir)
{

	/*
	 * No name or file so far.
	 */
	bestnb[0] = 0;
	bestfd = -1;

	/*
	 * Search /usr/preserve and, if we can get there, /tmp
	 * (actually the users "directory" option).
	 */
	searchdir(dir);
	if (chdir(mydir) == 0)
		searchdir(mydir);
	if (bestfd != -1) {
		/*
		 * Gotcha.
		 * Put the file (which is already open) in the file
		 * used by the temp file routines, and save its
		 * name for later unlinking.
		 */
		tfile = bestfd;
		strcpy(nb, bestnb);
		(void)lseek(tfile, 0l, 0);

		/*
		 * Gotta be able to read the header or fall through
		 * to lossage.
		 */
		if (read(tfile, (char *) &H, sizeof H) == sizeof H)
			return;
	}

	/*
	 * Extreme lossage...
	 */
	error_exr(MSGSTR(M_274, " File not found"), 0);
}

/*
 * Search for the file in directory dirname.
 *
 * Don't chdir here, because the users directory
 * may be ".", and we would move away before we searched it.
 * Note that we actually chdir elsewhere (because it is too slow
 * to look around in /usr/preserve without chdir'ing there) so we
 * can't win, because we don't know the name of '.' and if the path
 * name of the file we want to unlink is relative, rather than absolute
 * we won't be able to find it again.
 */
static
void searchdir(char *dirname)
{
	struct dirent *dirp;
	register DIR *dir;
	/* char dbuf[BUFFERSIZ]; */

	dir = opendir(dirname);
	if (dir == NULL)
		return;
	/* setbuf(dir, dbuf); this breaks UNIX/370. */
	while ((dirp = readdir(dir)) != NULL) {
		if (dirp->d_name[0] != 'E')
			continue;
		/*
		 * Got a file in the directory starting with E...
		 * Save a consed up name for the file to unlink
		 * later, and check that this is really a file
		 * we are looking for.
		 */
		(void)strcat(strcat(strcpy(nb, dirname), "/"), dirp->d_name);
		if (yeah(nb)) {
			/*
			 * Well, it is the file we are looking for.
			 * Is it more recent than any version we found before?
			 */
			if (H.Time > besttime) {
				/*
				 * A winner.
				 */
				(void)close(bestfd);
				bestfd = dup(tfile);
				besttime = H.Time;
				strcpy(bestnb, nb);
			}
			/*
			 * Count versions so user can be told there are
			 * ``yet more pages to be turned''.
			 */
			vercnt++;
		}
		(void)close(tfile);
	}
	closedir(dir);
}

/*
 * Given a candidate file to be recovered, see
 * if its really an editor temporary and of this
 * user and the file specified.
 */
static
int yeah(char *name)
{

	struct stat st_buf;
	stat(name, &st_buf);
	if (!S_ISREG(st_buf.st_mode))
		return (0);
	tfile = open(name, 2);
	if (tfile < 0)
		return (0);
	if (read(tfile, (char *) &H, sizeof H) != sizeof H) {
nope:
		(void) close(tfile);
		return (0);
	}
	if (!eq(savedfile, file))
		goto nope;
	if (getuid() != H.Uid)
		goto nope;
	/*
	 * This is old and stupid code, which
	 * puts a word LOST in the header block, so that lost lines
	 * can be made to point at it.
	 */
	(void) lseek(tfile, (long)(BUFBYTES*HBLKS-8), 0);
	(void) write(tfile, "LOST", 5);
	return (1);
}


/*
 * Find the true end of the scratch file, and ``LOSE''
 * lines which point into thin air.  This lossage occurs
 * due to the sandbagging of i/o which can cause blocks to
 * be written in a non-obvious order, different from the order
 * in which the editor tried to write them.
 *
 * Lines which are lost are replaced with the text LOST so
 * they are easy to find.  We work hard at pretty formatting here
 * as lines tend to be lost in blocks.
 *
 * This only seems to happen on very heavily loaded systems, and
 * not very often.
 */
static
void scrapbad(void)
{
	register line *ip;
	struct stat stbuf;
	off_t size, maxt;
	int bno, cnt, bad, was;
	char bk[BUFFERSIZ];

	(void) fstat(tfile, &stbuf);
	size = stbuf.st_size;
	maxt = (size >> SHFT) | (BNDRY-1);
	bno = (maxt >> OFFBTS) & BLKMSK;
#ifdef DEBUG
	fprintf(stderr, "size %ld, maxt %o, bno %d\n", size, maxt, bno);
#endif

	/*
	 * Look for a null separating two lines in the temp file;
	 * if last line was split across blocks, then it is lost
	 * if the last block is.
	 */
	while (bno > 0) {
		(void) lseek(tfile, (long) BUFBYTES * bno, 0);
		cnt = read(tfile, (char *) bk, BUFFERSIZ);
		while (cnt > 0)
			if (bk[--cnt] == 0)
				goto null;
		bno--;
	}
null:

	/*
	 * Magically calculate the largest valid pointer in the temp file,
	 * consing it up from the block number and the count.
	 */
	maxt = ((bno << OFFBTS) | (cnt >> SHFT)) & ~1;
#ifdef DEBUG
	fprintf(stderr, "bno %d, cnt %d, maxt %o\n", bno, cnt, maxt);
#endif

	/*
	 * Now cycle through the line pointers,
	 * trashing the Lusers.
	 */
	was = bad = 0;
	for (ip = one; ip <= dol; ip++)
		if (*ip > maxt) {
#ifdef DEBUG
			fprintf(stderr, "%d bad, %o > %o\n", ip - zero, *ip, maxt);
#endif
			if (was == 0)
				was = ip - zero;
			*ip = ((HBLKS*BUFFERSIZ)-8) >> SHFT;
		} else if (was) {
			if (bad == 0)
				fprintf(stderr, " [%s", MSGSTR(M_275, "Lost line(s):"));
			fprintf(stderr, " %d", was);
			if ((ip - 1) - zero > was)
				fprintf(stderr, "-%d", (ip - 1) - zero);
			bad++;
			was = 0;
		}
	if (was != 0) {
		if (bad == 0)
			fprintf(stderr, " [%s", MSGSTR(M_275, "Lost line(s):"));
		fprintf(stderr, " %d", was);
		if (dol - zero != was)
			fprintf(stderr, "-%d", dol - zero);
		bad++;
	}
	if (bad)
		fprintf(stderr, "]");
}

static int	cntch, cntln, cntodd, cntnull;
/*
 * Following routines stolen mercilessly from ex.
 */
/* similar to putfile() in ex_io.c */
static
void putfile_exr(void)
{
	line *a1;
	register char *fp, *lp;
	register int nib;
	char outbuf[BUFFERSIZ];
	char tmpbuf[sizeof(linebuf)];

	a1 = addr1;
	clrstats_exr();
	cntln = addr2 - a1 + 1;
	if (cntln == 0)
		return;
	nib = BUFFERSIZ;
	fp = outbuf;
	do {
		getline_exr(*a1++);


		if (wcstombs(tmpbuf, linebuf, sizeof(tmpbuf)) == -1)
			printf(MSGSTR(M_651, "Invalid wide character string, conversion failed."));

		lp = tmpbuf;
		for (;;) {
			if (--nib < 0) {
				nib = fp - outbuf;
				if (write(io, outbuf, nib) != nib) {
					wrerror_exr();
				}
				cntch += nib;
				nib = BUFFERSIZ - 1;
				fp = outbuf;
			}
			if ((*fp++ = *lp++) == 0) {
				fp[-1] = '\n';
				break;
			}
		}
	} while (a1 <= addr2);
	nib = fp - outbuf;
	if (write(io, outbuf, nib) != nib) {
		wrerror_exr();
	}
	cntch += nib;
}

static void
wrerror_exr(void)
{
	syserror_exr();
}

/* similar to clrstats in ex_io.c */
static void clrstats_exr(void)
{

	ninbuf = 0;
	cntch = 0;
	cntln = 0;
	cntnull = 0;
	cntodd = 0;
}

#define	READ	0
#define	WRITE	1

/* aside from "static", same as getline() in ex_temp.c */
static
void getline_exr(line tl)
{
	register wchar_t *bp, *lp;
	register int nl;

	lp = linebuf;
	bp = getblock_exr(tl, READ);
	nl = nleft;
	tl &= ~OFFMSK;
	while (*lp++ = *bp++)
		if (--nl == 0) {
			bp = getblock_exr(tl += INCRMT, READ);
			nl = nleft;
		}
}

/* similar to getblock in ex_temp.c */
static wchar_t *
getblock_exr(line atl, int iof)
{
	register int bno, off;
	
	bno = (atl >> OFFBTS) & BLKMSK;
	off = (atl << SHFT) & LBTMSK;
	if (bno >= NMBLKS)
		error_exr(MSGSTR(M_277, " Tmp file too large"), DUMMY_INT);
	nleft = INCRMT - off;
	if (bno == iblock) {
		ichanged |= iof;
		return (ibuff + off);
	}
	if (bno == oblock)
		return (obuff + off);
	if (iof == READ) {
		if (ichanged)
			blkio_exr(iblock, ibuff, write);
		ichanged = 0;
		iblock = bno;
		blkio_exr((short)bno, ibuff, read);
		return (ibuff + off);
	}
	if (oblock >= 0)
		blkio_exr(oblock, obuff, write);
	oblock = bno;
	return (obuff + off);
}

/* similar to blkio in ex_temp.c */
static
void blkio_exr(short b, wchar_t *buf, int (*iofcn)(int, char *, unsigned int))
{

	(void) lseek(tfile, (long) (unsigned) b * BUFBYTES, 0);
	if ((*iofcn)(tfile, (char *) buf, BUFBYTES) != BUFBYTES)
		syserror_exr();
}

/* similar to syserror() in ex_subr.c */
static void
syserror_exr(void)
{
	extern int sys_nerr;

	dirtcnt = 0;
	write(2, " ", 1);
	if (errno >= 0 && errno < sys_nerr)
		error_exr(strerror(errno), DUMMY_INT);
	else
		error_exr(MSGSTR(M_278, "System error %d"), errno);

	/* close the message catalog */
	catclose(catd);
	exit(1);
}
