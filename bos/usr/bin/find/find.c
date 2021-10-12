static char sccsid[] = "@(#)40	1.70  src/bos/usr/bin/find/find.c, cmdscan, bos412, 9446B 11/15/94 20:11:34";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27, 71
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
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */

#include <strings.h>
#include <string.h>
#include <unistd.h>
#include	<stdio.h>
#include	<pwd.h>
#include	<grp.h>
#include	<utmp.h>
#include	<fshelp.h>
#include        <sys/stat.h>
#include        <sys/errno.h>
#include        <sys/fullstat.h>
#include        <sys/types.h>
#include        <sys/param.h>
#include        <sys/vfs.h>
#include        <dirent.h>
#include        <sys/utsname.h>
#include        <sys/param.h>
#include	<locale.h>
#include 	<sys/signal.h>
#include 	<langinfo.h>
#include	<stdlib.h>
#include	<fnmatch.h>
#include 	"find_msg.h"
nl_catd catd;
static char	*ystr;
#define MSGSTR(Num, Str) catgets(catd,MS_FIND, Num, Str)

#define RPT_BLK_SZ	512		/* P1003.2/Draft 11 now says 512 */
					/* used to be 1024, with comments:*/
					/* Used to be UBSIZE, but we
					 * need to report in 1024 not 512
					 */
#define A_DAY	86400L /* a day full of seconds */
#define EQ(x, y)	(strcmp(x, y)==0)
#define BUFSIZE	512	/* In u370 I can't use BUFSIZ nor BSIZE */
#define CPIOBSZ	4096
#define Bufsize	5120

static char	Pathname[MAXPATHLEN+1];
#define DEV_BSHIFT      9               /* log2(DEV_BSIZE) */
/* DEV_BSIZE is in sys/dir.h, included from dirent.h */
#define dbtob(db)               /* calculates (db * DEV_BSIZE) */ \
        ((unsigned)(db) << DEV_BSHIFT)

#define MAXNODES	100
#define PREVDIR		".."       /* A13989 */

#define UUID     1
#define GGID     2

static int	Randlast;
static struct anode {
	int (*F)();
	struct anode *L, *R;
} Node[MAXNODES];
static int Nn=0;  /* number of nodes */
static char	*Fname;
static char    fstyped;         /* -fstype option specified */
static char    Fstype[BUFSIZE];
static long	Now;
static int	Argc,
	Ai,
	Pi;
static char	**Argv;
/* cpio stuff */
static int	Cpio;
static short	*SBuf, *Dbuf, *Wp;
static char	*Buf, *Cbuf, *Cp;
static char	Strhdr[500],
	*Chdr = Strhdr;
static int	Wct = Bufsize / 2, Cct = Bufsize;
static int	Cflag;
static int	depthf = 0;
static struct  xutsname xname;
static int	printflag;
static int	execflag;
static int	okflag;
static int	exeq(), ok(), findglob(),  mtime(), atime(), lctime(), user(),
	group(), size(), perm(), links(), print(), prune(),
	type(), ino(), depth(), nnode(), cpio(), newer(), fstype(),
	ls(), crossdev(), nouser(), nogroup();
static int	and(), or(), not();
static void	child_err();

static int	exit_status = 0;
static int     pruned = 0;    		/* TRUE if -prune flag was specified	      */
static int	Xdev = 1;		/* true if SHOULD cross devices (filesystems) */
static struct	stat	Devstat;	/* stats of each argument path's file system  */

/* struct stat Statb;   */
static struct fullstat Statb;
static struct fullstat Statb2;

static struct	anode	*exp(),
		*e1(),
		*e2(),
		*e3(),
		*mk();
static char	*nxtarg();
static char	Home[MAXPATHLEN + 1];
static long	Blocks;
static	lctime();
static char	*whereami();
static char	*getname(uid_t);
static char	*getgroup(gid_t);
static struct	vfs_ent *vfsp;
static int	atoint(char *, char *);
static void usage();

/*
 * SEE ALSO:	updatedb, bigram.c, code.c
 *		Usenix ;login:, February/March, 1983, p. 8.
 *
 * REVISIONS: 	James A. Woods, Informatics General Corporation,
 *		NASA Ames Research Center, 6/81.
 *
 *		The second form searches a pre-computed filelist
 *		(constructed nightly by /usr/lib/crontab) which is
 *		compressed by updatedb (v.i.z.)  The effect of
 *			find <name>
 *		is similar to
 *			find / +0 -name "*<name>*" -print
 *		but much faster.
 *
 *		8/82 faster yet + incorporation of bigram coding -- jaw
 *
 *		1/83 incorporate glob-style matching -- jaw
 */
#undef	AMES			/* Not supporting FAST FIND */
	static struct header {
		short	h_magic,
			h_dev;
		ushort	h_ino,
			h_mode,
			h_uid,
			h_gid;
		short	h_nlink,
			h_rdev,
			h_mtime[2],
			h_namesize,
			h_filesize[2];
		char	h_name[256];
	} hdr;


static struct  utmp utmp;
#define NMAX    (sizeof (utmp.ut_name))
#define SCPYN(a, b)     strncpy(a, b, NMAX)

#define NUID    6400
#define NGID    300

static struct ncache {
        int     uid;
        char    name[NMAX+1];
} nc[NUID];
static char    groups[NGID][NMAX+1];
static char    outrangegroup[NMAX+1];
static int     outrangegid = -1;

main(argc, argv) 
int argc;
char *argv[];
{
	struct anode *exlist;
	int paths;
	register char *cp, *sp = 0;

#ifdef  AMES
	if (argc < 2) 
		usage();
	if (argc == 2) {
		fastfind(argv[1]);
		exit(exit_status);
	}
#endif  /* AMES */
	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_FIND,NL_CAT_LOCALE);
	ystr = MSGSTR(YES,"yes");
	signal(SIGUSR1, child_err);
	time(&Now);
	unamex(&xname);
        if(getcwd(Home,sizeof(Home)) == 0) {
                perror("pwd");
                exit(2);
	}
	Argc = argc; Argv = argv;
	if(argc < 2) 
		usage();

	for(Ai = paths = 1; Ai < argc; ++Ai, ++paths)
		if(*Argv[Ai] == '-' || *Argv[Ai] == '(' || *Argv[Ai] == '!')
			break;
	if(paths == 1) /* no path-list */
		usage();

        if (Ai == argc) {
                /*
                 * 'find paths...' without any selectors
                 */
                exlist = mk(print, (struct anode *)0, (struct anode *)0);
                printflag++;

        } else if(!(exlist = exp())) { /* parse and compile the arguments */
		fprintf(stderr,MSGSTR( PARSERR, "find: parsing error\n"));
		exit(1);
	}
       if(!okflag && !execflag && !printflag)
                /*
                 * POSIX gives you free -print, when no -exec, -ok or -print present
                 */
                exlist = mk(and, exlist, mk(print,(struct anode *)0,(struct anode *)0));

	if(Ai<argc) {
		fprintf(stderr, MSGSTR( CONJ, "find: missing conjunction\n"));
		exit(1);
	}
	for(Pi = 1; Pi < paths; ++Pi) {
		sp = NULL;
		strcpy(Pathname, Argv[Pi]);
		if(*Pathname != '/')
			chdir_access(Home);
		if(cp = strrchr(Pathname, '/')) {
			sp = cp + 1;
			*cp = '\0';
			if(chdir_access(*Pathname? Pathname: "/") == -1) {
				fprintf(stderr,MSGSTR( BADSTART, "find: bad starting directory\n"));
				exit_status++;
				continue;
			}
			*cp = '/'; /* ??? */
		}
		Fname = sp ? sp: Pathname;
		if ( '\0' == *Fname )
			Fname = ".";
		
		if (fstyped || !Xdev) {
			if( lstat(Fname,  &Devstat)<0) {
				fprintf(stderr,MSGSTR( BADSTAT, "find: bad status-- %s\n"), Pathname);
				exit_status++;
				continue;
			}

			if (fstyped) {
				if (vfsp = getvfsbytype(Devstat.fst_vfstype))
					strcpy(Fstype, vfsp->vfsent_name);
				else
					Fstype[0] = '\0';
			}
		}


		/* to find files that match  */
		descend(Pathname, Fname, Fstype, 0, exlist);
	}
	if(Cpio) {
		strcpy(Pathname, "TRAILER!!!");
		Statb.st_size = 0;
		cpio();
		printf("%ld blocks\n", Blocks*10);
	}
	exit(exit_status);
}

/* compile time functions:  priority is  exp()<e1()<e2()<e3()  */

static struct anode *exp() /* parse ALTERNATION (-o)  */
{
	register struct anode * p1;

	p1 = e1() /* get left operand */ ;
	if(EQ(nxtarg(), "-o")) {
		Randlast--;
		return(mk(or, p1, exp()));
	}
	else if(Ai <= Argc) --Ai;
	return(p1);
}
static struct anode *e1()  /* parse CONCATENATION (formerly -a) */
{
	register struct anode * p1;
	register char *a;

	p1 = e2();
	a = nxtarg();
	if(EQ(a, "-a")) {
And:
		Randlast--;
		return(mk(and, p1, e1()));
	} else if(EQ(a, "(") || EQ(a, "!") || (*a=='-' && !EQ(a, "-o"))) {
		--Ai;
		goto And;
	} else if(Ai <= Argc) --Ai;
	return(p1);
}
static struct anode *e2()  /* parse NOT (!) */
{
	if(Randlast) {
		fprintf(stderr,MSGSTR( OPFOP, "find: operand follows operand\n"));
		exit(1);
	}
	Randlast++;
	if(EQ(nxtarg(), "!"))
		return(mk(not, e3(), (struct anode *)0));
	else if(Ai <= Argc) --Ai;
	return(e3());
}
static struct anode *e3()  /* parse parens and predicates */
{
	struct anode *p1;
	int i;
	unsigned long n, getnid();
	register char *a, *b;
	register int  s=0x0000;

	a = nxtarg();
	if(EQ(a, "(")) {
		Randlast--;
		p1 = exp();
		a = nxtarg();
		if(!EQ(a, ")")) goto err;
		return(p1);
	}
	else if(EQ(a, "-print")) {
		printflag++;
		return(mk(print, (struct anode *)0, (struct anode *)0));
	}
	else if (EQ(a, "-nouser")) {
		return (mk(nouser, (struct anode *)0, (struct anode *)0));
	}
        else if (EQ(a, "-ls")) {
		printflag++;			/* Avoid double-printing */
                return (mk(ls, (struct anode *)0, (struct anode *)0));
        }
		/*  prune in this instance will mean, 
 		 *  don't descend directories if -prune was evaluated
		 */
	else if(EQ(a, "-prune")) {
		return(mk(prune, (struct anode *)0, (struct anode *)0));
	}
	else if (EQ(a, "-nogroup")) {
		return (mk(nogroup, (struct anode *)0, (struct anode *)0));
	}
	else if(EQ(a, "-xdev")) {
		Xdev = 0;
		return(mk(crossdev, (struct anode *)0, (struct anode *)0));
	}
	else if(EQ(a, "-depth")) {
		depthf = 1;
		return(mk(depth, (struct anode *)0, (struct anode *)0));
	}
	b = nxtarg();
	s = *b;
	if(EQ(a, "-name"))
		return(mk(findglob, (struct anode *)b, (struct anode *)0));
	else if(EQ(a, "-perm")) {
		char *tail;
		if(s=='-') b++;
		i = strtoul(b,&tail,8);
		if( tail && *tail ) {		/* Not an octal number! */
		  i = permissions(b);		/* Get the o+rw,g=u syntax */
		}
		return(mk(perm, (struct anode *)i, (struct anode *)s));
	}
	else if(EQ(a, "-type")) {
		i = s=='d' ? S_IFDIR :
		    s=='b' ? S_IFBLK :
		    s=='c' ? S_IFCHR :
		    s=='p' ? S_IFIFO :
		    s=='f' ? S_IFREG :
		    s=='l' ? S_IFLNK :
		    s=='s' ? S_IFSOCK :
		    0;
		return(mk(type, (struct anode *)i, (struct anode *)0));
	}
	else if(EQ(a, "-fstype")) {
		fstyped = 1;
		return(mk(fstype, (struct anode *)b, (struct anode *)0));
	}
	else if (EQ(a, "-exec")) {
		execflag++;
		i = Ai - 1;
		while(!EQ(nxtarg(), ";"));
		return(mk(exeq, (struct anode *)i, (struct anode *)0));
	}
	else if (EQ(a, "-ok")) {
		okflag++;
		i = Ai - 1;
		while(!EQ(nxtarg(), ";"));
		return(mk(ok, (struct anode *)i, (struct anode *)0));
	}
	else if(EQ(a, "-cpio")) {
		if((Cpio = creat(b, 0666)) < 0) {
			fprintf(stderr,MSGSTR( NOCREATE, "find: cannot create %s\n"), b);
			exit(1);
		}
		SBuf = (short *)sbrk(CPIOBSZ);
		Wp = Dbuf = (short *)sbrk(Bufsize);
		depthf = 1;
		return(mk(cpio, (struct anode *)0, (struct anode *)0));
	}
	else if(EQ(a, "-ncpio")) {
		if((Cpio = creat(b, 0666)) < 0) {
			fprintf(stderr,MSGSTR( NOCREATE, "find: cannot create %s\n"), b);
			exit(1);
		}
		Buf = (char*)sbrk(CPIOBSZ);
		Cp = Cbuf = (char *)sbrk(Bufsize);
		Cflag++;
		depthf = 1;
		return(mk(cpio, (struct anode *)0, (struct anode *)0));
	}
	else if(EQ(a, "-newer")) {
		time_t	*Newer = (time_t *) sbrk (sizeof *Newer);

		/*
		 * Save the modification time of this file (which must exist
		 * and which is only checked once) for future comparisions
		 * against other files.
		 */

		if(stat(b, (struct stat *)&Statb) < 0) {
			fprintf(stderr,MSGSTR(NOACCES,"find: cannot access %s\n"), b);
			exit(1);
		}
		*Newer = Statb.st_mtime;
		return mk(newer, (struct anode *) Newer, (struct anode *)0);
	} else if(EQ(a, "-node")) {
		if((n=getnid(b)) == -1) {
			fprintf(stderr,MSGSTR( BADNODE, "find: invalid node id %s\n"), b);
			exit(1);
		}
		return(mk(nnode, (struct anode *)n, (struct anode *)0));
	}

	/*
	 * The remaining primaries take the 'n' argument which is an integer
	 * optionally preceeded by (amoung other things) a "+" character.
	 * Be carefull to add primaries which do not take the 'n' argument
	 * above this code, since the 'b' variable might be modified.
	 */
	if(s=='+') b++;
	if(EQ(a, "-mtime"))
		return(mk(mtime, (struct anode *)atoint(a,b), (struct anode *)s));
	else if(EQ(a, "-atime"))
		return(mk(atime, (struct anode *)atoint(a,b), (struct anode *)s));
	else if(EQ(a, "-ctime"))
		return(mk(lctime, (struct anode *)atoint(a,b), (struct anode *)s));
	else if(EQ(a, "-user")) {
		if((i=getunum(UUID, b)) == -1) {
			if ((*b) && 
			   (((*b >= '0') && (*b <= '9')) ||
			   ((*b == '-') && (*(b+1) >= '0') && (*(b+1) <= '9'))))
				return mk(user, (struct anode *)atoint(a,b), (struct anode *)s);
			fprintf(stderr, MSGSTR( NOUSER, "find: cannot find -user name\n"));
			exit(1);
		}
		return(mk(user, (struct anode *)i, (struct anode *)s));
	}
	else if(EQ(a, "-inum") || EQ(a, "-i"))
		return(mk(ino, (struct anode *)atoint(a,b), (struct anode *)s));
	else if(EQ(a, "-group")) {
		if((i=getunum(GGID, b)) == -1) {
			if ((*b) &&
			   (((*b >= '0') && (*b <= '9')) ||
			   ((*b == '-') && (*(b+1) >= '0') && (*(b+1) <= '9'))))
				return mk(group, (struct anode *)atoint(a,b), (struct anode *)s);
			fprintf(stderr,MSGSTR( NOGROUP, "find: cannot find -group name\n"));
			exit(1);
		}
		return(mk(group, (struct anode *)i, (struct anode *)s));
	}
	else if(EQ(a, "-size"))
		return(mk(size, (struct anode *)atoint(a,b), 
				(struct anode *)(s|(strrchr(b,'c')?0x7f<<8:0)|(strrchr(b,'k')?0xf0<<8:0))));
	else if(EQ(a, "-links"))
		return(mk(links, (struct anode *)atoint(a,b), (struct anode *)s));
err:	fprintf(stderr,MSGSTR( BADOPTION, "find: bad option %s\n"), a);
	exit(1);
}
static struct anode *mk(f, l, r)
int (*f)();
struct anode *l, *r;
{
	if (Nn >= MAXNODES) {
		fprintf(stderr, MSGSTR(TOOMANYOPTS,"find: too many options\n"));
		exit(1);
	}

	Node[Nn].F = f;
	Node[Nn].L = l;
	Node[Nn].R = r;
	return(&(Node[Nn++]));
}

static char *nxtarg()  /* get next arg from command line */
{
	static strikes = 0;

	if(strikes==3) {
		fprintf(stderr,MSGSTR( INCSTATE, "find: incomplete statement\n"));
		exit(1);
	}
	if(Ai>=Argc) {
		strikes++;
		Ai = Argc + 1;
		return("");
	}
	return(Argv[Ai++]);
}

/* execution time functions */
static
and(p)
register struct anode *p;
{
	return(((*p->L->F)(p->L)) && ((*p->R->F)(p->R))?1:0);
}
static
or(p)
register struct anode *p;
{
	 return(((*p->L->F)(p->L)) || ((*p->R->F)(p->R))?1:0);
}
static
not(p)
register struct anode *p;
{
	return( !((*p->L->F)(p->L)));
}
static
findglob(p)
register struct { int f; char *pat; } *p; 
{	/* fnmatch() returns a zero if Fname matches pattern */
	return(!fnmatch(p->pat, Fname, FNM_PERIOD));
}
static
print()
{
	puts(Pathname);
	return(1);
}
static
mtime(p)
register struct { int f, t, s; } *p; 
{
	if (!(p->t) && (p->s=='0') && ((Now + A_DAY - Statb.st_mtime) < 0))
		return (0);	/* kludge for future-matching */
	else
		return(scomp((int)( ((Now + A_DAY) - Statb.st_mtime) / A_DAY),
								 p->t, p->s));
}
static
atime(p)
register struct { int f, t, s; } *p; 
{
	if (!(p->t) && (p->s=='0') && ((Now + A_DAY - Statb.st_atime) < 0))
		return (0);	/* kludge for future-matching */
	else
		return(scomp((int)( ((Now + A_DAY) - Statb.st_atime) / A_DAY),
								 p->t, p->s));
}
static
lctime(p)
register struct { int f, t, s; } *p; 
{
	if (!(p->t) && (p->s=='0') && ((Now + A_DAY - Statb.st_ctime) < 0))
		return (0);	/* kludge for future-matching */
	else
		return(scomp((int)( ((Now + A_DAY) - Statb.st_ctime) / A_DAY),
								 p->t, p->s));
}
static
user(p)
register struct { int f, u, s; } *p; 
{
	if (xname.nid != Statb.fst_nid) {
		Statb.st_uid = p->u;
		Statb.st_gid = 0;

		if( lstat(Fname, &Statb)<0) {
			fprintf(stderr,MSGSTR( BADSTAT, "find: bad status-- %s\n"), Fname);
			exit_status++;
			return(0);
		}
	}

	return(scomp(Statb.st_uid, p->u, p->s));
}
static
nouser(p)
struct anode *p;
{
	return (getname(Statb.st_uid) == (char *) NULL);
}
static
ino(p)
register struct { int f, u, s; } *p;
{
	return(scomp((int)Statb.st_ino, p->u, p->s));
}

static
group(p)
register struct { int f, u, s; } *p; 
{
	if (xname.nid != Statb.fst_nid) {
		Statb.st_uid = 0;
		Statb.st_gid = p->u;

		if( lstat(Fname,  &Statb)<0) {
			fprintf(stderr,MSGSTR( BADSTAT, "find: bad status-- %s\n"), Fname);
			exit_status++;
			return(0);
		}
	}

	return(scomp(Statb.st_gid, p->u, p->s));
}
static
nogroup(p)
struct anode *p;
{

	return (getgroup(Statb.st_gid) == NULL);
}
static
links(p)
register struct { int f, link, s; } *p; 
{
	return(scomp(Statb.st_nlink, p->link, p->s));
}
static
size(p)
register struct { int f, sz, s; } *p; 
{ int temp=Statb.st_size;

			/* p->s >> 8 == 0  --> temp = # of 512-byte blocks  */
			/* p->s >> 8 == 7f --> temp = # of 1-byte blocks    */
			/* p->s >> 8 == f0 --> temp = # of 1024 byte blocks */
			/* RPT_BLK_SZ = 512 				    */
	if (p->s>>8 == 0)	
	     temp = (temp+(RPT_BLK_SZ - 1))/RPT_BLK_SZ;
	else
		if (p->s>>8 == 0xf0)
		     temp = (temp+(2*RPT_BLK_SZ - 1))/(2*RPT_BLK_SZ);
	return(scomp(temp, p->sz, (char)p->s));
}
static
perm(p)
register struct { int f, per, s; } *p; 
{
	register i;
	i = (p->s=='-') ? p->per : 0740007777; /* '-' means only arg bits */
	return((Statb.st_mode & i & 0740007777) == p->per);
}
static
type(p)
register struct { int f, per, s; } *p;
{
	return((Statb.st_mode&S_IFMT)==p->per);
}
static
fstype(p)
register struct { int f; char *fsname; } *p;
{
	return(Fstype && !strcmp(Fstype, p->fsname));
}
static
prune(p)
register struct { int f, per, s; } *p;
{
	pruned = 1;
	return(1);
}
static
exeq(p)
register struct { int f, com; } *p;
{
	fflush(stdout); /* to flush possible `-print' */
	return(doex(p->com));
}
static
ok(p)
struct { int f, com; } *p;
{
	int yes=0;
	char c[LINE_MAX+1];

	fflush(stdout); /* to flush possible `-print' */
	fprintf(stderr, MSGSTR(OKPROMT, "< %s ... %s > ? "),
			    Argv[p->com], Pathname);
	fflush(stderr);
	if (gets(c) == NULL)
		exit(2);
	if (rpmatch(c) == 1)
		yes = 1;
	return(yes? doex(p->com): 0);
}

#define MKSHORT(v, lv) {U.l=1L;if(U.c[0]) U.l=lv, v[0]=U.s[1], v[1]=U.s[0]; else U.l=lv, v[0]=U.s[0], v[1]=U.s[1];}
static union { long l; short s[2]; char c[4]; } U;
static
long mklong(v)
short v[];
{
	U.l = 1;
	if(U.c[0] /* VAX */)
		U.s[0] = v[1], U.s[1] = v[0];
	else
		U.s[0] = v[0], U.s[1] = v[1];
	return U.l;
}

static
crossdev()
{
	return(1);
}

static
depth()
{
	return(1);
}

static
nnode(p)
register struct { int f, nid; } *p;
{
	return(p->nid == Statb.fst_nid);
}

static
cpio()
{
#define MAGIC 070707
#define HDRSIZE	(sizeof hdr - 256)
#define CHARS	76
	register ifile, ct;
	static long fsz;
	register i;

	strcpy(hdr.h_name, !strncmp(Pathname, "./", 2)? Pathname+2: Pathname);
	hdr.h_magic = MAGIC;
	hdr.h_namesize = strlen(hdr.h_name) + 1;
	hdr.h_uid = Statb.st_uid;
	hdr.h_gid = Statb.st_gid;
	hdr.h_dev = Statb.st_dev;
	hdr.h_ino = Statb.st_ino;
	hdr.h_mode = Statb.st_mode;
	hdr.h_nlink = Statb.st_nlink;
	hdr.h_rdev = Statb.st_rdev;
	MKSHORT(hdr.h_mtime, Statb.st_mtime);
	fsz = (hdr.h_mode & S_IFMT) == S_IFREG? Statb.st_size: 0L;
	MKSHORT(hdr.h_filesize, fsz);

	if (Cflag)
		bintochar(fsz);

	if(EQ(hdr.h_name, "TRAILER!!!")) {
		Cflag? writehdr(Chdr, CHARS + hdr.h_namesize):
			bwrite((short *)&hdr, HDRSIZE + hdr.h_namesize);
		for (i = 0; i < 10; ++i)
			Cflag? writehdr(Buf, BUFSIZE): bwrite(SBuf, BUFSIZE);
		return;
	}
	if(!mklong(hdr.h_filesize)) {
		Cflag? writehdr(Chdr, CHARS + hdr.h_namesize):
			bwrite((short *)&hdr, HDRSIZE + hdr.h_namesize);
		return;
	}
	if((ifile = open(Fname, 0)) < 0) {
cerror:
		fprintf(stderr,MSGSTR( NOCOPY, "find: cannot copy %s\n"), hdr.h_name);
		exit_status++;
		return;
	}
	Cflag? writehdr(Chdr, CHARS + hdr.h_namesize):
		bwrite((short *)&hdr, HDRSIZE+hdr.h_namesize);
	for(fsz = mklong(hdr.h_filesize); fsz > 0; fsz -= CPIOBSZ) {
		ct = fsz>CPIOBSZ? CPIOBSZ: fsz;
		if(read(ifile, Cflag? Buf: (char *)SBuf, ct) < 0)  {
			fprintf(stderr,MSGSTR( NOREAD, "Cannot read %s\n"), hdr.h_name);
			exit_status++;
			continue;
		}
		Cflag? writehdr(Buf, ct): bwrite(SBuf, ct);
	}
	close(ifile);
	return 1;
}

static
bintochar(t)
long t;
{
	sprintf(Chdr, "%.6ho%.6ho%.6ho%.6ho%.6ho%.6ho%.6ho%.6ho%.11lo%.6ho%.11lo%s",
		MAGIC,Statb.st_dev,Statb.st_ino,Statb.st_mode,Statb.st_uid,
		Statb.st_gid,Statb.st_nlink,Statb.st_rdev & 00000177777,
		Statb.st_mtime,(short)strlen(hdr.h_name)+1,t,hdr.h_name);
}

static
ls(p)
struct anode *p;
{
        list(Pathname, &Statb);
        return (1);
}

static
newer(p)
struct anode *p;
{
	/*
	 * Compare modification time of current file to modification
	 * time for the file on the command line that was save in the
	 * node.
	 */

	return Statb.st_mtime > *(time_t *) p->L;
}

static
scomp(a, b, s) /* funny signed compare */
register a, b;
register char s;
{
	if(s == '+')
		return(a > b);
	if(s == '-')
		return(a <= (b * -1));
	return(a == b);
}

static caught;

static
void
catcher(sig)
{

	signal(sig, catcher);
	caught = sig;
}

static
void
child_err(sig)
{
	signal(sig, child_err);
	exit_status++;
}

static
doex(com)
{
	register np;
	register char *na;
	void (*old_quit)(), (*old_intr)();
	static char *nargv[50];
	static ccode;
	static pid;

	ccode = np = 0;
	while (na=Argv[com++]) {
		if(strcmp(na, ";")==0) break;
		if(strcmp(na, "{}")==0) nargv[np++] = Pathname;
		else nargv[np++] = na;
	}
	nargv[np] = 0;
	if (np==0) return(9);
	/*
	 * catch deadly signals, since the programme we exec may ignore them.
	 */
	old_quit = signal(SIGQUIT, catcher);
	old_intr = signal(SIGINT,  catcher);
	caught = 0;

	if(pid = fork())
		while(wait(&ccode) != pid);
	else { /*child*/
		chdir_access(Home);
		execvp(nargv[0], nargv);
		/* PTM # 34450 */
 		fprintf(stderr, MSGSTR(CANTEXEC,"find: cannot execute %s"), nargv[0]);
 		fflush(stdout);
 		perror(":");
		kill(getppid(), SIGUSR1); /* inform parent of exec failure */
		exit(1);
	}

	signal(SIGQUIT, old_quit);
	signal(SIGINT,  old_intr);

	if (caught)
		kill(0, caught);

	return(ccode ? 0:1);
}

static
getunum(t, s)
int	t;
char	*s;
{
	register i;
	struct	passwd	*pw;
	struct	group	*gr;

	i = -1;
	if( t == UUID ){
		if( ((pw = getpwnam( s )) != (struct passwd *)NULL) && pw != (struct passwd *)EOF )
			i = pw->pw_uid;
	} else {
		if( ((gr = getgrnam( s )) != (struct group *)NULL) && gr != (struct group *)EOF )
			i = gr->gr_gid;
	}
	return(i);
}

/************************************************************************/
static char    valid_chars[] = "0123456789abcdefABCDEF";

static unsigned long
getnid(nodename)
char *nodename;
{
	unsigned long nid;
	register char *testp;   /* used in nodename validation          */
	char * strchr();
	int length;             /* length of nodename                   */
	int i;                  /* counter and error returns            */

	/* is this nodename actually a nickname?                        */

	/* NOT AVAILABLE 
	i = drsname(nodename,&nid);
	*/
	return (-1);

	/*NOTREACHED*/
	if (i == 0)
		return(nid);      /* nodename translated into node id   */

	/* validate characters in nodename against allowable characters
	   in hex representation                                        */
	testp = (char *) ~0;
	if ((length = strlen(nodename)) != 8)
		return(-1);
	for (i=0; (i<length) && (testp); i++)
		testp = strchr( valid_chars, nodename[i] );
	if (!testp)
		return(-1);
	sscanf(nodename, "%x", &nid);
	return(nid);
}
/************************************************************************/
#define ROOT "/"

static descend(name, fname, pfstype, pfsno, exlist)
	struct anode *exlist;
	char *name, *fname;
	char *pfstype;		/* fstype  of parent dir */
	dev_t pfsno;		/* device number of parent dir */
{
	DIR	*dir = NULL;
	register struct dirent	*dp;
	register char *c1;
	int rv = 0;
	char *endofname;
	dev_t cfsno;
	char cfstype[BUFSIZE];
	char *wd; 		/* working directory */
	char backptr[PATH_MAX + 1];
	char savedFname[PATH_MAX + 1];

	strcpy(backptr,PREVDIR);
	if ( depthf )
		strcpy(savedFname,Fname);

	if( lstat(fname,  &Statb)<0) {
		fprintf(stderr,MSGSTR( BADSTAT, "find: bad status-- %s\n"), name);
		exit_status++;
		return(0);
	}
	cfsno = Statb.st_dev;
	if (fstyped) {
		if (cfsno != pfsno) {
			if( lstat(Fname,  &Statb2)<0) {
				fprintf(stderr,MSGSTR( BADSTAT, 
					"find: bad status-- %s\n"), Fname);
				exit_status++;
				return(0);
			}
			if (vfsp = getvfsbytype(Statb2.fst_vfstype))
				strcpy(Fstype,vfsp->vfsent_name);
			else
				return(0);

			/* the following line was removed in 1.5.1.1.2 */
			if (Devstat.st_vfstype != Statb.st_vfstype) 
					return(1);
		} else
			strcpy(Fstype, pfstype);
		strcpy(cfstype, Fstype);
	}

	/* set prunded to FLASE, if -prune is evaluated, it will be set to
        ** TRUE when exlist is evaluated.
        */
	if (!depthf)
		(*exlist->F)(exlist);

	if((Statb.st_mode&S_IFMT)!=S_IFDIR ||
	   !Xdev && Devstat.st_dev != Statb.st_dev){    
		/* don't need to reset pruned since we will return anyway */
		if (depthf)
			(*exlist->F)(exlist);
		return(1);
		}              

	/* the following line was different in 1.5.1.1.2 */
	if (pruned && !depthf) {
		pruned = 0;
		return(1);
	}

	for (c1 = name; *c1; ++c1);
	if (*(c1-1) == '/')
		--c1;
	endofname = c1;
	/* if filesystem is remote, save previous directory incase we can't
	** chdir("..")   A13989
	*/
	if (Statb.st_vfstype == FS_REMOTE) {
		strcpy(backptr,whereami());
	}
	if (chdir_access(fname) == -1) {
		wd=whereami();
		if (strcmp(ROOT,wd) == 0)
			fprintf(stderr,MSGSTR(NOCDROOT,"find: cannot chdir to </%s>"),fname);
		else

			fprintf(stderr,MSGSTR(NOCD,"find: cannot chdir to <%s/%s>"), wd, fname);
		exit_status++;
		perror(" ");
		return(0);
	}
	if ((dir = opendir(".")) == NULL) {
		fprintf(stderr, MSGSTR(NOOPEN,"find: cannot open < %s >\n"), name);
		exit_status++;
		rv = 0;
		goto ret;
	}
	for (dp = readdir(dir); dp != NULL; dp = readdir(dir)) {
		if ((dp->d_name[0]=='.' && dp->d_name[1]=='\0') ||
		    (dp->d_name[0]=='.' && dp->d_name[1]=='.' && dp->d_name[2]=='\0'))
			continue;
		c1 = endofname;
		*c1++ = '/';
		strcpy(c1, dp->d_name);
		Fname = endofname+1;
		if(!descend(name, Fname, cfstype, cfsno, exlist)) {
			*endofname = '\0';
		}
	}
	rv = 1;
ret:
	if(dir)
		closedir(dir);
	c1 = endofname;
        *c1 = '\0';
	if(chdir_access(backptr) == -1) {       /* A13989 */
		*endofname = '\0';
		fprintf(stderr, MSGSTR(BADDIR,"find: bad directory <%s>\n"), name);
		exit(1);
	}
	if(depthf){
		strcpy(Fname, savedFname);
		if( lstat(fname, &Statb)<0) {
			fprintf(stderr, MSGSTR(NOSTAT,"find: cannot get information about <%s>\n"), fname);
			exit_status++;
		}
		(*exlist->F)(exlist);
	}
	return(rv);
}

static bwrite(rp, c)
register short *rp;
register c;
{
	register short *wp = Wp;

	c = (c+1) >> 1;
	while(c--) {
		if(!Wct) {
again:
			if(write(Cpio, (char *)Dbuf, Bufsize)<0) {
				Cpio = chgreel(1, Cpio);
				goto again;
			}
			Wct = Bufsize >> 1;
			wp = Dbuf;
			++Blocks;
		}
		*wp++ = *rp++;
		--Wct;
	}
	Wp = wp;
}

static writehdr(rp, c)
register char *rp;
register c;
{
	register char *cp = Cp;

	while (c--)  {
		if (!Cct)  {
again:
			if(write(Cpio, Cbuf, Bufsize) < 0)  {
				Cpio = chgreel(1, Cpio);
				goto again;
			}
			Cct = Bufsize;
			cp = Cbuf;
			++Blocks;
		}
		*cp++ = *rp++;
		--Cct;
	}
	Cp = cp;
}

static chgreel(x, fl)
{
	register f;
	char str[22];
	FILE *devtty;
	struct stat statb;

	fprintf(stderr,( x ? MSGSTR( WRTOUT, "find: can't write output\n") 
		      	   : MSGSTR( READIN, "find: can't read input\n")));
	exit_status++;

	fstat(fl, &statb);
	if((statb.st_mode&S_IFMT) != S_IFCHR)
		exit(1);
again:
	fprintf(stderr,MSGSTR( ASKDEV, "If you want to go on, type device/file name when ready\n"));
	devtty = fopen("/dev/tty", "r");
	 fgets(str, 20, devtty);
	str[strlen(str) - 1] = '\0';
	if(!*str)
		exit(1);
	close(fl);
	if((f = open(str, x? 1: 0)) < 0) {
		fprintf(stderr,MSGSTR( NOGOOD, "That didn't work"));
		fclose(devtty);
		goto again;
	}
	return f;
}

#ifdef	AMES
/*
 * 'fastfind' scans a file list for the full pathname of a file
 * given only a piece of the name.  The list has been processed with
 * with "front-compression" and bigram coding.  Front compression reduces
 * space by a factor of 4-5, bigram coding by a further 20-25%.
 * The codes are:
 *
 *	0-28	likeliest differential counts + offset to make nonnegative 
 *	30	escape code for out-of-range count to follow in next word
 *	128-255 bigram codes, (128 most common, as determined by 'updatedb')
 *	32-127  single character (printable) ascii residue
 *
 * A novel two-tiered string search technique is employed: 
 *
 * First, a metacharacter-free subpattern and partial pathname is
 * matched BACKWARDS to avoid full expansion of the pathname list.
 * The time savings is 40-50% over forward matching, which cannot efficiently
 * handle overlapped search patterns and compressed path residue.
 *
 * Then, the actual shell glob-style regular expression (if in this form)
 * is matched against the candidate pathnames using the slower routines
 * provided in the standard 'find'.
 */

#define	FCODES 	"/usr/lib/find/find.codes"
#define	YES	1
#define	NO	0
#define	OFFSET	14
#define	ESCCODE	30

static fastfind ( pathpart )	
	char pathpart[];
{
	register char *p, *s;
	register int c; 
	char *q, *index(), *patprep();
	int i, count = 0, globflag;
	FILE *fp, *fopen();
	char *patend, *cutoff;
	char path[1024];
	char bigram1[128], bigram2[128];
	int found = NO;

	if ( (fp = fopen ( FCODES, "r" )) == NULL ) {
		fprintf ( stderr, MSGSTR(CANTOPEN,"find: cannot open %s\n"), FCODES );
		exit ( 1 );
	}
	for ( i = 0; i < 128; i++ ) 
		bigram1[i] = getc ( fp ),  bigram2[i] = getc ( fp );
	
	if ( index ( pathpart, '*' ) || index ( pathpart, '?' ) || index ( pathpart, '[' ) )
		globflag = YES;
	patend = patprep ( pathpart );

	c = getc ( fp );
	for ( ; ; ) {

		count += ( (c == ESCCODE) ? getw ( fp ) : c ) - OFFSET;

		for ( p = path + count; (c = getc ( fp )) > ESCCODE; )	/* overlay old path */
			if ( c < 0200 )	
				*p++ = c;
			else		/* bigrams are parity-marked */
				*p++ = bigram1[c & 0177],  *p++ = bigram2[c & 0177];
		if ( c == EOF )
			break;
		*p-- = NULL;
		cutoff = ( found ? path : path + count);

		for ( found = NO, s = p; s >= cutoff; s-- ) 
			if ( *s == *patend ) {		/* fast first char check */
				for ( p = patend - 1, q = s - 1; *p != NULL; p--, q-- )
					if ( *q != *p )
						break;
				if ( *p == NULL ) {	/* success on fast match */
					found = YES;
					if ( globflag == NO || !fnmatch(pathparth, path, FNM_PERIOD))  /* fnmatch() returns a 0 if it matches */
						puts ( path );
					break;
				}
			}
	}
}

/*
    extract last glob-free subpattern in name for fast pre-match;
    prepend '\0' for backwards match; return end of new pattern
*/
static char globfree[100];

static char *
patprep ( name )
	char *name;
{
	register char *p, *endmark;
	register char *subp = globfree;

	*subp++ = '\0';
	p = name + strlen ( name ) - 1;
	/*
	   skip trailing metacharacters (and [] ranges)
	*/
	for ( ; p >= name; p-- )
		if ( index ( "*?", *p ) == 0 )
			break;
	if ( p < name )
		p = name;
	if ( *p == ']' )
		for ( p--; p >= name; p-- )
			if ( *p == '[' ) {
				p--;
				break;
			}
	if ( p < name )
		p = name;
	/*
	   if pattern has only metacharacters,
	   check every path (force '/' search)
	*/
	if ( (p == name) && index ( "?*[]", *p ) != 0 )
		*subp++ = '/';					
	else {				
		for ( endmark = p; p >= name; p-- )
			if ( index ( "]*?", *p ) != 0 )
				break;
		for ( ++p; (p <= endmark) && subp < (globfree + sizeof ( globfree )); )
			*subp++ = *p++;
	}
	*subp = '\0';
	return ( --subp );
}
#endif

/* rest should be done with nameserver or database */



/*
 * This function assumes that the password file is hashed
 * (or some such) to allow fast access based on a name key.
 * If this isn't true, duplicate the code for getgroup().
 */
static char *
getname(uid_t uid)
{
	register struct passwd *pw;
	struct passwd *getpwent();
	register int cp;
#ifdef	NO_PW_STAYOPEN

	_pw_stayopen = 1;
#endif

#if	(((NUID) & ((NUID) - 1)) != 0)
	cp = uid % (NUID);
#else
	cp = uid & ((NUID) - 1);
#endif
	if (nc[cp].uid == uid && nc[cp].name[0])
		return (nc[cp].name);
	pw = getpwuid(uid);
	if (!pw)
		return (0);
	nc[cp].uid = uid;
	SCPYN(nc[cp].name, pw->pw_name);
	return (nc[cp].name);
}

static char *
getgroup(gid_t gid)
{
	register struct group *gr;
	static init;

	if (gid < NGID && groups[gid][0])
		return (&groups[gid][0]);
	if (gid == outrangegid)
		return (outrangegroup);
rescan:
	if (init == 2) {
		if (gid < NGID)
			return (0);
		setgrent();
		while (gr = getgrent()) {
			if (gr->gr_gid != gid)
				continue;
			outrangegid = gr->gr_gid;
			SCPYN(outrangegroup, gr->gr_name);
			endgrent();
			return (outrangegroup);
		}
		endgrent();
		return (0);
	}
	if (init == 0)
		setgrent(), init = 1;
	while (gr = getgrent()) {
		if (gr->gr_gid >= NGID) {
			if (gr->gr_gid == gid) {
				outrangegid = gr->gr_gid;
				SCPYN(outrangegroup, gr->gr_name);
				return (outrangegroup);
			}
			continue;
		}
		if (groups[gr->gr_gid][0])
			continue;
		SCPYN(groups[gr->gr_gid], gr->gr_name);
		if (gr->gr_gid == gid)
			return (&groups[gid][0]);
	}
	init = 2;
	goto rescan;
}

#define permoffset(who)		((who) * 3)
#define permission(who, type)	((type) >> permoffset(who))
#define kbytes(bytes)		(((bytes) + 1023) / 1024)

static list(file, stp)
	char *file;
	register struct stat *stp;
{
	char pmode[32], uname[32], gname[32], fsize[32], ftime[32];
	char *ctime();
	static long special[] = { S_ISUID, 's', S_ISGID, 's', S_ISVTX, 't' };
	static time_t sixmonthsago = -1;
#ifdef	S_IFLNK
	char flink[MAXPATHLEN + 1];
#endif
	register int who;
	register char *cp;
	time_t now;

	if (file == NULL || stp == NULL)
		return (-1);

	time(&now);
	if (sixmonthsago == -1)
		sixmonthsago = now - 6L*30L*24L*60L*60L;

	switch (stp->st_mode & S_IFMT) {
#ifdef	S_IFDIR
	case S_IFDIR:	/* directory */
		if(stp->st_vfstype != FS_REMOTE)
                pmode[0] = 'd';
		else
                pmode[0] = 'D';
		break;
#endif
#ifdef	S_IFCHR
	case S_IFCHR:	/* character special */
		pmode[0] = 'c';
		break;
#endif
#ifdef	S_IFBLK
	case S_IFBLK:	/* block special */
		pmode[0] = 'b';
		break;
#endif
#ifdef	S_IFLNK
	case S_IFLNK:	/* symbolic link */
		pmode[0] = 'l';
		break;
#endif
#ifdef	S_IFSOCK
	case S_IFSOCK:	/* socket */
		pmode[0] = 's';
		break;
#endif
#ifdef	S_IFREG
	case S_IFREG:	/* regular */
#endif
	default:
		if(stp->st_vfstype != FS_REMOTE)
                pmode[0] = '-';
		else
                pmode[0] = 'F';
		break;
	}

	for (who = 0; who < 3; who++) {
		if (stp->st_mode & permission(who, S_IREAD))
			pmode[permoffset(who) + 1] = 'r';
		else
			pmode[permoffset(who) + 1] = '-';

		if (stp->st_mode & permission(who, S_IWRITE))
			pmode[permoffset(who) + 2] = 'w';
		else
			pmode[permoffset(who) + 2] = '-';

		if (stp->st_mode & special[who * 2])
			pmode[permoffset(who) + 3] = special[who * 2 + 1];
		else if (stp->st_mode & permission(who, S_IEXEC))
			pmode[permoffset(who) + 3] = 'x';
		else
			pmode[permoffset(who) + 3] = '-';
	}
	pmode[permoffset(who) + 1] = '\0';

	cp = getname(stp->st_uid);
	if (cp != NULL)
		sprintf(uname, "%-9.9s", cp);
	else
		sprintf(uname, "%-9d", stp->st_uid);

	cp = getgroup(stp->st_gid);
	if (cp != NULL)
		sprintf(gname, "%-9.9s", cp);
	else
		sprintf(gname, "%-9d", stp->st_gid);

	if (pmode[0] == 'b' || pmode[0] == 'c')
		sprintf(fsize, "%3d,%4d",
			major(stp->st_rdev), minor(stp->st_rdev));
	else {
		sprintf(fsize, "%8ld", stp->st_size);
#ifdef	S_IFLNK
		if (pmode[0] == 'l') {
			/*
			 * Need to get the tail of the file name, since we have
			 * already chdir()ed into the directory of the file
			 */
			cp = rindex(file, '/');
			if (cp == NULL)
				cp = file;
			else
				cp++;
			who = readlink(cp, flink, sizeof flink - 1);
			if (who >= 0)
				flink[who] = '\0';
			else
				flink[0] = '\0';
		}
#endif
	}

	cp = ctime(&stp->st_mtime);
	if (stp->st_mtime < sixmonthsago || stp->st_mtime > now)
		sprintf(ftime, "%-7.7s %-4.4s", cp + 4, cp + 20);
	else
		sprintf(ftime, "%-12.12s", cp + 4);

	printf("%5lu %4ld %s %2d %s%s%s %s %s%s%s\n",
		stp->st_ino,				/* inode #	*/
#ifdef	S_IFSOCK
		(long) kbytes(dbtob(stp->st_blocks)),	/* kbytes       */
#else
		(long) kbytes(stp->st_size),		/* kbytes       */
#endif
		pmode,					/* protection	*/
		stp->st_nlink,				/* # of links	*/
		uname,					/* owner	*/
		gname,					/* group	*/
		fsize,					/* # of bytes	*/
		ftime,					/* modify time	*/
		file,					/* name		*/
#ifdef	S_IFLNK
		(pmode[0] == 'l') ? " -> " : "",
		(pmode[0] == 'l') ? flink  : ""		/* symlink	*/
#else
		"",
		""
#endif
	);

	return (0);
}
/*
 * Make sure the directory can be read before actually doing the cd
 */
static chdir_access(directory)
        char *directory;
{
        DIR *dir_desc;
        struct dirent *dir_pointer;
        int saved_errno;
        if ((dir_desc = opendir(directory)) == NULL) 
        	return (-1);
        dir_pointer = readdir(dir_desc);
       	saved_errno = errno;
        closedir(dir_desc);
        if (dir_pointer == NULL) {
                errno = saved_errno;
                return (-1);
        }
        return (chdir(directory));
}
static char *
whereami()
{
static char pathname[PATH_MAX + 1];

        if(getcwd(pathname,sizeof(pathname)) == 0) {
                perror("pwd");
                exit(1);
        }
        return(pathname);

}
static void
usage()
{
	fprintf(stderr, MSGSTR( USAGE, "Usage: find path-list [predicate-list]\n"));
#ifdef AMES
		fprintf(stderr, MSGSTR( USAGE1, "Usage: find file\n"));
#endif /* AMES */
		exit(++exit_status);
}

static int
atoint( char * opstr, char * val )
{
	register nval;
	if ((nval=atoi(val)) == 0)
	{
		if (((*val == '0') && (val[1] == '\0')) || 
		    ((*val == '-') && (val[1] == '0') && (val[2] == '\0')))
			return (0);
		fprintf(stderr, MSGSTR( BADNUM, 
			"find: Specify a decimal integer for %s\n"), opstr);
		exit_status++;
		usage();
	}
	return(nval);	
}
