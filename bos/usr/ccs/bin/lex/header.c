static char sccsid[] = "@(#)11  1.10.2.7  src/bos/usr/ccs/bin/lex/header.c, cmdlang, bos41J, 9520A_a 4/10/95 13:42:54";
/**
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: phead1 chd1 rhd1 phead2 chd2 ptail ctail rtail statistics
 *
 * ORIGINS: 27 65 71
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */
/*static char rcsid[] = "RCSfile: header.c,v  Revision: 2.6  (OSF) Date:
 90/10/07 17:42:03 ";*/ 

        /* Multi-byte support added by Michael S. Flegel, July 1991 */
           
# include "ldefs.h"

phead1(){
        ratfor ? rhd1() : chd1();
        }

chd1(){
        fprintf(fout,"# include <stdio.h>\n");
        fprintf(fout,"# include <stddef.h>\n");
        fprintf(fout,"# include <locale.h>\n");
        fprintf(fout,"# include <stdlib.h>\n");

        if (ZCH>128) /*ASSUMES 8-bit bytes */
        fprintf(fout, "# define U(x) ((x)&0377)\n");
        else
        fprintf(fout, "# define U(x) x\n");
        fprintf(fout, "# define NCH %d\n", NCH);
        fprintf(fout, "# define NLSTATE yyprevious=YYNEWLINE\n");
        fprintf(fout,"# define BEGIN yybgin = yysvec + 1 +\n");
        fprintf(fout,"# define INITIAL 0\n");
        fprintf(fout,"# define YYLERR yysvec\n");
        fprintf(fout,"# define YYSTATE (yyestate-yysvec-1)\n");
        if(optim)
                fprintf(fout,"# define YYOPTIM 1\n");
# ifdef DEBUG
        fprintf(fout,"# define LEXDEBUG 1\n");
# endif
        fprintf(fout,"# define YYLMAX 200\n");

        /* support for C++: */
        fprintf(fout,"\n#ifdef __cplusplus\n");

        /* declare some functions */
        fprintf(fout,"int yylook(void);\n");
        fprintf(fout,
            "extern \"C\" int yywrap(void), yyless(int), yyreject(void);\n");
        fprintf(fout,"#endif /* __cplusplus */\n\n");

        /* handle iostreams as well as stdio */
        fprintf(fout,"#if defined(__cplusplus) && defined(_CPP_IOSTREAMS)\n");
        fprintf(fout,"# include <iostream.h>\n");
        fprintf(fout,"# define output(c) (*yyout) << ((unsigned char) c)\n");
        fprintf(fout, "%s%d%s\n",
                "# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):yyin->get())==",
                ctable['\n'],
                "?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)");
        fprintf(fout,"# define ECHO (*yyout) << yytext\n");
        fprintf(fout,"istream *yyin = &cin;\nostream *yyout = &cout;\n");
        fprintf(fout,"#else\n");

        /*
         * 8-bit macros needed, these can be programmer redefined
         */
        fprintf(fout,"# define output(c) putc(c,yyout)\n");
        fprintf(fout, "%s%d%s\n",
                "# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==",
                ctable['\n'],
                "?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)");
        fprintf(fout,"# define ECHO fprintf(yyout, \"%%S\",yywtext)\n");
        fprintf(fout,"FILE *yyin = NULL, *yyout = NULL;\n");
        fprintf(fout,
            "#endif /* defined(__cplusplus) && defined(_CPP_IOSTREAMS) */\n");
        fprintf(fout,
                "# define unput(c) {yytchar= (c);if(yytchar=='\\n')yylineno--;*yysptr++=yytchar;}\n");
        fprintf(fout,"# define yymore() (yymorfg=1)\n");
        fprintf(fout,"# define REJECT { yynstr = yyreject(); goto yyfussy;}\n");
        /*
         * multi-byte macros needed, these can be programmer redefined
         */
        fprintf(fout,"# define yysetlocale() setlocale(LC_ALL,\"\")\n");
        fprintf(fout,"# define wreturn(r) return(yywreturn(r))\n");
        fprintf(fout,"# define winput() yywinput()\n");
        fprintf(fout,"# define wunput(c) yywunput(c)\n");
        fprintf(fout,"# define woutput(c) yywoutput(c)\n");
        /*
         * workspace needed
         */
        fprintf(fout,"int yyleng;\n");
        if(yytext_type)
        {
            fprintf(fout,"extern char *yytext;\n");
        }
        else
        {
            fprintf(fout,"extern char yytext[];\n");
        }
        fprintf(fout,"int yywleng; extern wchar_t yywtext[];\n");
        fprintf(fout,"int yymorfg;\n");
        fprintf(fout,"int yymbcurmax = -1;\n");
        /*defect 39925, GH 10/15/91, making sure setlocale is only called once*/
        fprintf(fout,"int __once_yylex = 1;\n");
        fprintf(fout,"extern unsigned char *yysptr, yysbuf[];\n");
        fprintf(fout,"int yytchar;\n");
        fprintf(fout,"extern int yylineno;\n");
        fprintf(fout,"struct yywork;\n");
        fprintf(fout,"struct yysvf { \n");
        fprintf(fout,"\tstruct yywork *yystoff;\n");
        fprintf(fout,"\tstruct yysvf *yyother;\n");
        fprintf(fout,"\tint *yystops;};\n");
        fprintf(fout,"struct yysvf *yyestate;\n");
        fprintf(fout,"extern struct yysvf yysvec[], *yybgin;\n");
        }

/* cfollow added for defect 69866 */
cfollow(){
        if(yytext_type)
        {
            fprintf(fout,"char yytmp[YYLMAX];\n");
            fprintf(fout,"char *yytext = yytmp;\n");
        }
        else
        {
            fprintf(fout,"char yytext[YYLMAX];\n");
        }
        fprintf(fout,"int yyback(int *yyp, int yym);\n");
        fprintf(fout,"# if YYHSIZE\n");
        fprintf(fout,"int yyhlook(int yyc, int yyv);\n");
        fprintf(fout,"int yymlook(int yyc);\n");
        fprintf(fout,"# endif /*YYHSIZE*/\n");
        fprintf(fout,"# if YYXSIZE\n");
        fprintf(fout,"int yyxlook (int yyc, int yyv);\n");
        fprintf(fout,"#endif /*YYXSIZE*/\n");
        fprintf(fout,"int yywinput();\n");
        fprintf(fout,"void yywoutput(int yyc);\n");
        fprintf(fout,"void yywunput(int yyc);\n");
        fprintf(fout,"int yywreturn(int yyr);\n");
        fprintf(fout,"int yyinput();\n");
        fprintf(fout,"void yyoutput(int yyc);\n");
        fprintf(fout,"void yyunput(int yyc);\n");
	/*defect 172434, lex interface routines need c external names*/
	fprintf(fout,"#ifdef __cplusplus\n");
	fprintf(fout,"extern \"C\" {\n");
	fprintf(fout,"#endif /* __cplusplus */\n");
        fprintf(fout,"int yymbinput();\n");
        fprintf(fout,"void yymboutput(int yyc);\n");
        fprintf(fout,"void yymbunput(int yyc);\n");
        fprintf(fout,"int yymbreturn(int yyx);\n");
	fprintf(fout,"#ifdef __cplusplus\n");
	fprintf(fout,"}\n");
	fprintf(fout,"#endif /* __cplusplus */\n");
        }

rhd1(){
        fprintf(fout,"integer function yylex(dummy)\n");
        fprintf(fout,"define YYLMAX 200\n");
        fprintf(fout,"define ECHO call yyecho(yytext,yyleng)\n");
        fprintf(fout,"define REJECT nstr = yyrjct(yytext,yyleng);goto 30998\n");
        fprintf(fout,"integer nstr,yylook,yywrap\n");
        fprintf(fout,"integer yyleng, yytext(YYLMAX)\n");
        fprintf(fout,"common /yyxel/ yyleng, yytext\n");
        fprintf(fout,"common /yyldat/ yyfnd, yymorf, yyprev, yybgin, yylsp, yylsta\n");
        fprintf(fout,"integer yyfnd, yymorf, yyprev, yybgin, yylsp, yylsta(YYLMAX)\n");
        fprintf(fout,"for(;;){\n");
        fprintf(fout,"\t30999 nstr = yylook(dummy)\n");
        fprintf(fout,"\tgoto 30998\n");
        fprintf(fout,"\t30000 k = yywrap(dummy)\n");
        fprintf(fout,"\tif(k .ne. 0){\n");
        fprintf(fout,"\tyylex=0; return; }\n");
        fprintf(fout,"\t\telse goto 30998\n");
        }

phead2(){
        if(!ratfor)chd2();
        }

chd2()
{
    fprintf(fout,"if (__once_yylex) {\n");
    fprintf(fout,"      yysetlocale();\n");
    fprintf(fout,"#if !(__cplusplus && _CPP_IOSTREAMS)\n");
    fprintf(fout,"      if (yyin == NULL) yyin = stdin;\n");
    fprintf(fout,"      if (yyout == NULL) yyout = stdout;\n");
    fprintf(fout,"#endif /* !(__cplusplus && _CPP_IOSTREAMS) */\n");
    fprintf(fout,"      __once_yylex = 0; }\n");
    fprintf(fout,"if(yymbcurmax<=0) yymbcurmax=MB_CUR_MAX;\n");
    fprintf(fout,"while((yynstr = yylook()) >= 0)\n");
    fprintf(fout,"yyfussy: switch(yynstr){\n");
    fprintf(fout,"case 0:\n");
    fprintf(fout,"if(yywrap()) return(0); break;\n");
}

ptail(){
        if(!pflag)
                ratfor ? rtail() : ctail();
        pflag = 1;
        }

ctail(){
        fprintf(fout,"case -1:\nbreak;\n");             /* for reject */
        fprintf(fout,"default:\n");
        fprintf(fout,"#if defined(__cplusplus) && defined(_CPP_IOSTREAMS)\n");
        fprintf(fout,"(*yyout) << \"bad switch yylook \" << yynstr;\n");
        fprintf(fout,"#else\n");
        fprintf(fout,"fprintf(yyout,\"bad switch yylook %%d\",yynstr);\n");
        fprintf(fout, "#endif /* defined(__cplusplus) && defined(_CPP_IOSTREAMS)*/\n");
        fprintf(fout,"} return(0); }\n");
        fprintf(fout,"/* end of yylex */\n");
        }

rtail(){
        register int i;
        fprintf(fout,"\n30998 if(nstr .lt. 0 .or. nstr .gt. %d)goto 30999\n",casecount);
        fprintf(fout,"nstr = nstr + 1\n");
        fprintf(fout,"goto(\n");
        for(i=0; i<casecount; i++)
                fprintf(fout,"%d,\n",30000+i);
        fprintf(fout,"30999),nstr\n");
        fprintf(fout,"30997 continue\n");
        fprintf(fout,"}\nend\n");
        }
statistics()
{
int     v;

    v = 0;
    fprintf(errorf,
            MSGSTR(STATISTIC,"%d/%d nodes(%%e), %d/%d positions(%%p), %d/%d (%%n), %ld transitions\n"),
            tptr, treesize, nxtpos-positions, maxpos, stnum+1, nstates, rcount);
    fprintf(errorf,
            MSGSTR(STATISTIC2,"%d/%d packed char classes(%%k)\n"),
            pcptr-pchar, pchlen);
    if(optim)
        fprintf(errorf,
                MSGSTR(STATISTIC3, "%d/%d packed transitions(%%a)\n"),
                nptr, ntrans);
    fprintf(errorf,
            MSGSTR(STATISTIC4, "%d/%d output slots(%%o)\n"),
            yytop, outsize);
    if (xccl && xccltop)
        fprintf (errorf,
                 MSGSTR (STATISTIC6, "%d/%d special character class output slots(%%z)\n"),
                 xccltop, xcclsize);
    if (wcrank && wcrank->top)
    {
        v = 1;
        fprintf(errorf,
                MSGSTR(STATISTIC5, "%d/%d multi-byte output slots(%%h)\n"),
                wcrank->top, wcranksize);
    }
    if (wmatch && wmatch->top)
    {
        v = 1;
        fprintf(errorf,
                MSGSTR(STATISTIC7, "%d/%d multi-byte character class character slots (%%m)\n"),
                wmatch->top, wmatchsize);
    }
    if (v)
        fprintf(errorf,MSGSTR(STATISTIC8,"%d%% hash table vacancy(%%v)\n"), whspace);

    putc('\n',errorf);
}
