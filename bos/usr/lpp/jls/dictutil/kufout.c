static char sccsid[] = "@(#)41	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kufout.c, cmdKJI, bos411, 9428A410j 7/23/92 01:26:26";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kufout
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:         kufout
 *
 * DESCRIPTIVE NAME:    data of user dictionary to file
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            user dictionary table handler
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1192  Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kufout
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kufout ( fd , topptr )
 *
 *  INPUT               fd      : file discripter
 *                      topptr  : top pointer of UDCS
 *
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         UDSUCC  : sucess return
 *
 * EXIT-ERROR:          IUFAIL  : error
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
 *                      Standard Liblary.
 *                              memset  :
 *                              memcpy  :
 *                              write   :
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Liblary.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Liblary.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*----------------------------------------------------------------------*
 * Include Standard.
 *----------------------------------------------------------------------*/
#include <stdio.h>      /* Standard I/O Package.                        */
/*#include <memory.h>*/ /* Memory package.                              */
#include <string.h>     /* String package.                              */
#include <fcntl.h>      /* File contorol package.                       */
#include <locale.h>     /*                                              */
#include <nl_types.h>   /*                                              */
#include <limits.h>     /*                                              */

/*----------------------------------------------------------------------*
 * Include Kanji Project.
 *----------------------------------------------------------------------*/
#include "kje.h"        /* Kanji Utility Define File.                   */
#include "kut.h"        /* Kanji Utility Define File.                   */
#include "kjdict.msg.h" /* Kanji Utility Define File.                   */

extern  int     cnvflg;         /* Conversion Type Code                 */

/*----------------------------------------------------------------------*
 * Copyright Identify.
 *----------------------------------------------------------------------*/
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989, 1992     ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */
int     kufout ( fd , topptr )

int     fd;             /* file discripter      	*/
UDCS    *topptr;        /* pointer to top of UDCS.      */
{

    UDCS    	*scp; 		/* work pointer to UDCS */
    uchar   	*wyomi;
    uchar   	wyomilen;
    int     	wrc;
    int     	nbyte;          /* print number 	*/
    size_t	pclen,euclen;
    static char kakko[2][4]; 	/* bracket data 	*/
    char    	headarea[70]; 	/* head label data.     */
    static char hl3d_d[20];
    struct outarea {
	char    ylkakko[2];
	char    yomi[U_YOMLEN];
	char    yrkakko[2];
	char    filler[2];
	char    glkakko[2];
	char    goku[U_GOKLEN];
	char    grkakko[2];
	char    cr;
    } outarea;
    size_t	outlen = sizeof( outarea );
    char 	outarea_euc[ sizeof( outarea ) * 2 ];
    size_t 	outlen_euc = sizeof( outarea ) * 2;

    strcpy(kakko[0], PC932_MNKAK2);
    strcpy(kakko[1], PC932_MNKAK3);
    strcpy(hl3d_d,   CU_MNUTIT);

    /*------------------------------------------------------------------*
     * 0. print header
     *------------------------------------------------------------------*/
    nbyte = 45;
    memset(headarea,0x20,26);
    headarea[26] = NULL;
    strcat(headarea,hl3d_d);

    wrc = write( fd, headarea, nbyte );
    if( wrc < 0 )
	  return( IUFAIL );

    /*----- write blank line -------------------------------------------*/
    headarea[0] = 0x0a;
    wrc = write( fd, headarea, 1 );

    /*------------------------------------------------------------------*
     * 1. print data
     *------------------------------------------------------------------*/
    /*----- set out area -----------------------------------------------*/
    nbyte = sizeof(outarea);
    memcpy(outarea.glkakko,kakko[0],2);
    memcpy(outarea.grkakko,kakko[1],2);
    memset(outarea.filler,0x20,2);
    outarea.cr = 0x0a;

    /*----- set work yomi & yomilen ------------------------------------*/
    wyomi = NULL;
    wyomilen = 0;
    scp = topptr;

    /*----- prcess loop. (loopend mark is '!!!!') ----------------------*/
    while ( TRUE ) {
	if ( scp == NULL )
	    break;
	/* check !  this data is valid. (endif mark is '####')        	*/
	if ( (scp->status == U_S_INIT)  ||
	     (scp->status == U_S_YOMA)  ||
	     (scp->status == U_S_KNJU) ) {
	    /* clear data field   */
	    memset(outarea.yomi,0x20,U_YOMLEN);
	    memset(outarea.goku,0x20,U_GOKLEN);
	    memset(outarea.ylkakko,0x20,2);
	    memset(outarea.yrkakko,0x20,2);

	    /* check !  need to print yomi        */
	    if (    (scp->yomilen != wyomilen)
		|| (memcmp(scp->yomi,wyomi,wyomilen) != 0) ) {
	      	/* case : need to yomi      */

	      	/*  keep current yomi data  */
	      	wyomi = scp->yomi;
	      	wyomilen = scp->yomilen;

	      	/* set data to outarea      */
	      	memcpy(outarea.ylkakko,kakko[0],2);
	      	memcpy(outarea.yrkakko,kakko[1],2);
	      	memcpy(outarea.yomi,wyomi,(int)(wyomilen));
	    }

	    /* set goku   */
	    memcpy(outarea.goku,scp->kan,(int)(scp->kanlen));

	    /* print out */
	    if ( cnvflg == U_EUC ) {
		pclen  = outlen;
		euclen = outlen_euc;
		wrc = kjcnvste( &outarea, &pclen, outarea_euc, &euclen );
		if( wrc < 0 )
	  	    return( IUFAIL );
	    	wrc = write( fd, outarea_euc, (outlen_euc-euclen) );
	    }
	    else {
	    	wrc = write( fd, &outarea, nbyte );
  	    }
	    if ( wrc < 0 )
	      return( IUFAIL );

	}  /* endif ####     */

	  /* set next data      */
	scp = scp->nx_pos;

    } /* endloop !!!!      */

    return( IUSUCC );

}

