static char sccsid[] = "@(#)90 1.22  src/bos/usr/ccs/bin/m4/m4.c, cmdm4, bos41J, 9521B_all 5/24/95 15:52:18";
/*
 * COMPONENT_NAME: (CMDM4) Macroprocessor 
 *
 * FUNCTIONS: catchsig, chkbltin, copy, ctol, delexit, error, error2,
 *	      expand, fpath, getchr, getflags, initalloc, inpmatch,
 *	      install, isbltin, itochr, lnsync, lookup, match,
 *	      min, pbnbr, pbnum, pbstr, puttok, setfname, undiv,
 *	      xcalloc, xfopen
 *
 * ORIGINS: 3, 27
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
 */
#define		_ILS_MACROS
#include	<stdio.h>
#include	<signal.h>
#include 	<locale.h>
#include	"m4.h"
#define		MSGSTR(Num, Str) catgets(catd, MS_M4, Num, Str)
static nl_catd catd;

#define match(c,s)	(c==*s && (!s[1] || inpmatch(s+1)))

main(argc,argv)
char 	**argv;
{
	FILE	*fp;
	int	errflag = 0;
	register t;
	void catchsig();

	static char nametemp[] = "/tmp/m4aXXXXX";
	{
	static	sigs[] = {SIGHUP, SIGINT, SIGPIPE, 0};
	for (t=0; sigs[t]; ++t)
		if (signal(sigs[t], SIG_IGN) != SIG_IGN)
			signal(sigs[t],catchsig);
	}

	setlocale(LC_ALL, "");
	catd = catopen(MF_M4, NL_CAT_LOCALE);

	mbcurmax =  MB_CUR_MAX;		/* how many bytes make a character */
					/*         in the current code set?*/ 
					/* Maximum of all such values */

	tempfile = mktemp(nametemp);
	close(creat(tempfile,0));


	procnam = argv[0];
	getflags(&argc,&argv);
	initalloc();

	setfname("-");
	if (argc>0) {
		if (strcmp(argv[0],"-")) {
			while (argc--) {
				if ((fp = fopen(argv[0], "r")) != NULL)
					break;
				perror("m4");
				errflag = 1;
				if (*(++argv) == NULL)
					delexit(NOT_OK);
			}
			ifile[ifx] = fp;
			setfname(argv[0]);
		} else
			--argc;
		++argv;
	}

	for (;;) {
		token[0] = t = getchr();
		token[1] = EOS;

		if (t==EOF) {
			if (ifx > 0) {
				fclose(ifile[ifx]);
				ipflr = ipstk[--ifx];
				continue;
			}

			optind = 0;
			getflags(&argc,&argv);

			if (argc<=0)
				if (Wrapstr) {
					pbstr(Wrapstr);
					Wrapstr = NULL;
					continue;
				} else
					break;

			if (ifile[ifx]!=stdin)
				fclose(ifile[ifx]);

                        if (*argv[0]=='-') {
                                ifile[ifx] = stdin;
				--argc;
                        } else {
				while (argc--) {
					if ((fp = fopen(argv[0], "r")) != NULL)
						break;
					perror("m4");
					errflag = 1;
					if (*(++argv) == NULL)
						delexit(NOT_OK);
				}
				ifile[ifx] = fp;
			}
                        setfname(argv[0]);
                        ++argv;
                        continue;
		}

		if (isalpha(t)||t=='_') {
			register char	*tp = token+1;
			register	tlim = toksize;
			struct nlist	*macadd;  /* temp variable */

			while (*tp++ = getchr(),isalnum(*(tp-1))||(*(tp-1)=='_'))
				if (--tlim<=0)
				       error2(MTB,"more than %d bytes in word",
							toksize);

			putbak(*--tp);
			*tp = EOS;

			macadd = lookup(token);
			*Ap = (char *) macadd;
			if (macadd->def) {
				if ((char *) (++Ap) >= astklm) {
					--Ap;
					error2(MTI,astkof,stksize);
				}

				if (Cp++==NULL)
					Cp = callst;

				Cp->argp = Ap;
				*Ap++ = op;
				puttok(token);
				stkchr(EOS);
				t = getchr();
				putbak(t);

				if (t!='(')
					pbstr("()");
				else	/* try to fix arg count */
					*Ap++ = op;

				Cp->plev = 0;
			} else {
				puttok(token);
			}
		} else if (match(t,lquote)) {
			register	qlev = 1;

			for (;;) {
				token[0] = t = getchr();
				token[1] = EOS;

				if (match(t,rquote)) {
					if (--qlev > 0)
						puttok(token);
					else
						break;
				} else if (match(t,lquote)) {
					++qlev;
					puttok(token);
				} else {
					if (t==EOF)
						error(QEOF,"EOF in quote");

					putchr(t);
				}
			}
		} else if (match(t,lcom)) {
			puttok(token);

			for (;;) {
				token[0] = t = getchr();
				token[1] = EOS;

				if (match(t,rcom)) {
					puttok(token);
					break;
				} else {
					if (t==EOF)
						error(CEOF,"EOF in comment");
					putchr(t);
				}
			}
		} else if (Cp==NULL) {
			putchr(t);
		} else if (t=='(') {
			if (Cp->plev)
				stkchr(t);
			else {
				/* skip white before arg */
				while (t=getchr(),isspace(t))
					;

				putbak(t);
			}

			++Cp->plev;
		} else if (t==')') {
			--Cp->plev;

			if (Cp->plev==0) {
				stkchr(EOS);
				expand(Cp->argp,Ap-Cp->argp-1);
				op = *Cp->argp;
				Ap = Cp->argp-1;

				if (--Cp < callst)
					Cp = NULL;
			} else
				stkchr(t);
		} else if (t==',' && Cp->plev<=1) {
			stkchr(EOS);
			*Ap = op;

			if ((char *) (++Ap) >= astklm) {
				--Ap;
				error2(MTI,astkof,stksize);
			}

			while (t=getchr(),isspace(t))
				;

			putbak(t);
		} else if (t=='$' && Cp->plev<=1) {
			stkchr(t);
			switch(t=getchr()) {
			case '@':
			case '#':
			case '*':
				stkchr(t);
				break;
			default:
				putbak(t);
				break;
			}
		} else
			stkchr(t);
	}

	if (Cp!=NULL)
		error(AEOF,"EOF in argument list");

	delexit(errflag ? NOT_OK : OK);
}

static char	*inpmatch(s)
register char	*s;
{
	register char	*tp = token+1;

	while (*s) {
		*tp = getchr();

		if (*tp++ != *s++) {
			*tp = EOS;
			pbstr(token+1);
			return 0;
		}
	}

	*tp = EOS;
	return token;
}

static
getflags(xargc,xargv)
register int	*xargc;
register char 	***xargv;
{
	int c;

	while ((c = getopt(*xargc, *xargv, "elsB:D:H:I:S:T:U:")) != EOF) {
		switch (c) {
			case 'B':
				bufsize = atoi(optarg);
				break;
			case 'D':
				{
				register char *t;
				char *s[2];

				initalloc();

				for (t = s[0] = optarg; *t; t++)
					if (*t=='=') {
						*t++ = EOS;
						break;
					}

				s[1] = t;
				dodef(&s[-1],2);
				break;
				}
			case 'H':
				hshsize = atoi(optarg);
				if(hshsize<=0)
				error2(MIA,"invalid argument: %s",
					optarg);
				break;
			case 'S':
				stksize = atoi(optarg);
				break;
			case 'T':
				toksize = atoi(optarg);
				break;
			case 'U':
				{
				char *s[1];

				initalloc();
				s[0] = optarg;
				doundef(&s[-1],1);
				break;
				}
			case 'e':
				setbuf(stdout,(char *) NULL);
				signal(SIGINT,SIG_IGN);
				break;
#ifdef XASLINE
			case 'l':
			/* turn on xas .xline sync */
				xsflag = 1;
				break;
#endif /* XASLINE */
			case 's':
			/* turn on line sync */
				sflag = 1;
				break;
	                case 'I':
                        /* do search paths */
				if (strcmp(optarg, "-") == 0)
					break;
				s_path[n_path] = malloc(strlen(optarg) + 2);
				strcpy(s_path[n_path], optarg);
				strcat(s_path[n_path], "/");
				n_path++;
				break;
			default:
				fprintf(stderr,MSGSTR(USAGE,"Usage: m4 [-els] [-B Number] [-D Name[=Value]]... [-H Number]\n\
       [-I Directory] [-S Number] [-T Number] [-U Name]... [File...]\n"),0);
				delexit(NOT_OK);
		}
	}
	*xargc -= optind;
	*xargv += optind;
	return;
}

static
initalloc()
{
	static	done = 0;
	register	t;

	if (done++)
		return;

	hshtab = (struct nlist **) xcalloc(hshsize,sizeof(struct nlist *));
	callst = (struct call *) xcalloc(stksize/3+1,sizeof(struct call));
	Ap = argstk = (char **) xcalloc(stksize+3,sizeof(char *));
	ipstk[0] = ipflr = ip = ibuf = xcalloc(bufsize+1,sizeof(char));
	op = obuf = xcalloc(bufsize+1,sizeof(char));
	token = xcalloc(toksize+1,sizeof(char));

	astklm = (char *) (&argstk[stksize]);
	ibuflm = &ibuf[bufsize];
	obuflm = &obuf[bufsize];
	toklm = &token[toksize];

	for (t=0; barray[t].bname; ++t) {
		static char	p[3] = {0,0x7f , EOS};
		p[0] = t|~LOW7;
		install(barray[t].bname,p,NOPUSH);
	}

	install("unix",nullstr,NOPUSH);

}

struct nlist	*
install(nam,val,mode)
char	*nam;
register char	*val;
{
	register struct nlist *np;
	register char	*cp;
	int		l;

	if (mode==PUSH)
		lookup(nam);	/* lookup sets hshval */
	else
		while (undef(nam))	/* undef calls lookup */
			;

	np = (struct nlist *) xcalloc(1,sizeof(*np));
	np->name = copy(nam);
	np->next = hshtab[hshval];
	hshtab[hshval] = np;

	cp = xcalloc((l=strlen(val))+1,sizeof(*val));
	np->def = cp;
	cp = &cp[l];

	while (*val)
		*--cp = *val++;
}

struct nlist	*
lookup(str)
char 	*str;
{
	register char		*s1;
	register struct nlist	*np;
	static struct nlist	nodef;

	s1 = str;

	for (hshval = 0; *s1; )
		hshval += *s1++;

	hshval %= hshsize;

	for (np = hshtab[hshval]; np!=NULL; np = np->next) {
		if (!strcmp(str, np->name))
			return(np);
	}

	return(&nodef);
}

static
expand(a1,c)
char	**a1;
{
	register char	*dp;
	register struct nlist	*sp;

	sp = (struct nlist *) a1[-1];

	if (sp->tflag || trace) {
		int	i;

		fprintf(stderr,MSGSTR(TR,"Trace(%d): %s"),Cp-callst,a1[0]);

		if (c > 0) {
			fprintf(stderr,"(%s",chkbltin(a1[1]));
			for (i=2; i<=c; ++i)
				fprintf(stderr,",%s",chkbltin(a1[i]));

			fputc(')',stderr);
		}

		fputc('\n',stderr);
	}

	dp = sp->def;

	for (; *dp; ++dp) {
		if (*dp==0x7f) {
			(*barray[*(dp+1)&LOW7].bfunc)(a1,c);
			++dp;
		} else if (dp[1]=='$') {
			if (isdigit(*dp)) {
				register	n;
				if ((n = *dp-'0') <= c)
					pbstr(a1[n]);
				++dp;
			} else if (*dp=='#') {
				pbnum((long) c);
				++dp;
			} else if (*dp=='*' || *dp=='@') {
				register i = c;
				char **a = a1;

				if (i > 0)
					for (;;) {
						if (*dp=='@')
							pbstr(rquote);

						pbstr(a[i--]);

						if (*dp=='@')
							pbstr(lquote);

						if (i <= 0)
							break;

						pbstr(",");
					}
				++dp;
			} else
				putbak(*dp);
		} else
			putbak(*dp);
	}
}

setfname(s)
register char 	*s;
{
        fname[ifx] = (char *) malloc(strlen(s) + 1);
	strcpy(fname[ifx],s);
	fline[ifx] = 1;
	nflag = 1;
	lnsync(stdout);
}


#ifdef XASLINE
/* this performs the -s option and the -l option		*/
/* the ifdef  XASLINE statements allow the new -l option	*/
/* code to be compiled or just the old code to be compiled.	*/
/* THIS IS MODIFIED TO PRODUCE .XLINE WHEN -l FLAG IS SPECIFIED */ 
/* NOTE: data structures are associated arrays  fline[] fname[] where    */
/*       where fline has line number in the file and fname is the name   */
/*       of the file. They are used as a stack.				 */
static
lnsync(iop)
register FILE	*iop;
{
	static int cline = 0;
	static int cfile = 0;

	/* check for the xsflag as well as the sflag			*/
	if ((!sflag && !xsflag) || iop!=stdout)
		return;
	if (nflag || ifx!=cfile) {
		nflag = 0;
		cfile = ifx;
		/* need to distinguish between sflag and xsflag		*/
	    	if (sflag){
			fprintf(iop,"#line %d \"",cline = fline[ifx]);
			fpath(iop);
			fputs("\"\n",iop);
			}
		if (xsflag){
			fprintf(iop,"\t.xline\t%d,\"%s",fline[ifx]-1,
				    fname[ifx]);
			fputs("\"\n",iop);
			cline = fline[ifx];
			}
		} 
	else if (++cline != fline[ifx]){
		if (sflag){
			fprintf(iop,"#line %d\n",cline = fline[ifx]);
			}
		if (xsflag){
			fprintf(iop,"\t.xline\t%d,\"%s\"\n",fline[ifx],
				    fname[ifx]);
			cline = fline[ifx];
			}
		}
}

#else /* XASLINE */
/* THIS IS THE ORIGINAL LNSYNC CODE UNMODIFIED   	*/
lnsync(iop)
register FILE	*iop;
{
	static int cline = 0;
	static int cfile = 0;

	if (!sflag || iop!=stdout)
		return;
	if (nflag || ifx!=cfile) {
		nflag = 0;
		cfile = ifx;
		fprintf(iop,"#line %d \"",cline = fline[ifx]);
		fpath(iop);
		fputs("\"\n",iop);

	} else if (++cline != fline[ifx])
		fprintf(iop,"#line %d\n",cline = fline[ifx]);
}
#endif /* XASLINE */

static
fpath(iop)
register FILE	*iop;
{
	register	i;

	fputs(fname[0],iop);

	for (i=1; i<=ifx; ++i)
		fprintf(iop,":%s",fname[i]);
}

static void
catchsig()
{
	signal(SIGHUP,SIG_IGN);
	signal(SIGINT,SIG_IGN);

	delexit(NOT_OK);
}

delexit(code)
{
	register i;

	cf = stdout;
	fflush(stdout);

/*	if (ofx != 0) {	quitting in middle of diversion
 *		ofx = 0;
 *		code = NOT_OK;
 *	}
 */
	ofx = 0;	/* ensure that everything comes out */
	for (i=1; i<10; i++)
		undiv(i,code);

	tempfile[7] = 'a';
	unlink(tempfile);

	if (code==OK)
		exit(code);

	_exit(code);
}

static
puttok(tp)
register char *tp;
{
	if (Cp) {
		while (*tp)
			stkchr(*tp++);
	} else if (cf)
		while (*tp)
			sputchr(*tp++,cf);
}

pbstr(str)
register char *str;
{
	register char *p;

	for (p = str + strlen(str); --p >= str; )
		putbak(*p);
}

undiv(i,code)
register	i;
{
	register FILE *fp;
	register	c;

	if (i<1 || i>9 || i==ofx || !ofile[i])
		return;

	fclose(ofile[i]);
	tempfile[7] = 'a'+i;

	if (code==OK && cf) {
		fp = xfopen(tempfile,"r");

		while ((c=getc(fp)) != EOF)
			sputchr(c,cf);

		fclose(fp);
	}

	unlink(tempfile);
	ofile[i] = NULL;
}

static char 	*copy(s)
register char *s;
{
	register char *p;

	p = xcalloc(strlen(s)+1,sizeof(char));
	strcpy(p, s);
	return(p);
}

pbnum(num)
long num;
{
	pbnbr(num,10,1);
}

pbnbr(nbr,base,len)
long	nbr;
register	base, len;
{
	register	neg = 0;

	if (base<=0)
		return;

	if (nbr<0)
		neg = 1;
	else
		nbr = -nbr;

	while (nbr<0) {
		register int	i;
		if (base>1) {
			i = nbr%base;
			nbr /= base;
			while (i > 0) {
				i -= base;
				++nbr;
			}
			i = -i;
		} else {
			i = 1;
			++nbr;
		}
		putbak(itochr(i));
		--len;
	}

	while (--len >= 0)
		putbak('0');

	if (neg)
		putbak('-');
}

static
itochr(i)
register	i;
{
	if (i>9)
		return i-10+'A';
	else
		return i+'0';
}

long ctol(str)
register char *str;
{
	register sign;
	long num;

	while (isspace(*str))
		++str;
	num = 0;
	if (*str=='-') {
		sign = -1;
		++str;
	}
	else
		sign = 1;
	while (isdigit(*str))
		num = num*10 + *str++ - '0';
	return(sign * num);
}

min(a,b)
{
	if (a>b)
		return(b);
	return(a);
}

FILE	*
xfopen(name,mode)
char	*name,
	*mode;
{
	FILE	*fp;

	if ((fp=fopen(name,mode))==NULL)
		error(OFL,badfile);

	return fp;
}

static char	*xcalloc(nbr,size)
{
	register int	alloc_size;
	register char	*ptr;

	alloc_size = nbr * size;

	/*
	** See adding  this improves perfromance....
	if ((ptr=calloc((unsigned) nbr,(unsigned) size)) == NULL)
		error(OST,nocore);
	*/

	if ((ptr= (unsigned char *) malloc(alloc_size)) == NULL)
		error(OST,nocore);
	bzero(ptr, alloc_size);
	return ptr;
}

error2(index,str,num)
	int index;
	char *str;
	int num;
{
	char buf[500];

	sprintf(buf,MSGSTR(index,str),num);
	error(0,buf);
}

error(index,str)
	int index;
	char *str;
{
	fprintf(stderr,"\n%s:",procnam);
	fpath(stderr);
	if(index)
		fprintf(stderr,":%d %s\n",fline[ifx],MSGSTR(index,str));
	else
		fprintf(stderr,":%d %s\n",fline[ifx],str);
	if (Cp) {
		register struct call	*mptr;

		/* fix limit */
		*op = EOS;
		(Cp+1)->argp = Ap+1;

		for (mptr=callst; mptr<=Cp; ++mptr) {
			register char	**aptr, **lim;

			aptr = mptr->argp;
			lim = (mptr+1)->argp-1;
			if (mptr==callst)
				fputs(*aptr,stderr);
			++aptr;
			fputs("(",stderr);
			if (aptr < lim)
				for (;;) {
					fputs(*aptr++,stderr);
					if (aptr >= lim)
						break;
					fputs(",",stderr);
				}
		}
		while (--mptr >= callst)
			fputs(")",stderr);

		fputs("\n",stderr);
	}
	delexit(NOT_OK);
}

static char	*chkbltin(s)
char	*s;
{
	static char	buf[24];

	if (*s==0x7f){
		sprintf(buf,"<%s>",barray[*(s+1)&LOW7].bname);
		return buf;
	}

	return s;
}

int	getchr()
{
	if (ip > ipflr)
		return (*--ip);
	C = feof(ifile[ifx]) ? EOF : getc(ifile[ifx]);
	if (C =='\n')
		fline[ifx]++;
	return (C);
}

