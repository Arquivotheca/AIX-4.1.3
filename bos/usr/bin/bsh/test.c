static char sccsid[] = "@(#)20  1.17  src/bos/usr/bin/bsh/test.c, cmdbsh, bos411, 9428A410j 11/30/93 14:05:20";
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS: test nxtarg exp e1 e2 e3 tio ftype filtyp fsizep bfailed
 *
 * ORIGINS: 3, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
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
 * 1.9  com/cmd/sh/sh/test.c, cmdsh, bos324
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

/*
 *      test expression
 *      [ expression ]
 */

#include	"defs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/access.h>
#include <sys/priv.h>

int	ap, ac;
uchar_t	**av;

test(argn, com)
uchar_t	*com[];
int	argn;
{
	ac = argn;
	av = com;
	ap = 1;
	if (eq(com[0],"["))
	{
		if (!eq(com[--ac], "]"))
			failed("test", MSGSTR(M_RBRACKET,"] missing"));
	}
	com[ac] = 0;
	if (ac <= 1)
		return(1);
	return(exp() ? 0 : 1);
}

uchar_t *
nxtarg(mt)
{
	if (ap >= ac)
	{
		if (mt)
		{
			ap++;
			return(0);
		}
		failed("test", MSGSTR(M_ARGEXP,"argument expected"));
	}
	return(av[ap++]);
}

exp()
{
	int	p1;
	uchar_t	*p2;

	p1 = e1();
	p2 = nxtarg(1);
	if (p2 != 0)
	{
		if (eq(p2, "-o"))
			return(p1 | exp());

		if (eq(p2, "]") && !eq(p2, ")"))
			failed("test", MSGSTR(M_SYNMSG,(char *)synmsg));
	}
	ap--;
	return(p1);
}

e1()
{
	int	p1;
	uchar_t	*p2;

	p1 = e2();
	p2 = nxtarg(1);

	if ((p2 != 0) && eq(p2, "-a"))
		return(p1 & e1());
	ap--;
	return(p1);
}

e2()
{
	if (eq(nxtarg(0), "!"))
		return(!e3());
	ap--;
	return(e3());
}

e3()
{
	int	p1;
	register uchar_t	*a;
	uchar_t	*p2;
	long	atol();
	long	int1, int2;

	a = nxtarg(0);
	if (eq(a, "("))
	{
		p1 = exp();
		if (!eq(nxtarg(0), ")"))
			failed("test",MSGSTR(M_RPAREN,") expected"));
		return(p1);
	}
	p2 = nxtarg(1);
	ap--;
	if ((p2 == 0) || (!eq(p2, "=") && !eq(p2, "!=")))
	{
		if (eq(a, "-r"))
			return(tio(nxtarg(0), R_ACC));
		if (eq(a, "-w"))
			return(tio(nxtarg(0), W_ACC));
		if (eq(a, "-x"))
			return(tio(nxtarg(0), X_ACC));
		if (eq(a, "-e"))
			return(fexist(nxtarg(0)));
		if (eq(a, "-d"))
			return(filtyp(nxtarg(0), S_IFDIR));
		if (eq(a, "-c"))
			return(filtyp(nxtarg(0), S_IFCHR));
		if (eq(a, "-b"))
			return(filtyp(nxtarg(0), S_IFBLK));
		if (eq(a, "-f"))
			return(filtyp(nxtarg(0), S_IFREG));
		if (eq(a, "-u"))
			return(ftype(nxtarg(0), S_ISUID));
		if (eq(a, "-g"))
			return(ftype(nxtarg(0), S_ISGID));
		if (eq(a, "-k"))
			return(ftype(nxtarg(0), S_ISVTX));
		if (eq(a, "-p"))
			return(filtyp(nxtarg(0),S_IFIFO));
   		if (eq(a, "-s"))
			return(fsizep(nxtarg(0)));
		if (eq(a, "-t"))
		{
			if (ap >= ac)		/* no args */
				failed("test", MSGSTR(M_ARGEXP,"argument expected"));
			else if (eq((a = nxtarg(0)), "-a") || eq(a, "-o"))
			{
				ap--;
				return(isatty(1));
			}
			else
			{
				int ret;
				char *ptr;

				errno = 0;
				ret = strtol(a, &ptr, 10);
				/* check for numeric number or partially
                                   converted */
				if (errno || strcmp(a, ptr) == 0 || 
				    *ptr != (char *)NULL)
					return(0);
				else
					return(isatty(ret));
			}
		}
                if (eq(a, "-L"))
                {
                        struct stat statb;
                        if (lstat((char *)nxtarg(0), &statb) < 0)
                                return(0);
                        return((statb.st_mode&S_IFMT)==S_IFLNK);
                }
		if (eq(a, "-n"))
			return(!eq(nxtarg(0), ""));
		if (eq(a, "-z"))
			return(eq(nxtarg(0), ""));
	}

	p2 = nxtarg(1);
	if (p2 == 0)
		return(!eq(a, ""));
	if (eq(p2, "-a") || eq(p2, "-o"))
	{
		ap--;
		return(!eq(a, ""));
	}
	if (eq(p2, "="))
		return(eq(nxtarg(0), a));
	if (eq(p2, "!="))
		return(!eq(nxtarg(0), a));
	int1 = atol(a);
	int2 = atol(nxtarg(0));
	if (eq(p2, "-eq"))
		return(int1 == int2);
	if (eq(p2, "-ne"))
		return(int1 != int2);
	if (eq(p2, "-gt"))
		return(int1 > int2);
	if (eq(p2, "-lt"))
		return(int1 < int2);
	if (eq(p2, "-ge"))
		return(int1 >= int2);
	if (eq(p2, "-le"))
		return(int1 <= int2);

	bfailed(btest, MSGSTR(M_BADOP,(char *)badop), p2);
/* NOTREACHED */
}

tio(name, mode)
uchar_t	*name;
int	mode;
{
	struct stat statb;
	static  uid_t  euid, egid;

	euid = geteuid();
	egid = getegid();

	if(*name==0)
		/* null statement */ ;

	/*
	 *  POSIX 1003.2-d11.2
	 *     -r file   True if file exists and is readable.
	 *     -w file   True if file exists and is writable.
	 *               True shall indicate only that the write flag is on.
	 *               The file shall not be writable on a read-only file
	 *               system even if this test indicates true.
	 *     -x file   True if file exists and is executable.
	 *               True shall indicate only that the execute flag is on.
	 *               If file is a directory, true indicates that the
	 *               file can be searched.
	 */
	else if( mode != W_OK && mode != X_OK )
	{
		if ( access(name,mode) == 0 )
			return(1);

	}
	else if(stat(name, &statb) == 0)
	{
		if ( access(name,mode) == 0 )
		{
			if(S_ISDIR(statb.st_mode) && mode == X_OK)
				return(1);

			/* needs permission for someone */
			if(mode == W_OK)
				mode = (S_IWRITE|(S_IWRITE>>3)|(S_IWRITE>>6));
			else
				mode = (S_IEXEC|(S_IEXEC>>3)|(S_IEXEC>>6));
		}
		else if(euid == statb.st_uid)
			mode <<= 6;

		else if(egid == statb.st_gid)
			mode <<= 3;

		else
		{
			/* you can be in several groups */
			int n = NGROUPS_MAX;
			gid_t groups[NGROUPS_MAX];

			n = getgroups(n,groups);
			while(--n >= 0)
				if(groups[n] == statb.st_gid)
				{
					mode <<= 3;
					break;
				}
		}

		if(statb.st_mode & mode)
			return(1);
	}
	return(0);
}


ftype(f, field)
uchar_t	*f;
int	field;
{
	struct stat statb;

	if (stat((char *)f, &statb) < 0)
		return(0);
	if ((statb.st_mode & field) == field)
		return(1);
	return(0);
}

filtyp(f,field)
uchar_t	*f;
int field;
{
	struct stat statb;

	if (stat((char *)f, &statb) < 0)
		return(0);
	if ((statb.st_mode & S_IFMT) == field)
		return(1);
	else
		return(0);
}



fsizep(f)
uchar_t	*f;
{
	struct stat statb;

	if (stat((char *)f, &statb) < 0)
		return(0);
	return(statb.st_size > 0);
}

fexist(f)
uchar_t *f;
{
        struct stat statb;

        if (lstat((char *)f, &statb) == 0)
                return(1);
        else
                return(0);
}

/*
 * fake diagnostics to continue to look like original
 * test(1) diagnostics
 */
bfailed(s1, s2, s3) 
uchar_t	*s1;
uchar_t	*s2;
uchar_t	*s3;
{
	prp();
	prs(s1);
	if (s2)
	{
		prs(colon);
		prs(s2);
		prs(s3);
	}
	newline();
	exitsh(ERROR);
}
