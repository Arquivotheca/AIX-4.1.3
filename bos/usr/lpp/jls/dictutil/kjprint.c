static char sccsid[] = "@(#)78  1.3  src/bos/usr/lpp/jls/dictutil/kjprint.c, cmdKJI, bos411, 9428A410j 8/27/91 12:18:22";
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


#if defined(PARMADD)
#include <stdio.h>      /* Standard I/O Package.                        */
/*#include <memory.h>*/ /* Memory package.                              */
#include <fcntl.h>      /* File contorol Package.                       */
#include <errno.h>      /* system call error ID.                        */
#include <sys/stat.h>

#include "kje.h"
#include "kut.h"                /* Kanji Utility Define File.           */

extern  char    *printdv;       /* Printer Device Pointer               */

#define PR(XX)  XX;

int kjprint( udcbptr )

UDCB    *udcbptr;       /* Pointer to User Dictionary Control Block (i) */
{
UDCS    *topptr  = NULL;        /* Pointer to Top of UDCS               */
UDCS    *lastptr = NULL;        /* Pointer to Last of UDCS              */
UDCS    *curptr  = NULL;        /* Pointer to Current of UDCS           */
UDMS    *dmappt  = NULL;        /* Pointer to UDMS                      */
uchar   *ipt;                   /* Pointer to Index Area                */
uchar   *dpt;                   /* Pointer to Data Area                 */
char    *pwd;                   /* Directory                            */
char    *tmpf;                  /* Pointer to Temporary File Name       */
char    pkick[1024];            /* Parameter for System                 */
uchar   yomidata[U_YOMFLD];     /* Yomi (PC code) String                */
uchar   knjdata[U_GOKFLD];      /* Kanji (PC code) String               */
uchar   cchar;                  /* Sample Data for Check                */
short   indlen;                 /* Length of Index Area of User Dict    */
short   datlen;                 /* Length of Data Area of User Dict.    */
short   dllen;                  /* Length of Data DL of User Dictionary */
short   indpos;                 /* Position of Index Area               */
short   datpos;                 /* Position of Data Area                */
short   dlpos;                  /* Position of DL Area                  */
short   rrn;                    /* Record Number of User Dictionary     */
short   yomilen;                /* Length of Yomidata. (PC code)        */
short   knjlen;                 /* Length of Kanji Data.                */
short   kanalen;                /* Length of Kanadata. (7bit code)      */
short   posc;                   /* Position Counter                     */
short   gyosu;                  /* Maximum Input Line Number            */
int     fd;                     /* File Discripter                      */
int     i;                      /* Loop Counter                         */
int     rc = 0;                 /* Return Value                         */

do {

    /*
     * Allocate Display Map
     */
    dmappt = (UDMS *)malloc(sizeof(UDMS));
    if ( dmappt == NULL ) {
	rc = IUFAIL;
	break;
    };
    dmappt->fld = (UDMFLD *)malloc(sizeof(UDMFLD));
    if ( dmappt->fld == NULL ) {
	if ( dmappt != NULL ) {
	    (void)free( dmappt );
	};
	rc = IUFAIL;
	break;
    };

    /*
     * Make User Dictionary Buffer
     */
    (void)kudcread( udcbptr, (short)3, rrn );

    ipt    = udcbptr->rdptr;            /* Set Work Pointer             */
    indlen = GETSHORT(ipt);             /* Index Area Length            */
    indpos = U_ILLEN + U_STSLEN + U_HARLEN + U_NARLEN;

    /*
     * Get Index
     */
    while( TRUE ) {
	if ( indpos >= indlen ) {       /* Check End of Index Area      */
	    break;
	};

	indpos += *(ipt+indpos);        /* set index position to RRN    */
	rrn     = *(ipt+indpos);        /* Set RRN                      */
	indpos += U_RRNLEN;             /* Set Index Position           */

	/*
	 * User Dictionary Read
	 */
	(void)kudcread( udcbptr, (short)4, rrn );

	dpt    = udcbptr->rdptr;        /* Set Data Area Pointer        */
	datlen = GETSHORT(dpt);         /* Data Area Length             */
	datpos = U_RLLEN;               /* Initialized Data Position    */

	/*
	 * Get Data Area
	 */
	while( TRUE ) {
	    if ( datpos >= datlen ) {   /* Check End of Data Area       */
		break;
	    };

	    kanalen = *(dpt+datpos) - U_KNLLEN; /* Yomi Length          */
	    datpos += U_KNLLEN;         /* Start Yomi Data Position     */
	    cchar   = *(dpt+datpos);    /* Get Sample Data              */

	    /*
	     * Yomi Convert 7 Bit to PC Kanji Code
	     */
	    (void)kudicycr( (uchar *)(dpt+datpos), kanalen,
				      yomidata, &yomilen );

	    datpos += kanalen;          /* Start DL Data Position       */
	    posc    = 0;                /* Initilized Kanji Pos. Counter*/
	    dllen   = GETSHORT((dpt+datpos));   /* DL Length            */

	    dlpos = datpos + U_DLLEN + U_RSVLEN;/* Init. DL Position    */
	    datpos += dllen;            /* Start Next Entry Position    */

	    if ( cchar != U_OMIT_C ) {  /* Check Yomi Data              */
		while( TRUE ) {
		    if ( dlpos >= datpos ) {    /* Check End of Data    */
			break;
		    };

		    knjlen = 0;         /* Initilaized Kanji Length     */

		    /*
		     * Kanji Convert
		     */
		    while( TRUE ) {
			  if ( *(dpt+dlpos) > U_CONV ) {
				break;
			  };

			/*
			 * Convert Type and High Byte
			 */
			if( *(dpt+dlpos) > U_7PCCC ) {
			    knjdata[knjlen] = *(dpt+dlpos) | U_7PCCU;
			} else {
			    knjdata[knjlen] = *(dpt+dlpos) | U_7PCCL;
			};
			dlpos++;        /* Convert of Low Byte          */
			knjlen++;       /* Convert of Low Byte          */
			knjdata[knjlen] = *(dpt+dlpos);
			dlpos++;        /* Count Up                     */
			knjlen++;       /* Count Up                     */

		    }; /* End of While (Knaji Conversion) */

		    /*
		     * Last 2 Byte Convert
		     */
		    knjdata[knjlen] = *(dpt+dlpos);
		    dlpos++;
		    knjlen++;
		    knjdata[knjlen] = *(dpt+dlpos);
		    dlpos++;            /* Count Up                     */
		    knjlen++;           /* Count Up                     */
		    dlpos += U_RSVLEN;  /* DL Pos. to Next Kanji Data   */

		    /*
		     * Dictionary Buffer Allocation
		     */
		    curptr = (UDCS *)malloc(sizeof(UDCS));
		    if ( curptr == NULL ) {
			if ( dmappt->fld != NULL ) {
				(void)free( dmappt->fld );
				if ( dmappt != NULL ) {
					(void)free( dmappt );
				};
			};
			return( IUFAIL );
		    };

		    /*
		     * First Allocate Process
		     */
		    if ( topptr == NULL ) {
			topptr = curptr;        /* Save First Pointer   */
			curptr->pr_pos = NULL;  /* Pointer Null Clear   */
		    } else {
			lastptr->nx_pos = curptr;
			curptr->pr_pos = lastptr;
		    };

		    curptr->status = U_S_INIT;  /* UDCS Status Info.    */
		    curptr->yomilen = yomilen;  /* Set Yomi Data od UDCS*/

		    for ( i = 0; i < yomilen; i++ ) {
			curptr->yomi[i] = yomidata[i];
		    };
		    for( i = yomilen; i < U_B_YOMI; i += 2 ) {
			curptr->yomi[i]    = C_SPACEH;
			curptr->yomi[i+ 1] = C_SPACEL;
		    };

		    posc++;             /* Set UDCS Position            */
		    curptr->pos = posc; /* Set UDCS Position            */

		    curptr->kanlen = knjlen;    /* Kanji Data of UDCS   */
		    for ( i = 0; i < knjlen; i++ ) {
			curptr->kan[i] = knjdata[i];
		    };
		    for ( i = knjlen; i < U_B_KAN; i +=2 )  {
			curptr->kan[i]    = C_SPACEH;
			curptr->kan[i+ 1] = C_SPACEL;
		    };

		    lastptr = curptr;   /* Set Last Pointer to UDCS     */
		    lastptr->nx_pos = 0;/* Clear Next Position          */
		}; /* End of While */
	    }; /* End of If (Yomi Data) */
	}; /* End of While (Data Get) */
    }; /* End of While (Index Data) */

    /*
     * Make Temporary File
     */
#if defined(TMPDIR)
    tmpf = tempnam(U_TMPDIR,"kuprn");
#else
    pwd = (char *)getcwd((char *)NULL,64);
    if ( pwd == NULL ) {
	(void)printf("%s\n", CU_MSGONC);
	break;
    };
    tmpf = tempnam(pwd,"kuprn");
#endif
    if ( tmpf == NULL ) {
	(void)printf("%s\n", CU_MSGONC);
	break;
    };

    /*
     * Output to Temporary File
     */
    fd = creat(tmpf,(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH));
    if ( fd < 0 ) {
	(void)printf("%s\n", CU_MSGONC);
	break;
    };

    /*
     * Yomi,Goku Data Output Flie
     */
    rc = kufout(fd,topptr);

    close(fd);                          /* Closed File                  */

    if ( rc != IUSUCC ) {               /* Check kufout Return Code     */
	(void)printf("%s\n", CU_MSGONC);
	break;
    };

    /*
     * Set System Parameter to Print
     */

/* --- */
    if ( printdv == (char *)NULL ) {
	sprintf(pkick,"cat %s; rm %s",tmpf,tmpf);
    } else {
	sprintf(pkick,"echo \"%s\"; cat %s; rm %s",
				    printdv,tmpf,tmpf);
    };
/* --- */

    system(pkick);

    /*
     *      free display map area and dictionary area.
     */
    if ( dmappt->fld != NULL ) {
	(void)free((char *)(dmappt->fld));
    };
    if ( dmappt != NULL ) {
	(void)free((char *)dmappt);
    };

} while ( 0 );

    return( rc );

}
#endif
