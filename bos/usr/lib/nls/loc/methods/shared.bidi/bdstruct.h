/* @(#)37       1.1  src/bos/usr/lib/nls/loc/methods/shared.bidi/bdstruct.h, bos, bos410 8/30/93 15:01:21 */
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <iconv.h>
#include <sys/lc_layout.h>

#define ARABIC_CHARSET1     "IBM-1046"
#define ARABIC_CHARSET2     "ISO8859-6"

#define NUMERALS_NONE       0x00001000

#define MaskAllTextDescptors       0xffff0000
#define MaskTypeOfText             0x03000000
#define MaskBidiType               0x00400000
#define MaskOrientation            0x00010000
#define MaskNumerals               0x00003000 
#define MaskWordBreak              0x00000200
#define MaskSwapping               0x00000100  
#define MaskOneCellShaping         0x00000080 
#define MaskSpecialShaping         0x00000060 
#define MaskTextShaping            0x0000001f

typedef struct
                 {
                   int orient;
                   wchar_t  *codeset;
                   int wordbreak;
                   int num_flag;
                   int flip_flag;
                   int symmetric;
                   int num;
                   char *buffer;
                   unsigned short Tables;
                   unsigned char *A_level;
                   unsigned int *SrcToTrgMap;
                   unsigned int *TrgToSrcMap;
                 } ICSPARAMRec;

typedef ICSPARAMRec *PICSPARAMRec;

typedef struct
              {
                wchar_t *InCharSet;
                wchar_t *OutCharSet;
                int OutCharSetSize;
                char ShapeState; 
                LayoutEditSizeRec ContextSize;
                LayoutTextDescriptorRec Orient;
                LayoutTextDescriptorRec Text;
                LayoutTextDescriptorRec Bidi;
                LayoutTextDescriptorRec Swap;
                LayoutTextDescriptorRec Num;
                LayoutTextDescriptorRec Shaping;
                LayoutTextDescriptorRec Word;
                LayoutTextDescriptorRec OneCell;
                LayoutTextDescriptorRec SpecialSh;
                char *temp_buf;
                size_t temp_count;
                size_t temp_index;
                iconv_t iconv_handle;
              } BidiValuesCoreRec, *BidiValuesRec;


