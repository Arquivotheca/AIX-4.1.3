static char sccsid[] = "@(#)57	1.1  src/bos/usr/lpp/jls/dictutil/kudiccs3.c, cmdKJI, bos411, 9428A410j 7/22/92 23:20:37";
/*
 * COMPONENT_NAME: User Dictionary Utility 
 *
 * FUNCTIONS: kudiccs3
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
 * US Government Users Restricted Rights - Use, duplication or disclosure
 * restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kudiccs3
 *
 * DESCRIPTIVE NAME:    System Dictionary Check
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1989
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance V1.0
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            Compare Kanji Data with System Dictionary.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1512 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudiccs3
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudiccs( sdcbpt, kanalen, kanadata,
 *                                       kjlen, kjdata, srcflg )
 *
 *  INPUT:              sfcbpt  :Pointer to System Dictionary Control Block.
 *                      kanalen :Kana (7 bit) Length.
 *                      kanadata:Kana (7 bit) Data String.
 *                      kjlen   :Kanji String Length.
 *                      kjdata  :Kanji Data String.
 *
 *  OUTPUT:             srcflg  :Search Flag. (0:Not Found  1:Found)
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      NA.
 *
 * EXIT-ERROR:          Wait State Codes.
 *                      NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              kudicmlc() : Mora Code Conversion.
 *                              kudcread() : System Dictionary Data Read.
 *                              kudiceng() : System Dictionary Data
 *                                           Entry Position Length Search.
 *                      Standard Library.
 *                              memcpy
 *                              malloc
 *                              free
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*----------------------------------------------------------------------*
 * Include Standard.
 *----------------------------------------------------------------------*/
#include <stdio.h>		/* Standard I/O Package.              	*/
#include <fcntl.h>		/* FILE Control Pakage.               	*/
#include <sys/stat.h>		/* 					*/

/*----------------------------------------------------------------------*
 * Include Kanji Project.
 *----------------------------------------------------------------------*/
#include "kut.h"		/* Kanji Utility Define File.         	*/
#include "kumdict.h"		/* Kanji Utility Define File.         	*/

/*----------------------------------------------------------------------*
 * Copyright Identify.
 *----------------------------------------------------------------------*/
static char    *cprt1 = "5601-125 COPYRIGHT IBM CORP 1989, 1992     ";
static char    *cprt2 = "LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

void
kudiccs3( sdcbpt, kanalen, kanadata, kjlen, kjdata, srcflg )

SDCB  	*sdcbpt;	/* Pointer to System Dictionry        		*/
short   kanalen;	/* Yomigana String Length             		*/
uchar   *kanadata;	/* Yomigana String                    		*/
short   kjlen;		/* Kanji String Length                		*/
uchar   *kjdata;	/* Kanji String                       		*/
long    *srcflg;	/* Return Code.                       		*/
{
    extern void     kudicmlc();		/* Mora Code Conversion         */
    extern void     kudiceng();		/* Entry Position Search        */
    extern void     kudcread();		/* Dictionary Pointer Search    */

    MULSYS          mcb;
    short           moralen;		/* Mora Code Lenght             */
    uchar           moradata[U_MAXKNL];	/* Mora Code Data               */
    short           mflag;		/* Mora Code Conversion Flag    */
    ushort          kjcode[256];

    uchar           data[4];	/* Work Mora Code Conversion Area       */
    int             hsy_mrdt;	/* Mora Code Conversion                 */
    int             hsy_dtbk;	/* Yomi Data Conversion                 */
    int             hsy_cnt;	/* System Dictionary Entry Block Counter*/
    uchar          *dicblkpt;	/* Pointer to Dictionary Block          */
    uchar          *work_ptr;	/* Pointer to Dictionary Block Work Area*/
    uchar          *wk_blkpt;	/* Work Area Pointer                    */

    short           enlen;	/* Dictionary Entry Length              */
    short           enpos;	/* Dictionary Entry Position(Offset)    */

    int             knj_cnt;	/* Kanji Data Counter                   */
    char            knj_flg;	/* Kanji Flag                           */
    char            fg_flg;	/* FG Flag                              */

    int             i, j;	/* Loop Counter                         */
    int             sfldes;	/* system dictionary file descripter    */
    uint            snelem;	/* system dictionary file size buffer   */
    struct stat     stbuf;	/* file status work buffer              */
    int             de_ret;	/* editor return code                   */
    int             rt_cd;	/* strncmp Return Code                  */

    /*----- Return Code Set --------------------------------------------*/
    *srcflg = 0;

    /*----- Mora Code Conversion ---------------------------------------*/
    (void)kudicmlc3( kanadata, kanalen, moradata, &moralen, &mflag );

    if ( mflag != 0 )
        return;

    /*----- Copy from (uchar *)kjdata to (ushort *)kjcode --------------*/
    for ( i = 0; i < kjlen; i += 2 ) {
        kjcode[i / 2] = (ushort) (kjdata[i] * 256 + kjdata[i + 1]);
    }
    kjlen /= 2;		/* the number of the arangement kjcode[]	*/

    for ( j=0; j<MAX_SYSDICT; j++ ) {

	if ( sdcbpt[j].dcptr == NULL )
	    continue;

	rt_cd = set_sdic( &mcb, &sdcbpt[j] );
	if ( rt_cd == -1 )
	    continue;

	rt_cd = rd_sdic( moralen, &mcb, &sdcbpt[j] );
	if ( rt_cd == -1 )
	    continue;

	rt_cd = -1;

	if ( chk_exist( &mcb, moralen, moradata ) == 0 ) {
	    if ( moralen == 1 ) {
		rt_cd = srch_mono( &mcb, sdcbpt[j].rdptr, moradata[0],
				  kjcode, kjlen );
	    }
	    else {
		for ( i = 0; i < mcb.poly_dl; i++ ) {
		    work_ptr = (uchar *) (sdcbpt[j].rdptr
					  + mcb.record_size * i);
		    rt_cd = srch_poly( &mcb, work_ptr, moradata, moralen,
				      kjcode, kjlen );
		    if (rt_cd == 0)
			break;
		}
	    }

	}

	if ( rt_cd == 0 ) {
	    *srcflg = 1;
	    return;
	}
    }
    return;
}

