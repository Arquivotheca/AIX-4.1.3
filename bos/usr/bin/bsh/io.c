static char sccsid[] = "@(#)03  1.31  src/bos/usr/bin/bsh/io.c, cmdbsh, bos41J, 9507C 2/8/95 10:29:53";
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS: initf estabf push pop pushtemp poptemp chkpipe chkopen sh_rename
 *	      create tmpfile copy link_iodocs swap_iodoc_nm savefd restore
 *
 * ORIGINS: 3, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
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
 * 1.18  com/cmd/sh/sh/io.c, , bos320, 9134320 8/12/91 19:52:31
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

#include	"defs.h"
#include	<fcntl.h>
#include	<errno.h>
#include 	<sys/mode.h>
#include	<string.h>
#include	"sym.h"

short topfd;

/* ========	input output and file copying ======== */

initf(fd)
int	fd;
{
	register struct fileblk *f = standin;

	f->fdes = fd;
	f->fsiz = ((flags & oneflg) == 0 ? SH_BUFSIZ : 1);
	f->fnxt = f->fend = f->fbuf;
	f->feval = 0;
	f->flin = 1;
	f->sh_feof = FALSE;
	f->fraw = FALSE;
}

estabf(s)
register uchar_t *s;
{
	register struct fileblk *f;

	(f = standin)->fdes = -1;
	f->fend = length(s) + (f->fnxt = s);
	f->flin = 1;

	/* fraw flag says s is already encoded, otherwise readc() encodes */
	/* only macro() sets this to TRUE after calling estabf()         */

	f->fraw = FALSE;
	return(f->sh_feof = (s == 0));
}

push(af)
struct fileblk *af;
{
	register struct fileblk *f;

	(f = af)->fstak = standin;
	f->sh_feof = 0;
	f->feval = 0;
	standin = f;
}

pop()
{
	register struct fileblk *f;

	if ((f = standin)->fstak)
	{
		if (f->fdes >= 0)
			close(f->fdes);
		standin = f->fstak;
		return(TRUE);
	}
	else
		return(FALSE);
}

struct tempblk *tmpfptr;

pushtemp(fd,tb)
	int fd;
	struct tempblk *tb;
{
	tb->fdes = fd;
	tb->fstak = tmpfptr;
	tmpfptr = tb;
}

poptemp()
{
	if (tmpfptr)
	{
		close(tmpfptr->fdes);
		tmpfptr = tmpfptr->fstak;
		return(TRUE);
	}
	else
		return(FALSE);
}
	
chkpipe(pv)
int	*pv;
{
	if (pipe(pv) < 0 || pv[INPIPE] < 0 || pv[OUTPIPE] < 0)
		error(MSGSTR(M_PIPERR,(char *)piperr));
}

chkopen(idf)
uchar_t *idf;
{
	register int	rc;

	if ((rc = open((char *)idf, O_RDONLY)) < 0)
		failed(idf, MSGSTR(M_BADOPEN,(char *)badopen));
	else
		return(rc);
}

sh_rename(f1, f2)
register int	f1, f2;
{
	if (f1 != f2)
	{
		dup2(f1, f2);
		close(f1);
		if (f2 == 0)
			ioset |= 1;
	}
}

create(s)
uchar_t *s;
{
	register int	rc;

	if ((rc = creat((char *)s, (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH))) < 0)
		failed(s, MSGSTR(M_BADCREATE,(char *)badcreate));
	else
		return(rc);
}

tmpfil(tb)
	struct tempblk *tb;
{
	int fd = -1;
	size_t sz = 0; /* string size of converted serial check digit */
	int try = 0;
	struct stat sb; /* dummy buffer for result of lstat() call */

	do {
		settmp();
		/* add check digit to tmpname string for uniqueness within a process */
		itos(serial++);

		/* determine the size of the string representing the check digit */
		sz = strlen(numbuf);
		movstr(numbuf, tempname);

		/* move the buffer pointer the size of the string just added */
		tempname = tempname + sz;

		/* add pid to tmpname string for uniqueness between processes */
		itos(getpid());
		movstr(numbuf, tempname);

		/* make sure that tmpout does not exist already */
		if (access(tmpout, F_OK) == 0 || lstat (tmpout, &sb) == 0)
			continue;

		/* create file rw-rw-rw, but only if it didn't exist already */
		if ((fd = open ((char *) tmpout,
			(O_WRONLY|O_CREAT|O_TRUNC|O_EXCL),
			(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH))) < 0)
			failed(tmpout, MSGSTR(M_BADCREATE,(char *)badcreate));

	} while (fd == -1 && try++ < 10);
	pushtemp(fd,tb);
	return(fd);
}

/*
 * set by trim
 */
extern BOOL		nosubst;
#define			CPYSIZ		512

copy(ioparg)
struct ionod	*ioparg;
{
	register uchar_t	*cline;
	register uchar_t	*clinep;
	register struct ionod	*iop;
	uchar_t	c;
	uchar_t	*ends = NULL;
	uchar_t	*start;
	int rc;
	char *r1, *r2;
	int		fd;
	int		i;
	int		stripflg;
	
/*
 *      Copy stdin to iop, a temporary file.
 *      "nosubst" is set if terminator (ends) is quoted e.g. cat<<"EOF"
 *      If nosubst is set, temp file is decoded and will be read directly
 *      by command it is being passed to.  If it is reset, temp file is
 *      encoded, and read later by subst() which does substition.
 *      (Why the shell doesn't do substitution now and be done with it
 *      is beyond me, but I'm not going to experiment...)
 */

	if (iop = ioparg)
	{
		struct tempblk tb;

		copy(iop->iolst);
		/* Check for the presence of a grave.  If it is present */
		/* then don't call mactrim(). 				*/

		r1=strchr(iop->ioname, '`');
		r2=strchr(iop->ioname, '$');
		if (r1 == NULL && r2 == NULL)
			ends = mactrim(iop->ioname);
		else
			ends=iop->ioname;

#ifdef	_OSF
		/* The following routine will reset nosubst */
		trim(ends);
#endif	/* _OSF */
		if (nosubst)
			NLSdecode(ends);
		else
			ends += 1;      /* skip FNLS chartacter */
		stripflg = iop->iofile & IOSTRIP;
		if (nosubst)
			iop->iofile &= ~IODOC;
		fd = tmpfil(&tb);

		/*
		 * allocate all iop's from the heap
		 */
		iop->ioname = make(tmpout);

		iop->iolst = iotemp;
		iotemp = iop;

		cline = clinep = start = locstak();
		if (stripflg)
		{
			iop->iofile &= ~IOSTRIP;
			while (*ends == '\t')
				ends++;
		}
		for (;;)
		{
			chkpr();
			if (nosubst)
			{
				/* mark new string as encoded */
				needmem(clinep);
				*clinep++ = FNLS;
				c = readc();
				if (stripflg)
					while (c == '\t')
						c = readc();

				while (!eolchar(c))
				{
					needmem(clinep);
					*clinep++ = c;
					c = readc();
				}
				needmem(clinep);
				*clinep = 0;
				NLSdecode(cline);
				clinep = cline + strlen(cline);
			}
			else
			{
				c = nextc(*ends);
				if (stripflg)
					while (c == '\t')
						c = nextc(*ends);
				
				while (!eolchar(c))
				{
					needmem(clinep);
					*clinep++ = c;
					c = nextc(*ends);
				}
			}

			needmem(clinep);
			*clinep = 0;

			if (eof || eq(cline, ends))
			{
				if ((i = cline - start) > 0)
					if ( write(fd, start, i) < 0 && errno == ENOSPC )
                        		{
                                		perror("bsh");
                                		exitsh(ERROR);
                        		}
				break;
			}
			else {
				needmem(clinep);
				*clinep++ = NL;
			}
			if ((i = clinep - start) < CPYSIZ)
				cline = clinep;
			else
			{
				if ( write(fd, start, i) < 0 && errno == ENOSPC )
                        	{
                               		perror("bsh");
                               		exitsh(ERROR);
                        	}
				cline = clinep = start;
			}
		}

		poptemp();		/* pushed in tmpfil -- bug fix for problem
					   deleting in-line scripts */
	}
}


link_iodocs(i)
	struct ionod	*i;
{
	while(i)
	{
		free(i->iolink);
		itos(serial++);
		movstr(numbuf, tempname);
		i->iolink = make(tmpout);
		link(i->ioname, i->iolink);

		i = i->iolst;
	}
}


swap_iodoc_nm(i)
	struct ionod	*i;
{
	while(i)
	{
		free(i->ioname);
		i->ioname = i->iolink;
		i->iolink = 0;

		i = i->iolst;
	}
}


savefd(fd)
	int fd;
{
	register int	f;

	f = fcntl(fd, F_DUPFD, USERIO);
	return(f);
}


restore(last)
	register int	last;
{
	register int 	i;
	register int	dupfd;

	for (i = topfd - 1; i >= last; i--)
	{
		if ((dupfd = fdmap[i].dup_fd) > 0)
			sh_rename(dupfd, fdmap[i].org_fd);
		else
			close(fdmap[i].org_fd);
	}
	topfd = last;
}

