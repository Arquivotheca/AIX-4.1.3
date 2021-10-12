static char sccsid[] = "@(#)30  1.2  src/bos/diag/tu/sun/makehuff.c, tu_sunrise, bos411, 9437A411a 7/20/94 17:13:02";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: ClearTables
 *              DropDefaultHuffmanTables
 *              GenerateCodeSizeTable
 *              GenerateHuffmanCodeTable
 *              LoadDefaultTable
 *              MakeCL550DecodeTable
 *              MakeCL550EncodeTable
 *              MakeCL560HuffmanTable
 *              MakeCL5xxHuffmanTable
 *              OrderHuffmanCodes
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
/*  File: MakeHuff.c                                                      */
/**************************************************************************/
/*  C-Cube Microsystems                                                   */
/*  1778 McCarthy Blvd.                                                   */
/*  Milpitas, CA 95035  U.S.A.                                            */
/*  Tel: (408) 944-6300  FAX: (408) 944-6314                              */
/**************************************************************************/
/*
        History -
                Written by L.S.     May-June, 1992
                Revised             August, 1992
                Modified for CL5xx  February, 1993

        Description -
        The functions contained herein will generate the machine-specific
        forms of the Huffman tables required for CL5xx initialization.
        The CL5xx's Huffman tables can either be loaded with default values
        (this is handy for compression) or be loaded from user-specified
        lists of Bits and Values. In the JPEG syntax, the Huffman table
        information is passed in two lists:

                BITS -  16 bytes, each byte in the list represents the number of
                                Huffman codes of that bit size, in order of increasing bit
                                size.

          VALUES -  A list of the 8 bit values which are assigned to each code.
                                This list of values is variable-length and must be placed
                                immediately following the BITS list.

        To use this information in a JPEG encode or decode process, it is
        first necessary to compute the Encoding Procedure Code Tables from
        the lists of BITS and VALUES. This process is described in the
        JPEG Standard Specication ( ISO/IEC #10918-1 ), Appendix C.

        Before the CL5xx can be used for compression or decompression,
        machine-specific versions of the Encoding Procedure Code Tables need
        to be built.

        All four Huffman RAMs be must be initialized. If only two tables are
        used, the other tables should be loaded with zeroes, and
        all unused locations within active tables should be loaded with zeroes.
        These tables are:

                CL5xxHuffmanYDC     - Luminance DC (32 words max)
                CL5xxHuffmanYAC     - Luminance AC (704 words max)
                CL5xxHuffmanCDC     - Chrominance DC (32 words max)
                CL5xxHuffmanCAC     - Chrominance AC (704 words max)

        These tables are declared as globals below.
*/

#include <stdio.h>
#include "pc5xx.h"
#include "makehuff.h"

/**** Static Prototypes *********************************************/

void ClearTables(void);
static void LoadDefaultTable
(
        unsigned short *InputList,
        unsigned short *OutputList
);
static void GenerateCodeSizeTable
(
        unsigned char   *Bits,
        short           *CodeSizeTable,
        short           *TotalCodes
);
static void GenerateHuffmanCodeTable
(
        short           *CodeSizeTable,
        unsigned short  *HuffmanCodeTable
);
static void OrderHuffmanCodes
(
        unsigned char   *BitsNVals,
        short           *CodeSizeTable,
        unsigned short  *HuffmanCodeTable,
        unsigned short  *EncodingProcedureCodeTable,
        unsigned short  *EncodingProcedureSizeTable,
        short           TotalCodes
);
static void MakeCL550EncodeTable
(
        unsigned short  *EncodingProcedureCodeTable,
        unsigned short  *EncodingProcedureSizeTable,
        unsigned short  *CL5xxHuffmanTable,
        short           MaxNumCodes
);
static void MakeCL550DecodeTable
(
        unsigned char   *ValsList,
        short           *CodeSizeTable,
        unsigned short  *HuffmanCodeTable,
        unsigned short  *CL5xxHuffmanTable,
        short           TotalCodes
);
static void MakeCL560HuffmanTable
(
        unsigned short  *EncodingProcedureCodeTable,
        unsigned short  *EncodingProcedureSizeTable,
        unsigned short  *CL5xxHuffmanTable,
        short           ACDC
);
static short TestTable(
        FILE * outfile,
        unsigned short DeviceBase,
        unsigned short *HuffmanTable,
        short count,
        unsigned short mask
);

/****  Global Data  ******************************************************/

unsigned short CL5xxHuffmanYDC[128];
unsigned short CL5xxHuffmanCDC[128];
unsigned short CL5xxHuffmanYAC[1024];
unsigned short CL5xxHuffmanCAC[1024];

/*************************************************************************/

/*  DropDefaultHuffmanTables()
        This function loads the tables above using default tables in MakeHuff.h.
        These tables are the example tables from the JPEG specification,
        Appendix K, and they are used here as defaults. These tables perform
        well over a variety of image data types.

        This function can be called at compression time to drop-in default
        huffman tables from MakeHuff.h. Also in the header file are the default
        lists of bits_and_values that can be dropped into JPEG file headers.
*/

void DropDefaultHuffmanTables(void)
{
        short i;

        ClearTables();              /* zero the tables */

        if(DeviceType == CL550)
        {  /* Drop-in CL550 tables. Direction-dependent */

                if(Direction == COMPRESS)   /* fill with default encode tables */
                {
                        LoadDefaultTable( CompDefaultHuffmanYAC, CL5xxHuffmanYAC );
                        LoadDefaultTable( CompDefaultHuffmanYDC, CL5xxHuffmanYDC );
                        LoadDefaultTable( CompDefaultHuffmanCAC, CL5xxHuffmanCAC );
                        LoadDefaultTable( CompDefaultHuffmanCDC, CL5xxHuffmanCDC );
                }
                else                        /* fill with default decode tables */
                {
                        LoadDefaultTable( DecompDefaultHuffmanYAC, CL5xxHuffmanYAC );
                        LoadDefaultTable( DecompDefaultHuffmanYDC, CL5xxHuffmanYDC );
                        LoadDefaultTable( DecompDefaultHuffmanCAC, CL5xxHuffmanCAC );
                        LoadDefaultTable( DecompDefaultHuffmanCDC, CL5xxHuffmanCDC );
                }
        }
        else
        {   /* Drop 560/570 tables. (Direction-independent) */

                LoadDefaultTable( Default_560_Huffman_YAC, CL5xxHuffmanYAC );
                LoadDefaultTable( Default_560_Huffman_YDC, CL5xxHuffmanYDC );
                LoadDefaultTable( Default_560_Huffman_CAC, CL5xxHuffmanCAC );
                LoadDefaultTable( Default_560_Huffman_CDC, CL5xxHuffmanCDC );

        }
}

/********************************************************************/

void ClearTables(void)
{   /* this function zeroes the Huffman arrays above. */

        short i;

        for(i = 0; i < 32; i++)
        {
                CL5xxHuffmanYDC[i] = 0;
                CL5xxHuffmanCDC[i] = 0;
        }
        for(i = 0; i < 704; i++)
        {
                CL5xxHuffmanYAC[i] = 0;
                CL5xxHuffmanCAC[i] = 0;
        }
}

/********************************************************************/

static void LoadDefaultTable
(
        unsigned short *InputList,
        unsigned short *OutputList
)
{   /* copy input list to output list until input list reads 0xfff */

        short i;

        i = 0;
        while( InputList[i] != 0xfff )
        {
                OutputList[i]  = InputList[i];
                i++;
                if(i > 704)
                        break;
        }
}

/***********************************************************************/

/*  MakeCL5xxHuffmanTable()
        This function generates a CL5xx-specific Huffman Encode or Decode
        table from an input list of BITS and VALUES. The BitsNValues list
        must contain 16 bytes of BITS information followed by a variable-length
        VALUES list. This function is direction-sensitive, and direction
        must be specified. The destination table area should be cleared
        prior to using this function.
*/

void MakeCL5xxHuffmanTable
(
        unsigned char   *BitsNVals,
        unsigned short  *CL550HuffmanTable,
        short           Direction,
        short           ACDC
)
{
        unsigned char   *Vals;
        short           TotalCodes, MaxNumCodes, DecodeTreeLength;
        short           CodeSizeTable[256];
        unsigned short  HuffmanCodeTable[256];
        unsigned short  EncodingProcedureCodeTable[256];
        unsigned short  EncodingProcedureSizeTable[256];

        short i;

        Vals = BitsNVals+16;    /* just the vals, please */

/* 1. Derive the SIZE and CODE lists from Bits and Values */

        GenerateCodeSizeTable( BitsNVals, CodeSizeTable, &TotalCodes );

        GenerateHuffmanCodeTable( CodeSizeTable, HuffmanCodeTable );

        OrderHuffmanCodes( Vals, CodeSizeTable, HuffmanCodeTable,
                        EncodingProcedureCodeTable, EncodingProcedureSizeTable,
                        TotalCodes );

/*
        printf("Encoding Procedure Huffman Tables\nSize\tCode\n\n");
        for(i=0; i < 256 ; i++)
                printf("%x\t%x\n", EncodingProcedureSizeTable[i],
                                EncodingProcedureCodeTable[i]);
*/

/* 2. Build a machine-specific form of the table for a CL5xx device */

        if(DeviceType == CL550)
        {  /* 2. Make CL550 Tables for compress or decompress */

                if( Direction == COMPRESS )
                {
                        /* set limits for size of Encoding Procedure Tables */
                        if(ACDC == DC)
                                MaxNumCodes = 16;      /* 16 max, 12 max for baseline */
                        else
                                MaxNumCodes = 256;     /* 256 max, 162 max for baseline */

                        MakeCL550EncodeTable
                        (   EncodingProcedureCodeTable, EncodingProcedureSizeTable,
                                CL550HuffmanTable, MaxNumCodes  );
                }
                else
                {
                        MakeCL550DecodeTable
                        (   Vals, CodeSizeTable, HuffmanCodeTable,
                                CL550HuffmanTable, TotalCodes   );
                }
        }
        else
        {  /* build a 560/570 Huffman Table */

                        MakeCL560HuffmanTable
                        (   EncodingProcedureCodeTable, EncodingProcedureSizeTable,
                                CL550HuffmanTable, ACDC  );
        }
}

/*********************************************************************/

/*  GenerateCodeSizeTable()
        This function takes as its input a 16-byte BITS array and computes
        a table of sizes for each of the Huffman codes specified as well as
        the total number of Huffman codes specified. For more information
        see the JPEG CD document (ISO/IEC CD #10918-1), Annex C, Figure C-1.
*/

static void GenerateCodeSizeTable
(
        unsigned char *Bits,
        short *CodeSizeTable,
        short *TotalCodes
)
{
        short i, j, k;
        short count, size;

        j = k = 0;
        size = 1;
        for( i = 0; i < 16; i++ )
        {
                count = (short) Bits[i];

                while( count > 0 )
                {
                         CodeSizeTable[k++] = size;
                         count -= 1;
                }
                size += 1;
        }

        CodeSizeTable[k] = 0;
        *TotalCodes = k;
}

/*******************************************************************/

/*  GenerateHuffmanCodeTable()
        This function generates a list of Huffman codes based on the list
        of code sizes. For more information see the JPEG CD document
        (ISO/IEC CD #10918-1), Annex C, Figure C-2.
*/

static void GenerateHuffmanCodeTable
(
        short *CodeSizeTable,
        unsigned short *HuffmanCodeTable
)
{
        short CurrentSize, k;
        unsigned short Code;
        short NextSize;

        k = 0;
        Code = 0;
        CurrentSize = CodeSizeTable[0];

        while(1)
        {
                HuffmanCodeTable[k] = Code;
                Code += 1;
                k += 1;

                NextSize = CodeSizeTable[k];
                if( NextSize == CurrentSize )
                        continue;

                if( NextSize == 0 )
                        break;

                do
                {
                        Code <<= 1;
                        CurrentSize++;
                }
                while( NextSize > CurrentSize );

        }
}

/*********************************************************************/

/*  OrderHuffmanCodes()
        This function uses the tables Values[], HuffmanCodeTable[], and
        CodeSizeTable[] to generate the JPEG encoding procedure tables
        EncodingProcedureCodeTable[] and EncodingProcedureSizeTable[].
        This is done by reordering the codes specified by HuffmanCodeTable[]
        and CodeSizeTable[] according to the values assigned to each code in
        Values[]. For more information see the JPEG CD document
        (ISO/IEC CD #10918-1), Annex C, Figure C-3.
*/

static void OrderHuffmanCodes
(
        unsigned char   *ValsList,
        short           *CodeSizeTable,
        unsigned short  *HuffmanCodeTable,
        unsigned short  *EncodingProcedureCodeTable,
        unsigned short  *EncodingProcedureSizeTable,
        short           TotalCodes
)
{
        short i;
        short Val;

        for( i = 0; i < 256; i++ )
        {   /* Initialize output arrays to zero */
                EncodingProcedureSizeTable[i] = 0;
                EncodingProcedureCodeTable[i] = 0;
        }

        for( i = 0; i < TotalCodes; i++ )
        {
                Val = (short) ValsList[i] & 0x00ff;
                EncodingProcedureSizeTable[Val] = CodeSizeTable[i];
                EncodingProcedureCodeTable[Val] = HuffmanCodeTable[i];
        }
}

/**********************************************************************/

/*  MakeCL550EncodeTable()
        This function takes the EncodingProcedureCodeTable[] and
        the EncodingProcedureSizeTable[] and formulates the
        CL550-specific Huffman lookup table for the compression direction.

        CL550 encoding tables are 18 bits per entry. The encode tables have the
        following format:

        16 bit Codes:
                bit 17,16 = 10, followed by 16 bits of Huffman code, lsb in bit 0.
        15 bit Codes:
                bits 17,16,15 = 110, followed by 15 bits of Huffman code, lsb in bit 0.
        14 bit Codes:
                bits 17,16,15 = 111, followed by huffman code, lsb in bit 0.
        less than 14 bits:
                bit 17 = 0, bits 16:13 = Size, followed by huffman code.
*/

static void MakeCL550EncodeTable
(
        unsigned short  *EncodingProcedureCodeTable,
        unsigned short  *EncodingProcedureSizeTable,
        unsigned short  *CL550EncodeTable,
        short           MaxNumCodes
)
{
        short             i,j;
        unsigned short    Code;
        short             Size;
        unsigned long     flipCode;
        unsigned long     bigCodeWord;
        unsigned long     SizeFields[17] = {
                0x00000, 0x02000, 0x04000, 0x06000,
                0x08000, 0x0a000, 0x0c000, 0x0e000,
                0x10000, 0x12000, 0x14000, 0x16000,
                0x18000, 0x1a000, 0x38000, 0x30000,
                0x20000 };

        for (i = 0; i < MaxNumCodes; i++)
        {
                /* get a code and size value from the input lists */
                Code = EncodingProcedureCodeTable[i];
                Size = EncodingProcedureSizeTable[i];

                bigCodeWord = 0;

                if(Size)    /* if Size is zero, then just load zero */
                {
                        /* flip the Code so lsb is in bit 0 */

                        flipCode = 0;
                        for( j = 0; j < Size; j++ )
                        {
                                flipCode = flipCode << 1;
                                if( Code & 1 )
                                        flipCode |= 1;
                                Code = Code >> 1;
                        }

                        /* set the size fields for the code table entries */

                        bigCodeWord = SizeFields[Size] | flipCode;
                }

                *CL550EncodeTable++ = (unsigned short)(bigCodeWord & 0x1FF);
                *CL550EncodeTable++ = (unsigned short)((bigCodeWord>>9) & 0x1FF);
        }
}

/*************************************************************************/

/*  MakeCL550DecodeTable()
        This function will generate a two-bit-grab Huffman decoding tree for
        the CL550 using the lists of Huffman Codes, Sizes, and Values.

        Each addressable word in the decode tree contains 18 bits ( a high 9
        and a low 9). Each 9 bit value in this tree is a node. A node contains:
        a leaf flag (1 bit) and either: A) the next state, if the leaf flag is
        not set, or B) the 8-bit value represented by the huffman code, if the
        leaf flag is set.
*/

static void MakeCL550DecodeTable
(
        unsigned char   *ValuesList,
        short           *CodeSizeTable,
        unsigned short  *HuffmanCodeTable,
        unsigned short  *CL550HuffmanTable,
        short           TotalCodes
)
{
        unsigned short *Base;
        short Grab2;
        short Code;
        short NextState;
        short Size;
        short i;

        NextState = 1;
        for ( i = 0; i < TotalCodes; i++ )
        {
                Size = CodeSizeTable[i];
                Code = HuffmanCodeTable[i] << (16 - Size);

                Base = CL550HuffmanTable;         /* start @ root of tree  */
                while( Size > 0 )
                {       /* stay in this loop until a leaf is found */

                        Grab2 = (Code < 0) ? 2 : 0;    /* Grab two bits. First one, */
                        if ( (Code = Code << 1) < 0 )  /* then the other.           */
                                Grab2 += 1;
                        Code = Code << 1;

                        if ( Size > 2 )      /* then not a leaf - make a node */
                        {
                                Base += Grab2;            /* point to node */
                                if ( *Base )              /* branch already defined ?? */
                                        /* yes... branch out */
                                        Base = CL550HuffmanTable + 4*(*Base);
                                else
                                {   /* no... define a new branch */
                                        *Base = NextState;
                                        Base = CL550HuffmanTable + 4*NextState;
                                        NextState++;
                                }
                                Size -= 2;
                        }
                        else
                        {   /* size is 1 or 2 - make a leaf (leaf flag is bit 8) */
                                if (Size == 2)              /* either even or odd */
                                {
                                        Base += Grab2;
                                        *Base = ValuesList[i] | 0x100; /* Value + Leaf */
                                }
                                else
                                {   /* size is 1, we've got one too many bits */
                                        Grab2 &= 2;
                                        Base += Grab2;
                                        *Base = ValuesList[i] | 0x100; /* Value + Leaf */
                                        Base ++;
                                        *Base = ValuesList[i] | 0x100; /* Value + Leaf */
                                }
                                break;
                        }
                }
        }

        /*  DecodeTableLength = NextState * 4;     save tree length in words */
}

/*  MakeCL560HuffmanTable()
        This function builds machine-specific huffman tables for the CL560. The
        inputs to this function are the EncodingProcedureCodeTable and
        EncodingProcedureSizeTable. Results are placed in CL5xxHuffmanTable.
*/

static void MakeCL560HuffmanTable
(
        unsigned short  *EncodingProcedureCodeTable,
        unsigned short  *EncodingProcedureSizeTable,
        unsigned short  *CL5xxHuffmanTable,
        short           ACDC
)
{
        short           i, j, k;
        unsigned long   Code, temp;
        unsigned short  Size;
        unsigned short  *Row, *Column;

        if( ACDC == DC )
        {   /* build a DC Table */

                Row = CL5xxHuffmanTable;    /* 1-D array */
                for(i = 0; i < 16; i++)
                {
                        Code = (unsigned long) EncodingProcedureCodeTable[i];
                        Size = EncodingProcedureSizeTable[i];

                        Code = Code << (20 - Size);     /* shift code to MSB of 20-bit cell */
                        Code += Size-1;                 /* add size field */

                        /* split 20-bit code into 2 16-bit words, 10 bits each, lsb-aligned */
                        *Row++ = (unsigned short) Code & 0x03ff;
                        *Row++ = (unsigned short) (Code>>10) & 0x03ff;
                }
        }
        else
        {   /* build an AC table -- 2-D style */

                k = 0;
                Column = CL5xxHuffmanTable;
                for( i = 0; i < 16; i++ )
                {   /* runs */
                        Row = Column;
                        for( j = 0; j < 16;  j++ )
                        {   /* sizes */
                                Code = (unsigned long) EncodingProcedureCodeTable[k];
                                Size = EncodingProcedureSizeTable[k++];

                                if( (j == 0) && ((i > 0) && (i < 15)) )
                                        /* all size=0's between EOB and ZRL get stuffed */
                                        Code = -1;
                                else if ( j > 10 )
                                        /* all run/size where size > 10 get stuffed */
                                        Code = -1;
                                else
                                {   /* build a code */
                                        Code = Code << (20 - Size);  /* align code to bit 19 */
                                        Code += Size-1;              /* add code length field */
                                }
                                temp = Code;
                                *Row++ = (unsigned short) temp & 0x3ff;
                                temp = Code >> 10;
                                *Row = (unsigned short) temp & 0x3ff;
                                Row += 31;     /* offset to next cell in Row */
                        }
                        Column += 2;       /* next column */
                }
        }
}
/* End of file */

