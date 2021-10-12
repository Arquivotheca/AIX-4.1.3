/* @(#)48	1.1  src/bos/diag/tu/sun/cl5xx.h, tu_sunrise, bos411, 9437A411a 3/28/94 17:47:59 */
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/**************************************************************************/
/*  File: CL5xx.h                                                         */
/**************************************************************************/
/*  C-Cube Microsystems                                                   */
/*  1778 McCarthy Blvd.                                                   */
/*  Milpitas, CA 95035  U.S.A.                                            */
/*  Tel: (408) 944-6300  FAX: (408) 944-6314                              */
/**************************************************************************/
/*
	History -
		Written by L.S.         May-June, 1992
		Updated for CL5xx       February, 1993 

	Description -
		CL5xx Initialization Tables
*/

/* Matrix Conversion constants for RGB-to-YUV mode */

#define R2Y     0x0133      /* compression direction: RGB->YUV */
#define G2Y     0x0259
#define B2Y     0x0074
#define R2U     0x0F54
#define G2U     0x0EAD
#define B2U     0x01FF
#define R2V     0x01FF
#define G2V     0x0E53
#define B2V     0x0FAE

#define Y2R     0x0400      /* decompression direction: YUV->RGB    */
#define U2R     0x0FFE
#define V2R     0x059C
#define Y2G     0x0400
#define U2G     0x0EA2
#define V2G     0x0D23
#define Y2B     0x0400
#define U2B     0x071B
#define V2B     0x0FFD

/* Pixel Mode:  MONO     422      444      4444    RGB_422   444_422 */

unsigned short Init_1_Values[2][NumPixModes] =
/* comp */    {0x0002L, 0x0042L, 0x0040L, 0x0042L, 0x003CL, 0x003CL,
/* decomp */   0x003EL, 0x003EL, 0x003EL, 0x003EL, 0x003EL, 0x003EL};

unsigned short Init_2_Values[2][NumPixModes] =
/* comp */    {0x0001L, 0x0001L, 0x0000L, 0x0001L, 0x003DL, 0x003DL,
/* decomp */   0x0037L, 0x0037L, 0x0037L, 0x0037L, 0x0037L, 0x0037L};

unsigned short Init_3_Values[2][NumPixModes] =
/* comp */    {0x0141L, 0x0081L, 0x007FL, 0x0081L, 0x007BL, 0x007BL,
/* decomp */   0x01FFL, 0x01FFL, 0x01FFL, 0x01FFL, 0x01FFL, 0x01FFL};

unsigned short Init_4_Values[2][NumPixModes] =
/* comp */    {0x00F7L, 0x00F7L, 0x00F5L, 0x00F7L, 0x00F1L, 0x00F1L,
/* decomp */   0x0049L, 0x0049L, 0x0049L, 0x0049L, 0x0049L, 0x0049L};

unsigned short Init_5_Values[2][NumPixModes] =
/* comp */    {0x0000L, 0x0000L, 0x000EL, 0x0000L, 0x000AL, 0x000AL,
/* decomp */   0x0005L, 0x0005L, 0x0005L, 0x0005L, 0x0005L, 0x0005L};

unsigned short Init_6_Values[2][NumPixModes] =
/* comp */    {0x0000L, 0x0000L, 0x0000L, 0x0000L, 0x0000L, 0x0000L,
/* decomp */   0x0000L, 0x0000L, 0x0000L, 0x0000L, 0x0000L, 0x0000L};

unsigned short Init_7_Values[2][NumPixModes] =
/* comp */    {0x005DL, 0x001DL, 0x001BL, 0x001DL, 0x0017L, 0x0017L,
/* decomp */   0x0022L, 0x0022L, 0x0022L, 0x0022L, 0x0022L, 0x0022L};

unsigned short QuantSync_Values[2][NumPixModes] =
/* comp */    {0x0406L, 0x0406L, 0x0404L, 0x0406L, 0x0400L, 0x0400L,
/* decomp */   0x043EL, 0x043EL, 0x043EL, 0x043EL, 0x043EL, 0x043EL};

unsigned short QuantYCSequence_Values[2][NumPixModes] =
/* comp */    {0x2000L, 0x2099L, 0x2044L, 0x2055L, 0x2099L, 0x2099L,
/* decomp */   0x2000L, 0x2033L, 0x2088L, 0x20AAL, 0x2033L, 0x2033L};

unsigned short QuantABSequence_Values[2][NumPixModes] =
/* comp */    {0x0000L, 0x0000L, 0x0088L, 0x0099L, 0x0000L, 0x0000L,
/* decomp */   0x0000L, 0x0000L, 0x0011L, 0x0033L, 0x0000L, 0x0000L};

unsigned short VideoLatency_Values[2][NumPixModes] =
/* comp */    {0x00BFL, 0x017FL, 0x0181L, 0x017FL, 0x0185L, 0x0185L,
/* decomp */   0x00BFL, 0x017FL, 0x017FL, 0x017FL, 0x017FL, 0x017FL};

unsigned short ConfigMode_Values[NumPixModes] =
	      {0x0080L, 0x0010L, 0x0040L, 0x0060L, 0x0030L, 0x0020L};

unsigned short HuffTableSequence_Values[NumPixModes] =
	      {0x0000L, 0x00CCL, 0x0000L, 0x0000L, 0x00CCL, 0x00CCL};

unsigned short DPCM_SeqHigh_Values[NumPixModes] =
	      {0x0000L, 0x0088L, 0x0044L, 0x00CCL, 0x0088L, 0x0088L};

unsigned short DPCM_SeqLow_Values[NumPixModes] =
		  {0x0000L, 0x0044L, 0x0012L, 0x00AAL, 0x0044L, 0x0044L};

unsigned short _550CoderAttr_Values[NumPixModes] =
		  {0x0001L, 0x0004L, 0x0003L, 0x0004L, 0x0004L, 0x0004L};

unsigned short CodingIntH_Values[NumPixModes] =
	      {0x0000L, 0x0000L, 0x0000L, 0x0000L, 0x0000L, 0x0000L};

unsigned short CodingIntL_Values[NumPixModes] =
	      {0x0100L, 0x0100L, 0x0100L, 0x0100L, 0x0008L, 0x0100L};
	      
unsigned short DecLength_Values[NumPixModes] =
	      {0x0008L, 0x0008L, 0x0006L, 0x0008L, 0x0008L, 0x0008L};

unsigned short DecCodeOrder_Values[NumPixModes] =
	      {0x0001L, 0x0001L, 0x0001L, 0x0001L, 0x0001L, 0x0001L};

unsigned short QuantABSelect_Values[NumPixModes] =
	      {0x0001L, 0x0001L, 0x0001L, 0x0001L, 0x0001L, 0x0001L};

unsigned short DCT_Values[NumPixModes] =
	      {0x0000L, 0x0000L, 0x000EL, 0x0000L, 0x000AL, 0x0000L};

/* CL560 registers */

unsigned short _560CoderAttr_Values[NumPixModes] =
		  {0x0001L, 0x0004L, 0x0003L, 0x0004L, 0x0004L, 0x0004L};

unsigned short _560CoderSync_Values[NumPixModes] =
		  {0x0100L, 0x01c0L, 0x01c2L, 0x01c0L, 0x01c6L, 0x01C0L};
