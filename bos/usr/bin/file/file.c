static char sccsid[] = "@(#)19   1.30  src/bos/usr/bin/file/file.c, cmdfiles, bos41J, 9507A 2/7/95 10:33:40";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: file
 *
 * ORIGINS: 3, 27, 71
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
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 * 
 *
 * OSF/1 1.1
 * 
 * $RCSfile: file.c,v $ $Revision: 2.9.2.6 $ (OSF) $Date: 92/03/16 13:17:45 $
 *
 *
 *
 *	File attempts to determine the type of a file by reading the
 *	first 1024 bytes.  It uses a data file /etc/magic to look up
 *	machine specific magic numbers.
 */                                                                   
#define _ILS_MACROS
#include	<stdio.h> 
#include	<ctype.h>
#include	<signal.h>
#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<locale.h>
#include	<string.h>
#include	<sys/access.h>

#include <nl_types.h>
static nl_catd catd;			/* global catalog descriptor */
#include "file_msg.h"
#define MSGSTR(C,D)		catgets(catd,MS_FILE,C,D)


/*
**	Types
*/

#define	BYTE	0
#define SHORT   1
#define LONG    2
#define STR     3

/*
**	Opcodes
*/

#define	EQ	0
#define	GT	1
#define	LT	2
#define	ANY	4
#define SUB     8       /* or'ed in */

/*
**	Misc
*/

#define	NENT	200
#define	BSZ	128
#define	FBSZ	1024
#define	reg	register

/* Assembly lang comment char */
#define ASCOMCHAR '#'

/*
**	Structure of magic file entry
*/

struct	entry	{
	char	e_level;	/* 0 or 1 */
	char	e_type;
	char	e_opcode;
	long	e_off;		/* in bytes */
	long    e_mask;
	union	{
		long	num;
		char	*str;
	}	e_value;
	char	*e_str;
};

typedef	struct entry	Entry;

static Entry	*mtab;
static char	fbuf[FBSZ];
static char	*mfile = "/etc/magic";
static char	*fort[] = {
	"function","subroutine","common","dimension","block","integer",
	"real","data","double","program",
	"FUNCTION","SUBROUTINE","COMMON","DIMENSION","BLOCK","INTEGER",
	"REAL","DATA","DOUBLE","PROGRAM",0};
static char	*asc[] = {
	"sys","mov","tst","clr","jmp",0};
static char	*c[] = {
	"int","char","float","struct","extern","double","void", "static",
	"typedef","signed","unsigned","register","long","short","extern",
	"const","volatile","auto","enum",0};
static char	*as[] = {
	"globl","byte","even","text","data","bss","comm",0};
static void		mkmtab();
static void hexcpy();
static int	i = 0;
static int	fbsz;
static int	ifd;
static char	*filename; /* PTM 35328 */

#define	prf(x)	fprintf(stdout,"%s:%s", x, strlen(x)>6 ? "\t" : "\t\t");
#define	eprf(x)	fprintf(stderr,"%s:%s", x, strlen(x)>6 ? "\t" : "\t\t");

	static nl_catd mfp = (nl_catd) -1;
	static int 	usealt = 0;
	static FILE    *fp;
	static char    buf[BSZ];
	static char *tp;

main(argc, argv)
int argc;
char **argv;
{
	reg	char	*p;
	reg	int	ch;
	reg	FILE	*fl;
	reg	int	cflg = 0, eflg = 0, fflg = 0;
	auto	char	ap[PATH_MAX+1];

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_FILE, NL_CAT_LOCALE);

	while((ch = getopt(argc, argv, "cf:m:")) != EOF)
	switch(ch) {
	case 'c':		/* check for correct magic file syntax */
		cflg++;
		break;

	case 'f':		/* examine list of files in this file  */
		fflg++;
		if ((fl = fopen(optarg, "r")) == NULL) {
			fprintf(stderr,MSGSTR(CANTOPEN,"%s: cannot open %s\n")
			, optarg);
		exit(2);
		}
		break;

	case 'm':		/* alternate magic file. */
		mfile = optarg;
		usealt++;
		break;

	case '?':
		eflg++;
		break;
	}
	if((!cflg) && (!fflg) && (eflg || optind == argc)) {
use:
		fprintf(stderr,MSGSTR(USAGE,
		"usage: file [-c] [-m magicfile] [-f listfile] [file...]\n"));
		exit(2);
	}
	if(cflg) {
		reg	Entry	*ep;

		mkmtab(1);
		printf(MSGSTR(CHEAD,
		"level  off type opcode     mask    value  string\n"));
		for(ep = mtab; ep->e_off != -1L; ep++) {
			/* VFMT */
			printf("%3d%7ld%5d%7o", ep->e_level, ep->e_off,
				ep->e_type, ep->e_opcode);
			if(ep->e_type == STR)
				printf("%18.18s", ep->e_value.str);
			else
				printf("%9lx%9lx", ep->e_mask, ep->e_value.num);
			printf("  %s\n", ep->e_str);
		}
		exit(0);
	}
	for(; fflg || optind < argc; optind += !fflg) {
		reg	int	l;

		if(fflg) {
			if((p = fgets(ap,PATH_MAX+1,fl)) == NULL) { 
				fflg = 0;
				optind--;
				continue;
			}
			l = strlen(p);
			if(l > 0)
				p[l - 1] = '\0';
		} else
			p = argv[optind];
		/* prf(p); */	/* PTM 35328 */
		filename = p;	/* PTM 35328 */
		type(p);
		if(ifd)
			close(ifd);
	}
	exit(0);
}

/*
 * NAME: type
 *                                                                    
 * FUNCTION:	Open each file and determine the type.  First stat the
 *		file then read the first 1024 bytes and try to determine
 *		the best solution.
 *                                                                    
 * RETURN VALUE: none
 *
 */  

static
type(file)
char	*file;
{
	int	j,nl;
	char	ch;
	struct	stat	mbuf;
	char 	slink[PATH_MAX+1];

	ifd = -1;
	if(lstat(file, &mbuf) < 0) {
		prf(file);
		printf(MSGSTR(SCANTOPEN,"cannot open\n"));
		return;
	}
	switch (mbuf.st_mode & S_IFMT) {
	case S_IFLNK:
		prf(file);
		j = readlink(file, slink, sizeof slink);
		if (j >= 0) {
			slink[j]='\0';
			printf(MSGSTR(MLINKTO,"symbolic link to %s\n"), slink);
		}
		else
			printf(MSGSTR(MLINK,"symbolic link\n"));
			
		return;	

	case S_IFCHR:
		prf(file);
		printf(MSGSTR(MCHAR,"character special (%d/%d)\n"),
			major(mbuf.st_rdev), minor(mbuf.st_rdev));
		return;

	case S_IFDIR:
		prf(file);
		printf(MSGSTR(MDIR,"directory\n"));
		return;

	case S_IFIFO:
		prf(file);
		printf(MSGSTR(MFIFO,"fifo\n"));
		return;

	case S_IFSOCK:
		prf(file);
		printf(MSGSTR(MSOCK,"socket\n"));
		return;

	case S_IFBLK:
		prf(file);
		printf(MSGSTR(MBLOCK,"block special (%d/%d)\n"),
			major(mbuf.st_rdev), minor(mbuf.st_rdev));
		return;
	}
	ifd = open(file, 0);
	if(ifd < 0) {
		prf(file);
		printf(MSGSTR(ECOFR,"cannot open for reading\n"));
		return;
	}
	fbsz = read(ifd, fbuf, FBSZ);
	if(fbsz == 0) {
		prf(file);
		printf(MSGSTR(MEMPTY,"empty\n"));
		goto out;
	}
	if(sccs()) {
		prf(file);
		printf(MSGSTR(MSCCS,"sccs\n"));
		goto out;
	}
	if(ckmtab())
		goto out;
	prf(file); /* PTM  35328 */
	i = 0;
	if(ccom() == 0)
		goto notc;
	while(fbuf[i] == '#') {
		j = i;
		while(fbuf[i++] != '\n') {
			if(i - j > 255) {
				printf(MSGSTR(M_DATA,"data\n"));
				goto out;
			}
			if(i >= fbsz)
				goto notc;
		}
		if(ccom() == 0)
			goto notc;
	}
check:
	if(lookup(c) == 1) {
		while((ch = fbuf[i++]) != ';' && ch != '{')
			if(i >= fbsz)
				goto notc;
		printf(MSGSTR(MCPROG,"c program text"));
		goto outa;
	}
	nl = 0;
	while(fbuf[i] != '(') {
		if(fbuf[i] <= (unsigned) 0)
			goto notas;
		if(fbuf[i] == ';'){
			i++; 
			goto check; 
		}
		if(fbuf[i++] == '\n')
			if(nl++ > 6)goto notc;
		if(i >= fbsz)goto notc;
	}
	while(fbuf[i] != ')') {
		if(fbuf[i++] == '\n')
			if(nl++ > 6)
				goto notc;
		if(i >= fbsz)
			goto notc;
	}
	while(fbuf[i] != '{') {
		if(fbuf[i++] == '\n')
			if(nl++ > 6)
				goto notc;
		if(i >= fbsz)
			goto notc;
	}
	printf(MSGSTR(MCPROG,"c program text"));
	goto outa;
notc:
	i = 0;
	while(fbuf[i] == 'c' || fbuf[i] == 'C' || fbuf[i] == '#') {
		while(fbuf[i++] != '\n')
			if(i >= fbsz)
				goto notfort;
	}
	if(lookup(fort) == 1){
		printf(MSGSTR(MFORT,"fortran program text"));
		goto outa;
	}
notfort:
	i = 0;
	if(ascom() == 0)
		goto notas;
	j = i-1;
	if(fbuf[i] == '.') {
		i++;
		if(lookup(as) == 1){
			printf(MSGSTR(MASS,"assembler program text"));
			goto outa;
		}
		else if(j != -1 && fbuf[j] == '\n' && isalpha((int)fbuf[j+2])){
			printf(MSGSTR(MTPROC,
			     "[nt]roff, tbl, or eqn input text"));
			goto outa;
		}
	}
	while(lookup(asc) == 0) {
		if(ascom() == 0)
			goto notas;
		while(fbuf[i] != '\n' && fbuf[i++] != ':')
			if(i >= fbsz)
				goto notas;
		while(fbuf[i] == '\n' || fbuf[i] == ' ' || fbuf[i] == '\t')
			if(i++ >= fbsz)
				goto notas;
		j = i - 1;
		if(fbuf[i] == '.'){
			i++;
			if(lookup(as) == 1) {
				printf(MSGSTR(MASS,"assembler program text")); 
				goto outa; 
			}
			else if(fbuf[j] == '\n' && isalpha((int)fbuf[j+2])) {
				printf(MSGSTR(MTPROC,
				  "[nt]roff, tbl, or eqn input text"));
				goto outa;
			}
		}
	}
	printf(MSGSTR(MASS,"assembler program text"));
	goto outa;
notas:
	for(i=0; i < fbsz; i++) {
		if(fbuf[i]&0200) { 
			if ((unsigned char)(fbuf[0])==(unsigned char)'\100' &&
			    (unsigned char)(fbuf[1])==(unsigned char)'\357')
				printf(MSGSTR(TROFF,"troff output\n"));
			else
				printf(MSGSTR(MDORNLS,"data or International Language text\n"));
			goto out;
		}
	}
	if (access(file, X_OK) == 0)
		printf(MSGSTR(MCOMD,"commands text"));


	else if(english(fbuf, fbsz))
/*ASCII*/
		printf(MSGSTR(MENG,"English text"));
	else
		printf(MSGSTR(MASCII,"ascii text"));
outa:
	while(i < fbsz)
		if((fbuf[i++]&0377) > 127) {
			printf(MSGSTR(AGARB," with garbage\n"));
			goto out;
		}

	fputc('\n',stdout);
out:
	close(ifd);
}

/*
 * NAME: mkmtab
 *                                                                    
 * FUNCTION: 	Create a table of file specific values (from /etc/magic
 *		by default.  If -m option is defined, use that file.
 *
 * RETURN VALUE:  None
 */  

static void
mkmtab(cflg)
int cflg;
{
	reg     char    *p;
	reg	Entry	*ep;
	int     lcnt = 0;
	size_t	nent = (size_t) NENT;
	Entry   *mend;
	char    *nextp;
	static  char whitespace[] = " \t";
	static  char FmtErr[] =
	    "file: format error at line %d, missing %s field.\n";
	static  char MemErr[] =
	    "file: no memory for magic table.\n";
	static  char OpnErr[] =
	    "file: cannot open magic file \"%s\".\n";

	if ((ep = (Entry *) calloc((size_t)sizeof(Entry),nent)) == NULL)
	{
	    fprintf(stderr,MSGSTR(NOMEM,MemErr));
	    exit(2);
	}
	mtab = ep;
	mend = &mtab[nent];
	if (cflg || usealt)	/* if -c or -m is on, we don't use magic.cat */
	    mfp = (nl_catd) -1;
	else {
	    mfp = catopen("magic.cat", NL_CAT_LOCALE);
	} 
	if ((fp = fopen(mfile, "r")) == NULL)
	{   
            fprintf(stderr,MSGSTR(COMF,OpnErr), mfile);
	    exit(2);
	}
	while (getmess(buf))
	{   
	    lcnt++;
	    p = buf;
	    if (*p == '\n' || *p == '#')
		continue;
	    /* LEVEL */
	    if (*p == '>')
	    {   ep->e_level = 1;
		p++;
	    }
	    /* OFFSET */
	    p += strspn(p, whitespace);
	    if (!p || *p == '\0')
	    {   if (cflg)
	            fprintf(stderr,MSGSTR(XOFF,
                    "Format error at line %1%d: The <offset> is missing"));
		continue;
	    }
	    ep->e_off = (long)strtoul(p, &nextp, 0);
	    p = nextp + strspn(nextp, whitespace);
	    if ( !p || *p == '\0')
	    {   if (cflg)
                    fprintf(stderr,MSGSTR(XTYPE,
                    "Format error at line %1%d: The <type> is missing"));
		continue;
	    }
	    if (*p == 's')
	    {   if (*(p+1) == 'h')
		    ep->e_type = SHORT;
		else
		    ep->e_type = STR;
	    } else
		if (*p == 'l')
		    ep->e_type = LONG;
	    p = strpbrk(p, whitespace);		/* Ignore non-blanks */
	    if (p)
	      p += strspn(p, whitespace); 	/*  and skip to next field */

	    /* OP-VALUE */
	    if (!p || *p == '\0')
	    {   if (cflg)
                    fprintf(stderr,MSGSTR(XVALUE,
                    "Format error at line %1%d: The <value> is missing"));
		continue;
	    }
	    if (ep->e_type != STR)
	    {   ep->e_mask = -1L;
		if (*p == '&')
		{   ep->e_mask = (long)strtoul(p+1, &nextp, 0);
		    p = nextp;
		}
		switch(*p)
		{ case '=':
		    ep->e_opcode = EQ;
		    p++;
		    break;
		  case '>':
		    ep->e_opcode = GT;
		    p++;
		    break;
		  case '<':
		    ep->e_opcode = LT;
		    p++;
		    break;
		  case 'x':
		    ep->e_opcode = ANY;
		    p++;
		    break;
		}
	    }
	    if (ep->e_opcode != ANY)
	    {   if (ep->e_type != STR)
		    ep->e_value.num = (long)strtoul(p, &nextp, 0);
		else
		{   nextp = strpbrk(p, whitespace);
		    if (!nextp)
		        {   if (cflg)
                                fprintf(stderr,MSGSTR(XSTRING,
                      "Format error at line %1%d: The <string> is missing"));
		        continue;
	                }
		    *nextp++ = '\0';
		    if ((ep->e_value.str = strdup(p)) == NULL)
			{
				perror ("strdup");
				exit (255);
			}
		    if (!cflg && (p[0] == '0' && p[1] == 'x'))
			hexcpy(ep->e_value.str, p + 2);
		}
		p = nextp;
	    }
	    p += strspn(p, whitespace);
	    nextp = strchr(p, '\n');
	    if (nextp) *nextp = '\0';

	    if (p && (*p != '\0'))
	    {   if ((ep->e_str = strdup(p)) == NULL)
		{
			eprf(filename); /* PTM 35328 */
			perror ("strdup");
			exit(255);
		}
		if (strchr(p, '%'))
		    ep->e_opcode |= SUB;
	    }
	    if( ++ep >= mend)
	    {	nent += NENT;
		if ((ep = (Entry *) realloc((void *)mtab,
					    (size_t)(sizeof(Entry)*nent)
					    )) == NULL)
		{
		    fprintf(stderr,MSGSTR(NOMEM,MemErr));
		    exit(2);
		}
		mtab = ep;
		mend = &mtab[nent];
		ep = &mtab[nent-NENT];
	    }
	}
	ep->e_off = -1L;

	fclose(fp);
}


/*
 * NAME: ckmtab
 *                                                                    
 * FUNCTION: 
 *
 * RETURN VALUE: 1 fail
 *		 0 pass
 */  

static
ckmtab()
{

	reg	Entry	*ep;
	reg	char	*p;
	reg	int	lev1 = 0;
	auto	long	val;
	static	char	init = 0;

	if(!init) {
		mkmtab(0);
		init = 1;
	}
	for(ep = mtab; ep->e_off != -1L; ep++) {
		if(lev1) {
			if(ep->e_level != 1)
				break;
		} else if(ep->e_level == 1)
			continue;
		p = &fbuf[ep->e_off];
		switch(ep->e_type) {
		case STR:
		  	{
			int i, j;
			i = strlen(ep->e_value.str);
			j = strncmp(p,ep->e_value.str,i);
			if(j)
				continue;
			if(!lev1)
				prf(filename); /* PTM 35328 */
			if (ep->e_level > 0)
				putchar (' ');
			if(ep->e_opcode & SUB)
				printf(ep->e_str, ep->e_value.str);
			else
				fputs(ep->e_str,stdout);

			lev1 = 1;
			continue;
			}
		case BYTE:
			val = (long)(*(unsigned char *) p);
			break;

		case SHORT:
			val = (long)(*(unsigned short *) p);
			break;

		case LONG:
			val = (*(long *) p);
			break;
		}
		val &= ep->e_mask;
		switch(ep->e_opcode & ~SUB) {
		case EQ:
			if(val != ep->e_value.num)
				continue;
			break;
		case GT:
			if(val <= ep->e_value.num)
				continue;
			break;
		case LT:
			if(val >= ep->e_value.num)
				continue;
			break;
		}
		if(!lev1)
			prf(filename); /* PTM 35328 */
		if(ep->e_level != 0)
			putchar(' ');
		if(ep->e_opcode & SUB)
			printf(ep->e_str, val);
		else
			fputs(ep->e_str,stdout);

		lev1 = 1;
	}
	if(lev1) {
		putchar('\n'); 	
		return(1);
	}
	return(0);
}

/*
 * NAME: lookup
 *                                                                    
 * FUNCTION:  Look up in the table made in mkmtab from /etc/magic the
 *		type and check for a match with the current position.
 *                                                                    
 * RETURN VALUE: 	1 - Match
 *			0 - no match
 */  

static
lookup(tab)
reg	char **tab;
{
	reg	char	r;
	reg	int	k,j,l;

	while(fbuf[i] == ' ' || fbuf[i] == '\t' || fbuf[i] == '\n')
		i++;
	for(j=0; tab[j] != 0; j++) {
		l = 0;
		for(k=i; ((r=tab[j][l++]) == fbuf[k] && r != '\0');k++);
		if(r == '\0')
			if(fbuf[k] == ' ' || fbuf[k] == '\n' || fbuf[k] == '\t'
			    || fbuf[k] == '{' || fbuf[k] == '/') {
				i=k;
				return(1);
			}
	}
	return(0);
}

/*
 * NAME: ccom
 *                                                                    
 * FUNCTION:	Check to see if the read in buffer is in C format.  
 *                                                                    
 * RETURN VALUE: 0 - not C format
 *		 1 - C format
 */  
static
ccom()
{
	reg	char	cc;

	while((cc = fbuf[i]) == ' ' || cc == '\t' || cc == '\n')
		if(i++ >= fbsz)
			return(0);
	if(fbuf[i] == '/' && fbuf[i+1] == '*') {
		i += 2;
		while(fbuf[i] != '*' || fbuf[i+1] != '/') {
			if(fbuf[i] == '\\')
				i += 2;
			else
				i++;
			if(i >= fbsz)
				return(0);
		}
		if((i += 2) >= fbsz)
			return(0);
	}
	if(fbuf[i] == '\n')
		if(ccom() == 0)
			return(0);
	return(1);
}

/*
 * NAME: ascom
 *                                                                    
 * FUNCTION:	Check to see if the read in buffer is in ascom format.  
 *                                                                    
 * RETURN VALUE: 0 - not assembler
 *		 1 - assembler
 */  
static
ascom()
{
	while(fbuf[i] == ASCOMCHAR) {
		i++;
		while(fbuf[i++] != '\n')
			if(i >= fbsz)
				return(0);
		while(fbuf[i] == '\n')
			if(i++ >= fbsz)
				return(0);
	}
	return(1);
}

/*
 * NAME: sccs
 *                                                                    
 * FUNCTION:	Check to see if the read in buffer is in sccs format.  
 *		<1h######> in the first line.
 *                                                                    
 * RETURN VALUE: 0 - not sccs
 *		 1 - sccs
 */  

static
sccs() {
	reg int j;

	if(fbuf[0] == 1 && fbuf[1] == 'h')
		for(j=2; j<=6; j++)
			if(isdigit((int)fbuf[j])) continue;
			else return(0);
	else
		return(0);
	return(1);
}

/*
 * NAME: english
 *                                                                    
 * FUNCTION:	Check to see if the read in buffer is in english format.  
 *		Just check the punctuation.
 *                                                                    
 * RETURN VALUE: 0 - not english
 *		 1 - english
 */  
static
english (bp, n)
char *bp;
{

	reg	int	j, vow, freq, rare;
	reg	int	badpun = 0, punct = 0;
	auto	int	ct[128];    	/* frequency-count for ascii chars */

	if (n<50)
		return(0); /* no point in statistics on squibs */
	for(j=0; isascii(j); j++)
		ct[j]=0;
	for(j=0; j<n; j++)
	{
		if (isascii(bp[j]))
			ct[bp[j]|040]++;
		switch (bp[j])
		{
		case '.': 
		case ',': 
		case ')': 
		case '%':
		case ';': 
		case ':': 
		case '?':
			punct++;
			if(j < n-1 && bp[j+1] != ' ' && bp[j+1] != '\n')
				badpun++;
		}
	}
	if (badpun*5 > punct)
		return(0);
	vow = ct['a'] + ct['e'] + ct['i'] + ct['o'] + ct['u'];
	freq = ct['e'] + ct['t'] + ct['a'] + ct['i'] + ct['o'] + ct['n'];
	rare = ct['v'] + ct['j'] + ct['k'] + ct['q'] + ct['x'] + ct['z'];
	if(2*ct[';'] > ct['e'])
		return(0);
	if((ct['>']+ct['<']+ct['/'])>ct['e'])
		return(0);	/* shell file test */
	return (vow*5 >= n-ct[' '] && freq >= 10*rare);
}
/*
 * NAME: getmess
 *                                                                    
 * FUNCTION:  get a line out of the catalog file.
 *                                                                    
 * RETURN VALUE: 0 read failed;
 *		 1 read from either catalog or magic file.
 */  

static int
getmess(buf)
char *buf;
{
	static int count = 1;		/* which message */

	if (mfp == (nl_catd)(-1) || usealt) 
		return (fgets(buf, BSZ, fp) != NULL);
	else
	{
	        tp = catgets( mfp, 1, count++, NULL );
		if(tp)
		  strncpy(buf,tp,BSZ);
		else
		  /*
		   * No message or no more messages in catalog
		   */
		  return (fgets(buf, BSZ, fp) != NULL);

		return(1);
	}
}

/*
 * NAME: hexcpy
 *                                                                    
 * RETURN VALUE: None
 */  
static void
hexcpy(str,p)
char *str;
char *p;
{
	for (; isxdigit((int)*p) && *p != '\0'; str++, p++) 
	{
		if (*p < 'A')
			*str = (*p & 07) << 4;
		else 	*str = (((*p + 9) & 017) << 4);
		p++;
		if (*p < 'A')
			*str |= (*p & 017);
		else	
			*str |= ((*p + 9) & 017);
	}
	*str = 0;
}		

