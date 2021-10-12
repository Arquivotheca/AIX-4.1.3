static char sccsid[] = "@(#)31	1.1  src/bos/diag/tu/sun/makeq.c, tu_sunrise, bos411, 9437A411a 3/28/94 17:46:04";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: CountBits
 *		MakeCL5xx_Q_Table
 *		MakeJPEG_Q_Table
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
/*  File: MakeQ.c                                                         */
/**************************************************************************/
/*  C-Cube Microsystems                                                   */
/*  1778 McCarthy Blvd.                                                   */
/*  Milpitas, CA 95035  U.S.A.                                            */
/*  Tel: (408) 944-6300  FAX: (408) 944-6314                              */
/**************************************************************************/
/*
	History -
		Written by L.S.         May-June, 1992
		Modified for CL5xx      February, 1993

	Description -
		This module contains functions for creating Quantization tables
	for use with CL5xx family devices.

	The JPEG baseline system specifies a Quantization Table as a list of
	64 8-bit entries, each entry can range from 1-255. The list is ordered
	corresponding to the zig-zag scan ordering of the DCT coefficients,
	as shown below.

		Q0, Q1, Q5, Q6, ...
		Q2, Q4, Q7, ...
		Q3, Q8, ...
		Q9, ...
		.
		.

	Up to four such lists can be specified, one for each component.

	In order to use these quantization tables with the CL5xx, it is first
	necessary to compute machine-specific versions of the tables for
	compression or decompression. The compress and decompress tables are
	different from each other, so Direction must be specified before
	using this module.

	The interface to this module consists of the following global lists:

	1)  JPEG Quantization Table lists, in zig-zag order.

		JPEG_Q_Table1[64],  JPEG_Q_Table2[64]
		JPEG_Q_Table3[64],  JPEG_Q_Table4[64];

	2)  CL550-specific Q tables for loading into CL5xx

		CL5xx_Q_Table1[64], CL5xx_Q_Table2[64]
		CL5xx_Q_Table3[64], CL5xx_Q_Table4[64]

	And the following utility functions:

	1)  MakeJPEG_Q_Table() -
		Generates a JPEG_Q_Table from a Visibility Table, in non-zig-zag
		order, and a Q Factor scalar. This applies mostly to compression
		applications. For decompression applications the JPEG_Q_Tables
		are initialized from the JPEG file header.

   2)  MakeCL5xx_Q_Table() -
	   Generates a CL5xx Q Table for compress or decompress based on
	   a JPEG_Q_Table list. This function is Direction-dependent.
*/

#include <stdio.h>
#include <math.h>
#include "pc5xx.h"
#include "makeq.h"

/****** Static Prototypes *******************************************/

static short CountBits( unsigned short quant );

/****** Globals *****************************************************/

unsigned char JPEG_Q_Table1[64];     /* JPEG integer lists 1-255 */
unsigned char JPEG_Q_Table2[64];     /* for import/export */
unsigned char JPEG_Q_Table3[64];
unsigned char JPEG_Q_Table4[64];

unsigned short CL5xx_Q_Table1[64];   /* CL550 Machine-Specific */
unsigned short CL5xx_Q_Table2[64];   /* for loading into CL550 */
unsigned short CL5xx_Q_Table3[64];
unsigned short CL5xx_Q_Table4[64];

short ZigZag[64] = {                    /* zig-zag scan order */
	 0,  1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63 };

/******** Functions ****************************************************/

/*  MakeJPEG_Q_Table()

	This function provides the means of creating a JPEG_Q_Table from
	an Input_Visi_Table. A Visi_Table is defined as a matrix of JPEG
	quantizers in non-zig-zag order (ordered from right-to-left,
	top-to-bottom) as they appear in Appendix K of the JPEG Standard
	Specification. Specifying Visi_Tables in non-zig-zag order allows
	for easier editing.

	Before saving to the JPEG_Q_Table, each Input_Visi_Table value
	is multiplied by (QFactor/50). QFactor provides the means to vary
	compression ratio using a single term, rather than modifying all
	64 values. A QFactor of 50 implies unity scaling.

	Finally, the JPEG_Q_Table is stored in zig-zag order, as required
	by the JPEG Interchange Format.

	Default sets of Input_Visi_Tables (for Luminance and Chrominance) are
	included in MakeQ.h
*/

void MakeJPEG_Q_Table
(
	short           QFactor,           /* scalar */
	short           *Input_Visi_Table, /* visibility matrix, non-zig-zag */
	unsigned char   *JPEG_Q_Table      /* output list. zig-zagged */
)
{
	short i;
	unsigned short temp;

	for( i = 0; i < 64; i++)
    {
		/* read input array in zig-zag order, scaling the results
		   and placing them into JPEG_Q_Table */

		temp = (short)((Input_Visi_Table[ ZigZag[i] ]
			   * QFactor / 50.0) + 0.5 );

		if( temp < 1)
			temp = 1;   /* minimum value is 1 */
		if( temp > 255)
			temp = 255;  /* maximum value is 255 */

		JPEG_Q_Table[i] = (unsigned char) temp;
	}
}

/*******************************************************************/

/*  MakeCL5xx_Q_Table()

	This function builds a machine-useable form of the JPEG_Q_Table
	for either compression or decompression.
*/

void MakeCL5xx_Q_Table
(
	unsigned char   *JPEG_Q_Table,
	unsigned short  *CL550_Q_Table
)
{

	short i, j;
	short qshift;
	unsigned short Temp;
	unsigned short UnZigged[64];
	unsigned short Temp_Q_Table[64];

/* 1. first, the JPEG_Q_Table is unzigged from its zig-zag order */

	for( i = 0 ; i < 64 ; i++ )
		UnZigged[ ZigZag[i] ] = (unsigned short) JPEG_Q_Table[i];

/* 2. From the UnZigged table, generate tables for the CL550 */

	if( Direction == COMPRESS )         /* build a compression table */
	{
		for( i = 0; i < 64 ; i++ )
		{
			Temp_Q_Table[i] =
				(unsigned short)(CompKFactor[i] / (UnZigged[i] << 1) + .5);

			if ( Temp_Q_Table[i] > (unsigned short) 32767. )
				Temp_Q_Table[i] = (unsigned short)32767.0;
		}
	}
	else    /* Direction == DECOMPRESS, build a decompression table */
	{
		for( i = 0; i < 64; i++ )
		{
			Temp_Q_Table[i] = (unsigned short)(DecompKFactor[i]
				* UnZigged[i]
				* pow( (double)2,(double)CountBits(UnZigged[i]) ) + .5);

			/* truncate to 13 significant bits */
			Temp_Q_Table[i] &= 0xfffc;

			qshift = 7 - CountBits( UnZigged[i] );

			Temp_Q_Table[i] = (qshift<<13) + (Temp_Q_Table[i]>>2);
							  /* exponent  +   mantissa */
		}
	}

/* 3.   Convert the Temp_Q_Table to CL550 ordering using a corner-to-corner
		transposition */

	for ( i=0; i<8; i++ )
		for ( j=0; j<8; j++ )
			CL550_Q_Table[i*8+j] = Temp_Q_Table[i+j*8];
}

/***************************************************************************/

/*  CountBits() -
	This function returns the number of significant bits for a given input
	value.
*/

static short CountBits( unsigned short quant )
{
    int i;
    unsigned char mask;

    for (i=0, mask=0x80; i<8; i++,mask>>=1)
        if (mask & quant) return(i);

}
