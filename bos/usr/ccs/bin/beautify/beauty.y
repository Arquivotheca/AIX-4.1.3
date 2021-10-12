/* @(#)92	1.5  src/bos/usr/ccs/bin/beautify/beauty.y, cmdprog, bos411, 9428A410j 3/9/94 12:48:30 */
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: main, forst, newline, pop, push, putout, tab, yyerror, yyinit
 *
 * ORIGINS: 26 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
%term  xxif 300 xxelse 301 xxwhile 302 xxrept 303 xxdo 304 xxrb 305 xxpred 306
%term xxident 307 xxle 308 xxge 309 xxne 310 xxnum 311 xxcom 312
%term xxstring 313 xxexplist 314 xxidpar 315 xxelseif 316  xxlb 318 xxend 319
%term xxcase 320 xxswitch 321 xxuntil 322 xxdefault 323 
%term xxeq 324

%left	'|'
%left	'&'
%left	'!'
%binary	'<' '>' xxeq xxne xxge xxle
%left	'+' '-'
%left	'*' '/'
%left	xxuminus
%right	'^'

%{

#include "b.h"
#include <stdio.h>
#include <locale.h>
#include <nl_types.h>
#ifdef MSG
#include "struct_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_STRUCT,n,s) 
#else
#define MSGSTR(n,s) s
#endif
%}

%%
%{
struct node *t;
%}


allprog:	prog xxnew
	;

prog:	stat
	|	prog stat
	;

stat:		 iftok pred nlevel elsetok nlevel
	|	 iftok  pred  nlevel
	|	xxtab whtok  pred  nlevel
	|	xxtab rpttok nlevel optuntil
	|	xxtab dotok nlevel
	|	xxtab swtok oppred pindent lbtok caseseq xxtab rbtok mindent
	|	xxtab fstok
	|	lbtok prog xxtab rbtok
	|	lbtok rbtok
	|	 labtok stat
	|	xxnl comtok stat
	|	error
	;


xxtab:		=	{
			if (!xxlablast) tab(xxindent);
			xxlablast = 0;
			}

xxnl:	=	newline();
xxnew:	=	putout('\n',"\n");
nlevel:	pindent stat mindent;
pindent:		=
				{
				if (xxstack[xxstind] != xxlb)
					++xxindent;
				};
mindent:			=
				{if (xxstack[xxstind] != xxlb && xxstack[xxstind] != xxelseif)
					--xxindent;
				pop();
				};
caseseq:	casetok caseseq
	|	casetok
	;

casetok:	xxtab xxctok predlist pindent prog mindent
	|	xxtab xxctok predlist pindent mindent
	|	xxtab deftok pindent prog mindent
	|	xxnl comtok casetok
	;

xxctok:	xxcase		=	{putout(xxcase,"case "); free((void *) $1); push(xxcase); }


deftok:		xxdefault ':'		=		{
						putout(xxcase,"default");
						free((void *) $1);
						putout(':',":");
						free((void *) $2);
						push(xxcase);
						}
swtok:	xxswitch			=	{putout(xxswitch,"switch"); free((void *) $1); push(xxswitch); }

fstok:	xxend		=	{
				free((void *) $1);
				putout(xxident,"end");
				putout('\n',"\n");
				putout('\n',"\n");
				putout('\n',"\n");
				}
	|	xxident	=	{
				putout(xxident,$1);
				free((void *) $1);
				newflag = 1;
				forst();
				newflag = 0;
				};

		

identtok:	xxident '(' explist ')'	=	{
				xxt = addroot($1,xxident,0,0);
				$$ = addroot("",xxidpar,xxt,$3);
				}

	|	xxident		=	$$ = addroot($1,xxident,0,0);
	;

predlist:	explist  ':'		=	{
				yield($1,0);
				putout(':',":");
				freetree($1);
				}
explist:	expr ',' explist		=	$$ = addroot($2,xxexplist,checkneg($1,0),$3);
	|	expr					=	$$ = checkneg($1,0);
	;


oppred:	pred
	|
	;

pred:	'(' expr ')'	=	{ t = (struct node *) checkneg($2,0);
				yield(t,100);  freetree(t);	};

expr:		'(' expr ')'	=	$$ = $2;
	|	'-' expr	%prec xxuminus	=	$$ = addroot($1,xxuminus,$2,0);
	|	'!' expr	=	$$ = addroot($1,'!',$2,0);
	|	expr '+' expr	=	$$ = addroot($2,'+',$1,$3);
	|	expr '-' expr	=	$$ = addroot($2,'-',$1,$3);
	|	expr '*' expr	=	$$ = addroot($2,'*',$1,$3);
	|	expr '/' expr	=	$$ = addroot($2,'/',$1,$3);
	|	expr '^' expr	=	$$ = addroot($2,'^',$1,$3);
	|	expr '|' expr	=	$$ = addroot($2,'|',$1,$3);
	|	expr '&' expr	=	$$ = addroot($2,'&',$1,$3);
	|	expr '>' expr	=	$$ = addroot($2,'>',$1,$3);
	|	expr '<' expr	=	$$ = addroot($2,'<',$1,$3);
	|	expr xxeq expr	=	$$ = addroot($2,xxeq,$1,$3);
	|	expr xxle expr	=	$$ = addroot($2,xxle,$1,$3);
	|	expr xxge expr	=	$$ = addroot($2,xxge,$1,$3);
	|	expr xxne expr	=	$$ = addroot($2,xxne,$1,$3);
	|	identtok		=	$$ = $1;
	|	xxnum		=	$$ = addroot($1,xxnum,0,0);
	|	xxstring		=	$$ = addroot($1,xxstring,0,0);
	;

iftok:	xxif		=
				{
				if (xxstack[xxstind] == xxelse && !xxlablast)
					{
					--xxindent;
					xxstack[xxstind] = xxelseif;
					putout(' '," ");
					}
				else
					{
					if (!xxlablast)
						tab(xxindent);
					xxlablast = 0;
					}
				putout(xxif,"if");
				free((void *) $1);
				push(xxif);
				}
elsetok:	xxelse	=
				{
				tab(xxindent);
				putout(xxelse,"else");
				free((void *) $1);
				push(xxelse);
				}
whtok:	xxwhile		=	{
				putout(xxwhile,"while");
				free((void *) $1);
				push(xxwhile);
				}
rpttok:	xxrept	=			{
					putout(xxrept,"repeat");
					free((void *) $1);
					push(xxrept);
					}
optuntil:	xxtab unttok pred
		|
		;

unttok:	xxuntil	  = 	{
			putout('\t',"\t");
			putout(xxuntil,"until");
			free((void *) $1);
			}
dotok:	dopart opdotok
	;
dopart:	xxdo	identtok '=' expr  ',' expr 		=
					{push(xxdo);
					putout(xxdo,"do");
					free((void *) $1);
					puttree($2);
					putout('=',"=");
					free((void *) $3);
					puttree($4);
					putout(',',",");
					free((void *) $5);
					puttree($6);
					}
opdotok:	',' expr		=	{
						putout(',',",");
						puttree($2);
						}
	|	;
lbtok:	'{'		=	{
				putout('{'," {");
				push(xxlb);
				}
rbtok:	'}'			=	{ putout('}',"}");  pop();   }
labtok:	xxnum		=	{
				tab(xxindent);
				putout(xxnum,$1);
				putout(' ',"  ");
				xxlablast = 1;
				}
comtok:	xxcom		=	{ putout(xxcom,$1);  free((void *) $1);  xxlablast = 0; }
	|	comtok xxcom		= { putout ('\n',"\n"); putout(xxcom,$2);  free((void *) $2);  xxlablast = 0; };
%%

yyerror(s)
char *s;
	{
	extern int yychar;
	fprintf(stderr,"\n%s",s);
	switch (yychar) {
		case '\t': fprintf(stderr, MSGSTR(INBEAUTIFY,
			" in beautifying, output line %d, on input: %s\n"),
			xxlineno + 1, "\\t"); return;
		case '\n': fprintf(stderr, MSGSTR(INBEAUTIFY,
			" in beautifying, output line %d, on input: %s\n"),
			xxlineno + 1, "\\n"); return;
		case '\0': fprintf(stderr, MSGSTR(INBEAUTIFY,
			" in beautifying, output line %d, on input: %s\n"),
			xxlineno + 1, "$end"); return;
		default: fprintf(stderr, MSGSTR(ONIN,
			" in beautifying, output line %d, on input: %c\n"),
			xxlineno + 1, yychar); return;
			}
	}

yyinit(argc, argv)			/* initialize pushdown store */
int argc;
char *argv[];
	{
	xxindent = 0;
	xxbpertab = 8;
	xxmaxchars = 120;
	}


#include <signal.h>
main()
	{
	int exit(int);

	setlocale(LC_ALL, "");
#ifdef MSG
	catd = catopen(MF_STRUCT,NL_CAT_LOCALE);
#endif
	if ( signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, (void(*)(int)) exit);
	yyinit();
	yyparse();
	}


putout(type,string)			/* output string with proper indentation */
int type;
char *string;
	{
	static int lasttype;
	if ( (lasttype != 0) && (lasttype != '\n') && (lasttype != ' ') && (lasttype != '\t') && (type == xxcom))
		accum("\t");
	else if (lasttype == xxcom && type != '\n')
		tab(xxindent);
	else
		if (lasttype == xxif	||
			lasttype == xxwhile	||
			lasttype == xxdo	||
			type == '='	||
			lasttype == '='	||
			(lasttype == xxident && (type == xxident || type == xxnum) )	||
			(lasttype == xxnum && type == xxnum) )
			accum(" ");
	accum(string);
	lasttype = type;
	}


accum(token)		/* fill output buffer, generate continuation lines */
char *token;
	{
	static char *buffer;
	static int lstatus,llen,bufind;
	int tstatus,tlen,i;

#define NEW	0
#define MID	1
#define CONT	2

	if (buffer == 0)
		{
		buffer = (char *) malloc(xxmaxchars);
		if (buffer == 0) error(MSGSTR(MALLOCSPC, "malloc out of space"),"",""); /*MSG*/
		}
	tlen = slength(token);
	if (tlen == 0) return;
	for (i = 0; i < tlen; ++i)
		if (!(token[i] != '\n' || tlen ==1)) error(MSGSTR(TOKENASSERT, "struct bug: assertion 'token[i] != '\n' || tlen' invalid in routine accum"),"",""); /*MSG*/
	switch(token[tlen-1])
		{
		case '\n':	tstatus = NEW;
				break;
		case '+':
		case '-':
		case '*':
		case ',':
		case '|':
		case '&':
		case '(':	tstatus = CONT;
				break;
		default:	tstatus = MID;
		}
	if (llen + bufind + tlen > xxmaxchars && lstatus == CONT && tstatus != NEW)
		{
		putchar('\n');
		++xxlineno;
		for (i = 0; i < xxindent; ++i)
			putchar('\t');
		putchar(' ');putchar(' ');
		llen = 2 + xxindent * xxbpertab;
		lstatus = NEW;
		}
	if (lstatus == CONT && tstatus == MID)
		{			/* store in buffer in case need \n after last CONT char */
		if (!(bufind + tlen < xxmaxchars)) error(MSGSTR(BUFINDASSERT, "struct bug: assertion 'bufind + tlen < xxmaxchars' invalid in routine accum"),"",""); /*MSG*/
		for (i = 0; i < tlen; ++i)
			buffer[bufind++] = token[i];
		}
	else
		{
		for (i = 0; i < bufind; ++i)
			putchar(buffer[i]);
		llen += bufind;
		bufind = 0;
		for (i = 0; i < tlen; ++i)
			putchar(token[i]);
		if (tstatus == NEW) ++xxlineno;
		llen = (tstatus == NEW) ? 0 : llen + tlen;
		lstatus = tstatus;
		}
	}

tab(n)
int n;
	{
	int i;
	newline();
	for ( i = 0;  i < n; ++i)
		putout('\t',"\t");
	}

newline()
	{
	static int already;
	if (already)
		putout('\n',"\n");
	else
		already = 1;
	}

error(mess1, mess2, mess3)
char *mess1, *mess2, *mess3;
	{
#if defined(KJI) || defined(NLS)
	NLfprintf(stderr,MSGSTR(ERRINBEAUT, "\nerror in beautifying, output line %d: %s %s %s \n"), /*MSG*/
#else KJI || NLS
	fprintf(stderr,MSGSTR(ERRINBEAUT, "\nerror in beautifying, output line %d: %s %s %s \n"), /*MSG*/
#endif KJI || NLS
		xxlineno, mess1, mess2, mess3);
	exit(1);
	}







push(type)
int type;
	{
	if (++xxstind > xxtop)
		error(MSGSTR(NESTDEEP, "nesting too deep, stack overflow"),"",""); /*MSG*/
	xxstack[xxstind] = type;
	}

pop()
	{
	if (xxstind <= 0)
		error(MSGSTR(STACKEXHAUST, "stack exhausted, can't be popped as requested"),"",""); /*MSG*/
	--xxstind;
	}


forst()
	{
	while( (xxval = yylex()) != '\n')
		{
		putout(xxval, yylval);
		free((void *) yylval);
		}
	free((void *) yylval);
	}
