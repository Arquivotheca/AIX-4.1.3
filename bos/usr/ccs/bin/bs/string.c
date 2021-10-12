static char sccsid[] = "@(#)09  1.5  src/bos/usr/ccs/bin/bs/string.c, cmdlang, bos411, 9428A410j 3/9/94 12:19:20";
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: ematch, errxx, sindex, substr, trans
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#if defined(NLS) || defined(KJI)
#include <NLchar.h>
#endif
#define ALLOC 127
#define ESIZE	(256*5)
#define PATSIZE	100
#ifdef MSG
#include "bs_msg.h"
extern nl_catd catd;
#define MSGSTR(N,S) catgets(catd,MS_BS,N,S)
#else
#define MSGSTR(N,S) S
#endif

char *salloc();

sindex(p, s)
char *p, *s;
{
	register i, j;
	int tmp;

	for(i = 0, tmp = 1; p[i]; ++i, tmp++)
#if defined(NLS) || defined(KJI)
	{
		for(j = 0; s[j]; ++j)
		{
			if(p[i] == s[j])
			{
				if (NCisshift(p[i]))
				{
					if (p[++i] == s[++j])
						return tmp;
					else 
						--i, --j;
				}
				else
					return tmp;
			}
			if (NCisshift(s[j])) ++j;
		}
		if (NCisshift(p[i])) ++i;
	}
#else 
		for(j = 0; s[j]; ++j)
			if(p[i] == s[j])
				return ++i;
#endif
	return 0;
}

char *trans(s, f, t)
char *s, *f, *t;
{
	char *ret;
	register i, j;
	int t_len = strlen(t);
#if defined(NLS) || defined(KJI)
	NLchar *tmp, *NLptr;
	int s_len = strlen(s);
	int f_len = strlen(f);
	int len1, len2, len3;
	NLchar *nls_s, *nls_f, *nls_t;

	nls_s = (NLchar *) malloc((s_len + 1) * sizeof(NLchar));
	nls_f = (NLchar *) malloc((f_len + 1) * sizeof(NLchar));
	nls_t = (NLchar *) malloc((t_len + 1) * sizeof(NLchar));
	len1 = NCdecstr(s, nls_s, s_len+1);
	len2 = NCdecstr(f, nls_f, f_len+1);
	len3 = NCdecstr(t, nls_t, t_len+1);

	NLptr = tmp = (NLchar *) malloc((len1 + 1) * sizeof(NLchar));
	for(i = 0; i<len1; i++) {
		for(j = 0; j<len2; ++j) {
			if(nls_s[i] == nls_f[j]) {
				if(j < len3)
					*tmp++ = nls_t[j];
				goto brk;
			}
		}
		*tmp++ = nls_s[i];
brk:		;
	}
	*tmp = '\0';
	/* if we translate string with one byte characters to a string with
	   two byte characters, we need double the space */
	ret = salloc((s_len * 2), 0);
	NCencstr(NLptr, ret, (s_len * 2));
	free(NLptr);
	free(nls_s);
	free(nls_f);
	free(nls_t);
	return ret;
#else
	register char *tmp;
	ret = tmp = salloc(strlen(s), 0);
	for(i = 0; s[i]; i++) {
		for(j = 0; f[j]; ++j) {
			if(s[i] == f[j]) {
				if(j < t_len)
					*tmp++ = t[j];
				goto brk;
			}
		}
		*tmp++ = s[i];
brk:		;
	}
	*tmp = '\0';
	return ret;
#endif
}

char *substr(s, f, w)
register char *s;
register w;
{
	register char *sc;
#ifdef KJI
	int sz = NLcplen(s);
#else
	int sz = strlen(s);
#endif
	char *sr;

	if(f <= 0 || f > sz)
		return "\0";
	--f;
	w = w > (sz - f)? sz - f: w;
#ifdef KJI
	while (f--) 
		s += NLchrlen(s);
	sr = sc = salloc(w*2, 0);
	do {
		if (NCisshift(*s))
			*sc++ = *s++;
		*sc++ = *s++;
	}  while(--w);
#else
	s += f;
	sr = sc = salloc(w, 0);
	do
		*sc++ = *s++;
	while(--w);
#endif
	*sc = '\0';
	return sr;
}

char	*Mstring[10];
extern int	nbra;

#define INIT	register char *sp = instring;
#define GETC()		(*sp++)
#define PEEKC()		(*sp)
#define UNGETC(c)	(--sp)
#define RETURN(c)	return
#define ERROR(c)	errxx(c)

extern char	*loc2;

ematch(s, p)
char *s;
register char *p;
{
	static char expbuf[ESIZE], last[PATSIZE];
	char *compile(), *salloc();
	register int i, num;
	extern char *braslist[], *braelist[];

	if(strcmp(p, last) != 0) {
		compile(p, expbuf, &expbuf[ESIZE], 0);
#ifdef KJI
		NLstrcpy(last, p);
#else
		strcpy(last, p);
#endif
	}
	if(advance(s, expbuf)) {
		for(i = nbra; i-- > 0;) {
			p = braslist[i];
			num = braelist[i] - p;
			strncpy(Mstring[i] = salloc(num + 1, 0), p, num);
			Mstring[i][num] = '\0';
		}
#ifdef KJI
		for (i = 0; s < loc2; s += NLchrlen(s), i++)
			;
		return(i);
#else
		return(loc2-s);
#endif
	}
	return(0);
}

errxx(c)
{
	error(MSGSTR(S_ERROR,"RE error"));				/*MSG*/
}

#if defined(NLS) || defined(KJI)
#include  <NLregexp.h>
#else
#include  <regexp.h>
#endif
