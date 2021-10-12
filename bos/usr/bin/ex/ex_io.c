#if !defined(lint)
static char sccsid[] = "@(#)80	1.27.2.16  src/bos/usr/bin/ex/ex_io.c, cmdedit, bos41J, 9514A_all 3/30/95 13:35:52";
#endif
/*
 * COMPONENT_NAME: (CMDEDIT) ex_io.c
 *
 * FUNCTIONS: checkmodeline, clrstats, dorop, edfile, filename, getargs,
 * getfile, getone, glob, gscan, iostats, putfile, rop, rop2, rop3, samei,
 * source, wop, wrerror
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

#include "ex.h"
#include "ex_argv.h"
#include "ex_temp.h"
#include "ex_tty.h"
#include "ex_vis.h"
#include <sys/file.h>
#if defined(_SECURITY)
#include <sys/priv.h>
#endif

/* AIX security enhancement */
#if defined(TVI)
#include <fcntl.h>
int	auditwrite(char *, int, ...);
int	tcb(char *, int);
#endif
/* TCSEC Division C Class C2 */

/*
 * File input/output, source, preserve and recover
 */


int	access(char *, int);

static short iostats(void);
static void wrerror(void);
/* AIX security enhancement */
#if !defined(TVI)
static void checkmodeline(wchar_t *);
#endif
/* TCSEC Division C Class C2 */

void	glob(struct glob *);

/*
 * Following remember where . was in the previous file for return
 * on file switching.
 */
static int	altdot;
static int	oldadot;
static int	samedot;
static short	wasalt;
static short	samefile;
static long	cntch;		/* Count of characters on unit io */
static long	cntnull;	/* Count of nulls " */
static int	cntln;		/* Count of lines " */

static wchar_t  *fpat = NULL;
#ifdef  FLOCKFILE
/*
 * The alternate, saved and current file are locked the extent of the
 * time that they are active. If the saved file is exchanged
 * with the alternate file, the file descriptors are exchanged
 * and the lock is not released.
 */
int     io_savedfile, io_altfile, io_curr ;
int     lock_savedfile, lock_altfile, lock_curr ;
#endif	/* FLOCKFILE */

/*
 * Parse file name for command encoded by comm.
 * If comm is E then command is doomed and we are
 * parsing just so user won't have to retype the name.
 */
void
filename(int comm)
{
	register int c = comm, d;
	register int i;
#ifdef  FLOCKFILE
	int lock ;

	lock = 0 ;
#endif	/* FLOCKFILE */

	d = ex_getchar();
	if (endcmd(d)) {
		samefile = strcmp(file, savedfile) == 0;
		if (savedfile[0] == 0 && comm != 'f')
			error(MSGSTR(M_069, "No file|No current filename"), DUMMY_INT);
		strcpy(file, savedfile);
#ifdef  FLOCKFILE
		if (io_curr && io_curr != io_savedfile) close(io_curr) ;
		lock = lock_curr = lock_savedfile ;
		io_curr = io_savedfile ;
#endif	/* FLOCKFILE */
		wasalt = 0;
		oldadot = altdot;
		if (c == 'e' || c == 'E')
			samedot = altdot = lineDOT();
		if (d == EOF)
			ungetchar(d);
	} else {
		ungetchar(d);
		getone();
		eol();
		if (savedfile[0] == 0 && c != 'E' && c != 'e') {
			c = 'e';
			edited = 0;
		}
		wasalt = strcmp(file, altfile) == 0;
		samefile = strcmp(file, savedfile) == 0;
		samedot = lineDOT();
		oldadot = altdot;
		switch (c) {

		case 'f':
			edited = 0;
			/* fall into ... */

		case 'e':
			if (savedfile[0]) {
#ifdef  FLOCKFILE
				if (strcmp(file,savedfile) == 0) break ;
#endif	/* FLOCKFILE */
				altdot = lineDOT();
				strcpy(altfile, savedfile);
#ifdef  FLOCKFILE
				if (io_altfile) close (io_altfile) ;
				io_altfile = io_savedfile ;
				lock_altfile = lock_savedfile ;
				io_savedfile = 0 ;
#endif	/* FLOCKFILE */
			}
			strcpy(savedfile, file);
#ifdef  FLOCKFILE
			io_savedfile = io_curr ;
			lock_savedfile = lock_curr ;
			io_curr = 0 ;           lock = lock_curr = 0 ;
#endif	/* FLOCKFILE */
			break;

		default:
			if (file[0]) {
#ifdef  FLOCKFILE
				if (wasalt) break ;
#endif
				if (c != 'E')
					altdot = lineDOT();
				strcpy(altfile, file);
#ifdef  FLOCKFILE
				if (io_altfile
				&& io_altfile != io_curr) close (io_altfile) ;
				io_altfile = io_curr ;
				lock_altfile = lock_curr ;
				io_curr = 0 ;           lock = lock_curr = 0 ;
#endif	/* FLOCKFILE */
			}
			break;
		}
	}
	if (hush && comm != 'f' || comm == 'E')
		return;
	if (file[0] != 0) {
		lprintf("\"%s\"", file);
		if (comm == 'f') {
			if (value(READONLY))
				ex_printf(MSGSTR(M_070, " [Read only]"));
			if (!edited)
				ex_printf(MSGSTR(M_071, " [Not edited]"));
			if (tchng)
				ex_printf(MSGSTR(M_072, " [Modified]"));
#ifdef  FLOCKFILE
			if (lock == LOCK_SH)
				ex_printf(MSGSTR(M_298, " [Shared lock]"));
			else if (lock == LOCK_EX)
				ex_printf(MSGSTR(M_299, " [Exclusive lock]"));
#endif	/* FLOCKFILE */
		}
		flush();
	} else
		ex_printf(MSGSTR(M_073, "No file "));
	if (comm == 'f') {
		if (!(i = lineDOL()))
			i++;
		ex_printf(MSGSTR(M_074, " line %d of %d --%ld%%--"), lineDOT(), lineDOL(), (long) 100 * lineDOT() / i);
	}
}

/*
 * Get the argument words for a command into genbuf
 * expanding # and %.
 */
int
getargs(void)
{
	register int c;
	register wchar_t *cp;
	register char *fp;		/* filename pointer */
	static wchar_t fpatbuf[128];	/* hence limit on :next +/pat */
	char tempchar[32];

	pastwh();
	if (peekchar() == '+') {
		for (cp = fpatbuf;;) {
			*cp++ = c = ex_getchar();
			if (cp >= &fpatbuf[WCSIZE(fpatbuf)])
				error(MSGSTR(M_075, "Pattern too long"), DUMMY_INT);
			if (c == '\\' && iswspace(peekchar()))
				c = ex_getchar();
			if (c == EOF || iswspace(c)) {
				ungetchar(c);
				*--cp = 0;
				fpat = &fpatbuf[1];
				break;
			}
		}
	}
	if (skipend())
		return (0);

	sprintf(tempchar, "echo ");
	if (mbstowcs(genbuf, tempchar, 6) == -1)
		error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);

	cp = &genbuf[5];
	for (;;) {
		c = ex_getchar();
		if (endcmd(c)) {
			ungetchar(c);
			break;
		}
		switch (c) {

		case '\\':
			if (any(peekchar(), "#%|"))
				c = ex_getchar();
			/* fall into... */

		default:
			if (cp > &genbuf[LBSIZE])
flong:
				error(MSGSTR(M_076, "Argument buffer overflow"), DUMMY_INT);
			*cp++ = c;
			break;

		case '#':
			fp = altfile;
			if (*fp == 0)
				error(MSGSTR(M_077, "No alternate filename@to substitute for #"), DUMMY_INT);
			goto filexp;

		case '%':
			fp = savedfile;
			if (*fp == 0)
				error(MSGSTR(M_078, "No current filename@to substitute for %%"), DUMMY_INT);
filexp:
			while (*fp) {
				int char_len;
				wchar_t pwcptr;

				if (cp > &genbuf[LBSIZE])
					goto flong;


				if ((char_len = mbtowc(&pwcptr, fp, MB_CUR_MAX)) > 0)
				{
				*cp++ = pwcptr;
				fp += char_len;
				}
				else
					error(MSGSTR(M_652, "Incomplete or invalid multibyte character, conversion failed."), DUMMY_INT);


			}
			break;
		}
	}
	*cp = 0;
	return (1);
}

/*
 * Glob the argument words in genbuf, or if no globbing
 * is implied, just split them up directly.
 */
void
glob(struct glob *gp)
{
	int pvec[2];
	register char **argvec = gp->argv;
	register char *cp = gp->argspac;
	register int c;
	char ch[MB_LEN_MAX];
	int numleft = NCARGS;

	gp->argc0 = 0;
	if (gscan() == 0) {
		register wchar_t *v = genbuf + 5;	  /* strlen("echo ") */

		for (;;) {
			while (iswspace(*v))
				v++;
			if (!*v)
				break;
			*argvec++ = cp;
			while (*v && !iswspace(*v)) {

				cp += wctomb(cp, *v);
				v++;

			}
			*cp++ = 0;
			gp->argc0++;
		}
		*argvec = 0;
		return;
	}
	if (pipe(pvec) < 0)
		error(MSGSTR(M_079, "Can't make pipe to glob"), DUMMY_INT);
	pid = fork();
	io = pvec[0];
	if (pid < 0) {
		close(pvec[1]);
		error(MSGSTR(M_080, "Can't fork to do glob"), DUMMY_INT);
	}
	if (pid == 0) {
		int oerrno;
		char tmpbuf[sizeof(genbuf)];

		close(1);
		dup(pvec[1]);
		close(pvec[0]);
		close(2);	/* so errors don't mess up the screen */
		open("/dev/null", 1);

		if (wcstombs(tmpbuf, genbuf, sizeof(tmpbuf)) == -1)
			error(MSGSTR(M_651, "Invalid wide character string, conversion failed."), DUMMY_INT);

		execl(svalue(SHELL), "sh", "-c", tmpbuf, 0);
		oerrno = errno; close(1); dup(2); errno = oerrno;
		filioerr(svalue(SHELL));
	}
	close(pvec[1]);
	do {
		*argvec = cp;
		for (;;) {
			int count;
			wchar_t wcharptr;

			c = -1;
			for(count = 0; count < MB_CUR_MAX; count++){
				if(read(io, &ch[count], 1) != 1){
					close(io);
					break;
				}
				if ((mbtowc(&wcharptr, ch, count + 1)) == count + 1){
					ch[count + 1] = 0;
					c = 1;
					break;
				}
			}

			if (c <= 0 || iswspace(wcharptr))
				break;
			if ((numleft - (count + 1)) <= 0){
				*cp = '\0';			/* terminate the string before jump */
				error(MSGSTR(M_081, "Arg list too long"), DUMMY_INT);
			}
			else 
				for (count = 0; ch[count] != 0; count++, numleft--)
					*cp++ = ch[count];
		}
		if (cp != *argvec) {
			--numleft;
			*cp++ = 0;
			gp->argc0++;
			if (gp->argc0 >= NARGS)
				error(MSGSTR(M_082, "Arg list too long"), DUMMY_INT);
			argvec++;
		}
	} while (c >= 0);
/* AIX security enhancement */
#if !defined(TVI)
	waitfor();	/* part of ex_unix.c */
#endif
/* TCSEC Division C Class C2 */
	if (gp->argc0 == 0)
		error(MSGSTR(M_083, "No match"), DUMMY_INT);
}

/*
 * Scan genbuf for shell metacharacters.
 * Set is union of v7 shell and csh metas.
 */
static int
gscan(void)
{
	register wchar_t *cp;

	for (cp = genbuf; *cp; cp++)
		if (any(*cp, "~{[*?$`'\"\\"))
			return (1);
	return (0);
}


/*
 * Parse one filename into file.
 */
static struct glob G;

void
getone(void)
{
	register char *str;

	if (getargs() == 0)
		error(MSGSTR(M_084, "Missing filename"), DUMMY_INT);
	glob(&G);
	if (G.argc0 > 1)
		error(MSGSTR(M_085, "Ambiguous|Too many file names"), DUMMY_INT);
	str = G.argv[G.argc0 - 1];
	if (strlen(str) > FNSIZE - 4)
		error(MSGSTR(M_086, "Filename too long"), DUMMY_INT);
	strcpy(file, str);
}

void
rop(int c, int r)
{
    sigset_t        setsig, oldsigs;
    void            dorop();

    /* block and unblock sigwinch iff not already blocked */

    if (!sigwinch_blocked++) {
        sigemptyset(&setsig);
        sigaddset(&setsig, SIGWINCH);
        sigprocmask(SIG_BLOCK, &setsig, NULL);
    }
    dorop(c, r);

    if (!--sigwinch_blocked)
        sigprocmask(SIG_UNBLOCK, &setsig, NULL);
}
/*
 * Read a file from the world.
 * C is command, 'e' if this really an edit (or a recover).
 */
static void
dorop(int c, int r)
{
	register int i;
	struct stat stbuf;
	short magic;
	static int ovro;	/* old value(READONLY) */
	static int denied;	/* 1 if READONLY was set due to file permissions */
#ifdef  FLOCKFILE
	int *lp, *iop;
#endif	/* FLOCKFILE */

	if (c != 'r') {         /* To prevent caryover of readonly option to new file */
		if (value(READONLY) && denied) {
			value(READONLY) = ovro;
			denied = 0;
		}
	}
	io = open(file, 0);
	if (io < 0) {
		if (c == 'e' && errno == ENOENT) {
			edited++;
			/*
			 * If the user just did "ex foo" he is probably
			 * creating a new file.  Don't be an error, since
			 * this is ugly, and it screws up the + option.
			 */
			strcpy(origfile,"");
			if (!hush)
				ex_printf(MSGSTR(M_087, " [New file]"));
			noonl();
			return;
		}
		syserror(0);
	}
	if (fstat(io, &stbuf))
		syserror(0);
	switch (stbuf.st_mode & S_IFMT) {

	case S_IFBLK:
		error(MSGSTR(M_088, " Block special file"), DUMMY_INT);

	case S_IFCHR:
		if (isatty(io))
			error(MSGSTR(M_089, " Teletype"), DUMMY_INT);
		if (samei(&stbuf, "/dev/null"))
			break;
		error(MSGSTR(M_090, " Character special file"), DUMMY_INT);

	case S_IFDIR:
		error(MSGSTR(M_091, " Directory"), DUMMY_INT);

	case S_IFREG:
		i = read(io, (char *) &magic, sizeof(magic));
		lseek(io, 0l, 0);
		if (i != sizeof(magic))
			break;
		switch (magic) {

		case 0737:	/* XCOFF executable (xxx) */
		case 0637:	/* XCOFF executable (RT) */
		case 0514:	/* 386 demand pagged executable */
		case 0403:	/* GPOFF executable */
		case 0405:	/* data overlay on exec */
		case 0407:	/* unshared */
		case 0410:	/* shared text */
		case 0411:	/* separate I/D */
		case 0413:	/* VM/Unix demand paged */
		case 0430:	/* PDP-11 Overlay shared */
		case 0431:	/* PDP-11 Overlay sep I/D */
			error(MSGSTR(M_092, " Executable"), DUMMY_INT);

/*
 *  Change the #if defined below to #if !defined to activate this code
 *  or place -MYDEBUG in makefile.  This code will then report on the
 *  magic number of any file which gets past the code above.  This is 
 *  useful in controling file editing in vi.
 *  See also /etc/magic for other magic numbers of interest.
 */
#if defined(MYDEBUG)
		default:
		ex_printf("%o octal %d dec %x hex %d bytes",magic,magic,magic,sizeof(magic));
		ex_printf(" Non-ascii file"); /* Magic number report */
#endif
		}
	}
	if (c != 'r') {
#if !defined(_SECURITY)
		if ((stbuf.st_mode & 0222) == 0 || access(file, 2) < 0) {
			ovro = value(READONLY);
			denied = 1;
			value(READONLY) = 1;
		}
#else
		privilege(PRIV_LAPSE);
		if ((stbuf.st_mode & 0222) == 0 ||
		    accessx(file, W_ACC, ACC_INVOKER) < 0) {
			ovro = value(READONLY);
			denied = 1;
			value(READONLY) = 1;
		}
		privilege(PRIV_ACQUIRE);
#endif
	}
	if (hush == 0 && value(READONLY)) {
		ex_printf(MSGSTR(M_095, " [Read only]"));
		flush();
	}
#ifdef  FLOCKFILE
	/*
	 * Attempt to lock the file. We use an sharable lock if reading
	 * the file, and an exclusive lock if editting a file.
	 * The lock will be released when the file is no longer being
	 * referenced. At any time, the editor can have as many as
	 * three files locked, and with different lock statuses.
	 */
	/*
	 * if this is either the saved or alternate file or current file,
	 * point to the appropriate descriptor and file lock status.
	 */
	if (strcmp (file,savedfile) == 0) {
		if (!io_savedfile) io_savedfile = dup(io) ;
		lp = &lock_savedfile ;  iop = &io_savedfile ;
	} else if (strcmp (file,altfile) == 0) {
		if (!io_altfile) io_altfile = dup(io) ;
		lp = &lock_altfile ;    iop = &io_altfile ;
	} else {
		/* throw away current lock, accquire new current lock */
		if (io_curr) close (io_curr) ;
		io_curr = dup(io) ;
		lp = &lock_curr ;       iop = &io_curr ;
		lock_curr = 0 ;
	}
	if (c == 'r' || value(READONLY) || *lp == 0) {
		/* if we have a lock already, don't bother */
		if (!*lp) {
			/* try for a shared lock */
			if (flock(*iop, LOCK_SH|LOCK_NB) < 0
			&& errno == EWOULDBLOCK) {
				ex_printf (MSGSTR(M_300, " [FILE BEING MODIFIED BY ANOTHER PROCESS]"));
				flush();
				goto fail_lock ;
			} else *lp = LOCK_SH ;
		}
	}
	if ( c != 'r'  && !value(READONLY) && *lp != LOCK_EX) {
		/* if we are editting the file, upgrade to an exclusive lock. */
		if (flock(*iop, LOCK_EX|LOCK_NB) < 0 && errno == EWOULDBLOCK) {
			ex_printf (MSGSTR(M_301, " [File open by another process]"));
			flush();
		} else *lp = LOCK_EX ;
	}
fail_lock:
#endif	/* FLOCKFILE */
	/* capture the name of the original file */
	if (r == 0)
		strcpy(origfile,file);
#ifdef TRACE
	if (trace)
		fprintf(trace,"\nrop: file %s, origfile %s\n",file,origfile);
#endif
#if defined(_SECURITY)
	/* get upto date acl of original file */
	if (r == 0) {
		if ((aclp = acl_fget(io)) == NULL)
			syserror(0);
/* AIX security enhancement */
#if !defined(TVI)
	 	/* Put acl of edited file on temporary file in case of crash */
	 	if (acl_fput(tfile, aclp, 0) == -1)
		 	syserror(0);
#endif
/*TCSEC Division C Class C2 */
	}
#endif	/* _SECURITY */
	if (c == 'r')
		setdot();
	else
		setall();
	if (FIXUNDO && inopen && c == 'r') {
		undap1 = undap2 = addr2 + 1;
		dot = addr2;
	}
	rop2();
	rop3(c);
}

void
rop2(void)
{
/* AIX security enhancement */
#if !defined(TVI)
	line *first, *last;
	line *a;
#endif
/* TCSEC Division C Class C2 */

	deletenone();
	clrstats();
/* AIX security enhancement */
#if !defined(TVI)
	first = addr2 + 1;
#endif
/* TCSEC Division C Class C2 */
	ignore(append(getfile, addr2));
	/*
	 * If the modeline variable is set, check the first and last five
	 * lines of the file for a mode setting line.
	 */
/* AIX security enhancement */
#if !defined(TVI)
	last = dot;
	if (value(MODELINE)) {
		for (a=first; a<=last; a++) {
			if (a==first+5 && last-first > 10)
				a = last - 4;
			getline(*a);
			checkmodeline(linebuf);
		}
	}
#endif
/* TCSEC Division C Class C2 */
}

void
rop3(int c)
{
	if (iostats() == 0 && c == 'e')
		edited++;
	if (c == 'e') {
		if (samefile && !fpat) {
			register line *addr = zero + samedot;

			if (addr > dol)
				addr = dol;
			if (addr >= one) {
				dot = addr;
				markpr(addr);
			} else
				goto other;
		} else if (wasalt || fpat) {
			register line *addr = zero + oldadot;

			if (addr > dol)
				addr = dol;
			if (fpat) {
				static wchar_t s_dollar[2] = { '$', 0 };
				globp = (*fpat) ? fpat : s_dollar;
				ex_sync();
				commands((short)1,(short)1);
				fpat = NULL;
			} else if (addr >= one) {
				if (inopen)
					dot = addr;
				markpr(addr);
			} else
				goto other;
		} else
other:
			if (dol > zero) {
				if (inopen)
					dot = one;
				markpr(one);
			}
		if(FIXUNDO)
			undkind = UNDNONE;
		if (inopen) {
			vcline = 0;
			vreplace(0, lines, lineDOL());
		}
	}
	if (laste) {
		tlaste();
		laste = 0;
		ex_sync();
	}
}

/*
 * Are these two really the same inode?
 */
static int
samei(struct stat *sp, char *cp)
{
	struct stat stb;

	if (stat(cp, &stb) < 0 || sp->st_dev != stb.st_dev)
		return (0);
	return (sp->st_ino == stb.st_ino);
}

/*
 * Write a file.
 */
void
wop(short dofname)
/* if 1 call filename, else use savedfile */
{
#if defined(_SECURITY)
	char *thisaclp;	/* a local acl pointer only */
#endif
	register int c, ex_clam, nonexist;
	line *saddr1, *saddr2;
	struct stat stbuf;
/* AIX security enhancement */
#if defined(TVI)
	char	backup[FNSIZE];
#endif
/* TCSEC Division C Class C2 */

#ifdef TRACE
		if (trace)
			fprintf(trace,"entering wop: file %s, origfile %s\n",file,origfile);
#endif
	c = 0;
	ex_clam = 0;
	if (dofname) {
		if (peekchar() == '!')
			ex_clam++, ignchar();
		ignore(skipwh());
		while (peekchar() == '>')
			ignchar(), c++, ignore(skipwh());
		if (c != 0 && c != 2)
			error(MSGSTR(M_096, "Write forms are 'w' and 'w>>'"), DUMMY_INT);
		filename('w');	/* writes the filename on the cmd line */
	} else {
		if (savedfile[0] == 0)
			error(MSGSTR(M_097, "No file|No current filename"), DUMMY_INT);
		saddr1=addr1;
		saddr2=addr2;
		addr1=one;
		addr2=dol;
		strcpy(file, savedfile); /* Grab original file name */
		if (inopen) {
			vclrech((short)0);
			splitw++;
		}
		lprintf("\"%s\"", file);
	}
	nonexist = stat(file, &stbuf);
#ifdef TRACE
		if (trace)
			fprintf(trace,"wop at nonexist: file %s, origfile %s\n",file,origfile);
#endif
/* AIX security enhancement */
#if defined (TVI)
	if (!nonexist)
	{
		char	*slash;
		char	*p;
		char	*q;

		/*
		 * create backup file name.
		 * if original file name is:
		 *	foo,
		 * backup is:
		 *	.foo
		 * if original file name is:
		 *	/abc/def/foo
		 * backup is:
		 *	/abc/def/.foo
		 */
		p = file;
		q = backup;
		slash = NULL;
		/*
		 * copy <file> name to <backup>, remembering the
		 * position of the last '/' in <backup>
		 */
		while (1)
		{
			*q = *p++;
			if (*q == '/')
				slash = q;
			if (*q == '\0')
				break;
			q++;
		}
		/*
		 * if there was no '/',
		 * the first character of the backup name is '.'
		 * and we copy all of the original <file> name
		 */
		if (slash == NULL)
		{
			p = file;
			q = backup;
		}
		/*
		 * the first character after the last '/' in <backup>
		 * should be '.'.
		 * then we will copy the last component of the original
		 * <file> name after this
		 */
		else
		{
			q = slash + 1;
			p = file + (q - backup);
		}
		*q++ = 'T';
		*q++ = 'V';
		*q++ = 'I';
		while (*q++ = *p++);

		/*
		 * copy the contents of <file> to <backup>
		 */
		{
			int	newfd;
			int	oldfd;

			if ((oldfd = open (file, O_RDONLY)) == -1)
				/* shouldn't fail here - just don't do backup */
				syserror(0);
			if ((newfd = open (backup, O_WRONLY | O_CREAT | O_TRUNC, 0400)) == -1)
			{
				close (oldfd);
				syserror(0);
			}
			while (1)
			{
				int	cnt;
				char	buf[BUFFERSIZ];

				cnt = read (oldfd, buf, sizeof (buf));
				if (cnt <= 0)
					break;
				if (write (newfd, buf, cnt) != cnt)
				{
					int	sav_errno;
					extern	int	errno;

					sav_errno = errno;
					unlink (backup);
					close (newfd);
					close (oldfd);
					errno = sav_errno;
					syserror(0);
				}
			}
#ifdef TRACE
		if (trace)
			fprintf(trace,"wop in tvi: newfd %d, file %s, origfile %s\n",newfd,file,origfile);
#endif
#if defined(_SECURITY)
			/* put acl of original file on file to be written */
		 	if ((acl_fput(newfd,aclp,0)) == -1)
				syserror(0); 
#endif
			fsync (newfd);
			close (newfd);
			close (oldfd);
		auditwrite ("tvi", 0, "update", sizeof("update"), file, strlen(file)+1,0);
		}
	}
#endif
/* TCSEC Division C Class C2 */
	switch (c) {

	case 0:
		if (!ex_clam && (!value(WRITEANY) || value(READONLY))) {
			if (edfile()) {
				if (value(READONLY)) {
					exit_status = 1;
					error(MSGSTR(M_099, " File is read only"), DUMMY_INT);
				}
			} else if (!nonexist) {
				if (((stbuf.st_mode & S_IFMT) != S_IFCHR) ||
				    (!samei(&stbuf, "/dev/null") &&
				     !samei(&stbuf, "/dev/tty"))) {
					io = open(file, 1);
					if (io < 0)
						syserror(0);
					if (!isatty(io))
						serror(MSGSTR(M_098, " File exists| File exists - use \"w! %s\" to overwrite"), file);
					close(io);
				}
			}
#ifdef TRACE
if (trace)
	fprintf(trace,"wop at edfile(): file %s, origfile %s\n",file,origfile);
#endif
		}
cre:
#ifdef TRACE
		if (trace)
			fprintf(trace,"wop at cre: file %s, origfile %s\n",file,origfile);
#endif
		io = creat(file, 0666);
		if (io < 0)
			syserror(0);
		writing = 1;
#ifdef TRACE
		if (trace)
			fprintf(trace,"wop after writing = 1: file %s, origfile %s\n",file,origfile);
#endif
		if (hush == 0)
			if (nonexist)
				ex_printf(MSGSTR(M_102, " [New file]"));
			else if (value(WRITEANY) && !edfile())
				ex_printf(MSGSTR(M_103, " [Existing file]"));
		break;

	case 2:

#ifdef TRACE
		if (trace)
			fprintf(trace, "wop at case2 \n");
#endif

		io = open(file, 1);
		if (io < 0) {
			if (ex_clam || value(WRITEANY))
				goto cre;
			syserror(0);
		}
#if (defined (TVI) && defined(_SECURITY))
		if ((thisaclp = acl_get(origfile)) == NULL)
			syserror(0);
		/* put current acl of original file on file to be written */
		if ((acl_put(file,thisaclp,1)) == -1)
			if (errno != EPERM)
				syserror(0);
#endif
#ifdef TRACE
		if (trace)
			fprintf(trace,"wop at case2 break: file %s, origfile %s\n",file,origfile);
#endif
		lseek(io, 0l, 2);
		break;
	}
#ifdef TRACE
		if (trace)
			fprintf(trace,"wop before putfile(): file %s, origfile %s\n",file,origfile);
#endif
	putfile(0);
#ifdef TRACE
		if (trace)
			fprintf(trace,"wop after putfile(): file %s, origfile %s\n",file,origfile);
#endif
	fsync(io);
/* AIX security enhancement */
#if defined (TVI)
	unlink (backup);
#endif
/* TCSEC Division C Class C2 */
	ignore(iostats());
	if (c != 2 && addr1 == one && addr2 == dol) {
		if (eq(file, savedfile))
			edited = 1;
		ex_sync();
	}
	if (!dofname) {
		addr1 = saddr1;
		addr2 = saddr2;
	}
	writing = 0;
}

/*
 * Is file the edited file?
 */
static int
edfile(void)
{
	return (edited && eq(file, savedfile));
}

/*
 * Extract the next line from the io stream.
 */
int
getfile(void)
{
	register wchar_t c;
	register char *fp;
	register wchar_t *lp;
	int char_len;
	wchar_t pwcptr;
	register int keep = 0;
	static char inbuf[BUFFERSIZ];
	static char *nextip, *ep;
	register char *Ep;
	register Ni;
	register mb_cur_max = MB_CUR_MAX;

	lp = linebuf;
	fp = nextip;
	Ep = ep;
	Ni = ninbuf;

	do {
		if (--Ni < 0) {
			Ni = read(io, inbuf + keep, BUFFERSIZ - keep) -1;
				/* ninbuf is -1 because the count is dec at start */
			if (Ni < 0) {
				if (lp != linebuf) {
					lp++;
					ex_printf(MSGSTR(M_104, " [Incomplete last line]"));
					break;
				}

				ninbuf = 0;
				return (EOF);
			}
			fp = inbuf;		/* set pointer to start of buffer */
			cntch += Ni +1;	/* byte count not char count */
			/* Ni now should reflect bytes in buffer */
			Ni += keep;
			Ep = ep = &inbuf[Ni];		/* set endbuf pointer */
			keep = 0;
		}
		if (mb_cur_max == 1) {
			c = (wchar_t) *fp++;
			if (!c) {
				++cntnull;
				continue;
			}
 		} else if ((char_len = mbtowc(&pwcptr, fp, mb_cur_max)) > 0
			   && (Ni -( char_len -1)) >= 0) {
				fp += char_len;
				c = pwcptr;
				Ni -= char_len -1;
		} else if (Ep - fp >= mb_cur_max){
 			if(char_len == 0){
 				/* it's a null so  get rid of it */
				/* increment nul count for command line */
				cntnull++;
 			}
 			else {
 				/* more characters following so it isn't a partial read */
 				ex_printf(MSGSTR(M_652, "Incomplete or invalid multibyte character, conversion failed."), DUMMY_INT); 
			}
 				fp++;
 				continue;	/* don't write char or null to the linebuffer */
 		}
		else {
			/* partial read move the char(s) to the start of the buffer */
			keep = Ep - fp + 1;	/* number of bytes there */
			strncpy((char *)inbuf, fp, (size_t)keep);	
			Ni = 0;		/* force a read for next bytes */
			continue;
		}

		if (junk(c))
			continue;
		*lp++ = c;	/* add the wide character to the line buffer */

		/* Defect 35814 - If 2048 chars have already been stored in  */
		/* linebuf, make sure one was an \n character.  If not, then */
		/* the line is too long.				     */
		if (lp > &linebuf[LBSIZE] || (lp == &linebuf[LBSIZE] && c != '\n')) {
			ninbuf = Ni;
			ep = Ep;
			error(MSGSTR(M_105, " Line too long"), DUMMY_INT);
		}
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	cntln++;
	ninbuf = Ni;
	ep = Ep;
	return (0);
}

/*
 * Write a range onto the io stream.
 */
void
putfile(int isfilter)
{
	line *a1;
	register char *fp, *lp;
	register int nib;
	char outbuf[BUFFERSIZ];
	char tmpbuf[sizeof(linebuf) * MB_LEN_MAX];
	wchar_t olb[LBSIZE];

	a1 = addr1;
	clrstats();
	cntln = addr2 - a1 + 1;
	if (cntln == 0)
		return;
	nib = BUFFERSIZ;
	fp = outbuf;
/* Save linebuf in olb to fix problem with autowrite on ^A command */
#ifdef TRACE
		if (trace)
			fprintf(trace,"putfile() at entry: file %s, origfile %s\n",file,origfile);
#endif
	CP(olb, linebuf);
	do {
		getline(*a1++);

		if (wcstombs(tmpbuf, linebuf, sizeof(tmpbuf)) == -1)
			error(MSGSTR(M_651, "Invalid wide character string, conversion failed."), DUMMY_INT);

		lp = tmpbuf;
		for (;;) {
			if (--nib < 0) {
				nib = fp - outbuf;
				if (write(io, outbuf, nib) != nib) {
					wrerror();
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
	CP(linebuf, olb);
#ifdef TRACE
		if (trace)
			fprintf(trace,"in putfile(): io %s, file %s, origfile %s\n",io,file,origfile);
#endif
 	if (write(io, outbuf, nib) != nib || (!isfilter && fsync(io))) {
		wrerror();
	}
	cntch += nib;
}

/*
 * A write error has occurred;	if the file being written was
 * the edited file then we consider it to have changed since it is
 * now likely scrambled.
 */
static void wrerror(void)
{

#ifdef TRACE
		if (trace)
			fprintf(trace,"wrerror() at entry: file %s, origfile %s\n",file,origfile);
#endif
	if (eq(file, savedfile) && edited)
		change();
	syserror(1);
}

/*
 * Source command, handles nested sources.
 * Traps errors since it mungs unit 0 during the source.
 */
short slevel = 0;
short ttyindes;

void
source(char *fil, short okfail)
{
	jmp_buf osetexit;
	register int saveinp, ointty, oerrno;
	wchar_t *saveglobp;
	int savepeekc;				/* code point */
/* AIX security enhancement */
#if defined(TVI)
	if (tcb("/usr/bin/tvi",TCB_QUERY) == TCB_ON){
		if ((!trusted_input) || tcb(fil,TCB_QUERY) == TCB_OFF)
			return;
	}
#endif
/* TCSEC Division C Class C2 */
	signal(SIGINT, SIG_IGN);
	saveinp = dup(0);
	savepeekc = peekc;
	saveglobp = globp;
	peekc = 0; globp = 0;
	if (saveinp < 0)
		error(MSGSTR(M_106, "Too many nested sources"), DUMMY_INT);
	if (slevel <= 0)
		ttyindes = saveinp;
	close(0);
	if (open(fil, 0) < 0) {
		oerrno = errno;
		setrupt();
		dup(saveinp);
		close(saveinp);
		errno = oerrno;
		if (!okfail)
			filioerr(fil);
		return;
	}
	slevel++;
	ointty = intty;
	intty = isatty(0);
	oprompt = value(PROMPT);
	value(PROMPT) &= intty;
	getexit(osetexit);
	setrupt();
	if (setexit() == 0)
		commands((short)1, (short)1);
	else if (slevel > 1) {
		close(0);
		dup(saveinp);
		close(saveinp);
		slevel--;
		resexit(osetexit);
		reset();
	}
	intty = ointty;
	value(PROMPT) = oprompt;
	close(0);
	dup(saveinp);
	close(saveinp);
	globp = saveglobp;
	peekc = savepeekc;
	slevel--;
		resexit(osetexit);
}

/*
 * Clear io statistics before a read or write.
 */
void
clrstats(void)
{

	ninbuf = 0;
	cntch = 0;
	cntln = 0;
	cntnull = 0;
}

/*
 * Io is finished, close the unit and print statistics.
 * Messages modified to remove (s) usage for translation.
 */
static short iostats(void)
{
	close(io);
	io = -1;
	if (hush == 0) {
		if (value(TERSE))
			ex_printf(MSGSTR(M_107, " %d/%ld"), cntln, cntch);
		else {
			if (cntln == 1 && cntch == 1)
				ex_printf(MSGSTR(M_292, " %d line, %ld character"), cntln, cntch);
			if (cntln == 1 && cntch > 1)
				ex_printf(MSGSTR(M_295, " %d line, %ld characters"), cntln, cntch);
			if (cntln > 1 && cntch > 1)
				ex_printf(MSGSTR(M_296, " %d lines, %ld characters"), cntln, cntch);
		}
		if (cntnull == 1) {
			ex_printf(MSGSTR(M_109, " (%ld null)"), cntnull);
		}
		if (cntnull > 1) {
			ex_printf(MSGSTR(M_297, " (%ld nulls)"), cntnull);
	
		}
		noonl();
		flush();
	}
	return (cntnull != 0);
}


/* AIX security enhancement */
#if !defined(TVI)

static void checkmodeline(wchar_t *ln)
{
	wchar_t *beg, *end;
	wchar_t cmdbuf[1024];
        int savpc;
        short savle;

	beg = (wchar_t *)wcschr(ln, ':');
	if (beg == NULL || beg < &ln[3])
		return;
	if (beg[-3] != ' ' && beg[-3] != '\t') return;
	if (!(beg[-2] == 'e' && beg[-1] == 'x') &&
	    !(beg[-2] == 'v' && beg[-1] == 'i')) return;
	(void)wcsncpy(cmdbuf, beg+1, WCSIZE (cmdbuf));
	end = (wchar_t *)wcsrchr(cmdbuf, ':');
	if (end == NULL)
		return;
	*end = 0;
        savpc = peekc; peekc = 0;
        savle = laste;
	globp = cmdbuf;
	commands((short)1, (short)1);
        peekc = savpc;
        laste = savle;
}
#endif
/* TCSEC Division C Class C2 */
