static char sccsid[] = "@(#)37  1.7  src/bos/usr/ccs/bin/regcmp/regcmp.c, cmdprog, bos411, 9428A410j 3/9/94 13:06:45";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: main, getnm, size
 *
 * ORIGINS: 3 10 27
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

#include <stdio.h>
#include <locale.h>
#include <nl_types.h>
#include "regcmp_msg.h"
nl_catd catd;
#define MSGSTR(n,s)     catgets(catd,MS_REGCMP,n,s)
FILE *iobuf;
int gotflg;
extern int __i_size;
char ofile[64];
char a1[1024];
char a2[64];
int c;

main(argc,argv) char **argv;
{
        register char *name, *str, *v;
        extern char *regcmp();
        char *bp, *cp, *sv;
        int j,k,cflg;

        setlocale(LC_ALL, "");
        catd = catopen(MF_REGCMP, NL_CAT_LOCALE);
        if (*argv[1] == '-') {
                cflg++;
                ++argv;
                argc--;
        }
        else cflg = 0;
        while(--argc) {
                ++argv;
                bp = *argv;
                if ((iobuf=fopen(*argv,"r")) == NULL) {
                     fprintf(stderr, MSGSTR(NOTOPN, "can not open %s\n"), *argv);
                     continue;
                }
                cp = ofile;
                while(*++bp)
                        if(*bp == '/') *bp = '\0';
                while(*--bp == '\0');
                while(*bp != '\0' && bp > *argv) bp--;
                while (*bp == 0)
                        bp++;
                while(*cp++ = *bp++);
                cp--; *cp++ = '.';
                if(cflg) *cp++ = 'c';
                else *cp++ = 'i';
                *cp = '\0';
                close(1);
                if (creat(ofile,0644)<0)  {
                     fprintf(stderr, MSGSTR(NOCREAT, "can not create .i file\n"));
                     exit(1);
                }
                gotflg = 0;
           while(1) {
                str = a1;
                name = a2;
                if (!gotflg)
                        while(((c=getc(iobuf)) == '\n') || (c == ' ') ||
                                (c == '\t'));
                else
                        gotflg = 0;
                if(c==EOF) break;
                *name++ = c;
                while(((*name++ = c = getc(iobuf)) != ' ') && (c != EOF) &&
                        (c != '\n') && (c != '\t'));
                *--name = '\0';
                while(((c=getc(iobuf)) == ' ') || (c == '\n') || (c == '\t'));
                if(c != '"') {
                     if (c==EOF) {
                        fprintf(stderr, MSGSTR(UNEXEOF, "unexpected eof\n"));
                        exit(1);
                     }
                     fprintf(stderr, MSGSTR(INITQUOT, "missing initial quote for %s"), a2);
                     fprintf(stderr, MSGSTR(REMIGN, " : remainder of line ignored\n"));
                     while((c=getc(iobuf)) != '\n');
                     continue;
                }
        keeponl:
                while(gotflg || (c=getc(iobuf)) != EOF) {
                gotflg = 0;
                switch(c) {
                case '"':
                        break;
                case '\\':
                        switch(c=getc(iobuf)) {
                        case 't':
                                *str++ = '\011';
                                continue;
                        case 'n':
                                *str++ = '\012';
                                continue;
                        case 'r':
                                *str++ = '\015';
                                continue;
                        case 'b':
                                *str++ = '\010';
                                continue;
                        case '\\':
                                *str++ = '\\';
                                continue;
                        default:
                                if (c<='7' && c>='0') 
                                                *str++ = getnm(c);
                                else *str++ = c;
                                continue;
                        }
                default:
                        *str++ = c;
                }
                if (c=='"') break;
                }
                if (c==EOF) {
                     fprintf(stderr, MSGSTR(UNEXEOF, "unexpected eof\n"));
                     exit(1);
                }
                while(((c=getc(iobuf)) == '\n') || (c == ' ') || (c == '\t'));
                if (c=='"') goto keeponl;
                else {
                        gotflg++;
                }
                *str = '\0';
                if(!(sv=v=regcmp(a1,0))) {
                        fprintf(stderr, MSGSTR(FAIL, "fail: %s\n"), a2);
                        continue;
                }
                printf("/* \"%s\" */\n",a1);
                printf("char %s[] = {\n",a2);
                while(__i_size > 0) {
                        for(k=0;k<12;k++)
                                if(__i_size-- > 0) printf("0%o,",*v++);
                        printf("\n");
                }
                printf("0};\n");
                free(sv);
           }
           fclose(iobuf);
           fflush(stdout);
        }
        exit(0);
}
size(p) char *p;
{
        register i;
        register char *q;

        i = 0;
        q = p;
        while(*q++) i++;
        return(i);
}
getnm(j) char j;
{
        register int i;
        register int k;
        i = j - '0';
        k = 1;
        while( ++k < 4 && (c=getc(iobuf)) >= '0' && c <= '7') 
                i = (i*8+(c-'0'));
        if (k >= 4)
                c = getc(iobuf);
        gotflg++;
        return(i);
}
