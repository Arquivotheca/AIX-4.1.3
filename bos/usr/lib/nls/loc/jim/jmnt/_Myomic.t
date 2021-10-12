/* @(#)52	1.2.1.2  src/bos/usr/lib/nls/loc/jim/jmnt/_Myomic.t, libKJI, bos411, 9428A410j 5/23/93 20:51:29 */
/*
 * COMPONENT_NAME : Japanese Input Method - Kanji Monitor
 *
 * FUNCTIONS : _Myomic.t
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:         _Myomic.t
 *
 * DESCRIPTIVE NAME:    Yomi conversion table.
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              Kanji Monitor V1.0
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            The conversion table it is referd from conversion
 *                          program to convert double bytes PC kanji code
 *                          to single byte 7 bit yomikana code.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Table
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        NA. ( Include in the module _Myomic(). )
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         NA.
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            #include _Myomic.t
 *
 *  INPUT:              NA.
 *
 *  OUTPUT:             NA.
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
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Trace Block(TRB).
 *                              NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              NA.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      _Myomic.t  conversion table double byte PC kanji code
 *                                         to single byte 7 yomikana code.
 */

/* define constant table. for hiragana and katakana.            */

#define HDEL    0xff    /* nothing code.                        */

#define H_VU    0x1a    /* 'vu'                                 */
#define HSKA    0x1b    /* shift 'ka'                           */
#define HSKE    0x1c    /* shift 'ke'                           */

#define HPND    0x1e    /* pound.                               */

#define HS_A    0x1f    /* shift 'a'                            */
#define H__A    0x20    /* 'a'                                  */
#define HS_I    0x21    /* shift 'i'                            */
#define H__I    0x22    /* 'i'                                  */
#define HS_U    0x23    /* shift 'u'                            */
#define H__U    0x24    /* 'u'                                  */
#define HS_E    0x25    /* shift 'e'                            */
#define H__E    0x26    /* 'e'                                  */
#define HS_O    0x27    /* shift 'o'                            */
#define H__O    0x28    /* 'o'                                  */

#define H_KA    0x29    /* 'ka'                                 */
#define H_GA    0x2a    /* 'ga'                                 */
#define H_KI    0x2b    /* 'ki'                                 */
#define H_GI    0x2c    /* 'gi'                                 */
#define H_KU    0x2d    /* 'ku'                                 */
#define H_GU    0x2e    /* 'gu'                                 */
#define H_KE    0x2f    /* 'ke'                                 */
#define H_GE    0x30    /* 'ge'                                 */
#define H_KO    0x31    /* 'ko'                                 */
#define H_GO    0x32    /* 'go'                                 */

#define H_SA    0x33    /* 'ka'                                 */
#define H_ZA    0x34    /* 'ga'                                 */
#define H_SI    0x35    /* 'si'                                 */
#define H_ZI    0x36    /* 'zi'                                 */
#define H_SU    0x37    /* 'su'                                 */
#define H_ZU    0x38    /* 'zu'                                 */
#define H_SE    0x39    /* 'se'                                 */
#define H_ZE    0x3a    /* 'ze'                                 */
#define H_SO    0x3b    /* 'so'                                 */
#define H_ZO    0x3c    /* 'zo'                                 */

#define H_TA    0x3d    /* 'ta'                                 */
#define H_DA    0x3e    /* 'da'                                 */
#define H_TI    0x3f    /* 'ti'                                 */
#define H_DI    0x40    /* 'di'                                 */
#define HSTU    0x41    /* shift 'tu'                           */
#define H_TU    0x42    /* 'tu'                                 */
#define H_zu    0x43    /* 'Zu'                                 */
#define H_TE    0x44    /* 'te'                                 */
#define H_DE    0x45    /* 'de'                                 */
#define H_TO    0x46    /* 'to'                                 */
#define H_DO    0x47    /* 'do'                                 */

#define H_NA    0x48    /* 'na'                                 */
#define H_NI    0x49    /* 'ni'                                 */
#define H_NU    0x4a    /* 'nu'                                 */
#define H_NE    0x4b    /* 'ne'                                 */
#define H_NO    0x4c    /* 'no'                                 */

#define H_HA    0x4d    /* 'ha'                                 */
#define H_BA    0x4e    /* 'ba'                                 */
#define H_PA    0x4f    /* 'pa'                                 */
#define H_HI    0x50    /* 'hi'                                 */
#define H_BI    0x51    /* 'bi'                                 */
#define H_PI    0x52    /* 'pi'                                 */
#define H_HU    0x53    /* 'hu'                                 */
#define H_BU    0x54    /* 'bu'                                 */
#define H_PU    0x55    /* 'pu'                                 */
#define H_HE    0x56    /* 'he'                                 */
#define H_BE    0x57    /* 'be'                                 */
#define H_PE    0x58    /* 'pe'                                 */
#define H_HO    0x59    /* 'ho'                                 */
#define H_BO    0x5a    /* 'bo'                                 */
#define H_PO    0x5b    /* 'po'                                 */

#define H_MA    0x5c    /* 'ma'                                 */
#define H_MI    0x5d    /* 'mi'                                 */
#define H_MU    0x5e    /* 'mu'                                 */
#define H_ME    0x5f    /* 'me'                                 */
#define H_MO    0x60    /* 'mo'                                 */

#define HSYA    0x61    /* shift 'ya'                           */
#define H_YA    0x62    /* 'ya'                                 */
#define HSYU    0x63    /* shift 'yu'                           */
#define H_YU    0x64    /* 'yu'                                 */
#define HSYO    0x65    /* shift 'yo'                           */
#define H_YO    0x66    /* 'yo'                                 */

#define H_RA    0x67    /* 'ra'                                 */
#define H_RI    0x68    /* 'ri'                                 */
#define H_RU    0x69    /* 'ru'                                 */
#define H_RE    0x6a    /* 're'                                 */
#define H_RO    0x6b    /* 'ro'                                 */

#define HSWA    0x6c    /* shift 'wa'                           */
#define H_WA    0x6d    /* 'wa'                                 */
#define HL_I    0x6e    /* lower 'i'                            */
#define HL_E    0x6f    /* lower 'e'                            */
#define HL_O    0x70    /* lower 'o'                            */
#define H__N    0x71    /* 'n'                                  */

/* #(B) 1988.01.19. Flying Conversion Change */
#define HDAK    0xff    /* dakuten (0x72)                       */

/* #(B) 1988.01.19. Flying Conversion Change */
#define HMAL    0xff    /* maru    (0x73)                       */
#define HNOB    0x74    /* nobasi                               */

#define H__0    0x75    /* '0'                                  */
#define H__1    0x76    /* '1'                                  */
#define H__2    0x77    /* '2'                                  */
#define H__3    0x78    /* '3'                                  */
#define H__4    0x79    /* '4'                                  */
#define H__5    0x7a    /* '5'                                  */
#define H__6    0x7b    /* '6'                                  */
#define H__7    0x7c    /* '7'                                  */
#define H__8    0x7d    /* '8'                                  */
#define H__9    0x7e    /* '9'                                  */

#define HCOM    0x72    /* ','                                  */
#define HPER    0x73    /* '.'                                  */

/* define constant table. for alhabet.                          */

#define AADM    0x01    /* '!' exciamation mark.                */
#define ADBQ    0x02    /* '"' double quotation.                */
#define ASHP    0x03    /* '#' sharp.                           */
#define APCT    0x05    /* '%' percent.                         */
#define AAMP    0x06    /* '&' ampersand.                       */
#define ASQT    0x07    /*  '  single quotation.                */
#define ALPR    0x08    /* '(' left parentheses.                */
#define ARPR    0x09    /* ')' right parentheses.               */
#define AAST    0x0a    /* '*' asterisk.                        */
#define APLS    0x0b    /* '+' plus.                            */
#define ACOM    0x0c    /* ',' comma.                           */
#define AMIN    0x0d    /* '-' minus sign.                      */
#define APER    0x0e    /* '.' period.                          */
#define AFSL    0x0f    /* '/' forward slash.                   */
#define ACOL    0x10    /* ':' colon                            */
#define ASCL    0x11    /* ';' semicolon.                       */
#define ALAB    0x12    /* '<' left angle bracket.              */
#define AEQU    0x13    /* '=' equal sign.                      */
#define ARAB    0x14    /* '>' right angle bracket.             */
#define AQUE    0x15    /* '?' question mark.                   */
#define AATM    0x16    /* '@' at mark.                         */
/***********************  OLD.Mora Code Defined	*****************/
#if NOT_USED
#define AADM    0x21    /* '!' exciamation mark.                */
#define ADBQ    0x22    /* '"' double quotation.                */
#define ASHP    0x23    /* '#' sharp.                           */
#define APCT    0x25    /* '%' percent.                         */
#define AAMP    0x26    /* '&' ampersand.                       */
#define ASQT    0x27    /*  '  single quotation.                */
#define ALPR    0x28    /* '(' left parentheses.                */
#define ARPR    0x29    /* ')' right parentheses.               */
#define AAST    0x2a    /* '*' asterisk.                        */
#define APLS    0x2b    /* '+' plus.                            */
#define ACOM    0x2c    /* ',' comma.                           */
#define AMIN    0x2d    /* '-' minus sign.                      */
#define APER    0x2e    /* '.' period.                          */
#define AFSL    0x2f    /* '/' forward slash.                   */
#define ACOL    0x3a    /* ':' colon                            */
#define ASCL    0x3b    /* ';' semicolon.                       */
#define ALAB    0x3c    /* '<' left angle bracket.              */
#define AEQU    0x3d    /* '=' equal sign.                      */
#define ARAB    0x3e    /* '>' right angle bracket.             */
#define AQUE    0x3f    /* '?' question mark.                   */
#define AATM    0x40    /* '@' at mark.                         */
#endif
/****************************************************************/

#define A__0    0x30    /* '0'                                  */
#define A__1    0x31    /* '1'                                  */
#define A__2    0x32    /* '2'                                  */
#define A__3    0x33    /* '3'                                  */
#define A__4    0x34    /*  4                                   */
#define A__5    0x35    /* '5'                                  */
#define A__6    0x36    /* '6'                                  */
#define A__7    0x37    /* '7'                                  */
#define A__8    0x38    /* '8'                                  */
#define A__9    0x39    /* '9'                                  */

#define A__A    0x41    /* 'a' or 'A'                           */
#define A__B    0x42    /* 'b' or 'B'                           */
#define A__C    0x43    /* 'c' or 'C'                           */
#define A__D    0x44    /* 'd' or 'D'                           */
#define A__E    0x45    /* 'e' or 'E'                           */
#define A__F    0x46    /* 'f' or 'F'                           */
#define A__G    0x47    /* 'g' or 'G'                           */
#define A__H    0x48    /* 'h' or 'H'                           */
#define A__I    0x49    /* 'i' or 'I'                           */
#define A__J    0x4a    /* 'j' or 'J'                           */
#define A__K    0x4b    /* 'k' or 'K'                           */
#define A__L    0x4c    /* 'l' or 'L'                           */
#define A__M    0x4d    /* 'm' or 'M'                           */
#define A__N    0x4e    /* 'n' or 'N'                           */
#define A__O    0x4f    /* 'o' or 'O'                           */
#define A__P    0x50    /* 'p' or 'P'                           */
#define A__Q    0x51    /* 'q' or 'Q'                           */
#define A__R    0x52    /* 'r' or 'R'                           */
#define A__S    0x53    /* 's' or 'S'                           */
#define A__T    0x54    /* 't' or 'T'                           */
#define A__U    0x55    /* 'u' or 'U'                           */
#define A__V    0x56    /* 'v' or 'V'                           */
#define A__W    0x57    /* 'w' or 'W'                           */
#define A__X    0x58    /* 'x' or 'X'                           */
#define A__Y    0x59    /* 'y' or 'Y'                           */
#define A__Z    0x5a    /* 'z' or 'Z'                           */

/****** Defind E.Mora Code **************************************/
#define EM_A	0xE0	/* 'A' 					*/
#define EM_B	0xE1	/* 'B' 					*/
#define EM_C	0xE2	/* 'C' 					*/
#define EM_D	0xE3	/* 'D' 					*/
#define EM_E	0xE4	/* 'E' 					*/
#define EM_F	0xE5	/* 'F' 					*/
#define EM_G	0xE6	/* 'G' 					*/
#define EM_H	0xE7	/* 'H' 					*/
#define EM_I	0xE8	/* 'I' 					*/
#define EM_J	0xE9	/* 'J' 					*/
#define EM_K	0xEA	/* 'K' 					*/
#define EM_L	0xEB	/* 'L' 					*/
#define EM_M	0xEC	/* 'M' 					*/
#define EM_N	0xED	/* 'N' 					*/
#define EM_O	0xEE	/* 'O' 					*/
#define EM_P	0xEF	/* 'P' 					*/
#define EM_Q	0xF0	/* 'Q' 					*/
#define EM_R	0xF1	/* 'R' 					*/
#define EM_S	0xF2	/* 'S' 					*/
#define EM_T	0xF3	/* 'T' 					*/
#define EM_U	0xF4	/* 'U' 					*/
#define EM_V	0xF5	/* 'V' 					*/
#define EM_W	0xF6	/* 'W' 					*/
#define EM_X	0xF7	/* 'X' 					*/
#define EM_Y	0xF8	/* 'Y' 					*/
#define EM_Z	0xF9	/* 'Z' 					*/

#define EM_a	0xB5	/* 'a'       				*/
#define EM_b	0xB6	/* 'b'       				*/
#define EM_c	0xB7	/* 'c'       				*/
#define EM_d	0xB8	/* 'd'       				*/
#define EM_e	0xB9	/* 'e'       				*/
#define EM_f	0xBA	/* 'f'       				*/
#define EM_g	0xBB	/* 'g'       				*/
#define EM_h	0xBC	/* 'h'       				*/
#define EM_i	0xBD	/* 'i'       				*/
#define EM_j	0xBE	/* 'j'       				*/
#define EM_k	0xBF	/* 'k'       				*/
#define EM_l	0xC0	/* 'l'       				*/
#define EM_m	0xC1	/* 'm'       				*/
#define EM_n	0xC2	/* 'n'       				*/
#define EM_o	0xC3	/* 'o'       				*/
#define EM_p	0xC4	/* 'p'       				*/
#define EM_q	0xC5	/* 'q'       				*/
#define EM_r	0xC6	/* 'r'       				*/
#define EM_s	0xC7	/* 's'       				*/
#define EM_t	0xC8	/* 't'       				*/
#define EM_u	0xC9	/* 'u'       				*/
#define EM_v	0xCA	/* 'v'       				*/
#define EM_w	0xCB	/* 'w'       				*/
#define EM_x	0xCC	/* 'x'       				*/
#define EM_y	0xCD	/* 'y'       				*/
#define EM_z	0xCE	/* 'z'       				*/

/* the conversion table for hiragana katakana code              */
static  uchar   kana_tbl[3][16*12] =
{
/* kana_tbl[0] : 00 -- 0f ( DBCS CODE : 8140 -- 814f )          */
HDEL,HDEL,HDEL,ACOM,APER,HDEL,ACOL,ASCL,
AQUE,AADM,HDAK,HMAL,HDEL,HDEL,HDEL,HDEL,

/* kana_tbl[0] : 10 -- 1f ( DBCS CODE : 8150 -- 815f )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HNOB,HDEL,HDEL,AFSL,HDEL,

/* kana_tbl[0] : 20 -- 2f ( DBCS CODE : 8160 -- 816f )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,ALPR,ARPR,HDEL,HDEL,HDEL,HDEL,HDEL,

/* kana_tbl[0] : 30 -- 3f ( DBCS CODE : 8170 -- 817f )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,APLS,AMIN,HDEL,HDEL,HDEL,

/* kana_tbl[0] : 40 -- 4f ( DBCS CODE : 8180 -- 818f )          */
HDEL,AEQU,HDEL,ALAB,ARAB,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* kana_tbl[0] : 50 -- 5f ( DBCS CODE : 8190 -- 819f )          */
HDEL,HDEL,HPND,APCT,ASHP,AAMP,AAST,AATM,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* kana_tbl[0] : 60 -- 6f ( DBCS CODE : 81a0 -- 81af )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* kana_tbl[0] : 70 -- 7f ( DBCS CODE : 81b0 -- 81bf )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* kana_tbl[0] : 80 -- 8f ( DBCS CODE : 81c0 -- 81cf )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* kana_tbl[0] : 90 -- 9f ( DBCS CODE : 81d0 -- 81df )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* kana_tbl[0] : a0 -- af ( DBCS CODE : 81e0 -- 81ef )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* kana_tbl[0] : b0 -- bf ( DBCS CODE : 81f0 -- 81ff )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,


/* kana_tbl[1] : 00 -- 0f ( DBCS CODE : 8240 -- 824f )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,H__0,

/* kana_tbl[1] : 10 -- 1f ( DBCS CODE : 8250 -- 825f )          */
H__1,H__2,H__3,H__4,H__5,H__6,H__7,H__8,
H__9,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* kana_tbl[1] : 20 -- 2f ( DBCS CODE : 8260 -- 826f )          */
EM_A,EM_B,EM_C,EM_D,EM_E,EM_F,EM_G,EM_H,
EM_I,EM_J,EM_K,EM_L,EM_M,EM_N,EM_O,EM_P,

/* kana_tbl[1] : 30 -- 3f ( DBCS CODE : 8270 -- 827f )          */
EM_Q,EM_R,EM_S,EM_T,EM_U,EM_V,EM_W,EM_X,
EM_Y,EM_Z,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* kana_tbl[1] : 40 -- 4f ( DBCS CODE : 8280 -- 828f )          */
HDEL,EM_a,EM_b,EM_c,EM_d,EM_e,EM_f,EM_g,
EM_h,EM_i,EM_j,EM_k,EM_l,EM_m,EM_n,EM_o,

/* kana_tbl[1] : 50 -- 5f ( DBCS CODE : 8290 -- 829f )          */
EM_p,EM_q,EM_r,EM_s,EM_t,EM_u,EM_v,EM_w,
EM_x,EM_y,EM_z,HDEL,HDEL,HDEL,HDEL,HS_A,

/* kana_tbl[1] : 60 -- 6f ( DBCS CODE : 82a0 -- 82af )          */
H__A,HS_I,H__I,HS_U,H__U,HS_E,H__E,HS_O,
H__O,H_KA,H_GA,H_KI,H_GI,H_KU,H_GU,H_KE,

/* kana_tbl[1] : 70 -- 7f ( DBCS CODE : 82b0 -- 82bf )          */
H_GE,H_KO,H_GO,H_SA,H_ZA,H_SI,H_ZI,H_SU,
H_ZU,H_SE,H_ZE,H_SO,H_ZO,H_TA,H_DA,H_TI,

/* kana_tbl[1] : 80 -- 8f ( DBCS CODE : 82c0 -- 82cf )          */
H_DI,HSTU,H_TU,H_zu,H_TE,H_DE,H_TO,H_DO,
H_NA,H_NI,H_NU,H_NE,H_NO,H_HA,H_BA,H_PA,

/* kana_tbl[1] : 90 -- 9f ( DBCS CODE : 82d0 -- 82df )          */
H_HI,H_BI,H_PI,H_HU,H_BU,H_PU,H_HE,H_BE,
H_PE,H_HO,H_BO,H_PO,H_MA,H_MI,H_MU,H_ME,

/* kana_tbl[1] : a0 -- af ( DBCS CODE : 82e0 -- 82ef )          */
H_MO,HSYA,H_YA,HSYU,H_YU,HSYO,H_YO,H_RA,
H_RI,H_RU,H_RE,H_RO,HSWA,H_WA,HL_I,HL_E,

/* kana_tbl[1] : b0 -- bf ( DBCS CODE : 82f0 -- 82ff )          */
HL_O,H__N,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,


/* kana_tbl[2] : 00 -- 0f ( DBCS CODE : 8340 -- 834f )          */
HS_A,H__A,HS_I,H__I,HS_U,H__U,HS_E,H__E,
HS_O,H__O,H_KA,H_GA,H_KI,H_GI,H_KU,H_GU,

/* kana_tbl[2] : 10 -- 1f ( DBCS CODE : 8350 -- 835f )          */
H_KE,H_GE,H_KO,H_GO,H_SA,H_ZA,H_SI,H_ZI,
H_SU,H_ZU,H_SE,H_ZE,H_SO,H_ZO,H_TA,H_DA,

/* kana_tbl[2] : 20 -- 2f ( DBCS CODE : 8360 -- 836f )          */
H_TI,H_DI,HSTU,H_TU,H_zu,H_TE,H_DE,H_TO,
H_DO,H_NA,H_NI,H_NU,H_NE,H_NO,H_HA,H_BA,

/* kana_tbl[2] : 30 -- 3f ( DBCS CODE : 8370 -- 837f )          */
H_PA,H_HI,H_BI,H_PI,H_HU,H_BU,H_PU,H_HE,
H_BE,H_PE,H_HO,H_BO,H_PO,H_MA,H_MI,HDEL,

/* kana_tbl[2] : 40 -- 4f ( DBCS CODE : 8380 -- 838f )          */
H_MU,H_ME,H_MO,HSYA,H_YA,HSYU,H_YU,HSYO,
H_YO,H_RA,H_RI,H_RU,H_RE,H_RO,HSWA,H_WA,

/* kana_tbl[2] : 50 -- 5f ( DBCS CODE : 8390 -- 839f )          */
HL_I,HL_E,HL_O,H__N,H_VU,HSKA,HSKE,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* kana_tbl[2] : 60 -- 6f ( DBCS CODE : 83a0 -- 83af )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* kana_tbl[2] : 70 -- 7f ( DBCS CODE : 83b0 -- 83bf )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* kana_tbl[2] : 80 -- 8f ( DBCS CODE : 83c0 -- 83cf )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* kana_tbl[2] : 90 -- 9f ( DBCS CODE : 83d0 -- 83df )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* kana_tbl[2] : a0 -- af ( DBCS CODE : 83e0 -- 83ef )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* kana_tbl[2] : b0 -- bf ( DBCS CODE : 83f0 -- 83ff )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL
};

/* the conversion table for alphabet code */
static  uchar   alph_tbl[2][16*12] =
{
/* alph_tbl[0] : 00 -- 0f ( DBCS CODE : 8140 -- 814f )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,ACOL,ASCL,
AQUE,AADM,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* alph_tbl[0] : 10 -- 1f ( DBCS CODE : 8150 -- 815f )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,AFSL,HDEL,

/* alph_tbl[0] : 20 -- 2f ( DBCS CODE : 8160 -- 816f )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,ALPR,ARPR,HDEL,HDEL,HDEL,HDEL,HDEL,

/* alph_tbl[0] : 30 -- 3f ( DBCS CODE : 8170 -- 817f )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,APLS,AMIN,HDEL,HDEL,HDEL,

/* alph_tbl[0] : 40 -- 4f ( DBCS CODE : 8180 -- 818f )          */
HDEL,AEQU,HDEL,ALAB,ARAB,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* alph_tbl[0] : 50 -- 5f ( DBCS CODE : 8190 -- 819f )          */
HDEL,HDEL,HDEL,APCT,ASHP,AAMP,AAST,AATM,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* alph_tbl[0] : 60 -- 6f ( DBCS CODE : 81a0 -- 81af )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* alph_tbl[0] : 70 -- 7f ( DBCS CODE : 81b0 -- 81bf )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* alph_tbl[0] : 80 -- 8f ( DBCS CODE : 81c0 -- 81cf )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* alph_tbl[0] : 90 -- 9f ( DBCS CODE : 81d0 -- 81df )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* alph_tbl[0] : a0 -- af ( DBCS CODE : 81e0 -- 81ef )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* alph_tbl[0] : b0 -- bf ( DBCS CODE : 81f0 -- 81ff )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,


/* alph_tbl[1] : 00 -- 0f ( DBCS CODE : 8240 -- 824f )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,A__0,

/* alph_tbl[1] : 10 -- 1f ( DBCS CODE : 8250 -- 825f )          */
A__1,A__2,A__3,A__4,A__5,A__6,A__7,A__8,
A__9,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* alph_tbl[1] : 20 -- 2f ( DBCS CODE : 8260 -- 826f )          */
A__A,A__B,A__C,A__D,A__E,A__F,A__G,A__H,
A__I,A__J,A__K,A__L,A__M,A__N,A__O,A__P,

/* alph_tbl[1] : 30 -- 3f ( DBCS CODE : 8270 -- 827f )          */
A__Q,A__R,A__S,A__T,A__U,A__V,A__W,A__X,
A__Y,A__Z,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* alph_tbl[1] : 40 -- 4f ( DBCS CODE : 8280 -- 828f )          */
HDEL,A__A,A__B,A__C,A__D,A__E,A__F,A__G,
A__H,A__I,A__J,A__K,A__L,A__M,A__N,A__O,

/* alph_tbl[1] : 50 -- 5f ( DBCS CODE : 8290 -- 829f )          */
A__P,A__Q,A__R,A__S,A__T,A__U,A__V,A__W,
A__X,A__Y,A__Z,HDEL,HDEL,HDEL,HDEL,HDEL,

/* alph_tbl[1] : 60 -- 6f ( DBCS CODE : 82a0 -- 82af )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* alph_tbl[1] : 70 -- 7f ( DBCS CODE : 82b0 -- 82bf )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* alph_tbl[1] : 80 -- 8f ( DBCS CODE : 82c0 -- 82cf )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* alph_tbl[1] : 90 -- 9f ( DBCS CODE : 82d0 -- 82df )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* alph_tbl[1] : a0 -- af ( DBCS CODE : 82e0 -- 82ef )          */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,

/* alph_tbl[1] : b0 -- bf ( DBCS CODE : 82f0 -- 82ff )           */
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,
HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL,HDEL
};

