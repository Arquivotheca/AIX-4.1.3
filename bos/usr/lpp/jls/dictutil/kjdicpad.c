static char sccsid[] = "@(#)74	1.3.1.1  src/bos/usr/lpp/jls/dictutil/kjdicpad.c, cmdKJI, bos411, 9428A410j 7/23/92 00:54:31";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kjdicpad
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
/*----------------------------------------------------------------------*
 * Include Standard.
 *----------------------------------------------------------------------*/
#include <stdio.h>      /* Standard I/O Package.                        */
/*#include <memory.h>*/ /* Memory Package                               */

/*----------------------------------------------------------------------*
 * Include Kanji Project.
 *----------------------------------------------------------------------*/
#include "kje.h"
#include "kut.h"        /* Utility Define File                          */

#define PR(XX)  XX;

extern	int	cnvflg;		/* Conversion Type Code */

int	kjdicpad( udcbptr, sdcbptr, ch_y, ch_g )

UDCB    *udcbptr;       /* User Dictionary Control Block Pointer    (i) */
SDCB    *sdcbptr;       /* System Dictionary Control Block Pointer  (i) */
char    *ch_y;          /* Yomi Data Pointer (PC code)              (i) */
char    *ch_g;          /* Goku Data Pointer (PC code)              (i) */
{

    short   mode;                       /* Mode                         */
    uchar   *csdata;                    /* System Dictionary Offset     */
    uchar   kanadata[U_YOMILN];         /* kana data area               */
    uchar   hkdata[U_KJFLD];            /* goku data area (conv)        */
    int     ylen;                       /* Yomi Length                  */
    int     glen;                       /* Goku Length                  */
    long    cslen;                      /* System Dictionary Length     */
    short   hklen;                      /* Goku Data Length(Conversion) */
    short   kanalen;                    /* Kana Length                  */
    short   shlen;                      /* Work Short Length            */
    union {
	uchar  buf1[sizeof(short)];     /* Work Buffer Characters       */
	ushort buf2;                    /* Work Buffer Short            */
    } wk1, wk2;                         /* Work Buffer                  */
    int     i;                          /* Loop Counter                 */
    short   src;                        /* Return Value (short)         */
    int     rc;                       	/* Return Value (int)           */

    char    cch_y[U_YMFLD*2];		/* Converted Yomi Data Pointer	*/
    char    cch_g[U_KJFLD*2];		/* Converted Goku Data Pointer	*/
    size_t  ilen;			/* Conversion Input Length	*/
    size_t  olen;			/* Conversion Output Buffer Length */
    int     errflg=0;

    do {

	ylen = strlen( ch_y );
	glen = strlen( ch_g );

	if (cnvflg == U_SJIS) {
		strcpy(cch_y, ch_y);
		strcpy(cch_g, ch_g);
	} else {
		ilen = ylen;
		olen = U_YMFLD*2;
		kjcnvste(ch_y, &ilen, cch_y, &olen);
		cch_y[U_YMFLD*2-olen] = '\0';
		ilen = glen;
		olen = U_KJFLD*2;
		kjcnvste(ch_g, &ilen, cch_g, &olen);
		cch_g[U_KJFLD*2-olen] = '\0';
	}

	/*--------------------------------------------------------------*
	 * Convert PC to 7 Bit Data
	 *--------------------------------------------------------------*/
	(void)kudicymc( ch_y, ylen, kanadata, &shlen, &src );
	if ( shlen > U_YMAX ) {         /* Check Max Kana Length        */
	    (void)printf("%s\n", CU_MSGYOR);
	    errflg++;
	    break;
	}
	kanalen = shlen;                /* Set Kana Length              */

	/*--------------------------------------------------------------*
	 * Check Mixed Kana, Katakana, Capital, Small Letter
	 *--------------------------------------------------------------*/
	if ( (src == U_KATA) || (src == U_CAPOFF) ) {
	    (void)kudicycr( kanadata, kanalen, ch_y, &shlen );
	    ylen = shlen;
	} else if( src == U_INVALD ) {
	    (void)printf("%s\n", CU_MSGIYM);
	    errflg++;
	    break;
	} else if ( src == U_HEMIX ) {
	    (void)printf("%s\n", CU_MSGHAM);
	    errflg++;
	    break;
	}

	/*--------------------------------------------------------------*
	 * Top of Data is Chouon
	 *--------------------------------------------------------------*/
	if ( (kanadata[0] == U_CHOUON) || (kanadata[0] == U_OMIT_C) ) {
	    (void)printf("%s\n", CU_MSGIYM);
	    errflg++;
	    break;
	}

	/*--------------------------------------------------------------*
	 * Same Data Length and Same Data
	 *--------------------------------------------------------------*/
	if ( (ylen == glen) && (memcmp(ch_y,ch_g,(int)ylen) == 0) ) {
	    (void)printf("%s(%s%s,%s%s)\n", CU_MSGYGE,
				CU_MSGYDT, cch_y, CU_MSGGDT, cch_g);
	    errflg++;
	    break;
	}

	/*--------------------------------------------------------------*
	 * Check Convert Goku Data
	 *--------------------------------------------------------------*/
	rc = kuhkfc( kanadata, kanalen, ch_g, glen, hkdata, &hklen );

	/*--------------------------------------------------------------*
	 * Yomi and Goku are Same
	 *--------------------------------------------------------------*/
	if ( rc != U_DIVHK ) {
	    cslen  = hklen;         /* Set Convert Length           */
	    csdata = hkdata;        /* Set Convert Data             */
	} else {
	    cslen  = glen;          /* Set Goku Length              */
	    csdata = ch_g;          /* Set Goku Data                */
	}

	/*--------------------------------------------------------------*
	 * Check Data by System Dictionary
	 *--------------------------------------------------------------*/
#if defined(_OLD_AIX320)
	(void)kudiccs( sdcbptr, kanalen, kanadata, cslen, csdata, &rc );
#else
	(void)kudiccs3( sdcbptr, kanalen, kanadata, cslen, csdata, &rc );
#endif /* defined(_OLD_AIX320) */

	/*--------------------------------------------------------------*
	 * Already Registration This Data in The System Dictionary
	 *--------------------------------------------------------------*/
	if ( rc != 0 ) {
		(void)printf("%s(%s%s,%s%s)\n", CU_MSGERG,
				CU_MSGYDT, cch_y, CU_MSGGDT, cch_g);
		errflg++;
		break;
	}

	/*--------------------------------------------------------------*
	 * Registration This Data
	 *--------------------------------------------------------------*/
	mode = U_REGIST;                /* Set Registration Mode        */
	rc = kudicadp( mode, kanadata, kanalen, ch_g, glen, udcbptr );

	if ( rc == UDSUCC ) {
		udcbptr->updflg = U_FON;/* Update Flag ON               */
		wk1.buf2 = kanalen;     /* Separate Yomi Length         */
		wk2.buf2 = glen;        /* Separate Goku Length         */
		/*------------------------------------------------------*
		 * Addition MRU Area
		 *------------------------------------------------------*/
		(void)kudcmrua( udcbptr->dcptr,
				kanadata, wk1.buf1[U_CHLOW],
				ch_g, wk2.buf1[U_CHLOW] );
		/*------------------------------------------------------*
		 * Save Dictionary Data
		 *------------------------------------------------------*/
		 rc = kutmwrt(udcbptr->orgfd, udcbptr->dcptr, udcbptr->ufilsz);
		 if( rc == IUSUCC ) {
		 	(void)printf("%s(%s%s,%s%s)\n", CU_MSGSUC,
		 			CU_MSGYDT, cch_y, CU_MSGGDT, cch_g);
		 } else {
		 	(void)printf("%s\n", CU_MSGUAE);
		 	udcbptr->updflg = U_FOF;
			errflg++;
		 }
	}
	else if ( rc == UDOVFDLE ) {
		(void)printf("%s(%s%s)\n", CU_MSGNYR, CU_MSGYDT, ch_y);
		errflg++;
	}
	else if ( rc == UDDCEXTE ) {
		(void)printf("%s(%s%s,%s%s)\n", CU_MSGETE,
				CU_MSGYDT, cch_y, CU_MSGGDT, cch_g);
		errflg++;
	}
	else if ( rc == UDDCFULE ) {
		(void)printf("%s\n", CU_MSGEFL);
		errflg++;
	}

    } while ( 0 );

    return ( errflg );

}
#endif
