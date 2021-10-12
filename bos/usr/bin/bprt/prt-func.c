static char sccsid[] = "@(#)28	1.2  src/bos/usr/bin/bprt/prt-func.c, libbidi, bos411, 9428A410j 11/4/93 15:32:56";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: CHANGE_LL
 *		NewLine
 *		NewWidth
 *		NextChar
 *		PutEscapes
 *		WriteCHAR
 *		_BIDI_SEQ_FOUND
 *		_CAN_FOUND
 *		_CHANGE_LL_IN
 *		_CHANGE_LL_OUT
 *		_DESTRUCT_BS
 *		_DOWNLOAD_EPSON
 *		_DOWNLOAD_PROPRINT2544
 *		_DW_DH
 *		_ESC_C_FOUND
 *		_ESC_FOUND
 *		_ESC_I
 *		_ESC_SINGLE
 *		_ESC_SINGLE_0_1
 *		_ESC_SUB_SUPER
 *		_FLUSH_BUFFER
 *		_GRAPHICS
 *		_GRAPHICS_STAR
 *		_HANDLE_ESC_BRKT2728
 *		_HT_FOUND
 *		_IGNORE_ESC_n
 *		_IGNORE_ESC_n_0
 *		_IGNORE_ESC_n_0002098
 *		_IGNORE_ESC_n_0_02050
 *		_LEFT_MARGIN_FOUND1438
 *		_PASSTHROUGH_IN_BRACKET2988
 *		_PASS_THROUGH
 *		_PMP
 *		_PRINTABLE
 *		_PRINTER_RESET
 *		_PRINT_BUFF
 *		_PRINT_SERVICE_13847
 *		_PRT_ALL
 *		_PRT_DESELECT
 *		_PRT_DESELECT_n
 *		_PRT_NEXT
 *		_PRT_SELECT
 *		_RESERVED_1
 *		_RESERVED_2
 *		_RESET_TAB_SETTINGS1811
 *		_RESTORE_JMP_2
 *		_REVERSE_LF
 *		_REVERSE_LF_n
 *		_RIGHT_MARGIN_FOUND1500
 *		_SAVE_RESTORE_CURSOR_POS3686
 *		_SELECT_CURSOR_POSITION3626
 *		_SELECT_FONT
 *		_SET_HOR_MARGINS1347
 *		_SET_VERT_HORIZ_MARGIN3746
 *		_SO_SI
 *		_SPACE_FOR_BAK
 *		_SPECIAL_ONE
 *		_TAB_SET_FOUND
 *		_ZERO_TERMINATOR1703
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *   OBJECT CODE ONLY SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************SOURCE FILE HEADER*******************************/
/*  Source File Name: prt-func.c                                              */
/*                                                                            */
/*  FUNCTION: prt-func.c contains all functions a particular ASCII Control    */
/*            Code or Escape Sequence requires for the right-to-left          */
/*            printing.                                                       */
/*                                                                            */
/*  Entry Points:                                                             */
/*     _PASS_THROUGH          This code is used for single byte codes         */
/*                            which do not perform any special features       */
/*                            on the printer.                                 */
/*                                                                            */
/*     _PRINT_SERVICE_1       This will pass the corresponding single         */
/*                            byte printer code directly to the printer       */
/*                            when received.                                  */
/*                                                                            */
/*     _DESTRUCT_BS           This will cause the corresponding single        */
/*                            byte code to delete the last entry in the       */
/*                            printer buffer.                                 */
/*                                                                            */
/*     _SPECIAL_ONE           This is used for single byte codes that         */
/*                            are printable under CHARACTER SET 2, and        */
/*                            are treated as control codes in CHARACTER       */
/*                            SET 1.                                          */
/*                                                                            */
/*     _HT_FOUND              This is used for single byte codes that         */
/*                            perform a horizontal tab.                       */
/*                                                                            */
/*     _FLUSH_BUFFER          This will cause the print buffer to be          */
/*                            printed (as with a Line Feed), and have         */
/*                            the next line's starting print position         */
/*                            right after the last received character.        */
/*                                                                            */
/*     _PRINT_BUFF            This is similar to the FLUSH_BUFFER             */
/*                            control, except the starting print position     */
/*                            for the next line will be right-justified       */
/*                            (as with a Carriage Return).                    */
/*                                                                            */
/*     _SO_SI                 This is short form for SHIFT_IN and             */
/*                            SHIFT_OUT printer controls.  It is used for     */
/*                            single byte print codes that change the         */
/*                            printing pitch.                                 */
/*                                                                            */
/*     _PRT_SELECT            This is for single byte codes that will         */
/*                            select the printer to go ON-LINE.  This is      */
/*                            useful only with PRT_DESELECT feature!          */
/*                                                                            */
/*     _PRT_DESELECT          The single byte code will send the printer      */
/*                            into an OFF-LINE mode - ie. the characters      */
/*                            sent to the printer will NOT be printed!        */
/*                                                                            */
/*     _CAN_FOUND             This will cause the print buffer (LineBuff)     */
/*                            to be cleared when the CANCEL is received!      */
/*                                                                            */
/*     _ESC_FOUND             This is one of the more frequently used         */
/*                            printer controls.  This is key to the use       */
/*                            of the ESCape sequences that follow             */
/*                            character 27!                                   */
/*                                                                            */
/*     _ESC_SINGLE            For ESCape sequences such as 'ESC E' this       */
/*                            function will act on the corresponding          */
/*                            printer encoded ATTRIBUTE.  This is only for    */
/*                            ESCape sequences that are two bytes long!       */
/*                                                                            */
/*     _IGNORE_ESC_n_0        For ESCape sequences that do not affect the     */
/*                            the operation of the printer driver - that      */
/*                            are 4 bytes long (ie. 'ESC x n 0').  The        */
/*                            entire sequence will be sent to the printer     */
/*                            immediately!                                    */
/*                                                                            */
/*     _IGNORE_ESC_n          As the above function, except it is for 3       */
/*                            byte codes.                                     */
/*                                                                            */
/*     _ESC_SINGLE_0_1        This is for ESCape sequences that use a         */
/*                            toggle of character 0/1 or ASCII character      */
/*                            "0/1" for enable and disable a feature.         */
/*                            An example is the 'ESC W 0/1' for double        */
/*                            width printing.                                 */
/*                                                                            */
/*     _ZERO_TERMINATOR       This is used for ESCape sequences which         */
/*                            are sent directly to the printer that end       */
/*                            with a character 0 (ie. to indicate that the    */
/*                            ESCape sequence has completed).                 */
/*                                                                            */
/*     _ESC_C_FOUND           This is for the standard 'ESC C ...'            */
/*                            sequence used to set the page size in inches    */
/*                            and/or lines of text.  It is a special case     */
/*                            ESCape sequence that is sent directly to the    */
/*                            printer when received in a datastream.          */
/*                                                                            */
/*     _TAB_SET_FOUND         This is used to set the new tab settings        */
/*                            for horizontal tabs.  It is terminated by       */
/*                            character 0 and the values following the        */
/*                            ESCape are the column tab settings.             */
/*                                                                            */
/*     _RIGHT_MARGIN_FOUND    This ESCape sequence is specifically for        */
/*                            setting the right margin (in terms of           */
/*                            columns) ONLY.                                  */
/*                                                                            */
/*     _ESC_SUB_SUPER         This ESCape sequence (as per ESC_SINGLE_0_1)    */
/*                            is a toggle type!  The ESCape sequence will     */
/*                            invoke the super/subscript form depending on    */
/*                            the 3rd byte value.                             */
/*                                                                            */
/*     _LEFT_MARGIN_FOUND     See RIGHT_MARGIN_FOUND                          */
/*                                                                            */
/*     _IGNORE_ESC_n_0_0      See IGNORE_ESC_n_0-used for 5 byte sequences    */
/*                                                                            */
/*     _REVERSE_LF_n          This will perform the same result as a          */
/*                            FLUSH_BUFFER except that it is used for         */
/*                            printers that support REVERSE LINE FEED's       */
/*                            (sequence is 3 bytes long).                     */
/*                                                                            */
/*     _GRAPHICS              This is used for printers that support          */
/*                            graphics (for dot matrix type printers).        */
/*                            When the GRAPHICS ESCape sequence is detecte    */
/*                            the count of binary information following is    */
/*                            sent directly to the printer.  If graphics      */
/*                            are printed when the driver is in right-to-     */
/*                            left then no text should be printed on the      */
/*                            same line.                                      */
/*                                                                            */
/*     _GRAPHICS_STAR         This is specific support for the EPSON FX-100   */
/*                            for the 'ESC * m n1 n2 ...' graphics sequence.  */
/*                            It operates similar to the GRAPHICS function    */
/*                            code - ie. only graphics on a line for right-   */
/*                            to-left output!                                 */
/*                                                                            */
/*     _DOWNLOAD_EPSON        This is a specific ESCape sequence used for     */
/*                            the EPSON FX-100 printer like download          */
/*                            sequences.                                      */
/*                                                                            */
/*     _DOWNLOAD_PROPRINT     This is as per DOWNLOAD_EPSON but for the       */
/*                            IBM 4201 Proprinter like sequences.             */
/*                                                                            */
/*     _RESET_TAB_SETTINGS    This will return the printer and the printer    */
/*                            driver back to the initial horizontal tab       */
/*                            settings.                                       */
/*                                                                            */
/*     _PRINTER_RESET         This is a printing mode reset used to clear     */
/*                            the printing modes back to the default modes!   */
/*                            The sequence is 2 bytes long (ESC x).           */
/*                                                                            */
/*     _CHANGE_LL_OUT         This is a specific code assigned for the        */
/*                            IBM 5201 Quietwriter printer.  The feature      */
/*                            is changing the length of the printing line     */
/*                            to 13.2 inches (from 8 inches).                 */
/*                                                                            */
/*     _CHANGE_LL_IN          See CHANGE_LL_OUT - to restore the printing     */
/*                            line length back to 8 inches!                   */
/*                                                                            */
/*     _SET_HOR_MARGINS       This is similar to the setting of the left      */
/*                            and right margins of the printing line.  The    */
/*                            difference is that one sequence sets both       */
/*                            margins.  This is a four byte sequence that     */
/*                            has 'ESC x left right' as the ESCape            */
/*                            format!                                         */
/*                                                                            */
/*     _IGNORE_ESC_n_000      See IGNORE_ESC_n_0-used for 6 byte sequences    */
/*                                                                            */
/*     _REVERSE_LF            See REVERSE_LF_n - except sequence is only      */
/*                            2 bytes long.                                   */
/*                                                                            */
/*     _PRT_DESELECT_n        This operates as per for PRT_DESELECT           */
/*                            sequences but it is 3 bytes long (ESC x n).     */
/*                                                                            */
/*     _PMP                   This is specifically for the IBM 3812 Page-     */
/*                            printer.  The PMP stands for Page Map Primitive */
/*                            commands that are supported by this printer.    */
/*                            The PMP sequence is sent directly to the        */
/*                            printer!                                        */
/*                                                                            */
/*     _RESERVED_1            This function is used to send two contiguous    */
/*                            attribute type escape sequences to the          */
/*                            printer one after the other.  The pair of       */
/*                            escape sequences may be located within the      */
/*                            Width Table, the Code On Table or the Code      */
/*                            Off Table.                                      */
/*                                                                            */
/*     _PRINTABLE             This function is used for single byte codes     */
/*                            that are treated as printable (for both         */
/*                            CHARACTER SET 1 and 2).  Refer to PRINTABLE!    */
/*                                                                            */
/*     _PRINT_NEXT            This function is used for single ESC  codes     */
/*                            that will print next character as graphic.      */
/*                                                                            */
/*     _PRINT_ALL             This function is used for ESC  codes (4 Byte    */
/*                            long) that will print the following             */
/*                            (Count=LLHH) characters as graphic.             */
/*                                                                            */
/*     _RESTORE_JMP_2         This will pass the ESC along with the single    */
/*                            byte printer code directly to the printer when  */
/*                            received.                                       */
/*                                                                            */
/*     _HANDLE_ESC_BRKT       This function reads the character after the [   */
/*                            and determines which function to call           */
/*                            accordingly, by setting the new G. parameter    */
/*                            G.BRACKET_FUNCTIONS                             */
/*                                                                            */
/*     _RESERVED_2            This function is used to send three             */
/*                            contiguous attribute type escape sequences to   */
/*                            the printer one after the other. The triplet    */
/*                            of escape sequences may be located within       */
/*                            the Width Table, the Code On Table or the       */
/*                            Code Off Table.                                 */
/*                                                                            */
/*     _BIDI_SEQ_FOUND        This function is used to update the current     */
/*                            BIDI ATTRIBUTE if the BIDI SEQUENCE is          */
/*                            detected on line or job Boundaries.             */
/*                                                                            */
/*     _DW_DH                 This function is used to process the ESCAPE     */
/*                            double width double height sequence.            */
/*                               ESCAPE <SubCode> @ n1 n2 m1 m2 m3 m4         */
/*                            where                                           */
/*                               n1,n2 represents the length of the sequence  */
/*                               m3    represents Width information           */
/*                               m4    represents Height information          */
/*                                                                            */
/*     _SPACE_FOR_BAK         This function is used to support the ESCAPE     */
/*                            sequences that move the printer head in         */
/*                            multiples of 1/120 of an inch.                  */
/*                               ESCAPE <SubCode> n1 n2                       */
/*                                                                            */
/*                                                                            */
/* Internal usage functions & macros:                                         */
/*                                                                            */
/*     NewLine                This function is used to write the data         */
/*                            from the line buffer into the output buffer     */
/*                                                                            */
/*     NextChar               This function reads the next character from     */
/*                            the input buffer and returns 1 if data input    */
/*                            buffer is totally consumed else it returns 0.   */
/*                                                                            */
/*     WriteESC               This macro writes the last escape and           */
/*                            subcode into the output buffer.                 */
/*                                                                            */
/*     WriteCHAR              This macro writes the last control              */
/*                            character into the output buffer.               */
/*                                                                            */
/*  External Routines Used:                                                   */
/*               WriteToBuffer                                                */
/*               InsertSpaces                                                 */
/*               PostProcess                                                  */
/*               AccProcessChar                                               */
/*                                                                            */
/******************************************************************************/
#include <stdio.h>
#include <sys/lc_layout.h>
#include "compose.h"
#include "bdprtdos.h"
#include "prt-c-em.h"
#include "prt-proc.h"
#include "prt-func.h"
#include "prt-cfg.h"
#include "prt-acc.h"
#include "typedef.h"
#include "psmtable.h"

extern PBDInterface BDI;
extern unsigned short *PsmTable;

extern unsigned char debug;

unsigned char  (*Functions[MAX_FUNCTIONS]) (void) =
{
/* 0 */ _PASS_THROUGH            ,
/* 1 */ _PRINT_SERVICE_1         ,
/* 2 */ _DESTRUCT_BS             ,
/* 3 */ _SPECIAL_ONE             ,
/* 4 */ _HT_FOUND                ,
/* 5 */ _FLUSH_BUFFER            ,
/* 6 */ _PRINT_BUFF              ,
/* 7 */ _SO_SI                   ,
/* 8 */ _PRT_SELECT              ,
/* 9 */ _PRT_DESELECT            ,
/* 10*/ _CAN_FOUND               ,
/* 11*/ _ESC_FOUND               ,
/* 12*/ _ESC_SINGLE              ,
/* 13*/ _IGNORE_ESC_n_0          ,
/* 14*/ _IGNORE_ESC_n            ,
/* 15*/ _ESC_SINGLE_0_1          ,
/* 16*/ _ZERO_TERMINATOR         ,
/* 17*/ _ESC_C_FOUND             ,
/* 18*/ _TAB_SET_FOUND           ,
/* 19*/ _RIGHT_MARGIN_FOUND      ,
/* 20*/ _ESC_SUB_SUPER           ,
/* 21*/ _LEFT_MARGIN_FOUND       ,
/* 22*/ _IGNORE_ESC_n_0_0        ,
/* 23*/ _REVERSE_LF_n            ,
/* 24*/ _GRAPHICS                ,
/* 25*/ _GRAPHICS_STAR           ,
/* 26*/ _DOWNLOAD_EPSON          ,
/* 27*/ _DOWNLOAD_PROPRINT       ,
/* 28*/ _RESET_TAB_SETTINGS      ,
/* 29*/ _PRINTER_RESET           ,
/* 30*/ _CHANGE_LL_OUT           ,
/* 31*/ _CHANGE_LL_IN            ,
/* 32*/ _SET_HOR_MARGINS         ,
/* 33*/ _IGNORE_ESC_n_000        ,
/* 34*/ _REVERSE_LF              ,
/* 35*/ _PRT_DESELECT_n          ,
/* 36*/ _PMP                     ,
/* 37*/ _RESERVED_1              ,
/* 38*/ _PRINTABLE               ,
/* 39*/ _PRT_NEXT                ,
/* 40*/ _PRT_ALL                 ,
/* 41*/ _RESTORE_JMP_2           ,
/* 42*/ _RESERVED_2              ,
/* 43*/ _HANDLE_ESC_BRKT         ,
/* 44*/ _SPACE_FOR_BAK           ,
/* 45*/ _BIDI_SEQ_FOUND		 ,
/* 46*/ _ESC_I
};

extern EntryPoint      G;
extern unsigned short  counter;
extern unsigned long   CURR_CHAR_ATTR;
extern unsigned long   character;
extern unsigned long   Char_Attr;
extern unsigned long   PRT_PSM_ATTR;

/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: NextChar                                                */
/*                                                                            */
/*   DESCRIPTIVE NAME: This function is to retreive the next character        */
/*                     from the data input buffer                             */
/*                                                                            */
/*   FUNCTION:  This function returns the next character from the data input  */
/*              buffer,if one exists, and returns 0. if the buffer was        */
/*              totally consumed,  the function returns 1.                    */
/*                                                                            */
/*   ENTRY POINT: NextChar                                                    */
/*       LINKAGE: CALL    from                                                */
/*                                                                            */
/*   INPUT: ( c )                                                             */
/*             c    @OTHER      -  Pointer to where to place the read         */
/*                                 character.                                 */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION:                                                */
/*           Ok      WORD       - flag to indicate whether the input buffer is*/
/*                                consumed or NOT                             */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*           G         OTHER    - Structure EntryPoint which contains         */
/*                                the processing context of the current       */
/*                                input buffer.                               */
/*                                                                            */
/*           counter   WORD     - Number of characters read from Data Packet  */
/*                                                                            */
/*           BDI       OTHER    - Strucure BidirectionalInterface which       */
/*                                contains current interface (including       */
/*                                the data buffer and size)                   */
/*                                                                            */
/*      ROUTINES: NONE                                                        */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*                AccProcessChar                                              */
/************************** END OF SPECIFICATIONS *****************************/
int NextChar ( unsigned char *c )
{ int Ok=1;

 if (counter < BDI->in_buff_len)
  {
     *c = (char) BDI->in_buff [counter++];
           /************************************/
           /* Read character from input buffer */
           /************************************/
     if (G.DEF_ATTR)
       AccProcessChar( (WCHAR)  *c );
           /***************************************************************/
           /* if the monitor is in a pass through state, the characters   */
           /* read are automatically echoed into the output buffer        */
           /*                                                             */
           /* This is WHY, in normal processing before writing anything   */
           /* into the output buffer a check of (!DEF_ATTR) is made to    */
           /* make sure that the character was not already written into   */
           /* the output buffer.                                          */
           /***************************************************************/
      return !Ok;
  }
 return Ok;
}


/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: WriteESC  (MACRO)                                       */
/*                                                                            */
/*   DESCRIPTIVE NAME: Write ESC                                              */
/*                                                                            */
/*   FUNCTION:  This macro checks if DEF_ATTR is active; if DEF_ATTR          */
/*              is active neither the escape nor the subcode are sent.        */
/*                                                                            */
/*   ENTRY POINT: WriteESC  (MACRO)                                           */
/*       LINKAGE: USED in (_*)                                                */
/*                                                                            */
/*   INPUT: NONE.                                                             */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION: NONE                                           */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*           G         OTHER    - Structure EntryPoint which contains         */
/*                                the processing context of the current       */
/*                                input buffer.                               */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*               AccProcessChar                                               */
/*                                                                            */
/************************** END OF SPECIFICATIONS *****************************/
#define  WriteESC                               \
   if (!G.DEF_ATTR)                             \
   {                                            \
       AccProcessChar((WCHAR) _ESCAPE);         \
       AccProcessChar((WCHAR) G.ESC_SubCode);   \
   }                                            \


/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: WriteCHAR (MACRO)                                       */
/*                                                                            */
/*   DESCRIPTIVE NAME: Write Character                                        */
/*                                                                            */
/*   FUNCTION:  This macro checks if DEF_ATTR is active; if DEF_ATTR          */
/*              is active the character is not sent.                          */
/*                                                                            */
/*   ENTRY POINT: WriteCHAR (MACRO)                                           */
/*       LINKAGE: USED in (_*)                                                */
/*                                                                            */
/*   INPUT: ( character )                                                     */
/*              DWORD character   - character to put in output buffer.        */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION: NONE                                           */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*           G         OTHER    - Structure EntryPoint which contains         */
/*                                the processing context of the current       */
/*                                input buffer.                               */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*               AccProcessChar                                               */
/*                                                                            */
/************************** END OF SPECIFICATIONS *****************************/
#define  WriteCHAR(character)                              \
  if (!G.DEF_ATTR)                                         \
  {                                                        \
     AccProcessChar( (WCHAR) character);                   \
  };                                                       \


/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: NewLine                                                 */
/*                                                                            */
/*   DESCRIPTIVE NAME: This function is called to write the contents of the   */
/*                     line buffer to the output buffer.                      */
/*                                                                            */
/*   FUNCTION:  This function calls process data to BIDI-process the data     */
/*              in the line buffer. Then it calls the post process to send    */
/*              data into the output buffer and embed proper controls. and    */
/*              the final call to initialize the buffer with blanks.          */
/*                                                                            */
/*   ENTRY POINT: NewLine()                                                   */
/*       LINKAGE: CALL    from                                                */
/*                 CHANGE_LL                                                  */
/*                _FLUSH_BUFFER                                               */
/*                _PRINT_BUFF                                                 */
/*                _SET_HOR_MARGINS                                            */
/*                _RIGHT_MARGIN_FOUND                                         */
/*                _REVERSE_LF_n                                               */
/*                _REVERSE_LF                                                 */
/*                _PRINTER_RESET                                              */
/*                                                                            */
/*   INPUT: NONE                                                              */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION: NONE                                           */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*         G       OTHER         -Structure EntryPoint which contains         */
/*                                the processing context of the current       */
/*                                input buffer.                               */
/*                                                                            */
/*      ROUTINES:                                                             */
/*        ProcessData                                                         */
/*        PostProcess                                                         */
/*        InitializeBuffer                                                    */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*                                                                            */
/************************** END OF SPECIFICATIONS *****************************/
void NewLine ()
{

   if (G.CurrPos)
   {
     ProcessData();
            /******************************************************/
            /* call ProcesssData to process the line in the line  */
            /* buffer according to the current BIDI ATTRIBUTE.    */
            /******************************************************/

     if (TASHKEEL)       /* Maya    4/8/1993 */
         PostProcessTashkeel();
     else PostProcess(G.LineBuff);
            /**********************************************************/
            /* call PostProcess to reembed the prober ESCape sequnces */
            /* and Control codes while writing the line to the        */
            /* output buffer.                                         */
            /**********************************************************/
   }

   G.CurrPos       = 0;
             /*******************************************************/
             /* Line is empty; Put the next character at position 0 */
             /*******************************************************/

   G.END_LINE_FLAG = 1;
            /***************************************************/
            /* set END_LINE_FLAG since an end of line occured. */
            /***************************************************/

   InitializeBuffer();
            /************************************************/
            /* Initialize Fills the line buffer with blanks */
            /************************************************/
}





/*/////////////////////////////////////////////////////////////////////////*/
/*/////////////////////////// ESC-SEQUENCES ///////////////////////////////*/
/*/////////////////////////////////////////////////////////////////////////*/

/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: NewWidth                                                */
/*                                                                            */
/*   DESCRIPTIVE NAME: This function is used to set G.CWIDTH to a new pitch   */
/*                                                                            */
/*   FUNCTION: G.CWIDTH is updated to the new value while updating the        */
/*             presentation status so as 12, 17, psm and selected font bits   */
/*	       are off.		        				      */
/*                                                                            */
/*   ENTRY POINT: (NewWidth)	                                              */
/*       LINKAGE: CALL   (from SELECT_FONT)                                   */
/*                                                                            */
/*   INPUT: x                                                                 */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION:                                                */
/*          temp_buff       DWORD      - a temporay storage to keep bits for  */
/*                                       updating the presentation status     */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*          G               OTHER       -Structure EntryPoint which contains  */
/*                                       the processing context of the current*/
/*                                       input buffer.                        */
/*                                                                            */
/*      ROUTINES:                                                             */
/*           None                                                             */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES: NONE                                                        */
/************************** END OF SPECIFICATIONS *****************************/

void NewWidth (int x)
{
unsigned long temp_buff;

 if ((G.PRESENTION_STATUS & 0x00C00000) != 0)
	G.CWIDTH = x * 2;
 else 
	G.CWIDTH = x;
}


/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: _BIDI_SEQ_FOUND                                         */
/*                                                                            */
/*   DESCRIPTIVE NAME: This function is used to update the BIDI ATTRIBUTE     */
/*                     if a BIDI ESCape Sequence was detected on either job   */
/*                     or line boundaries.                                    */
/*                                                                            */
/*   FUNCTION: When a BIDI ESCape Sequence is detected, the next following    */
/*             8 characters should be read (4 for the length and 4 for the    */
/*             BIDI ATTRIBUTE). A check is then made to determine if we are   */
/*             on line boundaries or not. If not then the BIDI ATTRIBUTE      */
/*             is not updated. Else the BIDI ATTRIBUTE is updated .  In any   */
/*             case the sequence is consumed, removed from the input stream.  */
/*                                                                            */
/*   ENTRY POINT: (_BIDI_SEQ_FOUND)                                           */
/*       LINKAGE: CALL   (from Processor)                                     */
/*                                                                            */
/*   INPUT: NONE                                                              */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION:                                                */
/*          length           DWORD      -the length of the BIDI ATTRIBUTE.    */
/*                                       ( currently ignored )                */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*          G               OTHER       -Structure EntryPoint which contains  */
/*                                       the processing context of the current*/
/*                                       input buffer.                        */
/*                                                                            */
/*      ROUTINES:                                                             */
/*           NextChar                                                         */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES: NONE                                                        */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char _BIDI_SEQ_FOUND (void)
{
  unsigned long   length;
  unsigned char   c;

    while  (G.PRT_ESC_OFFSET<4)
      if (NextChar (&c))
        return 1 ;
      else
        G.COUNT[G.PRT_ESC_OFFSET++] = (char) c;
             /************************************/
             /* Read character from input buffer */
             /************************************/

    length = * ((unsigned long *)G.COUNT);

    while  (G.PRT_ESC_OFFSET<8)
       if (NextChar (&c))
          return 1;
       else
          G.COUNT[(G.PRT_ESC_OFFSET++)-4] = (char) c;
             /************************************/
             /* Read character from input buffer */
             /************************************/
/*    if (G.END_LINE_FLAG == 1){
*/
       G.BIDI_ATTR   = G.COUNT[3]*256L*256L*256L + G.COUNT[2]*256L*256L +
                           G.COUNT[1]*256L + G.COUNT[0];

       G.ORIENTATION =  G.COUNT[2];
       G.DEF_ATTR    = (char) (G.BIDI_ATTR==DEFAULT_BIDI_ATTRIBUTES);
/*    } */

    if (G.BIDI_ATTR & 0x00000010)       /* base shapes*/
       set_new_layout_values((G.BIDI_ATTR & 0xFFFF0000),
                             (G.BIDI_ATTR & 0x0000FFFF));
    else if (G.BIDI_ATTR & 0x00000001)    /* passthrough */
         set_new_layout_values((G.BIDI_ATTR & 0xFFFF0000),
                               (G.BIDI_ATTR & 0x0000FFC0));
         else set_new_layout_values((G.BIDI_ATTR & 0xFFFF0000)|0x00000010,
                                    (G.BIDI_ATTR & 0x0000FFFF));
    return 0;
 }


/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _ESC_SINGLE_0_1                                          */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is for ESCape sequences that use a        */
/*                    toggle of character 0/1 or ASCII character "0/1" for    */
/*                    enable and disable a feature.                           */
/*                    An example is the 'ESC W 0/1' for double width          */
/*                    printing.                                               */
/*                                                                            */
/*  FUNCTION: The function is required to read the next character in the      */
/*            data input buffer to see whether it is a zero or a one and set  */
/*            Char_Attr accordingly. Therefore a check is made at the         */
/*            beginning to see if we have reached the end of the input buffer */
/*            or not. If we have reached the end of the input buffer the      */
/*            function returns a return code equal to one to indicate that    */
/*            the function was not completed. Else get the next character and */
/*            set Char_Attr accordingly.                                      */
/*                                                                            */
/*  ENTRY POINT: (*Functions[ESC_SINGLE_0_1])                                 */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*         c               CHAR        - Temporary variable to store          */
/*                                       the next character.                  */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         Char_Attr       CHAR        -the attribute associated with the     */
/*                                      escape SubCode .                      */
/*                                                                            */
/*     ROUTINES:                                                              */
/*        NextChar                                                            */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _ESC_SINGLE_0_1(void)
{
unsigned char  c;


  if (NextChar(&c))
    return 1;

  else
    {
     switch (c%2)   /* IRENE : was switch (c)*/
          {
       case '0'  :
       case '\0' :
                   Char_Attr &= 0x7F;
                   break;

       case '1'  :
       case 0x01 :
                   break;

       default :
                 Char_Attr = 0;
                 break;
      }                      /* end switch */
     return 0;
    }  /* end else */
}

/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _ESC_I                                                   */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is for ESCape I, for now it will be       */
/*                    passthrough                                             */
/*                    Later on  it will be modified to support ESC I          */
/*                                                                            */
/*  FUNCTION: To be filled later, after supporting esc I                      */
/*                                                                            */
/*  ENTRY POINT: (*Functions[ESC_I])                                          */
/*      LINKAGE: CALL (from Processor)                                        */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*         c               CHAR        - Temporary variable to store          */
/*                                       the next character.                  */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         Char_Attr       CHAR        -the attribute associated with the     */
/*                                      escape SubCode .                      */
/*                                                                            */
/*     ROUTINES:                                                              */
/*        NextChar                                                            */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _ESC_I (void)
{
unsigned char  c;
unsigned char  i;
unsigned long temp_buff;
int dw;


  if (NextChar(&c))
    return 1;
  
  G.ESC_I_FLAG = 1;
  G.ESC_I_SEQ [2] = c;

   /* to support ESC I change pitch to pitch of selected resident font */
	/* and update presentation status to reflect new pitch */
  switch (G.PRT_NUM)
	{
	  case _4201 :
	  case _4202 :
			switch (c)
			  {
				case I_4201_0 :
				case I_4201_2 :
				case I_4201_3 :
				case I_4201_4 :
				case I_4201_6 :
				case I_4201_7 :
				case I_4201_11 :
				case I_4201_15 :
					    /* 10 cpi */
					    NewWidth (204);
				break;
				case I_4201_1 :
				case I_4201_5 :
					    /* 12 cpi */
					    NewWidth (170);
				 	    G.PRESENTION_STATUS |=
							0x01000000;
						/* end 17cpi and psm */
					    G.PRESENTION_STATUS &=  
						  	0xFDFEFFFF;
				break;
			  }
	  break;
	  case _4207 :
	  case _4208 :
			switch (c)
			  {
				case I_4207_0 :
				case I_4207_2 :
				case I_4207_4 :
				case I_4207_6 :
					   /* 10 cpi */
					   NewWidth (204);
				break;
				case I_4207_8 :
				case I_4207_10 :
				case I_4207_12 :
				case I_4207_14 :
					    /* 12 cpi */
					    NewWidth (170);
				 	    G.PRESENTION_STATUS |=
							0x01000000;
						/* end 17cpi and psm */
					    G.PRESENTION_STATUS &=  
						  	0xFDFEFFFF;
				break;
				case I_4207_16 :
				case I_4207_18 :
				case I_4207_20 :
				case I_4207_22 :
					    /* 17 cpi */
					    NewWidth (119);
					    G.PRESENTION_STATUS |=
						  	0x02000000;
						/* end 12 cpi and psm */
					    G.PRESENTION_STATUS &=
						  	0xFEFEFFFF;
				break;
				case I_4207_3 :
				case I_4207_7 :
				   	   /* psm */
					   NewWidth (204);	
					   G.PRESENTION_STATUS |=
						  	0x00010000;
						/* end 12 cpi and 17 cpi */
					   G.PRESENTION_STATUS &=
						  	0xFCFFFFFF;
				break;
			  }
	  break;
	  case _5202 :
			switch (c)
			  {
				case I_5202_32 :
				case I_5202_35 :
					   /* 10 cpi */
					   NewWidth (204);
				break;
				case I_5202_33 :
					    /* 12 cpi */
					    NewWidth (170);
				 	    G.PRESENTION_STATUS |=
							0x01000000;
						/* end 17cpi and psm */
					    G.PRESENTION_STATUS &=  
						  	0xFDFEFFFF;
				break;
				case I_5202_34 :
					    /* 17 cpi */
					    NewWidth (119);
					    G.PRESENTION_STATUS |=
						  	0x02000000;
						/* end 12 cpi and psm */
					    G.PRESENTION_STATUS &=
						  	0xFEFEFFFF;
				break;
			  }
	  break;
	  case _5204 :
			switch (c)
			  {
				case I_5204_32 :
				case I_5204_36 :
					   /* 10 cpi */
					   NewWidth (204);
				break;
				case I_5204_33 :
				case I_5204_37 :
					    /* 12 cpi */
					    NewWidth (170);
				 	    G.PRESENTION_STATUS |=
							0x01000000;
						/* end 17cpi and psm */
					    G.PRESENTION_STATUS &=  
						  	0xFDFEFFFF;
				break;
				case I_5204_34 :
				case I_5204_38 :
					    /* 17 cpi */
					    NewWidth (119);
					    G.PRESENTION_STATUS |=
						  	0x02000000;
						/* end 12 cpi and psm */
					    G.PRESENTION_STATUS &=
						  	0xFEFEFFFF;
				break;
				case I_5204_35 :
				case I_5204_39 :
				   	   /* psm */
					   NewWidth (204);	
					   G.PRESENTION_STATUS |=
						  	0x00010000;
						/* end 12 cpi and 17 cpi */
					   G.PRESENTION_STATUS &=
						  	0xFCFFFFFF;
				break;
			  }
	  break;
	  case _4019 :
			switch (c)
			  {
				case I_4019_32 :
				case I_4019_36 :
				case I_4019_40 :
				case I_4019_41 :
					    /* 10 cpi */
					    NewWidth (204);
				break;
				case I_4019_33 :
				case I_4019_37 :
					    /* 12 cpi */
					    NewWidth (170);
				 	    G.PRESENTION_STATUS |=
							0x01000000;
						/* end 17cpi and psm */
					    G.PRESENTION_STATUS &=  
						  	0xFDFEFFFF;
				break;
				case I_4019_34 :
				case I_4019_38 :
					    /* 17 cpi */
					    NewWidth (119);
					    G.PRESENTION_STATUS |=
						  	0x02000000;
						/* end 12 cpi and psm */
					    G.PRESENTION_STATUS &=
						  	0xFEFEFFFF;
				break;
				case I_4019_35 :
				case I_4019_39 :
				   	   /* psm */
					   NewWidth (204);	
					   G.PRESENTION_STATUS |=
						  	0x00010000;
						/* end 12 cpi and 17 cpi */
					   G.PRESENTION_STATUS &=
						  	0xFCFFFFFF;
				break;
			  }

	  break;
	}  /* end switch */
  return (0);
}

/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _ESC_C_FOUND                                             */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is for the standard 'ESC C ...'           */
/*                    sequence used to set the page size in inches and/or     */
/*                    lines of text.                                          */
/*                                                                            */
/*  FUNCTION: The standard 'ESC C ...' is a special ESCape sequence since     */
/*            its length could either be 3 or 4 bytes depending on the        */
/*            value of the third byte. The function is required to read       */
/*            the next character in the data input buffer and see if its      */
/*            value is a zero or not. If the value of the third byte in the   */
/*            sequence is a zero, then the next character in the input buffer */
/*            is read and all 4 bytes are sent directly to the printer.       */
/*            If the value of the third byte in the sequence is not a zero    */
/*            then the sequence is only three bytes long and the three        */
/*            bytes are send to the printer.                                  */
/*            A check is made before reading each character to see if we      */
/*            have reached the end of the input buffer or not. If we have     */
/*            reached the end of the input buffer, the function returns a     */
/*            return code equal to one to indicate that the function was not  */
/*            completed and PRT_ESC_OFFSET is set to the number of characters */
/*            read within this function such that the next time the function  */
/*            is called we enter the function in the exact place where we     */
/*            left the previous time.                                         */
/*                                                                            */
/*  ENTRY POINT: (*Functions[ESC_C_FOUND])                                    */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*         c               CHAR        - Temporary variable to store          */
/*                                       the next character.                  */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*          G               OTHER       -Structure EntryPoint which contains  */
/*                                       the processing context of the current*/
/*                                       input buffer.                        */
/*                                                                            */
/*     ROUTINES:                                                              */
/*          NextChar                                                          */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _ESC_C_FOUND(void)
{ unsigned char   c;

      switch (G.PRT_ESC_OFFSET){
      case 0:
         WriteESC;
         if (NextChar(&c))
            return 1;
         else  {
                 WriteCHAR( c );
                 if (c != 0)
                    return 0;
                 else
                    G.PRT_ESC_OFFSET++;
                }

      case 1:
         if (NextChar(&c))
            return 1;
         else  {
                WriteCHAR( (long) c );
                return 0;
            }
      }                                                         /*end switch*/
 };



/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: _ESC_SINGLE                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME: For ESCape sequences such as 'ESC E' this function     */
/*                     will act on the corresponding printer encoded          */
/*                     ATTRIBUTE.  This is only for ESCape sequences that     */
/*                     are two bytes long.  The function also implements      */
/*                     some of the 420X priority chart which states:          */
/*                          Pitch 17 +  Emphasize -->   Emphasize             */
/*                          Pitch 17 +  Pitch 12  -->   Pitch 12              */
/*                                                                            */
/*   FUNCTION:  The function  implements  some of the 420X priority chart     */
/*              which states:                                                 */
/*                   Pitch 17 +  Emphasize -->   Emphasize                    */
/*                   Pitch 17 +  Pitch 12  -->   Pitch 12                     */
/*              that is if emphasize is to be turned off, the function        */
/*              turns off the condensed mode; and similarly for 12CPI.        */
/*              The update of the PRESENTATION_STATUS is left to the          */
/*              Processor to carry out.                                       */
/*                                                                            */
/*   ENTRY POINT: (*Functions[ESC_SINGLE])                                    */
/*       LINKAGE: CALL   (from Processor)                                     */
/*                                                                            */
/*   INPUT: NONE                                                              */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION: NONE                                           */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*          G               OTHER       -Structure EntryPoint which contains  */
/*                                       the processing context of the current*/
/*                                       input buffer.                        */
/*                                                                            */
/*          PRT_SPACE_FOR   OTHER       -Array of characters that is used     */
/*                                       here to know whether the printer     */
/*                                       is one of the 420X family.           */
/*                                                                            */
/*          PRT_CWIDTH                  -Printer character width in apels.    */
/*                                                                            */
/*          PRT_SPEC_PITCH              - Character Width in APELS in case of */
/*                                        special pitch 17.125 .              */
/*      ROUTINES: NONE                                                        */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES: NONE                                                        */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _ESC_SINGLE(void)
{
    switch (G.ESC_SubCode) {
    case 'E':
       if (((G.PRESENTION_STATUS & 0X02000000)!=0) && (CFG->PRT_SPACE_FOR[0]==0) ){
           /* check if the condensed mode is on and the printer type is    */
           /* the one that do not support the combination of condesed mode */
           /* plus emphasize                                               */
         G.PRESENTION_STATUS &= 0XFDFFFFFF;
           /* set condensed mode off                                       */
         G.COND_EMP=1;

         if ((G.PRESENTION_STATUS & 0X00C00000)!=0)
           /* check on double width                                        */
            G.CWIDTH = CFG->PRT_OLD_CWIDTH*2;
         else
            G.CWIDTH = CFG->PRT_OLD_CWIDTH;
        }
    break;

    case 'F':
     if (G.COND_EMP) {
         G.PRESENTION_STATUS |= 0X02000000;
           /* if we were in condensed+emphasize , set the condensed mode on */

         G.COND_EMP = 0;

         if ((G.PRESENTION_STATUS & 0X00C00000)!=0)
           /* if double width ( temporary or contineous ) is on...          */
            G.CWIDTH = CFG->PRT_SPEC_PITCH*2;
         else
            G.CWIDTH = CFG->PRT_SPEC_PITCH;

     }
    break;

    case ':':

	/* to turn off escI flag */
	G.ESC_I_FLAG = 0;

       /* turn off esc [ I bit */
       G.PRESENTION_STATUS &= 0xFFFF7FFF;

       G.COND_EMP=0;
       if (((G.PRESENTION_STATUS & 0X02000000)!=0) && (CFG->PRT_SPACE_FOR[0]==0) ) {
           /* if we detect 12 cpi then we turn off the condensed if it  */
           /* was active, this is for the 420x family only              */
         G.PRESENTION_STATUS &= 0XFDFFFFFF;
           /* set condensed mode off                                       */
         if ((G.PRESENTION_STATUS & 0X00C00000)!=0)
           /* check on double width                                        */
            G.CWIDTH = CFG->PRT_OLD_CWIDTH*2;
         else
            G.CWIDTH = CFG->PRT_OLD_CWIDTH;
     }
    break;
    } /* endswitch */

    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _PRT_DESELECT_n                                          */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is for double byte codes that will        */
/*                    send the printer to an OFF-LINE mode (i.e. the          */
/*                    characters sent to the printer will NOT be printed.)    */
/*                                                                            */
/*  FUNCTION: When the function is called a test is made on the 3rd byte      */
/*            to see if the printer required to be deselected is the          */
/*            current printer or not. If this is true then the flag           */
/*            (SELECT_FLAG) is reset to indicate that the printer is          */
/*            OFF-LINE.                                                       */
/*                                                                            */
/*  ENTRY POINT: (*Functions[PRT_DESELECT_n])                                 */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*          c               CHAR       - Temporary variable to store          */
/*                                       the next character.                  */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*          G               OTHER      - Structure EntryPoint which contains  */
/*                                       the processing context of the current*/
/*                                       input buffer.                        */
/*                                                                            */
/*          DESELECT_n      CHAR       - The one Value of 'n' that causes     */
/*                                       the printer to go OFF-LINE.          */
/*                                                                            */
/*     ROUTINES: NONE                                                         */
/*         NextChar                                                           */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _PRT_DESELECT_n(void)
{ unsigned char   c;

     if (NextChar(&c))
        return 1;
     else  {
       WriteESC  ;
       WriteCHAR ((long)c);
       if (c== CFG->DESELECT_n)
         G.SELECT_FLAG = 0;
       return 0;
     }
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _ESC_SUB_SUPER                                           */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is for toggle type ESCape sequences       */
/*                    (as per ESC_SINGLE_0_1). The ESCape sequence will       */
/*                    invoke the super/subscript form depending on the        */
/*                    3rd byte value.                                         */
/*                                                                            */
/*  FUNCTION: The function is required to read the next character in the      */
/*            data input buffer to see whether it is a zero or a one and set  */
/*            Char_Attr accordingly. Therefore a check is made at the         */
/*            beginning to see if we have reached the end of input buffer or  */
/*            not. If we have reached the end of input buffer the function    */
/*            returns a return code equal to one to indicate that the         */
/*            function was not completed. Else get the next character and     */
/*            set Char_Attr accordingly.                                      */
/*                                                                            */
/*  ENTRY POINT: (*Functions[ESC_SUB_SUPER])                                  */
/*      LINKAGE: CALL (from Processor)                                        */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*         c                  CHAR     - Temporary variable to store          */
/*                                           the next character.              */
/*                                                                            */
/*         OFF                DWORD    - Temporary variable to store          */
/*                                       intermediate calculations.           */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G                  OTHER    - Structure EntryPoint which contains  */
/*                                       the processing context of the current*/
/*                                       input buffer.                        */
/*                                                                            */
/*         Char_Attr          CHAR     - the attribute associated with the    */
/*                                       code itself.                         */
/*                                                                            */
/*     ROUTINES:                                                              */
/*           NextChar                                                         */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES:  NONE                                                        */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _ESC_SUB_SUPER(void)
{ unsigned char  c;
  unsigned long OFF;

     if (NextChar(&c))
        return 1;
     else  {
         switch (c%2){   /* IRENE : was switch (c) */
            case 0x30:
            case 0x00:
              if (G.ESC_SubCode=='S') {
                 OFF = (Char_Attr & 0x1f);
                 OFF = ((unsigned long) 1 << OFF);
                 OFF = ~OFF;
                 G.PRESENTION_STATUS &= OFF;
              } /* endif */
              Char_Attr -= 1;
            break;

            case 0x31:
            case 0x01:
              if (G.ESC_SubCode=='S') {
                 OFF = ((Char_Attr-1) & 0x1f);
                 OFF = ( (unsigned long) 1 << OFF);
                 OFF = ~OFF;
                 G.PRESENTION_STATUS &= OFF;
              } /* endif */
            break;

            default:
               Char_Attr = 0;
            break;
         }                                                     /* end switch */
         return 0;
     }
 }




/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _SET_HOR_MARGINS                                         */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is similar to the setting of the left     */
/*                    and right margins of the printing line.  The            */
/*                    difference is that one sequence sets both margins.      */
/*                    This is four byte sequence that has                     */
/*                                'ESC x left right' as the ESCape format.    */
/*                                                                            */
/*  FUNCTION: when the above ESCape sequence is detected. Both the left       */
/*            (PRT_L_MARGIN) and right (PRT_R_MARGIN) margins are set to      */
/*            the value stated and the sequence is sent to the printer.       */
/*            The printer's line length (PRT_LINE_LENGTH) is calculated       */
/*            from the value of the both the left and right margins in        */
/*            APELS such that when inserting the trailing spaces, the         */
/*            proper number of spaces are inserted.                           */
/*                                                                            */
/*  ENTRY POINT: (*Functions[SET_HOR_MARGINS])                                */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*         c               CHAR        -a dummy varaiable used to store the   */
/*                                      characters read from the data input   */
/*                                      buffer.                               */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*         PRT_OLD_CWIDTH  WORD        -Default pitch in APELS                */
/*                                                                            */
/*                                                                            */
/*     ROUTINES:                                                              */
/*       NextChar                                                             */
/*       NewLine                                                              */
/*       AccProcessChar                                                       */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _SET_HOR_MARGINS (void)
{  unsigned  char  c;

   switch (G.PRT_ESC_OFFSET){
   case 0:
       if (NextChar(&c))
             return 1;
       else  {
              if  (c > G.PRT_R_MARGIN)
                 G.PRT_L_MARGIN = 0;
              else if (c != 0)
                 G.PRT_L_MARGIN = c-1;

              G.PRT_ESC_OFFSET++;
             }

   case 1:
       if (NextChar(&c))
             return 1;
       else  {
             if ( (c < G.PRT_L_MARGIN) || (c > (unsigned char) G.SFN_PRT_LAST) )
                G.PRT_R_MARGIN = (unsigned char)G.SFN_PRT_LAST;
             else if (c != 0)
                G.PRT_R_MARGIN = c;

             G.PRT_LINE_LENGTH = (G.PRT_R_MARGIN - G.PRT_L_MARGIN)*CFG->PRT_OLD_CWIDTH;

             if (G.LINE_LENGTH > G.PRT_LINE_LENGTH) {
                NewLine();
                G.LINE_LENGTH = 0;
                AccProcessChar( (WCHAR)  10L);
               }

             if (!G.DEF_ATTR 
			&& ((G.PRESENTION_STATUS & 0x00008000) >> 15) == 0) {
			/* set margins if no font is selected */
                AccProcessChar( (WCHAR)  _ESCAPE );
                AccProcessChar( (WCHAR)  G.ESC_SubCode );
                AccProcessChar( (WCHAR)  G.PRT_L_MARGIN+ 1);
                AccProcessChar( (WCHAR)  G.PRT_R_MARGIN);
                AccProcessChar( (WCHAR)  13L);
             } /* endif */

             return 0;
           } /* endif */
   } /* end switch */
}




/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _LEFT_MARGIN_FOUND                                       */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function sets the left margin in terms of          */
/*                    columns.                                                */
/*                                                                            */
/*  FUNCTION: when the above ESCape sequence is detected. The left            */
/*            margin (PRT_L_MARGIN) is set to the value stated and the        */
/*            sequence is sent to the printer.                                */
/*            The printer's line length (PRT_LINE_LENGTH) is decremented      */
/*            by the value of the left margin in APELS such that when         */
/*            inserting the trailing spaces, the proper number of spaces      */
/*            are inserted.                                                   */
/*                                                                            */
/*  ENTRY POINT: (*Functions[LEFT_MARGIN_FOUND])                              */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*         c               CHAR        - Temporary variable to store          */
/*                                       the next character.                  */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*         PRT_OLD_CWIDTH              -Default pitch in APELS                */
/*                                                                            */
/*     ROUTINES:                                                              */
/*         AccProcessChar                                                     */
/*         NextChar                                                           */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _LEFT_MARGIN_FOUND  (void)
{ unsigned char  c;

      if (NextChar(&c))
           return 1;
      else {
             if (!G.DEF_ATTR) {
                AccProcessChar( (WCHAR)  _ESCAPE );
                AccProcessChar( (WCHAR)  G.ESC_SubCode );
                AccProcessChar( (WCHAR)  c);
                AccProcessChar( (WCHAR)  13L);
             }
             G.PRT_L_MARGIN = c;
             G.PRT_LINE_LENGTH = (G.PRT_R_MARGIN - G.PRT_L_MARGIN) * CFG->PRT_OLD_CWIDTH;
             return 0;
           }
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _RIGHT_MARGIN_FOUND                                      */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function sets the right margin in terms of         */
/*                    columns.                                                */
/*                                                                            */
/*  FUNCTION: when the above ESCape sequence is detected, the line buffer     */
/*            is processed and written into the outout buffer. The right      */
/*            margin (PRT_R_MARGIN) is set to the value stated and the        */
/*            sequence is sent to the printer.                                */
/*            The printer's line length (PRT_LINE_LENGTH) is decremented      */
/*            by the value of the right margin in APELS such that when        */
/*            inserting the trailing spaces, the proper number of spaces      */
/*            are inserted.                                                   */
/*                                                                            */
/*  ENTRY POINT: (*Functions[RIGHT_MARGIN_FOUND])                             */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*         c               CHAR        - Temporary variable to store          */
/*                                           the next character.              */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*         PRT_OLD_CWIDTH              -Default pitch in APELS                */
/*                                                                            */
/*     ROUTINES:                                                              */
/*       NextChar                                                             */
/*       NewLine                                                              */
/*       AccProcessChar                                                       */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _RIGHT_MARGIN_FOUND  (void)
{ unsigned char  c;

      if (NextChar(&c))
         return 1;
      else  {
         G.PRT_R_MARGIN = c;
         G.PRT_LINE_LENGTH = (G.PRT_R_MARGIN - G.PRT_L_MARGIN) * CFG->PRT_OLD_CWIDTH;

         if ( G.LINE_LENGTH > G.PRT_LINE_LENGTH) {
            NewLine();
            G.LINE_LENGTH = 0;
            AccProcessChar( (WCHAR)  _ESCAPE);
            AccProcessChar( (WCHAR)  G.ESC_SubCode);
            AccProcessChar( (WCHAR)  G.PRT_R_MARGIN);
            AccProcessChar( (WCHAR)  10L);
            AccProcessChar( (WCHAR)  13L);
         }

         return 0;
      } /* endif */
 }



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _REVERSE_LF_n                                            */
/*                                                                            */
/*  DESCRIPTIVE NAME: This is the same as _REVERSE_LF except it is for        */
/*                    codes that are 3 bytes long.                            */
/*                                                                            */
/*  FUNCTION: When the function is called indicating an end of line,          */
/*            a number of spaces is inserted at the end of the line to        */
/*            complete a line. The line is then processed as required.        */
/*            After the line has been processed, PostProcessore is            */
/*            called to  embed the correct Control Code and ESCape            */
/*            sequences and write the line buffer (Line Buff) to the          */
/*            output buffer. The third byte of the sequences is then read     */
/*            from the input buffer and the whole sequence is then written to */
/*            the output buffer following the line to cause a line feed.      */
/*            A Carriage Return is also written to the output buffer to move  */
/*            the print head to the left.                                     */
/*            The line length (LINE_LENGTH) is not initialized to zero        */
/*            since we want the first character in the next line to be        */
/*            printed rigth after the last character in the previous line.    */
/*                                                                            */
/*  ENTRY POINT: (*Functions[_REVERSE_LF_n])                                  */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*         c               CHAR        - Temporary variable to store          */
/*                                       the next character.                  */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES:                                                              */
/*        AccProcessChar                                                      */
/*        NextChar                                                            */
/*        InsertSpaces                                                        */
/*        NewLine                                                             */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _REVERSE_LF_n (void)
{ unsigned char  c;

     if (NextChar(&c))
        return 1;
     else  {
        if (!G.DEF_ATTR) {
          if (G.ORIENTATION == 1)
          {
	    G.INSERT_SPACES_START = bidi_out_buff_len; /* 4/8/1993 TASHKEEL */
            InsertSpaces();
            G.INSERT_SPACES_END   = bidi_out_buff_len; /* 4/8/1993 TASHKEEL */
          }

          NewLine();
          G.INSERT_SPACES_START = G.INSERT_SPACES_END = 0; /* 4/8/1993 TASHKEEL */
          AccProcessChar( (WCHAR)  _ESCAPE);
          AccProcessChar( (WCHAR)  G.ESC_SubCode);
          AccProcessChar( (WCHAR)  c);

          if (G.ORIENTATION == 1)
             AccProcessChar( (WCHAR)  13L);
        }
        return 0;
     }
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _REVERSE_LF                                              */
/*                                                                            */
/*  DESCRIPTIVE NAME: This is the same as _FLUSH_BUFFER except it is for      */
/*                    printers that support Reverse Line Feeds.               */
/*                    The sequence is 2 bytes long.                           */
/*                                                                            */
/*  FUNCTION: When the function is called indicating an end of line,          */
/*            a number of spaces is inserted at the end of the line to        */
/*            complete a line. The line is then processed as required.        */
/*            After the line has been processed, the post processor is        */
/*            called to embed the correct Control Code and ESCape             */
/*            sequences and write the line buffer (Line Buff) to the          */
/*            output buffer. The sequence is then written to the output       */
/*            buffer following the line to cause a line feed.                 */
/*            A Carriage Return is also written to the output buffer to move  */
/*            the print head to the left.                                     */
/*            The line length (LINE_LENGTH) is not initialized to zero        */
/*            since we want the first character in the next line to be        */
/*            printed rigth after the last character in the previous line.    */
/*                                                                            */
/*  ENTRY POINT: (*Functions[_REVERSE_LF])                                    */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES:                                                              */
/*        InsertSpaces                                                        */
/*        NewLine                                                             */
/*        AccProcessChar                                                      */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _REVERSE_LF  (void)
{

    if (!G.DEF_ATTR) {
       if (G.ORIENTATION == 1)
       {
         G.INSERT_SPACES_START = bidi_out_buff_len;   /* 4/8/1993 TASHKEEL */
         InsertSpaces();
         G.INSERT_SPACES_END   = bidi_out_buff_len;   /* 4/8/1993 TASHKEEL */
       }

       NewLine();
       G.INSERT_SPACES_START = G.INSERT_SPACES_END = 0;  /* 4/8/1993 TASHKEEL */
       WriteESC;

       if (G.ORIENTATION == 1)
          AccProcessChar( (WCHAR)  13L);
    }
    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _ZERO_TERMINATOR                                         */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is used for ESCape sequences which        */
/*                    are sent directly to the printer and  end with a        */
/*                    character 0.   ( to indicate that the ESCape            */
/*                    sequence has completed).                                */
/*                                                                            */
/*  FUNCTION: The function sends the ESCape and ESCape subcode directly       */
/*            to the printer and sets a flag (ZERO_TERM_FLAG)to indicate      */
/*            that all following characters read from the input buffers are   */
/*            sent to the printer directly until a zero is detected. The      */
/*            Processor checks this flag and performs the required function.  */
/*            Note that the zero will also be sent to the printer.            */
/*                                                                            */
/*  ENTRY POINT: (*Functions[ZERO_TERMINATOR])                                */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:NONE                                             */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES: NONE                                                         */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _ZERO_TERMINATOR (void)
{
    WriteESC;
    G.ZERO_TERM_FLAG = 1;
    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _TAB_SET_FOUND                                           */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is used to set the new tab settings       */
/*                    for horizontal tabs.                                    */
/*                                                                            */
/*  FUNCTION: The ESCape sequence used to set the horizontal tabs is          */
/*            terminated by character 0 and the values following the          */
/*            escape subcode are the column tab settings.                     */
/*            The values are checked for validity ,  stored in array          */
/*            of tabs (PRT_TABS), and are also sent to the printer.           */
/*                                                                            */
/*  ENTRY POINT: (*Functions[TAB_SET_FOUND])                                  */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:NONE                                             */
/*         c               CHAR        - Temporary variable to store          */
/*                                       the next character.                  */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES:                                                              */
/*        NextChar                                                            */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _TAB_SET_FOUND (void)
{ unsigned char c;

    WriteESC;
    while (G.PRT_CURR_TAB<28) {
      if (NextChar(&c))
          return 1;
      else  {
              WriteCHAR( (long) c );
              if (c==0){
                 while (G.PRT_CURR_TAB<28)
                    G.PRT_TABS[G.PRT_CURR_TAB++]=0;
              }
              else{
                 if ((CFG->TAB_FLAG & 1) == 0)
                    --c;
                 G.PRT_TABS[G.PRT_CURR_TAB++] = (c < G.PRT_R_MARGIN) ?
                                                 c : 0;
              }
        }
    } /* endwhile */

    G.PRT_CURR_TAB=0;
    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _RESET_TAB_SETTINGS                                      */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function return the printer back to the            */
/*                    printer's default horizontal tab settings.              */
/*                                                                            */
/*  FUNCTION: when an ESCape sequence reseting the horizontal tabs            */
/*            to the printer's default tab settings is detected, the          */
/*            array of tabs (PRT_TABS) is initialized to the printer's        */
/*            default tabs (PRT_DEF_TABS) given in the printer's              */
/*            corresponding CFG file.                                         */
/*                                                                            */
/*  ENTRY POINT: (*Functions[RESET_TAB_SETTINGS])                             */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*         PRT_DEF_TABS    @OTHER      -pointer to the array of DEFAULT tabs. */
/*                                                                            */
/*     ROUTINES: NONE                                                         */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES:                                                              */
/*         MemMove                                                            */
/*                                                                            */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _RESET_TAB_SETTINGS(void)
{
   WriteESC;
   MemMove(G.PRT_TABS, CFG->PRT_DEF_TABS, sizeof(CFG->PRT_DEF_TABS));
   return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _PRT_NEXT                                                */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is for single ESC codes that will         */
/*                    print next character as graphic.                        */
/*                                                                            */
/*  FUNCTION: The function is required to print the next character as         */
/*            graphics. Therefore GRAPHICS_CHAR_COUNT is set to ONE.          */
/*            The presentation status (PRESENTION_STATUS) is updated in       */
/*            the Processor, Bit 0 is set.  This Bit remains active           */
/*            while GRAPHICS_CHAR_COUNT>0. GRAPHICS_CHAR_COUNT is             */
/*            decremented  for each printable character (_PRINTABLE).         */
/*                                                                            */
/*                                                                            */
/*  ENTRY POINT: (*Functions[PRT_NEXT])                                       */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES:NONE                                                          */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _PRT_NEXT (void)
{
   G.GRAPHICS_CHAR_COUNT = 1;
   return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _PRT_ALL                                                 */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is for ESCape sequences that are 4        */
/*                    Bytes long that will print the following(Count=LH)      */
/*                    characters as graphic.                                  */
/*                                                                            */
/*  FUNCTION: The function is required to read the next two characters in     */
/*            the data input buffer and set the count of number of characters */
/*            to be printed as graphics (GRAPHICS_CHAR_COUNT) where this      */
/*            count is equal to (3rd byte + 256 * 4th byte). refer to         */
/*            PRT_NEXT for more details.                                      */
/*                                                                            */
/*  ENTRY POINT: (*Functions[PRT_ALL])                                        */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES:                                                              */
/*         NextChar                                                           */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _PRT_ALL (void)
{
    while  (G.PRT_ESC_OFFSET<2)
       if ( NextChar(G.COUNT+G.PRT_ESC_OFFSET) )
          return 1;
       else
          G.PRT_ESC_OFFSET++;
/* 301290 Removed Typecast                                 */
   /*G.GRAPHICS_CHAR_COUNT = *((unsigned int *) G.COUNT);
   */
    G.GRAPHICS_CHAR_COUNT = G.COUNT[1]*256 + G.COUNT[0];
    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _IGNORE_ESC_n                                            */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is for ESCape sequences that are 3        */
/*                    bytes long and their position in the data stream is     */
/*                    not critical.                                           */
/*                    The entire sequence will be sent to the printer         */
/*                    immediately. ( before data in LineBuff )                */
/*                    NOTE: Sending the ESCape sequence immediately           */
/*                    applies to ALL 'IGNORE_ESC....' codes.                  */
/*                                                                            */
/*  FUNCTION: The function is required to read the next character in the      */
/*            data input buffer to send the whole ESCape sequence (3 bytes)   */
/*            to the printer directly. So the ESCape and subcode is sent      */
/*            to the output buffer and IGNORE_COUNT is updated, for the       */
/*            processor to read and pass the rest of the sequence to          */
/*            the output buffer.                                              */
/*                                                                            */
/*  ENTRY POINT: (*Functions[IGNORE_ESC_n])                                   */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*           c                CHAR       - Temporary character to read        */
/*                                         characters in.                     */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES:                                                              */
/*         NextChar                                                           */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _IGNORE_ESC_n(void)
{
    WriteESC;
    G.IGNORE_COUNT = 1;
    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _IGNORE_ESC_n_0                                          */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is for ESCape sequences that are 4        */
/*                    bytes long and their position in the data stream is     */
/*                    not critical.                                           */
/*                    The entire sequence will be sent to the printer         */
/*                    immediately.                                            */
/*                                                                            */
/*  FUNCTION: The function is required to read the next character in the      */
/*            data input buffer to send the whole ESCape sequence (4 bytes)   */
/*            to the printer directly. So the ESCape and subcode is sent      */
/*            to the output buffer and IGNORE_COUNT is updated, for the       */
/*            processor to read and pass the rest of the sequence to          */
/*            the output buffer.                                              */
/*                                                                            */
/*                                                                            */
/*  ENTRY POINT: (*Functions[IGNORE_ESC_n_0])                                 */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*           c                CHAR       - Temporary character to read        */
/*                                         characters in.                     */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES:                                                              */
/*         NextChar                                                           */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _IGNORE_ESC_n_0      (void)
{
    WriteESC;
    G.IGNORE_COUNT = 2;
    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _IGNORE_ESC_n_0_0                                        */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is for ESCape sequences that are 5        */
/*                    bytes long and their position in the data stream is     */
/*                    not critical.                                           */
/*                    The entire sequence will be sent to the printer         */
/*                    immediately.                                            */
/*                                                                            */
/*  FUNCTION: The function is required to read the next character in the      */
/*            data input buffer to send the whole ESCape sequence (5 bytes)   */
/*            to the printer directly. So the ESCape and subcode is sent      */
/*            to the output buffer and IGNORE_COUNT is updated, for the       */
/*            processor to read and pass the rest of the sequence to          */
/*            the output buffer.                                              */
/*                                                                            */
/*  ENTRY POINT: (*Functions[IGNORE_ESC_n_0_0])                               */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*           c                CHAR       - Temporary character to read        */
/*                                         characters in.                     */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES:                                                              */
/*         NextChar                                                           */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _IGNORE_ESC_n_0_0    (void)
{
    WriteESC;
    G.IGNORE_COUNT = 3;
    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _IGNORE_ESC_n_000                                        */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is for ESCape sequences that are 6        */
/*                    bytes long and their position in the data stream is     */
/*                    not critical.                                           */
/*                    The entire sequence will be sent to the printer         */
/*                    immediately.                                            */
/*                                                                            */
/*  FUNCTION: The function is required to read the next character in the      */
/*            data input buffer to send the whole ESCape sequence (6 bytes)   */
/*            to the printer directly. So the ESCape and subcode is sent      */
/*            to the output buffer and IGNORE_COUNT is updated, for the       */
/*            processor to read and pass the rest of the sequence to          */
/*            the output buffer.                                              */
/*                                                                            */
/*  ENTRY POINT: (*Functions[IGNORE_ESC_n_000])                               */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*           c                CHAR       - Temporary character to read        */
/*                                         characters in.                     */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES:                                                              */
/*         NextChar                                                           */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _IGNORE_ESC_n_000    (void)
{
    WriteESC;
    G.IGNORE_COUNT = 4;
    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _NEW_GRAPH                                               */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is used for printers that support         */
/*                    graphics (for dot matrix type printers).                */
/*                                                                            */
/*  FUNCTION: The function has two entry points , one before IGNORE_COUNT is  */
/*            determined (case 0,1) and one after the ESCape sequence is      */
/*            passed (case default). in case 0,1: the IGNORE_COUNT is         */
/*            calculated as n2*256+n1 , This is the number of Data bytes to   */
/*            follow. A check is made to determine the width of the printer   */
/*            carriage, Then the number of PelsPerLine is calculated          */
/*            according to the Graphics mode activated.   The line buffer is  */
/*            flushed.  The LINE_LENGTH is modified such that when the        */
/*            GRAPHICS data is totally passed,Case default, The last bit      */
/*            will be adjacent to the previous Text data in RTL mode.         */
/*            Finally in Defailt Case, a carriage return is sent to move      */
/*            the carriage home (RTL only).                                   */
/*                                                                            */
/*  ENTRY POINT: (*Functions[GRAPHICS])                                       */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*         PelsPerInch    DWORD        -Temporary   variable   that will      */
/*                                      store the number of PELS (Picture     */
/*                                      elements) per Inch; it depends on     */
/*                                      the Graphics density activated.       */
/*                                                                            */
/*         APelsPerInch   DWORD        -Temporary   variable   that will      */
/*                                      store the number of APELS Per Inch.   */
/*                                      This depends on the number and type   */
/*                                      of pitches supported by a printer.    */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*         PRT_D_PITCH     CHAR        -printer default pitch.                */
/*                                                                            */
/*     ROUTINES:                                                              */
/*            NextChar                                                        */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _GRAPHICS            (void)
{ unsigned long   PelsPerInch,
                  ApelsPerInch,
                                  temp1;
char *tmp;

/* irene */
   switch (G.PRT_ESC_OFFSET) {
    case 0:
    case 1: while  (G.PRT_ESC_OFFSET<2)
             if ( NextChar(G.COUNT+G.PRT_ESC_OFFSET) )
                return 1;
             else
		{
                 G.PRT_ESC_OFFSET++;
		 G.LineBuff [G.CurrPos].ch = 0x20; /* neutral */
		 G.LineBuff [G.CurrPos].attr |= 0x00002000; 
						 /* set graphics bit on */
		}


	 G.SAVED_GRAPHICS = G.COUNT[1]*256+G.COUNT[0];
	 G.LineBuff [G.CurrPos].escsize = G.COUNT[1]*256+G.COUNT[0]+4;

	 G.LineBuff [G.CurrPos].escapes = 
			malloc (G.LineBuff [G.CurrPos].escsize);


	 memset (G.LineBuff [G.CurrPos].escapes, 0, 
		G.LineBuff [G.CurrPos].escsize);

         switch (G.ESC_SubCode) {
          case 'K': PelsPerInch =(unsigned long) 60;
                    break;
          case 'L':
          case 'Y': PelsPerInch =(unsigned long) 120;
                    break;
          case 'Z': PelsPerInch =(unsigned long) 240;
                    break;
        /*  default:  WriteESC;
                    WriteCHAR((long)G.COUNT[0]);
                    WriteCHAR((long)G.COUNT[1]);
                    return 0;*/
          } /* endswitch */


        ApelsPerInch    = (unsigned long)((unsigned long) G.PRT_LINE_LENGTH
                                 *(unsigned long) CFG->PRT_D_PITCH)
                                 /(unsigned long) G.SFN_PRT_LAST;


        temp1 = ((unsigned long) G.COUNT[1]) * 256L +
                                           ((unsigned long) G.COUNT[0]) ;
        G.LINE_LENGTH  += (int)(( (unsigned long) temp1
                                * (unsigned long) ApelsPerInch)
                                / (unsigned long) PelsPerInch);


	G.LineBuff [G.CurrPos].escapes [0] = 27;
	G.LineBuff [G.CurrPos].escapes [1] = G.ESC_SubCode;
	G.LineBuff [G.CurrPos].escapes [2] = G.COUNT [0];
	G.LineBuff [G.CurrPos].escapes [3] = G.COUNT [1];
        return 1;

     default:
 	 return 0;
   }  /* endswitch */
}




/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _SPACE_FOR_BAK                                           */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is used for printers that support         */
/*                    horizontal head movement with multiples of 1/120        */
/*                    of an inch. ESC-d moves the head forword while          */
/*                    ESC-e moves the head backwords.                         */
/*                                                                            */
/*  FUNCTION:  The function starts by reading the count n1,n2 which the       */
/*             head will move in 1/120 of an inch. The count is evaluated     */
/*             as n2*256+n1. If the current mode is pass-through (DEF_ATTR)   */
/*             the sequence is passed as received; Else processing starts     */
/*             by flushing the line buffer, a  check is made to determine     */
/*             the printer type (wide/normal) carriage and the value of       */
/*             the line length is updated: (incremented for ESC-d and         */
/*             decremented for ESC-e). Next, the proper number of spaces      */
/*             is inserted to position the head where the ESC-(d/e) would     */
/*             be released; finally the ESCape is passed and a carriage       */
/*             return  is sent to return  the head to the left margin.        */
/*                                                                            */
/*  ENTRY POINT: (*Functions[SPACE_FOR_BAK])                                  */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*         Temp           WORD         -Temporary variable  to store the      */
/*                                      count for further processing.         */
/*                                                                            */
/*         APels          DWORD        -Temporary   variable  to store        */
/*                                      the width of the "SPACE" in APELS.    */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G              OTHER        -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*         PRT_SPACE_FOR  @CHAR        -Pointer to string; The first character*/
/*                                      of which contains the number of APELS */
/*                                      in 1/120 of an inch.                  */
/*                                                                            */
/*     ROUTINES:                                                              */
/*            NextChar                                                        */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _SPACE_FOR_BAK    (void)
{ unsigned long  Apels;
  unsigned long  Temp;

    while  (G.PRT_ESC_OFFSET<2)
       if ( NextChar(G.COUNT+G.PRT_ESC_OFFSET) )
          return 1;
       else
          G.PRT_ESC_OFFSET++;
/* 301290 Removed typecast                  */
   /* Temp  = *((unsigned int *) G.COUNT);
   */
    Temp  = G.COUNT[1]*256 + G.COUNT[0];

/******************* Quiet-Writer Space Forward/Backword ****************/

    if (G.DEF_ATTR==0) {

        if (G.CurrPos!=0) {
             if (G.ORIENTATION == 1)
             {
               G.INSERT_SPACES_START = bidi_out_buff_len;   /* 4/8/1993 TASHKEEL */
               InsertSpaces();
               G.INSERT_SPACES_END   = bidi_out_buff_len;   /* 4/8/1993 TASHKEEL */
 	     }
             NewLine();
             G.INSERT_SPACES_START = G.INSERT_SPACES_END = 0;  /* 4/8/1 993 TASHKEEL */
             if (G.ORIENTATION==1)
                AccProcessChar( (WCHAR)  13L);
        }

      Apels = (unsigned long)Temp*(unsigned long)CFG->PRT_SPACE_FOR[0];

      switch ((int)G.ESC_SubCode) {
        case 'd': if ( ((unsigned long)G.LINE_LENGTH+Apels) > ((unsigned long)G.PRT_LINE_LENGTH) ){
                      Temp = (long) (G.PRT_LINE_LENGTH-G.LINE_LENGTH)
                            /  CFG->PRT_SPACE_FOR[0];
                      G.LINE_LENGTH=G.PRT_LINE_LENGTH;
                  }
                  else
                     G.LINE_LENGTH  += (int)Apels;
                  break;

        case 'e':if ( ((unsigned long)G.LINE_LENGTH-Apels) < (unsigned long)0 ){
                      G.LINE_LENGTH=0;
                  }
                  else
                     G.LINE_LENGTH  -= (int)Apels;
                  break;
      } /* endswitch */

        if (G.ORIENTATION == 1)
           InsertSpaces();


      if( (G.PRESENTION_STATUS & 0X04000000) != 0) /* activate the underscore  */
      {
         AccProcessChar( (WCHAR)  _ESCAPE);
         AccProcessChar( (WCHAR)  '-');
         AccProcessChar( (WCHAR)  1);
      }


        WriteESC;
        WriteCHAR((long)Temp % 0x100 );
        WriteCHAR((long)Temp / 0x100 );

        if (G.ORIENTATION==1)
           AccProcessChar( (WCHAR)  13L);
        if( (G.PRESENTION_STATUS & 0X04000000) != 0) /* end the underscore  */
        {
           AccProcessChar( (WCHAR)  _ESCAPE);
           AccProcessChar( (WCHAR)  '-');
           AccProcessChar( (WCHAR)  0);
        }

    } /* endif */

    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _GRAPHICS_STAR                                           */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is used for printers that support         */
/*                    graphics (for dot matrix type printers). When the       */
/*                    GRAPHICS ESCape sequence is detected, the count of      */
/*                    binary information following is sent directly to the    */
/*                    printer.  If graphics are printed when the driver is    */
/*                    in right-to-left then no text should be printed on      */
/*                    the same line. ( or the text will not be properly       */
/*                    right justified.                                        */
/*                                                                            */
/*  FUNCTION: When the function is called, one more character is read.        */
/*            then the count of binary data following n1,n2. The count of     */
/*            data to skip IGNORE_COUNT is updated to n2*25+n1.               */
/*                                                                            */
/*  ENTRY POINT: (*Functions[GRAPHICS_STAR])                                  */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*        i                WORD        -Loop counter                          */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES:                                                              */
/*       NextChar                                                             */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _GRAPHICS_STAR       (void)
{ int i;

    while  (G.PRT_ESC_OFFSET<3)
       if ( NextChar(G.COUNT+G.PRT_ESC_OFFSET) )
          return 1;
       else
          G.PRT_ESC_OFFSET++;

    WriteESC;
    for (i=0;i<3;i++)
        WriteCHAR((long)G.COUNT[i]);

/* 301290 Removed typecast                               */
   /*G.IGNORE_COUNT = *((unsigned int *) (G.COUNT+1));
   */
    G.IGNORE_COUNT = G.COUNT[2]*256 + G.COUNT[1];
    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: _PMP                                                    */
/*                                                                            */
/*   DESCRIPTIVE NAME: This function is used for 3812 PagePrinter.            */
/*                     The sequence is passed as received.                    */
/*                                                                            */
/*   FUNCTION: When the function is called, it calls the STAR graphics        */
/*             function.                                                      */
/*                                                                            */
/*   ENTRY POINT: (*Functions[_PMP])                                          */
/*       LINKAGE: CALL   (from Processor)                                     */
/*                                                                            */
/*   INPUT: NONE                                                              */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION: NONE                                           */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS: NONE                                         */
/*                                                                            */
/*      ROUTINES:                                                             */
/*           _GRAPHICS_STAR                                                   */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES: NONE                                                        */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _PMP                 (void)
{
  return _GRAPHICS_STAR();
}



/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: _DOWNLOAD_EPSON                                         */
/*                                                                            */
/*   DESCRIPTIVE NAME: This function is used for EPSON printers that support  */
/*                     download (for dot matrix type printers). When the      */
/*                     DOWNLOAD ESCape sequence is detected, the count of     */
/*                     binary information following is sent directly to the   */
/*                     printer.  If graphics are printed when the driver is   */
/*                     in right-to-left then no text should be printed on     */
/*                     the same line.                                         */
/*                                                                            */
/*   FUNCTION: The counter of binary information (IGNORE_COUNT) is set to the */
/*             value following the sequence. All binary information           */
/*             following is sent to the printer directly.                     */
/*                                                                            */
/*   ENTRY POINT: (*Functions[_DOWNLOAD_EPSON])                               */
/*       LINKAGE: CALL   (from Processor)                                     */
/*                                                                            */
/*   INPUT: NONE                                                              */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION:                                                */
/*            i                WORD     -Loop counter                         */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*      ROUTINES:                                                             */
/*        NextChar                                                            */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES: NONE                                                        */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _DOWNLOAD_EPSON      (void)
{ int i;

    while  (G.PRT_ESC_OFFSET<3)
       if ( NextChar(G.COUNT+G.PRT_ESC_OFFSET) )
          return 1;
       else
          G.PRT_ESC_OFFSET++;

    WriteESC;
    for (i=0;i<3;i++)
      WriteCHAR((long)G.COUNT[i]);

    G.IGNORE_COUNT = (G.COUNT[2] - G.COUNT[1] + 1) * 12;
    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: _DOWNLOAD_PROPRINT                                      */
/*                                                                            */
/*   DESCRIPTIVE NAME: This function is used for proprinters that support     */
/*                     download (for dot matrix type printers).     The       */
/*                     DOWNLOAD ESCape sequence is treated exactly like       */
/*                     graphics, so a call to graphics function is made.      */
/*                                                                            */
/*   FUNCTION: When the function is called, it calls the graphics function.   */
/*                                                                            */
/*   ENTRY POINT: (*Functions[_DOWNLOAD_PROPRINT])                            */
/*       LINKAGE: CALL   (from Processor)                                     */
/*                                                                            */
/*   INPUT: NONE                                                              */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION: NONE                                           */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*      ROUTINES:                                                             */
/*         NextChar                                                           */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:  NONE                                                      */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _DOWNLOAD_PROPRINT (void)
{ int i;

    while  (G.PRT_ESC_OFFSET<2)
       if ( NextChar(G.COUNT+G.PRT_ESC_OFFSET) )
          return 1;
       else
          G.PRT_ESC_OFFSET++;

    WriteESC;
    for (i=0;i<2;i++)
      WriteCHAR((long)G.COUNT[i]);
/* 301290 Removed Typecast                          */
   /*G.IGNORE_COUNT = *((unsigned int *) G.COUNT);
   */
    G.IGNORE_COUNT = G.COUNT[1]*256 + G.COUNT[0];
    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: _RESTORE_JMP_2                                          */
/*                                                                            */
/*   DESCRIPTIVE NAME: This function is used for escape sequences that        */
/*                     do NOT affect bidiprocessing and are 2 bytes long.     */
/*                                                                            */
/*   FUNCTION: When the function is called, it performs no function BUT       */
/*             passing the escape and subcode to the output buffer.           */
/*                                                                            */
/*   ENTRY POINT: (*Functions[_RESTORE_JMP_2])                                */
/*       LINKAGE: CALL   (from Processor)                                     */
/*                                                                            */
/*   INPUT: NONE                                                              */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION: NONE                                           */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*         G               OTHER       -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*      ROUTINES: NONE                                                        */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES: NONE                                                        */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _RESTORE_JMP_2     (void)
{
   WriteESC;
   return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _PRINTER_RESET                                           */
/*                                                                            */
/*  DESCRIPTIVE NAME: Reset printer to default mode.                          */
/*                                                                            */
/*  FUNCTION: This function is called whenever the reset printing mode        */
/*            escape sequence is received.  The function processes the        */
/*            current line.then changes the printing mode to the start-up     */
/*            (default) settings.  The escape sequence is passed to the       */
/*            output buffer(printer).                                         */
/*                                                                            */
/*  ENTRY POINT: (*Functions[_PRINTER_RESET])                                 */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:  NONE                                           */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*         G                 OTHER     -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*        Char_Attr          WORD      -Encoded ESC character attribute.      */
/*                                                                            */
/*        PRT_OLD_CWIDTH     WORD      -default character width in APELS      */
/*                                                                            */
/*        PRT_DEF_TABS       OTHER     -default tabs.                         */
/*                                                                            */
/*        PRT_LAST           WORD      -characters/line in default pitch.     */
/*                                                                            */
/*        PRT_CHAR2_ATTR1    WORD      - Mask used in conjunction with        */
/*                                       PRESENTION_STATUS to test for        */
/*                                       character set 2.                     */
/*     ROUTINES:                                                              */
/*        PutByte                                                             */
/*        InsertSpaces                                                        */
/*        NewLine                                                             */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES:                                                              */
/*         MemMove                                                            */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _PRINTER_RESET       (void)
{ int i;

      if (G.ORIENTATION == 1)
      {
        G.INSERT_SPACES_START = bidi_out_buff_len;   /* 4/8/1993 TASHKEEL */
        InsertSpaces();
        G.INSERT_SPACES_END   = bidi_out_buff_len;   /* 4/8/1993 TASHKEEL */
      }

      NewLine();
      G.INSERT_SPACES_START = G.INSERT_SPACES_END = G.LINE_LENGTH = 0;  /* 4/8/1993 TASHKEEL */
      /***********************************************************/
      /* the line length (G.LINE_LENGTH) is initialised to zero to */
      /* indicate that the next character is to be printed at the*/
      /* the beggining of the line.                              */
      /***********************************************************/
      WriteESC;
      G.PRESENTION_STATUS = 0x00000000;
      PutByte((char *)&G.PRESENTION_STATUS, 3L, (long) (CFG->PRT_CHAR2_ATTR1&0xFF) );
      G.PRT_LINE_LENGTH = CFG->PRT_DFLT_LENGTH;
      G.CWIDTH = CFG->PRT_OLD_CWIDTH;
      G.PRT_L_MARGIN = 1;
      G.PRT_R_MARGIN = (unsigned char) CFG->PRT_LAST;
      G.PRT_CURR_TAB = 0;
      G.END_LINE_FLAG = 1;
      G.PREV_CHAR_ATTR = 0;
      G.PRT_PSM_W[0] = 0;
      G.PRT_PSM_W[1] = 0;
      G.PRT_PSM_W[2] = 0;

      for (i = 0 ;i<28 ;i++ )
         G.PRT_TABS[i] = CFG->PRT_DEF_TABS[i];

      return 0;
}
/* the following set of fucntions have been added to support the esc [ multi  */
/* functions ... date 12- 3- 1991                                             */

/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _HANDLE_ESC_BRKT                                         */
/*                                                                            */
/*  DESCRIPTIVE NAME:this is to handle all the different cases of the ESC [   */
/*                                                                            */
/*  FUNCTION: This function reads the character after the [ and determines    */
/*            which function to call accordingly, by setting the new G.       */
/*            parameter G.BRACKET_FUNCTIONS                                   */
/*                                                                            */
/*  ENTRY POINT: (*Functions[_HANDLE_ESC_BRKT                                 */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*        c              CHAR          - Temporary variable to store          */
/*                                       the next character.                  */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*        G              OTHER         -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES:                                                              */
/*        NextChar                                                            */
/*        WriteESC                                                            */
/*        _DW_DH                                                              */
/*        _PASSTROUGH_IN_BRACKET                                              */
/*        _SELECT_FONT                                                        */
/*        _SLECT_CURSOR_POSITION                                              */
/*        _SAVE_RESTORE_CURSOR_POSITION                                       */
/*        _SET_VERT_HORIZ_MARGIN                                              */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/*                                                                            */
/************************** END OF SPECIFICATIONS *****************************/

unsigned char  _HANDLE_ESC_BRKT (void)
{
   unsigned char c = 0;

    switch (G.PRT_ESC_OFFSET) {
     case 0:
           G.BRACKET_FUNCTIONS= 0;
           if (NextChar(&c))  /* read the character following the '[' i.e '@'*/
              return 1;
           switch (c) {
             case '@':
                G.BRACKET_FUNCTIONS = DWDH;   /* double hieght double width  */
                G.DOUBLE_HEIGHT_ESC [2] = c;

             break;

             case 'K':
             case 'h':
             case 'i':
             case 'm':
             case '\\':
             case 'T':
             case 'F':
             case 'l':
                G.BRACKET_FUNCTIONS = PTU;  /* all these escapes
                                               does not affect the bibi */
             break;

             case 'I':
                G.BRACKET_FUNCTIONS = SFG;  /* select font global */
		G.ESC_BRKT_I [2] = c;
             break;

             case 'Q':
               G.BRACKET_FUNCTIONS = SCP;   /* set cursor position */
             break;

             case 'j':
                G.BRACKET_FUNCTIONS = RSCP;  /* save and restore
                                                cursor position */
             break;

             case 'S':
                G.BRACKET_FUNCTIONS = SVHM;  /* set vertical and
                                                horizintal margins */
             break;

             } /* end switch on the character */

        break;
        }  /* end switch on the PRT_ESC_OFFSET   */

/* here we are not going to increment PRT_ESC_OFFSET so that later when the   */
/* -later- the corresponding function is called it enters with offset 0 and   */
/* from there on it increments the offset                                     */

        switch (G.BRACKET_FUNCTIONS){  /*here we are calling the corresponding function*/

           case DWDH:
              _DW_DH(c);
           break;

           case PTU:
              _PASSTHROUGH_IN_BRACKET(c);
           break;

           case SFG:
              _SELECT_FONT(c);
           break;

           case SCP:
              _SELECT_CURSOR_POSITION(c);
           break;

           case RSCP:
              _SAVE_RESTORE_CURSOR_POS(c);
           break;

           case SVHM:
              _SET_VERT_HORIZ_MARGIN(c);
           break;

       } /* end switch on fucntions */

} /* end */


/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _DW_DH                                                   */
/*                                                                            */
/*  DESCRIPTIVE NAME: Double Width Double Height                              */
/*                                                                            */
/*  FUNCTION: This function is called whenever the reset printing mode        */
/*                                                                            */
/*  ENTRY POINT: (*Functions[_DW_DH])                                         */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT:                                                                    */
/*      c       CHAR            -character to output after escape             */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*        TEMP_PS        DWORD         - Temporary variable to store          */
/*                                       the PS.                              */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*        G              OTHER         -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES:                                                              */
/*        InsertSpaces                                                        */
/*        NewLine                                                             */
/*        NextCHAR                                                            */
/*        AccProcessChar                                                      */
/*        WriteESC                                                            */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/*                                                                            */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _DW_DH  (unsigned char c)
{
   unsigned long TEMP_PS;

    switch (G.PRT_ESC_OFFSET) {
     case 0:
              if (!G.DEF_ATTR) {
              /*     if (G.ORIENTATION==1)
                       InsertSpaces();
                   NewLine();
                   if (G.ORIENTATION==1)
                       AccProcessChar( (WCHAR)  13L);
                   WriteESC;
                   WriteCHAR( (long) c);*/
                   G.PRT_ESC_OFFSET++;

             }/* endif default attributes */

     case 1:    /* reading n1 which is the length of ESC */
     case 2:    /* reading n2 which is the length of ESC */
           while (G.PRT_ESC_OFFSET<3) {
              if (NextChar(G.COUNT+G.PRT_ESC_OFFSET))
                   return 1;
              else  {
                  G.DOUBLE_HEIGHT_ESC [2+G.PRT_ESC_OFFSET] =
                                        G.COUNT[G.PRT_ESC_OFFSET];
                /*  WriteCHAR( (long) G.COUNT[G.PRT_ESC_OFFSET]);*/
                  G.PRT_ESC_OFFSET++;
               }
           } /* endwhile */
           G.COUNT_DH = G.COUNT[2]*256 + G.COUNT[1];   /* n2*256+n1 */

     case 3:  /* reading m1 if the length determined G.COUNT_DH allows */
     case 4:  /* reading m2 if the length determined G.COUNT_DH allows */
     case 5:  /* reading m3 if the length determined G.COUNT_DH allows */
     case 6:  /* reading m4 if the length determined G.COUNT_DH allows */
           while ((G.COUNT_DH > 0) && (G.PRT_ESC_OFFSET<7)) {
                 if (NextChar(G.COUNT+G.PRT_ESC_OFFSET-3))
                       return 1;
                 else  {
                  G.DOUBLE_HEIGHT_ESC [2+G.PRT_ESC_OFFSET] =
                                                G.COUNT[G.PRT_ESC_OFFSET-3];
                   /*  WriteCHAR((long) G.COUNT[G.PRT_ESC_OFFSET-3]);*/
                     G.COUNT_DH--;
                     G.PRT_ESC_OFFSET++;
                  }
             }

           if (G.PRT_ESC_OFFSET>5)
              switch (G.COUNT[2]) {
                case 2:
                case 18:
                case 34:
                   G.DOUBLE_HEIGHT_FLAG=1;
			G.PRESENTION_STATUS |= 0x00004000;   /* irene */
                break;

                case 1:
                case 17:
                case 33:
if (G.DOUBLE_HEIGHT_FLAG)
			G.PRESENTION_STATUS |= 0x00004000;  /* irene */
                   G.DOUBLE_HEIGHT_FLAG=0;
                   if ( (G.PRESENTION_STATUS & 0X02000000) != 0){
                      if ((G.PRESENTION_STATUS & 0X00C00000)!=0)
                        /* if double width ( temporary or continous ) is on. */
                         G.CWIDTH = CFG->PRT_SPEC_PITCH*2;
                      else
                         G.CWIDTH = CFG->PRT_SPEC_PITCH;
                   }
                  break;
                /*********************************************************/
                /* Here we set/reset the DOUBLE_HEIGHT_FLAG according to */
                /* the value of the parameter m3                         */
                /*********************************************************/
              } /* endswitch */


           if (G.PRT_ESC_OFFSET<7)
               return 0;

           TEMP_PS = G.PRESENTION_STATUS & 0X00800000;
             /* checking if the continious double width is already     */
             /* active or not                                          */

           if ( (G.COUNT[3]==1) && (TEMP_PS != 0) ){
             /* case of turning off double width which was active  */
             G.PRESENTION_STATUS &= 0XFF7FFFFF;
             G.PREV_CHAR_ATTR    &= 0XFF7FFFFF;
             G.CWIDTH = G.CWIDTH /2;
           }
           else if ( (G.COUNT[3]==2) && (TEMP_PS == 0) ){
             /* case of turning on double width which was not active  */
             G.PRESENTION_STATUS |= 0X00800000;
             G.PREV_CHAR_ATTR    |= 0X00800000;
             G.CWIDTH = G.CWIDTH * 2;
           }

           G.IGNORE_COUNT = G.COUNT_DH;
    } /* endswitch */
    return 0;
} /* end function  */


/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _PASSTHROUGH_IN_BRACKET                                  */
/*                                                                            */
/*  DESCRIPTIVE NAME: to handle ESC [ K, ESC [ h, ESC [ i, ESC [ m, ESC [ \   */
/*                              ESC [ T, ESC [ F and ESC [ l                  */
/*                                                                            */
/*  FUNCTION: This function is called when the above ESC is detected          */
/*                                                                            */
/*  ENTRY POINT: (*Functions [_PASSTHROUGH_IN_BRACKET])                       */
/*      LINKAGE: CALL   (from _HANDLE_ESC_[)                                  */
/*                                                                            */
/*  INPUT:                                                                    */
/*      c       CHAR            -character to output after escape             */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*        G              OTHER         -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES:                                                              */
/*        NextCHAR                                                            */
/*        WriteESC                                                            */
/*        WriteChar                                                           */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/*                                                                            */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _PASSTHROUGH_IN_BRACKET (unsigned char c)
{
    switch (G.PRT_ESC_OFFSET) {
     case 0:
              if (!G.DEF_ATTR) {
                   WriteESC;
                   WriteCHAR( (long) c);
                   G.PRT_ESC_OFFSET++;
             }/* endif default attributes */

     case 1:    /* reading n1 which is the length of ESC */
     case 2:    /* reading n2 which is the length of ESC */
           while (G.PRT_ESC_OFFSET<3) {
              if (NextChar(G.COUNT+G.PRT_ESC_OFFSET))
                   return 1;
              else  {
                  WriteCHAR( (long) G.COUNT[G.PRT_ESC_OFFSET]);
                  G.PRT_ESC_OFFSET++;
               }
           } /* endwhile */

           G.COUNT_DH = G.COUNT[2]*256 + G.COUNT[1];   /* n2*256+n1 */
           G.IGNORE_COUNT= G.COUNT_DH;   /* n2*256+n1 */
     break;
    } /* endswitch */
    return 0;
} /* end function  */






/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _SELECT_FONT                                             */
/*                                                                            */
/*  DESCRIPTIVE NAME: to handle ESC [ I                                       */
/*                                                                            */
/*  FUNCTION: This function is called when the above ESC is detected          */
/*                                                                            */
/*  ENTRY POINT: (*Functions [_SELECT_FONT])                                  */
/*      LINKAGE: CALL   (from _HANDLE_ESC_[)                                  */
/*                                                                            */
/*  INPUT:                                                                    */
/*      c       CHAR            -character to output after escape             */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*        G              OTHER         -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES:                                                              */
/*        WriteESC                                                            */
/*        WriteChar                                                           */
/*        NextChar                                                            */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/*                                                                            */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _SELECT_FONT (unsigned char c)
{
unsigned long temp_buff;

    switch (G.PRT_ESC_OFFSET) {
     case 0:
              if (!G.DEF_ATTR) {
                   G.PRT_ESC_OFFSET++;
             }/* endif default attributes */

     case 1:    /* reading n1 which is the length of ESC */
     case 2:    /* reading n2 which is the length of ESC */
           while (G.PRT_ESC_OFFSET<3) {
              if (NextChar(G.COUNT+G.PRT_ESC_OFFSET))
                   return 1;
              else {
                  G.ESC_BRKT_I [G.PRT_ESC_OFFSET+2] = G.COUNT[G.PRT_ESC_OFFSET];
                  G.PRT_ESC_OFFSET++;
               }
           } /* endwhile */

           G.COUNT_DH=G.ESC_BRKT_I[3]+G.ESC_BRKT_I[4]*2;

     case 3:  /* reading the high byte of the fgid                      */
     case 4:  /* reading the low byte of the fgid                       */
     case 5:  /* reading the high byte of the size of the font          */
     case 6:  /* reading the low byte of the size of the font           */
     case 7:  /* reading the fa parameter                               */
           while ((G.COUNT_DH > 0) && (G.PRT_ESC_OFFSET<8)) {
                 if (NextChar(G.ESC_BRKT_I+2+G.PRT_ESC_OFFSET))
                       return 1;
                 else  {
                     G.COUNT_DH--;
                     G.PRT_ESC_OFFSET++;
                  }
             }

           if (G.PRT_ESC_OFFSET==G.ESC_BRKT_I[3]+G.ESC_BRKT_I[4]*256)
           /* wait until the needed parameters are read */
             {

		G.PRESENTION_STATUS &= 0xFCFEFFFF;  /* end 17, 12 & psm */
                G.ESC_I_FLAG = 0;
               /* previously esc I selected font is ended */
           switch ( (G.ESC_BRKT_I[5]*256+G.ESC_BRKT_I[6]))
               /* switch on the fgid in the ESC  */
              {
                   /******************************************************/
                   /* in some  cases the pitch of the font is determined */
                   /* by the font global id, in others size and fa have  */
                   /* to be used.                                        */
                   /******************************************************/
                case COURIER_NASSEEM_BOLD_79:  /* case of 7.9 pitch     */
                case COURIER_NASSEEM_BOLD_79_IT:
                                        NewWidth (259);
                                        temp_buff = ((long)1 << 15);
                                        G.PRESENTION_STATUS |= temp_buff;
                break;

                case COURIER_NASSEEM_10:       /* case of 10 pitch     */
                case COURIER_NASSEEM_BOLD_10:
                case COURIER_NASSEEM_IT_10:
                case COURIER_NASSEEM_BOLD_IT_10:
                case COURIER_10:
                case COURIER_10_BOLD:
		case COURIER_SHALOM_10:
		case COURIER_SHALOM_BOLD_10:
                                        NewWidth (CFG->PRT_OLD_CWIDTH);
                                        temp_buff = ~((long)1 << 24);
                                        G.PRESENTION_STATUS &= temp_buff;
                                        temp_buff = ((long)1 << 15);
                                        G.PRESENTION_STATUS |= temp_buff;
                break;

                case COURIER_NASSEEM_12:      /* case of 12 pitch     */
                case COURIER_NASSEEM_IT_12:
                case PRESTIGE_ELITE:
                case COURIER_12:
		case COURIER_SHALOM_BOLD_12:
                                        NewWidth (170);
                                   /*     temp_buff = ((long)1 << 24);
                                        G.PRESENTION_STATUS |= temp_buff;*/
                                        temp_buff = ((long)1 << 15);
                                        G.PRESENTION_STATUS |= temp_buff;
                break;

                case COURIER_NASSEEM_15:
		case COURIER_SHALOM_BOLD_15:
                                        NewWidth (136);
                                        temp_buff = ((long)1 << 15);
                                        G.PRESENTION_STATUS |= temp_buff;
                break;

                case COURIER_NASSEEM_17:
                case COURIER_17:
                                        NewWidth (119);
                                        /*temp_buff = ((long)1 << 25);
                                        G.PRESENTION_STATUS |= temp_buff;*/
                                        temp_buff = ((long)1 << 15);
                                        G.PRESENTION_STATUS |= temp_buff;
                break;

                case GOTHIC_NASSEEM_20:
		case LETTER_GOTHIC_AVIV_20:
                                        NewWidth (102);
                                        temp_buff = ((long)1 << 15);
                                        G.PRESENTION_STATUS |= temp_buff;
                break;

                case BOLDFACE_PS :
                                        NewWidth (204);
                                        temp_buff = ((long)1 << 16);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        temp_buff = ((long)1 << 15);
                                        G.PRESENTION_STATUS |= temp_buff;



                break;

                case BASKERVIL_NASSEEM :
                                        temp_buff = ((long)1 << 15);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        temp_buff = ((long)1 << 16);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        PsmTable = BASK_NASS_TYPO_12;
                break;

                case BASKERVIL_NASSEEM_IT :
                                        temp_buff = ((long)1 << 15);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        temp_buff = ((long)1 << 16);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        PsmTable = BASK_NASS_TYPO_IT_12;
                break;

					/* psm hebrew */
		case BOLDFACE_BARAK :    
    					NewWidth (204);
                                        temp_buff = ((long)1 << 15);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        temp_buff = ((long)1 << 16);
                                        G.PRESENTION_STATUS |= temp_buff;
					PsmTable = BOLDFACE_BARAK_12;
		break;

                /* typo fonts */
                case BASKERVIL_NASSEEM_2 :
                                        temp_buff = ((long)1 << 15);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        temp_buff = ((long)1 << 16);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        PsmTable = BASK_NASS_TYPO_12;
                break;

                case BASKERVIL_NASSEEM_BOLD_18 :
                                        temp_buff = ((long)1 << 15);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        temp_buff = ((long)1 << 16);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        PsmTable = BASK_NASS_BOLD_TYPO_18;

                break;

                case BASKERVIL_NASSEEM_BOLD_24 :
                                        temp_buff = ((long)1 << 15);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        temp_buff = ((long)1 << 16);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        PsmTable = BASK_NASS_BOLD_TYPO_24;
                break;

                case BASKERVIL_NASSEEM_IT_2 :
                                        temp_buff = ((long)1 << 15);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        temp_buff = ((long)1 << 16);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        PsmTable = BASK_NASS_TYPO_IT_12;
                break;

                case BASKERVIL_NASSEEM_BOLD_IT_12 :
                                        temp_buff = ((long)1 << 15);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        temp_buff = ((long)1 << 16);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        PsmTable = BASK_NASS_TYPO_B_IT_12;
                break;

                case BASKERVIL_NASSEEM_BOLD_IT_18 :
                                        temp_buff = ((long)1 << 15);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        temp_buff = ((long)1 << 16);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        PsmTable = BASK_NASS_TYPO_B_IT_18;
                break;

                case BASKERVIL_NASSEEM_BOLD_IT_24 :
                                        temp_buff = ((long)1 << 15);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        temp_buff = ((long)1 << 16);
                                        G.PRESENTION_STATUS |= temp_buff;
                                        PsmTable = BASK_NASS_TYPO_B_IT_24;
                break;

                /***********************************************************/
                /* the following are cases in which having the fgid is not */
                /* enough to determine the width of the font and hence the */
                /* fa and then the size parameter have to be quieried.     */
                /***********************************************************/
                case BASKERVIL_NASSEEM_BOLD_TYPO_PSM :
                                             /* typo 12, 18, 24 and psm */
                  temp_buff = ((long)1 << 15);
                  G.PRESENTION_STATUS |= temp_buff;
                  temp_buff = ((long)1 << 16);
                  G.PRESENTION_STATUS |= temp_buff;
                  switch (G.ESC_BRKT_I[9])
                   {
                    case 1 :
                    case 2 :
                                /* space width (in inches) * 1440 */
                             switch (G.ESC_BRKT_I[7]*256 + G.ESC_BRKT_I[8])
                               {
                                case 150 :  /* PSM */
                                           PsmTable = CFG->PSM_TABLE_1046;
                                break;
                                case 67  :  /* pt size = 12 */
                                           PsmTable = BASK_NASS_BOLD_TYPO_12;
                                break;
                                case 101 :  /* pt size = 18 */
                                           PsmTable = BASK_NASS_BOLD_TYPO_18;
                                break;
                                case 130 :  /* pt size = 24 */
                                           PsmTable = BASK_NASS_BOLD_TYPO_24;
                                break;
                                default :
                                break;
                               }
                    break;
                    case 3 :
                                /* point size * 20 */
                             switch (G.ESC_BRKT_I[7]*256 + G.ESC_BRKT_I[8])
                               {
                                case 240 :   /* pt size 12 */
                                           PsmTable = BASK_NASS_BOLD_TYPO_12;
                                break;
                                case 360 :
                                           PsmTable = BASK_NASS_BOLD_TYPO_18;
                                break;
                                case 480 :
                                           PsmTable = BASK_NASS_BOLD_TYPO_24;
                                break;
                                default :
                                break;
                               }
                    break;
                    case 4 :
                                /* point size * 20 / 3 */
                             switch (G.ESC_BRKT_I[7]*256 + G.ESC_BRKT_I[8])
                               {
                                case 80 :   /* pt size 12 */
                                           PsmTable = BASK_NASS_BOLD_TYPO_12;
                                break;
                                case 120 :
                                           PsmTable = BASK_NASS_BOLD_TYPO_18;
                                break;
                                case 160 :
                                           PsmTable = BASK_NASS_BOLD_TYPO_24;
                                break;
                                default :
                                break;
                               }
                    break;
                 } /* end switch size */
                break;


                case BASKERVIL_BOLDFACE_NASSEEM_BOLD :
                                             /* psm and typo : 12 */
                  temp_buff = ((long)1 << 15);
                  G.PRESENTION_STATUS |= temp_buff;
                  temp_buff = ((long)1 << 16);
                  G.PRESENTION_STATUS |= temp_buff;
                  switch (G.ESC_BRKT_I [9])
                   {
                    case 1 :
                    case 2 :
                             if (G.ESC_BRKT_I[7]*256 + G.ESC_BRKT_I[8] == 150)
                              /* psm */
                               PsmTable = CFG->PSM_TABLE_1046;
                    break;
                    case 3 :
                             if (G.ESC_BRKT_I[7]*256 + G.ESC_BRKT_I[8] == 240)
                              /* typo */
                               PsmTable = BASK_NASS_TYPO_12;
                    break;
                    case 4 :
                             if (G.ESC_BRKT_I[7]*256 + G.ESC_BRKT_I[8] == 80)
                              /* typo */
                               PsmTable = BASK_NASS_TYPO_12;
                    break;
                    default :
                    break;
                   }


                case BASKERVIL_NASSEEM_BOLD_IT :
                                             /* typo 12, 18, and 24 */
                  temp_buff = ((long)1 << 15);
                  G.PRESENTION_STATUS |= temp_buff;
                  temp_buff = ((long)1 << 16);
                  G.PRESENTION_STATUS |= temp_buff;
                  switch (G.ESC_BRKT_I[9])
                   {
                    case 1 :
                    case 2 :
                                /* space width (in inches) * 1440 */
                             switch (G.ESC_BRKT_I[7]*256 + G.ESC_BRKT_I[8])
                               {
                                case 67  :  /* pt size = 12 */
                                           PsmTable = BASK_NASS_TYPO_B_IT_12;
                                break;
                                case 101 :  /* pt size = 18 */
                                           PsmTable = BASK_NASS_TYPO_B_IT_18;
                                break;
                                case 130 :  /* pt size = 24 */
                                           PsmTable = BASK_NASS_TYPO_B_IT_24;
                                break;
                                default :
                                break;
                               }
                    break;
                    case 3 :
                             /* point size * 20  not done yet */
                             switch (G.ESC_BRKT_I[7]*256 + G.ESC_BRKT_I[8])
                               {
                                case 240 :   /* pt size 12 */
                                           PsmTable = BASK_NASS_TYPO_B_IT_12;
                                break;
                                case 360 :
                                           PsmTable = BASK_NASS_TYPO_B_IT_18;
                                break;
                                case 480 :
                                           PsmTable = BASK_NASS_TYPO_B_IT_24;
                                break;
                                default :
                                break;
                               }
                    break;
                    case 4 :
                                /* point size * 20 / 3 */
                             switch (G.ESC_BRKT_I[7]*256 + G.ESC_BRKT_I[8])
                               {
                                case 80 :   /* pt size 12 */
                                           PsmTable = BASK_NASS_TYPO_B_IT_12;
                                break;
                                case 120 :
                                           PsmTable = BASK_NASS_TYPO_B_IT_18;
                                break;
                                case 160 :
                                           PsmTable = BASK_NASS_TYPO_B_IT_24;
                                break;
                                default :
                                break;
                               }
                    break;
                 } /* end switch on fa */
                break;

                case TIMES_ROMAN_NARK :
                                             /* typo 8, 10 and 12 */
                  temp_buff = ((long)1 << 15);
                  G.PRESENTION_STATUS |= temp_buff;
                  temp_buff = ((long)1 << 16);
                  G.PRESENTION_STATUS |= temp_buff;
                  switch (G.ESC_BRKT_I[9])
                   {
		/*  case 1 : unknown reaction of fa = 1 */
                    case 2 :
                                /* space width (in inches) * 1440 */
                             switch (G.ESC_BRKT_I[7]*256 + G.ESC_BRKT_I[8])
                               {
                                case 38 :  /* pt size = 8 */
					/* unknown space width */
                                           PsmTable = TIMES_ROMAN_NARK_TYPO_8;
                                break;
                                case 48 : /* pt size = 10 */
					/* unknown space width */
                                           PsmTable = TIMES_ROMAN_NARK_TYPO_10;
                                break;
                                case 62 :  /* pt size = 12 */
                                           PsmTable = TIMES_ROMAN_NARK_TYPO_12;
                                break;
                                default :
                                break;
                               }
                    break;
                    case 3 :
                             /* point size * 20 */
                             switch (G.ESC_BRKT_I[7]*256 + G.ESC_BRKT_I[8])
                               {
                                case 160 :   /* pt size 8 */
                                           PsmTable = TIMES_ROMAN_NARK_TYPO_8;
                                break;
                                case 200 :   /*  pt size 10 */
                                           PsmTable = TIMES_ROMAN_NARK_TYPO_10;
                                break;
                                case 240 :   /* pt size 12 */
                                           PsmTable = TIMES_ROMAN_NARK_TYPO_12;
                                break;
                                default :
                                break;
                               }
                    break;
                    case 4 :
                                /* point size * 20 / 3 */
                             switch (G.ESC_BRKT_I[7]*256 + G.ESC_BRKT_I[8])
                               {
                                case 53 :  /* 53.33 ?? pt size 8 */
                                           PsmTable = TIMES_ROMAN_NARK_TYPO_8;
                                break;
                                case 66 :  /* 66.66 ?? pt size 10 */
                                           PsmTable = TIMES_ROMAN_NARK_TYPO_10;
                                break;
                                case 80 :   /* pt size 12 */
                                           PsmTable = TIMES_ROMAN_NARK_TYPO_12;
                                break;
                                default :
                                break;
                               }
                    break;
                 } /* end switch on fa */
                break;

		case TIMES_ROMAN_NARK_BOLD :
                                             /* typo 8, 10, 12, 18, and 24 */
                  temp_buff = ((long)1 << 15);
                  G.PRESENTION_STATUS |= temp_buff;
                  temp_buff = ((long)1 << 16);
                  G.PRESENTION_STATUS |= temp_buff;
                  switch (G.ESC_BRKT_I[9])
                   {
		/*  case 1 : unknown reaction of fa = 1 */
                    case 2 :
                                /* space width (in inches) * 1440 */
                             switch (G.ESC_BRKT_I[7]*256 + G.ESC_BRKT_I[8])
                               {
                                case 38  :  /* pt size = 8 */
                                          PsmTable = TIMES_ROMAN_NARK_B_TYPO_8;
                                break;
                                case 48 :  /* pt size = 10 */
                                          PsmTable = TIMES_ROMAN_NARK_B_TYPO_10;
                                break;
                                case 62 :  /* pt size = 12 */
                                          PsmTable = TIMES_ROMAN_NARK_B_TYPO_12;
                                break;
                                case 91 :  /* pt size = 18 */
                                          PsmTable = TIMES_ROMAN_NARK_B_TYPO_18;
                                break;
                                case 120 :  /* pt size = 24 */
                                          PsmTable = TIMES_ROMAN_NARK_B_TYPO_24;
                                break;
                                default :
                                break;
                               }
                    break;
                    case 3 :
                             /* point size * 20 */
                             switch (G.ESC_BRKT_I[7]*256 + G.ESC_BRKT_I[8])
                               {
                                case 160 :   /* pt size 8 */
                                          PsmTable = TIMES_ROMAN_NARK_B_TYPO_8;
                                break;
                                case 200 :   /* pt size 10 */
                                          PsmTable = TIMES_ROMAN_NARK_B_TYPO_10;
                                break;
                                case 240 :   /* pt size 12 */
                                          PsmTable = TIMES_ROMAN_NARK_B_TYPO_12;
                                break;
                                case 360 :
                                          PsmTable = TIMES_ROMAN_NARK_B_TYPO_18;
                                break;
                                case 480 :
                                          PsmTable = TIMES_ROMAN_NARK_B_TYPO_24;
                                break;
                                default :
                                break;
                               }
                    break;
                    case 4 :
                                /* point size * 20 / 3 */
                             switch (G.ESC_BRKT_I[7]*256 + G.ESC_BRKT_I[8])
                               {
                                case 53 :   /* 53.33 ?? pt size 8 */
                                          PsmTable = TIMES_ROMAN_NARK_B_TYPO_8;
                                break;
                                case 66 :   /* 66.66 ?? pt size 10 */
                                          PsmTable = TIMES_ROMAN_NARK_B_TYPO_10;
                                break;
                                case 80 :   /* pt size 12 */
                                          PsmTable = TIMES_ROMAN_NARK_B_TYPO_12;
                                break;
                                case 120 :
                                          PsmTable = TIMES_ROMAN_NARK_B_TYPO_18;
                                break;
                                case 160 :
                                          PsmTable = TIMES_ROMAN_NARK_B_TYPO_24;
                                break;
                                default :
                                break;
                               }
                    break;
                 } /* end switch on fa */
                break;

                default:  /* the case of unrecoginized fgid */
                          /* meanning that the font selected was not */
                          /* recognized, thus, as if nothing was detected*/
                break;
              } /* endswitch on the fgid */
	}

     case 8:  /* reading the st parameter                               */
     case 9:  /* reading the high byte of the code page parameter       */
     case 10: /* reading the low byte of the code page parameter        */

           while (G.COUNT_DH>0) {
              if (NextChar(G.ESC_BRKT_I+2+G.PRT_ESC_OFFSET))
                   return 1;
              else {
                  G.PRT_ESC_OFFSET++;
                  G.COUNT_DH--;
               }
           } /* endwhile */
     break;
        G.IGNORE_COUNT=G.COUNT_DH; /* to ignore further bytes in the esc  */
                                   /* if any                              */
    } /* endswitch */
  return 0;
} /* end function  */


/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _SELECT_CURSOR_POSITION                                  */
/*                                                                            */
/*  DESCRIPTIVE NAME: to handle ESC [ Q                                       */
/*                                                                            */
/*  FUNCTION: This function is called when the above ESC is detected          */
/*                                                                            */
/*  ENTRY POINT: Function (*_SELECT_CURSOR_POSITION)                          */
/*      LINKAGE: CALL   (from _HANDLE_ESC_[)                                  */
/*                                                                            */
/*  INPUT:                                                                    */
/*      c       CHAR            -character to output after escape             */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*        G              OTHER         -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES:                                                              */
/*        WriteESC                                                            */
/*        WriteCHAR                                                           */
/*        NextChar                                                            */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/*                                                                            */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _SELECT_CURSOR_POSITION (unsigned char c)
{
    switch (G.PRT_ESC_OFFSET) {
     case 0:
              if (!G.DEF_ATTR) {
                   WriteESC;
                   WriteCHAR( (long) c);
                   G.PRT_ESC_OFFSET++;
             }/* endif default attributes */

     case 1:    /* reading n1 which is the length of ESC */
     case 2:    /* reading n2 which is the length of ESC */
           while (G.PRT_ESC_OFFSET<3) {
              if (NextChar(G.COUNT+G.PRT_ESC_OFFSET))
                   return 1;
              else  {
                  WriteCHAR( (long) G.COUNT[G.PRT_ESC_OFFSET]);
                  G.PRT_ESC_OFFSET++;
               }
           } /* endwhile */

           G.COUNT_DH = G.COUNT[2]*256 + G.COUNT[1];   /* n2*256+n1 */
           G.IGNORE_COUNT= G.COUNT_DH;   /* n2*256+n1 */
     break;
    } /* endswitch */
    return 0;
} /* end function  */

/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _SAVE_RESTORE_CURSOR_POS                                 */
/*                                                                            */
/*  DESCRIPTIVE NAME: to handle ESC [ j                                       */
/*                                                                            */
/*  FUNCTION: This function is called when the above ESC is detected          */
/*                                                                            */
/*  ENTRY POINT: (*Functions [_SAVE_RESTORE_CURSOR_POS])                      */
/*      LINKAGE: CALL   (from _HANDLE_ESC_[)                                  */
/*                                                                            */
/*  INPUT:                                                                    */
/*      c       CHAR            -character to output after escape             */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*        G              OTHER         -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES:                                                              */
/*        WriteESC                                                            */
/*        WriteCHAR                                                          */
/*        NextChar                                                            */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/*                                                                            */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _SAVE_RESTORE_CURSOR_POS (unsigned char c)
{
    switch (G.PRT_ESC_OFFSET) {
     case 0:
              if (!G.DEF_ATTR) {
                   WriteESC;
                   WriteCHAR( (long) c);
                   G.PRT_ESC_OFFSET++;
             }/* endif default attributes */

     case 1:    /* reading n1 which is the length of ESC */
     case 2:    /* reading n2 which is the length of ESC */
           while (G.PRT_ESC_OFFSET<3) {
              if (NextChar(G.COUNT+G.PRT_ESC_OFFSET))
                   return 1;
              else  {
                  WriteCHAR( (long) G.COUNT[G.PRT_ESC_OFFSET]);
                  G.PRT_ESC_OFFSET++;
               }
           } /* endwhile */

           G.COUNT_DH = G.COUNT[2]*256 + G.COUNT[1];   /* n2*256+n1 */
           G.IGNORE_COUNT= G.COUNT_DH;   /* n2*256+n1 */
     break;
    } /* endswitch */
    return 0;
} /* end function  */

/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _SET_VERT_HORIZ_MARGIN                                   */
/*                                                                            */
/*  DESCRIPTIVE NAME: to handle ESC [ S                                       */
/*                                                                            */
/*  FUNCTION: This function is called when the above ESC is detected          */
/*                                                                            */
/*  ENTRY POINT: (*Functions _SET_VERT_HORIZ_MARGIN)                          */
/*      LINKAGE: CALL   (from _HANDLE_ESC_[)                                  */
/*                                                                            */
/*  INPUT:                                                                    */
/*      c       CHAR            -character to output after escape             */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*        G              OTHER         -Structure EntryPoint which contains   */
/*                                      the processing context of the current */
/*                                      input buffer.                         */
/*                                                                            */
/*     ROUTINES:                                                              */
/*        WriteESC                                                            */
/*        WriteCHAR                                                           */
/*        NextChar                                                            */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/*                                                                            */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _SET_VERT_HORIZ_MARGIN (unsigned char c)
{
    switch (G.PRT_ESC_OFFSET) {
     case 0:
              if (!G.DEF_ATTR) {
                   WriteESC;
                   WriteCHAR( (long) c);
                   G.PRT_ESC_OFFSET++;
             }/* endif default attributes */

     case 1:    /* reading n1 which is the length of ESC */
     case 2:    /* reading n2 which is the length of ESC */
           while (G.PRT_ESC_OFFSET<3) {
              if (NextChar(G.COUNT+G.PRT_ESC_OFFSET))
                   return 1;
              else  {
                  WriteCHAR( (long) G.COUNT[G.PRT_ESC_OFFSET]);
                  G.PRT_ESC_OFFSET++;
               }
           } /* endwhile */

           G.COUNT_DH = G.COUNT[2]*256 + G.COUNT[1];   /* n2*256+n1 */
           G.IGNORE_COUNT= G.COUNT_DH;   /* n2*256+n1 */
     break;
    } /* endswitch */
    return 0;
} /* end function  */


/*///////////////////////////////////////////////////////////////////////////*/
/*//////////////////////////// CONTROL-CODES ////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////*/



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _PASS_THROUGH                                            */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is used for single byte codes which       */
/*                    do not perform any special features on the printer.     */
/*                                                                            */
/*  FUNCTION: whenever a code of the above type is detected, the function     */
/*            is called and the code is written to the output buffer.         */
/*                                                                            */
/*  ENTRY POINT: (*Functions[PASS_THROUGH])                                   */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*        character      DWORD     -this is the Control Code to be written to */
/*                                  the output buffer.                        */
/*                                                                            */
/*     ROUTINES: NONE                                                         */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char _PASS_THROUGH(void)
{
  WriteCHAR( (long)  character);
  return 0;
 }



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _PRINT_SERVICE_1                                         */
/*                                                                            */
/*  DESCRIPTIVE NAME: This Function will pass the corresponding single        */
/*                    byte printer code directly to the printer when          */
/*                    received.                                               */
/*                                                                            */
/*  FUNCTION: whenever a code of the above type is detected, the function     */
/*            is called and the code is sent directly to the printer.         */
/*            (i.e. it is written to the output buffer).                      */
/*                                                                            */
/*  ENTRY POINT: (*Functions[PRINT_SERVICE_1])                                */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS: NONE                                          */
/*        character      DWORD     -this is the Control Code to be written to */
/*                                  the output buffer.                        */
/*                                                                            */
/*     ROUTINES: NONE                                                         */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _PRINT_SERVICE_1(void)
{
  WriteCHAR( (long)  character);
  return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _SPECIAL_ONE                                             */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is used for single byte codes that are    */
/*                    printable under CHARACTER SET 2, and are treated as     */
/*                    control codes in CHARACTER SET 1.                       */
/*                                                                            */
/*  FUNCTION: whenever a code of the above type is detected, the function     */
/*            is called. A test is made on bit 31 of the presentation         */
/*            status (PRESENTION_STATUS) defining the character set. If       */
/*            this bit is set to ZERO then we are printing using character    */
/*            SET 1 and the codes are consumed Else the codes are treated as  */
/*            printable chracters and are written to the line buffer          */
/*            (LineBuff).                                                     */
/*                                                                            */
/*  ENTRY POINT: (*Functions[SPECIAL_ONE])                                    */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*        character      DWORD     -this is the Control Code to be processed  */
/*                                                                            */
/*        G              OTHER     -Structure EntryPoint which contains       */
/*                                  the processing context of the current     */
/*                                  input buffer.                             */
/*                                                                            */
/*        PRT_CHAR2_ATTR1    WORD      - Mask used in conjunction with        */
/*                                       PRESENTION_STATUS to test for        */
/*                                       character set 2.                     */
/*                                                                            */
/*     ROUTINES:                                                              */
/*       WriteToBuffer             -used to write the control code to the     */
/*                                  line buffer (LineBuff).                   */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _SPECIAL_ONE (void)
{
    if ((!G.DEF_ATTR) && (((G.PRESENTION_STATUS >> 16) & CFG->PRT_CHAR2_ATTR1) != 0))
        WriteToBuffer((long)character, G.PRESENTION_STATUS);
    return 0;
 }



void PutEscapes ()
{
unsigned char i, size, l;
int index, totsize = 0;

     index = 0;
     if (((G.LineBuff [G.CurrPos-1].attr & 0x00008000) >> 15) == 1)
       totsize = 5+G.ESC_BRKT_I [3] + G.ESC_BRKT_I [4]*256;
     if (((G.LineBuff [G.CurrPos-1].attr & 0x00004000) >> 14) == 1)
       totsize += 5+G.DOUBLE_HEIGHT_ESC [3] + G.DOUBLE_HEIGHT_ESC [4]*256;
     if (G.ESC_I_FLAG == 1)
	totsize+=3;

     G.LineBuff [G.CurrPos-1].escsize = totsize;
     G.LineBuff [G.CurrPos-1].escapes = malloc (totsize);
     memset (G.LineBuff [G.CurrPos-1].escapes, 0, totsize);

    if (TASHKEEL)                       /* 10/8/1993 */
    {
      G.UpperVowelBuff[G.CurrPos-1].escsize = G.LowerVowelBuff[G.CurrPos-1].escsize = totsize;
      G.UpperVowelBuff[G.CurrPos-1].escapes = malloc(totsize);
      G.LowerVowelBuff[G.CurrPos-1].escapes = malloc(totsize);
      memset (G.UpperVowelBuff[G.CurrPos-1].escapes, 0, totsize);
      memset (G.LowerVowelBuff[G.CurrPos-1].escapes, 0, totsize);
    }

    /* if font is selected by ESC [ I */
    if (((G.LineBuff [G.CurrPos-1].attr & 0x00008000) >> 15) == 1)
      {
	size = G.ESC_BRKT_I [3] + G.ESC_BRKT_I [4]*256;
	G.LineBuff [G.CurrPos-1].escapes [index++] = G.ESC_BRKT_I [0]; /*esc*/
	G.LineBuff [G.CurrPos-1].escapes [index++] = G.ESC_BRKT_I [1]; /* [ */
	G.LineBuff [G.CurrPos-1].escapes [index++] = G.ESC_BRKT_I [2]; /* I */
	for (i=index; i<=size+index+2; i++)
	   G.LineBuff [G.CurrPos-1].escapes [i] = G.ESC_BRKT_I [i];
        index+=size+2;
      }

    /* double width double height by ESC [ @ */
    if (((G.LineBuff [G.CurrPos-1].attr & 0x00004000) >> 14) == 1)
      {
	size = G.DOUBLE_HEIGHT_ESC [3] + G.DOUBLE_HEIGHT_ESC [4]*256;
	G.LineBuff [G.CurrPos-1].escapes [index++] = G.DOUBLE_HEIGHT_ESC [0]; /* esc */
	G.LineBuff [G.CurrPos-1].escapes [index++] = G.DOUBLE_HEIGHT_ESC [1]; /* [ */ 
	G.LineBuff [G.CurrPos-1].escapes [index++] = G.DOUBLE_HEIGHT_ESC [2]; /* @ */

	for (l=3,i=index; i<=size+index+2; l++,i++)
	   G.LineBuff [G.CurrPos-1].escapes [i] = G.DOUBLE_HEIGHT_ESC [l];
        index+=size+2;
      }
	

    /* selected font by ESC I */
    if (G.ESC_I_FLAG == 1)
      {
	G.LineBuff [G.CurrPos-1].escapes [index++] = G.ESC_I_SEQ [0]; 
	G.LineBuff [G.CurrPos-1].escapes [index++] = G.ESC_I_SEQ [1]; 
	G.LineBuff [G.CurrPos-1].escapes [index++] = G.ESC_I_SEQ [2]; 
      }

    /* 10/8/1993 */
    if (TASHKEEL)
    {
      index = 0;

      if (((G.LineBuff [G.CurrPos-1].attr & 0x00008000) >> 15) == 1)
      {
        size = G.ESC_BRKT_I [3] + G.ESC_BRKT_I [4]*256;
        G.UpperVowelBuff [G.CurrPos-1].escapes [index] = G.LowerVowelBuff [G.CurrPos-1].escapes [index++] = G.ESC_BRKT_I [0];
        G.UpperVowelBuff [G.CurrPos-1].escapes [index] = G.LowerVowelBuff [G.CurrPos-1].escapes [index++] = G.ESC_BRKT_I [1];
        G.UpperVowelBuff [G.CurrPos-1].escapes [index] = G.LowerVowelBuff [G.CurrPos-1].escapes [index++] = G.ESC_BRKT_I [2];
        for (i=index; i<=size+index+2; i++)
           G.UpperVowelBuff [G.CurrPos-1].escapes [i] = G.LowerVowelBuff [G.CurrPos-1].escapes [i] = G.ESC_BRKT_I [i];
        index+=size+2;
      }

      if (((G.LineBuff [G.CurrPos-1].attr & 0x00004000) >> 14) == 1)
      {
        size = G.DOUBLE_HEIGHT_ESC [3] + G.DOUBLE_HEIGHT_ESC [4]*256;
        G.UpperVowelBuff [G.CurrPos-1].escapes [index] = G.LowerVowelBuff [G.CurrPos-1].escapes [index++] = G.DOUBLE_HEIGHT_ESC [0];
        G.UpperVowelBuff [G.CurrPos-1].escapes [index] = G.LowerVowelBuff [G.CurrPos-1].escapes [index++] = G.DOUBLE_HEIGHT_ESC [1];
        G.UpperVowelBuff [G.CurrPos-1].escapes [index] = G.LowerVowelBuff [G.CurrPos-1].escapes [index++] = G.DOUBLE_HEIGHT_ESC [2];

        for (l=3,i=index; i<=size+index+2; l++,i++)
           G.UpperVowelBuff [G.CurrPos-1].escapes [i] = G.LowerVowelBuff [G.CurrPos-1].escapes [i] = G.DOUBLE_HEIGHT_ESC [l];
        index+=size+2;
      }

      if (G.ESC_I_FLAG == 1)
      {
        G.UpperVowelBuff [G.CurrPos-1].escapes [index] = G.LowerVowelBuff [G.CurrPos-1].escapes [index++] = G.ESC_I_SEQ [0];
        G.UpperVowelBuff [G.CurrPos-1].escapes [index] = G.LowerVowelBuff [G.CurrPos-1].escapes [index++] = G.ESC_I_SEQ [1];
        G.UpperVowelBuff [G.CurrPos-1].escapes [index] = G.LowerVowelBuff [G.CurrPos-1].escapes [index++] = G.ESC_I_SEQ [2];
      }
    } /* IF TASHKEEL */
}


/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: _PRINTABLE                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME: This function is used for characters and printable     */
/*                     codes.                                                 */
/*                                                                            */
/*   FUNCTION: When the function is called, WriteToBuffer is called to        */
/*             write the characters to the line buffer (LineBuff).            */
/*             If the character to be written to the line buffer was a        */
/*             printable code (i.e.GRAPHICS_CHAR_COUNT <> 0) then             */
/*             GRAPHICS_CHAR_COUNT is decremented to indicate that a code     */
/*             has been printed as graphics. If GRAPHICS_CHAR_COUNT was       */
/*             equal to zero then the bit number zero in the presentation     */
/*             status (PRESENTION_STATUS) which indicates that we are         */
/*             printing codes as graphics is reset to zero to now indicate    */
/*             that we are no more printing codes as graphics.                */
/*                                                                            */
/*   ENTRY POINT: (*Functions[PRINTABLE])                                     */
/*       LINKAGE: CALL   (from Processor)                                     */
/*                                                                            */
/*   INPUT: NONE                                                              */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION: NONE                                           */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*        character      DWORD     -this is the Control Code to be processed  */
/*                                                                            */
/*        G              OTHER     -Structure EntryPoint which contains       */
/*                                  the processing context of the current     */
/*                                  input buffer.                             */
/*                                                                            */
/*        PSM_ATTR       DWORD     -PSM mask to test for PSM active           */
/*                                                                            */
/*      ROUTINES: NONE                                                        */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES:                                                             */
/*         PSM_WriteToBuffer                                                  */
/*         WriteToBuffer                                                      */
/*                                                                            */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _PRINTABLE (void)
{

    G.END_LINE_FLAG = 0;
      /***************************************************************/
      /* Resetting this flag causes no bidi sequences to be accepted */
      /* until the flag is set again (see _BIDI_SEQ_FOUND)           */
      /***************************************************************/


    if (!G.DEF_ATTR)  {
        G.PRT_PSM_W[0]  = G.CWIDTH;
        if ((G.PRESENTION_STATUS & PRT_PSM_ATTR) != 0)
        {
          if (TASHKEEL  &&  character > 127  &&  Vowels[character-128])
              WriteToBuffer (character,G.PRESENTION_STATUS); 
          else PSM_WriteToBuffer (character,G.PRESENTION_STATUS);
        }
        else
          WriteToBuffer (character,G.PRESENTION_STATUS);

     PutEscapes ();

     G.PRT_PSM_W[2] = G.PRT_PSM_W[1];
     G.PRT_PSM_W[1] = G.PRT_PSM_W[0];
    }

    if (G.GRAPHICS_CHAR_COUNT) {
          /**********************************************************/
          /* GRAPHICS_CHAR_COUNT represents the count of characters */
          /* to print from all-characters chart.                    */
          /**********************************************************/
        G.GRAPHICS_CHAR_COUNT--;
          /**********************************************************/
          /* A character was printed above. so count is decremented */
          /**********************************************************/
        if (!G.GRAPHICS_CHAR_COUNT)
           /****************************************************/
           /* if no more graphics characters are to be printed */
           /* the presentation status is updated to reset the  */
           /* all charts bit.                                  */
           /****************************************************/
            G.PRESENTION_STATUS &= 0xFFFFFFFE;
    }

    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: _ESC_FOUND                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME: This will cause the following character to be          */
/*                     treated as ESCape Sequences.                           */
/*                                                                            */
/*   FUNCTION: When the function is called a check is made for the special    */
/*             subcodes 27,43 which are handled differently than all the      */
/*             others. in the default case, the escape table is searched      */
/*             for the received subcode. if found START_FUNC and ESC_OFFSET   */
/*             are initialized to refelect the new functions parameters and   */
/*             PRT_ESC_OFFSET is initialized to start the new function fresh. */
/*             the function usually returns one ( to indicate that START_FUNC */
/*             contains   the   number  of  a function  that has not finished */
/*             processing and will be invoked after reading the next character*/
/*             in the Processor function. However the only case when ESC_FOUND*/
/*             returns 0 is when the function received a SubCode that is      */
/*             neither 27 nor 43 nor included in the table.                   */
/*                                                                            */
/*   ENTRY POINT: (*Functions[ESC_FOUND])                                     */
/*       LINKAGE: CALL   (from Processor)                                     */
/*                                                                            */
/*   INPUT: NONE                                                              */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION:                                                */
/*        k              CHAR      -Loop counter                              */
/*                                                                            */
/*        Seq_Found      CHAR      -Flag that indicates that the sequence     */
/*                                  was found in the ESC table (i.e it is     */
/*                                  a supported escape sequence.              */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*        G              OTHER     -Structure EntryPoint which contains       */
/*                                  the processing context of the current     */
/*                                  input buffer.                             */
/*                                                                            */
/*        counter        WORD      -counter indicating the number of chars    */
/*                                  were processed from the input buffer.     */
/*                                                                            */
/*        Char_Attr      CHAR      -the Encoded Character attribute           */
/*                                  that defines how the presentation         */
/*                                  status will be updated.                   */
/*                                                                            */
/*      ROUTINES:                                                             */
/*          NextChar                                                          */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES: NONE                                                        */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _ESC_FOUND (void)
{ unsigned char Seq_Found,k;
/*  #define ESC_TABLE_LEN 45 */
  unsigned char c;


    G.PRT_ESC_OFFSET = 0;
    Char_Attr        = 0;

    if (NextChar (&c))
          return 1;
    else
      {
        G.ESC_SubCode = c;
             /**********************************/
             /* Read SubCode from input buffer */
             /**********************************/
        switch (G.ESC_SubCode) {
          case (27):
             /*******************************************************/
             /* Double escapes behave exactly like a single one. So */
             /* the subcode read which turned out to be ESCape is   */
             /* returned back to the input buffer (by decrementing  */
             /* the counter) and the function returns 1 to cause    */
             /* the function to be called again in an attempt to    */
             /* get a valid SubCode the next time.                  */
             /*******************************************************/
            if (G.DEF_ATTR)
               {
                 AccProcessChar( (WCHAR) _ESCAPE);
                 AccProcessChar( (WCHAR) G.ESC_SubCode);
               }
            return 1;

          case (43):
              /******************************************************/
              /* 43(+) is the Subcode for the BIDI escape sequence; */
              /* This subcode is treated differently because it is  */
              /* an internal function. i.e. Not included in the CFG */
              /* file (ESC_CHAR_TAB).                               */
              /******************************************************/
            G.START_FUNC = BIDI_SEQ_FOUND;
            return 1;

          default:
            Seq_Found=0;
            for (k=0 ; ((k<MAX_FUNCTIONS) && (!Seq_Found)) ; ++k)
              Seq_Found= (char)(G.ESC_SubCode == CFG->ESC_CHARS_TAB[k].subcode);

            if (Seq_Found) {
 /***************** CASE OF RESOLVED SEQUENCE ************************/
                     k--;
                     G.ESC_OFFSET     = k;
                     G.START_FUNC     = CFG->ESC_CHARS_TAB[k].func;
                     if (G.START_FUNC<MAX_FUNCTIONS)
                       {
                          if (G.DEF_ATTR)
                          {
                            AccProcessChar( (WCHAR) _ESCAPE);
                            AccProcessChar( (WCHAR) G.ESC_SubCode);
                          }
                          return 1;
                       }
                      else  {
                             /**************************************/
                             /* Corrupted CFG file. Treated as an  */
                             /* unresolved sequence                */
                             /**************************************/
                         AccProcessChar( (WCHAR) _ESCAPE);
                         AccProcessChar( (WCHAR) G.ESC_SubCode);
                         return 0;
                      }
                  }
            else  {
 /********** CASE OF UNRESOLVED ESCAPES ***********************/
                     AccProcessChar( (WCHAR) _ESCAPE);
                     AccProcessChar( (WCHAR) G.ESC_SubCode);
                     return 0;
                  }
        }                                       /*endswitch*/
      }      /* endif */
}



/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: CHANGE_LL                                               */
/*                                                                            */
/*   DESCRIPTIVE NAME: Change line length, used to switch the line            */
/*                     length from 8" to 13.2" and vise versa.                */
/*                                                                            */
/*   FUNCTION: This function is called whenever a _CHANGE_LL escape           */
/*             sequence is detected it modifies the global variables          */
/*             accordingly (to accept more/less characters per line):         */
/*             The line buffer is processed and written the output buffer.    */
/*             Both the left (PRT_L_MARGIN)  and right  (PRT_R_MARGIN)        */
/*             margin are set to  the value stated.                           */
/*             The printer's line length (PRT_LINE_LENGTH) is determined      */
/*             by the value of the both the left and right margins in APELS   */
/*             such that when inserting the trailing spaces, the proper       */
/*             number of spaces are inserted.                                 */
/*                                                                            */
/*   ENTRY POINT: (*Functions[CHANGE_LL])                                     */
/*       LINKAGE: CALL   (from _CHANGE_LL_IN,_CHANGE_LL_OUT)                  */
/*                                                                            */
/*   INPUT: (L_MARGIN, R_MARGIN)                                              */
/*        L_MARGIN       CHAR      -This is the new left margin               */
/*        R_MARGIN       CHAR      -This is the new right margin to be set.   */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION: NONE                                           */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*        G              OTHER     -Structure EntryPoint which contains       */
/*                                  the processing context of the current     */
/*                                  input buffer.                             */
/*                                                                            */
/*                                                                            */
/*        PRT_OLD_CWIDTH  WORD     -Default pitch in APELS                    */
/*                                                                            */
/*      ROUTINES:                                                             */
/*          NewLine                                                           */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES: NONE                                                        */
/************************** END OF SPECIFICATIONS *****************************/
void  CHANGE_LL  ( unsigned char L_MARGIN,
                   unsigned char R_MARGIN )
{
         G.PRT_L_MARGIN    = L_MARGIN;
         G.PRT_R_MARGIN    = R_MARGIN;
         G.SFN_PRT_LAST    = G.PRT_R_MARGIN;
         G.PRT_DFLT_LENGTH = G.SFN_PRT_LAST * CFG->PRT_OLD_CWIDTH;
         G.PRT_LINE_LENGTH = G.PRT_DFLT_LENGTH;

         if ( (G.LINE_LENGTH/CFG->PRT_OLD_CWIDTH) > G.PRT_R_MARGIN){
             /***************************************************/
             /* if the new margin is less than the current line */
             /* buffer width. The line buffer is flushed before */
             /* setting the new margins.                        */
             /***************************************************/
            NewLine();
            G.LINE_LENGTH = 0;
            WriteCHAR( 13L );
            WriteCHAR( 10L );
         }
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _CHANGE_LL_OUT    ( for 5201 printer )                   */
/*                                                                            */
/*  DESCRIPTIVE NAME: Change line length to 13.2"                             */
/*                                                                            */
/*  FUNCTION: This function is called whenever a _CHANGE_LL_OUT escape        */
/*            sequence is detected it calls _CHANGE_LL function to            */
/*            modify the margins and line settings.                           */
/*            The escape sequence is passed to the output buffer(printer).    */
/*                                                                            */
/*  ENTRY POINT: (*Functions[_CHANGE_LL_OUT])                                 */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:  None.                                          */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*        character      DWORD     -this is the Control Code to be processed  */
/*                                                                            */
/*     ROUTINES:                                                              */
/*         CHANGE_LL                                                          */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _CHANGE_LL_OUT       (void)
{
    CHANGE_LL( 1, 158 );
    WriteCHAR( character );
    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _CHANGE_LL_IN    ( for 5201 printer )                    */
/*                                                                            */
/*  DESCRIPTIVE NAME: Change line length to 8"                                */
/*                                                                            */
/*  FUNCTION: This function is called whenever a _CHANGE_LL_IN escape         */
/*            sequence is detected it calls _CHANGE_LL function to            */
/*            modify the margins and line settings.                           */
/*            The escape sequence is passed to the output buffer(printer).    */
/*                                                                            */
/*  ENTRY POINT: (*Functions[_CHANGE_LL_IN])                                  */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:  None.                                          */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*        character      DWORD     -this is the Control Code to be processed  */
/*                                                                            */
/*     ROUTINES:                                                              */
/*         CHANGE_LL                                                          */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _CHANGE_LL_IN        (void)
{
    CHANGE_LL( 1, 96 );
    WriteCHAR( character );
    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: _SO_SI                                                  */
/*                                                                            */
/*   DESCRIPTIVE NAME: This is short form for SHIFT_IN and SHIFT_OUT          */
/*                     printer controls.  It is used for SINGLE byte print    */
/*                     codes that change the printing pitch.                  */
/*                                                                            */
/*   FUNCTION: When the function is called the encoded attribute associated   */
/*             with the code is fetched from the table of control codes       */
/*             (NORMAL_CHARS_TAB) with an index equal to the code.            */
/*             The function UpdateAttBuff is then called to update the        */
/*             presentation status (PRESENTION_STATUS).                       */
/*                                                                            */
/*   ENTRY POINT: (*Functions[SO_SI])                                         */
/*       LINKAGE: CALL   (from Processor)                                     */
/*                                                                            */
/*   INPUT: NONE                                                              */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION:                                                */
/*          SAVED_CWIDTH        WORD  -Temporary variable to store CWIDTH     */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*        character      DWORD     -this is the Control Code to be            */
/*                                  processed.                                */
/*                                                                            */
/*        Char_Attr      CHAR      -the Encoded Character attribute           */
/*                                  that defines how the presentation         */
/*                                  status will be updated.                   */
/*                                                                            */
/*        G              OTHER     -Structure EntryPoint which contains       */
/*                                  the processing context of the current     */
/*                                  input buffer.                             */
/*                                                                            */
/*        PRT_SPACE_FOR  OTHER     -Array of characters that is used          */
/*                                  here to know whether the printer          */
/*                                  is one of the 420X family.                */
/*                                                                            */
/*      ROUTINES:                                                             */
/*        UpdatePresStatus                                                    */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES: NONE                                                        */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char _SO_SI(void)
{  unsigned int SAVED_CWIDTH;

   if (character==15L)
	{
/*	  G.ESC_I_FLAG = 0;*/
      if ( ((G.PRESENTION_STATUS & 0X01000000)!=0) &&
           (CFG->PRT_SPACE_FOR[0]==0) )
               /*********************************************/
               /* check if the 12 cpi is on for 420x family */
               /*********************************************/
          Char_Attr=0;
      else {
         if( G.DOUBLE_HEIGHT_FLAG) {
            SAVED_CWIDTH = G.CWIDTH ;
            UpdatePresStatus( &G.PRESENTION_STATUS, (unsigned long)Char_Attr);
         /******************************************************************/
         /* UpdatePresStatus is called to update the presentation status   */
         /* (PRESENTION_STATUS) to reflect the new change in printing mode.*/
         /******************************************************************/
            G.CWIDTH = SAVED_CWIDTH;
            Char_Attr=0;
         }
	}
      } /* endif */

	
    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: _PRT_SELECT                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME: This function is for single byte codes that will       */
/*                     select the printer to go ON-LINE.                      */
/*                                                                            */
/*   FUNCTION: When the function is called a flag (SELECT_FLAG) is set        */
/*             to indicate that the printer is ON-LINE.  And the code         */
/*             is written to the output buffer.                               */
/*                                                                            */
/*   ENTRY POINT: (*Functions[PRT_SELECT])                                    */
/*       LINKAGE: CALL   (from Processor)                                     */
/*                                                                            */
/*   INPUT: NONE                                                              */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION: NONE                                           */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*        character      DWORD     -this is the Control Code to be processed  */
/*                                                                            */
/*        G              OTHER     -Structure EntryPoint which contains       */
/*                                  the processing context of the current     */
/*                                  input buffer.                             */
/*                                                                            */
/*      ROUTINES: NONE                                                        */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES: NONE                                                        */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _PRT_SELECT(void)
{
    G.SELECT_FLAG = 1;
    WriteCHAR( character );
    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: _PRT_DESELECT                                           */
/*                                                                            */
/*   DESCRIPTIVE NAME: This is function is for single byte codes that will    */
/*                     send the printer to an OFF-LINE mode (i.e. the         */
/*                     characters sent to the printer will NOT be printed!)   */
/*                                                                            */
/*   FUNCTION: When the function is called a flag (SELECT_FLAG) is reset      */
/*             to indicate that the printer is OFF-LINE.  And the code        */
/*             is written to the output buffer.                               */
/*                                                                            */
/*   ENTRY POINT: (*Functions[PRT_DESELECT])                                  */
/*       LINKAGE: CALL   (from Processor)                                     */
/*                                                                            */
/*   INPUT: NONE                                                              */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION: NONE                                           */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*        character      DWORD     -this is the Control Code to be processed  */
/*                                                                            */
/*        G              OTHER     -Structure EntryPoint which contains       */
/*                                  the processing context of the current     */
/*                                  input buffer.                             */
/*                                                                            */
/*      ROUTINES: NONE                                                        */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES: NONE                                                        */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _PRT_DESELECT        (void)
{
    G.SELECT_FLAG = 0;
    WriteCHAR( character );
    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: _CAN_FOUND                                              */
/*                                                                            */
/*   DESCRIPTIVE NAME: This will cause the line buffer to be cleared when     */
/*                     the CANCEL is received!                                */
/*                                                                            */
/*   FUNCTION: When the function is called data written to the line buffer    */
/*             (LineBuff) is cleared by reseting the current position in      */
/*             the line buffer to zero. The line length is then also set      */
/*             to zero.  The code will be written to the output buffer.       */
/*                                                                            */
/*   ENTRY POINT: (*Functions[CAN_FOUND])                                     */
/*       LINKAGE: CALL   (from Processor)                                     */
/*                                                                            */
/*   INPUT: NONE                                                              */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION: NONE                                           */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*        character      DWORD     -this is the Control Code to be processed  */
/*                                                                            */
/*        G              OTHER     -Structure EntryPoint which contains       */
/*                                  the processing context of the current     */
/*                                  input buffer.                             */
/*                                                                            */
/*      ROUTINES:                                                             */
/*          InitializeBuffer                                                  */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES: NONE                                                        */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _CAN_FOUND           (void)
{
      G.CurrPos = 0;
      InitializeBuffer();
      G.LINE_LENGTH = 0;
      WriteCHAR( character );
      return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: _RESERVED_1                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME: This function is used to send two contiguous           */
/*                     attribute type escape sequences to the printer one     */
/*                     after the other.  The pair of escape sequences         */
/*                     may be located within the Width Table, the Code On     */
/*                     Table or the Code Off Table.                           */
/*                     The first escape sequence is located at COUNT+1 and    */
/*                     and the second escape sequence is located at COUNT.    */
/*                                                                            */
/*   FUNCTION: When the function is called the encoded attribute associated   */
/*             with the code is fetched from the table of control codes       */
/*             (NORMAL_CHARS_TAB) with an index equal to the code.            */
/*             The function UpdateAttBuff is then called to update the        */
/*             presentation status (PRESENTION_STATUS).                       */
/*             The encoded attribute is then incremented and the function     */
/*             UpdateAttBuff is called to update the presentation status      */
/*             (PRESENTION_STATUS).                                           */
/*                                                                            */
/*   ENTRY POINT: (*Functions[RESERVED_1])                                    */
/*       LINKAGE: CALL   (from Processor)                                     */
/*                                                                            */
/*   INPUT: none                                                              */
/*                                                                            */
/*   SIDE EFFECTS: NONE                                                       */
/*                                                                            */
/*   INTERNAL REFERENCES:                                                     */
/*      LOCAL DATA DEFINITION: NONE                                           */
/*                                                                            */
/*      GLOBAL DATA DEFINITIONS:                                              */
/*        character      DWORD     -this is the Control Code to be processed  */
/*                                                                            */
/*        G              OTHER     -Structure EntryPoint which contains       */
/*                                  the processing context of the current     */
/*                                  input buffer.                             */
/*                                                                            */
/*        Char_Attr      CHAR      -the Encoded Character attribute           */
/*                                  that defines how the presentation         */
/*                                  status will be updated.                   */
/*                                                                            */
/*      ROUTINES:                                                             */
/*            UpdatePresStatus                                                */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES: NONE                                                        */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _RESERVED_1(void)
{
      G.COND_EMP=0;
         /**********************************************************/
         /* Clear the CONDENSED-EMPHASIZE flag. because reserved_1 */
         /* changes the condensed mode.                            */
         /**********************************************************/
      UpdatePresStatus( &G.PRESENTION_STATUS, Char_Attr+1);


      G.ESC_I_FLAG = 0;
      /* set off font selection bit */
      G.PRESENTION_STATUS &= 0xFFFF7FFF;

      return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _RESERVED_2                                              */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is used to send three contiguous          */
/*                    attribute type escape sequences to the printer one      */
/*                    one after the other. The triplet of escape sequences    */
/*                    may be located within the Width Table, the Code On      */
/*                    Table or the Code Off Table.                            */
/*                    The first escape sequence is located at COUNT+2 and     */
/*                    and the second escape sequence is located at COUNT+1    */
/*                    and the third is locted at count.                       */
/*                                                                            */
/*  FUNCTION: When the function is called the encoded attribute associated    */
/*            with the code is fetched from the table of control codes        */
/*            (NORMAL_CHARS_TAB) with an index equal to the code.             */
/*            The function UpdateAttBuff is then called to update the         */
/*            presentation status (PRESENTION_STATUS).                        */
/*            The encoded attribute is then incremented twice calling         */
/*            UpdateAttBuff each time to update the presentation status       */
/*            (PRESENTION_STATUS).                                            */
/*                                                                            */
/*  ENTRY POINT: (*Functions[RESERVED_2])                                     */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: none                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*        character      DWORD     -this is the Control Code to be processed  */
/*                                                                            */
/*        G              OTHER     -Structure EntryPoint which contains       */
/*                                  the processing context of the current     */
/*                                  input buffer.                             */
/*                                                                            */
/*        Char_Attr      CHAR      -the Encoded Character attribute           */
/*                                  that defines how the presentation         */
/*                                  status will be updated.                   */
/*                                                                            */
/*     ROUTINES:                                                              */
/*         UpdatePresStatus                                                   */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _RESERVED_2          (void)
{

      if (!G.DEF_ATTR)
	/*******************************************/
	/* selected font is ended with a new pitch */
	/*******************************************/
	{
	  /* set off font selection bit */
	  G.PRESENTION_STATUS &= 0xFFFF7FFF;

	  /**********************/
	  /* to adjust G.CWIDTH */
	  /**********************/
	  switch (G.PRESENTION_STATUS & 0x3000000)
		{
		 case 2 : 
			  NewWidth (119);
			  break;
		 case 1 : 
			  NewWidth (170);
			  break;
		 case 0 : 
			  NewWidth (204);
			  break;
		}
	}
      UpdatePresStatus( &G.PRESENTION_STATUS, Char_Attr+2);
      UpdatePresStatus( &G.PRESENTION_STATUS, Char_Attr+1);
      return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _DESTRUCT_BS                                             */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function will cause the corresponding single       */
/*                    byte code to delete the last entry in the printer       */
/*                    buffer (LineBuff). The code itself will be consumed.    */
/*                                                                            */
/*  FUNCTION: whenever a code of the above type is detected, the function     */
/*            is called. The last character written to the line buffer        */
/*            (LineBuff) is deleted by decrementing the counter of the        */
/*            number of characters in the line buffer such that the next      */
/*            character will overwrite the character to be deleted. The       */
/*            code itself is consumed and not written to the output buffer.   */
/*                                                                            */
/*  ENTRY POINT: (*Functions[DESTRUCT_BS])                                    */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: ( character)                                                       */
/*                                                                            */
/*         character WORD  -this is the Control Code to be consumed.          */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*        W              @OTHER    - array of three elements to hold          */
/*                                   intermediat values.                      */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*        G              OTHER     -Structure EntryPoint which contains       */
/*                                  the processing context of the current     */
/*                                  input buffer.                             */
/*                                                                            */
/*     ROUTINES:                                                              */
/*       GET_PSM_CWIDTH                                                       */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char _DESTRUCT_BS (void)
{
   int             i, index;
   unsigned int    W[3];
   unsigned long   TempAtt, RETURN_CODE;
   BooleanValue    effect=EDITREPLACE;
   size_t  	   BufferIndex, InpBufferSize, OutBufferSize; 

 if (G.DEF_ATTR==0){
   if (G.CurrPos != 0) {
/******************************** 23.1 **************************************/
      if ((G.PRESENTION_STATUS & PRT_PSM_ATTR) != 0){

         W[0] =  GET_PSM_CWIDTH ((long)        G.LineBuff[G.CurrPos-1].ch,
                                               G.LineBuff[G.CurrPos-1].attr,
                                (unsigned long)G.PRT_PSM_W[0]);
         if ( G.CurrPos == 1){
            W[1] = 0;
         }
         else{
            W[1] = GET_PSM_CWIDTH ((long)        G.LineBuff[G.CurrPos-2].ch,
                                                 G.LineBuff[G.CurrPos-2].attr,
                                  (unsigned long)G.PRT_PSM_W[1]);

          /****************************************************/
          /* we also get the PSM width for the preceding      */
          /* character, this character will take a different  */
          /* shape due to the shaping call after the          */
          /* deletion then we subtract this PSM width also.   */
          /****************************************************/
         }

         if ((G.CurrPos == 1) || (G.CurrPos==2)){
            W[2] = 0;
         }
         else{
            W[2] = GET_PSM_CWIDTH ((long)         G.LineBuff[G.CurrPos-3].ch,
                                                  G.LineBuff[G.CurrPos-3].attr,
                                   (unsigned long)G.PRT_PSM_W[2]);
         }

         G.LINE_LENGTH = G.LINE_LENGTH - W[0] - W[1] - W[2];

      }

 /*     G.CurrPos-=2;  */

     --G.CurrPos;


      G.LineBuff[G.CurrPos].ch = 0x20;
      G.LineBuff[G.CurrPos].attr = 0L;
       
      if (TASHKEEL)    /* 7/8/1993 */
      {
        G.UpperVowelBuff[G.CurrPos].ch   = G.LowerVowelBuff[G.CurrPos].ch   = 0x20;
        G.UpperVowelBuff[G.CurrPos].attr = G.LowerVowelBuff[G.CurrPos].attr = 0L;
      }

      if ((G.PRESENTION_STATUS & PRT_PSM_ATTR) != 0)
      {
          for (i=0; i<G.CurrPos; i++)
               G.CharLineBuff[i] = G.LineBuff[i].ch;

          BufferIndex   = (unsigned long)(G.CurrPos-1);
          OutBufferSize = InpBufferSize = (int)(G.CurrPos);

          RETURN_CODE = layout_object_editshape(plh, effect, 
                                     	        &BufferIndex,
              			     		(unsigned char *)G.CharLineBuff,
				     		&InpBufferSize,
				     		(unsigned char *)G.CharLineBuff2,
				     		&OutBufferSize);

          for (i=0; i<OutBufferSize; i++)
               G.CharLineBuff[BufferIndex+i] = G.CharLineBuff2[i];
          for (i=0; i<G.CurrPos; i++)
               G.LineBuff[i].ch = G.CharLineBuff[i];

         /* maya 28/3/1993 */
         /* if (RETURN_CODE != 0)
           TRACE("/aix3.2/maya/out", "Ret Code from layout_object_editshape =  %d\n", RETURN_CODE); */

         if (G.CurrPos == 0){
            W[1] = 0;
         }
         else{
            W[1] = GET_PSM_CWIDTH ((long)         G.LineBuff[G.CurrPos-1].ch,
                                                  G.LineBuff[G.CurrPos-1].attr,
                                   (unsigned long)G.PRT_PSM_W[1]);

          /****************************************************/
          /* here we get the PSM width for the last character */
          /* in the G.LineBuff after being shaped.            */
          /****************************************************/
         }

         if ((G.CurrPos == 0) || (G.CurrPos == 1)){
            W[2] = 0;
         }
         else{
            W[2] = GET_PSM_CWIDTH ((long)         G.LineBuff[G.CurrPos-2].ch,
                                                  G.LineBuff[G.CurrPos-2].attr,
                                   (unsigned long)G.PRT_PSM_W[2]);
         }
         G.LINE_LENGTH = G.LINE_LENGTH + W[1] + W[2];
      }

      else{
    /*    G.LINE_LENGTH = G.LINE_LENGTH - G.PRT_PSM_W[0];  */
        G.LINE_LENGTH = G.LINE_LENGTH - G.CWIDTH;



      }

      switch (G.CurrPos){

         case 0:
            G.PRT_PSM_W[0] = 0;
            G.PRT_PSM_W[1] = 0;
            G.PRT_PSM_W[2] = 0;
         break;

         case 1:
            G.PRT_PSM_W[0] = G.PRT_PSM_W[1];
            G.PRT_PSM_W[1] = 0;
            G.PRT_PSM_W[2] = 0;
         break;

         case 2:
            G.PRT_PSM_W[0] = G.PRT_PSM_W[1];
            G.PRT_PSM_W[1] = G.PRT_PSM_W[2];
            G.PRT_PSM_W[2] = 0;
         break;

         default:
            G.PRT_PSM_W[0] = G.PRT_PSM_W[1];
            G.PRT_PSM_W[1] = G.PRT_PSM_W[2];
            index = 0;

                /****************************************************/
                /* Here we are going to calculate the character     */
                /* width of the second preceding to get the value   */
                /* of G.PRT_PSM_W[2]                                  */
                /****************************************************/


            TempAtt = G.LineBuff[G.CurrPos].attr;
            for (i=1; i<EXP_ATTR_LENGTH; i++) {
               if ((G.LineBuff[G.CurrPos].attr &(0x80000000 >> index)) != 0) {
                  if (CFG->CODE_ON_TAB[index].pitch != 0) {
                      G.PRT_PSM_W[2] =
                        ( ((CFG->PRT_DFLT_LENGTH / (CFG->CODE_ON_TAB[index].pitch))
                                               * CFG->PRT_D_PITCH) / G.SFN_PRT_LAST );
                  }
                  else{
                     G.PRT_PSM_W[2] = CFG->PRT_OLD_CWIDTH;
                  }
               }
               ++index;
            }                                                     /* endfor */
         break;
      }                                                       /* end switch */
   }                                                              /* end if */
 } /* endif not default */
 return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _HT_FOUND                                                */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function is used for single byte codes that        */
/*                    perform a horizontal tab.                               */
/*                                                                            */
/*  FUNCTION: TABS set by codes that set the tabs are stored in               */
/*            an array named PRT_TABS with a maximum number of tabs equal     */
/*            to 28 and a zero ending the tabs. When a code that performs     */
/*            a horizontal tab is detected, the function is called. The       */
/*            position following the current position in the line is          */
/*            compared to the tabs set in PRT_TABS until either a tab set     */
/*            in PRT_TABS which is greater than the current position in       */
/*            the line is found or the tab set in PRT_TABS is equal to        */
/*            zero (end of TABS).                                             */
/*            If the tab set in PRT_TABS is equal to zero, the control        */
/*            code is consumed since the current position in the line was     */
/*            greater than all tabs set in PRT_TABS. While if a tab in        */
/*            PRT_TABS was found greater than the current position in the     */
/*            line a number of spaces are inserted starting from the          */
/*            position following the current position in line and ending      */
/*            at the desired tab stop.                                        */
/*                                                                            */
/*  ENTRY POINT: (*Functions[HT_FOUND])                                       */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION:                                                 */
/*        i              WORD      -a dummy variable used as an index to      */
/*                                  the array PRT_TABS.                       */
/*                                                                            */
/*        CurrTab        WORD      -A Temporary variable to store the         */
/*                                  destination Tab .                         */
/*                                                                            */
/*        denom          WORD      -                                          */
/*        SAVED_CWIDTH   WORD      -                                          */
/*        HT_ATTR        DWORD     -                                          */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*        character      DWORD     -this is the Control Code to be processed  */
/*                                                                            */
/*        PRT_HT_ATTR_1  WORD      -Contains features that are supported      */
/*                                  during horizontal tab execution.          */
/*                                  ( HI-WORD )                               */
/*                                                                            */
/*        PRT_HT_ATTR_2  WORD      -Contains features that are supported      */
/*                                  during horizontal tab execution.          */
/*                                  ( LOW-WORD )                              */
/*                                                                            */
/*        PRT_HOME_HEAD  @OTHER    -array of character                        */
/*                                  Used to move the head to HOME position.   */
/*                                                                            */
/*        TAB_FLAG       CHAR      -Indicates whether tab stops start from    */
/*                                  0 or 1.                                   */
/*                                                                            */
/*     ROUTINES:                                                              */
/*        InsertSpaces             -used to write a number of spaces to       */
/*                                  the line buffer.                          */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char _HT_FOUND (void)
{
   unsigned int   i, CurrTab, PRT_CURR, denom, SAVED_CWIDTH;
   unsigned long  HT_ATTR;

 if (G.DEF_ATTR == 0 ){

   if (G.ORIENTATION == 1){
      if ((CFG->TAB_FLAG & 2) == 0){
         SAVED_CWIDTH = G.CWIDTH;
         HT_ATTR = (CFG->PRT_HT_ATTR_1 * 0x10000) + CFG->PRT_HT_ATTR_2;
         HT_ATTR &= G.PRESENTION_STATUS;
         if ( ((G.PRESENTION_STATUS & 0x00C00000) != 0)){
              HT_ATTR &= 0xFF3FFFFF;
              G.CWIDTH = G.CWIDTH / 2;
         }
         i        = 0;
         CurrTab  = G.PRT_TABS[i];
         PRT_CURR = (G.LINE_LENGTH / G.CWIDTH)+1;
         while ( (PRT_CURR >= CurrTab) && (CurrTab !=0) ) {
           /*******************************************************************/
           /* the position following the current position in the line         */
           /*          ((G.LINE_LENGTH/PRT_OLD_CWIDTH) + 1)                   */
           /* is compared to the tabs set in G.PRT_TABS until                 */
           /* either G.PRT_TABS[i]                                            */
           /* is equal to zero or G.PRT_TABS[i] is greater than the position  */
           /* following the current position in the line.                     */
           /*******************************************************************/
            CurrTab = G.PRT_TABS[i++];
         }                                                        /* endwhile */

         if (CurrTab != 0)
            if (CurrTab < G.PRT_R_MARGIN) {
               for (i = 0; i < (CurrTab - PRT_CURR); i++)
                   WriteToBuffer(0x20L, HT_ATTR);
            }
            else {
                   AccProcessChar( (WCHAR) 10L);
                   G.LINE_LENGTH=0;
                 }

         G.CWIDTH = SAVED_CWIDTH;
      }
      else{
         if ( (G.PRESENTION_STATUS & 0x00C00000) != 0 )
                /*********************************************/
                /* Temporary Double width is ignore in TABS  */
                /*********************************************/
            denom = G.CWIDTH/2;
         else
            denom = G.CWIDTH;

         PRT_CURR = (G.LINE_LENGTH / denom)+1;

         i        = 0;
         CurrTab  = G.PRT_TABS[i];
         while ( (PRT_CURR > CurrTab)
                   && (CurrTab !=0) ) {
           /*******************************************************************/
           /* the position following the current position in the line         */
           /*          ((LINE_LENGTH/PRT_OLD_CWIDTH) + 1)                     */
           /* is compared to the tabs set in PRT_TABS until either PRT_TABS[i]*/
           /* is equal to zero or PRT_TABS[i] is greater than the position    */
           /* following the current position in the line.                     */
           /*******************************************************************/
            CurrTab = G.PRT_TABS[++i];
         }                                                        /* endwhile */

         if (CurrTab != 0)  {
            G.INSERT_SPACES_START = bidi_out_buff_len;   /* 4/8/1993 TASHKEEL */
            InsertSpaces();
            G.INSERT_SPACES_END   = bidi_out_buff_len;   /* 4/8/1993 TASHKEEL */

            ProcessData();
            if (TASHKEEL)		 		 /* 4/8/1993 TASHKEEL */
                PostProcessTashkeel(); 
            else PostProcess(G.LineBuff);
            G.INSERT_SPACES_START = G.INSERT_SPACES_END = 0;  /* 4/8/1993 TASHKEEL */
            G.CurrPos = 0;
            if (CurrTab < G.PRT_R_MARGIN)
                   G.LINE_LENGTH = CurrTab * denom;
            else {
                   AccProcessChar( (WCHAR) 10L);
                   G.LINE_LENGTH=0;
                 }
            for (i=0; i<CFG->PRT_HOME_HEAD[0]; ++i)
               AccProcessChar( (WCHAR)  CFG->PRT_HOME_HEAD[i+1]);
         }
      }
   }
   else{
         G.LineBuff[G.CurrPos].ch = 0x20;

         i=0;
         CurrTab = G.PRT_TABS[i];
         PRT_CURR = G.CurrPos;

         if (  (G.LINE_LENGTH == 0)
             && ((G.PRESENTION_STATUS & 0x00C00000) != 0)){
              HT_ATTR &= 0xFF3FFFFF;
              G.CWIDTH = G.CWIDTH / 2;
         }

         else {
            if (G.CWIDTH == 408){
               G.CWIDTH   = CFG->PRT_OLD_CWIDTH;
               PRT_CURR = G.LINE_LENGTH / G.CWIDTH;
               HT_ATTR &= 0xFF3FFFFF;
            }
         }

      while ( (PRT_CURR > CurrTab)
                && (CurrTab !=0) ) {
        /*******************************************************************/
        /* the position following the current position in the line         */
        /*          ((G.LINE_LENGTH/PRT_OLD_CWIDTH) + 1)                     */
        /* is compared to the tabs set in G.PRT_TABS until either G.PRT_TABS[i]*/
        /* is equal to zero or G.PRT_TABS[i] is greater than the position    */
        /* following the current position in the line.                     */
        /*******************************************************************/
         CurrTab = G.PRT_TABS[++i];
      }                                                         /* endwhile */

      if ((CurrTab != 0) && (CurrTab < G.PRT_R_MARGIN)) {
         ProcessData();
         if (TASHKEEL)		 		 /* 4/8/1993 TASHKEEL */
            PostProcessTashkeel(); 
         else PostProcess(G.LineBuff);
         G.CurrPos = 0;
         G.LINE_LENGTH = CurrTab * G.CWIDTH;
          /*****************************************************/
          /* AccProcessCharis called and given the code to be  */
          /* written to the output buffer.                     */
          /*****************************************************/
      }

      AccProcessChar( (WCHAR)  character);
   }
 } /* endif not default */
 return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _FLUSH_BUFFER                                            */
/*                                                                            */
/*  DESCRIPTIVE NAME: This function will cause the line buffer to be          */
/*                    printed (as with a Line Feed), and have the next        */
/*                    line's starting print position right after the last     */
/*                    received character.                                     */
/*                                                                            */
/*  FUNCTION: When the function is called indicating an end of line,          */
/*            a number of spaces is inserted at the end of the line to        */
/*            complete a line. The line is then processed as required.        */
/*            After the line has been processed, the post processor is        */
/*            called to reembed the correct Control Code and ESCape           */
/*            sequences and write the line buffer (Line Buff) to the          */
/*            output buffer. The Code is then written to the output buffer    */
/*            following the line to cause a line feed (or several in case     */
/*            of vertical tab). In LTR mode a Carriage Return is also written */
/*            to the output buffer to move the print head to the left (where  */
/*            the next call to InsertSpaces is expecting the head to be).     */
/*            The line length (LINE_LENGTH) is not initialized to zero        */
/*            since we want the first character in the next line to be        */
/*            printed right after the last character in the previous line.    */
/*                                                                            */
/*  ENTRY POINT: (*Functions[FLUSH_BUFFER])                                   */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*        character      DWORD     -this is the Control Code to be processed  */
/*                                                                            */
/*        G              OTHER     -Structure EntryPoint which contains       */
/*                                  the processing context of the current     */
/*                                  input buffer.                             */
/*                                                                            */
/*        Char_Attr      CHAR      -the Encoded Character attribute           */
/*                                  that defines how the presentation         */
/*                                  status will be updated.                   */
/*                                                                            */
/*     ROUTINES:                                                              */
/*        InsertSpaces             -used to write a number of spaces to       */
/*                                  the line buffer.                          */
/*                                                                            */
/*        AccProcessChar           -used to write the control code to the     */
/*                                  output buffer.                            */
/*                                                                            */
/*        UpdatePresStatus         -Update Presentation Status                */
/*                                                                            */
/*        NewLine                  -Write LineBuff to Accumulator             */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _FLUSH_BUFFER (void)
{
    if (!G.DEF_ATTR) {
      if (G.ORIENTATION == 1)
      {
        G.INSERT_SPACES_START = bidi_out_buff_len;      /* 4/8/1993 TASHKEEL */
        InsertSpaces();
        G.INSERT_SPACES_END   = bidi_out_buff_len;      /* 4/8/1993 TASHKEEL */
      }
      NewLine();
      G.INSERT_SPACES_START = G.INSERT_SPACES_END = 0;  /* 4/8/1993 TASHKEEL */
      AccProcessChar( (WCHAR)  character);
      if (G.ORIENTATION == 1)
         AccProcessChar( (WCHAR)  13L);
    }
    return 0;
}



/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: _PRINT_BUFF                                              */
/*                                                                            */
/*  DESCRIPTIVE NAME: This is similar to the FLUSH_BUFFER control, except     */
/*                    the starting print position for the next line will      */
/*                    be right-justified (as with a Carriage Return).         */
/*                                                                            */
/*  FUNCTION: When the function is called indicating an end of line,          */
/*            a number of spaces is inserted at the end of the line to        */
/*            complete a line. The line is then processed as required.        */
/*            After the line has been processed, the post processor is        */
/*            called to reembed the correct Control Code and ESCape           */
/*            sequences and write the line buffer (Line Buff) to the          */
/*            output buffer. The Code is then written to the output buffer    */
/*            following the line to start a new line.                         */
/*                                                                            */
/*  ENTRY POINT: (*Functions[PRINT_BUFF])                                     */
/*      LINKAGE: CALL   (from Processor)                                      */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*        character      DWORD     -this is the Control Code to be processed  */
/*                                                                            */
/*        G              OTHER     -Structure EntryPoint which contains       */
/*                                  the processing context of the current     */
/*                                  input buffer.                             */
/*                                                                            */
/*        Char_Attr      CHAR      -the Encoded Character attribute           */
/*                                  that defines how the presentation         */
/*                                  status will be updated.                   */
/*                                                                            */
/*     ROUTINES:                                                              */
/*        InsertSpaces             -used to write a number of spaces to       */
/*                                  the line buffer.                          */
/*                                                                            */
/*        AccProcessChar           -used to write the control code to the     */
/*                                  output buffer.                            */
/*                                                                            */
/*        UpdatePresStatus         -Update Presentation Status                */
/*                                                                            */
/*        NewLine                  -Write LineBuff to Accumulator             */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES: NONE                                                         */
/************************** END OF SPECIFICATIONS *****************************/
unsigned char  _PRINT_BUFF (void)
{
    if (!G.DEF_ATTR) {
      if (G.ORIENTATION == 1)
      {
        G.INSERT_SPACES_START = bidi_out_buff_len;   /* 4/8/1993 TASHKEEL */
        InsertSpaces();
        G.INSERT_SPACES_END   = bidi_out_buff_len;   /* 4/8/1993 TASHKEEL */
      }
      NewLine();
      G.INSERT_SPACES_START = G.INSERT_SPACES_END = G.LINE_LENGTH = 0;  /* 4/8/1993 TASHKEEL */
      AccProcessChar( (WCHAR)  character);
    }
    return 0;
}

