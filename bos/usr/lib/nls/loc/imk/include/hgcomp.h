/* @(#)41	1.1  src/bos/usr/lib/nls/loc/imk/include/hgcomp.h, libkr, bos411, 9428A410j 5/25/92 15:36:28 */
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		hgcomp.h
 *
 * ORIGINS :		27
 *
 * (C) COPYRIGHT International Business Machines Corp.  1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define  C1     0x80
#define  C2     0x40
#define  C3     0x20
#define  C4     0x10
#define  V1     0x08
#define  V2     0x04
#define  V3     0x02
#define  V4     0x01
#define  C0     0xf0
#define  V0     0x0f

#define  State_0         0
#define  State_1         1
#define  State_2_1       2
#define  State_2_2       3
#define  State_2_3       4
#define  State_3_1       5
#define  State_3_2       6
#define  State_Delete    7       /* support deletion by eum-so  */

#define  Null            0x0000

#define  Ci_V_Null       0xffe0  /*  1111111111100000 */
#define  Ci_Fill_Fill    0x8021  /*  1000000000100001 */
#define  Ci_V_Fill       0x8001  /*  1000000000000001 */

#define  Ci_Mask         0x7c00  /*  0111110000000000 */
#define  V_Mask          0x03e0  /*  0000001111100000 */
#define  Cf_Mask         0x001f  /*  0000000000011111 */

#define  Jamo_O          0x01a0  /*  0000000110100000 */
#define  Jamo_EO         0x00e0  /*  0000000011100000 */
#define  Jamo_F_G        0x0002  /*  0000000000000010 */
#define  Jamo_F_N        0x0005  /*  0000000000000101 */
#define  Jamo_F_B        0x0012  /*  0000000000010010 */
#define  Jamo_F_S        0x0014  /*  0000000000010100 */
#define  Jamo_F_L        0x0009  /*  0000000000001001 */
#define  Jamo_I_GG       0x2c00  /*  0010110000000000 */
#define  Jamo_I_SS       0x5000  /*  0101000000000000 */

#define  Vowel_A         0xa4bf
#define  Vowel_AE        0xa4c0
#define  Vowel_EO        0xa4c3
#define  Vowel_E         0xa4c4
#define  Vowel_I         0xa4d3

#define  Conson_GG       0xa4a2
#define  Conson_SS       0xa4b6
#define  Conson_G        0xa4a1
#define  Conson_S        0xa4b5
#define  Conson_J        0xa4b8
#define  Conson_H        0xa4be

/* Support for the chars which are missed in KS code but they are used   */
/* as interim char to compose some specific char                         */
/* The composition codes of the missing chars are in Missing_Char_Tbl.   */
/* I assigned the temporary KS code to that chars from 0xada1 to 0xada5  */
/* These chars would be displayed as double byte space in IM Editor      */
/*                    				    ----- kedProc.c      */

#define  Mchar_Code_Start	0xada1
#define  Missing_Char_Num 	5

int Missing_Char_Tbl[] =
	{ 0xbde1, 0xd0a1, 0xd161, 0xd321, 0xde41 } ;

int Jamo_Attr_Tbl[] =
        { 0x90, 0x40, 0x00, 0x80, 0x00, 0x00, 0x40, 0x20, 0x80, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x90, 0x20, 0x00,
          0x90, 0x40, 0x40, 0x50, 0x20, 0x40, 0x40, 0x50, 0x50, 0x50,
          0x09, 0x03, 0x08, 0x02, 0x09, 0x03, 0x08, 0x02, 0x04, 0x00,
          0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x02, 0x08, 0x00, 0x03 } ;

int Ci_Tbl[] =
        { 0x2800, 0x2c00, 0x0000, 0x3000, 0x0000, 0x0000, 0x3400, 0x3800,
        /*  ㄱ      ㄲ      ㄱㅅ    ㄴ      ㄴㅈ    ㄴㅎ    ㄷ      ㄸ   */
          0x3c00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        /*  ㄹ      ㄹㄱ    ㄹㅁ    ㄹㅂ    ㄹㅅ    ㄹㅌ    ㄹㅍ    ㄹㅎ  */ 
          0x4000, 0x4400, 0x4800, 0x0000, 0x4c00, 0x5000, 0x5400, 0x5800,
        /*  ㅁ      ㅂ      ㅃ      ㅂㅅ    ㅅ      ㅆ      ㅇ      ㅈ    */ 
          0x5c00, 0x6000, 0x6400, 0x6800, 0x6c00, 0x7000 };
        /*  ㅉ      ㅊ      ㅋ      ㅌ      ㅍ      ㅎ  */

int V_Tbl[] =
        { 0x0040, 0x0060, 0x00a0, 0x00c0, 0x00e0, 0x0120, 0x0140, 0x0160,
        /*  ㅏ      ㅐ      ㅑ      ㅒ      ㅓ      ㅔ      ㅕ      ㅖ  */
          0x01a0, 0x01c0, 0x01e0, 0x0220, 0x0240, 0x0260, 0x02a0, 0x02c0,
        /*  ㅗ      와      왜      외      ㅛ      ㅜ      워      웨  */
          0x02e0, 0x0320, 0x0340, 0x0360, 0x03a0 }  ;
        /*  위      ㅠ      ㅡ      의     ㅣ     */

int Cf_Tbl[] =
        { 0x02, 0x03, 0x00, 0x05, 0x00, 0x00, 0x08, 0x00, 0x09, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x12, 0x00, 0x00, 0x14, 0x15,
          0x16, 0x17, 0x00, 0x18, 0x19, 0x1a, 0x1b, 0x1c } ;
int L_Tbl[] =
        { 0xa4a1, 0xa4b1, 0xa4b2, 0xa4b5, 0xa4bc, 0xa4bd, 0xa4be } ;

int C5_Split_Tbl[] =
        { 0x2800, 0x2800, 0x4c00, 0x3000, 0x5800, 0x7000, 0x3400, 0x3c00,
          0x2800, 0x4000, 0x4400, 0x4c00, 0x6800, 0x6c00, 0x7000, 0x4000,
          0x4400, 0x4c00, 0x4c00, 0x4c00, 0x5400, 0x5800, 0x6000, 0x6400,
          0x6800, 0x6c00, 0x7000 } ;
