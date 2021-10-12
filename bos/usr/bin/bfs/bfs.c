static char sccsid[] = "@(#)31	1.19  src/bos/usr/bin/bfs/bfs.c, cmdscan, bos411, 9428A410j 11/24/93 12:24:07";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS: bfs
 *
 * ORIGINS: 3, 27
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
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 */                                                                   

#include <unistd.h>		/* SEEK_SET */
#include <sys/stat.h>		/* S_IR..., S_IW... */
#include <fcntl.h>		/* O_RDONLY */
#include <sys/param.h>		/* BSIZE */
#include <stdio.h>		/* contains limits.h */
#include <setjmp.h>		/* longjmp, setjmp, jmp_buf */
#include <signal.h>		/* signal */
#include <locale.h>
#include <nl_types.h>
#include <stdlib.h>
#include <regex.h>
#include "bfs_msg.h"
#include  <string.h>
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_BFS,Num,Str)

jmp_buf env;

struct Comd {
	int Cnumadr;
	int Cadr[2];
	char Csep;
	char Cop;
};

#define MAX_BFSFILE_SIZE	32767	/* Maximum file size */
#define S_BFSRW		S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#define LINE_LENGTH	256		/* Maximum line size */

int range;
int Dot, Dollar;
int markarray[26], *mark;		/* valid markers are a-z */
int fstack[15] = {1, -1};
int infildes = STDIN_FILENO;		/* Current input file */
int outfildes = STDOUT_FILENO;		/* output file */
char internal[BSIZE], *intptr;
char comdlist[LINE_LENGTH];
char charbuf = '\n';
int peeked;
char currex[LINE_LENGTH];
int trunc = (LINE_LENGTH-1);
int crunch = -1;
int segblk[BSIZE], segoff[BSIZE], txtfd, prevblk, prevoff;
int saveblk = -1;
int oldfd = 0;
int flag4 = 0;
int flag3 = 0;
int flag2 = 0;
int flag1 = 0;
int flag = 0;
int gflag = 0;
int lprev = 1;
int status[1];
char *lincnt, tty, *bigfile;
char fle[PATH_MAX];
char prompt = 0;
char verbose = 1;
char varray[10][100];
double outcnt;
char strtmp[32];
static void reset(void);
int getrex(struct Comd *p, int prt, char c);
int getstr(int prt, char *buf, char brk, char ignr, int nonl);
int begin(struct Comd *p);
int bigopen(char *file);
int sizeprt(int blk, int off);
void ecomd(void);
int fcomd(struct Comd *p);
int gcomd(struct Comd *p, int k);
int kcomd(struct Comd *p);
int ncomd(struct Comd *p);
int pcomd(struct Comd *p);
int qcomd(void);
int xcomds(struct Comd *p);
int xbcomd(struct Comd *p);
int xccomd(struct Comd *p);
int xfcomd(struct Comd *p);
int xocomd(struct Comd *p);
int xtcomd(struct Comd *p);
int xvcomd(void);
int wcomd(struct Comd *p);
int nlcomd(struct Comd *p);
int eqcomd(struct Comd *p);
int colcomd(struct Comd *p);
int xcomdlist(struct Comd *p);
int excomd(void);
int defaults(struct Comd *p, int prt, int max, int def1, int def2, int setdot, int errsok);
int getcomd(struct Comd *p, int prt);
int getadrs(struct Comd *p, int prt);
int getadr(struct Comd *p, int prt);
int getrel(struct Comd *p, int prt);
int getmark(struct Comd *p, int prt);
int hunt(int prt, char *rex, int start, int down, int wrap, int errsok);
int jump(int prt, char *label);
int err(int prt, char *msg);
int getch(void);
int readc(int f, char *c);
int percent (char *line);
int newfile(int prt, char *f);
int push(int s[], int d);
int pop(int s[]);
int peekc(void);
int eat(void);
int more(void);
int out(char *ln);
char *untab(char *l);
int patoi(char *b);
int size(char *s);
int equal(char *a, char *b);
int getnumb(struct Comd *p, int prt);
int rdnumb(int prt);

/*
 * FUNCTION: bfs
 *
 * 'bfs' is a read-only version of ed, with the exception of
 * being able to handle much larger files and some additional subcommands.
 * Input files can be up to 32k lines long, with up to LINE_LENGTH-1
 * characters per line.
 */

main(int argc, char *argv[])
{
	struct Comd comdstruct, *p;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_BFS,NL_CAT_LOCALE);

	if(argc < 2 || argc > 3) {
		fprintf(stderr, MSGSTR(USAGE,"Usage: bfs [-] filename\n"));
		exit(1);
	}

	mark = markarray-'a';
	if(argc == 3) verbose = 0;
	setbuf(stdout, (char *)0);
	if(bigopen(bigfile=argv[argc-1])) exit(1);
	tty = isatty(0);

	p = &comdstruct;
	setjmp(env);
	signal(SIGINT,(void (*) (int))reset);
	err(0,"");
	printf("\n");
	flag = 0;
	while(1) begin(p);
}


/*
 * NAME: reset
 *
 * FUNCTION: This function is called when a signal is received.  It
 * will return to the setjump state.
 *
 * RETURN VALUE DESCRIPTION: always 1
 */  

void
reset(void)		/* for longjmp on signal */
{
	longjmp(env,1);
}
 

/*
 * NAME: begin
 *
 * FUNCTION: This is the actual interactive part of bfs.  Commands
 * will be read, parsed and executed from this main loop.
 */

begin(struct Comd *p)
{
	char line[LINE_LENGTH];

strtagn:	
if(flag == 0) eat();
	if(infildes != 100) {
		if(infildes == STDIN_FILENO && prompt) printf("*");
		flag3 = 1;
		getstr(1,line,(char)0,(char)0,0);
		flag3 = 0;
		if(percent(line) < 0) goto strtagn;
		newfile(1,"");		/* initialize things */
	}
	if (getcomd(p,1) < 0)
		return;
	switch (p->Cop) {

		case 'e':	if(!flag) ecomd();
				else err(0,"");
				break;

		case 'f':	fcomd(p);
				break;

		case 'g':	if(flag == 0) gcomd(p,1);
				else err(0,"");
				break;

		case 'k':	kcomd(p);
				break;

		case 'n':	ncomd(p);
				break;

		case 'p':	pcomd(p);
				break;

		case 'q':	qcomd();
				break;

		case 'v':	if(flag == 0) gcomd(p,0);
				else err(0,"");
				break;

		case 'x':	if(!flag) xcomds(p);
				else err(0,"");
				break;

		case 'w':	wcomd(p);
				break;

		case '\n':	nlcomd(p);
				break;

		case '=':	eqcomd(p);
				break;

		case ':':	colcomd(p);
				break;

		case '!':	excomd();
				break;

		case 'P':	prompt = !prompt;
				break;

		default:	if(flag) err(0,"");
				else err(1,MSGSTR(BADCMD,
						"bad command"));
				break;
	}
}


/*
 * NAME: bigopen
 *
 * FUNCTION: open a newfile to be looked at and reset globals.
 */

bigopen(char *file)
{
	int l, off, cnt;
	int blk, newline, n, s;
	char block[BSIZE];

	if((txtfd=open(file,O_RDONLY)) < 0) {
		prompt=1;
		return(err(1,MSGSTR(CANTOPEN,"can't open the file")));
	}

	saveblk = -1;
	prevblk = 0;
	prevoff = 0;
	lprev   = 0;

	blk = -1;
	newline = 1;
	l = cnt = s = 0;
	off = BSIZE;
	if((lincnt=(char*)sbrk(4096)) == (char*)-1) {
		prompt=1;
		return(err(1,MSGSTR(TOOLINES,"too many lines")));
	}

	while((n=read(txtfd,block,BSIZE)) > 0) {
		blk++;
		for(off=0; off<n; off++) {
			if(newline) {
				newline = 0;
				if(l>0 && !(l&07777))
					if(sbrk(4096) == -1) {
					  prompt=1;
					  return(err(1,MSGSTR(TOOLINES,
						"too many lines")));
				}
				lincnt[l] = cnt;
				cnt = 0;
				if(!(l++ & 077)) {
					segblk[s] = blk;
					segoff[s++] = off;
				}
				if(l < 0 || l > MAX_BFSFILE_SIZE){
					prompt=1;
					return(err(1,MSGSTR(TOOLINES,
						"too many lines")));
				}
			}
			if(block[off] == '\n') newline = 1;
			cnt++;
		}
	}
	if(!(l&07777)) if(sbrk(2) == -1) {
		prompt = 1;
		return(err(1,MSGSTR(TOOLINES,"too many lines"))); 
	}
	lincnt[Dot = Dollar = l] = cnt;
	sizeprt(blk,off);
	return(0);
}

/*
 * NAME: sizeprt
 *                                                                    
 * FUNCTION: print out the size of the read in file.
 */  


sizeprt(int blk, int off)
{
	if(verbose) 
		printf("%.0f",((float)BSIZE)*blk+off);
}


/*
 * NAME: bigread
 *                                                                    
 * FUNCTION: Read in buffer from file.
 */  

bigread(int l, char *rec)
{
	register int i;
	register char *r, *b;
	int off;
	static char savetxt[BSIZE];

	if((i=l-lprev) == 1) prevoff += lincnt[lprev]&0377;
	else if(i >= 0 && i <= 32)
		for(i=lprev; i<l; i++) prevoff += lincnt[i]&0377;
	else if(i < 0 && i >= -32)
		for(i=lprev-1; i>=l; i--) prevoff -= lincnt[i]&0377;
	else {
		prevblk = segblk[i=(l-1)>>6];
		prevoff = segoff[i];
		for(i=(i<<6)+1; i<l; i++) prevoff += lincnt[i]&0377;
	}

	prevblk += (prevoff/BSIZE);
	prevoff &= (BSIZE-1);
	lprev = l;

	if(prevblk != saveblk) {
		lseek(txtfd,((long)(saveblk=prevblk)) * BSIZE,SEEK_SET);
		read(txtfd,savetxt,BSIZE);
	}

	r = rec;
	off = prevoff;
	while(1) {
		for(b=(char *)(((int)savetxt)+off);
			b<(char *)(((int)savetxt)+BSIZE); b++) {
			if((*r++ = *b) == '\n') {
				*(r-1) = '\0';
				return;
			}
			if(((unsigned)r - (unsigned)rec) > (LINE_LENGTH-2)) {
				write(2, "line too long\n", 14);
				exit(1);
			}
		}
		read(txtfd,savetxt,BSIZE);
		off = 0;
		saveblk++;
	}
}


/*
 * NAME: ecomd
 *                                                                    
 * FUNCTION: The 'e' command means the user wants to open a new file.
 */  

void ecomd(void)
{
	register int i = 0;

	while(peekc() == ' ') getch();
	while((fle[i++] = getch()) != '\n');
	fle[--i] = '\0';
	if(bigopen(bigfile = fle)) exit(1);
	printf("\n");
}

/*
 * NAME: fcomd
 *
 * FUNCTION: Display the file being scanned.
 */

fcomd(struct Comd *p)
{
	if(more() || defaults(p,1,0,0,0,0,0)) return(-1);
	printf("%s\n",bigfile);
	return(0);
}


/*
 * NAME: gcomd
 *
 * FUNCTION: This is the ed 'g' command. 
 */

gcomd(struct Comd *p, int k)
{
	register char d;
	register int i, end;
	char line[LINE_LENGTH];
	regex_t re;

	if(defaults(p,1,2,1,Dollar,0,0)) return(-1);

	if((d=getch()) == '\n')
		return(err(1,MSGSTR(SYNTAX,"syntax")));	
	if(peekc() == d) getch();
	else 
		if(getstr(1,currex,d,(char)0,1)) return(-1);
	if (regcomp(&re, currex, REG_EXTENDED|REG_NOSUB) != 0)
		return(err(1,MSGSTR(SYNTAX,"syntax"))); 

	if(getstr(1,comdlist,(char)0,(char)0,0)) return(-1);
	i = p->Cadr[0];
	end = p->Cadr[1];
	gflag = 1;
	while (i<=end) {
		bigread(i,line);
		if(regexec(&re, line, (size_t)0, (regmatch_t *)NULL, 0 ) != 0) {
			if(!k) {
				Dot = i;
				if (xcomdlist(p))
					return(err(1,MSGSTR(BADCMDL,
						"bad cmd list"))); 
			}
			i++;
		}
		else {
			if(k) {
				Dot = i;
				if (xcomdlist(p))
					return(err(1,MSGSTR(BADCMDL,
						"bad cmd list"))); 
			}
			i++;
		}
	}
	gflag = 0;
	return(0);
}


/*
 * NAME: kcomd
 *
 * FUNCTION: do the ed 'k' command
 */

kcomd(struct Comd *p)
{
	register char c;

	if((c = peekc()) < 'a' || c > 'z')
		return(err(1,MSGSTR(BADMARK,"bad mark"))); 
	getch();
	if(more() || defaults(p,1,1,Dot,0,1,0)) return(-1);

	mark[c] = Dot = p->Cadr[0];
	return(0);
}


/*
 * NAME: ncomd
 *                                                                    
 * FUNCTION: ed 'n' command.  User wants to start looking at a new file.
 *                                                                    
 */  

ncomd(struct Comd *p)
{
	char line[LINE_LENGTH];
	int i;

	if(more() || defaults(p,1,2,Dot,Dot,1,0)) return(-1);

	for(i = p->Cadr[0]; i <= p->Cadr[1] && i > 0; i++) {
		bigread(i,line);
		printf("%d	",i);
		out(line);
	}

	return(0);
}


/*
 * NAME: pcomd
 *                                                                    
 * FUNCTION: ed 'p' command.  Print the current line.
 *                                                                    
 */  

pcomd(struct Comd *p)
{
	register int i;
	char line[LINE_LENGTH];

	if(more() || defaults(p,1,2,Dot,Dot,1,0)) return(-1);

	for(i = p->Cadr[0]; i <= p->Cadr[1] && i>0; i++) {
		bigread(i,line);
		out(line);
	}
	return(0);
}


/*
 * NAME: qcomd
 *                                                                    
 * FUNCTION: ed 'q' command.  Quit the program.
 *                                                                    
 */  

qcomd(void)
{
	if(more()) return(-1);
	exit(1);
}


/* All of the following x commands will be the same as their 'ed'
 * counterpart and be appended with 'comd'.
 */

xcomds(struct Comd *p)
{
	switch(getch()) {
		case 'b':	return(xbcomd(p));
		case 'c':	return(xccomd(p));
		case 'f':	return(xfcomd(p));
		case 'o':	return(xocomd(p));
		case 't':	return(xtcomd(p));
		case 'v':	return(xvcomd());
		default:	return(err(1,MSGSTR(BADCMD,
						"bad command")));
	}
}


xbcomd(struct Comd *p)
{
	register int fail, n;
	register char d;
	char str[50];

	fail = 0;
	if(defaults(p,0,2,Dot,Dot,0,1)) fail = 1;
	else {
		if((d=getch()) == '\n')
			return(err(1,MSGSTR(SYNTAX,"syntax")));	
		if(d == 'z') {
			if(status[0] != 0) return(0);
			getch();
			if(getstr(1,str,(char)0,(char)0,0)) return(-1);
			return(jump(1,str));
		}
		if(d == 'n') {
			if(status[0] == 0) return(0);
			getch();
			if(getstr(1,str,(char)0,(char)0,0)) return(-1);
			return(jump(1,str));
		}
		if(getstr(1,str,d,' ',0)) return(-1);
		if((n = hunt(0,str,p->Cadr[0]-1,1,0,1)) < 0)
			fail = 1;
		if(getstr(1,str,(char)0,(char)0,0)) return(-1);
		if(more())
			return(err(1,MSGSTR(SYNTAX,"syntax"))); 
	}

	if(!fail) {
		Dot = n;
		return(jump(1,str));
	}
	return(0);
}


xccomd(struct Comd *p)
{
	char arg[100];

	if(getstr(1,arg,(char)0,' ',0) || defaults(p,1,0,0,0,0,0)) return(-1);

	if(equal(arg,"")) crunch = -crunch;
	else if(equal(arg,"0")) crunch = -1;
	else if(equal(arg,"1")) crunch = 1;
	else return(err(1,MSGSTR(SYNTAX,"syntax")));

	return(0);
}


xfcomd(struct Comd *p)
{
	char fl[100];
	register char *f;

	if(defaults(p,1,0,0,0,0,0)) return(-1);

	while(peekc() == ' ') getch();
	for(f = fl; (*f = getch()) != '\n'; f++);
	if(f == fl)
		return(err(1,MSGSTR(NOFILEM,"no file")));
	*f = '\0';

	return(newfile(1,fl));
}


xocomd(struct Comd *p)
{
	register int fd;
	char arg[100];

	if(getstr(1,arg,(char)0,' ',0) || defaults(p,1,0,0,0,0,0))
		return(-1);

	if(!arg[0]) {
		if(outfildes == STDOUT_FILENO)
			return(err(1,MSGSTR(NODIV,"no diversion"))); 
		close(outfildes);
		outfildes = STDOUT_FILENO;
	}
	else {
		if(outfildes != STDOUT_FILENO)
			return(err(1,MSGSTR(ALRDYDIV,"already diverted")));
		if((fd = creat(arg,S_BFSRW)) < 0)
			return(err(1,MSGSTR(CANTCREAT,"can't create")));
		outfildes = fd;
	}
	return(0);
}


xtcomd(struct Comd *p)
{
	register int t;
	register char c;

	while((c = peekc()) == ' ') getch();
	if (c == '\n') { /* default selected */
	   trunc = LINE_LENGTH -1;
	   return(0);
	}
	if((t = rdnumb(1)) < 0 || more() || defaults(p,1,0,0,0,0,0))
		return(-1);

	trunc = t;
	return(0);
}


xvcomd(void)
{
	register char c;
	register int i;
	int temp0, temp1, temp2;
	int fildes[2];

	if((c = peekc()) < '0' || c > '9')
		return(err(1,MSGSTR(DIGRQD,"digit required")));
	getch();
	c -= '0';
	while(peekc() == ' ') getch();
	if(peekc() == '\\') getch();
	else if(peekc() == '!') {
		if(pipe(fildes) < 0) {
			printf(MSGSTR(TRYAGN, "Try again"));
			return;
		}
		temp0 = dup(0);
		temp1 = dup(1);
		temp2 = infildes;
		close(0);
		dup(fildes[0]);
		close(1);
		dup(fildes[1]);
		close(fildes[0]);
		close(fildes[1]);
		getch();
		flag4 = 1;
		excomd();
		close(1);
		infildes = STDIN_FILENO;
	}

	for(i = 0;(varray[c][i] = getch()) != '\n';i++);
	varray[c][i] = '\0';
	if(flag4) {
		infildes = temp2;
		close(0);
		dup(temp0);
		close(temp0);
		dup(temp1);
		close(temp1);
		flag4 = 0;
		charbuf = ' ';
	}
	return(0);
}


wcomd(struct Comd *p)
{
	register int i, fd, savefd;
	int savecrunch, savetrunc;
	char arg[100], line[LINE_LENGTH];

	if(getstr(1,arg,(char)0,' ',0) || defaults(p,1,2,1,Dollar,1,0))
		return(-1);
	if(!arg[0]) return(err(1,MSGSTR(NOFLNM,"no file name")));
	if(equal(arg,bigfile))
		return(err(1,MSGSTR(NOCHANGE,"no change indicated")));
	if((fd=creat(arg,S_BFSRW)) <0)
		return(err(1,MSGSTR(CANTCREAT,"can't create")));

	savefd = outfildes;
	savetrunc = trunc;
	savecrunch = crunch;
	outfildes = fd;
	trunc = (LINE_LENGTH-1);
	crunch = -1;

	outcnt = 0;
	for(i = p->Cadr[0]; i <= p->Cadr[1] && i > 0; i++) {
		bigread(i,line);
		out(line);
	}
	if(verbose) printf("%.0f\n",outcnt);
	close(fd);

	outfildes = savefd;
	trunc = savetrunc;
	crunch = savecrunch;
	return(0);
}


/*
 * NAME: nlcomd
 *
 * FUNCTION: '\n' command.  Print lines.
 */

nlcomd(struct Comd *p)
{
	if(defaults(p,1,2,Dot+1,Dot+1,1,0)) {
		getch();
		return(-1);
	}
	return(pcomd(p));
}


/*
 * NAME: eqcomd
 *
 * FUNCTION: '=' command.  Print current line.
 */

eqcomd(struct Comd *p)
{
	if(more() || defaults(p,1,1,Dollar,0,0,0))
		return(-1);
	printf("%d\n",p->Cadr[0]);
}


/*
 * NAME: colcomd
 *
 * FUNCTION: ':' command.
 */

colcomd(struct Comd *p)
{
	return(defaults(p,1,0,0,0,0,0));
}


xcomdlist(struct Comd *p)
{
	flag = 1;
	flag2 = 1;
	newfile(1,"");
	while(flag2) begin(p);
	if(flag == 0) return(1);
	flag = 0;
	return(0);
}

/* Remove when sleep in pri.h is not redefined. */
#undef sleep
excomd(void)
{
	register int i;

	if(infildes != 100) charbuf = '\n';
	while((i = fork()) < 0) sleep(10);
	if(!i) {
		signal(SIGINT, SIG_DFL); /*Guarantees child can be intr. */
		if(infildes == 100 || flag4) {
			execl("/usr/bin/sh","sh","-c",intptr,0);
			exit(0);
		}
		if(infildes != STDIN_FILENO) {
			close(0);
			dup(infildes);
		}
		for(i = 3; i < 15; i++) close(i);
		execl("/usr/bin/sh","sh","-t",0);
		exit(0);
	}
	signal(SIGINT, SIG_IGN);
	while(wait(status) != i);
	status[0] = status[0] >> 8;
	signal(SIGINT, (void (*)(int))reset); /* Restore previous signal mask */

	if((infildes == STDIN_FILENO || (infildes  == 100 &&
		fstack[fstack[0]] == 0)) && verbose && (!flag4))
			printf("!\n");
	return(0);
}


defaults(struct Comd *p, int prt, int max, int def1, int def2,
	int setdot, int errsok)
{
	range = 0;
	if(!def1) def1 = Dot;
	if(!def2) def2 = def1;
	if(p->Cnumadr >= max)
		return(errsok?-1:err(prt,MSGSTR(CUSAGE,"adr count")));
	if(p->Cnumadr < 0) {
		p->Cadr[++p->Cnumadr] = def1;
		p->Cadr[++p->Cnumadr] = def2;
	}
	else if(p->Cnumadr < 1) {
		p->Cadr[++p->Cnumadr] = p->Cadr[0];
		range = p->Cadr[0];
	     }
	     else
		range = p->Cadr[1];
	if(p->Cadr[0] < 1 || p->Cadr[0] > Dollar ||
	   p->Cadr[1] < 1 || p->Cadr[1] > Dollar)
		return(errsok?-1:err(prt,MSGSTR(RANGE,"range")));
	if(p->Cadr[0] > p->Cadr[1])
		return(errsok?-1:err(prt,MSGSTR(ADDRERR,"adr1 > adr2")));
	if(setdot) Dot = p->Cadr[1];
	return(0);
}


/*
 * NAME: getcomd
 *
 * FUNCTION: Get command from input for begin() function
 */

getcomd(struct Comd *p, int prt)
{
	register int r;
	register char c;

	p->Cnumadr = -1;
	p->Csep = ' ';
	switch(c = peekc()) {
		case ',':
		case ';':       getch();
				p->Cadr[++p->Cnumadr] = (c == ',') ? 1 : Dot;
				p->Cadr[++p->Cnumadr] = Dollar;
				p->Cop = getch();
				return(0);
	}

	if((r=getadrs(p,prt)) < 0) return(r);

	if((int)(c=peekc()) < 0) return(err(prt,MSGSTR(SYNTAX,"syntax")));
	if(c == '\n') {
		if(gflag)
			p->Cop = 'p';
		else
			p->Cop = '\n';

		}
	else
		p->Cop = getch();

	return(0);
}


getadrs(struct Comd *p, int prt)
{
	register int r;
	register char c;

	if((r=getadr(p,prt)) < 0) return(r);

	switch(c=peekc()) {
		case ';':	Dot = p->Cadr[0];
		case ',':       getch();
				p->Csep = c;
				return(getadr(p,prt));
	}

	return(0);
}


/*
 * NAME: getadr
 *
 * FUNCTION: Get the actual next command input character for then line editor.
 */  

getadr(struct Comd *p, int prt)
{
	register int r;
	register char c;

	r = 0;
	switch(c = peekc()) {
		case '\n':
		case ',':       
		case ';':	return(0);

		case '\'':      getch();
				r = getmark(p,prt);
				break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':	r = getnumb(p,prt);
				break;

		case '.':       getch();
		case '+':
		case '-':	p->Cadr[++p->Cnumadr] = Dot;
				break;

		case '$':       getch();
				p->Cadr[++p->Cnumadr] = Dollar;
				break;

		case '^':       getch();
				p->Cadr[++p->Cnumadr] = Dot - 1;
				break;

		case '/':
		case '?':
		case '>':
		case '<':       getch();
				r = getrex(p,prt,c);
				break;

		default:	return(0);
	}

	if(r == 0) r = getrel(p,prt);
	return(r);
}


/*
 * NAME: getnumb
 *
 * FUNCTION: Call rdnumb to input a number.
 */

getnumb(struct Comd *p, int prt)
{
	register int i;


	if((i=rdnumb(prt)) < 0) return(-1);
	p->Cadr[++p->Cnumadr] = i;
	return(0);
}


/*
 * NAME: rdnumb
 *
 * FUNCTION: Input a number.
 */

rdnumb(int prt)
{
	char num[20], *n;
	int i;

	n = num;
	while((*n=peekc()) >= '0' && *n <= '9') {
		n++;
		getch();
	}

	*n = '\0';
	if((i=patoi(num)) >= 0)
		return(i);
	return(err(prt,MSGSTR(BADNUM,"bad num")));
}


getrel(struct Comd *p, int prt)
{
	register int op, n;
	register char c;
	int j;

	n = 0;
	op = 1;
	while((c=peekc())=='+' || c=='-') {
		if(c=='+') n++;
		else n--;
		getch();
	}
	j = n;
	if(n < 0) op = -1;
	if(c=='\n') p->Cadr[p->Cnumadr] += n;
	else {
		if((n=rdnumb(0)) > 0 && p->Cnumadr >= 0) {
			p->Cadr[p->Cnumadr] += op*n;
			getrel(p,prt);
		}
		else {
			if(c=='-')
				p->Cadr[p->Cnumadr] += j;
			else
				p->Cadr[p->Cnumadr] += j;
		}
	}
	return(0);
}


/*
 * NAME: getmark
 *
 * FUNCTION: get a marker.  It must be a small letter from a-z.
 */

getmark(struct Comd *p, int prt)
{
	register char c;

	if((c=peekc()) < 'a' || c > 'z')
		return(err(prt,MSGSTR(BADMARK,"bad mark")));
	getch();

	if(!mark[c])	
		return(err(prt,MSGSTR(UNDEFMARK,"undefined mark")));

	p->Cadr[++p->Cnumadr] = mark[c];
	return(0);
}


/*
 * NAME: getrex
 *
 * FUNCTION: Figure out what type of search user wants to do
 * and call hunt to do the search.
 */

getrex(struct Comd *p, int prt, char c)
{
	register int down, wrap, start;

	if(peekc() == c) getch();
	else if(getstr(prt,currex,c,(char)0,1)) return(-1);

	switch(c) {
		case '/':	down = 1; wrap = 1; break;
		case '?':	down = 0; wrap = 1; break;
		case '>':	down = 1; wrap = 0; break;
		case '<':	down = 0; wrap = 0; break;
	}

	if(p->Csep == ';') start = p->Cadr[0];
	else start = Dot;

	if((p->Cadr[++p->Cnumadr]=hunt(prt,currex,start,down,wrap,0)) < 0)
		return(-1);
	return(0);
}


/*
 * NAME: hunt
 *
 * FUNCTION: Search for a pattern based on user input.  Direction and
 * length are specified by parameters.
 */

hunt(int prt, char *rex, int start, int down, int wrap, int errsok)
{
	register int i, end1, incr;
	int start1, start2;
	char line[LINE_LENGTH];
	regex_t re;

	if(down) {
		start1 = start + 1;
		end1 = range ? range : Dollar;
		start2 = 1;
		incr = 1;
	}
	else {
		start1 = start  - 1;
		end1 = 1;
		start2 = range ? range : Dollar;
		incr = -1;
	}

	if (regcomp(&re, rex, REG_EXTENDED|REG_NOSUB) != 0)
		return(errsok?-1:err(prt,MSGSTR(SYNTAX,"syntax")));

	for(i=start1; i != end1+incr; i += incr) {
		bigread(i,line);
		if(regexec(&re, line, (size_t)0, (regmatch_t *)NULL, 0 ) == 0)
			return(i);
	}

	if(!wrap)	
		return(errsok?-1:err(prt,MSGSTR(NOTFOUND,"not found"))); 

	for(i=start2; i != start1; i += incr) {
		bigread(i,line);
		if(regexec(&re, line, (size_t)0, (regmatch_t *)NULL, 0 ) == 0)
			return(i);
	}

	return(errsok?-1:err(prt,MSGSTR(NOTFOUND,"not found")));
}


/*
 * NAME: jump
 *
 * FUNCTION: Go to a user specified label.
 */

jump(int prt, char *label)
{
	register char  *l;
	char line[LINE_LENGTH];
	regex_t re;

	if(infildes == STDIN_FILENO && tty)
		return(err(prt,MSGSTR(JUMPTTY,"jump on tty")));
	if(infildes == 100) intptr = internal;
	else lseek(infildes,0L,SEEK_SET);

	sprintf(strtmp, "^: *%s$", label);
	if (regcomp(&re, strtmp, REG_EXTENDED|REG_NOSUB) != 0)
		return (-1);

	for(l=line; readc(infildes,l); l++) {
		if(*l == '\n') {
			*l = '\0';
			if(regexec(&re, line, (size_t)0, (regmatch_t *)NULL, 0 ) == 0) {
				charbuf = '\n';
				return(peeked = 0);
			}
			l = line - 1;
		}
	}

	return(err(prt,MSGSTR(LBLNFND,"label not found")));
}


/*
 * NAME: getstr
 *
 * FUNCTION: Get the next line of input to be viewed.
 *
 * DATA STRUCTURES: line is returned thru parameter buf.
 *
 * RETURN VALUE DESCRIPTION:   A character pointer pointing to NULL
 * is returned on successful completion.  On error, a -1 is returned.
 * This really is a subroutine.  Return value is inconsistent.
 */  

getstr(int prt, char *buf, char brk, char ignr, int nonl)
{
	register char *b, c, prevc;

	prevc = 0;
	for(b=buf; c=peekc(); prevc=c) {
		if(c == '\n') {
			if(prevc == '\\' && (!flag3))
				*(b-1) = getch();
			else if(prevc == '\\' && flag3) {
				*b++ = getch();
			}
			else if(nonl) break;
			else return(*b='\0');
		}
		else {
			getch();
			if(c == brk) {
				if(prevc == '\\') *(b-1) = c;
				else return(*b='\0');
			}
			else if(b != buf || c != ignr) *b++ = c;
		}
	}
	return(err(prt,MSGSTR(SYNTAX,"syntax")));
}


/*
 * NAME: err
 *
 * FUNCTION: Print out an error message if prompting is turned on.
 *
 * DATA STRUCTURES: Clears global flags and pops the current working
 * file.
 *
 * RETURN VALUE DESCRIPTION:  returns -1.
 */

err(int prt, char *msg)
{
	if(prt) (prompt? printf("%s\n",msg): printf("?\n"));
	if(infildes != STDIN_FILENO) {
		infildes = pop(fstack);
		charbuf = '\n';
		peeked = 0;
		flag3 = 0;
		flag2 = 0;
		flag = 0;
		gflag = 0;
	}
	return(-1);
}


/*
 * NAME: getch
 *
 * FUNCTION: Read in the next line in from the currently active file
 * 		descriptor.  Count \\ as a line continuation.
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to a character buffer read
 * in from the latest file descriptor.
 */

getch(void)
{
	if(!peeked) {
		while((!(infildes == oldfd && flag)) && (!flag1) &&
			(!readc(infildes,&charbuf))) {
				if(infildes == 100 && (!flag))
					flag1 = 1;
				if((infildes=pop(fstack)) == -1)
					exit(1);
				if((!flag1) && infildes == STDIN_FILENO &&
					flag3 && prompt)
						printf("*");
		}
		if(infildes == oldfd && flag) flag2 = 0;
		flag1 = 0;
	}
	else peeked = 0;
	return(charbuf);
}


/*
 * NAME: readc
 *
 * FUNCTION:  Get next character from the currently opened file.
 *
 * RETURN VALUE DESCRIPTION: Returns 0 upon successful completion and
 *				1 if no characters available.
 */

readc(int f, char *c)
{
	if(f == 100) {
		if(!(*c = *intptr++)) {
			intptr--;
			charbuf = '\n';
			return(0);
		}
	}
	else if(read(f,c,1) != 1) {
		close(f);
		charbuf = '\n';
		return(0);
	}
	return(1);
}


/*
 * NAME: percent
 *
 * FUNCTION: Take an inputted command line and expand any variables
 *	of the form "%[0-9]".
 *
 * RETURN VALUE DESCRIPTION: 0 successful, 1 failed.
 */

percent (char *line)
{
	register char *olp, *lp;
	register len;
	char num;

	internal[0] = '\0';
	for (olp = line;lp = strchr (olp, '%'); olp = lp) {
		if (strlen (lp) == 1)
			return(err (1, MSGSTR(NODIGIT,
				"No digit specified\n")));
		*lp++ = '\0';
		if ((len = strlen (olp)) >= 1 && olp[len - 1] == '\\') {
			olp[len - 1] = '\0';
			strcat (internal, olp);
			strcat (internal, "%");
			continue;
		}
		strcat (internal, olp);
		num = *lp++;
		if (num < '0' || num > '9')
			return (err (1, MSGSTR(BADDIGIT,
				"bad digit specified")));
		strcat (internal, varray[num - '0']);
	}
	strcat (internal, olp);
	return (0);
}


/*
 * NAME: newfile
 *
 * FUNCTION: open new file.
 *
 * DATA STRUCTURES:	fstack is updated and the top of stack gets
 *			the old line.
 *			If the file parameter is NULL, then initialize
 *			everything otherwise open the next file.
 *
 * RETURN VALUE DESCRIPTION:  If we can't open the new file, -1 is returned.
 */

newfile(int prt, char *f)
{
	register int fd;

	if(!*f) {
		if(flag != 0) {
			oldfd = infildes;
			intptr = comdlist;
		}
		else intptr = internal;
		fd = 100;
	}
	else if((fd=open(f,O_RDONLY)) < 0) {
		sprintf(strtmp, MSGSTR(CANOTOPEN, "cannot open %s"), f);
		return err(prt, strtmp);
	}

	push(fstack,infildes);
	if(flag4) oldfd = fd;
	infildes = fd;
	return(peeked=0);
}


/*
 * NAME: push
 *
 * FUNCTION: Increment number of open files, store value of the old one.
 */

push(int s[], int d)
{
	s[++s[0]] = d;
}


/*
 * NAME: pop
 *
 * FUNCTION: Decrement the number of open files.
 */

pop(int s[])
{
	return(s[s[0]--]);
}


peekc(void)
{
	register char c;

	c = getch();
	peeked = 1;

	return(c);
}


/*
 * NAME: eat
 *
 * FUNCTION: Read in a line into a buffer.
 */

eat(void)
{
	if(charbuf != '\n') while(getch() != '\n');
	peeked = 0;
}


more(void)
{
	if(getch() != '\n') return(err(1,MSGSTR(SYNTAX,"syntax")));
	return(0);
}

/*
 * NAME: out
 *
 * FUNCTION: Write out the buffer (ln) to outfildes.
 */

out(char *ln)
{
	register char *rp, *wp, prev;
	int lim, oldtrunc;
	int nbytes;
	
	if (MB_CUR_MAX > 1) /* MBCS */
	 	mbout(ln); 
	else { /* SBCS */
		oldtrunc = trunc; 

		if(crunch > 0) {
			ln = untab(ln);
			rp = wp = ln - 1;
			prev = ' ';
			while(*++rp) {
				if(prev != ' ' || *rp != ' ') *++wp = *rp;
				prev = *rp;
			}
			*++wp = '\n';
			lim = wp - ln;
			*++wp = '\0';

			if(*ln == '\n') 
				return;
		}
		else {
		/*
		 * Step through the line passed in up to the '\n' or
		 * until LINE_LENGTH is reached.
		 */
			ln[lim=size(ln)-1] = '\n';
			wp = ln;
			while (*wp != '\n')
				wp++;
		}
		if(lim > trunc) ln[lim=trunc] = '\n';
	/*
	 * Write out current line up to the '\n' character.
	 */
		nbytes = write(outfildes,ln,lim+1); 
		outcnt += (double) nbytes;
		trunc = oldtrunc;
	} /* sbcs */
	return;
}


/*
 * NAME: untab
 *
 * FUNCTION: change all tabs in a line to a series of spaces.
 */

char *
untab(char *l)
{
	static char line[LINE_LENGTH];
	register char *q, *s;

	s = l;
	q = line;
	do {
		if(*s == '\t')
			do *q++ = ' '; while((q-line)%8);
		else *q++ = *s;
	} while(*s++);
	return(line);
}


/*
 *	Function to convert ascii string to integer.  Converts
 *	positive numbers only.  Returns -1 if non-numeric
 *	character encountered.
 */

patoi(char *b)
{
	register int i;
	register char *a;

	a = b;
	i = 0;
	while(*a >= '0' && *a <= '9')
		i = 10*i + *a++ - '0';

	if(*a) return(-1);
	return(i);
}


/*
 * NAME: size
 *
 * FUNCTION: Returns size (counting null byte) of arg string.
 */

size(char *s)
{
	register int i;

	i = 0;
	while(s[i++]);

	return(i);
}


/*
	Compares 2 strings.  Returns 1 if equal, 0 if not.
*/

equal(char *a, char *b)
{
	register char *x, *y;

	x = a;
	y = b;
	while (*x == *y++)
		if (*x++ == 0) return(1);
	return(0);
}


/*
 * NAME:mbout
 *
 * FUNCTION: Write out the buffer (ln) to outfildes in multibyte characters.
 */

mbout(char *ln)
{
	register char *rp, *wp, prev;
	int lim, oldtrunc;
	register charcount=0;
	int nbytes;
	int mbcnt;

	oldtrunc = trunc; /* oldtrunc is real value to truncate in mb chars */
			  /* trunc now is number of bytes to truncate */
	if(crunch > 0) { /* in compressed mode of xc sub command */
		ln = untab(ln);
		rp = wp = ln - 1;
		prev = ' ';

		while(*++rp) { /* figure out number of bytes to output */
			mbcnt = mblen(rp, MB_CUR_MAX);
			if (mbcnt == -1)
				mbcnt = 1;		
			if(prev != ' ' || *rp != ' ') 
				strncpy(++wp, rp, mbcnt);
			++charcount;
			if ( (mbcnt > 1) && charcount <= oldtrunc)
				trunc+=(mbcnt - 1);
			prev = *rp;
			rp+=(mbcnt - 1);
			wp+=(mbcnt - 1);
		}
		*++wp = '\n';
		lim = wp - ln;
		*++wp = '\0';

		if(*ln == '\n') 
		{
			return;
		}
	}
	else {
		/*
		 * Step through the line passed in up to the '\n' or
		 * until LINE_LENGTH is reached.
		 */
		ln[lim=size(ln)-1] = '\n';
		wp = ln;
		while (*wp != '\n') {
			++charcount;
			mbcnt = mblen(wp, MB_CUR_MAX);
			if (mbcnt == -1)
				mbcnt = 1;
			if ( (mbcnt > 1) && charcount <= oldtrunc )
				trunc+=mbcnt-1;
			wp+=mbcnt;
		}
	}
	if (lim > trunc) 
		ln[lim=trunc] = '\n';
	/*
	 * Write out current line up to the '\n' character.
	 */
	nbytes = write(outfildes,ln,lim+1);
	outcnt += (double) nbytes;
	trunc = oldtrunc;
	return;
}

