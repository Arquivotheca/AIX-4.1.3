static char sccsid[] = "@(#)14	1.2  src/bos/usr/bin/bprt/arpr.c, libbidi, bos411, 9428A410j 11/4/93 15:25:28";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: AccProcessChar
 *		AccProcessLine
 *		BidiAp
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "prt-acc.h"
#include "prt-proc.h"
#include <sys/lc_layout.h>
#include "bdprtdos.h"
#include "arpr.h"

#include <piostruct.h>

/* declare global variables */


#include "prt-func.h"
extern unsigned char debug;

extern unsigned char Arabic, Hebrew;
unsigned char trace = 0;
extern EntryPoint G;


/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: BidiAp                                                   */
/*                                                                            */
/*  DESCRIPTIVE NAME:   This procedure either initializes the environment or  */
/*                      processes a data buffer.                              */
/*                                                                            */
/*  FUNCTION:   BidiPrint is called to either initialize the environment for  */
/*              processing the data buffer or for processing the buffer.      */
/*              If mode = 0 then initialize environment and if mode = 1 then  */
/*              process buffer. The code page is set (from colon file - "aT"  */
/*              attribute) to find see if the translation from code page 1046 */
/*		or iso-6 to code page 864 is needed or not. 	       	      */
/*              Note: We can not depend on the translation done by piobe      */
/*              since shaping is envolved ie the data is changed after the    */
/*              translation done by the piobe.                                */
/*                                                                            */
/*  ENTRY POINT: BidiAp                                                       */
/*      LINKAGE: CALL NEAR (from main)                                        */
/*                                                                            */
/*  INPUT: (prt_name, bidi_attributes, in_buff, in_buff_len, prt_len, mode)   *//*      prt_name        OTHER   a string containing the name of the printer   */
/*                              on which data stream is to be outputted       */
/*      bidi_att        OTHER   a string containing the bidi attributes       */
/*      in_buff         OTHER   a buffer containing the input data which is   */
/*                              to be processed                               */
/*      in_buff_len     USHORT  the size of the input data buffer             */
/*      prt_len         UCHAR   the width in characters of printer paper      */
/*      mode            USHORT  flag to indicate the initialisation of        */
/*                              processor (mode=0) or processing of input     */
/*                              data buffer (mode=1)                          */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*      prt_num         USHORT  number containing the number of printer being */
/*                              currently used                                */
/*      BidiAtts        ULONG   number containing the bidi attributes         */
/*      trans_env       OTHER   string contating the string value of the      */
/*                              TRANS environment variable                    */
/*      trans           INT     flag to indicate whether translation from CP  */
/*                              1046 to 864 is to be done or not (depending   *//*                              on the "aT" colon file attribute).            */
/*      tempstr         OTHER   string to contain the string value of the     */
/*                              colon file attribute "aT"                     */
/*      BDInterface     OTHER   structure containing values needed in the     */
/*                              interface between pioout and the support      */
/*                                                                            */
/*  GLOBAL DATA DEFINITIONS: NONE                                             */
/*      bidi_out_buff [4*BUFSIZ] OTHER  buffer which contains the output data */
/*                                      stream                                */
/*      bidi_out_buff_len        USHORT length of buffer containing the       */
/*                                      output data stream                    */
/*      Arabic                   UCHAR  flag which indicates whether the      */
/*                                      current language in Arabic or not.    */
/*      Hebrew                   UCHAR  flag which indicates whether the      */
/*                                      language in Hebrew or not.            */
/*                                                                            */
/*  ROUTINES:                                                                 */
/*      piogenmsg                                                             */
/*      pioexit                                                               */
/*      ReadColonFile                                                         */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES:                                                              */
/*              sscanf                                                        */
/*              getenv                                                        */
/*              atoi                                                          */
/*                                                                            */
/************************** END OF SPECIFICATIONS *****************************/

unsigned short BidiAp (char *prt_name, 
                       char * bidi_attributes,
                       char * in_buff,
                       unsigned long in_buff_len,
                       unsigned char prt_len,
		       unsigned char PrtCodePage,
                       unsigned short mode)
{
unsigned short prt_num;
unsigned long BidiAtts;
struct BidirectionalInterface BDInterface;


  if (bidi_attributes == NULL)
    bidi_attributes = 0x00000000;
  sscanf(bidi_attributes ,"%x", &BidiAtts); 
  prt_num = prn_num (prt_name);

  if (Arabic && PrtCodePage == CP864)
    BDInterface.PrtCodePage = CP864;
  else
    if (Hebrew)
       BDInterface.PrtCodePage = CP862;
    else
       BDInterface.PrtCodePage = CP1046;


  if (mode == 0) {  /* initialize the bidi printing  */
      BDInterface.Mode = 0;
      BDInterface.BDAtts =  BidiAtts;
      BDInterface.CFG = prt_num; 
      BDInterface.prt_len = prt_len; 
      BidiPrint (&BDInterface);
  }


  if (mode == 1) {
      BDInterface.Mode = 1;
      BDInterface.in_buff = in_buff;
      BDInterface.in_buff_len = in_buff_len;
      BidiPrint (&BDInterface);
  }
} /* end of the entry point procedure */






/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: AccProcessLine                                           */
/*                                                                            */
/*  DESCRIPTIVE NAME: accumulates data into output buffer                     */
/*                                                                            */
/*  FUNCTION: accumulates data in a buffer with a certain increment into      */
/*            output buffer.                                                  */
/*                                                                            */
/*  ENTRY POINT: AccProcessLine                                               */
/*      LINKAGE: CALL NEAR (from PADD_SPACES_2,                               */
/*                               PostProces)                                  */
/*                                                                            */
/*                                                                            */
/*  INPUT: (LineLength, AccStep, Line)                                        */
/*      LineLength      LONG    the size of data buffer                       */
/*      AccStep         LONG    the increment in buffer to be skipped when    */
/*                              putting the data in output buffer             */
/*      Line            OTHER   the input data buffer                         */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*      LinePtr         LONG    loop counter pointing to data (jumping the    *//*                              increment                                     */
/*                                                                            */
/*  GLOBAL DATA DEFINITIONS: NONE                                             */
/*                                                                            */
/*  ROUTINES: NONE                                                            */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE.                                                        */
/*                                                                            */
/************************** END OF SPECIFICATIONS *****************************/
void AccProcessLine(LONG    LineLength,
                    LONG    AccStep,
                    char  * Line)
{ 
  LONG LinePtr;
    
  LinePtr = 0;
  while  (LineLength-->0)
  {
    bidi_out_buff [bidi_out_buff_len ++] = Line [LinePtr] ; 
    LinePtr += AccStep;
  }  /* while */
}




/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: AccProcessChar                                           */
/*                                                                            */
/*  DESCRIPTIVE NAME: returns a pointer to a value in a string                */
/*                                                                            */
/*  FUNCTION: gets a string pointer which points to the character after       */
/*            the flag character.                                             */
/*                                                                            */
/*  ENTRY POINT: AccProcessChar                                               */
/*      LINKAGE: CALL NEAR (from NextChar,                                    */
/*                               WriteESC,                                    */
/*                               WriteCHAR,                                   */
/*                               ESC_I,                                       */
/*                               _SET_HOR_MARGINS,                            */
/*                               _LEFT_MARGIN_FOUND,                          */
/*                               _RIGHT_MARGIN_FOUND,                         */
/*                               _REVERSE_LF_n,                               */
/*                               _REVERSE_LF,                                 */
/*                               _SPACE_FOR_BAK,                              */
/*                               _DW_DH,                                      */
/*                               _SELECT_FONT,                                */
/*                               _RESERVED_2,                                 */
/*                               _ESC_FOUND,                                  */
/*                               _HT_FOUND,                                   *//*                               _FLUSH_BUFFER,                               */
/*                               _PRINT_BUFF,                                 */
/*                               Processor,                                   */
/*                               FlushLine,                                   */
/*                               FLUSH_SPACES,                                */
/*                               PADD_SPACES_1,                               */
/*                               PADD_SPACES_2,                               */
/*                               PostProcess,                                 */
/*                               PSM_WriteToBuffer)                           */
/*                                                                            */
/*                                                                            */
/*  INPUT: (Char)                                                             */
/*      Char    LONG    character to be inserted in output data buffer        */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*                                                                            */
/*  GLOBAL DATA DEFINITIONS: NONE                                             */
/*                                                                            */
/*  ROUTINES: NONE                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE.                                                        */
/*                                                                            */
/************************** END OF SPECIFICATIONS *****************************/

void AccProcessChar(LONG Char)
{
  bidi_out_buff [bidi_out_buff_len++] = Char;
}

