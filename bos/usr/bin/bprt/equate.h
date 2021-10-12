/* @(#)21	1.1  src/bos/usr/bin/bprt/equate.h, libbidi, bos411, 9428A410j 8/27/93 09:56:58 */
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: none
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
/******************************************************************************/
/*                                                                            */
/*      GENERAL PURPOSE PRINTER DRIVER 'CFG' Source Include File.             */
/*                                                                            */
/*      This   EQUATE.H   source file contains the Association Table          */
/*      equates required to create a 'CFG' binary file.  The equates          */
/*      contain  descriptions  for the type of function a particular          */
/*      ASCII  Control  Code  or  Escape  Sequence  requires for the          */
/*      right-to-left printing.                                               */
/*                                                                            */
/*         NOTE:  The list of EQUates should NOT be modified.                 */
/*                                                                            */
/*      ASSOCIATION TABLE:   The descriptions provided relate to the          */
/*      printer  operation when in a right-to-left orientation.  The          */
/*      printer  freature  for  an  ASCII  Control  Code  or  Escape          */
/*      Sequence  may  operate  differently  on  some  printers when          */
/*      printing in the left-to-right  orientation.   These  equates          */
/*      are  valid  for  entry  into  the  following  tables  as  an          */
/*      association word:                                                     */
/*                                                                            */
/*                      3 - TABLE OF ASCII CONTROL CODES                      */
/*                                                                            */
/*                      4 - TABLE OF ESC SEQUENCES                            */
/*                                                                            */
/*-------------------------------------------------------------------------   */
/* ASSOCIATION WORD                       DESCRIPTION                         */
/*-------------------------------------------------------------------------   */
/* PASS_THROUGH       =  00  This code is used for single byte codes which    */
/*                           do not perform any special features on the       */
/*                           printer.                                         */
/*                                                                            */
/* PRINT_SERVICE_1    =  01  This will pass the corresponding single byte     */
/*                           printer code directly to the printer when        */
/*                           received.                                        */
/*                                                                            */
/* DESTRUCT_BS        =  02  This will cause the corresponding single byte    */
/*                           code to delete the last entry in the printer     */
/*                           buffer (effective for right-to-left printing).   */
/*                                                                            */
/* SPECIAL_ONE        =  03  This is used for single byte codes that are      */
/*                           printable under CHARACTER SET 2, and are         */
/*                           treated as control codes in CHARACTER SET 1.     */
/*                                                                            */
/* HT_FOUND           =  04  This is used for single byte codes that          */
/*                           perform a horizontal tab.  The result of this    */
/*                           tab in right-to-left printing is described in    */
/*                           the Programmer's Reference.                      */
/*                                                                            */
/* FLUSH_BUFFER       =  05  This will cause the print buffer to be printed   */
/*                           (as with a Line Feed), and have the next line's  */
/*                           starting print position right after the last     */
/*                           received character.                              */
/*                                                                            */
/* PRINT_BUFF         =  06  This is similar to the FLUSH_BUFFER control,     */
/*                           except the starting print position for the       */
/*                           next line will be right-justified (as with       */
/*                           a Carriage Return).                              */
/*                                                                            */
/* SO_SI              =  07  This is short form for SHIFT_IN and SHIFT_OUT    */
/*                           printer controls.  It is used for single byte    */
/*                           print codes that change the printing pitch.      */
/*                           For most printers this is useful for the         */
/*                           compress and double width modes!                 */
/*                                                                            */
/* PRT_SELECT         =  08  This is for single byte codes that will          */
/*                           select the printer to go ON-LINE.  This is       */
/*                           useful only with PRT_DESELECT feature!           */
/*                                                                            */
/* PRT_DESELECT       =  09  The single byte code will send the printer       */
/*                           into an OFF-LINE mode - ie. the characters       */
/*                           sent to the printer will NOT be printed!         */
/*                                                                            */
/* CAN_FOUND          =  10  This will cause the print buffer to be cleared   */
/*                           when the CANCEL is received!                     */
/*                                                                            */
/* ESC_FOUND          =  11  This is one of the more frequently used          */
/*                           printer controls.  This is key to the use        */
/*                           of the ESCape sequences that follow character    */
/*                           27!                                              */
/*                                                                            */
/* ESC_SINGLE         =  12  For ESCape sequences such as 'ESC E' this        */
/*                           function will act on the corresponding printer   */
/*                           encoded ATTRIBUTE.  This is only for ESCape      */
/*                           sequences that are two bytes long!               */
/*                                                                            */
/* IGNORE_ESC_n_0     =  13  For ESCape sequences that do not affect the      */
/*                           the operation of the printer driver - that       */
/*                           are 4 bytes long (ie. 'ESC x n 0').  The         */
/*                           entire sequence will be sent to the printer      */
/*                           immediately!                                     */
/*                                                                            */
/*                           NOTE: Sending the ESCape sequence immediately    */
/*                                 applies to ALL 'IGNORE_ESC....' codes!     */
/*                                                                            */
/* IGNORE_ESC_n       =  14  As per function 13, except it is for             */
/*                           3 byte codes.                                    */
/*                                                                            */
/* ESC_SINGLE_0_1     =  15  This is for ESCape sequences that use a          */
/*                           toggle of character 0/1 or ASCII character       */
/*                           "0/1" for enable and disable a feature.          */
/*                           An example is the 'ESC W 0/1' for double         */
/*                           width printing.                                  */
/*                                                                            */
/* ZERO_TERMINATOR    =  16  This is used for ESCape sequences which          */
/*                           are sent directly to the printer that end        */
/*                           with a character 0 (ie. to indicate that the     */
/*                           ESCape sequence has completed).                  */
/*                                                                            */
/* ESC_C_FOUND        =  17  This is the standard 'ESC C ...' sequence        */
/*                           used to set the page size in inches and/or       */
/*                           lines of text.  It is a special case ESCape      */
/*                           sequence that is sent directly to the printer    */
/*                           when received in a datastream.                   */
/*                                                                            */
/* TAB_SET_FOUND      =  18  This is used to set the new tab settings         */
/*                           for horizontal tabs.  It is terminated by        */
/*                           character 0 and the values following the         */
/*                           ESCape are the column tab settings.              */
/*                                                                            */
/* RIGHT_MARGIN_FOUND =  19  This ESCape sequence is specifically for         */
/*                           setting the right margin (in terms of            */
/*                           columns) ONLY.  ESCape sequences that            */
/*                           combine the left and right margins in one        */
/*                           sequence are covered by another function.        */
/*                                                                            */
/* ESC_SUB_SUPER      =  20  This ESCape sequence (as per ESC_SINGLE_0_1)     */
/*                           is a toggle type!  The ESCape sequence will      */
/*                           invoke the super/subscript form depending on     */
/*                           the 3rd byte value.                              */
/*                                                                            */
/* LEFT_MARGIN_FOUND  =  21  See RIGHT_MARGIN_FOUND                           */
/*                                                                            */
/* IGNORE_ESC_n_0_0   =  22  See IGNORE_ESC_n_0 - used for 5 byte sequences   */
/*                                                                            */
/* REVERSE_LF_n       =  23  This will perform the same result as a           */
/*                           FLUSH_BUFFER except that it is used for          */
/*                           printers that support REVERSE LINE FEED's        */
/*                           (sequence is 3 bytes long).                      */
/*                                                                            */
/* GRAPHICS           =  24  This is used for printers that support           */
/*                           graphics (for dot matrix type printers).         */
/*                           When the GRAPHICS ESCape sequence is detected,   */
/*                           the count of binary information following is     */
/*                           sent directly to the printer.  If graphics       */
/*                           are printed when the driver is in right-to-      */
/*                           left then no text should be printed on the       */
/*                           same line.                                       */
/*                                                                            */
/* GRAPHICS_STAR      =  25  This is specific support for the EPSON FX-100    */
/*                           for the 'ESC * m n1 n2 ...' graphics sequence.   */
/*                           It operates similar to the GRAPHICS function     */
/*                           code - ie. only graphics on a line for right-    */
/*                           to-left output!                                  */
/*                                                                            */
/* DOWNLOAD_EPSON     =  26  This is a specific ESCape sequence used for      */
/*                           the EPSON FX-100 printer like download           */
/*                           sequences.                                       */
/*                                                                            */
/* DOWNLOAD_PROPRINT  =  27  This is as per DOWNLOAD_EPSON but for the        */
/*                           IBM 4201 Proprinter like sequences.              */
/*                                                                            */
/* RESET_TAB_SETTINGS =  28  This will return the printer and the printer     */
/*                           driver back to the initial horizontal tab        */
/*                           settings.                                        */
/*                                                                            */
/* PRINTER_RESET      =  29  This is a printing mode reset used to clear      */
/*                           the printing modes back to the default modes!    */
/*                           The sequence is 2 bytes long (ESC x).            */
/*                                                                            */
/*                           CAUTION:  With some printers this will also      */
/*                                     clear the downloaded Arabic/English    */
/*                                     font.                                  */
/*                                                                            */
/* CHANGE_LL_OUT      =  30  This is a specific code assigned for the         */
/*                           IBM 5201 Quietwriter printer.  The feature       */
/*                           is changing the length of the printing line      */
/*                           to 13.2 inches (from 8 inches).                  */
/*                                                                            */
/* CHANGE_LL_IN       =  31  See CHANGE_LL_OUT - to restore the printing      */
/*                           line length back to 8 inches!                    */
/*                                                                            */
/* SET_HOR_MARGINS    =  32  This is similar to the setting of the left       */
/*                           and right margins of the printing line.  The     */
/*                           difference is that one sequence sets both        */
/*                           margins.  This is four byte sequence that has    */
/*                           'ESC x left right' as the ESCape format!         */
/*                                                                            */
/* IGNORE_ESC_n_000   =  33  See IGNORE_ESC_n_0 - used for 6 byte sequences   */
/*                                                                            */
/* REVERSE_LF         =  34  See REVERSE_LF_n - except sequence is only       */
/*                           2 bytes long.                                    */
/*                                                                            */
/* PRT_DESELECT_n     =  35  This operates as per for PRT_DESELECT sequence   */
/*                           but it is 3 bytes long (ESC x n).                */
/*                                                                            */
/* PMP                =  36  This is specifically for the IBM 3812 Page-      */
/*                           printer.  The PMP stands for Page Map Primitive  */
/*                           commands that are supported by this printer.     */
/*                           The PMP sequence is sent directly to the         */
/*                           printer!                                         */
/*                                                                            */
/* RESERVED_1         =  37  This function is used to send two contiguous     */
/*                           attribute type escape sequences to the printer   */
/*                           one after the other.  The pair of escape         */
/*                           sequences may be located within the Width        */
/*                           Table, the Code On Table or the Code Off Table.  */
/*                           The first escape sequence is located at COUNT+1  */
/*                           and the second escape sequence is located at     */
/*                           COUNT.                                           */
/*                                                                            */
/* PRINTABLE          =  38  This function is used for single byte codes      */
/*                           that are treated as printable (for both          */
/*                           CHARACTER SET 1 and 2).  Refer to PRINTABLE!     */
/*                                                                            */
/* PRINT_NEXT         =  39  This function is used for single ESC  codes      */
/*                           that will print next character as graphic.       */
/*                                                                            */
/* PRINT_ALL          =  40  This function is used for ESC codes(4 Bytes long)*/
/*                           that will print the following (Count=LLHH)       */
/*                           characters as graphic.                           */
/*                                                                            */
/* RESTORE_JMP_2      =  41  This will pass the ESC along with the single     */
/*                           byte printer code directly to the printer when   */
/*                           received.                                        */
/*                                                                            */
/* RESERVED_2         =  42  This function is used to send three contiguous   */
/*                           attribute type escape sequences to the printer   */
/*                           one after the other.  The triplet of escape      */
/*                           sequences may be located within the Width        */
/*                           Table, the Code On Table or the Code Off Table.  */
/*                           The first escape sequence is located at COUNT+2  */
/*                           and the second escape sequence is located at     */
/*                           COUNT+1 and the third is located at COUNT.       */
/*                                                                            */
/******************************************************************************/

#define  PASS_THROUGH            0
#define  PRINT_SERVICE_1         1
#define  DESTRUCT_BS             2
#define  SPECIAL_ONE             3
#define  HT_FOUND                4
#define  FLUSH_BUFFER            5
#define  PRINT_BUFF              6
#define  SO_SI                   7
#define  PRT_SELECT              8
#define  PRT_DESELECT            9
#define  CAN_FOUND               10
#define  ESC_FOUND               11
#define  ESC_SINGLE              12
#define  IGNORE_ESC_n_0          13
#define  IGNORE_ESC_n            14
#define  ESC_SINGLE_0_1          15
#define  ZERO_TERMINATOR         16
#define  ESC_C_FOUND             17
#define  TAB_SET_FOUND           18
#define  RIGHT_MARGIN_FOUND      19
#define  ESC_SUB_SUPER           20
#define  LEFT_MARGIN_FOUND       21
#define  IGNORE_ESC_n_0_0        22
#define  REVERSE_LF_n            23
#define  GRAPHICS                24
#define  GRAPHICS_STAR           25
#define  DOWNLOAD_EPSON          26
#define  DOWNLOAD_PROPRINT       27
#define  RESET_TAB_SETTINGS      28
#define  PRINTER_RESET           29
#define  CHANGE_LL_OUT           30
#define  CHANGE_LL_IN            31
#define  SET_HOR_MARGINS         32
#define  IGNORE_ESC_n_000        33
#define  REVERSE_LF              34
#define  PRT_DESELECT_n          35
#define  PMP                     36
#define  RESERVED_1              37
#define  PRINTABLE               38
#define  PRT_NEXT                39
#define  PRT_ALL                 40
#define  RESTORE_JMP_2           41
#define  RESERVED_2              42
#define  HANDLE_ESC_BRKT         43
#define  SPACE_FOR_BAK           44
#define  BIDI_SEQ_FOUND          45
#define  ESC_I                   46

