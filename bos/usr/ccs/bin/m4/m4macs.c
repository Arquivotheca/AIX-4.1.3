static char sccsid[] = "@(#)87	1.21  src/bos/usr/ccs/bin/m4/m4macs.c, cmdm4, bos412, 9446C 11/14/94 12:27:26";
/*
 * COMPONENT_NAME: (CMDM4) Macroprocessor 
 *
 * FUNCTIONS: arg, def, dochcom, docq, dodecr, dodef, dodefn,
 *	      dodiv, dodivnum, dodlen, dodnl, dodump, doerrp,
 *	      doeval, doexit, doif, doifdef, doincl, doincr,
 *	      doindex, dolen, domake, dopopdef, dopushdef,
 *	      doshift, dosincl, dosubstr, dosyscmd, dosysval,
 *	      dotransl, dotroff, dotron, doundef, doundiv,
 *	      dowrap, dump, incl, leftmatch, ncgetbuf, undef
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
#include	<unistd.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<sys/sysmacros.h>
#include	<sys/stat.h>
#include	"m4.h"

#define arg(n)	(c<(n)? nullstr: ap[n])

char *s_path[40];
int n_path = 0;
static char *mkpidtmp();

static wchar_t* 
ncgetbuf(src)
char *src;
{
	register int dlen;
	register wchar_t *dest;

	/* Alloc one wchar_t for each of the bytes in the multibyte
	** string: src.  Plus one more wchar_t for the null wchar_t.
	*/
	dlen = ( (strlen(src)+1) * sizeof(wchar_t));
	dest = (wchar_t*) malloc( dlen);
	if(dest == NULL){
		error(OST,nocore);
	}
	if (mbstowcs(dest, src, dlen/sizeof(wchar_t)) <0) {
		error2(MBWC,MBtoWCerr,src);
	}
	return (dest);
}


static
dochcom(ap,c)
char	**ap;
{
	register char	*l = arg(1);
	register char	*r = arg(2);

	if (strlen(l)>MAXSYM || strlen(r)>MAXSYM)
		error2(CMLB,"comment marker longer than %d bytes",MAXSYM);
	strcpy(lcom,l);
	strcpy(rcom,*r?r:"\n");
}

static
docq(ap,c)
register char 	**ap;
{
	register char	*l = arg(1);
	register char	*r = arg(2);

	if (strlen(l)>MAXSYM || strlen(r)>MAXSYM)
		error2(QLNB,"quote marker longer than %d bytes", MAXSYM);

	if (c<=1 && !*l) {
		l = "`";
		r = "'";
	} else if (c==1) {
		r = l;
	}

	strcpy(lquote,l);
	strcpy(rquote,r);
}

static
dodecr(ap,c)
char 	**ap;
{
	pbnum(ctol(arg(1))-1);
}

dodef(ap,c)
char	**ap;
{
	def(ap,c,NOPUSH);
}

static
def(ap,c,mode)
register char 	**ap;
{
	register char	*s;

	if (c<1)
		return;

	s = ap[1];
	if (isalpha(*s)||*s=='_')
		while (s++,isalnum(*s)||*s=='_')
			;
	if (*s || s==ap[1])
		error(BMN,"bad macro name");

	if ((strcmp(ap[1],ap[2])==0)||(strcmp(ap[2],"$0")==0))
		error(MD,"macro defined as itself");
	install(ap[1],arg(2),mode);
}

static
dodefn(ap,c)
register char	**ap;
register c;
{
	register char *d;

	while (c > 0)
		if ((d = lookup(ap[c--])->def) != NULL) {
			pbstr(rquote);
			while (*d)
				putbak(*d++);
			pbstr(lquote);
		}
}

static
dodiv(ap,c)
register char **ap;
{
	register int f;

	f = atoi(arg(1));
	if (f>=10 || f<0) {
		cf = NULL;
		ofx = f;
		return;
	}
	tempfile[7] = 'a'+f;
	if (ofile[f] || (ofile[f]=xfopen(tempfile,"w"))) {
		ofx = f;
		cf = ofile[f];
	}
}

/* ARGSUSED */
static
dodivnum(ap,c)
{
	pbnum((long) ofx);
}

/* ARGSUSED */
static
dodnl(ap,c)
char 	*ap;
{
	register t;

	while ((t=getchr())!='\n' && t!=EOF)
		;
}

static
dodump(ap,c)
char 	**ap;
{
	register struct nlist *np;
	register	i;

	if (c > 0)
		while (c--) {
			if ((np = lookup(*++ap))->name != NULL)
				dump(np->name,np->def);
		}
	else
		for (i=0; i<hshsize; i++)
			for (np=hshtab[i]; np!=NULL; np=np->next)
				dump(np->name,np->def);
}

static
dump(name,defnn)
register char	*name,
		*defnn;
{
	register char	*s = defnn;

	fprintf(stdout,"%s:\t",name);

	while (*s++)
		;
	--s;

	while (s>defnn)
		if (*defnn == 0x7f) {
			fprintf(stdout,"<%s>",barray[*(defnn + 1)&LOW7].bname);
			s = defnn;
		} else {
			fputc(*--s,stdout);
		}

	fputc('\n',stdout);
}

static
doerrp(ap,c)
char 	**ap;
{
	if (c > 0)
		fputs(ap[1],stderr);
}

long	evalval;	/* return value from yacc stuff */
char	*pe;	/* used by grammar */
static
doeval(ap,c)
char 	**ap;
{
	register	base = atoi(arg(2));
	register	pad = atoi(arg(3));

	evalval = 0;
	if (c > 0) {
		pe = ap[1];
		if (yyparse()!=0)
			error(IE,"invalid expression");
	}
	pbnbr(evalval, base>0?base:10, pad>0?pad:1);
}

static
doexit(ap,c)
char	**ap;
{
	delexit(atoi(arg(1)));
}

static
doif(ap,c)
register char **ap;
{
	if (c < 3)
		return;
	while (c >= 3) {
		if (strcmp(ap[1],ap[2])==0) {
			pbstr(ap[3]);
			return;
		}
		c -= 3;
		ap += 3;
	}
	if (c > 0)
		pbstr(ap[1]);
}

static
doifdef(ap,c)
char 	**ap;
{
	if (c < 2)
		return;

	while (c >= 2) {
		if (lookup(ap[1])->name != NULL && strcmp(lookup(ap[1])->def,"0")) {
			pbstr(ap[2]);
			return;
		}
		c -= 2;
		ap += 2;
	}

	if (c > 0)
		pbstr(ap[1]);
}

static
doincl(ap,c)
char	**ap;
{
	incl(ap,c,1);
}

static
incl(ap,c,noisy)
register char 	**ap;
{
  FILE *fd = NULL;
  int i = 0;
  char *temp_path;

	if (c>0 && strlen(ap[1])>0) {
		if (ifx >= 9)
			error(IFLN,"input file nesting too deep (9)");
                if (n_path != 0) {

                  while ((fd == NULL) && (i < n_path)) {
                    temp_path = malloc(strlen(s_path[i]) + strlen(ap[1]) + 1);
                    strcpy(temp_path, s_path[i]);
                    strcat(temp_path, ap[1]);
                    fd = fopen(temp_path, "r");
                    i++;
                    free(temp_path);
                  }
                }
                else
                 fd = fopen(ap[1], "r");

		if ((ifile[++ifx]=fd)==NULL){
			--ifx;
			if (noisy)
				error(OFL,badfile);
		} else {
			ipstk[ifx] = ipflr = ip;
			setfname(ap[1]);
		}
	}
}

static
doincr(ap,c)
char 	**ap;
{
	pbnum(ctol(arg(1))+1);
}

static
doindex(ap,c)
char	**ap;
{
	register char	*subj = arg(1);
	register char	*obj  = arg(2);
	register	i;

	for (i=0; *subj; ++i)
		if (leftmatch(subj++,obj)) {
			pbnum( (long) i );
			return;
		}

	pbnum( (long) -1 );
}

static
leftmatch(str,substr)
register char	*str;
register char	*substr;
{
	while (*substr)
		if (*str++ != *substr++)
			return (0);

	return (1);
}

/* returns byte length */
static
dolen(ap,c)
char 	**ap;
{
	pbnum((long) strlen(arg(1)));
}

/* new function - returns display length */
static
dodlen(ap,c)
char **ap;
{
	wchar_t *wcs=0;
	int mblen, dispcol;
	char 	*p;

	p = arg(1);

	mblen = (  strlen(p) + 1) * sizeof(wchar_t) ;
	wcs = (wchar_t *)malloc(mblen);
	if(wcs == NULL) {
		error(OST,nocore);
	}
	if(mbstowcs(wcs, p, mblen/sizeof(wchar_t)) <0) {
		error2(MBWC,MBtoWCerr,p);
	};
	dispcol = wcswidth(wcs,mblen/sizeof(wchar_t));
	if(dispcol<0){
		error(WCDISP,WCDISPerr);
	}
	pbnum((long)dispcol);

	if (wcs) cfree((char *)wcs);
}

static
domake(ap,c)
char 	**ap;
{
	if (c > 0)
		pbstr(mkpidtmp(ap[1]));
}

char *
mkpidtmp(template)
char *template;
{
	char *tmp, pidstr[40];
	int mb_cur_max = MB_CUR_MAX;
	wchar_t wc;
	int len, cnt;

	if (template ==  NULL || !(*template))
		return (NULL);

	if (mb_cur_max>1) {
		for (cnt=0, tmp=template; *tmp; tmp += (len>0)? len:1) {
			len = mbtowc(&wc, tmp, mb_cur_max);
			cnt = (len < 1 || wc != L'X')? 0 : cnt + 1;
		}
		tmp -= len * cnt;
	}else {
		len = 1;
		for (tmp=template; *tmp; tmp++);	/* goto end of str. */
		for (cnt = 0; (*(tmp-1) == 'X'); tmp--, cnt++);
	}
                
	if (cnt==0) {   			   /* no X's found */
		if (access(template, F_OK) == -1){ /* is filename unique? */
			return (template);
		}else {
			error2(BADTMP,"%s already exists",template);
		}
	}
	/* From now on, tmp is pointing to the begining of */
	/* the last consequtive X's  (e.g. "abcdXXXXX")    */
	/*                             tmp  ----^          */ 

	/* get process id as a string */
	sprintf(pidstr, "%d", getpid());
	if ((strlen(template) + strlen(pidstr) - (len * cnt)) > PATH_MAX)
		error(TOOLONG,"temporary file name is too long");
	for (cnt=0; cnt < strlen(pidstr); cnt++)
		tmp[cnt] = pidstr[cnt];
	tmp[cnt] = '\0';
	if (access(template, F_OK) == 0)
		error2(BADTMP,"%s already exists",template);
	return (template);
}

static
dopopdef(ap,c)
char	**ap;
{
	register	i;

	for (i=1; i<=c; ++i)
		undef(ap[i]);
}

static
dopushdef(ap,c)
char	**ap;
{
	def(ap,c,PUSH);
}

static
doshift(ap,c)
register char	**ap;
register c;
{
	if (c <= 1)
		return;

	for (;;) {
		pbstr(rquote);
		pbstr(ap[c--]);
		pbstr(lquote);

		if (c <= 1)
			break;

		pbstr(",");
	}
}

static
dosincl(ap,c)
char	**ap;
{
	incl(ap,c,0);
}

static
dosubstr(ap,c)
register char 	**ap;
{
	char	*str;
	int	inlen, outlen;
	register	offset, ix;

	inlen = strlen(str=arg(1));
	offset = atoi(arg(2));

	if (offset<0 || offset>=inlen)
		return;

	outlen = c>=3? atoi(ap[3]): inlen;
	ix = min(offset+outlen,inlen);

	while (ix > offset)
		putbak(str[--ix]);
}

static
dosyscmd(ap,c)
char 	**ap;
{
	sysrval = 0;
	if (c > 0) {
		fflush(stdout);
		sysrval = system(ap[1]);
	}
}

/* ARGSUSED */
static
dosysval(ap,c)
char	**ap;
{
	pbnum((long) (sysrval < 0 ? sysrval :
		(sysrval >> 8) & ((1 << 8) - 1)) |
		((sysrval & ((1 << 8) - 1)) << 8));
}

static
dotransl(ap,c)
char 	**ap;
{

	if(mbcurmax== 1)
		dotransl_sb(ap,c);
	else
		dotransl_mb(ap,c);

}
		

static
dotransl_sb(ap,c)
char 	**ap;
{
	/* Transliteration takes place as dictated by the from and to strings.
	   The buffer on which this takes place is pointed to by ap[1] 
	   (source and sink) Note that the source buffer is also the 
	   sink buffer, ie, replacements are done in the buffer given by ap[1].
	*/

	char	*sink, *from, *sto;
	register char	*source, *to;
	register char	*p;

	if (c<1)
		return;
	sink 	= ap[1];
	from 	= arg(2);
	sto 	= arg(3);
	for (source = ap[1]; *source; source++) {
		to = sto;
		for (p = from; *p; ++p) {
			if (*source==*p)
				break;
			if (*to)
				++to;
		}
		if (*p) {
			if (*to)
				*sink++ = *to;
		} else
			*sink++ = *source;
	}
	*sink = EOS;
	pbstr(ap[1]);
}
static
dotransl_mb(ap,c)
char 	**ap;
{
	wchar_t  	*ap1, *sink, *fr, *sto;
	wchar_t		*f_ap1=0, *f_fr=0, *f_sto=0;

	char 	*dest=0;       /* used for final result instead of ap[1] */ 
	int 	dest_len;

	int 	dlen;
	register wchar_t 	*to, *source;
	register wchar_t	*i, *wc;


	if (c<1)
		return;


	f_ap1 = ap1  =	ncgetbuf(ap[1]);
	sink	=	ap1;

	f_fr = fr   =  	(c < 2 ?  (wchar_t*)nullstr : ncgetbuf(ap[2]));
	f_sto = sto =	(c < 3 ?  (wchar_t*)nullstr : ncgetbuf(ap[3]));

	/* how many bytes are need by the multibyte string after wctomb 
	   conversion? Assume each wchar_t can be expanded to MB_CUR_MAX 
	   number of bytes and add one for the terminating null byte 
	*/

	dest_len = (wcslen(sink) +1) * mbcurmax ;
	dest = (char *)malloc(dest_len);
	if(dest == NULL){
		error(OST,nocore);
	}

	for (source = ap1; *source; source++) {
		to = sto;
		for (wc = fr; *wc; ++wc) {
			if (*source == *i)
				break;
			if (*to)
				++to;
		}
		if (*wc) {
			if (*to)
				*sink++ = *to;
		} else
			*sink++ = *source;
	}
	*sink = EOS;

	if (f_fr) cfree((char *)f_fr);
	if (f_sto) cfree((char *)f_sto);

	if(wcstombs( dest, ap1, dest_len/mbcurmax) <0){
		error(WCMB,WCtoMBerr);
	}

	pbstr(dest);

	if (dest) cfree(dest);
	if (f_ap1) cfree((char *)f_ap1);
}

static
dotroff(ap,c)
register char	**ap;
{
	register struct nlist	*np;

	trace = 0;

	while (c > 0)
		if ((np=lookup(ap[c--]))->name)
			np->tflag = 0;
}

static
dotron(ap,c)
register char	**ap;
{
	register struct nlist	*np;

	trace = !*arg(1);

	while (c > 0)
		if ((np=lookup(ap[c--]))->name)
			np->tflag = 1;
}

doundef(ap,c)
char	**ap;
{
	register	i;

	for (i=1; i<=c; ++i)
		while (undef(ap[i]))
			;
}

undef(nam)
char	*nam;
{
	register struct	nlist *np, *tnp;

	if ((np=lookup(nam))->name==NULL)
		return 0;
	tnp = hshtab[hshval];	/* lookup sets hshval */
	if (tnp==np)	/* it's in first place */
		hshtab[hshval] = tnp->next;
	else {
		while (tnp->next != np)
			tnp = tnp->next;

		tnp->next = np->next;
	}
	cfree(np->name);
	cfree(np->def);
	cfree((char *) np);
	return 1;
}

static
doundiv(ap,c)
register char 	**ap;
{
	register int i;

	if (c<=0)
		for (i=1; i<10; i++)
			undiv(i,OK);
	else
		while (--c >= 0)
			undiv(atoi(*++ap),OK);
}

static
dowrap(ap,c)
char	**ap;
{
	register char	*a = arg(1);

	if ((Wrapstr = realloc(Wrapstr, strlen(Wrapstr) + strlen(a)+1)) == NULL)
		error(OST,nocore);
	strcat(Wrapstr,a);
}

struct bs	barray[] = {
	dochcom,	"changecom",
	docq,		"changequote",
	dodecr,		"decr",
	dodef,		"define",
	dodefn,		"defn",
	dodiv,		"divert",
	dodivnum,	"divnum",
	dodlen,		"dlen",
	dodnl,		"dnl",
	dodump,		"dumpdef",
	doerrp,		"errprint",
	doeval,		"eval",
	doexit,		"m4exit",
	doif,		"ifelse",
	doifdef,	"ifdef",
	doincl,		"include",
	doincr,		"incr",
	doindex,	"index",
	dolen,		"len",
	domake,		"maketemp",
	dopopdef,	"popdef",
	dopushdef,	"pushdef",
	doshift,	"shift",
	dosincl,	"sinclude",
	dosubstr,	"substr",
	dosyscmd,	"syscmd",
	dosysval,	"sysval",
	dotransl,	"translit",
	dotroff,	"traceoff",
	dotron,		"traceon",
	doundef,	"undefine",
	doundiv,	"undivert",
	dowrap,		"m4wrap",
	0,		0
};
