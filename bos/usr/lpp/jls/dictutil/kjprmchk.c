static char sccsid[] = "@(#)79  1.4  src/bos/usr/lpp/jls/dictutil/kjprmchk.c, cmdKJI, bos411, 9428A410j 10/11/91 11:42:40";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * COPYRIGHT:           5621-012 COPYRIGHT IBM CORP 1991
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


#if defined(PARMADD)
#include <stdio.h>

#include "kje.h"

#if defined(CNVEVT)
extern  int     cnvflg;         /* Conversion Type Code                 */
#endif

#define PR(XX)  XX;

int kjprmchk( argc, argv, dpos, ppos )

int     argc;           /* Input Parameter Count                    (i) */
char    *argv[];        /* input Parameter Data String              (i) */
int     *dpos;          /* User Dictionary Name Position            (o) */
int     *ppos;          /* Printer Device Name Postion              (o) */
{
char    *curptr;                /* Current Data Pointer                 */
char    *nxtptr;                /* Next Data Pointer                    */
char    *ch;                    /* Data String Pointer                  */
char    *ch_1;                  /* test use */

#if defined(CNVEVT)
char    cnvdt[U_MEMSIZ];        /* Conversion Output Data Area          */
size_t     ilen;                   /* Conversion Input Length              */
size_t     olen;                   /* Conversion Output Buffer Length      */
size_t     cnvlen;                 /* Conversion Output Length             */
#endif
int     dcnt = 0;               /* User Dictionary Define Parm. Counter */
int     pcnt = 0;               /* Printer Device Define Parm. Dounter  */
int     len;                    /* Data Length                          */
int     i;                      /* Loop Counter                         */
int     j;                      /* Loop Counter                         */
int     dflag = U_CLEARF;       /* User Dictionary Control Flag         */
int     flag  = U_CLEARF;       /* Yomi,Goku Control Flag               */
int     pflag = U_CLEARF;       /* Printer Queue Control Flag           */
int     errmsg;                 /* Error Message ID Area                */
int     rc = U_NOMSKD;          /* Return Value                         */

do {
    *dpos =
    *ppos = 0;
    for ( i = 1, nxtptr = NULL; i < argc; i++, nxtptr = NULL ) {

/*
#if defined(CNVEVT)
	if ( cnvflg == U_EUC ) {
	    ilen = strlen(argv[i]);
	    olen = U_MEMSIZ;
	ch_1 = argv[i];

	    (void)kjcnvets( argv[i], &ilen, cnvdt, &olen );

	    cnvlen = U_MEMSIZ - olen;  

	ch_1 = cnvdt;
	    strncpy(argv[i],cnvdt,cnvlen);
	    argv[i][cnvlen] = '\0';
	};
#endif
*/
	len = strlen(argv[i]);
	curptr = argv[i];
	if ( strncmp(curptr,U_PRMJSY,2) == 0 ) {
	    dcnt++;
	    dflag = ((dflag != U_USDCFL) ? U_USDCFL : U_CLEARF);
	    if ( len > 2 ) {
		*dpos = i | 0x80000000;
		nxtptr = (curptr+2);
		continue;
	    } else if ( len == 2 ) {
		if ( (i+1) >= argc ) {
		    rc |= U_INUDCT;
		    break;
		};

/*
#if defined(CNVEVT)
		if ( cnvflg == U_EUC ) {
		    ilen = strlen(argv[i+1]);
		    olen = U_MEMSIZ;
		    (void)kjcnvets( argv[i+1], &ilen, cnvdt, &olen );
	            cnvlen = U_MEMSIZ - olen;    
		    strncpy(argv[i+1],cnvdt,cnvlen);
		    argv[i+1][cnvlen] = '\0';
		};
#endif 
*/

		nxtptr = argv[i+1];
	    };
	    if ( (strncmp(nxtptr,U_PRMJSY,2) == 0) ) {
		rc |= U_DUPPRD;
		(void)kjprmsg( i, curptr, U_DUPPRD );
	    } else if ( (strncmp(nxtptr,U_PRMYMI,2) == 0) ||
			(strncmp(nxtptr,U_PRMGKU,2) == 0) ||
			(strncmp(nxtptr,U_PRMPDV,2) == 0) ) {
		rc |= U_INUDCT;
		dflag = U_CLEARF;
	    };
	} else if ( strncmp(curptr,U_PRMYMI,2) == 0 ) {
		rc |= U_PRMADD;
		rc |=
		errmsg = U_PATTRN(flag,U_GOKUFL,U_IGPTRY);
		(void)kjprmsg( i, curptr, errmsg );
		flag = ((flag == U_CLEARF) ? U_YOMIFL : U_CLEARF);
		if ( len > 2 ) {
		    nxtptr = (curptr+2);
		} else if ( len == 2 ) {
		    if ( (i+1) >= argc ) {
			rc |= U_NOTYMI;
			(void)kjprmsg( i, NULL, U_NOTYMI );
			break;
		    };
		    i++;
	ch_1 = argv[i];
#if defined(CNVEVT)
		if ( cnvflg == U_EUC ) {
		    ilen = strlen(argv[i]);
		    olen = U_MEMSIZ;
		    (void)kjcnvets( argv[i], &ilen, cnvdt, &olen );
		    cnvlen = U_MEMSIZ - olen;  
		    strncpy(argv[i],cnvdt,cnvlen);
		    argv[i][cnvlen] = '\0';
	ch_1 = cnvdt;
		};
#endif 

	ch_1 = argv[i];
		    nxtptr = argv[i];
		};
		if ( (strncmp(nxtptr,U_PRMJSY,2) == 0) ||
		     (strncmp(nxtptr,U_PRMYMI,2) == 0) ||
		     (strncmp(nxtptr,U_PRMGKU,2) == 0) ||
		     (strncmp(nxtptr,U_PRMPDV,2) == 0) ) {
		    rc |= U_NOTYDT;
		    (void)kjprmsg( i, nxtptr, U_NOTYDT );
		    flag = U_CLEARF;
		} else {
		    len = strlen(nxtptr);
		    if ( (len % U_DBBYT) != 0 ) {
			rc |= U_YNOLDB;
			(void)kjprmsg( i, nxtptr, U_YNOLDB );
			continue;
		    };
		    ch = nxtptr;
		    for ( j = 0; j < len; j += U_DBBYT ) {
			if ( U_DBCHK(*(ch+j)) != 0 ) {
			    rc |= U_YNOLDB;
			    (void)kjprmsg( i, nxtptr, U_YNOLDB );
			    break;
			};
		    };
		};
	} else if ( strncmp(curptr,U_PRMGKU,2) == 0 ) {
		rc |= U_PRMADD;
		rc |=
		errmsg = U_PATTRN(flag,U_YOMIFL,U_IGPTRG);
		(void)kjprmsg( i, curptr, errmsg );
		flag = ((flag == U_CLEARF) ? U_GOKUFL : U_CLEARF);
		if ( len > 2 ) {
		    nxtptr = (curptr+2);
		} else if ( len == 2 ) {
		    if ( (i+1) >= argc ) {
			rc |= U_NOTGKU;
			(void)kjprmsg( i, NULL, U_NOTGKU );
			break;
		    };
		    i++;

#if defined(CNVEVT)
		    if ( cnvflg == U_EUC ) {
			ilen = strlen(argv[i]);
			olen = U_MEMSIZ;
			(void)kjcnvets( argv[i], &ilen, cnvdt, &olen );
			cnvlen = U_MEMSIZ - olen;  
			strncpy(argv[i],cnvdt,cnvlen);
			argv[i][cnvlen] = '\0';
		    };
#endif

		    nxtptr = argv[i];
		};
		if ( (strncmp(nxtptr,U_PRMJSY,2) == 0) ||
		     (strncmp(nxtptr,U_PRMYMI,2) == 0) ||
		     (strncmp(nxtptr,U_PRMGKU,2) == 0) ||
		     (strncmp(nxtptr,U_PRMPDV,2) == 0) ) {
		    rc |= U_NOTGDT;
		    (void)kjprmsg( i, nxtptr, U_NOTGDT );
		} else {
		    len = strlen(nxtptr);
		    if ( (len % U_DBBYT) != 0 ) {
			rc |= U_GNOLDB;
			(void)kjprmsg( i, nxtptr, U_GNOLDB );
			continue;
		    };
		    ch = nxtptr;
		    for ( j = 0; j < len; j += U_DBBYT ) {
			if ( U_DBCHK(*(ch+j)) != 0 ) {
			    rc |= U_GNOLDB;
			    (void)kjprmsg( i, nxtptr, U_GNOLDB );
			    break;
			};
		    };
		};
	} else if ( strncmp(curptr,U_PRMPDV,2) == 0 ) {
	    pcnt++;
	    pflag = ((pflag != U_PRDVFL) ? U_PRDVFL : U_CLEARF);
	    if ( len > 2 ) {
		*ppos = i | 0x80000000;
		nxtptr = (curptr+2);
		continue;
	    } else if ( len == 2 ) {
		if ( (i+1) >= argc ) {
		    rc |= U_INPRDV;
		    break;
		};

/*
#if defined(CNVEVT)
		if ( cnvflg == U_EUC ) {
		    ilen = strlen(argv[i+1]);
		    olen = U_MEMSIZ;
		    (void)kjcnvets(argv[i+1],&ilen,cnvdt,&olen);
		    cnvlen = U_MEMSIZ - olen;  
		    strncpy(argv[i+1],cnvdt,cnvlen);
		    argv[i+1][olen] = '\0';
		};
#endif
*/

		nxtptr = argv[i+1];
	    };
	    if ( (strncmp(nxtptr,U_PRMPDV,2) == 0) ) {
		rc |= U_DUPPRP;
		(void)kjprmsg( i, curptr, U_DUPPRP );
	    } else if ( (strncmp(nxtptr,U_PRMJSY,2) == 0) ||
			(strncmp(nxtptr,U_PRMYMI,2) == 0) ||
			(strncmp(nxtptr,U_PRMGKU,2) == 0) ) {
		rc |= U_INPRDV;
		pflag = U_CLEARF;
	    };
	} else {
	    rc |=
	    errmsg = (dflag == U_USDCFL ||
		      pflag == U_PRDVFL) ? U_NOMSKD : U_IGDATA;
	    (void)kjprmsg( i, curptr, errmsg );
	    if ( dflag == U_USDCFL ) {
		*dpos = i;
	    };
	    if ( pflag == U_PRDVFL ) {
		*ppos = i;
	    };
	    dflag = U_CLEARF;
	    pflag = U_CLEARF;
	};
    };

    rc |=
    errmsg = ((dcnt > 1) ? U_DUPPRD : U_NOMSKD);
    (void)kjprmsg( 0, NULL, errmsg );
    rc |=
    errmsg = ((pcnt > 1) ? U_DUPPRP : U_NOMSKD);
    (void)kjprmsg( 0, NULL, errmsg );
    rc |=
    errmsg = (flag == U_YOMIFL) ? U_NOTGKU :
	     ((flag == U_GOKUFL) ? U_NOTYMI : U_NOMSKD);
    (void)kjprmsg( 0, NULL, errmsg );

} while ( 0 );

    return( rc );

}
#endif
