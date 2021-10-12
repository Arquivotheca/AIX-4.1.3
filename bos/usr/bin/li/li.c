static char sccsid[] = "@(#)69  1.35.1.4  src/bos/usr/bin/li/li.c, cmdfiles, bos41J, 9520A_all 5/16/95 14:50:32";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: li
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * list files, directories, archives
 *
   Rewritten by D. Landauer to combine the functions of seven different
   `ls' programs we had here (May '79) ... also made it use stdio.  Flags:
	-I<abcfghilmnoprsu>    include a field
	-E<abcfghilmnoprsu>    exclude a field                  **rrr**
	    accessed-time      headers       modified-time  protections
	    character count    i-number      name           size (in blocks)
	    group id           link count    owner id       updated-time
	-O<abcdfpsx> list only these types of files
	    archives           directories           symbolic links
	    block devices      files (normal)        executable files
	    character devices  pipes (fifos)

	-S[r]<acmnsux> sort order [r= reversed]
	    accessed-time (latest first)            name (default)
	    modified-time (latest first)            size (biggest first)
	    updated-time  (latest first)            x = no sort at all
	-R[<n>anqp] recursive, to level n
	    a absolute pathnames
	    n don't list qdirs' contents (default)
	    p pathnames (relative)
	    q do list qdirs' contents
	-<n>    at most n columns
	-a      all files
		     VMS:   all versions of files
		     other: even ones beginning w/ `.'
	-d      directories (not their contents)
	-f      force interpretation as a directory
	-l      local long form  (-Icglmnop)
	-k	remote long form (-Ibcfmnpr)		* removed *
	-n      real file names:  do not make them printable (==> -1)
	-s      sortable verbose (Berklix style)
	-v      visual/verbose
	-x      extended (-Iabcfgilmoprsu)
	-L	follow symbolic links (default is to not follow)

   Routines are in alpha order except main & init (the first two) and
   getpwn, getgrpn and search (the last three).   Throughout, I refer to
   archives as "quasi-directories", often abbreviated as qdirs.
   (Quasi-directories are a superset of directories.)

   Because of the removal of the Distribution Services, the following
   options will be affected:
	1) -k option is removed.
	2) -Ibfr options are ignored, but it will display '-'s instead.
	3) -Ebfr options are ignored.
	4) -x option is affected. It will display '-' for -bfr fields.  
 */

#define _ILS_MACROS
#define SGN(x)  ((x) == 0 ? 0 : ((x) > 0 ? 1 : -1))
#define	ismajor(c)  (strchr(majorflags,(int)(c)))

#define DOT "."
#define DOTDOT ".."
#define UID(p)    ((p)->s.st_uid)
#define USERNAMESIZE 8

#define UBLOCKS(n)  ((n * DEV_BSIZE ) / UBSIZE)

/* tblocks for archives are NOT blocks, but FILESIZE/BLOCKSIZE */
#define UBLOCKS_ARCHIVE(n) ((n + UBSIZE - 1)/UBSIZE)

#include <sys/types.h>
#include <locale.h>
#include <nl_types.h>
#include "li_msg.h"

nl_catd catd;

#define MSGSTR(Num,Str) catgets(catd,MS_LI,Num,Str)

#include <time.h>
#include <netdb.h>
#include <pwd.h>
#include <grp.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <IN/CSdefs.h>
#include <sys/fullstat.h>                                       /*rrr*/
#include <sys/stat.h>    
#include <stdio.h>
#include <ctype.h>
#include <IN/standard.h> 
#include <IN/ARdefs.h>
#include <sys/utsname.h>                                        /*rrr*/
#define  _h_DIR                                                 /*rrr*/
#include <sys/user.h>                                           /*rrr*/
#include <sys/syspest.h>                                        /*rrr*/
#include <dirent.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mntctl.h>			/* to access remote nodes */
#include <sys/vmount.h>			/* to access remote nodes */
#include <sys/errno.h>
/**#define major(dev)      ((dev>>16)&0xFFFF)   include sys/sysmacros.h
   #define minor(dev)      (dev&0xFFFF)         instead  **/
#define LONGTIME 16056321
#define CORE            256 /* is a msg-nr. for complain */
#define CANT            257 /* is another msg-nr. for complain */
#define OURPATHLEN     4096 /* well, better than 80, but still rigid */
#define INFINIT        128  /* infinite depth for all practical purposes */
#define INITDEPTH	2   /* Initial value of depth */
#define LBSIZE      (sizeof (struct lbuf))
#define LITBSIZE    (sizeof (struct littlebuf))
#define PTRSIZE     (sizeof (char *))
#define wcSIZE      (sizeof (wchar_t))
#define INDENT          2

#define FUNNYCHAR      USHRT_MAX			  /* b4: NLCHARMAX */
					/* beware, USHRT_MAX is being used */
					/*  a char. that can't exist in    */
					/*  filename so it'll sort last.   */
#define QDIRINIT       100
#define QDIRINCR        50

#define GRP         3
#define LI_OTHER    6

#define ANY_EXEC  (S_IEXEC|(S_IEXEC>>GRP)|(S_IEXEC>>LI_OTHER))

	/* maximum length of a single component of a filename */
#define BIGCOMPONENT    60
#define NAMELEVELS      INFINIT
#define WIDTH           79
#define MAXNCOL         (WIDTH/2) /* 2 is minimum column width */

	/* these go in lxbits in the lbuf struct */
#define ISARG     01
#define ISDIR     02
#define ARCHIVE   04
#define ISLNK	 010 /* is a symbolic link */
#define ISQDIR   020 /* is quasi-directory:  directory|archive */
#define ISQARG   040 /* set when either of ISQDIR or ISARG is set */
#define ISEXEC  0100 /* is executable by someone (but is not directry) */
#define SPECIAL 0200 /* is block or character special file */

/*
	Major:  EIORS
	minor for E,I:  abcfghilmnoprsu		*display '-' for -bfr fields*
		  O:    abcdfsx
		  S:    acmnrsu
		  R:    ap (numbers are special)
	no corresp. major: adflnvxL             *display '-' for -bfr fields* 
		(numbers are special)
*/
/* Since the flags (really in the following arrays) are used as if
   they were globals, here are all the definitions */


#define aflg normal[0]
#define dflg normal[1]
#define fflg normal[2]
#define eflg normal[8]
#define lflg normal[3]
#define nflg normal[4]
#define sflg normal[5]
#define vflg normal[6]
#define xflg normal[7]
#define Lflg normal[9]                                          /*rrr*/
#define Faflg field[0]
#define Fbflg field[1]				/*display '-' for -b field*/
#define Fcflg field[2]
#define Ffflg field[3]				/*display '-' for -f field*/
#define Fgflg field[4]
#define Fhflg field[5]
#define Fiflg field[6]
#define Flflg field[7]
#define Fmflg field[8]
#define Fnflg field[9]
#define Foflg field[10]
#define Fpflg field[11]
#define Frflg field[12]				/*display '-' for -r field*/
#define Fsflg field[13]
#define Fuflg field[14]
#define Oaflg only[0]
#define Obflg only[1]
#define Ocflg only[2]
#define Odflg only[3]
#define Offlg only[4]
#define Opflg only[5]
#define Osflg only[6]
#define Oxflg only[7]
#define Saflg sort[0]
#define Scflg sort[1]
#define Smflg sort[2]
#define Snflg sort[3]
#define Srflg sort[4]
#define Ssflg sort[5]
#define Suflg sort[6]
#define Sxflg sort[7]
#define Raflg recur[0]
#define Rnflg recur[1]
#define Rpflg recur[2]
#define Rqflg recur[3]

#define NNormal     9
#define NField      15
#define NOnly       8
#define NSort       8
#define NRecur      4

int errs = 0;

char normal[NNormal];
char field[NField];
char only[NOnly];
char sort[NSort];
char recur[NRecur];

struct flags {
	char *fminors;
	char *flagarray;
	char *statrequire;
} flagtable[6] =  {
	/* N */ "adflnsvxeL",   normal, "0001011110",		/*-k removed*/            
	/* E */ "abcfghilmnoprsu", field,  "000000000000000",   /*no stat*/
	/* I */ "abcfghilmnoprsu", field,  "111110111011111",   /*-bfr ignored*/
	/* O */ "abcdfpsx",     only,   "!111111!",
	/* R */ "anpq",         recur,  "1111",
	/* S */ "acmnrsux",     sort,   "11100110"
};

char majorflags[] = "NEIORS";

struct littlebuf {
	wchar_t *lnamep;
	long   linum;
	char    lxbits;
	char    ltchar;
	char   *realname;
};

struct lbuf {
	wchar_t *lnamep;
	long   linum;
	char    lxbits;     /* Extra bits that didn't fit */
	char    ltchar;     /* -|F c|C b|B l|L p|P or d|D */    /*rrr*/
	char 	lemode;	    /* extended attributes: ACL, TP or TCB */
	char   *realname;
	char   *flinkto;
	struct fullstat s;
};

long    tblocks; /* total blocks in current argument's contents */
long    gtblocks; /* grand total number of blocks */
int     statreq = FALSE;
int     pass2now = FALSE;
int     pass2req = FALSE;
int     depth = INITDEPTH;
int     maxdepth = 1;
int     maxncols = 40;


char	*truename;
char    sortit, Oflg, Rflg;
static  int srt;        /* determines sort order by being 1 or -1 */

char    cwd[OURPATHLEN];
char    *dotp = ".";
char    spacez[INDENT*NAMELEVELS+1],
	*spaceking;

wchar_t pathnm[OURPATHLEN];
wchar_t *pathptr;

struct lbuf *parent, *gstat();
char    *savestr();
int compar ();

void get_host();		/* create vfsnumber/hostname table */
static int get_vmount();	/* get vmount info */
extern int errno;

BUGVDEF(lidebug,BUGACT)                                              /*rrr*/

main(argc, argv)
int argc;
char **argv;
{
	register struct lbuf **arglbptrs, /* arguments' lbuf pointers */
			     *alp;        /* one argument's lbuf pointer */
	int i,
	    notdot,
	    ndecent;
	char once, ptb = 0;
	wchar_t *p;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_LI, NL_CAT_LOCALE);

        for(i=0; i<INDENT*NAMELEVELS; i++)
          spacez[i] = ' ';
        spacez[INDENT*NAMELEVELS]='\0';
        spaceking = &spacez[INDENT*NAMELEVELS];

	argc= init(argc, &argv);
#ifdef DEBUG
	      prtflags();                                       /*rrr*/
#endif

	if(Raflg)
          CScurdir(cwd);

	 /* get space for pointers to an lbuf per file argument */
	arglbptrs= (struct lbuf **)malloc((size_t)(argc*PTRSIZE));
	if(arglbptrs==NULL)
          complain(CORE);


	/* For each filename argument ... */
	for(ndecent= i=0; i < argc; i++) {

		BUGLPR(lidebug,BUGACT,("Do gstat on `%s'\n", *(argv+1)))
		if((alp = gstat(*++argv, 1)) != NULL) {
			alp->realname= *argv;
			fixname(alp, *argv);
			alp->lxbits |= (ISARG|ISQARG);
			arglbptrs[ndecent++]= alp;
		}
	}

	    /* sort the pointers to the arguments' lbufs */
	if(fflg || !Sxflg)
		qsort((void *)arglbptrs, (size_t)ndecent, (size_t)PTRSIZE, (int(*)())compar);

	once = TRUE;

	for(i = 0; i < ndecent; i++) {
		alp = arglbptrs[i];
		pass2now= pass2req;
                if(*p == FUNNYCHAR)
                        p++;
                if (Raflg && *p != '/') {
                        mbstowcs (pathnm, cwd, OURPATHLEN);
                }
		if(alp->lxbits&ISQDIR && maxdepth>0 || fflg) {
			tblocks = 0;
			lsflush();
			pass2now= pass2req;

			p= alp->lnamep;
			if(*p == FUNNYCHAR)
                          p++;

                        if (!Raflg && (argc>1)) {
				char dirname[OURPATHLEN];
				wcstombs(dirname, p, OURPATHLEN);
				printf("\n%s:\n", dirname);
			}
			pathnm[0] = 0;
			notdot= strcmp(".", alp->realname);
			if (Raflg && *p != '/') {
				mbstowcs (pathnm, cwd, OURPATHLEN);
				if (notdot) {
                                        char tmpslash[2]="/";
                                        wchar_t slash[2];
                                        mbstowcs(&slash, tmpslash, 2);
                                        wcscat (pathnm, &slash);
				}
			}
			if (Rpflg && notdot)
                          wcscat (pathnm, p);
			pathptr = pathnm + wcslen(pathnm);
			print_hdrs();

                        if (Raflg & notdot) {
                                char out[OURPATHLEN];
                                wcstombs(out, p, OURPATHLEN);
                                printf("%s/%s\n", cwd, out);
                        }

			listdir(alp);

			if(Fsflg|Fcflg) { /* if we saw any file sizes */
				printf(MSGSTR(TOTALBLOCKS,"total %ld blocks\n"),
					tblocks);
				gtblocks += tblocks;
				ptb++; /* printed any totals? */
			}
		}
		else {
			if(once)
                        { 
                          print_hdrs();
                          once= FALSE;
                        }
			parent = NULL;
			if( onlyallows(alp) )
		          pentry(alp);
		}

		free((void *)alp->lnamep);
		free((void *)alp);
	} /* end for */
	lsflush();
	if( ptb > 1 )
		printf(MSGSTR(GRANDTOTAL,"grand total %ld blocks\n"), gtblocks);
	exit(errs);
	/*NOTREACHED*/

} /* end main */

 /* Establish buffering for output.
    Interpret all the flag arguments; set argv to point to first
    filename arg,  and return modified argc */
init (ac, avp) int ac; char ***avp;
{
	register char *p,
		      **av = *avp;
	int i,
	    minix;
	struct flags *maj;
	char c,
	     colset = FALSE,
	     cs[2],
	     majchr,
	     mdset = FALSE;

	cs[1]= 0;

	/* If ls is called with names "di", "dir", "m", "n" or "o", set the
	   flags "-Ialmops" (for di & dir), "-xEahm", "-v" or "-1". */

	p = CSsname(av[0]);
	switch(*p)
	{
		case 'd':  av[0]= "-Ialmops"; av--; ac++; break;
		case 'm':  av[0]= "-xEahm"; av--; ac++; break;
		case 'n':  av[0]= "-v"; av--; ac++; break;
		case 'o':  av[0]= "-1"; av--; ac++; break;
	}

	Fnflg = TRUE;   /* Initially assume we'll output filenames */
	Raflg = FALSE;
	Rpflg = FALSE;
	Rflg = FALSE;

		/* while there are flag arguments:  */
	while(--ac > 0 && *av[1] == '-') {
		av++;
		BUGLPR(lidebug,BUGACT,
			 ("In init's while(per arg), av is %o -> `%s'\n",
				av, *av))
		/* per-argument initializations */
		maj= &flagtable[0];
		majchr= 'N';

		while (c= *++*av) {

			BUGLPR(lidebug,BUGACT,
			  ("INside arg, av is %o -> `%s', c %d(%c)\n",
			     av, *av, c, c))
			cs[0] = c;
			if(ismajor(c)) {

				BUGLPR(lidebug,BUGACT,("Major flag %c\n",
					      c))
				i = (char *) CSlocs(majorflags, cs) -
					     majorflags;
				if (majorflags[i] == 0)
				    complain(0, c);
				if(c=='R')
					Rflg = TRUE;
				if(c=='R'  &&  mdset == FALSE)
				    maxdepth= INFINIT;
				majchr= c;
				maj= &flagtable[i];
				continue;
			} /* end if major new flag */

			if(isdigit((int)c)) {

				BUGLPR(lidebug,BUGACT,("Number %c (%s)\n",
				      c, *av))
				i = (int) CStol(*av, av, 10);
				--*av;
				if(majchr=='R') {
				    maxdepth= i;  mdset= TRUE;
				}
				else {
				    majchr = 'N';
				    maxncols= i;  colset= TRUE;
				}
				continue;
			}

			BUGLPR(lidebug,BUGACT,("Minor flag %c (for %c)\n",
				  c, majchr))
			minix = (char *) CSlocs(maj -> fminors, cs) -
					 maj->fminors;
			if (maj->fminors[minix] == 0)
				complain(majchr, c);

			BUGLPR(lidebug,BUGACT,(" ... is OK, ix= %d\n",
				minix))
			maj->flagarray[minix]=
				(majchr == 'E' ?  FALSE  :  TRUE);

			/* Doing some implications here inside the loop
			   allows things like ls -xEahm to work. */
			switch(majchr) {
				case 'N':
					if(c=='x') {
						Fiflg= Fsflg= Faflg= Fuflg= TRUE;
						Frflg= Ffflg= Fbflg= TRUE;
						/* as well as all that l does */
						c= 'l';
					}
					if(c=='l') {
						Fcflg = Fmflg = Fgflg =
						Foflg = Fpflg = Flflg = TRUE;
						/* like `I' */
						if(!colset)
                                                  maxncols= 1;
					}
					if(c=='e') {
						Fcflg = Fmflg = Fgflg =
						Foflg = Fpflg = Flflg = TRUE;
						eflg = TRUE;	
						if(!colset)
                                                  maxncols= 1;
					}
					if(c=='n' && !colset)
                                          maxncols= 1;
					break;

				case 'I':
					if(c!='n' && !colset)
                                          maxncols= 1;
					break;
				case 'O':
					Oflg= TRUE;
					break;
				case 'R':
					if (c == 'n')
                                          Rqflg= FALSE;
					else
                                          if ((c=='a' || c=='p') && !colset )
						maxncols= 1;
					break;
			} /* end switch */

			/* Does this flag cause a stat to be required? */
			if(maj->statrequire[minix] != '0')
				statreq= TRUE;
		} /* end while doing an argument */
	} /* end while there are flag arguments */

	BUGLPR(lidebug,BUGACT,
		("After init's while loop, av is %o -> `%s'\n", av, *av))

	if(maxncols > 1)
		pass2req= TRUE;

	if(Raflg)
		Rpflg= TRUE;

	if(maxdepth > 1)
		statreq= TRUE;

	if(dflg) {
	    if(maxdepth > 1)
		complain(MSGSTR(CONFLICT,"Conflicting flags -d and -R\n"),0);
	    maxdepth = 0;
	}

	if (fflg) {
		aflg = Sxflg = TRUE;
		maxdepth= 1;
		vflg = statreq = FALSE;
		/* in addition, fflg implies turn off ALL -Iflags
		   except -i & -n; and turn off ALL -Rflags. */
	}

	sortit = !Sxflg;

	if( (Saflg|Scflg|Smflg|Srflg|Ssflg|Suflg|Sxflg) == 0 )
		Snflg = TRUE;

	srt = Srflg?   -1       /* use sort-in-reverse flag to set a  */
		   :    1;      /* multiplier                         */

	if (ac <= 0) {
		ac = 1;
		av = &dotp - 1;
	}

	*avp = av; /* reset main's argv */
	return ac;
} /* end init */


    /* Always, in an lbuf, realname is the real name by which I could
      open this file.   I.e., the relative or absolute pathname.
      lnamep is just the entry name, "fix"ed; pathnm is the pathname,
      "fix"ed (that is, printable) */

branchdir (p_ep)
	struct lbuf *p_ep;
{
	register wchar_t *p1, *q1, *mark;
	register char *p, *q;

	if (p_ep->linum == -1)
          return;
	if (depth > maxdepth)
          return;
	p = q = CSsname(p_ep->realname);
	if (*q++ == '.')
		if ((*q == 0) || (*q++ == '.' && *q == 0))
                  return;
	depth++;
	spaceking -= 2;

	if ((p1 = mark = pathptr) != pathnm)
          *p1++ = '/';
	q1 = p_ep->lnamep;
	if (*q1 == FUNNYCHAR)
          q1++;
	while ((*p1++ = *q1++) != 0); /* NOTE: Null loop. -- HF 6/12/91 */
	pathptr = p1 - 1;

	    /* listdir may call branchdir recursively */
	listdir(p_ep);

	pathptr = mark;
	*pathptr = 0;
	spaceking += 2;
	depth--;
}



compar (ap1, ap2)
struct lbuf **ap1, **ap2;
{
	register struct lbuf *p1, *p2;
	register int i = 0;
	static int qdandarg = (ISQDIR|ISARG);

	p1 = *ap1;
	p2 = *ap2;

	if (dflg==0) {
		/* argument qdirs (including dirs) always follow all others */
		if ((p1->lxbits&qdandarg) == qdandarg) {
			if ((p2->lxbits&qdandarg) != qdandarg)
                          return(1);
		}
                else
                {
			if ((p2->lxbits&qdandarg) == qdandarg)
                          return(-1);
		}
	}
	if(Scflg) /* file size, character count */
	{
		BUGLPR(lidebug,BUGACT,("Scflg"))

		if (p2->s.st_size > p1->s.st_size)
			i++;
		else
                  if (p2->s.st_size < p1->s.st_size)
			i--;
		if (i)
                  return(i*srt);
	}
	if(Ssflg) /* file size in blocks */
	{
		BUGLPR(lidebug,BUGACT,("Ssflg"))

		if (p2->s.st_blocks > p1->s.st_blocks)
			i++;
		else
                  if (p2->s.st_blocks < p1->s.st_blocks)
			i--;
		if (i)
                  return(i*srt);
	}
	if(Smflg)
	{
		BUGLPR(lidebug,BUGACT,("Smflg "))

		if (p2->s.st_mtime > p1->s.st_mtime)
                  i++;
		else
                  if (p2->s.st_mtime < p1->s.st_mtime)
                    i--;
		if (i)
                  return(i*srt);
	}
	if(Saflg)
	{
		BUGLPR(lidebug,BUGACT,("Saflg "))

		if (p2->s.st_atime > p1->s.st_atime)
                  i++;
		else
                  if (p2->s.st_atime < p1->s.st_atime)
                    i--;
		if (i)
                  return(i*srt);
	}
	if(Suflg)
	{
		BUGLPR(lidebug,BUGACT,("Suflg "))

		if (p2->s.st_ctime > p1->s.st_ctime)
                  i++;
		else 
                  if (p2->s.st_ctime < p1->s.st_ctime)
                    i--;
		if (i)
                  return(i*srt);
	}
	/* compare the names */

	BUGLPR(lidebug,BUGACT,("Names: `%s' vs `%s'\n",
			p1->lnamep, p2->lnamep))
	if (nflg)
		i = strcoll (p1->realname, p2->realname);
	/* else use lnamep */
	else 
          if (*p1->lnamep == FUNNYCHAR)
		if (*p2->lnamep == FUNNYCHAR)
			i = wcscoll (p1->lnamep+1, p2->lnamep+1);
		else
			i = 1;
	else 
          if (*p2->lnamep == FUNNYCHAR)
		i = -1;
	else
		i = wcscoll (p1->lnamep, p2->lnamep);
	return srt * i;
}

complain (majchr, minchr)
{
	if(majchr == 0 || (majchr=='N' && minchr!=0))
	    fprintf(stderr,MSGSTR(UNKNOWN1,"Unknown option `-%c'\n"), minchr);
	else
          if(majchr == CORE)
		fprintf(stderr,MSGSTR(NOMEMORY,"li: out of memory.\n"));
	  else
            if(majchr == CANT)
		fprintf(stderr, 
	          MSGSTR(CANTHAPPEN,"li:  \"Can't happen\" error #%d.\n"),
		  minchr);
	    else 
              if(isdigit(minchr))
	        fprintf(stderr,
		    MSGSTR(UNKNOWN2,"Unknown option `-%c<number>'\n"),
		    majchr);
	      else
                if(minchr != 0)
	          fprintf(stderr, 
		       MSGSTR(UNKNOWN3,"Unknown option `-%c%c'\n"),
		       majchr, minchr);
	        else
	          fprintf(stderr, "%s", majchr);
	fprintf(stderr,MSGSTR(USAGE,"Usage: li [-I[abcfghilmnoprsu]] [-E[abcfghilmnoprsu>]]\n\t[-O[abcdfpx]] [-S[acmnrsux]] [-R[Number][anqp]]\n\t[-Number] [-adflensvxL] [File...] [Directory...]\n"));
	exit((int)EXITBAD);
}

fixname (lp, origname)
struct lbuf *lp;
register char *origname;
{
        int i,j;
	register wchar_t *nnp, *newname;
	wchar_t c;
	char folchr = 0;
	char tempstr[5];
	int l;
	char ctl_rep [2];     /* control char representation */

	newname = nnp = (wchar_t *) malloc((size_t)(wcSIZE*OURPATHLEN));
	if (nnp==NULL)
		complain(CORE);
	if (maxdepth > 1 && (lp->lxbits & ISQDIR))
		*nnp++ = FUNNYCHAR; /* make directories sort last */
	if (vflg) {
		/* Stick in a  character so these will sort last */
		/* (pentry strips it back off before printing) */
		if (lp->lxbits & ISQDIR) {
			if (maxdepth < 2)
				*nnp++ = FUNNYCHAR; /* make qdirs sort last */
			if (lp->lxbits & ISDIR) {
				*nnp++ = '[';
				folchr = ']';
			}
		}
		else {  
			if (lp->lxbits & (ISEXEC | SPECIAL | ISLNK))
				*nnp++ = FUNNYCHAR;
			if (lp->lxbits & ISEXEC) {
				*nnp++ = '<';
				folchr = '>';
			}
			else 
                          if (lp->lxbits & SPECIAL) {
				*nnp++ = folchr = '*';
			  }
			  else
                            if (lp->lxbits & ISLNK) {
				*nnp++ = folchr = '@';
			}
		}
	}

	if ( sflg ) {
		if ( lp->lxbits & ISDIR )
			folchr = '/';
		else 
                  if ( lp->lxbits & ISEXEC )
			folchr = '*';
		  else
                    if ( lp->lxbits & SPECIAL )
			folchr = '?';
		    else
                      if ( lp->lxbits & ISLNK )
			folchr = '@';
	}

        while (*origname)
        {
          i = mbtowc(&c, origname, MB_CUR_MAX);
          if (i<=0)
          {
            c=*origname & 0xFF;  /* Avoid sign extension. */
            i=1;
          }
          if (iswcntrl (c))
          {
            if (nnp == newname)
              /* 1st time through while loop */
              *nnp++ = FUNNYCHAR;
            *nnp++ = '^';
            ctl_rep [0] = '@' + c;
            (void) mbtowc(nnp, ctl_rep, MB_CUR_MAX);
            nnp++;
          }
          else if (! iswprint (c))
          {
 	    if (nnp == newname)
 	      *nnp++ = FUNNYCHAR;
 
            for(j=0; j<i; j++, nnp += 4)
            { 
 	      sprintf (tempstr, "\\%03o", origname[j]);
 	      mbstowcs (nnp, tempstr, OURPATHLEN);
 	    }
          }
          else {
            *nnp++ = c;
	  }
          origname += i;
        }
        if (folchr)
        {
          (void) mbtowc(nnp, &folchr, MB_CUR_MAX);
          nnp++;
        }
        *nnp = 0;
        l = wcslen(newname);

        /* Give back however much of that I didn't use */
        newname = (wchar_t *) realloc ((void *)newname,(size_t)(wcSIZE*(l+1)));
        /* set return value */
        lp->lnamep = newname;
} /* end fixname */

isarch(fnm)
char *fnm;
{
	FILE *file;
	int retval;

	if ((file = fopen(fnm, "r")) == NULL)
		return(0);

	retval = ARisarchive(file);
	fclose(file);
	return(retval);
}

fmtchar (p)
	register struct lbuf *p;
{
	int rm_entry;                                           

	if (p->s.st_flag & FS_REMOTE)
		rm_entry = 1;
	else 
		rm_entry = 0;

	switch(p->s.st_mode & S_IFMT) {
		case S_IFIFO: return (rm_entry ? 'P' : 'p');    
		case S_IFDIR: return (rm_entry ? 'D' : 'd');    
		case S_IFCHR: return (rm_entry ? 'C' : 'c');    
		case S_IFBLK: return (rm_entry ? 'B' : 'b');    
		case S_IFREG: return (rm_entry ? 'F' : '-');    
		case S_IFLNK: return (rm_entry ? 'L' : 'l');   
		default:      return '?';
	}
} /* end fmtchar */

freentry (ep)
	struct lbuf *ep;
{
	if(statreq && (ep->lxbits&ISARG) == 0) {

	    BUGLPR(lidebug,BUGACT,("Free real name %04x\n", ep->realname))

	    free((void *)ep->realname);
	}

	BUGLPR(lidebug,BUGACT,
	      ("Free fixed name %04x and entry pointer %04x\n",
	       ep->lnamep, ep))
	free((void *)ep->lnamep);
	free((void *)ep);
} /* end freentry */



struct lbuf *gstat (file, argfl)
	char *file;
{
	register struct lbuf *rep;
	long nbl;
        static  int  dotCounter = 0 ;

	BUGLPR(lidebug,BUGACT,("Gstat `%s':  ",  file))
	if (argfl || statreq) {

		BUGLPR(lidebug,BUGACT,("Argfl %d, statreq %d\n",
			argfl, statreq))
		rep= (struct lbuf *)malloc((size_t)sizeof(*rep));
		if(rep==NULL)
                  complain(CORE);
		rep->lxbits= 0;

		if(fullstat(file, Lflg? 0:FL_NOFOLLOW, &(rep->s) ) < 0)
			if(!Lflg || fullstat(file, FL_NOFOLLOW,
				&(rep->s) ) < 0)
			{        /* stat failed */
				if (errno==ENOENT)
                                {
			    	  fprintf(stderr,
		MSGSTR(NOENTRY,"li: File %s was not found.\n"),file);
                                  errs=1;
                                }
				else
				  perror(file);
				if (argfl)
                                  return NULL;
				rep->s.st_ino = -1;
				rep->s.st_mode = 0;
				rep->s.st_size= 0;
			}
		if (rep->s.st_mode&S_IXACL ||
		    rep->s.st_mode&S_ITCB || rep->s.st_mode&S_ITP)
			rep->lemode = '+';
		else
                  rep->lemode = '-'; 

		rep->linum = rep->s.st_ino;
		rep->ltchar= fmtchar(rep);
		rep->flinkto = NULL;

		BUGLPR(lidebug,BUGACT,
			("ltchar of `%s' is `%c', Inode= `%d`\n",
			file, rep->ltchar, rep->linum))

		switch(rep->ltchar) {
			case '-':
			case 'F':                               /*rrr*/
				if(Rqflg || Oaflg) {
					if (isarch(file))
						rep->lxbits |=
							(ARCHIVE|ISQDIR|ISQARG);
				}
				    /* Only plain files are EXECutable */
    				if (!(accessx(file,X_ACC, ACC_ANY)))
				     rep->lxbits |= ISEXEC;
				break;

			case 'b':
			case 'B':                               /*rrr*/
			case 'c':
			case 'C':                               /*rrr*/
				rep->lxbits |= SPECIAL;
			case 'l':
			case 'L':
				if (!Lflg)
				{
					char    buf[BUFSIZ];
					int     cc;
 
					cc = readlink(file, buf, BUFSIZ);
					if (cc >= 0)
					{
						buf[cc] = 0;
						rep->flinkto = savestr(buf);
					}
				}
				rep->lxbits |= ISLNK;
				break;
			case 'p':
			case 'P':                               /*rrr*/
				break;
			case 'd':
			case 'D':                               /*rrr*/
				rep->lxbits |= ISDIR|ISQDIR|ISQARG;
				break;
			case '?':
				break; /* the stat failed. */
			default:
				complain(CANT, 0);
		}

		if( !argfl ) {
			nbl = UBLOCKS( rep->s.st_blocks );
			if ( ( strcmp(truename, DOT) && 
			       strcmp(truename, DOTDOT) ) ||
			     dotCounter++ < 2 )
				tblocks += nbl;
		}
		BUGLPR(lidebug,BUGACT,("Tblox now %ld\n", tblocks))
		BUGLPR(lidebug,BUGACT,("Lxbits %o\n", rep->lxbits))
	}
	else {  /* neither an arg nor is stat required on it */
		BUGLPR(lidebug,BUGACT,("Itty bitty buffer\n"))

		rep= (struct lbuf *)calloc((size_t)1, (size_t)LITBSIZE );
		if( rep==NULL)
		    complain(CORE);
		rep->linum= 0;
	}
	return rep;
} /* end gstat */

listdir (dp)
	register struct lbuf *dp;
{
	int i,
	    nen;
	struct lbuf **ptrs;

	BUGLPR(lidebug,BUGACT,("listdir %s\n", dp->realname))

	if((dp->lxbits&ISQDIR)==0 && !fflg)
		complain(CANT, 1);

	pass2now = pass2req;

	parent = dp;
	if(dp->lxbits&ISQDIR) {
		if(dp->lxbits&ISDIR)
                  nen= li_readdir(dp, &ptrs);
		else
                  if(dp->lxbits&ARCHIVE)
                    nen= readarch(dp, &ptrs);
	}
	else
          if(fflg) {
		nen= li_readdir(dp, &ptrs);
		lsflush();
		return;
	}
	if(nen <= 0)
        {
          lsflush();
          return;
        }

	BUGLPR(lidebug,BUGACT,("listdir before qsort:  nen %d, ptrs %o\n",
		       nen, ptrs))
	BUGLPR(lidebug,BUGACT,("Names that I read:   "))
#ifdef DEBUG
	for(i= 0; i<nen; i++)
		BUGLPR(lidebug,BUGACT,("ptrs[%d] (=%o) ->lnamep (=%o) `%s'\n",
		      i, ptrs[i], ptrs[i]->lnamep, ptrs[i]->lnamep))
#endif

	if( sortit )
		qsort((void *)ptrs, (size_t)nen, (size_t)PTRSIZE, (int(*)())compar);

	for(i= 0; i < nen; i++)
		/* List one entry and test whether we'll recursively
		    list this entry (if it's a qdir); also free it. */
		list_one(ptrs[i], dp);

	lsflush();
	free((void *)ptrs);
} /* end listdir */


list_one (ep, dp)
	register struct lbuf *ep,
			     *dp;
{
	char recurq;

	/* Test whether we'll recursively list this directory:
	   If maxdepth <= 1, the ep entries are shorties;  we never
	   recurse if fflg; normally, it's gotta be a real directory
	   to recurse; if Rqflg, a quasi will do.  */
	if( maxdepth > 1 && !fflg &&
	    ( (ep->lxbits&ISDIR) || (Rqflg && (ep->lxbits&ISQDIR))))
	     recurq= TRUE;
	else
          recurq= FALSE;
	if(recurq)
          lsflush();
	if(onlyallows(ep))
          pentry(ep);
	if(recurq) {
		pass2now= pass2req;
		branchdir(ep);
		parent= dp;
	}
	freentry(ep);

} /* end listdir */


struct list {
	int dlen, blen;
	struct list *link;
	char *bufptr;
} *head = NULL,
  *tail = NULL;

int dlensofar = 0, blensofar = 0,
    lines = 0,      /* Nr of lines gotten since a flush */
    longline = -1;  /* length of longest line since a flush */

	/* VARARGS */
lprint (ctl, p1, p2, p3)
	char *ctl;
{
	static char aline[OURPATHLEN+1];
	static char *inline = aline;
	register struct list *lptr;
	register char *bufcopy;
	int dlength;	/* display length */
	int blength;	/* length in bytes */

	if(pass2now)
	{
		sprintf(inline, ctl, p1, p2, p3);
		if ((dlength = strlen (inline)) <= 0)
                  return;
		dlensofar += dlength;
		blength = strlen (inline);
		blensofar += blength;
		inline += blength;
		if(*(inline-1) != '\n')
		    return;

		BUGLPR(lidebug,BUGACT,("Whole line is `%s'.\n", aline))

		lines++;

		if (dlensofar > longline)
                  longline = dlensofar;
		bufcopy = (char *) malloc ((size_t)(blensofar+1));

		if(bufcopy==NULL)
                  complain(CORE);
		strcpy(bufcopy, aline);

		lptr = (struct list *)malloc((size_t)(sizeof(*lptr)));
		if(lptr==NULL)
                  complain(CORE);

		BUGLPR(lidebug,BUGACT,("lprint:  Head %x, tail %x, lptr %x\n", head, tail,
		      lptr))

		if(head==NULL)
                  head = tail = lptr;
		tail->link= lptr;
		tail= lptr;
		lptr->link= NULL;
		lptr->bufptr= bufcopy;
		lptr->dlen = dlensofar;
		lptr->blen = blensofar;
		blensofar = dlensofar= 0;
		inline = aline;
	}
	else {
		printf(ctl, p1, p2, p3);
		blensofar = dlensofar = lines = 0;
		longline = -1;
	}
} /* end lprint */

lsflush ()
{
	register struct list *p;

	if (dlensofar > 0)
          lprint ("\n",0,0,0);

	BUGLPR(lidebug,BUGACT,("lsflush:  p2n %d, longline %d, w/2 %d\n",
	      pass2now, longline, WIDTH/2))

	if((pass2now && longline < WIDTH/2) && !(Rflg && depth == INITDEPTH))
		lsmc();
	else /* It's gotta be a single column.  Simple. */
		for(p = head; p != NULL; p = p->link) {
			printf("%s", p->bufptr);
			free((void *)p->bufptr); free((void *)p);
		}
	pass2now = FALSE;
	longline = -1;
	lines = blensofar = dlensofar  = 0;
	head = tail = NULL;
}


/* print vertical-columns table of files:
   relevant globals are lines, longline and WIDTH
   divided by longline gives number of columns of file entries
   longline is guaranteed by lsflush to be < WIDTH/2.
*/
lsmc ()
{
	int cc,
	    i,
	    j,
	    blanx,     /* nr of blanks in spaceking, precede each line */
	    colwidth,  /* nr of chars in longest string. */
	    ncols,     /* nr of columns that would allow, in WIDTH. */
	    nincol,    /* nr entries in each col but last */
	    slop,      /* nr entries in last col */
	    colnr,     /* col we're printing now (0 to ncols-1) */
	    rownr,     /* row we're printing now (0 to nincol-1) */
	    wid;       /* WIDTH minus the nr of blanx */
	struct list *cols[MAXNCOL], *cp;

	if(head == NULL)
          return;
	wid= WIDTH + 1 + strlen(spaceking);
	if(maxdepth>1 && !Rpflg) {
		blanx = strlen(spaceking);
		wid -= blanx;
	}
	else
          blanx = 0;
	colwidth = longline; /* includes a \n; colwidth shd include a space */
	ncols = wid/colwidth; /* # cols possible ignoring maxncols */
	colwidth++;
	if (ncols>maxncols)
          ncols = maxncols; /* don't ignore it */

	nincol = (lines + ncols - 1)/ncols; /* (ceiling of lines/ncols) */

	ncols = (lines + nincol - 1)/nincol; /* (ceiling of lines/nincol) */

	if ((slop = lines%nincol) == 0)
          slop = nincol;

	BUGLPR(lidebug,BUGACT,
	      ("lsmc: blanx %d, colwidth %d, ncols %d, nincol %d\n",
	      blanx, colwidth, ncols, nincol))
	BUGLPR(lidebug,BUGACT,("slop %d, wid %d, longline %d, lines %d\n",
	      slop, wid, longline, lines))

	/* Set up cols array to point into right places in list */
	for(i = 0, cp = head; i < ncols; i++) {
		if(cp==NULL)
                  break;

		cols[i] = cp; /* point at the ( i*nincols )-th listelement */

		for(j= 0; j<nincol; j++) {
		/* Here's where the newlines pentry put there get removed */
			if(cp->bufptr[cp->blen-1] == '\n') {
				cp->dlen--;
				cp->bufptr[--(cp->blen)] = 0;
			}
			if((cp = cp->link) == NULL)
				goto break2;
		}
	}
break2:

	BUGLPR(lidebug,BUGACT,("i is %d, ncols %d\n", i, ncols))

	for(rownr = 0; rownr<nincol; rownr++) { /* for each row */
		cc= 0;
		if(maxdepth>1 && !Rpflg)
                  printf("%s", spaceking);
		for(colnr= 0; colnr<ncols; colnr++) { /* for each col */
			if(colnr==ncols-1 && rownr >= slop)
                          break;
			cp = cols[colnr];
			cc += cp->dlen+1;
			cols[colnr] = cols[colnr]->link;
			printf("%s", cp->bufptr);
			free((void *)cp->bufptr); free((void *)cp);

			/* fill out col with blanx if necessary */
			if(colnr == ncols - 2 && rownr >= slop)
                          break;
			if(colnr >= ncols - 1)
                          break;
			while(cc%colwidth != 0) {
				putchar(' ');
				cc++;
			}
		}
		putchar('\n');
	} /* end for each row */

} /* end lsmc */

char *makename (dir, file)
	char *dir, *file;
{
	static char *dfile = NULL;
	static int dflen = 0;
	register char *dp, *fp;
	register int i;

	if ((i = strlen(dir) + OURPATHLEN) > dflen) {
		if (dfile != NULL)
			free((void *)dfile);
		if ((dfile = malloc((size_t)i)) == NULL) {
			fprintf(stderr, MSGSTR(NOMEMORY,"li: out of memory\n"));
			exit(1);
		}
		dflen = i;
	}
	dp= dfile;
	fp= dir;
	while (*fp)
		*dp++ = *fp++;
	*dp++ = '/';
	fp = file;
	truename = file;
	for (i = 0; *fp != '\0' ; i++)
		*dp++ = *fp++;
	*dp = 0;
	if(dfile[0] == '.' && dfile[1] == '/')
          return &dfile[2];
	else
          return dfile;
}

 /* Return true iff the -Only restrictions given allow this
    file to be printed:  types are
	abcdfsx
 */
onlyallows (lptr)
	struct lbuf *lptr;
{
	register char allow = 0;
	register int lx, tc;

	if(!Oflg)
          return TRUE;
	lx = lptr->lxbits;
	tc = lptr->ltchar;
	if(Oaflg)
          if(lx&ARCHIVE)
            allow= TRUE;
	if(Obflg)
          if(tc == 'b' || tc == 'B')
            allow= TRUE;
	if(Ocflg) if(tc == 'c' || tc == 'C')
          allow= TRUE;
	if(Odflg) if(tc == 'd' || tc == 'D')
          allow= TRUE;
	if(Opflg) if(tc == 'p' || tc == 'P')
          allow= TRUE;
	if(Offlg) if(tc == '-' || tc == 'F')
          allow= TRUE;
	if(Osflg) if(tc == 'l' || tc == 'L')
          allow= TRUE;
	if(Oxflg) if(lx & ISEXEC)
          allow= TRUE;

	BUGLPR(lidebug,BUGACT,("Onlyallows (%s) is %d\n",
		    lptr->lnamep, allow))
	return allow;
} /* end onlyallows */

pentry (p)
	register struct lbuf *p;
{
	static char unmbuf[USERNAMESIZE+1];
	static char gnmbuf[USERNAMESIZE+1];

	if (p->linum == -1)
	    return;

	BUGLPR(lidebug,BUGACT,("pentry entry pt\n"))

	if(Fiflg)
		lprint("%5d ", p->linum,0,0);

	if(!fflg) {
		if(Fpflg)
                  pmode(p);
		if(Flflg)
                  lprint("%3d ", p->s.st_nlink,0,0);

		/* display '-' with -Ir or -Er */
		if(Frflg) {
			static int once;
			static char *hosttable[100];

			/* 
			 * just build vfsnumber -> node table once
			 */

			if (! once) {	
				once++;
				get_host(hosttable);
			}
			lprint("%-15.15s", hosttable[p->s.st_vfs],0,0);
		}

		if(Foflg) {
			/* if it fails, print the number: */
			if ( getpwn( UID(p), unmbuf) )
				lprint("%-8d ", p->s.st_uid,0,0);
			else
                          lprint("%-9.9s", unmbuf,0,0);
		}
		/* display raw UID with -If */
		if(Ffflg)
                  lprint("%-6d", p->s.st_uid,0,0);
		if(Fgflg) {
			if (getgrpn(p->s.st_gid, gnmbuf))
				lprint("%-8d ", p->s.st_gid,0,0);
			else
                          lprint("%-9.9s", gnmbuf,0,0);
		}
		/* display raw GID with -Ib */
		if(Fbflg)
                  lprint("%-6d", p->s.st_gid,0,0);
		if(Fcflg)
		    if (p->lxbits & SPECIAL)
				lprint(" %3d,%3d ", major(p->s.st_rdev),
				       minor(p->s.st_rdev),0);
		    else
			lprint("%8ld ", p->s.st_size,0,0);
		if(Fsflg)
			if (p->lxbits & SPECIAL)
				/* already done it */
				if(Fcflg)
                                  lprint("         ",0,0,0);
				else
                                  lprint(" %3d,%3d ", major(p->s.st_rdev),
				          minor(p->s.st_rdev),0);
			else
			{
				long nbl;
				nbl = UBLOCKS( p->s.st_blocks );
				lprint("%7ldB ", nbl,0,0);
			}
		if(Fmflg)
                  ptime(p->s.st_mtime);
		if(Faflg)
                  ptime(p->s.st_atime);
		if(Fuflg)
                  ptime(p->s.st_ctime);
	} /* end if not force-flag */

	if (!Fnflg)
          lprint("\n",0,0,0);
	else {      /* Print the filename ... */
		char sep,
		     frcp [OURPATHLEN]; /* (First Real Character ptr) */
		wchar_t *frnp;	/* (First Real wchar_t ptr) */
		if (nflg)
                  strcpy (frcp, p->realname);
		else  {
			if( *(frnp= p->lnamep) == FUNNYCHAR)
				frnp++;
			wcstombs (frcp, frnp, OURPATHLEN);
		}

		if(Rpflg) {
			if(parent != NULL && (parent->lxbits&ISDIR) == 0)
				sep = ':';
			else
                          sep = '/';
			if(pathnm[0]) {
				char enc_pathnm[OURPATHLEN];
				wcstombs (enc_pathnm, pathnm, OURPATHLEN);
				lprint("%s%c", enc_pathnm, sep, 0);
			}
			lprint("%s", frcp, 0, 0);
		}
		else 
                  if (pass2now)  
			lprint("%s", frcp, 0, 0);
		  else
	                lprint("%s%s", spaceking, frcp, 0);
		if (lflg && p->flinkto) {
			lprint("%s%s\n", " -> ", p->flinkto, 0);
			free((void *)p->flinkto);
		}
                else
			lprint("\n",0,0,0);
	} /* end else (print name field) */
} /* end pentry */

int     m1[] = { 1, S_IREAD,  'r', '-' };
int     m2[] = { 1, S_IWRITE, 'w', '-' };
int     m3[] = { 3, S_ISUID|S_IEXEC, 's', S_IEXEC, 'x', S_ISUID, 'S', '-' };
int     m4[] = { 1, (S_IREAD>>GRP),  'r', '-' };
int     m5[] = { 1, (S_IWRITE>>GRP), 'w', '-' };
int     m6[] = { 3, S_ISGID|(S_IEXEC>>GRP), 's', (S_IEXEC>>GRP), 'x', S_ISGID, 'S', '-' };
int     m7[] = { 1, (S_IREAD>>LI_OTHER),  'r', '-' };
int     m8[] = { 1, (S_IWRITE>>LI_OTHER), 'w', '-' };
int     m9[] = { 1, (S_IEXEC>>LI_OTHER),  'x', '-' };
int     m10[] = { 1, S_ISVTX,  't', ' ' };

int     *m[] = { m1, m2, m3, m4, m5, m6, m7, m8, m9, m10};

pmode (p)
	register struct lbuf *p;
{
	register int n, *mp;
	int **mpp, modes;
	char buf[13], *bp;

	bp= buf;
	modes = p->s.st_mode;
	*bp++ = p->ltchar;

	for (mpp = &m[0]; mpp < &m[10]; mpp++) {
		mp= *mpp;
		n = *mp++;
		while (--n>=0) {
			if ((modes&*mp)==*mp) {
			mp++;
				break;
		 	}
		 	mp += 2;
		}
		*bp++ = *mp;
	}
	if (eflg)
		*bp++ = p->lemode;
	*bp++ = ' ';
	*bp++ = 0;
	lprint(buf,0,0,0);
} /* end pmode */

print_hdrs ()
{

	if(!Fhflg)
          return;

	if(Fiflg)
          lprint(MSGSTR(INODE,"Inode "),0,0,0);
	if(Fpflg)
          lprint(MSGSTR(TPROT,"t  prot     "),0,0,0);
	if(Flflg)
          lprint(MSGSTR(LINKS," ln "),0,0,0);
	if(Frflg)
          lprint(MSGSTR(NODE," node_name/id  "),0,0,0);                 
	if(Foflg)
          lprint(MSGSTR(OWNER,"owner    "),0,0,0);
	if(Ffflg)
          lprint(MSGSTR(RUID, "r_uid "),0,0,0);                          
	if(Fgflg)
          lprint(MSGSTR(GROUP, "group    "),0,0,0);
	if(Fbflg)
          lprint(MSGSTR(RGID,"r_gid "),0,0,0);                            
/* It'd be nice to note the substitution of maj,min for nblk or chars */
	if(Fcflg)
          lprint(MSGSTR(CHARCT," char-ct "),0,0,0);
	if(Fsflg)
          lprint(MSGSTR(BLOCKS,"  blocks "),0,0,0);
	if(Fmflg)
          lprint(MSGSTR(MODIFIED,"   modified   "),0,0,0);
	if(Faflg)
          lprint(MSGSTR(ACCESSED,"   accessed   "),0,0,0);
	if(Fuflg)
          lprint(MSGSTR(UPDATED,"   updated    "),0,0,0);
	if(Fnflg)
          lprint(MSGSTR(FILENAME,"filename"),0,0,0);

	lprint("\n",0,0,0);
} /* end print_hdrs */

ptime (time2p)
	long time2p;
{
	static long timeago = 0;
	static long timeahead = 0;
	long now;
	struct tm *timestr;
	char timec [NLTBMAX];
	static int datewid = 0, ldatewid, sdatewid;

	if(timeago == 0) {
		time(&now);
		timeago= now - LONGTIME;  /* 6 months (or so) ago */
		timeahead = now + LONGTIME; /* 6 months (or so) in the future */
	}

	timestr = localtime (&time2p);
	if (! datewid) {
		ldatewid = strftime (timec, (size_t)NLTBMAX, "%sD %Y", timestr);
		sdatewid = strftime (timec, (size_t)NLTBMAX, "%sD %sT", timestr);
		datewid = ldatewid > sdatewid ? ldatewid : sdatewid;
	}
	if(time2p < timeago || time2p > timeahead) {
		strftime (timec, (size_t)NLTBMAX, "%sD %Y", timestr);
		lprint (" %*.*s ", -datewid, ldatewid, timec); 
	}
	else {
		strftime (timec, (size_t)NLTBMAX, "%sD %sT", timestr);
		lprint (" %*.*s ", -datewid, sdatewid, timec);
	}
} /* end ptime */


readalloc (arrayptr, nen)
	struct lbuf ***arrayptr;
{
	struct lbuf **elp;

	if( (elp = (struct lbuf **)malloc((size_t)(nen*PTRSIZE))) == NULL) {
		lsflush(); pass2now = FALSE;
		lprint(MSGSTR(NOSORTSPACE,"li: Not enough memory available now for sorting.\n"),0,0,0);
		sortit = FALSE;
		*arrayptr = NULL;
		return 0;
	}

	BUGLPR(lidebug,BUGACT,("readalloc:  elp is at %o\n", elp))

	*arrayptr = elp;
	return nen;
}

/* variables shared by lifcn and readarch */
static char *archname;
static struct lbuf **elp, *ep, ***elp_p;
static long nen, nspace;

/* process one archive member; called from ARforeach */
static int lifcn(parmp) ARparm *parmp; {

    char *p=makename(archname,parmp->name);
    if (parmp->name[0]==0)
      return(0);
    /* Fake a gstat for the archive entry */
    if (statreq) {
	  if ((ep=(struct lbuf *)calloc((size_t)1,(size_t)(sizeof(*ep))))==NULL)
            complain(CORE);
	  ep->lxbits = 0;
	  ep->linum = 0;
	  ep->s.st_mode = parmp->mode;

	  ep->ltchar = fmtchar(ep);
	  if( parmp->mode & ANY_EXEC )
            ep->lxbits |= ISEXEC;
	  ep->s.st_nlink= 1;
	  ep->s.st_uid = parmp->uid;
	  ep->s.st_gid = parmp->gid;
	  ep->s.st_size   = parmp->size;
	  ep->s.st_atime = ep->s.st_mtime = ep->s.st_ctime = parmp->date;
	  tblocks += UBLOCKS_ARCHIVE(parmp->size);
	  if ((ep->realname=malloc((size_t)(strlen(p)+1))) == NULL)
            complain(CORE);
	  strcpy(ep->realname,p);
       }
       else {  /* "stat" not required */
	  if ((ep=(struct lbuf *)calloc((size_t)1,(size_t)LITBSIZE))==NULL)
            complain(CORE);
	  ep->lxbits = ep->linum = 0;}
    fixname(ep, parmp->name);
    nen++;
    if (sortit) { /* getany? */
          if (nen >= nspace) {
	     if ((nspace=readrealloc(elp_p,nspace)) <= 0)
               return(1);
	     elp = &((*elp_p)[nen-1]);
          }

	     BUGLPR(lidebug,BUGACT,("readarch elp %o gets %o (`%s')\n",
			 elp, ep, ep->lnamep))

	     *elp++ = ep;}
       else
       {
         pentry(ep);
         freentry(ep);
       }
    return(0);
  }

/* Read an archive & if sortit, stash away the entries, else print them
   returns nr of entries or -1 on error (==> didn't alloc any ptrs) */

readarch (dptr, elp_pp) register struct lbuf *dptr, ***elp_pp; {
    register FILE *arfp;
    register int retval;
    elp_p=elp_pp;
    archname=dptr->realname;
    /* I'm about to add in the blocks for this archive's contents,
	so subtract off the blocks of the file itself */
    /* doesn't this leave you off by the size of the archive headers? */
    tblocks -= UBLOCKS_ARCHIVE( dptr->s.st_size );
    if ((arfp = fopen(archname, "r")) == NULL) {
	lprint(MSGSTR(CANTOPEN,"Cannot open %s\n"), archname, 0, 0); return -1;}
    if (sortit) { /* getany? */
	nspace = readalloc(elp_p, QDIRINIT);
	if(nspace <= 0)
        {
          fclose(arfp);
          return -1;
        }
    } 
    elp = *elp_p;

    BUGLPR(lidebug,BUGACT,("Readarch name = `%s', lx %o\n",
		     archname, dptr->lxbits))
    BUGLPR(lidebug,BUGACT,("elp_p %o->%o\n", elp_p, elp))

    nen = 0;

    retval=ARforeach(arfp,lifcn);
    fclose(arfp);
    return(retval!=0 ? -1 : sortit ? nen : 0);}


/* Read directory & if sortit, stash away the entries, else print them
   returns nr of entries or -1 on error (==> didn't alloc any ptrs)
*/
li_readdir (dptr, elp_p)
	struct lbuf *dptr,
		    ***elp_p;
{
	register char *name,
		      *p;
	register struct lbuf **elp,
			     *ep;
	char *q;
	/* char getany = (sortit|statreq); ? */
	int nen, nspace;
	struct dirent *dp;
	DIR *dirp;

	name = dptr->realname;
	if((dirp = opendir(name)) == NULL ) {
		lprint(MSGSTR(UNREADABLE,"%s unreadable\n"), name, 0, 0);
		return -1;
	}
	if(sortit) { /* getany? */
		nspace= readalloc(elp_p, (int) (dptr->s.st_size/16));
		if(nspace <= 0) {
closedir:
			closedir(dirp);
			return 0;
		}
	}
	elp= *elp_p;

	BUGLPR(lidebug,BUGACT,("Readdir name = `%s', lx %o\n",
		 name, dptr->lxbits))
	nen= 0;

	while (dp = readdir(dirp)) {
		if(!aflg && dp->d_name[0]=='.') 
			continue;
		p = makename(name, dp->d_name);
		ep = gstat(p, 0);

		BUGLPR(lidebug,BUGACT,("Readdir uid = %d\n", ep->s.st_uid))

		if(nflg || statreq) {
			q = malloc(strlen(p)+1);
			if (q == NULL) 
				complain(CORE);
			strcpy(q,p);

			BUGLPR(lidebug,BUGACT,("Readdir:  realname `%s'\n", q))
			ep->realname= q;
		}

		if (ep->linum != -1 && !statreq) {
			BUGLPR(lidebug,BUGACT,
			      ("Change I-num from `%d` to `%d'\n",
			       ep->linum, dp->d_ino))

			ep->linum = dp->d_ino;
		}
		fixname(ep, dp->d_name);

		nen++;
		if(sortit) { /* getany? */
			if(nen >= nspace) {
				nspace = readrealloc(elp_p, nspace);
				if(nspace <= 0) 
					goto closedir;
				elp = &((*elp_p)[nen-1]);
			}
			*elp++ = ep;
		}
		/* print the entry and maybe (recursively)
		  list its contents; then free it */
		else
                  list_one(ep, dptr);
	}
	closedir(dirp);
	return sortit ? nen : 0; /* getany? */
} /* end li_readdir */

	/* messy - ought to recover if the realloc fails */
readrealloc (arrayptr, nen)
	struct lbuf ***arrayptr;
{
	struct lbuf **arp;
	int nget = nen + QDIRINCR;

	arp = *arrayptr;
	free((void *)arp);
	*arrayptr= (struct lbuf **)realloc((void *)arp,(size_t)(nget*PTRSIZE));
	if ( *arrayptr == NULL) {
		lsflush();
		pass2now = sortit = FALSE;
		lprint(MSGSTR(NOSORTSPACE,"li: Not enough memory available now for sorting.\n"), 0, 0, 0);
		/* if this realloc failed, we may lose some output */
		/* unless we print it out right here. */
		return 0;
	}
	return nget;
} /* end readrealloc */

char *
savestr(str)
char	*str;
{
	char	*cp = malloc(strlen(str) + 1);

	if (cp == NULL)
	{
		fprintf(stderr, MSGSTR(NOMEMORY,"li: out of memory\n"));
		exit(1);
	}
	(void) strcpy(cp, str);
	return (cp);
}

/***********************************************************************/
/* getpwn and getgrpn return false on success, true on failure */

getpwn(uid, ubuf)
char   *ubuf;
{   static struct passwd *pw;
    static prevuid = -1;
    

    if (uid == prevuid)
	return 0;
    if ((pw = getpwuid((uid_t)uid)) != NULL) {
	CScpy(ubuf,pw->pw_name);
	prevuid = uid;
	return 0;
    }
    return 1;
}


getgrpn(gid, gbuf)
register int gid;
char   *gbuf;
{   static struct group *gr;
    static prevgid = -1;
    

    if (gid == prevgid)
	return 0;
    if ((gr = getgrgid((gid_t)gid)) != NULL) {
	CScpy(gbuf,gr->gr_name);
	prevgid = gid;
	return 0;
    }
    return 1;
}

#ifdef DEBUG
/************************* print flags ****************************/

prtflags()                                                      /*rrr*/
{                                                               /*rrr*/
	BUGLPR(lidebug,BUGACT,(aflg ? "  aflg set," : ""))      /*rrr*/
	BUGLPR(lidebug,BUGACT,(dflg ? "  dflg set," : ""))      /*rrr*/
	BUGLPR(lidebug,BUGACT,(fflg ? "  fflg set," : ""))      /*rrr*/
	BUGLPR(lidebug,BUGACT,(lflg ? "  lflg set," : ""))      /*rrr*/
	BUGLPR(lidebug,BUGACT,(nflg ? "  nflg set," : ""))      /*rrr*/
	BUGLPR(lidebug,BUGACT,(sflg ? "  sflg set," : ""))      /*rrr*/
	BUGLPR(lidebug,BUGACT,(vflg ? "  vflg set," : ""))      /*rrr*/
	BUGLPR(lidebug,BUGACT,(xflg ? "  xflg set," : ""))      /*rrr*/
	BUGLPR(lidebug,BUGACT,(Lflg ? "  Lflg set," : ""))      /*rrr*/
								/*rrr*/
	BUGLPR(lidebug,BUGACT,(Faflg ? "  Faflg set," : ""))    /*rrr*/
	BUGLPR(lidebug,BUGACT,(Fbflg ? "  Fbflg set," : ""))    /*rrr*/
	BUGLPR(lidebug,BUGACT,(Fcflg ? "  Fcflg set," : ""))    /*rrr*/
	BUGLPR(lidebug,BUGACT,(Ffflg ? "  Ffflg set," : ""))    /*rrr*/
	BUGLPR(lidebug,BUGACT,(Fgflg ? "  Fgflg set," : ""))    /*rrr*/
	BUGLPR(lidebug,BUGACT,(Fhflg ? "  Fhflg set," : ""))    /*rrr*/
	BUGLPR(lidebug,BUGACT,(Fiflg ? "  Fiflg set," : ""))    /*rrr*/
	BUGLPR(lidebug,BUGACT,(Flflg ? "  Flflg set," : ""))    /*rrr*/
	BUGLPR(lidebug,BUGACT,(Fmflg ? "  Fmflg set," : ""))    /*rrr*/
	BUGLPR(lidebug,BUGACT,(Fnflg ? "  Fnflg set," : ""))    /*rrr*/
	BUGLPR(lidebug,BUGACT,(Foflg ? "  Foflg set," : ""))    /*rrr*/
	BUGLPR(lidebug,BUGACT,(Fpflg ? "  Fpflg set," : ""))    /*rrr*/
	BUGLPR(lidebug,BUGACT,(Frflg ? "  Frflg set," : ""))    /*rrr*/
	BUGLPR(lidebug,BUGACT,(Fsflg ? "  Fsflg set," : ""))    /*rrr*/
	BUGLPR(lidebug,BUGACT,(Fuflg ? "  Fuflg set," : ""))    /*rrr*/
								/*rrr*/
	BUGLPR(lidebug,BUGACT,("\n"))                           /*rrr*/
} /* end print_flags */
#endif

/*
 * NAME: get_host
 *
 * FUNCTION: create a table of vfsnumbers and the host name associated
 *	with the vfsnumber
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * NOTES: 
 *
 * RETURNS: the address of the host table is passed in and the buffer is
 * 	filled
 */

void get_host(hosttable)
char **hosttable;
{
	struct vmount	*vmountp;
	register struct vmount	*vmt;
	register int mounts;
	register int nmounts;

	/* 
	 * get the mntctl information 
	 */

	if ((nmounts = get_vmount(&vmountp)) <= 0)
		exit(1);

	/* 
	 * create a table that has the host id for each vfsnumber
	 */

	for (mounts = nmounts, vmt = vmountp; mounts--; vmt =
		 (struct vmount *)((int)vmt + vmt->vmt_length)) {
		 hosttable[vmt->vmt_vfsnumber] = 
			vmt2dataptr(vmt,VMT_HOST);
	}
}

/*
 * NAME: get_vmount
 *
 * FUNCTION: get the mount status for this local machine using mntctl
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * NOTES: gets the vmount structures
 *
 * RETURNS: < 0 for -error or > 0 for number of struct vmounts in buffer
 *	which is pointed to by pointer at *vmountpp.
 */

static int
get_vmount(vmountpp)
struct vmount	**vmountpp;
{
	struct vmount	*vmountp;
	int		 size;
	int		 nmounts;

	size = BUFSIZ;			/* Initial buffer size */
	while (1) {
	  if ((vmountp = (struct vmount *)malloc(size)) == NULL) {
	    return(-errno);
	  }

	  nmounts = mntctl(MCTL_QUERY, size, vmountp);
	  if (nmounts > 0 && vmountp->vmt_revision != VMT_REVISION) {
	    return(-ESTALE);
	  }
		if (nmounts > 0) {
			*vmountpp = vmountp;
			return(nmounts);
		}
                else
                  if (nmounts == 0) {
			/* the buffer wasn't big enough ... */
			size = vmountp->vmt_revision;
			free((void *)vmountp);
		  }
                  else
                  {
			/* some other kind of error occurred */
			free((void *)vmountp);
			return(-errno);
		  }
	}
	/*NOTREACHED*/
	return 0;
}
