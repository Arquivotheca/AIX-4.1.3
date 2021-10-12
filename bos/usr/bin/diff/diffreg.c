static char sccsid[] = "@(#)91  1.30  src/bos/usr/bin/diff/diffreg.c, cmdfiles, bos412, 9446C 11/14/94 16:52:38";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 18, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */
#define _ILS_MACROS
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <nl_types.h>
#include "diff.h"

#define NUMALLOC 512
#define isblank(c)	is_wctype(c, _ISBLANK)
#define iswblank(wc)	is_wctype(wc, _ISBLANK)
extern nl_catd catd;
extern int mbcodeset;  /* 0=current locale SBCS, 1=current locale MBCS */
extern char ignore_mb[MB_LEN_MAX];

/*
 * Output format options
 */
extern int	opt;
extern int	tflag;			/* expand tabs on output */

/*
 * Algorithm related options
 */
extern int	hflag;			/* -h, use halfhearted DIFFH */
extern int	bflag;			/* ignore blanks in comparisons */
extern int	wflag;			/*totally ignore blanks in comparisons*/
extern int	iflag;			/* ignore case in comparisons */

/*
 * Variables for -I D_IFDEF option.
 */
extern int	wantelses;		/* -E */
extern char	*def1;			/* String for -1 */
extern char	*def2;			/* String for -2 */
static char		*endname;		/* What we will print on next #endif */
static int		indef;

/*
 * Variables for -c context option.
 */
extern int	context;		/* lines of context to be printed */

/*
 * State for exit status.
 */
extern int	status;
static int		anychange;
extern char	*tempfile;	/* used when comparing against std input */

/*
 * Variables for diffdir.
 */
extern char	**diffargv;	/* option list to pass to recursive diffs */

/*
 * Input file names.
 * With diffdir, file1 and file2 are allocated BUFSIZ space,
 * and padded with a '/', and then efile0 and efile1 point after
 * the '/'.
 */
extern char	*file1, *file2, *efile1, *efile2;
extern struct	stat stb1, stb2;

/*
 * This is allocated early, and used
 * to reset the free storage pointer to effect space compaction.
 */
extern char	*dummy;

extern char	*talloc(), *ralloc(), *mktemp(), *rindex();
extern char	*savestr(); 
static char		*splice();
static char		*copytemp();
extern int	done(void);
static long		readhash();

/*
 * diff - compare two files.
 */

/*
 *	Uses an algorithm due to Harold Stone, which finds
 *	a pair of longest identical subsequences in the two
 *	files.
 *
 *	The major goal is to generate the match vector J.
 *	J[i] is the index of the line in file1 corresponding
 *	to line i file0. J[i] = 0 if there is no
 *	such line in file1.
 *
 *	Lines are hashed so as to work in core. All potential
 *	matches are located by sorting the lines of each file
 *	on the hash (called ``value''). In particular, this
 *	collects the equivalence classes in file1 together.
 *	Subroutine equiv replaces the value of each line in
 *	file0 by the index of the first element of its 
 *	matching equivalence in (the reordered) file1.
 *	To save space equiv squeezes file1 into a single
 *	array member in which the equivalence classes
 *	are simply concatenated, except that their first
 *	members are flagged by changing sign.
 *
 *	Next the indices that point into member are unsorted into
 *	array class according to the original order of file0.
 *
 *	The cleverness lies in routine stone. This marches
 *	through the lines of file0, developing a vector klist
 *	of "k-candidates". At step i a k-candidate is a matched
 *	pair of lines x,y (x in file0 y in file1) such that
 *	there is a common subsequence of length k
 *	between the first i lines of file0 and the first y 
 *	lines of file1, but there is no such subsequence for
 *	any smaller y. x is the earliest possible mate to y
 *	that occurs in such a subsequence.
 *
 *	Whenever any of the members of the equivalence class of
 *	lines in file1 matable to a line in file0 has serial number 
 *	less than the y of some k-candidate, that k-candidate 
 *	with the smallest such y is replaced. The new 
 *	k-candidate is chained (via pred) to the current
 *	k-1 candidate so that the actual subsequence can
 *	be recovered. When a member has serial number greater
 *	that the y of all k-candidates, the klist is extended.
 *	At the end, the longest subsequence is pulled out
 *	and placed in the array J by unravel
 *
 *	With J in hand, the matches there recorded are
 *	check'ed against reality to assure that no spurious
 *	matches have crept in due to hashing. If they have,
 *	they are broken, and "jackpot" is recorded--a harmless
 *	matter except that a true match for a spuriously
 *	mated line may now be unnecessarily reported as a change.
 *
 *	Much of the complexity of the program comes simply
 *	from trying to minimize core utilization and
 *	maximize the range of doable problems by dynamically
 *	allocating what is needed and reusing what is not.
 *	The core requirements for problems larger than somewhat
 *	are (in words) 2*length(file0) + length(file1) +
 *	3*(number of k-candidates installed),  typically about
 *	6n words for files of length n. 
 */

extern char diffh[], diff[], pr[];
#define	prints(s)	fputs(s,stdout)

static FILE	*input[2];

static struct cand {
	int	x;
	int	y;
	int	pred;
} cand;
static struct line {
	long	serial;
	long	value;
} *file[2], line;
static int	len[2];
static struct	line *sfile[2];	/* shortened by pruning common prefix and suffix */
static int	slen[2];
static int	pref, suff;	/* length of prefix and suffix */
static int	*class;		/* will be overlaid on file[0] */
static int	*member;	/* will be overlaid on file[1] */
static int	*klist;		/* will be overlaid on file[0] after class */
static struct	cand *clist;	/* merely a free storage pot for candidates */
static int	calloced = 0;
static int	clen = 0;
static int	*J;		/* will be overlaid on class */
static long	*ixold;		/* will be overlaid on klist */
static long	*ixnew;		/* will be overlaid on file[1] */


/*
 * NAME: diffreg
 * FUCNTION: directs the diff between to files
 */
diffreg()
{
	register int i,j,k;
	int f1, f2;
	FILE *F1, *F2;
	char buf1[BUFSIZ], buf2[BUFSIZ];

	int mb_cur_max;
	mb_cur_max = MB_CUR_MAX;
	mbcodeset = (mb_cur_max > 1?1:0);

	if (hflag) {
		diffargv[0] = "diffh";
		execv(diffh, diffargv);
		fputs("diff: ",stderr);
		perror(diffh);
		status = 2;
		done();
	}
	dummy = malloc((size_t)1);


	if ((stb1.st_mode & S_IFMT) == S_IFDIR) {
		file1 = splice(file1, file2);
		if (stat(file1, &stb1) < 0) {
			fputs("diff: ",stderr);
			perror(file1);
			status = 2;
			done();
		}
	} else if ((stb2.st_mode & S_IFMT) == S_IFDIR) {
		file2 = splice(file2, file1);
		if (stat(file2, &stb2) < 0) {
			fputs("diff: ",stderr);
			perror(file2);
			status = 2;
			done();
		}
	} else if (!strcmp(file1, "-")) {
		if (!strcmp(file2, "-")) {
			fprintf(stderr, MSGSTR(DSTDINS,
				"diff: can't specify - -\n"));
			status = 2;
			done();
		}
		file1 = copytemp();
		if (stat(file1, &stb1) < 0) {
			fputs("diff: ",stderr);
			perror(file1);
			status = 2;
			done();
		}
	} else if (!strcmp(file2, "-")) {
		file2 = copytemp();
		if (stat(file2, &stb2) < 0) {
			fputs("diff: ",stderr);
			perror(file2);
			status = 2;
			done();
		}
	}
	if ((F1 = fopen(file1, "r")) == NULL) {
		fputs("diff: ",stderr);
		perror(file1);
		status = 2;
		done();
	}
	if ((F2 = fopen(file2, "r")) == NULL) {
		fputs("diff: ",stderr);
		perror(file2);
		fclose(F1);
		status = 2;
		done();
	}

	f1 =  fileno (F1);
	f2 =  fileno (F2);

	if (stb1.st_size != stb2.st_size)
		goto notsame;
	for (;;) {
		i = fread((void *)buf1, (size_t) 1, (size_t) BUFSIZ, F1);
		j = fread((void *)buf2, (size_t) 1, (size_t) BUFSIZ, F2);
		if (i < 0 || j < 0 || i != j)
			goto notsame;
		if (i == 0 && j == 0) {
			fclose(F1);
			fclose(F2);
			status = 0;		/* files don't differ */
			goto same;
		}
		for (j = 0; j < i; j++)
			if (buf1[j] != buf2[j])
				goto notsame;
	}
notsame:
	/*
	 *	Files certainly differ at this point; set status accordingly
	 */
	status = 1;
/*	if (!ascii(f1) || !ascii(f2)) {
		printf(MSGSTR(DBINARY,"Binary files %s and %s differ\n")
			, file1, file2);
		fclose(F1);
		fclose(F2);
		done();
	} Commented this for 141843 */
/* The above part is from bsd 4.3 code */

	prepare(0, file1);
	prepare(1, file2);
	prune();
	sort(sfile[0],slen[0]);
	sort(sfile[1],slen[1]);

	member = (int *)file[1];
	equiv(sfile[0], slen[0], sfile[1], slen[1], member);
	member = (int *)ralloc((char *)member,(slen[1]+2)*sizeof(int));

	class = (int *)file[0];
	unsort(sfile[0], slen[0], class);
	class = (int *)ralloc((char *)class,(slen[0]+2)*sizeof(int));

	klist = (int *)talloc((slen[0]+2)*sizeof(int));
	clist = (struct cand *)talloc((calloced = NUMALLOC) * sizeof(cand));
	k = stone(class, slen[0], member, klist);
	free((void *)member);
	free((void *)class);

	J = (int *)talloc((len[0]+2)*sizeof(int));
	unravel(klist[k]);
	free((void *)clist);
	free((void *)klist);

	ixold = (long *)talloc((len[0]+2)*sizeof(long));
	ixnew = (long *)talloc((len[1]+2)*sizeof(long));
	check();


	output();
	status = anychange;
same:	
	if (opt == DI_CONTEXT && anychange == 0)
		printf(MSGSTR(ENODIF,"No differences encountered\n"));
	done();
}


/*
 * NAME: copytemp
 * FUNCTION: copy stdin to a temp file
 */
char *
copytemp()
{
	char buf[BUFSIZ];
	register int i, f;

	signal(SIGHUP,(void (*)(int))done);
	signal(SIGINT,(void (*)(int))done);
	signal(SIGPIPE,(void (*)(int))done);
	signal(SIGTERM,(void (*)(int))done);
	signal(SIGQUIT,(void (*)(int))done);
	if ( (tempfile = malloc( strlen("/tmp/dXXXXXX") + 10) ) == NULL) {
		fprintf(stderr,MSGSTR(EALLOTMP,
				"diff: Could not malloc space for tempfile\n"));
	done();
	}
	strcpy(tempfile, "/tmp/dXXXXXX");
	tempfile = mktemp(tempfile);
	f = creat(tempfile,0600);
	if (f < 0) {
		fputs("diff: ",stderr);
		perror(tempfile);
		status = 2;
		done();
	}
	while ((i = read(0,buf,BUFSIZ)) > 0)
		if (write(f,buf,i) != i) {
			fputs("diff: ",stderr);
			perror(tempfile);
			status = 2;
			done();
		}
	close(f);
	return (tempfile);
}

/*
 * NAME: splice
 * FUNCTION: add the name of the file at the end of the path dir
 */
char *
splice(dir, file)
	char *dir, *file;
{
	char *tail;
	char buf[BUFSIZ];

	if (!strcmp(file, "-")) {
		fprintf(stderr,MSGSTR(DSTDIN, 
		  "diff: can't specify - with other argument a  directory\n"));
		status = 2;
		done();
	}
	tail = rindex(file, '/');
	if (tail == 0)
		tail = file;
	else
		tail++;
	sprintf(buf, "%s/%s", dir, tail);
	return (savestr(buf));
}

/*
 * NAME:  prepare
 * FUNCTION: prepare memory for the diff
 */
static
prepare(i, arg)
char *arg;
{
	register struct line *p;
	register j;
	register long h;
	int alloced = 0;
	if((input[i] = fopen(arg,"r")) == NULL){
		fputs("diff: ",stderr);
		perror(arg);
		status = 2;
		done();
	}
	p = (struct line *)talloc((alloced = NUMALLOC) * sizeof(line));
	for(j=0; h=readhash(input[i],arg);) {
		if (++j+3 > alloced)
			p = (struct line *)ralloc((char *)p,(alloced += NUMALLOC)*sizeof(line));
		p[j].value = h;
	}
	len[i] = j;
	file[i] = p;
	fclose(input[i]);
}

static
prune()
{
	register i,j;
	for(pref=0;pref<len[0]&&pref<len[1]&&
		file[0][pref+1].value==file[1][pref+1].value;
		pref++ ) ;
	for(suff=0;suff<len[0]-pref&&suff<len[1]-pref&&
		file[0][len[0]-suff].value==file[1][len[1]-suff].value;
		suff++) ;
	for(j=0;j<2;j++) {
		sfile[j] = file[j]+pref;
		slen[j] = len[j]-pref-suff;
		for(i=0;i<=slen[j];i++)
			sfile[j][i].serial = i;
	}
}

static
equiv(a,n,b,m,c)
struct line *a, *b;
int *c;
{
	register int i, j;
	i = j = 1;
	while(i<=n && j<=m) {
		if(a[i].value <b[j].value)
			a[i++].value = 0;
		else if(a[i].value == b[j].value)
			a[i++].value = j;
		else
			j++;
	}
	while(i <= n)
		a[i++].value = 0;
	b[m+1].value = 0;
	j = 0;
	while(++j <= m) {
		c[j] = (int)-b[j].serial;
		while(b[j+1].value == b[j].value) {
			j++;
			c[j] = (int)b[j].serial;
		}
	}
	c[j] = -1;
}

static
stone(a,n,b,c)
int *a;
int *b;
int *c;
{
	register int i, k,y;
	int j, l;
	int oldc, tc;
	int oldl;
	k = 0;
	c[0] = newcand(0,0,0);
	for(i=1; i<=n; i++) {
		j = a[i];
		if(j==0)
			continue;
		y = -b[j];
		oldl = 0;
		oldc = c[0];
		do {
			if(y <= clist[oldc].y)
				continue;
			l = search(c, k, y);
			if(l!=oldl+1)
				oldc = c[l-1];
			if(l<=k) {
				if(clist[c[l]].y <= y)
					continue;
				tc = c[l];
				c[l] = newcand(i,y,oldc);
				oldc = tc;
				oldl = l;
			} else {
				c[l] = newcand(i,y,oldc);
				k++;
				break;
			}
		} while((y=b[++j]) > 0);
	}
	return(k);
}

static
newcand(x,y,pred)
{
	register struct cand *q;
        if(clen++ >= calloced)
          clist=(struct cand *)
                 ralloc((char *)clist,(calloced += NUMALLOC) *sizeof(cand));
	q = clist + clen -1;
	q->x = x;
	q->y = y;
	q->pred = pred;
	return(clen-1);
}

static
search(c, k, y)
int *c;
{
	register int i, j, l;
	int t;
	if(clist[c[k]].y<y)	/*quick look for typical case*/
		return(k+1);
	i = 0;
	j = k+1;
	while((l=(i+j)/2) > i) {
		t = clist[c[l]].y;
		if(t > y)
			j = l;
		else if(t < y)
			i = l;
		else
			return(l);
	}
	return(l+1);
}

static
unravel(p)
{
	register int i;
	register struct cand *q;
	for(i=0; i<=len[0]; i++)
		J[i] =	i<=pref ? i:
			i>len[0]-suff ? i+len[1]-len[0]:
			0;
	for(q=clist+p;q->y!=0;q=clist+q->pred)
		J[q->x+pref] = q->y+pref;
}

/* check does double duty:
1.  ferret out any fortuitous correspondences due
to confounding by hashing (which result in "jackpot")
2.  collect random access indexes to the two files */

static
check()
{
	register int i, j;
	int jackpot;
	long ctold, ctnew;
	wchar_t wc,wd;
	int wcl,wdl;
	register int c,d;

	if ((input[0] = fopen(file1,"r")) == NULL) {
		perror(file1);
		status = 2;
		done();
	}
	if ((input[1] = fopen(file2,"r")) == NULL) {
		perror(file2);
		status = 2;
		done();
	}
	j = 1;
	ixold[0] = ixnew[0] = 0;
	jackpot = 0;
	ctold = ctnew = 0;
	for(i=1;i<=len[0];i++) {
		if(J[i]==0) {
			ixold[i] = ctold += skipline(0);
			continue;
		}
		while(j<J[i]) {
			ixnew[j] = ctnew += skipline(1);
			j++;
		}
		if(bflag || wflag || iflag) { /* Filter input as it is processed */
			for(;;) {
			if (mbcodeset==1) {
				/* Find ctold = cumulative character count into file
				 * of last character of current line: MBCS version.
				 */
				wc = getwc(input[0]);
				if (wc == (wchar_t) WEOF) break;
				wd = getwc(input[1]);
				if (wd == (wchar_t) WEOF) break;
				wcl = wctomb(ignore_mb,wc);
				wdl = wctomb(ignore_mb,wd);
				ctold += wcl;
				ctnew += wdl;
				if(bflag && (iswblank(wc) || wc==(wchar_t)'\n') && (iswblank(wd) || wd==(wchar_t)'\n')) {
					do {
						if(wc==(wchar_t)'\n')
							break;
						wc = getwc(input[0]);
						if (wc == (wchar_t) WEOF) break;
						ctold += wctomb(ignore_mb,wc);
					} while(iswblank(wc));
					do {
						if(wd==(wchar_t)'\n')
							break;
						wd = getwc(input[1]);
						if (wd == (wchar_t) WEOF) break;
						ctnew += wctomb(ignore_mb,wd);
					} while(iswblank(wd));
				} else if (wflag) {
					while(iswblank(wc) && wc!=(wchar_t)'\n' ) {
						wc = getwc(input[0]);
						if (wc == (wchar_t) WEOF) break;
						ctold += wctomb(ignore_mb,wc);
					}
					while(iswblank(wd) && wd!=(wchar_t)'\n' ) {
						wd = getwc(input[1]);
						if (wd == (wchar_t) WEOF) break;
						ctnew += wctomb(ignore_mb,wd);
					}
				}
				if (cmpwchar(wc,wd)) {
					jackpot++;
					J[i] = 0;
					if(wc!=(wchar_t)'\n')
						ctold += skipline(0);
					if(wd!=(wchar_t)'\n')
						ctnew += skipline(1);
					break;
				}
				if(wc==(wchar_t)'\n')
				{
	/*				ctold++;
					ctnew++;		*/	
					break;
				}
			} else {
				/* Find ctold = cumulative character count into file
				 * of last character of current line: SBCS version.
				 */
				c = getc(input[0]);
				if (c == EOF) break;
				d = getc(input[1]);
				if (d == EOF) break;
				ctold++;
				ctnew++;
				if(bflag && (isblank(c) || c=='\n') && (isblank(d) || d=='\n')) {
					do {
						if(c == EOF || c=='\n')
							break;
						ctold++;
					} while(c=getc(input[0]),isblank(c));
					do {
						if(d == EOF || d=='\n')
							break;
						ctnew++;
					} while(d=getc(input[1]),isblank(d));
				} else if (wflag) {
					while(isblank(c) && c!='\n' ) {
						c=getc(input[0]);
						if (c == EOF) break;
						ctold++;
					}
					while( isblank(d) && d!='\n' ) {
						d=getc(input[1]);
						if (d == EOF) break;
						ctnew++;
					}
				}
				if (cmpchar(c,d)) {
					jackpot++;
					J[i] = 0;
					if(c!='\n')
						ctold += skipline(0);
					if(d!='\n')
						ctnew += skipline(1);
					break;
				}
				if(c=='\n')
				{
	/*				ctold++;
					ctnew++;		*/	
					break;
				}
			} /* if (mbcodeset) */
			} /* for (;;) */
		} else {	/* Take input asis, no filter as processed */
			for(;;) {
			if (mbcodeset==1) { /* Do ????? for MBCS characters */
				if((wc=getwc(input[0])) != (wd=getwc(input[1]))) {
					if (wd == (wchar_t) WEOF || wc == (wchar_t) WEOF ) break;
					wcl = wctomb(ignore_mb,wc);
					wdl = wctomb(ignore_mb,wd);
					ctold += wcl;
					ctnew += wdl;
					/* jackpot++; */
					J[i] = 0;
					if(wc!=(wchar_t)'\n')
						ctold += skipline(0);
					if(wd!=(wchar_t)'\n')
						ctnew += skipline(1);
					break;
				} else {
						wcl = wctomb(ignore_mb,wc);
						wdl = wctomb(ignore_mb,wd);
						ctold += wcl;
						ctnew += wdl;
				}
				if (wd == (wchar_t) WEOF || wc == (wchar_t) WEOF) break;
				if(wc==(wchar_t)'\n')
				{
					break;
				}
			} else { /* Do ????? for SBCS characters */ 
				if((c=getc(input[0])) != (d=getc(input[1]))) {
					if (d == EOF || c == EOF ) break;
					ctold++;
					ctnew++;
					/* jackpot++; */
					J[i] = 0;
					if(c!='\n')
						ctold += skipline(0);
					if(d!='\n')
						ctnew += skipline(1);
					break;
				} else {
						ctold++;
						ctnew++;
				}
				if (d == EOF || c == EOF ) break;
				if(c=='\n')
				{
					break;
				}
			} /* if (mbcodeset) */
			} /* for (;;) */
		}
		ixold[i] = ctold;
		ixnew[j] = ctnew;
		j++;
	}
	for(;j<=len[1];j++) {
		ixnew[j] = ctnew += skipline(1);
	}
	fclose(input[0]);
	fclose(input[1]);
}

static
sort(a,n)	/*shellsort CACM 201*/
struct line *a;
{
	struct line w;
	register int j,m;
	struct line *ai;
	register struct line *aim;
	int k;

	if (n == 0)
		return;

	for(j=1;j<=n;j*= 2)
		m = 2*j - 1;
	for(m/=2;m!=0;m/=2) {
		k = n-m;
		for(j=1;j<=k;j++) {
			for(ai = &a[j]; ai > a; ai -= m) {
				aim = &ai[m];
				if(aim < ai)
					break;	/*wraparound*/
				if(aim->value > ai[0].value ||
				   aim->value == ai[0].value &&
				   aim->serial > ai[0].serial)
					break;
				w.value = ai[0].value;
				ai[0].value = aim->value;
				aim->value = w.value;
				w.serial = ai[0].serial;
				ai[0].serial = aim->serial;
				aim->serial = w.serial;
			}
		}
	}
}

static
unsort(f, l, b)
struct line *f;
int *b;
{
	register int *a;
	register int i;
	a = (int *)talloc((l+1)*sizeof(int));
	for(i=1;i<=l;i++)
		a[f[i].serial] = f[i].value;
	for(i=1;i<=l;i++)
		b[i] = a[i];
	free((void *)a);
}

static
skipline(f)
{
	register int i,j,c;
	wint_t wc;
	
	j=0;
	if (mbcodeset==1) {
		for(i=0;(wc=getwc(input[f]))!=(wint_t)'\n';i+=wctomb(ignore_mb,(wchar_t)wc))
			if (wc == WEOF )
				return(++i);
	} else {
		for(i=0;(c=getc(input[f]))!='\n';i++)
			if (c == EOF)
				return(++i);
	}
	return(++i);
}

static
output()
{
	int m;
	register int i0, i1, j1;
	wint_t wi0;
	int j0;

	input[0] = fopen(file1,"r");
	input[1] = fopen(file2,"r");
	m = len[0];
	J[0] = 0;
	J[m+1] = len[1]+1;
	if(opt!=DI_EDIT) for(i0=1;i0<=m;i0=i1+1) {
		while(i0<=m&&J[i0]==J[i0-1]+1) i0++;
		j0 = J[i0-1]+1;
		i1 = i0-1;
		while(i1<m&&J[i1+1]==0) i1++;
		j1 = J[i1+1]-1;
		J[i1] = j1;
		change(i0,i1,j0,j1);
	} else for(i0=m;i0>=1;i0=i1-1) {
		while(i0>=1&&J[i0]==J[i0+1]-1&&J[i0]!=0) i0--;
		j0 = J[i0+1]-1;
		i1 = i0+1;
		while(i1>1&&J[i1-1]==0) i1--;
		j1 = J[i1-1]+1;
		J[i1] = j1;
		change(i1,i0,j1,j0);
	}
	if(m==0)
		change(1,0,1,len[1]);
	if (opt==DI_IFDEF) {
		for (;;) {
		if (mbcodeset==1) { /* Do whatever it does for MBCS characters */
			wi0 = getwc(input[0]);
			if (wi0 == WEOF)
				return;
			putwchar(wi0);
		} else {	 /* Do whatever it does for SBCS characters */
			i0 = getc(input[0]);
			if (i0 < 0)
				return;
			putchar(i0);
		} /* if (mbcodeset) */
		} /* for (;;) */
	}
	if (anychange && opt == DI_CONTEXT)
		dump_context_vec();
}

/*
 * The following struct is used to record change information when
 * doing a "context" diff.  (see routine "change" to understand the
 * highly mneumonic field names)
 */
struct context_vec {
	int	a;	/* start line in old file */
	int	b;	/* end line in old file */
	int	c;	/* start line in new file */
	int	d;	/* end line in new file */
};

static struct	context_vec	*context_vec_start,
			*context_vec_end,
			*context_vec_ptr;

#define	MAX_CONTEXT	128

/* indicate that there is a difference between lines a and b of the from file
   to get to lines c to d of the to file.
   If a is greater then b then there are no lines in the from file involved
   and this means that there were lines appended (beginning at b).
   If c is greater than d then there are lines missing from the to file.
*/
static
change(a,b,c,d)
{
	int ch;
	struct stat stbuf;

	if (opt != DI_IFDEF && a>b && c>d)
		return;
	if (anychange == 0) {
		anychange = 1;
		if(opt == DI_CONTEXT) {
			printf("*** %s	", file1);
			stat(file1, &stbuf);
			printf("%s--- %s	",
			    ctime(&stbuf.st_mtime), file2);
			stat(file2, &stbuf);
			fputs(ctime(&stbuf.st_mtime),stdout);

			context_vec_start = (struct context_vec *) 
						malloc((size_t)(MAX_CONTEXT *
						   sizeof(struct context_vec)));
			context_vec_end = context_vec_start + MAX_CONTEXT;
			context_vec_ptr = context_vec_start - 1;
		}
	}
	if (a <= b && c <= d)
		ch = 'c';
	else
		ch = (a <= b) ? 'd' : 'a';
	if(opt == DI_CONTEXT) {
		/*
		 * if this new change is within 'context' lines of
		 * the previous change, just add it to the change
		 * record.  If the record is full or if this
		 * change is more than 'context' lines from the previous
		 * change, dump the record, reset it & add the new change.
		 */
		if ( context_vec_ptr >= context_vec_end ||
		     ( context_vec_ptr >= context_vec_start &&
		       a > (context_vec_ptr->b + 2*context) &&
		       c > (context_vec_ptr->d + 2*context) ) )
			dump_context_vec();

		context_vec_ptr++;
		context_vec_ptr->a = a;
		context_vec_ptr->b = b;
		context_vec_ptr->c = c;
		context_vec_ptr->d = d;
		return;
	}
	switch (opt) {

	case DI_NORMAL:
	case DI_EDIT:
		range(a,b,",");
		if (mbcodeset==1) { /* Do whatever it does for MBCS characters */
			putwchar(a>b?L'a':c>d?L'd':L'c');
			if(opt==DI_NORMAL)
				range(c,d,",");
			putwchar(L'\n');
		} else {         /* Do whatever it does for SBCS characters */
			putchar(a>b?'a':c>d?'d':'c');
			if(opt==DI_NORMAL)
				range(c,d,",");
			putchar('\n');
		} /* if(mbcodeset) */
		break;
	case DI_REVERSE:
		if (mbcodeset==1) { /* Do whatever it does for MBCS characters */
			putwchar(a>b?L'a':c>d?L'd':L'c');
			range(a,b," ");
			putwchar(L'\n');
		} else {	 /* Do whatever it does for SBCS characters */
			putchar(a>b?'a':c>d?'d':'c');
			range(a,b," ");
			putchar('\n');
		} /* if(mbcodeset) */
		break;
        case DI_NREVERSE:
                if (a>b)
                        printf("a%d %d\n",b,d-c+1);
                else {
                        printf("d%d %d\n",a,b-a+1);
                        if (!(c>d))
                           /* add changed lines */
                           printf("a%d %d\n",b, d-c+1);
                }
                break;
	}
	if(opt == DI_NORMAL || opt == DI_IFDEF) {
		fetch(ixold,a,b,input[0],"< ", 1);
		if(a<=b&&c<=d && opt == DI_NORMAL)
			prints("---\n");
	}
	fetch(ixnew,c,d,input[1],opt==DI_NORMAL?"> ":"", 0);
	if ((opt ==DI_EDIT || opt == DI_REVERSE) && c<=d)
		prints(".\n");
	if (indef) {
		/* PTM 35116: Modified print statement to meet ANSI C */
		fprintf(stdout, "#endif /* %s */ \n", endname);
		indef = 0;
	}
}

static
range(a,b,separator)
char *separator;
{
	printf("%d", a>b?b:a);
	if(a<b) {
		printf("%s%d", separator, b);
	}
}


static
fetch(f,a,b,lb,s,oldfile)
long *f;
FILE *lb;
char *s;
{
	register int i, j;
	register int c;
	wchar_t wc;
	register int col;
	register int nc;
	int oneflag = (*def1!='\0') != (*def2!='\0');

	/*
	 * When doing ifdef's, copy down to current line
	 * if this is the first file, so that stuff makes it to output.
	 */
	if (opt == DI_IFDEF && oldfile){
		long curpos = ftell(lb);
		/* print through if append (a>b), else to (nb: 0 vs 1 orig) */
		nc = f[a>b? b : a-1 ] - curpos;
		for (i = 0; i < nc;    )
		if (mbcodeset==1) {
			wc = getwc(lb);
			if (wc == (wchar_t) WEOF) break;
			i += wctomb(ignore_mb,wc);
			putwchar(wc);
		} else {
			c=getc(lb);
			if (c == EOF) break;
			putchar(c);
			i++;
		}
	}
	if (a > b)
		return;
	if (opt == DI_IFDEF) {
		if (indef) {
			/* PTM 35116: Modified print statement to meet ANSI C */
			fprintf(stdout,"#else /* %s%s */ \n", oneflag && oldfile==1 ? "!" : "", def2);
		}
		else {
			if (oneflag) {
				/* There was only one ifdef given */
				endname = def2;
				if (oldfile)
					fprintf(stdout,"#ifndef %s\n",
					    endname);
				else
					fprintf(stdout, "#ifdef %s\n", 
						endname);
			}
			else {
				endname = oldfile ? def1 : def2;
				fprintf(stdout,"#ifdef %s\n",
					endname);
			}
		}
		indef = 1+oldfile;
	}

	for(i=a;i<=b;i++) {
		fseek(lb,f[i-1],0);
		nc = f[i]-f[i-1];
		if (opt != DI_IFDEF)
			prints(s);
		col = 0;
		for(j=0;j<nc;   ) {
		if (mbcodeset==1) {
			wc = getwc(lb);
			if (wc == (wchar_t) WEOF) {
				putwchar(L'\n');
				break;
			}
			j += wctomb(ignore_mb,wc); 
			if (wc == L'\t' && tflag)
				do
					putwchar(L' ');
				while (++col & 7);      /* assuming tab is 8 spaces */
			else {
				register len;
				putwchar(wc);
				col += (len = wcwidth((wint_t)wc)) == -1 ? 1 : len;
			}
		} else {
			c = getc(lb);
			if (c == EOF) {
				putchar('\n');
				break;
			}
			if (c == '\t' && tflag)
				do
					putchar(' ');
				while (++col & 7);      /* assuming tab is 8 spaces */
			else {
				putchar(c);
				col++;
			}
			j++;
		} /* if (mbcodeset) */
		} /* for (j=0;...) */
	}

	if (indef && !wantelses) {
		/* PTM 35116: Modified print statement to meet ANSI C */
		fprintf(stdout,"#endif /* %s */ \n", endname);
		indef = 0;
	}
}

#define HALFLONG MB_LEN_MAX*8
#define low(x)	(x&((1L<<HALFLONG)-1))
#define high(x)	(x>>HALFLONG)

/*
 * hashing has the effect of
 * arranging line in 7-bit bytes and then
 * summing 1-s complement in 16-bit hunks 
 */

long
readhash(f,name)
register FILE *f;
char	*name;
{
	register long sum;
	register unsigned shift;
	register t;
	wchar_t wt;
	register space;
	int anychars=0;

	sum = 1;
	space = 0;
	if(!bflag && !wflag) {	 /* Hash with blanks and tabs significant */
		if (mbcodeset==1) { /* Hash MBCS with blanks and tabs significant */
		if(iflag)
		  {
			for(shift=0;(wt=getwc(f))!='\n';shift+=7) {
				if(wt==(wchar_t)-1) {
				    if (!anychars)
					return(0);
				    else
					  {
						fprintf(stderr, MSGSTR(NONEW, "diff: Not a text file, newline is missing at the end of the file %s.\n"), name);
						exit(2);
					  }
				}
				anychars++;
				sum += (long)tranchar(wt) << (shift%=HALFLONG);
			}
			/* Shift the newline too to take care of the case of a file */
			/* with newline and the other file having no newline at the */
			/* end of the file. Difference in the lines should be shown */
			/* as the contents are the same except for the newline      */

			anychars++;
			sum += (long)tranchar(wt) << (shift%=HALFLONG);
		  }
		else
		  {
			for(shift=0;(wt=getwc(f))!='\n';shift+=7) {
				if(wt==(wchar_t)-1){
				    if (!anychars)
					return(0);
				    else
					  {
						fprintf(stderr, MSGSTR(NONEW, "diff: Not a text file, newline is missing at the end of the file %s.\n"), name);
						exit(2);
					  }
				}
				sum += (long)wt << (shift%=HALFLONG);
				anychars++;
			}
			/* Shift the newline too to take care of the case of a file */
			/* with newline and the other file having no newline at the */
			/* end of the file. Difference in the lines should be shown */
			/* as the contents are the same except for the newline      */

			sum += (long)wt << (shift%=HALFLONG);
			anychars++;
		  }
		} else {	 /* Hash SBCS with blanks and tabs significant */
		if(iflag)
		  {
			for(shift=0;(t=getc(f))!='\n';shift+=7) {
				if(t==-1) {
				    if (!anychars)
					return(0);
				    else
					  {
						fprintf(stderr, MSGSTR(NONEW, "diff: Not a text file, newline is missing at the end of the file %s.\n"), name);
						exit(2);
					  }
				}
				sum += (long)tranchar((wchar_t)t) << (shift%=HALFLONG);
				anychars++;
			}
			/* Shift the newline too to take care of the case of a file */
			/* with newline and the other file having no newline at the */
			/* end of the file. Difference in the lines should be shown */
			/* as the contents are the same except for the newline      */

			sum += (long)tranchar((wchar_t)t) << (shift%=HALFLONG);
			anychars++;
		  }
		else
		  {
			for(shift=0;(t=getc(f))!='\n';shift+=7) {
				if(t==-1) {
				    if (!anychars)
					return(0);
				    else
					  {
						fprintf(stderr, MSGSTR(NONEW, "diff: Not a text file, newline is missing at the end of the file %s.\n"), name);
						exit(2);
					  }
				}
				sum += (long)t << (shift%=HALFLONG);
				anychars++;
			}
			/* Shift the newline too to take care of the case of a file */
			/* with newline and the other file having no newline at the */
			/* end of the file. Difference in the lines should be shown */
			/* as the contents are the same except for the newline      */

			sum += (long)t << (shift%=HALFLONG);
			anychars++;
		  }
		} /* if (mbcodeset) */
	} else {	 /* Hash ignoring blanks and tabs */
		for(shift=0;;) {
			if (mbcodeset==1)
				wt=getwc(f);
			else
/*
				wt=(wchar_t)getc(f);
*/
				wt=(unsigned int)getc(f);
			switch(wt) {
			case (wchar_t)-1:
				if (!anychars)
					return(0);
				else
				  {
					fprintf(stderr, MSGSTR(NONEW, "diff: Not a text file, newline is missing at the end of the file %s.\n"), name);
					exit(2);
				  }
			default:
				if(iswblank(wt))
					space++;
				else {
					if(space && !wflag) {
						shift += 7;
						space = 0;
					}
					sum += (long)tranchar(wt) << (shift%=HALFLONG);
					shift += 7;
				}
				anychars++;
				continue;
			case L'\n':
			/* Shift the newline too to take care of the case of a file */
			/* with newline and the other file having no newline at the */
			/* end of the file. Difference in the lines should be shown */
			/* as the contents are the same except for the newline      */

				sum += (long)tranchar(wt) << (shift%=HALFLONG);
				anychars++;
				break;
			}
			break;
		}
	}
	/* Check for valid hash value of zero and change it to a nonzero value
	 * so caller  prepare()  does not think EOF arrived early.
	 * (The line  "\t\ta=none\n"  hashes to zero here.) 
	 */
	if (sum == 0)
		sum =0x55555555;

	return(sum);
}

/* dump accumulated "context" diff changes */
static
dump_context_vec()
{
	register int	a, b, c, d;
	register char	ch;
	register struct	context_vec *cvp = context_vec_start;
	register int	lowa, upb, lowc, upd;
	register int	do_output;

	if ( cvp > context_vec_ptr )
		return;

	lowa = max(1, cvp->a - context);
	upb  = min(len[0], context_vec_ptr->b + context);
	lowc = max(1, cvp->c - context);
	upd  = min(len[1], context_vec_ptr->d + context);

	fputs("***************\n*** ",stdout);
	range(lowa,upb,",");
	fputs(" ****\n",stdout);

	/*
	 * output changes to the "old" file.  The first loop suppresses
	 * output if there were no changes to the "old" file (we'll see
	 * the "old" lines as context in the "new" list).
	 */
	do_output = 0;
	for ( ; cvp <= context_vec_ptr; cvp++)
		if (cvp->a <= cvp->b) {
			cvp = context_vec_start;
			do_output++;
			break;
		}
	
	if ( do_output ) {
		while (cvp <= context_vec_ptr) {
			a = cvp->a; b = cvp->b; c = cvp->c; d = cvp->d;

			if (a <= b && c <= d)
				ch = 'c';
			else
				ch = (a <= b) ? 'd' : 'a';

			if (ch == 'a')
				fetch(ixold,lowa,b,input[0],"  ");
			else {
				fetch(ixold,lowa,a-1,input[0],"  ");
				fetch(ixold,a,b,input[0],ch == 'c' ? "! " : "- ");
			}
			lowa = b + 1;
			cvp++;
		}
		fetch(ixold, b+1, upb, input[0], "  ");
	}

	/* output changes to the "new" file */
	fputs("--- ",stdout);
	range(lowc,upd,",");
	fputs(" ----\n",stdout);

	do_output = 0;
	for (cvp = context_vec_start; cvp <= context_vec_ptr; cvp++)
		if (cvp->c <= cvp->d) {
			cvp = context_vec_start;
			do_output++;
			break;
		}
	
	if (do_output) {
		while (cvp <= context_vec_ptr) {
			a = cvp->a; b = cvp->b; c = cvp->c; d = cvp->d;

			if (a <= b && c <= d)
				ch = 'c';
			else
				ch = (a <= b) ? 'd' : 'a';

			if (ch == 'd')
				fetch(ixnew,lowc,d,input[1],"  ");
			else {
				fetch(ixnew,lowc,c-1,input[1],"  ");
				fetch(ixnew,c,d,input[1],ch == 'c' ? "! " : "+ ");
			}
			lowc = d + 1;
			cvp++;
		}
		fetch(ixnew, d+1, upd, input[1], "  ");
	}

	context_vec_ptr = context_vec_start - 1;
}

static int 
cmpwchar(wc1,wc2)
wchar_t wc1,wc2;
{
	wchar_t wch1,wch2;
	wch1 = wc1;
	wch2 = wc2;
	if (bflag || wflag)
		if (iswblank(wc1) && iswblank(wc2))
			return(0);
	if (iflag) {
		if (iswupper(wc1) != 0)
			wch1 = tolower(wc1);
		if (iswupper(wc2) != 0)
			wch2 = tolower(wc2);
	}
	if (wch1 == wch2)
		return(0);
	else
		return(1);

}

static int 
cmpchar(c1,c2)
char c1,c2;
{
	char ch1,ch2;
	
	ch1 = c1;
	ch2 = c2;
	if (bflag || wflag)
		if (isblank(c1) && isblank(c2))
			return(0);
	if (iflag) {
		if (isupper(c1) != 0)
			ch1 = tolower(c1);
		if (isupper(c2) != 0)
			ch2 = tolower(c2);
	}
	if (ch1 == ch2)
		return(0);
	else
		return(1);
}	

static int 
tranchar(wc)
wchar_t wc;
{
	if (iflag)
		if (iswupper(wc) != 0)
			return(towlower(wc));
	return(wc);
}
