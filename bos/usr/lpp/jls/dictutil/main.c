static char sccsid[] = "@(#)81	1.4.1.1  src/bos/usr/lpp/jls/dictutil/main.c, cmdKJI, bos411, 9428A410j 7/23/92 01:41:13";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1992
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:    OCO Source Material - IBM Confidential.
 *                    (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:
 *
 * NOTES:               NA.
 *
 */

/*----------------------------------------------------------------------*
 * Include Standard.
 *----------------------------------------------------------------------*/
#if defined(CNVSTOC)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#if defined(CNVEVT)
#include <iconv.h>
#endif
#include <locale.h>
#include <nl_types.h>
#include <limits.h>
#include <errno.h>

/*----------------------------------------------------------------------*
 * Include Kanji Project.
 *----------------------------------------------------------------------*/
#include "kje.h"

#define PR(XX)  XX;

#if defined(PARMADD)
char    *printdv = NULL;        /* Printer Device Pointer               */
#endif
#if defined(CNVEVT)
iconv_t icnvfd;                 /* Input Conversion File Discripter     */
iconv_t ocnvfd;                 /* Output Conversion File Discripter    */
int     cnvflg;                 /* Conversion Flag                      */
#endif

main( argc, argv_s )

int     argc;           /* Argument Number                          (i) */
char    *argv_s[];      /* Argument String                          (i) */
{

    struct  stat    sbuf;       /* Stat Structure Definition            */
    char    **argv;             /* Argment List                         */
    char    *term = NULL;       /* Display Type                         */
    char    *home = NULL;       /* HOME Directory                       */
    char    *ptr  = NULL;       /* Getenv Work Pointer                  */
    char    *lang = NULL;       /* Langage Type                         */
#if defined(CNVEVT)
    char    *euc  = NULL;       /* EUC Conversion Name Pointer          */
    char    *sjis = NULL;       /* Shift JIS Conversion Name Pointer    */
#endif
    char    msgfcl[U_MEMSIZ];   /* Message Facility Name                */
    char    usrdic[U_MEMSIZ];   /* User Dictionar Name                  */
    char    udir[U_MEMSIZ];     /* User Dictionary Directory Name       */
    char    ubas[U_MEMSIZ];     /* User Dictionary Base Name            */
    char    ubak[U_MEMSIZ];     /* User Dictionary Backup Name          */
    char    cpdic[U_MEMSIZ];    /* User Dictionary Copy Directory Data  */
    char    cpbak[U_MEMSIZ];    /* User Dictionary Copy Backup Data     */
    char    rmfile[U_MEMSIZ];   /* User Dictionary Remove File Data     */
    char    *parm[U_PARM];      /* User Dictionary Default Parameter    */
    char    parm1[U_MEMSIZ];    /* kudichnd Parameter-1 Area Pointer    */
    char    parm2[U_MEMSIZ];    /* kudichnd Parameter-2 Area Pointer    */
    char    parm3[U_MEMSIZ];    /* kudichnd Parameter-3 Area Pointer    */
#if defined(PARMADD)
    char    prtdev[U_MEMSIZ];   /* Printer Device                       */
    int     dpos = 0;           /* User Dictionary Name Position        */
    int     ppos = 0;           /* Printer Device Neme Position         */
#endif
#if defined(TMPDIR)
    char    chmod[U_MEMSIZ];    /* Change Mode Name Area                */
#endif
    int     len;                /* Argment String Length                */
    int     i;                  /* Loop Counter                         */
    int     irc = 0;            /* Information Return Value             */
    int     rc  = 0;            /* Return Value                         */
    char    *locale;	
    int	    exit_code = 0;

    locale = setlocale(LC_ALL, "");
    if (!strncmp(U_SJISLG, locale, 5)) {
	cnvflg = U_SJIS;			/* set pc932 mode 	*/
    } else if (!strncmp(U_EUCLG, locale, 5)) {
	cnvflg = U_EUC;				/* set euc mode 	*/
    } else {
	printf("This program can not run when LANG=%s\n", getenv(U_LANG));
	printf("LANG must be Ja_JP or ja_JP\n");
	exit (1);
    }

    kjinit(cnvflg);

    /*----- libcur: extended mode off ----------------------------------*/
    extended(FALSE);

    do {
	argv = (char **)malloc(sizeof(char *)*argc);
	if ( argv == (char **)NULL ) {
	    (void)printf(CU_MSGMAE);
	    exit_code = 1;
	    break;
	}

	for ( i = 0; i < argc; i++ ) {
	    len = strlen(argv_s[i]);
	    argv[i] = (char *)malloc(sizeof(char)*(len+1));
	    if ( argv[i] == (char *)NULL ) {
		(void)printf(CU_MSGMAE);
		break;
	    }
	    strcpy(argv[i],argv_s[i]);
	}
	if ( argc < i ) {
	    exit_code = 1;
	    break;
	}

	/*--------------------------------------------------------------*
	 * Initialized Data Clear
	 *--------------------------------------------------------------*/
	memset(msgfcl, '\0', U_MEMSIZ);
	memset(usrdic, '\0', U_MEMSIZ);
	memset(udir,   '\0', U_MEMSIZ);
	memset(ubas,   '\0', U_MEMSIZ);
	memset(ubak,   '\0', U_MEMSIZ);
	memset(cpdic,  '\0', U_MEMSIZ);
	memset(cpbak,  '\0', U_MEMSIZ);
	memset(rmfile, '\0', U_MEMSIZ);

	lang = getenv(U_LANG);          /* Get Langage Type 		*/

	if ( lang == NULL ) {
	    printf(U_MSGLNG);       	/* direct print 		*/
	    exit_code = 1;
	    break;
	}
   
	ptr = strrchr(lang,'.');
	if ( ptr == NULL ) {
		ptr = lang;
	} else {
		ptr++;
	}
  
#if defined(CNVEVT)
	euc = lang;
	if ( euc == NULL ) {
	    printf(U_MSGCLN);      /* direct print */
	    exit_code = 1;
	    break;
        }

        sjis = U_SJISLG;

	/*
         * if ( strcmp(euc,sjis) == 0 ) {
         *       cnvflg = U_SJIS;
         * } else if ( strcmp(euc,U_EUCLG) == 0 ) {
         *       cnvflg = U_EUC;
         * } else {
         *       cnvflg = U_DEF;
	 *       printf("LANG must be Ja_JP or ja_JP   stop");
	 *       exit_code = 1;
         *       break;	
         * }
         */
#endif
	(void)printf(CU_MSGTIL);
	(void)printf(CU_MSGCPL);

	term = getenv(U_TERM);
	if ( term == NULL ) {
	    (void)printf(CU_MSGTRE);
	    exit_code = 1;
	    break;
	}
	if ( (strcmp(term,U_TRJAIX) != 0) &&
	     (strcmp(term,U_TRAIX) != 0) &&
	     (strcmp(term,U_TRAIXM)  != 0) ) {
	    (void)printf(CU_MSGIVT);
	    exit_code = 1;
	    break;
	}

	ptr = getenv(U_JIMUSR);
	if ( ptr != NULL && strlen(ptr) > 0 ) {
	    strcpy(usrdic,ptr);
	    errno = 0;
	    stat(usrdic,&sbuf);
	    if ( errno != 0 ) {
	        (void)printf(CU_MSGNUD, usrdic);
	    	exit_code = 1;
		break;
	    }
	} else {
	    home = getenv(U_HOME);
	    if ( home == NULL ) {
		(void)printf(CU_MSGHME);
	    	exit_code = 1;
		break;
	    }
	    if (home[strlen(home)-1] == '/') {
			sprintf(usrdic,"%s%s",home,U_CURDIC);
	    } else {
			sprintf(usrdic,"%s%s%s",home,U_SLASH,U_CURDIC);
	    }
	    errno = 0;
	    stat(usrdic,&sbuf);
	    if ( errno != 0 ) {
			strcpy(usrdic,U_STDUDC);
	    }
	}
	errno = 0;

#if defined(PARMADD)
	memset(prtdev,'\0',U_MEMSIZ);
	ptr = getenv(U_PRTDEV);
	if ( ptr != NULL && strlen(ptr) > 0 ) {
	    sprintf(prtdev,"%s %s",U_PRMPDV,argv[ppos]);
	}

	irc = kjprmchk( argc, argv, &dpos, &ppos );    

	if ( (irc & ~(U_PRMADD | U_INUDCT | U_INPRDV)) != 0 ) {
	    exit_code = 1;
	    break;
	}

	if ( (irc & U_INUDCT) == U_INUDCT ) {
	    (void)kjinput(usrdic, CU_MSGINU);
	} else if ( dpos > 0 ) {
	    strcpy(usrdic,argv[dpos]);
	} else if ( (dpos & ~0x80000000) > 0 ) {
	    ptr = (argv[(dpos&~0x80000000)]+2);
	    strcpy(usrdic,ptr);
	}

	if ( (irc & U_INPRDV) == U_INPRDV ) {
	    sprintf(prtdev,"%s  ",U_PRMPDV);
	    (void)kjinput(&prtdev[3], CU_MSGINP);
	} else if ( ppos > 0 ) {
	    sprintf(prtdev,"%s %s",U_PRMPDV,argv[ppos]);
	} else if ( (ppos & ~0x80000000) > 0 ) {
	    ptr = (argv[(ppos&~0x80000000)]+2);
	    sprintf(prtdev,"%s %s",U_PRMPDV,ptr);
	}

	rc = kjflchk( usrdic, udir, ubas, ubak );

#else
	if ( argc == 1 ) {
	    rc = kjflchk( usrdic, udir, ubas, ubak );
	} else if ( argc == 2 ) {
	    if ( strcmp(argv[1],U_PRMJSY) == 0 ) {
		(void)kjinput(usrdic, CU_MSGINU);
		rc = kjflchk( usrdic, udir, ubas, ubak );
	    } else {
		(void)printf(CU_MSGIV1, argv[1]);
	    	exit_code = 1;
		break;
	    }
	} else if ( argc == 3 ) {
	    if ( strcmp(argv[1],U_PRMJSY) == 0 ) {
		if ( strcmp(argv[2],U_PRMJSY) == 0 ) {
		    (void)printf(CU_MSGIV1, argv[2]);
	    	    exit_code = 1;
		    break;
		} else {
		    strcpy(usrdic,argv[2]);
		    rc = kjflchk( usrdic, udir, ubas, ubak );
		}
	    } else {
		(void)printf(CU_MSGIV2, argv[1], argv[2]);
	    	exit_code = 1;
		break;  
	    }
	} else {
	    (void)printf(CU_MSGIV0);
	    exit_code = 1;
	    break;
	}
#endif
	if ( rc != 0 ) {
	    exit_code = 1;
	    break;
	}

	parm[0] = parm1;
	parm[1] = parm2;
	parm[2] = parm3;
	memset(parm[0],'\0',U_MEMSIZ);
	memset(parm[1],'\0',U_MEMSIZ);
	memset(parm[2],'\0',U_MEMSIZ);
	strcpy(parm[1],usrdic);
	strcpy(parm[2],U_DEFDEF);

#if defined(PARMADD)
	if ( strlen(prtdev) > 0 ) {
	    printdv = prtdev;
	}

	if ( (irc & U_PRMADD) == U_PRMADD ) {
	    rc = kjdicadd( argc, argv, usrdic, parm[2] ); 
	} else {
	    rc = kudichnd( 3, parm );
	}
#else
	rc = kudichnd( 3, parm );
#endif

	if ( rc == 1 )
	    (void)printf(CU_MSGUPD, usrdic);
	else if ( rc < 0 ) {
	    exit_code = 1;
	}

	for ( i = 0; i < argc; i++ ) {
		if ( argv[i] != NULL ) {
			(void)free(argv[i]);
		}
	}
	(void)free(argv);

    } while( 0 );

    exit( exit_code );
}
#endif
