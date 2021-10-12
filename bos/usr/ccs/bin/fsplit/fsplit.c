static char sccsid[] = "@(#)59  1.11  src/bos/usr/ccs/bin/fsplit/fsplit.c, cmdprog, bos411, 9428A410j 4/14/94 13:18:26";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: main, badparms, functs, get_name, getline, lend, lname, look,
              saveit, scan_name, skiplab
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*              include file for message texts          */
#define _ILS_MACROS    /* 139729 - use macros for better performance */
#include <sys/localedef.h>
#ifdef KJI
#include <NLctype.h>
#else
#include <ctype.h>
#endif
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>
#include <nl_types.h>
#ifdef MSG
#include "fsplit_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
#define MSGSTR(N,S) catgets(scmc_catd,MS_fsplit,N,S)
#else
#define MSGSTR(N,S) S
#endif

/*
 *      usage:          fsplit [-e efile] ... [file]
 *
 *      split single file containing source for several fortran programs
 *              and/or subprograms into files each containing one
 *              subprogram unit.
 *      each separate file will be named using the corresponding subroutine,
 *              function, block data or program name if one is found; otherwise
 *              the name will be of the form mainNNN.f or blkdtaNNN.f .
 *              If a file of that name exists, it is saved in a name of the
 *              form zzz000.f .
 *      If -e option is used, then only those subprograms named in the -e
 *              option are split off; e.g.:
 *                      fsplit -esub1 -e sub2 prog.f
 *              isolates sub1 and sub2 in sub1.f and sub2.f.  The space 
 *              after -e is optional.
 *
 *      Modified Feb., 1983 by Jerry Berkman, Computing Services, U.C. Berkeley.
 *              - added comments
 *              - more function types: double complex, character*(*), etc.
 *              - fixed minor bugs
 *              - instead of all unnamed going into zNNN.f, put mains in
 *                mainNNN.f, block datas in blkdtaNNN.f, dups in zzzNNN.f .
 */

#define BSZ 512
char buf[BSZ];
FILE *ifp;
char    x[]="zzz000.f",
        mainp[]="main000.f",
        blkp[]="blkdta000.f";
char *look(), *skiplab(), *functs();

#define TRUE 1
#define FALSE 0
int     extr = FALSE,
        extrknt = -1,
        extrfnd[100];
char    extrbuf[1000],
        *extrnames[100];
struct stat sbuf;

#define trim(p) while (*p == ' ' || *p == '\t') p++

main(argc, argv)
char **argv;
{
        register FILE *ofp;     /* output file */
        register rv;            /* 1 if got card in output file, 0 otherwise */
        register char *ptr;
        int nflag,              /* 1 if got name of subprog., 0 otherwise */
                retval,
                i;
        char name[20],
                *extrptr = extrbuf;
/* 19938 GH setlocale was called after catopen */
        setlocale(LC_ALL, "");
#ifdef MSG
        scmc_catd = catopen(MF_FSPLIT, NL_CAT_LOCALE);
#endif
        /*  scan -e options */
        while ( argc > 1  && argv[1][0] == '-' && argv[1][1] == 'e') {
                extr = TRUE;
                ptr = argv[1] + 2;
                if(!*ptr) {
                        argc--;
                        argv++;
                        if(argc <= 1) badparms();
                        ptr = argv[1];
                }
                extrknt = extrknt + 1;
                extrnames[extrknt] = extrptr;
                extrfnd[extrknt] = FALSE;
                while(*ptr) *extrptr++ = *ptr++;
                *extrptr++ = 0;
                argc--;
                argv++;
        }

        if (argc > 2)
                badparms();
        else if (argc == 2) {
                if ((ifp = fopen(argv[1], "r")) == NULL) {
                        fprintf(stderr,  MSGSTR(M_MSG_3, "fsplit: cannot open %s\n") , argv[1]);
                        exit(1);
                }
        }
        else
                ifp = stdin;
    for(;;) {
        /* look for a temp file that doesn't correspond to an existing file */
        get_name(x, 3);
        ofp = fopen(x, "w");
        nflag = 0;
        rv = 0;
        while (getline() > 0) {
                rv = 1;
                fprintf(ofp, "%s", buf);
                if (lend())             /* look for an 'end' statement */
                        break;
                if (nflag == 0)         /* if no name yet, try and find one */
                        nflag = lname(name);
        }
        fclose(ofp);
        if (rv == 0) {                  /* no lines in file, forget the file */
                unlink(x);
                retval = 0;
                for ( i = 0; i <= extrknt; i++ )
                        if(!extrfnd[i]) {
                                retval = 1;
                                fprintf( stderr,  MSGSTR(M_MSG_5, "fsplit: %s not found\n") , extrnames[i]);
                        }
                exit( retval );
        }
        if (nflag) {                    /* rename the file */
                if(saveit(name)) {
                        if (stat(name, &sbuf) < 0 ) {
                                link(x, name);
                                unlink(x);
                                printf("%s\n", name);
                                continue;
                        } else if (strcmp(name, x) == 0) {
                                printf("%s\n", x);
                                continue;
                        }
                        printf( MSGSTR(M_MSG_8, "%s already exists, put in %s\n") , name, x);
                        continue;
                } else
                        unlink(x);
                        continue;
        }
        if(!extr)
                printf("%s\n", x);
        else
                unlink(x);
    }
}

badparms()
{
        fprintf(stderr,  MSGSTR(M_MSG_10, "fsplit: usage:  fsplit [-e efile] ... [file] \n") );
        exit(1);
}

saveit(name)
char *name;
{
        int i;
        char    fname[50],
                *fptr = fname;

        if(!extr) return(1);
        while(*name) *fptr++ = *name++;
        *--fptr = 0;
        *--fptr = 0;
        for ( i=0 ; i<=extrknt; i++ ) 
                if( strcmp(fname, extrnames[i]) == 0 ) {
                        extrfnd[i] = TRUE;
                        return(1);
                }
        return(0);
}

get_name(name, letters)
char *name;
int letters;
{
        register char *ptr;

        while (stat(name, &sbuf) >= 0) {
                for (ptr = name + letters + 2; ptr >= name + letters; ptr--) {
                        (*ptr)++;
                        if (*ptr <= '9')
                                break;
                        *ptr = '0';
                }
                if(ptr < name + letters) {
                        fprintf( stderr,  MSGSTR(M_MSG_11, "fsplit: ran out of file names\n") );
                        exit(1);
                }
        }
}

getline()
{
        register char *ptr;

        for (ptr = buf; ptr < &buf[BSZ]; ) {
                *ptr = getc(ifp);
                if (feof(ifp))
                        return (-1);
                if (*ptr++ == '\n') {
                        *ptr = 0;
                        return (1);
                }
        }
        while (getc(ifp) != '\n' && feof(ifp) == 0) ;
        fprintf(stderr,  MSGSTR(M_MSG_12, "line truncated to %d characters\n") , BSZ);
        return (1);
}

/* return 1 for 'end' alone on card (up to col. 72),  0 otherwise */
lend()
{
        register char *p;

        if ((p = skiplab(buf)) == 0)
                return (0);
        trim(p);
        if (*p != 'e' && *p != 'E') return(0);
        p++;
        trim(p);
        if (*p != 'n' && *p != 'N') return(0);
        p++;
        trim(p);
        if (*p != 'd' && *p != 'D') return(0);
        p++;
        trim(p);
        if (p - buf >= 72 || *p == '\n' || *p == '!')
        /* Check for end of line, or a comment following end */
                return (1);
        return (0);
}

/*              check for keywords for subprograms      
                return 0 if comment card, 1 if found
                name and put in arg string. invent name for unnamed
                block datas and main programs.          */
lname(s)
char *s;
{
#       define LINESIZE 80 
        register char *ptr, *p, *sptr;
        char    line[LINESIZE], *iptr = line;

        /* first check for comment cards */
        if(buf[0] == 'c' || buf[0] == 'C' || buf[0] == '*' || buf[0] == '!') return(0);
        ptr = buf;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        if(*ptr == '\n' || *ptr == '!') return(0);


        ptr = skiplab(buf);

        /*  copy to buffer and converting to lower case */
        p = ptr;
        while (*p && p <= &buf[71] ) {
           *iptr = isupper(*p) ? tolower(*p) : *p;
           iptr++;
           p++;
        }
        *iptr = '\n';

        if ((ptr = look(line, "subroutine")) != 0 ||
            (ptr = look(line, "function")) != 0 ||
            (ptr = functs(line)) != 0) {
                if(scan_name(s, ptr)) return(1);
                strcpy( s, x);
        } else if((ptr = look(line, "program")) != 0) {
                if(scan_name(s, ptr)) return(1);
                get_name( mainp, 4);
                strcpy( s, mainp);
        } else if((ptr = look(line, "blockdata")) != 0) {
                if(scan_name(s, ptr)) return(1);
                get_name( blkp, 6);
                strcpy( s, blkp);
        } else if((ptr = functs(line)) != 0) {
                if(scan_name(s, ptr)) return(1);
                strcpy( s, x);
        } else {
                get_name( mainp, 4);
                strcpy( s, mainp);
        }
        return(1);
}

                
scan_name(s, ptr)
char *s, *ptr;
{
        char *sptr;

        /* scan off the name */
        trim(ptr);
        sptr = s;
        while (*ptr != '(' && *ptr != '\n' && *ptr != '*' && *ptr != '!') {
        /* Check for end of the name of the function, could be
           due to parameter declaration '(', end of line '\n',
           length of type declaration '*', or beginning of comment '!'
         */
                if (*ptr != ' ' && *ptr != '\t')
                        *sptr++ = *ptr;
                ptr++;
        }

        if (sptr == s) return(0);

        *sptr++ = '.';
        *sptr++ = 'f';
        *sptr++ = 0;
        return(1);
}

char *functs(p)
char *p;
{
        register char *ptr;

/*      look for typed functions such as: real*8 function,
                character*16 function, character*(*) function  */

        if((ptr = look(p,"character")) != 0 ||
           (ptr = look(p,"logical")) != 0 ||
           (ptr = look(p,"real")) != 0 ||
           (ptr = look(p,"integer")) != 0 ||
           (ptr = look(p,"doubleprecision")) != 0 ||
           (ptr = look(p,"complex")) != 0 ||
           (ptr = look(p,"doublecomplex")) != 0 ) {
                while ( *ptr == ' ' || *ptr == '\t' || *ptr == '*'
                        || (*ptr >= '0' && *ptr <= '9')
                        || *ptr == '(' || *ptr == ')') ptr++;
                ptr = look(ptr,"function");
                return(ptr);
        }
        else
                return(0);
}

/*      if first 6 col. blank, return ptr to col. 7,
        if blanks and then tab, return ptr after tab,
        else return 0 (labelled statement, comment or continuation */
/*      D58464 - fsplit does not split code if label proceeds "END" statement.
	   Check for labelled statement before deciding to return. */
char *skiplab(p)
char *p;
{
        register char *ptr;

        for (ptr = p; ptr < &p[6]; ptr++) {
                if (*ptr == ' ')
                        continue;
                if ((*ptr >= '0') && (*ptr <= '9') && (ptr < &p[5]))
                        continue; /*D58464 - skip over numeric labels.*/
                if (*ptr == '\t') {
                        ptr++;
                        break;
                }
                return (0);
        }
        return (ptr);
}

/*      return 0 if m doesn't match initial part of s;
        otherwise return ptr to next char after m in s */
char *look(s, m)
char *s, *m;
{
        register char *sp, *mp;

        sp = s; mp = m;
        while (*mp) {
                trim(sp);
                if (*sp++ != *mp++)
                        return (0);
        }
        return (sp);
}
