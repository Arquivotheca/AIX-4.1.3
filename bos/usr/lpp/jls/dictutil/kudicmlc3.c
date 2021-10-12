static char sccsid[] = "@(#)58	1.1  src/bos/usr/lpp/jls/dictutil/kudicmlc3.c, cmdKJI, bos411, 9428A410j 7/22/92 23:23:48";
/*
 * COMPONENT_NAME: User Dictionary Utility 
 *
 * FUNCTIONS: kudicmlc3
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

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kudicmlc3
 *
 * DESCRIPTIVE NAME:    Conversion Mora code data.
 *
 * COPYRIGHT:           XXXX-XXX COPYRIGHT IBM CORP 1989, 1992
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            Convert to mora code data from 7bit code data.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1828 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudicmlc3
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kuy2mora(kanadata,kanalen,moradata,moralen,mflag)
 *
 *  INPUT:              kanadata:Pointer to kanadata(7bit) string.
 *                      kanalen :Length of kanadata.
 *
 *  OUTPUT:             moradata:Pointer to moradata string.
 *                      moralen :Length of moradata.
 *                      mflag   :convert flag.
 *
 * EXIT-NORMAL:         NA.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
 *                      Standard Library.
 *                              NA.
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
#include <stdio.h>		/* Standard I/O Package.                */

/*----------------------------------------------------------------------*
 * Include Kanji Project.
 *----------------------------------------------------------------------*/
#include "kut.h"		/* Kanji Utility Define File.           */

/*----------------------------------------------------------------------*
 * Copyright Identify.
 *----------------------------------------------------------------------*/
static char    *cprt1 = "5601-125 COPYRIGHT IBM CORP 1989, 1992     ";
static char    *cprt2 = "LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*----------------------------------------------------------------------*
 * Define.
 *----------------------------------------------------------------------*/
#define D_VU     0xb4		/* 'vu'                               	*/
#define DSKA     0xff		/* shift 'ka'                         	*/
#define DSKE     0xff		/* shift 'ke'                         	*/
#define DS_A     0x1f		/* shift 'a'                          	*/
#define D__A     0x20		/* 'a'                                	*/
#define DS_I     0x21		/* shift 'i'                          	*/
#define D__I     0x22		/* 'i'                                	*/
#define DS_U     0x23		/* shift 'u'                          	*/
#define D__U     0x24		/* 'u'                                	*/
#define DS_E     0x25		/* shift 'e'                          	*/
#define D__E     0x26		/* 'e'                                	*/
#define DS_O     0x27		/* shift 'o'                          	*/
#define D__O     0x28		/* 'o'                                	*/
#define D_KA     0x29		/* 'ka'                               	*/
#define D_GA     0x2a		/* 'ga'                               	*/
#define D_KI     0x2b		/* 'ki'                               	*/ 
#define D_GI     0x2c		/* 'gi'                               	*/ 
#define D_KU     0x2d		/* 'ku'                               	*/
#define D_GU     0x2e		/* 'gu'                               	*/
#define D_KE     0x2f		/* 'ke'                               	*/
#define D_GE     0x30		/* 'ge'                               	*/
#define D_KO     0x31		/* 'ko'                               	*/
#define D_GO     0x32		/* 'go'                               	*/
#define D_SA     0x33		/* 'sa'                               	*/
#define D_ZA     0x34		/* 'za'                               	*/ 
#define D_SI     0x35		/* 'si'                               	*/
#define D_ZI     0x36		/* 'zi'                               	*/
#define D_SU     0x37		/* 'su'                               	*/
#define D_ZU     0x38		/* 'zu'                               	*/
#define D_SE     0x39		/* 'se'                               	*/
#define D_ZE     0x3a		/* 'ze'                               	*/
#define D_SO     0x3b		/* 'so'                               	*/
#define D_ZO     0x3c		/* 'zo'                               	*/
#define D_TA     0x3d		/* 'ta'                               	*/
#define D_DA     0x3e		/* 'da'                               	*/
#define D_TI     0x3f		/* 'ti'                               	*/
#define D_DI     0xff		/* 'di'                               	*/
#define DSTU     0x41		/* shift 'tu'                         	*/
#define D_TU     0x42		/* 'tu'                               	*/
#define D_DU     0xff		/* 'du'                               	*/
#define D_TE     0x44		/* 'te'                               	*/
#define D_DE     0x45		/* 'de'                               	*/
#define D_TO     0x46		/* 'to'                               	*/
#define D_DO     0x47		/* 'do'                               	*/
#define D_NA     0x48		/* 'na'                               	*/
#define D_NI     0x49		/* 'ni'                               	*/
#define D_NU     0x4a		/* 'nu'                               	*/
#define D_NE     0x4b		/* 'ne'                               	*/
#define D_NO     0x4c		/* 'no'                               	*/
#define D_HA     0x4d		/* 'ha'                               	*/
#define D_BA     0x4e		/* 'ba'                               	*/
#define D_PA     0x4f		/* 'pa'                               	*/
#define D_HI     0x50		/* 'hi'                               	*/
#define D_BI     0x51		/* 'bi'                               	*/
#define D_PI     0x52		/* 'pi'                               	*/
#define D_HU     0x53		/* 'hu'                               	*/
#define D_BU     0x54		/* 'bu'                               	*/
#define D_PU     0x55		/* 'pu'                               	*/
#define D_HE     0x56		/* 'he'                               	*/
#define D_BE     0x57		/* 'be'                               	*/
#define D_PE     0x58		/* 'pe'                               	*/
#define D_HO     0x59		/* 'ho'                               	*/
#define D_BO     0x5a		/* 'bo'                               	*/
#define D_PO     0x5b		/* 'po'                               	*/
#define D_MA     0x5c		/* 'ma'                               	*/
#define D_MI     0x5d		/* 'mi'                               	*/
#define D_MU     0x5e		/* 'mu'                               	*/
#define D_ME     0x5f		/* 'me'                               	*/
#define D_MO     0x60		/* 'mo'                               	*/
#define D_YA     0x62		/* 'ya'                               	*/
#define D_YU     0x64		/* 'yu'                               	*/
#define D_YO     0x66		/* 'yo'                               	*/
#define D_RA     0x67		/* 'ra'                               	*/
#define D_RI     0x68		/* 'ri'                               	*/
#define D_RU     0x69		/* 'ru'                               	*/
#define D_RE     0x6a		/* 're'                               	*/
#define D_RO     0x6b		/* 'ro'                               	*/
#define DSWA     0x6c		/* shift 'wa'                          	*/
#define D_WA     0x6d		/* 'wa'                               	*/
#define D_WI     0x6e		/* 'wi'                               	*/
#define D_WE     0x6f		/* 'we'                               	*/
#define D_WO     0x70		/* 'wo'                               	*/
#define D__N     0x71		/* 'nn'                               	*/
#define DBAR     0x74		/* '-'                                	*/
#define DCDT     0x77		/* '.'                                	*/
#define DPOD     0x78		/* POND code                          	*/
#define DBA2     0x7a		/* '-'                                	*/
#define DSLS     0x7b		/* '/'                                	*/
#define DKYA     0x90		/* 'kya'                              	*/
#define DGYA     0x93		/* 'gya'                              	*/
#define DSYA     0x96		/* 'sya'                              	*/
#define DZYA     0x99		/* 'zya'                              	*/
#define DCYA     0x9c		/* 'cya'                              	*/
#define DNYA     0xa2		/* 'nya'                              	*/
#define DHYA     0xa5		/* 'hya'                              	*/
#define DBYA     0xa8		/* 'bya'                              	*/
#define DPYA     0xab		/* 'pya'                              	*/
#define DMYA     0xae		/* 'mya'                              	*/
#define DRYA     0xb1		/* 'rya'                              	*/
#define DKYU     0x91		/* 'kyu'                              	*/
#define DGYU     0x94		/* 'gyu'                              	*/
#define DSYU     0x97		/* 'syu'                              	*/
#define DZYU     0x9a		/* 'zyu'                              	*/
#define DCYU     0x9d		/* 'cyu'                              	*/
#define DNYU     0xa3		/* 'nyu'                              	*/
#define DHYU     0xa6		/* 'hyu'                              	*/
#define DBYU     0xa9		/* 'byu'                              	*/
#define DPYU     0xac		/* 'pyu'                              	*/
#define DMYU     0xaf		/* 'myu'                              	*/
#define DRYU     0xb2		/* 'ryu'                              	*/
#define DKYO     0x92		/* 'kyo'                              	*/
#define DGYO     0x95		/* 'gyo'                              	*/
#define DSYO     0x98		/* 'syo'                              	*/
#define DZYO     0x9b		/* 'zyo'                              	*/
#define DCYO     0x9e		/* 'cyo'                              	*/
#define DNYO     0xa4		/* 'nyo'                              	*/
#define DHYO     0xa7		/* 'hyo'                              	*/
#define DBYO     0xaa		/* 'byo'                              	*/
#define DPYO     0xad		/* 'pyo'                              	*/
#define DMYO     0xb0		/* 'myo'                              	*/
#define DRYO     0xb3		/* 'ryo'                              	*/
#define D_N0     0xcf		/* '0'                                	*/
#define D_N1     0xd0		/* '1'                                	*/
#define D_N2     0xd1		/* '2'                                	*/
#define D_N3     0xd2		/* '3'                                	*/
#define D_N4     0xd3		/* '4'                                	*/
#define D_N5     0xd4		/* '5'                                	*/
#define D_N6     0xd5		/* '6'                                	*/
#define D_N7     0xd6		/* '7'                                	*/
#define D_N8     0xd7		/* '8'                                	*/
#define D_N9     0xd8		/* '9'                                	*/

#define DALA     0xe0		/* 'A'                                	*/
#define DALB     0xe1		/* 'B'                                	*/
#define DALC     0xe2		/* 'C'                                	*/
#define DALD     0xe3		/* 'D'                                	*/
#define DALE     0xe4		/* 'E'                                	*/
#define DALF     0xe5		/* 'F'                                	*/
#define DALG     0xe6		/* 'G'                                	*/
#define DALH     0xe7		/* 'H'                                	*/
#define DALI     0xe8		/* 'I'                                	*/
#define DALJ     0xe9		/* 'J'                                	*/
#define DALK     0xea		/* 'K'                                	*/
#define DALL     0xeb		/* 'L'                                	*/
#define DALM     0xec		/* 'M'                                	*/
#define DALN     0xed		/* 'N'                                	*/
#define DALO     0xee		/* 'O'                                	*/
#define DALP     0xef		/* 'P'                                	*/
#define DALQ     0xf0		/* 'Q'                                	*/
#define DALR     0xf1		/* 'R'                                	*/
#define DALS     0xf2		/* 'S'                                	*/
#define DALT     0xf3		/* 'T'                                	*/
#define DALU     0xf4		/* 'U'                                	*/
#define DALV     0xf5		/* 'V'                                	*/
#define DALW     0xf6		/* 'W'                                	*/
#define DALX     0xf7		/* 'X'                                	*/
#define DALY     0xf8		/* 'Y'                                	*/
#define DALZ     0xf9		/* 'Z'                                	*/

#define D_CNVYA  0x61		/* shift 'ya'                         	*/
#define D_CNVYU  0x63		/* shift 'yu'                         	*/
#define D_CNVYO  0x65		/* shift 'yo'                         	*/

#define D__OK     0		/* Nomal conversion code             	*/
#define D_ESC     1		/* ESC('1D') code                    	*/
#define D_CNV     3		/* 					*/

void 
kudicmlc3( kanadata, kanalen, moradata, moralen, mflag )
uchar 	*kanadata;	/* Pointer to kanadata(7bit code) string 	*/
short   kanalen;	/* Lenght of kanadata                    	*/
uchar   *moradata;	/* Pointer to moradata string            	*/
short   *moralen;	/* Lenght of moradata                    	*/
short   *mflag;		/* Convert code                          	*/
{

    char 	isw;	/* While loop switch flag                	*/
    char        nsw;	/* Error switch flag                     	*/
    int         ck_cd;	/* Character type                        	*/
    uchar       shift;
    uchar       mora_cd;/* Mora code area                        	*/
    uchar       kana_cd;/* 7bit code area                        	*/

    /*------------------------------------------------------------------*
     * Usually convert table from kanadata(7bit code) to moradata 
     *------------------------------------------------------------------*/
    static uchar    cnv_tbl1[2][128] = {

	/*  Kana       0     1     2     3     4     5     6     7  */  
	/*             8     9     a     b     c     d     e     f  */  
	/*  0  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	             NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*  1  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	             NULL, NULL, D_VU, DSKA, DSKE, NULL, DPOD, DS_A,
	/*  2  */    D__A, DS_I, D__I, DS_U, D__U, DS_E, D__E, DS_O,
	             D__O, D_KA, D_GA, D_KI, D_GI, D_KU, D_GU, D_KE,
	/*  3  */    D_GE, D_KO, D_GO, D_SA, D_ZA, D_SI, D_ZI, D_SU,
	             D_ZU, D_SE, D_ZE, D_SO, D_ZO, D_TA, D_DA, D_TI,
	/*  4  */    D_ZI, DSTU, D_TU, D_ZU, D_TE, D_DE, D_TO, D_DO,
	             D_NA, D_NI, D_NU, D_NE, D_NO, D_HA, D_BA, D_PA,
	/*  5  */    D_HI, D_BI, D_PI, D_HU, D_BU, D_PU, D_HE, D_BE,
		     D_PE, D_HO, D_BO, D_PO, D_MA, D_MI, D_MU, D_ME,
	/*  6  */    D_MO, NULL, D_YA, NULL, D_YU, NULL, D_YO, D_RA,
		     D_RI, D_RU, D_RE, D_RO, DSWA, D_WA, D__I, D__E,
	/*  7  */    D_WO, D__N, NULL, NULL, DBAR, D_N0, D_N1, D_N2,
                     D_N3, D_N4, D_N5, D_N6, D_N7, D_N8, D_N9, NULL,

	/*  Alpha      0     1     2     3     4     5     6     7  */  
	/*             8     9     a     b     c     d     e     f  */  
	/*  0  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*  1  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*  2  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL, NULL, DBA2, NULL, DSLS,
	/*  3  */    D_N0, D_N1, D_N2, D_N3, D_N4, D_N5, D_N6, D_N7,
		     D_N8, D_N9, NULL, NULL, NULL, NULL, NULL, NULL,
	/*  4  */    NULL, DALA, DALB, DALC, DALD, DALE, DALF, DALG,
		     DALH, DALI, DALJ, DALK, DALL, DALM, DALN, DALO,
	/*  5  */    DALP, DALQ, DALR, DALS, DALT, DALU, DALV, DALW,
		     DALX, DALY, DALZ, NULL, NULL, NULL, NULL, DBA2,
	/*  6  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*  7  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    };

    /*------------------------------------------------------------------*
     * Character type table
     *------------------------------------------------------------------*/
    static int      cnv_tbl2[2][128] = {

	/*  Kana       0      1      2      3      4      5      6      7  */  
	/*             8      9      a      b      c      d      e      f  */  
	/*  0  */   U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR,
		    U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR,
	/*  1  */   U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR,
		    U_ERR, U_ERR, D__OK, D__OK, D__OK, D_ESC, D__OK, D__OK,
	/*  2  */   D__OK, D__OK, D__OK, D__OK, D__OK, D__OK, D__OK, D__OK,
		    D__OK, D__OK, D__OK, D_CNV, D_CNV, D__OK, D__OK, D__OK,
	/*  3  */   D__OK, D__OK, D__OK, D__OK, D__OK, D_CNV, D_CNV, D__OK,
		    D__OK, D__OK, D__OK, D__OK, D__OK, D__OK, D__OK, D_CNV,
	/*  4  */   D_CNV, D__OK, D__OK, D__OK, D__OK, D__OK, D__OK, D__OK,
		    D__OK, D_CNV, D__OK, D__OK, D__OK, D__OK, D__OK, D__OK,
	/*  5  */   D_CNV, D_CNV, D_CNV, D__OK, D__OK, D__OK, D__OK, D__OK,
		    D__OK, D__OK, D__OK, D__OK, D__OK, D_CNV, D__OK, D__OK,
	/*  6  */   D__OK, U_ERR, D__OK, U_ERR, D__OK, U_ERR, D__OK, D__OK,
		    D_CNV, D__OK, D__OK, D__OK, D__OK, D__OK, D__OK, D__OK,
	/*  7  */   D__OK, D__OK, U_ERR, U_ERR, D__OK, D__OK, D__OK, D__OK,
		    D__OK, D__OK, D__OK, D__OK, D__OK, D__OK, D__OK, U_ERR,

	/*  Alpha      0      1      2      3      4      5      6      7  */  
	/*             8      9      a      b      c      d      e      f  */  
	/*  0  */   U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR,
		    U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR,
	/*  1  */   U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR,
		    U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, D_ESC, U_ERR, U_ERR,
	/*  2  */   U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR,
		    U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, D__OK, U_ERR, D__OK,
	/*  3  */   D__OK, D__OK, D__OK, D__OK, D__OK, D__OK, D__OK, D__OK,
		    D__OK, D__OK, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR,
	/*  4  */   U_ERR, D__OK, D__OK, D__OK, D__OK, D__OK, D__OK, D__OK,
		    D__OK, D__OK, D__OK, D__OK, D__OK, D__OK, D__OK, D__OK,
	/*  5  */   D__OK, D__OK, D__OK, D__OK, D__OK, D__OK, D__OK, D__OK,
		    D__OK, D__OK, D__OK, U_ERR, U_ERR, U_ERR, U_ERR, D__OK,
	/*  6  */   U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR,
		    U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR,
	/*  7  */   U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR,
		    U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR, U_ERR
    }; 

    /*------------------------------------------------------------------*
     * Shift 'ya' table
     *------------------------------------------------------------------*/
    static uchar    cnv_tbl3_1[128] = {

	/*  Kana       0     1     2     3     4     5     6     7  */
	/*             8     9     a     b     c     d     e     f  */
	/*  0  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*  1  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*  2  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, DKYA, DGYA, NULL, NULL, NULL,
	/*  3  */    NULL, NULL, NULL, NULL, NULL, DSYA, DZYA, NULL,
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, DCYA,
	/*  4  */    DZYA, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, DNYA, NULL, NULL, NULL, NULL, NULL, NULL,
	/*  5  */    DHYA, DBYA, DPYA, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL, NULL, DMYA, NULL, NULL,
	/*  6  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     DRYA, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*  7  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    };

    /*------------------------------------------------------------------*
     * Shift 'yu' table
     *------------------------------------------------------------------*/
    static uchar    cnv_tbl3_2[128] = {

	/*  Kana       0     1     2     3     4     5     6     7  */
	/*             8     9     a     b     c     d     e     f  */
	/*  0  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*  1  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*  2  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, DKYU, DGYU, NULL, NULL, NULL,
	/*  3  */    NULL, NULL, NULL, NULL, NULL, DSYU, DZYU, NULL,
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, DCYU,
	/*  4  */    DZYU, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, DNYU, NULL, NULL, NULL, NULL, NULL, NULL,
	/*  5  */    DHYU, DBYU, DPYU, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL, NULL, DMYU, NULL, NULL,
	/*  6  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     DRYU, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*  7  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    };

    /*------------------------------------------------------------------*
     * Shift 'yo' table
     *------------------------------------------------------------------*/
    static uchar    cnv_tbl3_3[128] = {

	/*  Kana       0     1     2     3     4     5     6     7  */
	/*             8     9     a     b     c     d     e     f  */
	/*  0  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*  1  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*  2  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, DKYO, DGYO, NULL, NULL, NULL,
	/*  3  */    NULL, NULL, NULL, NULL, NULL, DSYO, DZYO, NULL,
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, DCYO,
	/*  4  */    DZYO, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, DNYO, NULL, NULL, NULL, NULL, NULL, NULL,
	/*  5  */    DHYO, DBYO, DPYO, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL, NULL, DMYO, NULL, NULL,
	/*  6  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     DRYO, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/*  7  */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    };

    /*------------------------------------------------------------------*
     * 1. Initialize flag.
     *------------------------------------------------------------------*/
    shift = 0;
    *mflag = D__OK;		/* Initialize convert code       	*/
    nsw = C_SWOFF;		/* Initialize error switch flag      	*/
    isw = C_SWON;		/* Initialize while loop switch flag 	*/

    while ( isw == C_SWON ) {

	/*--------------------------------------------------------------*
	 * 2.1. Check mora code conversion table
	 *--------------------------------------------------------------*/
	/*----- Set local variable -------------------------------------*/
	ck_cd = cnv_tbl2[shift][*kanadata];

	/*----- Check ck_cd of local variable is invalid value ---------*/
	if ( ck_cd == D_ESC ) {
	    shift ^= 1;
	    kanadata++;		/* Add pointer to kanadata  		*/
	    kanalen--;		/* Subtract kanalen        		*/
	}

	for ( *moralen = 0; kanalen > 0; ) {

	    /*----------------------------------------------------------*
	     * 2.2. Management out of code. 
	     *----------------------------------------------------------*/
	    /*----- Kanadata is out of code ----------------------------*/
	    if ( *kanadata & C_SB80 )
		ck_cd = U_ERR;			/* Set local variable  	*/
	    else
	        ck_cd = cnv_tbl2[shift][*kanadata]; /* Set local variable*/

	    /*----------------------------------------------------------*
	     * 2.3. Set conversion code.
	     *----------------------------------------------------------*/
	    /*----- Check error switch flag and character type ---------*/
	    if ( nsw == C_SWOFF && ck_cd != D_CNV )
		*mflag = ck_cd;		/* Set conversion code  	*/

	    /*----------------------------------------------------------*
	     * 2.4. Convert moracode.
	     *----------------------------------------------------------*/
	    switch ( ck_cd ) {
	    case D__OK:		/* Management nomal conversion type 	*/
		/*----- Set mora code area -----------------------------*/
		mora_cd = cnv_tbl1[shift][*kanadata];
		kanadata++;	/* Add pointer to kanadata  		*/
		kanalen--;	/* Subtract kanalen        		*/
		break;

	    case U_ERR:		/* Management error type  		*/
		/*----- Set mora code area -----------------------------*/
		mora_cd = NULL;
		nsw = C_SWON;	/* Set error switch flag    		*/
		kanadata++;	/* Add pointer to kanadata  		*/
		kanalen--;	/* Subtract kanalen        		*/
		break;

	    case D_CNV:
		/*----- Set kanadata to local variable -----------------*/
		kana_cd = *kanadata;

		/*----- Check lenght of kanadata -----------------------*/
		if ( kanalen == 1 ) {
		    /*----- Set mora code area -------------------------*/
		    mora_cd = cnv_tbl1[shift][kana_cd];
		    kanalen--;	/* Subtract kanalen        		*/
		    break;
		}

		kanadata++;	/* Add pointer to kanadata  		*/
		kanalen--;	/* Subtract kanalen        		*/

		switch ( *kanadata ) {
		case D_CNVYA:	/* Shift 'ya' 				*/
		    mora_cd = cnv_tbl3_1[kana_cd]; /* Set mora code area*/
		    kanadata++;		/* Add pointer to kanadata  	*/
		    kanalen--;		/* Subtract kanalen        	*/
		    break;

		case D_CNVYU:	/* Shift 'yu' 				*/
		    mora_cd = cnv_tbl3_2[kana_cd]; /* Set mora code area*/
		    kanadata++;		/* Add pointer to kanadata  	*/
		    kanalen--;		/* Subtract kanalen        	*/
		    break;

		case D_CNVYO:	/* Shift 'yo' 				*/
		    mora_cd = cnv_tbl3_3[kana_cd]; /* Set mora code area*/
		    kanadata++;		/* Add pointer to kanadata  	*/
		    kanalen--;		/* Subtract kanalen        	*/
		    break;

		default:
		    /*----- Set mora code area -------------------------*/
		    mora_cd = cnv_tbl1[shift][kana_cd];

		}		/* end switch(*kanadata)  	*/

	    }			/* end switch(ck_cd)  		*/

	    /*----------------------------------------------------------*
	     * 2.5. Stor the mora code.
	     *----------------------------------------------------------*/
	    moradata[*moralen] = mora_cd;	/* Set moradata  	*/
	    (*moralen)++;	/* Add lenght of moradata  		*/

	}			/* end for  			*/

	isw = C_SWOFF;		/* Set while loop switch flag  		*/

    }				/* end while  			*/

    /*------------------------------------------------------------------*
     * 3. Return. 
     *------------------------------------------------------------------*/
    return;

}

