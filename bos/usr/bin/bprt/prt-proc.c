static char sccsid[] = "@(#)30	1.2  src/bos/usr/bin/bprt/prt-proc.c, libbidi, bos411, 9428A410j 11/4/93 15:33:28";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: AdjustWidth
 *		FLUSH_SPACES
 *		FirstInitialize
 *		FlushLine
 *		GET_PSM_CWIDTH
 *		GetTheByte
 *		InitializeBuffer2601
 *		InsertSpaces
 *		PADD_SPACES_1
 *		PADD_SPACES_2
 *		PSM_WriteToBuffer3003
 *		PostProcess
 *		PostProcessTashkeel1622
 *		ProcessData
 *		Processor
 *		ProcessorInitialize
 *		PutByte
 *		UpdatePresStatus2388
 *		WriteToBuffer
 *		if
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
/* SOURCE FILE NAME: prt-proc.c (Processor)                                   */
/*                                                                            */
/* FUNCTION:   This module contains the bidirectional processing routines     */
/*             that are called to process a certain input buffer.             */
/*                                                                            */
/* ENTRY POINTS:							      */
/*	     InitializeBuffer						      */
/*	     WriteToBuffer						      */
/*	     GET_PSM_CWIDTH						      */
/*	     PSM_WriteToBuffer						      */
/*	     FlushLine							      */
/*           ProcessorInitialize                                              */
/*           Processor                                                        */
/*                                                                            */
/* EXTERNAL ROUTINES USED:                                                    */
/*   From Functions module (prt-func.c):                                      */
/*           _PASS_THROUGH                                                    */
/*           _PRINT_SERVICE_1                                                 */
/*           _DESTRUCT_BS                                                     */
/*           _SPECIAL_ONE                                                     */
/*           _HT_FOUND                                                        */
/*           iFLUSH_BUFFER                                                    */
/*           _PRINT_BUFF                                                      */
/*           _SO_SI                                                           */
/*           _PRT_SELECT                                                      */
/*           _PRT_DESELECT                                                    */
/*           _CAN_FOUND                                                       */
/*           _ESC_FOUND                                                       */
/*           _ESC_SINGLE                                                      */
/*           _IGNORE_ESC_n_0                                                  */
/*           _IGNORE_ESC_n                                                    */
/*           _ESC_SINGLE_0_1                                                  */
/*           _ZERO_TERMINATOR                                                 */
/*           _ESC_C_FOUND                                                     */
/*           _TAB_SET_FOUND                                                   */
/*           _RIGHT_MARGIN_FOUND                                              */
/*           _ESC_SUB_SUPER                                                   */
/*           _LEFT_MARGIN_FOUND                                               */
/*           _IGNORE_ESC_n_0_0                                                */
/*           _REVERSE_LF_n                                                    */
/*           _GRAPHICS                                                        */
/*           _GRAPHICS_STAR                                                   */
/*           _DOWNLOAD_EPSON                                                  */
/*           _DOWNLOAD_PROPRINT                                               */
/*           _RESET_TAB_SETTINGS                                              */
/*           _PRINTER_RESET                                                   */
/*           _CHANGE_LL_OUT                                                   */
/*           _CHANGE_LL_IN                                                    */
/*           _SET_HOR_MARGINS                                                 */
/*           _IGNORE_ESC_n_000                                                */
/*           _REVERSE_LF                                                      */
/*           _PRT_DESELECT_n                                                  */
/*           _PMP                                                             */
/*           _RESERVED_1                                                      */
/*           _PRINTABLE                                                       */
/*           _PRT_NEXT                                                        */
/*           _PRT_ALL                                                         */
/*           _RESTORE_JMP_2                                                   */
/*           _RESERVED_2                                                      */
/*           _HANDLE_ESC_BRKT                                                 */
/*           _SPACE_FOR_BAK                                                   */
/*           _BIDI_SEQ_FOUND                                                  */
/*	     _ESC_I							      */
/*                                                                            */
/*  From accumulator module (arpr.c):                                         */
/*           AccProcessChar                                                   */
/*           AccProcessLine                                                   */
/*                                                                            */
/*  OTHERS:                                                                   */
/*           layout_object_transform					      */
/*           layout_object_editshape                                          */
/*           CodePageTrans                                                    */
/*           MemMove                                                          */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

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



/*        done for temporary test driver for the BidiAp */

extern EntryPoint G;
extern unsigned short counter;
extern unsigned long character;
extern unsigned long Char_Attr;
extern unsigned long PRT_PSM_ATTR;
extern unsigned long CURR_CHAR_ATTR;
extern int CodeSet;
extern unsigned char Hebrew, Arabic;
extern unsigned char debug;


/***************************************************/


unsigned long HCP = 0;
extern PBDInterface BDI;
extern unsigned short *PsmTable;

/**************************** START OF SPECIFICATIONS *************************/
/*  SUBROUTINE NAME: ProcessorInitialize                                      */
/*                                                                            */
/*  DESCRIPTIVE NAME: used to initialise the different variables used         */
/*                    in the bidi processing of a buffer                      */
/*                                                                            */
/*  FUNCTION: this function is called one before each print job in order      */
/*            to initialize variables used in the processing of the buffer    */
/*            which is done according to the bidi attributes.                 */
/*                                                                            */
/*  ENTRY POINT: InitializeProcessor                                          */
/*      LINKAGE: CALL (from BidiPrint)                                        */
/*                                                                            */
/*  INPUT: NONE                                                               */
/*                                                                            */
/*  SIDE EFFECTS: NONE                                                        */
/*                                                                            */
/*  INTERNAL REFERENCES:                                                      */
/*     LOCAL DATA DEFINITION: NONE                                            */
/*                                                                            */
/*     GLOBAL DATA DEFINITIONS:                                               */
/*        G              OTHER     -Structure EntryPoint which contains       */
/*                                  the processing context of the current     */
/*                                  input buffer, is to be initialized.       */
/*                                                                            */
/*    EntryPoint has the following structure:                                 */
/*                                                                            */
/*   G.LineBuff     OTHER               -  array [255] of structure Buffer    */
/*                                         which contains characters to be    */
/*                                         Bidi processed (reversed,shaped    */
/*                                         Etc.)                              */
/*                         CHAR  ch            - character to be processed    */
/*                         LONG  attr          - its presentation status      */
/*                                                                            */
/*   G.PRESENTION_STATUS  DWORD    - Current presentation status defining     */
/*                                   the printers current status              */
/*                                   (i.e the mode in which the next          */
/*                                   character is to be printed).             */
/*                                   e.g. emphasized, condensed etc..         */
/*                                                                            */
/*   G.PREV_CHAR_ATTR     DWORD    - Previous character attibute.             */
/*                                                                            */
/*   G.GRAPHICS_CHAR_COUNT                                                    */
/*                        DWORD - Count of characters that are printable      */
/*                                  (from all characters chart).              */
/*                                                                            */
/*   G.IGNORE_COUNT       DWORD   - Count of characters that will not be      */
/*                                  processed and will be sent to the         */
/*                                  output buffer once received.              */
/*                                                                            */
/*   G.BIDI_ATTR          DWORD   - Current Bidirectional printer attribute.  */
/*                                  It indicates how the characters in the    */
/*                                  LineBuff will be (bidi) processed.        */
/*                                  Note that the BIDI_ATTR is not allowed    */
/*                                  to change unless the LineBuff is empty.   */
/*                                                                            */
/*   G.PRT_LINE_LENGTH    WORD    - Currently active Printer Width (APELS)    */
/*                                  as described by L_MARGIN and R_MARGIN     */
/*                                                                            */
/*   G.LINE_LENGTH        WORD    - Width of Characters in the LineBuff       */
/*                                  (APELS), is zero when LineBuff is         */
/*                                  empty.                                    */
/*                                                                            */
/*   G.CWIDTH             WORD    - Character Width (APELS)                   */
/*                                                                            */
/*   G.PRT_PSM_W[]        OTHER   - Array [3] of WORD                         */
/*                                  Used in PSM processing                    */
/*                                                                            */
/*   G.CurrPos            WORD    - Denotes the current position within       */
/*                                  the LineBuff.                             */
/*                                                                            */
/*   G.SELECT_FLAG        WORD    - Denotes whether the printer is            */
/*                                    0=ONLINE                                */
/*                                    1=OFFLINE                               */
/*                                                                            */
/*   G.COUNT_DH           WORD    - Contains the count found in the _DW_DH    */
/*                                  Function.                                 */
/*                                                                            */
/*   G.PRT_DFLT_LENGTH    WORD    - Printer Maximum Width in APELS.           */
/*                                                                            */
/*   G.START_FUNC         WORD    - Contains the number of the function       */
/*                                  that is currently active or will be       */
/*                                  called next if (IncompFlag==1).           */
/*                                                                            */
/*   G.PRT_TABS           OTHER                                               */
/*                                                                            */
/*   G.ORIENTATION        CHAR    - Currently active orientation              */
/*                                  Contains 0=LTR                            */
/*                                           1=RTL                            */
/*                                                                            */
/*   G.PRT_CURR_TAB       CHAR    - Contains the position where the next      */
/*                                  TAB will be read in TAB_SET_FOUND.        */
/*                                                                            */
/*   G.ZERO_TERM_FLAG     CHAR    - Puts the monitor in a passthru state      */
/*                                  until a NULL=0 character is received.     */
/*                                                                            */
/*   G.PRT_ESC_OFFSET     CHAR    - Contains a count of the number of         */
/*                                  characters read within a function.        */
/*                                  It is used to put the function where      */
/*                                  it left off when a packet is consumed     */
/*                                  before the function completes its         */
/*                                  operation.                                */
/*                                                                            */
/*   G.END_LINE_FLAG      CHAR    - Denotes whether the printer is at a line  */
/*                                  boundry. BIDI sequences are only accepted */
/*                                  when (END_LINE_FLAG==1).                  */
/*                                  This flag is set at the end of each line  */
/*                                  and reset when the first character is     */
/*                                  written to the line buffer.               */
/*                                                                            */
/*   G.InCompFlag         CHAR    - Denotes whether the previous function     */
/*                                  has finished processing or not. The       */
/*                                  START_FUNC will continue to gain control  */
/*                                  untill it returns 0 indicating completing */
/*                                  its processing.                           */
/*                                                                            */
/*   G.ESC_OFFSET         CHAR    - Contains the position of the ESCape       */
/*                                  subcode within the ESC_TABLE and is       */
/*                                  used to invoke the function with the      */
/*                                  proper processing attribute Char_Attr.    */
/*                                                                            */
/*   G.COUNT[4]           OTHER   - Array [4] of CHAR                         */
/*                                  Temporary array used to store multiple    */
/*                                  byte counters (2 or 4) against packet     */
/*                                  breaks.                                   */
/*                                                                            */
/*   G.PRT_L_MARGIN       CHAR    - Active Printer Left Margin.               */
/*                                                                            */
/*   G.PRT_R_MARGIN       CHAR    - Active Printer right Margin.              */
/*                                                                            */
/*   G.SFN_PRT_LAST       CHAR    - Number of Chararacter Per line in         */
/*                                  Default Pitch.                            */
/*                                                                            */
/*   G.S1                 CHAR    - Used in Shaping.                          */
/*                                                                            */
/*   G.DOUBLE_HEIGHT_FLAG CHAR    - This flag is set on when current          */
/*                                  printing mode is double width / double    */
/*                                  height,and is set off otherwize (default) */
/*                                                                            */
/*   G.DOUBLE_HEIGHT_ESC  OTHER   - This is an array to hold any ESC @ -      */
/*                                  a double width / double height escape     */
/*                                  sequence encountered in data              */
/*                                                                            */
/*   G.BRACKET_FUNCTIONS  CHAR    - Holds the function number of function     */
/*                                  that is called when an escape [ is found  */
/*                                  in data buffer                            */
/*                                                                            */
/*   G.PITCH              CHAR                                                */
/*                                                                            */
/*   G.DEF_ATTR           CHAR    - Default Bidi Attribute.                   */
/*                                  When the BIDI_ATTR = DEFAULT_BIDI         */
/*                                  the Printer Monitor (LTR-PASSTHRU)        */
/*                                  is invisible from the user point of       */
/*                                  view but still monitors the data stream   */
/*                                  for different escape sequences.           */
/*                                                                            */
/*   G.ESC_SubCode        CHAR    - The Last ESCAPE Subcode received.         */
/*                                                                            */
/*        PRT_PSM_ATTR   DWORD     -Mask to test for PSM bits in the          */
/*                                  Presentation status.                      */
/*        counter        WORD      -index to character currently processed    */
/*                                  in buffer                                 */
/*        character      DWORD     -character pointed to by counter           */
/*        Char_Attr                                                           */
/*        CURR_CHAR_ATTR                                                      */
/*        PSM_TABLE      @OTHER    -an array containing the with (in apels)   */
/*                                  of characters of current used code page   */
/*                                                                            */
/*        BDI            @OTHER    -strucure BidirectionalInterface           */
/*            in_buff        @OTHER  - input buffer containing data to be     */
/*                                     processed                              */
/*            in_buff_len    WORD    -length of input buffer                  */
/*            BDAtts         LONG    -attributes according to which           */
/*                                    processing is to be performed           */
/*            CFG            CHAR    -cfg number of cfg header file           */
/*            Mode           CHAR    -Whether to initialize or process        */
/*            PrtCodePage    CHAR    -code page used by printer (output file) */
/*            prt_len        CHAR    -width (in 10cpi characters of printer   */
/*                                                                            */
/*     ROUTINES:                                                              */
/*           InitializeBuffer                                                 */
/*           MemMove                                                          */
/*                                                                            */
/*  EXTERNAL REFERENCES:                                                      */
/*     ROUTINES:                                                              */
/************************** END OF SPECIFICATIONS *****************************/
void ProcessorInitialize ( void )
{
   PRT_PSM_ATTR  =(0x10000 * CFG->PRT_PSM_ATTR_1) + CFG->PRT_PSM_ATTR_2;
   G.PITCH=10;
   /* for the 4019 printer, date = 21/1/92 */ 
   /* this is to account to decrementing that occurs in the PADD_SPACES_1
      done to overcome the cutting done by the device driver */
   if (BDI->CFG == _4019 && BDI->prt_len >= 80) 
     G.SFN_PRT_LAST=BDI->prt_len-1;
   else
     G.SFN_PRT_LAST=BDI->prt_len;
   G.PRT_DFLT_LENGTH  = (G.SFN_PRT_LAST) * CFG->PRT_OLD_CWIDTH;
   G.PRESENTION_STATUS = (CFG->PRT_ATTR1 * 0x10000) + CFG->PRT_ATTR2;
   G.CurrPos = 0;
   G.PRT_LINE_LENGTH = G.PRT_DFLT_LENGTH;
   G.INSERT_SPACES_START = G.INSERT_SPACES_END = 0;   	  /* 4/8/1993 TASHKEEL */
   G.LINE_LENGTH = 0;
   G.CWIDTH = CFG->PRT_OLD_CWIDTH;
   G.SELECT_FLAG = 1;
   G.GRAPHICS_CHAR_COUNT = 0;
   G.PRT_L_MARGIN = 1;
   G.PRT_R_MARGIN = G.SFN_PRT_LAST;
   G.START_FUNC = 0;
   G.InCompFlag = 0;
   G.ESC_OFFSET = 0;
   G.ESC_SubCode = 0;
   G.PRT_ESC_OFFSET = 0;
   G.ZERO_TERM_FLAG = 0;
   G.PRT_CURR_TAB = 0;
   G.END_LINE_FLAG = 1;
   G.PREV_CHAR_ATTR = 0;
   G.S1 = 0;
   Char_Attr = 0;
   G.PRT_PSM_W[0] = 0;
   G.PRT_PSM_W[1] = 0;
   G.PRT_PSM_W[2] = 0;
   G.COUNT_DH = 0;
   G.COND_EMP = 0;
   G.DOUBLE_HEIGHT_FLAG = 0;
   G.BIDI_ATTR = BDI->BDAtts;

   if (BDI->CFG == _4207)
			/* ie. in case of 864 codepage then seen, sad etc */
			/* are on one cell (yasmine font).		  */
     G.BIDI_ATTR |= 0x00000080;
			/* then force the sad, dad etc. to be shaped on   */
			/* one cell.					  */
   /* else */
			/* else force them to be on two cells 	 	  */
			/* eg. sad + tail				  */
     /* G.BIDI_ATTR &= 0xFFFFFF7F; */

   G.IGNORE_COUNT = G.SAVED_GRAPHICS = 0;

   G.ORIENTATION = (unsigned char) ((G.BIDI_ATTR & 0x00110000) >> 16);

   if (Hebrew) 
	HCP=1;
     
   if (Hebrew && BDI->CFG == _4019)
     PsmTable = BOLDFACE_BARAK_12;
   else
     if (BDI->PrtCodePage == CP864)
       PsmTable = CFG->PSM_TABLE_1046;
     else
       PsmTable = CFG->PSM_TABLE_ORG_1046;

  if (G.BIDI_ATTR == DEFAULT_BIDI_ATTRIBUTES)
        /*****************************************************************/
        /* here we check if the printer attributes are set to the default*/
        /* so that no monitor interferance is included                   */
        /*****************************************************************/
    G.DEF_ATTR = 1;
  else
    G.DEF_ATTR = 0;
/*///////////////////////// Initializing BidiAttributes and Flags //////////*/
   counter        = 0;
          /****************************************************************/
          /* "counter" is the index into the data buffer that indicates   */
          /*  which characters were processed and which will be processed */
          /*  next.                                                       */
          /****************************************************************/
   CURR_CHAR_ATTR = 0;

   G.DOUBLE_HEIGHT_ESC [0] = 27;
   G.DOUBLE_HEIGHT_ESC [1] = G.ESC_BRKT_I [1] = 91;
   G.ESC_I_SEQ [0] = G.ESC_BRKT_I [0] = 27;
   G.ESC_I_SEQ [1] = 'I';
   G.ESC_I_FLAG = 0;
   G.PRT_NUM = BDI->CFG;

   /*InitializeBuffer ();*/
   FirstInitialize ();

       /*******************************************************/
       /* fill the line buffer with blanks of null attribute. */
       /*******************************************************/

       MemMove( G.PRT_TABS, CFG->PRT_DEF_TABS, sizeof(G.PRT_TABS));
       /********************************************************/
       /* Initialize PRT_TABS by copying PRT_DEF_TABS into it. */
       /********************************************************/


       /*****************************************************************/
       /*   Here we will initialize the Layout Values from G.BIDI_ATTR. */
       /*****************************************************************/
       /* sprintf(TempStr, "G.BIDI_ATTR=%x\n", G.BIDI_ATTR);
        write(1, TempStr, strlen(TempStr)); */

       if (G.BIDI_ATTR & 0x00000010)    /* base shapes */
           set_new_layout_values((G.BIDI_ATTR & 0xFFFF0000),
                                 (G.BIDI_ATTR & 0x0000FFFF));
       else if (G.BIDI_ATTR & 0x00000001)    /* passthrough */
                set_new_layout_values((G.BIDI_ATTR & 0xFFFF0000),
                                      (G.BIDI_ATTR & 0x0000FFC0));
             else set_new_layout_values((G.BIDI_ATTR & 0xFFFF0000)|0x00000010,
                                        (G.BIDI_ATTR & 0x0000FFFF));
}


/**************************** START OF SPECIFICATIONS ***********************/
/*  SUBROUTINE NAME: Processor                                              */
/*                                                                          */
/*  DESCRIPTIVE NAME: this is the main function which when called, the      */
/*                    data stream directed to the printer will be           */
/*                    processed according to the current BIDI attributes.   */
/*                                                                          */
/*  FUNCTION: When the function is called, its function is to process       */
/*            the buffer (BDI->in_buff) sent from BidiPrint, and whose      */
/*            length is stored in BDI->in_buff_len                          */
/*                                                                          */
/*            The function is given a pointer to a BidirectionalInterface   */
/*            structure in which is stored the processing context. The      */
/*            context is copied into a global variable G to be reached by   */
/*            all the processing functions.                                 */
/*                                                                          */
/*            The buffer is processed and characters are written to the     */
/*            output buffer (bidi_out_buff). After the whole packet has been*/
/*            processed, the variables used by the Processor in G are saved */
/*            back to BDI structure                                         */
/*                                                                          */
/*            The data stream flowing  in the  chain includes  both         */
/*            printable characters and codes.                               */
/*            Codes are divided into three categories:                      */
/*                                     -Pass Through Codes                  */
/*                                     -Content Dependant Codes             */
/*                                 and -Attribute Type Codes                */
/*            Pass Through Codes are ASCII Control Codes or Escape          */
/*            Sequences that affect the operation of a printer but their    */
/*            position is not critical in right to left printing. These     */
/*            sequences are immediately sent to the printer (written to     */
/*            the output buffer) and are not imbeded in the reversed data   */
/*            stream (e.g setting the line spacing or vertical tabs).       */
/*                                                                          */
/*            Content Dependant Codes are ASCII Control Codes or Escape     */
/*            Sequences where the information contained in these codes      */
/*            affect the operation of the program (e.g setting the          */
/*            margins). After these codes are processed, they are sent      */
/*            to the printer (written to the output buffer) as with the     */
/*            pass through type.                                            */
/*                                                                          */
/*            Attribute Type Codes are ASCII Control Codes or Escape        */
/*            Sequences that affect the size and format of the printed      */
/*            characters. These codes update the presentation status(PS)    */
/*            determining the size and format of the printed characters.    */
/*            AS characters are sent to the printer ,the required ASCII     */
/*            Control Codes and Escape Sequences are imbeded in the data    */
/*            stream as changes in the printer attibute are determined.     */
/*            This ensures proper printing modes when text is printed in    */
/*            right to left ORIENTATION.                                    */
/*                                                                          */
/*            When a control code or escape sequences is detected it is     */
/*            processed according to predefined functions found in eit-     */
/*            her the NORMAL_CHARS_TAB or ESC_CHAR_TAB. Printable           */
/*            characters are written to the line buffer (G.LineBuff)        */
/*            together with their associated attributes. When a line is     */
/*            completed, the characters in LineBuff are processed and       */
/*            sent to the printer (written to the output buffer).           */
/*                                                                          */
/*  ENTRY POINT: Processor                                                  */
/*      LINKAGE: CALL (from BidiPrint)                                      */
/*                                                                          */
/*  INPUT : NONE                                                            */
/*                                                                          */
/*  SIDE EFFECTS: NONE                                                      */
/*                                                                          */
/*  INTERNAL REFERENCES:                                                    */
/*     LOCAL DATA DEFINITION: NONE                                          */
/*                                                                          */
/*     GLOBAL DATA DEFINITIONS:                                             */
/*       BDI->in_buff_len                                                   */
/*                       WORD       -The index pointing to end of data      */
/*                                   sent in buffer (BDI->in_buff)          */
/*                                                                          */
/*       BDI->in_buff   @OTHER      -The input data buffer                  */
/*                                                                          */
/*       BDI->PrtCodePage                                                   */
/*                       CHAR       -The printer code page                  */
/*                                      CP864  code page 1046               */
/*                                      CP1046 code page 1046               */
/*                                                                          */
/*       charcter        CHAR       -character pointed to by counter        */
/*                                                                          */
/*       G               OTHER      -Structure EntryPoint which contains    */
/*                                   the processing context of the current  */
/*                                   input buffer.                          */
/*                                                                          */
/*       Char_Attr       CHAR       -a temporary variable used to           */
/*                                   hold the attribute associated          */
/*                                   with the ESCape sequence or with       */
/*                                   the Control code.                      */
/*                                                                          */
/*       counter         CHAR       -a counter used to loop on the          */
/*                                   characters in the packet.              */
/*                                                                          */
/*       Functions       @OTHER     -a pointer to an array of pointers      */
/*                                   of size MAX_FUNCTIONS; where the       */
/*                                   addresses of the processing            */
/*                                   functions are stored.                  */
/*                                                                          */
/*       CFG->NORMAL_CHARS_TAB                                              */
/*                       @OTHER    -a pointer to an array of control        */
/*                                   character table of structure:          */
/*                                      WORD  attr    - Encoded attribute   */
/*                                      WORD  func    - Function number     */
/*                                          (Index within Functions table)  */
/*                                                                          */
/*       CFG->ESC_CHARS_TAB                                                 */
/*                       @OTHER   -a pointer to an array of supported ESCs  */
/*                                  of the following structure:             */
/*                                      CHAR  subcode - ESCape subcode      */
/*                                      CHAR  attr    - Encoded attribute   */
/*                                      WORD  func    - Function number     */
/*                                          (Index within Functions table)  */
/*     ROUTINES:                                                            */
/*         AccProcessChar                                                   */
/*         Functions [G.START_FUNC]                                         */
/*         UpdatePresStatus                                                 */
/*         _PRINTABLE                                                       */
/*         _PASS_THROUGH                                                    */
/*                                                                          */
/*  EXTERNAL REFERENCES:                                                    */
/*     ROUTINES: NONE                                                       */
/************************** END OF SPECIFICATIONS ***************************/
void Processor ( void )
{                                                            /*begin main*/
unsigned long i;


  if (G.BIDI_ATTR == DEFAULT_BIDI_ATTRIBUTES)
        /*****************************************************************/
        /* here we check if the printer attributes are set to the default*/
        /* so that no monitor interferance is included                   */
        /*****************************************************************/
    G.DEF_ATTR = 1;
  else
    G.DEF_ATTR = 0;
/*///////////////////////// Initializing BidiAttributes and Flags //////////*/
  counter        = 0;
          /****************************************************************/
          /* "counter" is the index into the data buffer that indicates   */
          /*  which characters were processed and which will be processed */
          /*  next.                                                       */
          /****************************************************************/

  CURR_CHAR_ATTR = 0;


  while(counter < BDI->in_buff_len)
                /**********************************************/
                /* Loop till the buffer is totally processed. */
                /**********************************************/
   {

    character  = BDI->in_buff[counter++];
    if (G.IGNORE_COUNT || G.SAVED_GRAPHICS)
                 /****************************************************/
                 /* IGNORE_COUNT indicates the number of characters  */
                 /* to pass unprocessed before processing continues. */
                 /****************************************************/
      {
	if (G.IGNORE_COUNT)
	  {
       	    AccProcessChar( (WCHAR) character);
       	    G.IGNORE_COUNT--;
	  }
	else
	  {
	      G.LineBuff [G.CurrPos].escapes [G.LineBuff [G.CurrPos].escsize-G.SAVED_GRAPHICS] = character;
	    if (G.SAVED_GRAPHICS == 1)
	      G.CurrPos++;
            G.SAVED_GRAPHICS--;
	  }
      }

    else
      if (G.ZERO_TERM_FLAG)
        {
         AccProcessChar( (WCHAR) character);
         G.ZERO_TERM_FLAG = (char)character;
                 /***************************************************/
                 /* ZERO_TERM_FLAG indicates that a Zero terminated */
                 /* sequence was received. Characters are passed    */
                 /* unprocessed until a NULL (zero) character is    */
                 /* received.                                       */
                 /***************************************************/
        }

      else
        if ((G.InCompFlag) && (G.START_FUNC<MAX_FUNCTIONS))
          {               /*****************************************/
           counter--;     /* Running functions get their own input */
                          /*****************************************/
           Char_Attr = (G.START_FUNC==BIDI_SEQ_FOUND)
                       ? (char) 0
                       : CFG->ESC_CHARS_TAB[G.ESC_OFFSET].attr;
                 /********************************************/
                 /* _BIDI_SEQ_FOUND is an internal function. */
                 /* i.e. not found in the ESC_CHARS_TAB      */
                 /* and it does not affect the  PS           */
                 /* PRESENTION_STATUS.                       */
                 /********************************************/

           G.InCompFlag=Functions[G.START_FUNC] ();
                 /**************************************************/
                 /*  The interrupted/Starting function is called   */
                 /*  to continue processing.                       */
                 /*  The function would return in InCompFlag:      */
                 /*                                                */
                 /*     1    To indicate unfinished processing.    */
                 /*          In this case the function will        */
                 /*          regain control again When control     */
                 /*          reaches this statment the next time.  */
                 /*                                                */
                 /*     0    To indicate that the function has     */
                 /*          terminated processing.                */
                 /**************************************************/

           if (Char_Attr != 0)
             UpdatePresStatus(&G.PRESENTION_STATUS, Char_Attr);
                 /*********************************************************/
                 /* The Encoded character attribute is executed to update */
                 /* the printing mode                                     */
                 /*********************************************************/
          }

        else
          {
           if (  (G.DEF_ATTR)
                        /***********************************************/
                        /* when in default mode, characters are echoed */
                        /* whenever received.                          */
                        /***********************************************/
                && (character!=27) )
                        /*********************************************/
                        /* This condition is added to let ESC_FOUND  */
                        /* filter out the bidi-sequence (ESC +),     */
                        /* EVEN when in default mode (G.DEF_ATTR==1) */
                        /*********************************************/
             AccProcessChar( (WCHAR) character);

           if ((G.SELECT_FLAG==1) || (character==17))
                                    /* if the printer is ONLINE */
             {
              if (G.GRAPHICS_CHAR_COUNT)
                _PRINTABLE     ();
                 /*****************************************************/
                 /* GRAPHICS_CHAR_COUNT is the count of characters    */
                 /* that are to be printed from ALL-CHARACTERS-CHART; */
                 /* Even if these characters are controls.            */
                 /* GRAPHICS_CHAR_COUNT is Set in                     */
                 /*      _PRT_NEXT                                    */
                 /*      _PRT_ALL                                     */
                 /* and reset in                                      */
                 /*      _PRINTABLE                                   */
                 /*****************************************************/
              else
                if (character<32)
                  {
                          /********************************************/
                          /* This is the case for control characters. */
                          /********************************************/
                   Char_Attr   = CFG->NORMAL_CHARS_TAB[character].attr;
                          /*********************************************/
                          /* Fetching the Encoded character attribute. */
                          /*********************************************/
                   G.START_FUNC=CFG->NORMAL_CHARS_TAB[character].func;
                   G.InCompFlag= Functions[G.START_FUNC]();
                          /******************************************/
                          /* The function implementing the controls */
                          /* is invoked.  Controls-functions all    */
                          /* return zero EXCEPT for _ESC_FOUND      */
                          /* which usually returns one to invoke    */
                          /* escape-sequence-functions.             */
                          /******************************************/

                   if (Char_Attr != 0)
                     UpdatePresStatus(&G.PRESENTION_STATUS, Char_Attr);
                     /*********************************************************/
                     /* The Encoded character attribute is executed to update */
                     /* the printing mode                                     */
                     /*********************************************************/
                  }
                else
                  if (character != 127)
                    _PRINTABLE    ();
                       /****************************************************/
                       /* This is the case of normal printable characters. */
                       /****************************************************/
                  else
                    _PASS_THROUGH ();
                       /***********************************************/
                       /* Character 127 is a non-printable character. */
                       /***********************************************/
             }                                    /* end of selected mode  */
           else
                    /*****************************/
                    /* if the prinnter is OFFLINE */
                    /*****************************/
             if (!G.DEF_ATTR)
               AccProcessChar( (WCHAR) character); /* Deselected mode */
          }
   }
}

/**************************** START OF SPECIFICATIONS *************************/
/*   SUBROUTINE NAME: FlushLine                                               */
/*                                                                            */
/*   DESCRIPTIVE NAME: This function is called to write the contents of the   */
/*                     line buffer to the output buffer.                      */
/*                                                                            */
/*   FUNCTION:  This function calls ProcessData to BIDI-process the data      */
/*              in the line buffer. Then it calls PostProcess to send         */
/*              data into the output buffer and embed proper controls. Then   */
/*              calls are made to write a carriage return and a line feed     */
/*              into the output buffer to force a new line.  And the final    */
/*              call to initialize the buffer with blanks.                    */
/*                                                                            */
/*   ENTRY POINT: FlushLine()                                                 */
/*       LINKAGE: CALL (from WriteToBuffer, PSM_WriteToBuffer)                */
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
/*        InsertSpaces                                                        */
/*        ProcessData                                                         */
/*        PostProcess                                                         */
/*        InitializeBuffer                                                    */
/*        AccProcessChar                                                      */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*      ROUTINES: None.                                                       */
/*                                                                            */
/************************** END OF SPECIFICATIONS *****************************/
void FlushLine()
{

    if (G.ORIENTATION)
    {
       G.INSERT_SPACES_START = bidi_out_buff_len;   /* 4/8/1993 TASHKEEL */
       InsertSpaces();
       G.INSERT_SPACES_END   = bidi_out_buff_len;   /* 4/8/1993 TASHKEEL */
    }
            /*******************************************************/
            /* if printing is in RTL mode (ORIENTATION = 1), call  */
            /* InsertSpaces to insert the proper number of spaces  */
            /* at the end of the line for right justification.     */
            /*******************************************************/
    ProcessData();
            /******************************************************/
            /* call ProcesssData to process the line in the line  */
            /* buffer according to the current BIDI ATTRIBUTE.    */
            /******************************************************/
    /* 4/8/1993 TASHKEEL */
    if (TASHKEEL)
       PostProcessTashkeel();
    else PostProcess(G.LineBuff);
            /**********************************************************/
            /* call PostProcess to reembed the proper ESCape sequnces */
            /* and Control codes while writing the line to the        */
            /* output buffer.                                         */
            /**********************************************************/
    AccProcessChar( (WCHAR) 10);
    AccProcessChar( (WCHAR) 13);
            /*************************************************************/
            /* A Line Feed and a Carriage Return are sent to the printer */
            /* to start a new line.                                      */
            /*************************************************************/
    G.END_LINE_FLAG = 1;
            /***************************************************/
            /* set END_LINE_FLAG since an end of line occured. */
            /***************************************************/
    G.CurrPos = 0;
              /*******************************************************/
              /* Line is empty; Put the next character at position 0 */
              /*******************************************************/
    InitializeBuffer ();
            /************************************************/
            /* Initialize Fills the line buffer with blanks */
            /************************************************/
    G.INSERT_SPACES_START = G.INSERT_SPACES_END = G.LINE_LENGTH = 0;  /* 4/8/1993 TASHKEEL */
}



/**************************** START OF SPECIFICATIONS ***********************/
/*  SUBROUTINE NAME: FLUSH_SPACES                                           */
/*                                                                          */
/*  DESCRIPTIVE NAME:   This fuction writes spaces in the output buffer     */
/*                                                                          */
/*  FUNCTION: It determines the width (in APLES) in which we are going      */
/*            to write a number of spaces to the output buffer.             */
/*            This is needed for the right justification of the line.       */
/*                                                                          */
/*                                                                          */
/*  ENTRY POINT: FLUSH_SPACES                                               */
/*                                                                          */
/*      LINKAGE: CALL (from PADD_SPACES_1 and PADD_SPACES_2)                */
/*                                                                          */
/*  INPUT: ( width )                                                        */
/*         width    DWORD               -the width of the character         */
/*                                       space in APLES                     */
/*                                                                          */
/*  SIDE EFFECTS: NONE                                                      */
/*                                                                          */
/*  INTERNAL REFERENCES:                                                    */
/*     LOCAL DATA DEFINITION: NONE                                          */
/*                                                                          */
/*     GLOBAL DATA DEFINITIONS:                                             */
/*         G              OTHER     -Structure EntryPoint which contains    */
/*                                   the processing context of the current  */
/*                                   input buffer.                          */
/*                                                                          */
/*         CFG->PRT_D_PITCH  WORD   - Default Pitch. (10,12,17..)           */
/*         CFG->PRT_OLD_CWIDTH                                              */
/*                           DWORD  - Default Character in APELS.           */
/*         CFG->PRT_SPACE_FOR                                               */
/*                           @OTHER -Used for right-justification.          */
/*                                    and test for 5202 in PSM_WIDTH.       */
/*                                                                          */
/*     ROUTINES:                                                            */
/*        AccProcessChar ()                                                 */
/*                                                                          */
/*  EXTERNAL REFERENCES:                                                    */
/*     ROUTINES: NONE                                                       */
/************************** END OF SPECIFICATIONS ***************************/
void FLUSH_SPACES(unsigned long width)
{
   if ((CFG->PRT_SPACE_FOR[1] & 0x40) != 0) {                 /* 5202 printer */
      if ((G.PRESENTION_STATUS & PRT_PSM_ATTR) == 0){    /* not PSM */
         width = (CFG->PRT_OLD_CWIDTH * CFG->PRT_D_PITCH) /
                 G.PITCH;
         width = (width == (CFG->PRT_SPEC_PITCH + 1)) ?
                           (unsigned long) CFG->PRT_SPEC_PITCH : width;
              /**********************************/
              /* Check for 5202 special pitch   */
              /* 120 apels changed to 119 apels */
              /**********************************/
      }
   }

   while (G.LINE_LENGTH + width <= G.PRT_LINE_LENGTH){
      AccProcessChar((long)0x20);
      G.LINE_LENGTH +=(unsigned int) width;
        /**********************************************************************/
        /* while there is space(room)in printer line (taking in consideration */
        /* that there is characters with width of LINE_LENGTH to be printed   */
        /* on the same line, after the spaces).                               */
        /**********************************************************************/
   }
}





/**************************** START OF SPECIFICATIONS ***********************/
/*  SUBROUTINE NAME: PADD_SPACES_1                                          */
/*                                                                          */
/*  DESCRIPTIVE NAME: Handles space insertion for printers that support     */
/*                    forward print head movement.                          */
/*                                                                          */
/*  FUNCTION: This function is called when we need to insert spaces         */
/*            for right justification for printers that supports            */
/*            a forward print head movement with a fraction of a pitch      */
/*                                                                          */
/*                                                                          */
/*  ENTRY POINT: PADD_SPACES_1                                              */
/*      LINKAGE: CALL (from InsertSpaces)                                   */
/*                                                                          */
/*  INPUT: NONE                                                             */
/*                                                                          */
/*  SIDE EFFECTS: NONE                                                      */
/*                                                                          */
/*  INTERNAL REFERENCES:                                                    */
/*     LOCAL DATA DEFINITION:                                               */
/*         counter         WORD                                             */
/*         i               WORD                                             */
/*         temp            WORD                                             */
/*         temp_width      DWORD                                            */
/*         devider         CHAR                                             */
/*                                                                          */
/*                                                                          */
/*     GLOBAL DATA DEFINITIONS:                                             */
/*       G              OTHER     -Structure EntryPoint which contains      */
/*                                 the processing context of the current    */
/*                                 input buffer.                            */
/*                                                                          */
/*       CFG->PRT_OLD_CWIDTH   DWORD - Default Character in APELS.          */
/*                                                                          */
/*       PRT_SPACE_FOR    @OTHER - Array of 11 characters Used in           */
/*                                 right-justification.                     */
/*        Byte 0     : Contains the information below:                      */
/*                        Bit 7    - ORIENTATION of count field of sequence */
/*                                       case 1: HH:LL                      */
/*                                       case 0: LL:HH                      */
/*                        Bit 6    - Indicates the size of fractions:       */
/*                                       case 1: 1/240 of an inch           */
/*                                       case 0: 1/120 of an inch           */
/*                        Bit 0..5 - ratio of Apels to Fraction of an inch  */
/*                                       3812,5202: 17                      */
/*                                       others   : 01                      */
/*        Byte 1     : Contains the information below:                      */
/*                        Bit 7    - Whether a single sequence could cover  */
/*                                   the whole line or we need to pad       */
/*                                   spaces first.                          */
/*                        Bit 6    - Indicates that the printer is a 5202   */
/*                                   for special handling.                  */
/*                        Bit 0..5 - the length of the escape sequence      */
/*        Byte 2     : Contains the offset of the count field within the    */
/*                     SPACE_FOR array, used to place the count in its      */
/*                     proper location.                                     */
/*        Byte 3..10 : Contains the escape sequence of the horizontal       */
/*                     movement with a fraction of an inch.                 */
/*                                                                          */
/*     ROUTINES:                                                            */
/*        AccProcessChar                                                    */
/*        GET_PSM_CWIDTH                                                    */
/*        FLUSH_SPACES                                                      */
/*                                                                          */
/*  EXTERNAL REFERENCES:                                                    */
/*     ROUTINES: NONE                                                       */
/************************** END OF SPECIFICATIONS ***************************/
void PADD_SPACES_1()
{
   int             cnter, i;
   unsigned int    temp;
   unsigned long   temp_width;
   unsigned char   divider;

   if ((CFG->PRT_SPACE_FOR[1] & 0x80) == 0){
         /*************************************************************/
         /*            Case of 3812 printer                           */
         /*  Padd with spaces, reducing the space to be added by      */
         /*  this routine. So that the Temp_Width would not overflow  */
         /*  one word.                                                */
         /*************************************************************/
      temp = G.CWIDTH;
      if ((G.PRESENTION_STATUS & PRT_PSM_ATTR) != 0)
         G.CWIDTH = GET_PSM_CWIDTH ((unsigned long)0x20, G.PRESENTION_STATUS,
                                    (unsigned long)CFG->PRT_OLD_CWIDTH);
      else
         G.CWIDTH = CFG->PRT_OLD_CWIDTH;
      FLUSH_SPACES((unsigned long)G.CWIDTH);
      G.CWIDTH = temp;
   }

   divider    = (CFG->PRT_SPACE_FOR[0] & 0x3F);
          /*********************************/
          /* divider contains the ratio of */
          /*    APELS : (1/120th) inch     */
          /*********************************/


   temp_width = G.PRT_LINE_LENGTH - G.LINE_LENGTH;
          /************************************************/
          /* Temp_Width contains the APELS to be added to */
          /* insure poper right-justification of text     */
          /************************************************/

   if ((CFG->PRT_SPACE_FOR[0] & 0x40) != 0)
      temp_width = temp_width * 2;
         /*************************************************************/
         /*            Case of 3812 printer                           */
         /*  Amuont of APELS to padd is adjusted to fit the ESC [ C   */
         /*  command of the 3812.                                     */
         /*************************************************************/

   temp_width = temp_width/ divider;
          /***************************************************/
          /* Temp_Width now contains the number of fractions */
          /* of an inch that represents the count to be put  */
          /* in the horizontal move escape sequence to be    */
          /* sent below.                                     */
          /***************************************************/


   if ((CFG->PRT_SPACE_FOR[0] & 0x80) != 0){
          /****************************************************/
          /* The count field in the escape sequence has to be */
          /* High byte first        count = HH:LL             */
          /****************************************************/
      CFG->PRT_SPACE_FOR[CFG->PRT_SPACE_FOR[2]] 
			  = (unsigned char)(temp_width / 0x100);
      CFG->PRT_SPACE_FOR[CFG->PRT_SPACE_FOR[2]+1]
			  = (unsigned char)(temp_width % 0x100);
   }
   else{
          /****************************************************/
          /* The count field in the escape sequence has to be */
          /* Low byte first        count = LL:HH              */
          /****************************************************/
      CFG->PRT_SPACE_FOR[CFG->PRT_SPACE_FOR[2]]
			   = (unsigned char)(temp_width % 0x100);
      CFG->PRT_SPACE_FOR[CFG->PRT_SPACE_FOR[2]+1]
			   = (unsigned char)(temp_width / 0x100);
   }

   cnter = (CFG->PRT_SPACE_FOR[1] & 0x3F);
          /*********************************************/
          /* Counter contains the length of the escape */
          /* sequence to be sent.                      */
          /*********************************************/

   for (i = 0; i < cnter; ++i)
      AccProcessChar( (long)CFG->PRT_SPACE_FOR[3+i]);
        /************************************************/
        /* Sending the horizontal-move-escape-sequence. */
        /************************************************/

}




/**************************** START OF SPECIFICATIONS ***********************/
/*  SUBROUTINE NAME: PADD_SPACES_2                                          */
/*                                                                          */
/*  DESCRIPTIVE NAME: Handles space insertion for printers that do          */
/*                    not support forward print movement                    */
/*                                                                          */
/*  FUNCTION: This fuction is called when we need to insert spaces          */
/*            needed for right justification for printers that do not       */
/*            support a forward print movement with fraction of a pitch     */
/*                                                                          */
/*                                                                          */
/*  ENTRY POINT: PADD_SPACES_2                                              */
/*                                                                          */
/*      LINKAGE: CALL (from InsertSpaces)                                   */
/*                                                                          */
/*  INPUT: NONE                                                             */
/*                                                                          */
/*  SIDE EFFECTS: NONE                                                      */
/*                                                                          */
/*  INTERNAL REFERENCES:                                                    */
/*     LOCAL DATA DEFINITION:                                               */
/*         i               WORD                                             */
/*         remainder       WORD                                             */
/*         temp            WORD                                             */
/*         denom           WORD                                             */
/*         pitch           CHAR                                             */
/*         PRT_END_ATTR    DWORD                                            */
/*         TEMP_BUFF       DWORD                                            */
/*                                                                          */
/*     GLOBAL DATA DEFINITIONS:                                             */
/*         G                OTHER  -Structure EntryPoint which contains     */
/*                                  the processing context of the current   */
/*                                  input buffer.                           */
/*                                                                          */
/*         CFG->PRT_OLD_CWIDTH   DWORD - Default Character in APELS.        */
/*                                                                          */
/*         CFG->PRT_SPACE_FOR    @OTHER -Used for right-justification.      */
/*                                  and test for 5202 in PSM_WIDTH.         */
/*                                                                          */
/*         CFG->PRT_MIN_CWIDTH   DWORD  -Minimum CWIDTH in APELS.           */
/*                                                                          */
/*         CFG->PRT_MIN_PNTER    DWORD  -points to control code in CODE_ON  */
/*                                  table which turns on the  minimum       */
/*                                  pitch.                                  */
/*                                                                          */
/*     ROUTINES:                                                            */
/*        AccProcessChar                                                    */
/*        AdjustWidth                                                       */
/*        FLUSH_SPACES                                                      */
/*                                                                          */
/*  EXTERNAL REFERENCES:                                                    */
/*     ROUTINES: NONE                                                       */
/************************** END OF SPECIFICATIONS ***************************/
void PADD_SPACES_2()
{
   int            i, remainder;
   unsigned int   temp, denom, SAVED_LINE_LENGTH;
   unsigned char  pitch;
   unsigned long  PRT_END_ATTR;
   unsigned long  TEMP_BUFF;

   SAVED_LINE_LENGTH = G.LINE_LENGTH;
/* increment line length in case of PSM */
   if ((G.PRESENTION_STATUS >> 16) & 0x1 == 1)
        G.LINE_LENGTH++;
   temp = G.CWIDTH;

      PRT_END_ATTR = (CFG->PRT_END_ATTR_1 * 0x10000L) + CFG->PRT_END_ATTR_2;
      CURR_CHAR_ATTR = G.PREV_CHAR_ATTR & (PRT_END_ATTR) ;
      TEMP_BUFF = (CURR_CHAR_ATTR ^ G.PREV_CHAR_ATTR);
                   /***********************************************************/
                   /* Xoring the current character attribute (CURR_CHAR_ATTR) */
                   /* and the previous character attribute (PREV_CHAR_ATTR)   */
                   /* indicates whether they are the same or not.In case they */
                   /* are different TempBuff will have some bits equal to     */
                   /* ones. These bits are in the positions where the         */
                   /* difference between both attributes occur.               */
                   /***********************************************************/
      if ((TEMP_BUFF & 0x00800000) != 0)
      {
 /*******************************/
 /* Flushing the output buffer. */
 /*******************************/
          AccProcessLine( (WCHAR) CFG->CODE_OFF_TAB[8].length
                         ,(WCHAR) 1
                         ,(char*) CFG->CODE_OFF_TAB[8].sequence);
                                   /*******************************************/
                                   /* call AccProcessLine to write the ESCape */
                                   /* sequence found in the CODE_OFF_TAB,     */
                                   /* with an index equal to index, to the    */
                                   /* output buffer.                          */
                                   /*******************************************/
       }
      G.PREV_CHAR_ATTR = CURR_CHAR_ATTR;

      if (G.DOUBLE_HEIGHT_FLAG)
         AccProcessLine (9, 1, DOUBLE_HEIGHT_END_ESC);
                    /******************************************/
                    /* This is to end DH_DW before insertion  */
                    /* of spaces, so that changing of pitch   */
                    /* is effective.                          */
                    /* done especially for 4201, 4208.        */
                    /******************************************/

	/* if font has been selected then make sure that pitch is 10 for
		padding spaces ie. temporarily ending the selection	*/
      if (G.ESC_I_FLAG)
	AccProcessChar (18);


   G.CWIDTH = CFG->PRT_OLD_CWIDTH;


   while ( ((G.LINE_LENGTH + CFG->PRT_MIN_CWIDTH) <= G.PRT_LINE_LENGTH) ){
         /***************************************************/
         /* While can add more spaces to the printer line,  */
         /* proceed to add the space with the proper pitch. */
         /***************************************************/
      remainder = (G.PRT_LINE_LENGTH - G.LINE_LENGTH) % G.CWIDTH;
            /**********************************************/
            /* Checking whether spaces of CWIDTH could be */
            /* used in padding, without leaving a small   */
            /* fraction.                                  */
            /**********************************************/

      if ((remainder != 0) && (remainder != 17))
          {
           /******************************************/
           /* This is the case when spaces of width  */
           /* CWIDTH will not do the job.            */
           /******************************************/
         for (i = 0; i<32; ++i)
                  {
            if (CFG->CODE_OFF_TAB[i].width == 102)
               denom = 408;
                   /*************************/
                   /* Case of Double width. */
                   /*************************/
            else
               denom = CFG->CODE_OFF_TAB[i].width;


            remainder = (CFG->CODE_OFF_TAB[i].width != 0) ?
                        ((G.PRT_LINE_LENGTH - G.LINE_LENGTH) % denom):
                         (G.PRT_LINE_LENGTH - G.LINE_LENGTH) % G.CWIDTH;

            if ((remainder == 0) ||
                (remainder == CFG->CODE_ON_TAB[CFG->PRT_MIN_PNTER/9].pitch))
                                {
                    /******************************************/
                    /* This is the case when spaces of width  */
                    /* CODE_OFF_TAB[i].width will do the job. */
                    /******************************************/
               AccProcessLine( (WCHAR) CFG->CODE_ON_TAB[i].length
                              ,(WCHAR) 1
                              ,(char*) CFG->CODE_ON_TAB[i].sequence);
                      /*************************************/
                      /* send the sequence to activate the */
                      /* desired pitch.                    */
                      /*************************************/
               pitch = CFG->CODE_ON_TAB[i].pitch;
               AdjustWidth((unsigned long)pitch,(unsigned long) 0x80);
               FLUSH_SPACES((unsigned long)G.CWIDTH);
                       /**********************************/
                       /* Padd spaces with the new width */
                       /**********************************/
               AccProcessLine( (WCHAR) CFG->CODE_OFF_TAB[i].length
                              ,(WCHAR) 1
                              ,(char*) CFG->CODE_OFF_TAB[i].sequence);
                      /*************************************/
                      /* send the sequence to activate the */
                      /* default pitch.                    */
                      /*************************************/
               AdjustWidth((unsigned long)pitch, (unsigned long)0x00);
               i = 32;
                      /**************************************/
                      /* This is done to exit the for loop. */
                      /**************************************/
            }  /* end if remainder */
         }  /* end for i */
      }   /* end if remainder */
      else
          {
         FLUSH_SPACES((unsigned long)G.CWIDTH);
                       /**************************************/
                       /* Padd spaces with the default width */
                       /**************************************/
      }


      if ((G.LINE_LENGTH + CFG->PRT_MIN_CWIDTH) <= G.PRT_LINE_LENGTH){
           /************************************************************/
           /* if there is room for a space with minimum pitch, add one */
           /************************************************************/
       AccProcessLine( (WCHAR) CFG->CODE_ON_TAB[CFG->PRT_MIN_PNTER/9].length
                      ,(WCHAR) 1
                      ,(char*) CFG->CODE_ON_TAB[CFG->PRT_MIN_PNTER/9].sequence);
               /**************************/
               /* Activate minimum pitch */
               /**************************/
         AccProcessChar( (long)0x20);
               /******************/
               /* Send one space */
               /******************/
         G.LINE_LENGTH += CFG->PRT_MIN_CWIDTH;
               /**********************/
               /* Update Line Length */
               /**********************/
         AccProcessLine( (WCHAR) CFG->CODE_OFF_TAB[CFG->PRT_MIN_PNTER/9].length
                        ,(WCHAR) 1
                        ,(char*) CFG->CODE_OFF_TAB
                                        [CFG->PRT_MIN_PNTER/9].sequence);
                /***************************/
                /* return to default pitch */
                /***************************/
      }
   }

      if (G.DOUBLE_HEIGHT_FLAG)
        AccProcessLine (9, 1, G.DOUBLE_HEIGHT_ESC);
                /******************************************/
                /* This is to restart DH_DW in case it    */
                /* was temporarily ended.                 */
                /* done especially for 4201, 4208.        */
                /******************************************/
      if (G.ESC_I_FLAG)
	AccProcessLine (3, 1, G.ESC_I_SEQ);

   G.CWIDTH = temp;
   G.LINE_LENGTH = SAVED_LINE_LENGTH;
}




/**************************** START OF SPECIFICATIONS ***********************/
/*  SUBROUTINE NAME: InsertSpaces                                           */
/*                                                                          */
/*  DESCRIPTIVE NAME: Insert spaces in the line buffer for right            */
/*                    justification                                         */
/*                                                                          */
/*  FUNCTION: This function determines if the connected printer supports    */
/*            a print forward movement, if yes, it calls PADD_SPACES_1      */
/*            else it calls PADD_SPACES_2                                   */
/*                                                                          */
/*  ENTRY POINT: InsertSpaces                                               */
/*                                                                          */
/*      LINKAGE: CALL      (from  _HT_FOUND,                                */
/*                                _FLUSH_BUFFER,                            */
/*                                _PRINT_BUFF,                              */
/*                                _REVERSED_LF_n,                           */
/*                                _GRAPHICS,                                */
/*                                _PRINTER_RESET,                           */
/*                                _REVERSE_LF,                              */
/*                                _SPACE_FOR_BAK,                           */
/*                                _DW_DH,                                   */
/*                                FlushLine,                                */
/*                                CHANGE_LL)                                */
/*                                                                          */
/*  INPUT: NONE                                                             */
/*                                                                          */
/*  SIDE EFFECTS: NONE                                                      */
/*                                                                          */
/*  INTERNAL REFERENCES:                                                    */
/*     LOCAL DATA DEFINITION: NONE                                          */
/*                                                                          */
/*     GLOBAL DATA DEFINITIONS: NONE                                        */
/*         G                OTHER  -Structure EntryPoint which contains     */
/*                                  the processing context of the current   */
/*                                  input buffer.                           */
/*                                                                          */
/*         PRT_SPACE_FOR    @OTHER -Used for right-justification.           */
/*                                  and test for 5202 in PSM_WIDTH.         */
/*                                                                          */
/*                                                                          */
/*     ROUTINES:                                                            */
/*        PADD_SPACES_1 ()                                                  */
/*        PADD_SPACES_2 ()                                                  */
/*                                                                          */
/*  EXTERNAL REFERENCES:                                                    */
/*     ROUTINES: NONE                                                       */
/************************** END OF SPECIFICATIONS ***************************/
void InsertSpaces()
{
   if (G.LINE_LENGTH != 0) {
         /*******************************************/
         /* if there is text to be RIGHT-JUSTIFIED. */
         /*******************************************/
      if (CFG->PRT_SPACE_FOR[0] == 0)
            /****************************************************/
            /* This is the case of printers that do NOT support */
            /* horizobtal monement with fraction of a pitch.    */
            /****************************************************/
        PADD_SPACES_2();
      else
            /************************************************/
            /* This is the case of printers that DO support */
            /* horizobtal monement with fraction of a pitch.*/
            /************************************************/
       PADD_SPACES_1();
   }
}




/**************************** START OF SPECIFICATIONS ***********************/
/*  SUBROUTINE NAME: GetTheByte                                             */
/*                                                                          */
/*  DESCRIPTIVE NAME: returns a byte from a string of bytes (e.g WORD or    */
/*                    DWORD).                                               */
/*                                                                          */
/*  FUNCTION: the function is called when a byte is to be read from a       */
/*            string of bytes.                                              */
/*                                                                          */
/*  ENTRY POINT: GetTheByte                                                 */
/*                                                                          */
/*      LINKAGE: CALL (from  ProcessData)                                   */
/*                                                                          */
/*  INPUT: (AttStr, ByteNumber)                                             */
/*         AttrStr          @CHAR     -a pointer to a string of CHAR.       */
/*                                                                          */
/*         ByteNumber       WORD      -the position from which the byte     */
/*                                     is to be read.                       */
/*                                                                          */
/*  INTERNAL REFERENCES:                                                    */
/*     LOCAL DATA DEFINITION: NONE                                          */
/*     GLOBAL DATA DEFINITIONS: NONE                                        */
/*                                                                          */
/*     ROUTINES: NONE                                                       */
/*                                                                          */
/*  EXTERNAL REFERENCES:                                                    */
/*     ROUTINES: NONE                                                       */
/************************** END OF SPECIFICATIONS ***************************/
unsigned long GetTheByte ( char *AttStr, unsigned long ByteNumber )
{
   return (AttStr[(sizeof (AttStr)-1)-ByteNumber]);
}




/**************************** START OF SPECIFICATIONS ***********************/
/*  SUBROUTINE NAME: PutByte                                                */
/*                                                                          */
/*  DESCRIPTIVE NAME: writes a byte to a string of bytes (e.g WORD or       */
/*                    DWORD).                                               */
/*                                                                          */
/*  FUNCTION: the function is called when a byte is to be written to a      */
/*            string of bytes.                                              */
/*                                                                          */
/*  ENTRY POINT: PutByte                                                    */
/*                                                                          */
/*      LINKAGE: CALL (from  ProcessData)                                   */
/*                                                                          */
/*  INPUT: (AttrStr, ByteNumber, AttByte)                                   */
/*         AttrStr          @CHAR     -a pointer to a string of CHAR.       */
/*                                                                          */
/*         ByteNumber       WORD      -the position where the byte is       */
/*                                     to be written.                       */
/*                                                                          */
/*         AttByte          CHAR      -the byte is to be written.           */
/*                                                                          */
/*  INTERNAL REFERENCES:                                                    */
/*     LOCAL DATA DEFINITION: NONE                                          */
/*     GLOBAL DATA DEFINITIONS: NONE                                        */
/*                                                                          */
/*     ROUTINES: NONE                                                       */
/*                                                                          */
/*  EXTERNAL REFERENCES:                                                    */
/*     ROUTINES: NONE                                                       */
/************************** END OF SPECIFICATIONS ***************************/
void PutByte ( char *AttStr, unsigned long ByteNumber, unsigned long AttByte )
{
   (AttStr[(sizeof(AttStr)-1)-ByteNumber]) = (char)AttByte;
  /* here we have modified the function, AIX addresses the string inverted */
}




/**************************** START OF SPECIFICATIONS ***********************/
/*  SUBROUTINE NAME: ProcessData                                            */
/*                                                                          */
/*  DESCRIPTIVE NAME: called when a line is completed to process this       */
/*                    line according to the BIDI ATTRIBUTE corresponding    */
/*                    to the current input buffer.                          */
/*                                                                          */
/*  FUNCTION: the function is called whenever a line is completed.          */
/*            Within this function, the function layout_object_transform    */
/*            is called to process this line according to the current       */
/*            BIDI  ATTRIBUTE after  the  From  Attribute and the           */
/*            To Attribute passed to layout_object_transform are set to     */
/*            their correct values.                                         */
/*                                                                          */
/*  ENTRY POINT: ProcessData                                                */
/*                                                                          */
/*      LINKAGE: CALL      (from  _FLUSH_BUFFER,                            */
/*                                _PRINT_BUFF,                              */
/*                                _RIGHT_MARGIN_FOUND,                      */
/*                                _LEFT_MARGIN_FOUND,                       */
/*                                _REVERSED_LF_n,                           */
/*                                _GRAPHICS,                                */
/*                                _GRAPHICS_STAR,                           */
/*                                _DOWNLOAD_EPSON,                          */
/*                                _PRINTER_RESET,                           */
/*                                _SET_HOR_MARGINS,                         */
/*                                _REVERSE_LF,                              */
/*                                CHANGE_LL,                                */
/*                                WriteToBuffer,                            */
/*                                PSM_WriteToBuffer)                        */
/*                                                                          */
/*  INPUT: NONE                                                             */
/*                                                                          */
/*  INTERNAL REFERENCES:                                                    */
/*     LOCAL DATA DEFINITION:                                               */
/*         FromAttr         DWORD      -the attribute from which the        */
/*                                      line is to be converted.            */
/*                                                                          */
/*         ToAttr           DWORD      -the attribute to which the          */
/*                                      line is to be converted.            */
/*                                                                          */
/*     GLOBAL DATA DEFINITIONS:                                             */
/*         G                OTHER  -Structure EntryPoint which contains     */
/*                                  the processing context of the current   */
/*                                  input buffer.                           */
/*                                                                          */
/*                                                                          */
/*     ROUTINES:                                                            */
/*         layout_object_transform    -used to process a string accord.     */
/*                                     to the current BIDI ATTRIBUTE.       */
/*         CodePageTrans                                                    */
/*                                                                          */
/*  EXTERNAL REFERENCES:                                                    */
/*     ROUTINES: NONE                                                       */
/************************** END OF SPECIFICATIONS ***************************/
void ProcessData()
{
   int 			i,s;
   unsigned long        RC;
   unsigned int 	*ToInpBuf;	    /* maya 12/5/1993 */
   size_t               InpSize, OutSize;	       
   struct Buffer  	TempLineBuff[MAX_BUFFER_LENGTH];
   char 		TempLower[MAX_BUFFER_LENGTH], TempUpper[MAX_BUFFER_LENGTH];

   if (G.CurrPos != 0) {
          /*****************************************************/
          /* check if the line buffer (LineBuff) is not empty. */
          /*****************************************************/

    ToInpBuf = (unsigned int *)malloc(sizeof(unsigned int)*G.CurrPos);
    
    for (i=0; i<G.CurrPos; i++)
         G.CharLineBuff[i] = G.LineBuff[i].ch;

    OutSize = InpSize = G.CurrPos;

    RC = layout_object_transform (plh,
                      (char *) G.CharLineBuff, &InpSize, 
      		      (char *) G.CharLineBuff, &OutSize, 
	   	      NULL, ToInpBuf, NULL); 

    /* Put each character with its corresponding attributes */
    for (i=0; i<OutSize; i++)
    {
      TempLineBuff[i].ch      = G.CharLineBuff[i];
      TempLineBuff[i].attr    = G.LineBuff[ToInpBuf[i]].attr;
      TempLineBuff[i].escsize = G.LineBuff[ToInpBuf[i]].escsize;
      TempLineBuff[i].escapes = G.LineBuff[ToInpBuf[i]].escapes;
    }
    for (i=0; i<OutSize; i++)
    {
      G.LineBuff[i].ch      = TempLineBuff[i].ch;
      G.LineBuff[i].attr    = TempLineBuff[i].attr;
      G.LineBuff[i].escsize = TempLineBuff[i].escsize;
      G.LineBuff[i].escapes = TempLineBuff[i].escapes;
    }

    /* Maya  4/8/1993   Begin TASHKEEL */ 
    if (TASHKEEL)
    {
      for (i=0; i<OutSize; i++)
      {
        TempLineBuff[i].ch      = G.UpperVowelBuff[ToInpBuf[i]].ch;
        TempLineBuff[i].attr    = G.UpperVowelBuff[ToInpBuf[i]].attr;
        TempLineBuff[i].escsize = G.UpperVowelBuff[ToInpBuf[i]].escsize;
        TempLineBuff[i].escapes = G.UpperVowelBuff[ToInpBuf[i]].escapes;
      }
      for (i=0; i<OutSize; i++)
      {
        G.UpperVowelBuff[i].ch      = TempLineBuff[i].ch;
        G.UpperVowelBuff[i].attr    = TempLineBuff[i].attr;
        G.UpperVowelBuff[i].escsize = TempLineBuff[i].escsize;
        G.UpperVowelBuff[i].escapes = TempLineBuff[i].escapes;
      }
      for (i=0; i<OutSize; i++)
      {
        TempLineBuff[i].ch      = G.LowerVowelBuff[ToInpBuf[i]].ch;
        TempLineBuff[i].attr    = G.LowerVowelBuff[ToInpBuf[i]].attr;
        TempLineBuff[i].escsize = G.LowerVowelBuff[ToInpBuf[i]].escsize;
        TempLineBuff[i].escapes = G.LowerVowelBuff[ToInpBuf[i]].escapes;
      }
      for (i=0; i<OutSize; i++)
      {
        G.LowerVowelBuff[i].ch      = TempLineBuff[i].ch;
        G.LowerVowelBuff[i].attr    = TempLineBuff[i].attr;
        G.LowerVowelBuff[i].escsize = TempLineBuff[i].escsize;
        G.LowerVowelBuff[i].escapes = TempLineBuff[i].escapes;
      }
    } /* End Tashkeel */
    free(ToInpBuf);

    /* maya 28/2/1993 */
    /* if (RC != 0) 
        TRACE("/aix3.2/maya/out", "Ret Code from layout_object_transform = %d\n", RC); */

         /****************************************************/
         /* call layout_object_transform to process the line */
         /* buffer according to the FromAttr and the ToAttr. */
         /****************************************************/


    if (BDI->PrtCodePage == CP864)
                /**********************************************/
                /* in case code page on printer is 864 rather */
                /* than 1046, then translation from 1046 to   */
                /* 864 is first needed before sending data to */
                /* the printer                                */
                /**********************************************/
	{
	   /*if (BDI->CFG == _4019)
  		Yasmin = 0;
	   else
		Yasmin = 1;
           CodePageTrans ( G.LineBuff, G.LineBuff,
                           G.CurrPos*(sizeof (G.LineBuff[0])),
                           sizeof (G.LineBuff[0]), 0x30000100, Yasmin);*/
	}
   }
}

/**************************** START OF SPECIFICATIONS ***********************/
/*  SUBROUTINE NAME: PostProcessTashkeel                                    */
/*                                                                          */
/*  DESCRIPTIVE NAME: used to call the PostProcess function (below) which   */
/*                    converts the presentation status (PS), associated     */
/*                    with characters to be printed, into proper ESCape     */
/*                    sequences or Control Codes.                           */
/*                                                                          */
/*  FUNCTION: To process the TASHKEEL buffers in addition to the Consonants */
/*            buffer by calling PostProcess() three times:		    */
/*            First time : The upper vowel buffer is processed.             */
/*            Second time: The Consonant buffer is processed.               */
/*            Third time : The Lower Vowel buffer is processed.             */
/*                                                                          */
/*  ENTRY POINT: PostProcessTashkeel                                        */
/*      LINKAGE: CALL (from NewLine,                                        */
/*                          _HT_FOUND,                                      */
/*                          FlushLine)                                      */
/*                                                                          */
/*  INPUT: NONE                                                             */
/*  SIDE EFFECTS: NONE                                                      */
/*                                                                          */
/*  GLOBAL DATA DEFINITIONS:                                                */
/*         G                OTHER  -Structure EntryPoint which contains     */
/*                                  the processing context of the current   */
/*                                  input buffer.                           */
/*                                                                          */
/*  ROUTINES:                                                               */
/*        AccProcessLine             -called to write a number of           */
/*                                    characters to the output buffer.      */
/*        AccProcessChar             -called to write a character to        */
/*                                    the output buffer.   		    */
/*                                                                          */
/*  EXTERNAL REFERENCES:                                                    */
/*     ROUTINES: NONE                                                       */
/************************** END OF SPECIFICATIONS ***************************/
void PostProcessTashkeel()
{
  unsigned int i, j;

  if (G.CurrPos != 0) 
  {
    AccProcessLine(3, 1, UPPER_ESC);     /* ESC, 74L, 4L */
    PostProcess(G.UpperVowelBuff);
    AccProcessChar((WCHAR) 13L);

    AccProcessLine(3, 1, CONSONANT_ESC); /* ESC, 74L, 10L */

    /*********************************************************/
    /* In case G.ORIENTATION = LTR, G.INSERT_SPACES_START    */
    /* G.INSERT_SPACES_END will be assigned to zero.         */
    /*********************************************************/
  
    for (i=G.INSERT_SPACES_START; i<G.INSERT_SPACES_END; i++)
         AccProcessChar((long)bidi_out_buff [i]);
    PostProcess(G.LineBuff);
    AccProcessChar((WCHAR) 13L); 
  
    AccProcessLine(3, 1, LOWER_ESC);    /* ESC, 74L, 9L */
  
    /*********************************************************/
    /* In case G.ORIENTATION = LTR, G.INSERT_SPACES_START    */
    /* G.INSERT_SPACES_END will be assigned to zero.         */
    /*********************************************************/
  
    for (i=G.INSERT_SPACES_START; i<G.INSERT_SPACES_END; i++)
         AccProcessChar((long)bidi_out_buff [i]);
  
    PostProcess(G.LowerVowelBuff);
  }
}


/**************************** START OF SPECIFICATIONS ***********************/
/*  SUBROUTINE NAME: PostProcess                                            */
/*                                                                          */
/*  DESCRIPTIVE NAME: used to convert the presentation status (PS),         */
/*                    associated with characters to be printed, into        */
/*                    proper ESCape sequences or Control Codes.             */
/*                                                                          */
/*  FUNCTION: when a line is completed and after it has been processed      */
/*            according to the BIDI attributes, this function is called     */
/*            to convert the presentation status, associated with the       */
/*            characters to be printed, into the proper ESCape sequences    */
/*            or Control codes.                                             */
/*            Sequences and Codes are only inserted when a change           */
/*            between the PS of the the previously printed character        */
/*            and that of the current character to be printed occur.        */
/*            This change can either indicate turning on one or more        */
/*            features or turning off one or more features.                 */
/*            In case of turnning on a feature the sequence is fetched      */
/*            from the CFG->CODE_ON_TAB with an index equal to the bit      */
/*            number corresponding to this feature.                         */
/*            In case of turnning off a feature the sequence is fetched     */
/*            from the CFG->CODE_OFF_TAB with an index equal to the bit     */
/*            number corresponding to this feature.                         */
/*            Characters and sequences to be printed are put into an        */
/*            output buffer to be sent to the printer.                      */
/*                                                                          */
/*  ENTRY POINT: PostProcess                                                */
/*      LINKAGE: CALL (from NewLine,                                        */
/*                          _HT_FOUND,                                      */
/*                          FlushLine)                                      */
/*                                                                          */
/*  INPUT: NONE                                                             */
/*  SIDE EFFECTS: NONE                                                      */
/*                                                                          */
/*  INTERNAL REFERENCES:                                                    */
/*     LOCAL DATA DEFINITION:                                               */
/*         TempBuff         DWORD      -a variable in which the bits set    */
/*                                      to one are the bits which have      */
/*                                      changed when we compair the         */
/*                                      previous character attribute        */
/*                                      (PREV_CHAR_ATTR) and the            */
/*                                      attribute of the current to be      */
/*                                      printed (CURR_CHAR_ATTR).           */
/*                                                                          */
/*         k                CHAR       -dummy counter used to loop on       */
/*                                      the line buffer (LineBuff).         */
/*                                                                          */
/*         i                CHAR       -dummy counter used to loop on       */
/*                                      the bits of the temporary buffer    */
/*                                      (TempBuff).                         */
/*                                                                          */
/*         index            CHAR       -a variable used to indicate the     */
/*                                      position in which a difference      */
/*                                      in bits between the PS of the       */
/*                                      previously printed character        */
/*                                      (PREV_CHAR_ATTR) and the PS of      */
/*                                      the current to be printed           */
/*                                      (CURR_CHAR_ATTR) occured.           */
/*                                                                          */
/*         PRT_END_ATTR     DWORD                                           */
/*                                                                          */
/*                                                                          */
/*         start            CHAR       -an index to the line buffer         */
/*                                      (LineBuff) indicating the           */
/*                                      beginning of the characters to      */
/*                                      be written to the output buffer.    */
/*                                                                          */
/*         count            CHAR       -a count of the number of            */
/*                                      characters with same attributes.    */
/*                                      these characters are written to     */
/*                                      the output buffer together.         */
/*                                                                          */
/*     GLOBAL DATA DEFINITIONS:                                             */
/*         G                OTHER  -Structure EntryPoint which contains     */
/*                                  the processing context of the current   */
/*                                  input buffer.                           */
/*                                                                          */
/*         PRT_END_ATTR_1   DWORD      -Contains features that are          */
/*                                      supported (remain on)  between      */
/*                                      lines.    ( HI-WORD)                */
/*                                                                          */
/*         PRT_END_ATTR_2   DWORD      -Contains features that are          */
/*                                      supported (remain on)  between      */
/*                                      lines.    ( LO-WORD)                */
/*     ROUTINES:                                                            */
/*          AccProcessLine             -called to write a number of         */
/*                                      characters to the output buffer.    */
/*                                                                          */
/*  EXTERNAL REFERENCES:                                                    */
/*     ROUTINES: NONE                                                       */
/************************** END OF SPECIFICATIONS ***************************/
void PostProcess(struct Buffer *ProcessBuff)
{

int j;


unsigned long  PRT_END_ATTR;
unsigned long  TEMP_BUFF;
int             k,i,index,count,start;

int l;

PRT_END_ATTR = (CFG->PRT_END_ATTR_1 * 0x10000L) + CFG->PRT_END_ATTR_2;

TEMP_BUFF = 0;

if (G.CurrPos !=0) {
           /************************************************************/
           /* check to see if the line buffer (ProcessBuff) is not empty. */
           /************************************************************/

   if ( (HCP != 1) && (CFG->TAIL_FLAG == 0x20 ) ){
         /*********************************************************/
         /* if not hebrew code page and TAIL is to be eliminated. */
         /*********************************************************/
      for(k=0; k < G.CurrPos; k++) {
         if (ProcessBuff[k].ch == 0x9F)
              /***********************************************************/
              /* if ProcessBuff[k] is a tail convert it to a NULL character */
              /***********************************************************/
            ProcessBuff[k].ch = 0x00;
      }
   }
   count = 0;
   start = 0;
           /***********************************************************/
           /* initialise start to point to the first character in the */
           /* line buffer (LineBuff).                                 */
           /***********************************************************/
   for(k=0; k <= G.CurrPos; k++) {                       /* open FOR loop */
              /****************************************************************/
              /* scan ProcessBuff starting from the first element (k=0) to the */
              /* last element (G.CurrPos-1).                                  */
              /****************************************************************/




/* check for escapes to be sent before checking differences in the attrs */
if (k>0)
  {
   if (ProcessBuff [k].escsize > 0 && ProcessBuff [k-1].escsize > 0)
     {
      if ((memcmp (ProcessBuff [k-1].escapes, ProcessBuff [k].escapes,
                        ProcessBuff [k].escsize) != 0) /*&&
                (((ProcessBuff [k].attr & 0x00008000) >> 15) == 1)*/)
        {
         if (ProcessBuff [k].escapes != NULL)
           {
	    AccProcessLine( (WCHAR) count
                             ,(WCHAR) sizeof(ProcessBuff[start])
                             ,(char*) &ProcessBuff[start].ch);
       	    count = 0;
   	    if (((ProcessBuff [k].attr & 0x00002000) >> 13) != 1)
      	      start = (k == G.CurrPos) ? 0 : k;
   	    else
              start = (k == G.CurrPos) ? 0 : k+1;

            AccProcessLine (ProcessBuff [k].escsize, 1, ProcessBuff [k].escapes);
           }
        }
     }
   else
     if (ProcessBuff [k].escsize > 0)
       {
         AccProcessLine( (WCHAR) count
                             ,(WCHAR) sizeof(ProcessBuff[start])
                             ,(char*) &ProcessBuff[start].ch);
         count = 0;
   	 if (((ProcessBuff [k].attr & 0x00002000) >> 13) != 1)
           start = (k == G.CurrPos) ? 0 : k;
   	 else
           start = (k == G.CurrPos) ? 0 : k+1;

         AccProcessLine (ProcessBuff [k].escsize, 1, ProcessBuff [k].escapes);
        }
  }
else
  if (ProcessBuff[k].escapes != NULL)
  /*     IRENE         && ((ProcessBuff [k].attr & 0x00008000) >> 15))*/
    {
      AccProcessLine( (WCHAR) count
                             ,(WCHAR) sizeof(ProcessBuff[start])
                             ,(char*) &ProcessBuff[start].ch);
      count = 0;
      if (((ProcessBuff [k].attr & 0x00002000) >> 13) != 1)
        start = (k == G.CurrPos) ? 0 : k;
      else
        start = (k == G.CurrPos) ? 0 : k+1;

      AccProcessLine (ProcessBuff [0].escsize, 1, ProcessBuff [0].escapes);
    }


	/* space character holding graphics escape sequence */
   if (((ProcessBuff [k].attr & 0x00002000) >> 13) != 1)
     { 

      CURR_CHAR_ATTR = (k == G.CurrPos) ? (G.PREV_CHAR_ATTR & (PRT_END_ATTR) ):
                                         ProcessBuff[k].attr;

      switch ((unsigned int) CURR_CHAR_ATTR & 0x00000001) {
         case 0x0001:
                 /************************************************/
                 /* this is the case were bit 0 is equal to one  */
                 /* indicating that the following character is   */
                 /* to be printed as graphics.                   */
                 /************************************************/


           AccProcessLine( (WCHAR) count
                          ,(WCHAR) sizeof(ProcessBuff[start])
                          ,(char*) &ProcessBuff[start].ch);
                   /*********************************************************/
                   /* call AccProcessLine to write all previous characters  */
                   /* of same attribute (their number = count) to the       */
                   /* output buffer.                                        */
                   /*********************************************************/
           count = 0;
                   /*******************************************************/
                   /* initialise count to indicate that all characters of */
                   /* the same attribute are written to the output buffer.*/
                   /*******************************************************/
           start = k+1;
                /***********************************************************/
                /* initialise start to point to the first character to be  */
                /* printed (written to the output buffer).                 */
                /***********************************************************/

           AccProcessLine( (WCHAR) CFG->CODE_ON_TAB[31].length
                           ,(WCHAR) 1
                           ,(char*) CFG->CODE_ON_TAB[31].sequence);
                   /***********************************************************/
                   /* call AccProcessLine to write the ESCape sequence which  */
                   /* tells the printer that the following character is to be */
                   /* printed as graphics.                                    */
                   /***********************************************************/
           AccProcessChar( (WCHAR) ProcessBuff[k].ch);
                   /***********************************************************/
                   /* call AccProcessChar to write the character to be printed*/
                   /* as graphics to the output buffer.                       */
                   /***********************************************************/

         default:
           TEMP_BUFF=(CURR_CHAR_ATTR ^ G.PREV_CHAR_ATTR);
                   /***********************************************************/
                   /* Xoring the current character attribute (CURR_CHAR_ATTR) */
                   /* and the previous character attribute (G.PREV_CHAR_ATTR  */
                   /* indicates whether they are the same or not.In case they */
                   /* are different TempBuff will have some bits equal to     */
                   /* ones. These bits are in the positions where the         */
                   /* difference between both attributes occur.               */
                   /***********************************************************/
           if ((TEMP_BUFF == 0) || (TEMP_BUFF == 1)){
                     /*******************************************************/
                     /* this is the case when CURR_CHAR_ATTR is the same as */
                     /* G.PREV_CHAR_ATTR (TEMP_BUFF = 0) or the difference is */
                     /* in bit zero (TEMP_BUFF) = 1). in this case no       */
                     /* ESCape sequence is passed to the printer.           */
                     /*******************************************************/

              if (((unsigned int) CURR_CHAR_ATTR & 0x00000001) != 1){
                 if (k < G.CurrPos)
                    count++;
                     /*******************************************************/
                     /* this is the case when CURR_CHAR_ATTR is the same as */
                     /* G.PREV_CHAR_ATTR. count is increment to indicate that */
                     /* another character of the same attribute as the      */
                     /* previous attribute occured.                         */
                     /*******************************************************/

              }
           }
           else{
                     /*************************************************/
                     /* this is the case where the attribute of the   */
                     /* previously printed character (G.PREV_CHAR_ATTR) */
                     /* and the attribute of the current character to */
                     /* be printed (CURR_CHAR_ATTR) are not the same. */
                     /*************************************************/
              AccProcessLine( (WCHAR) count
                             ,(WCHAR) sizeof(ProcessBuff[start])
                             ,(char*) &ProcessBuff[start].ch);
                     /*******************************************************/
                     /* call AccProcessLine to write the characters of same */
                     /* attribute to the output buffer.                     */
                     /*******************************************************/
              count = 0;
                     /*******************************************************/
                     /* initialise count to indicate that all characters of */
                     /* the same attribute are written to the output buffer.*/
                     /*******************************************************/

              start = (k == G.CurrPos) ? 0 : k;
                  /***********************************************************/
                  /* initialise start to point to the first character to be  */
                  /* printed (written to the output buffer).                 */
                  /***********************************************************/

              index = 0;
              for (i=1; i<EXP_ATTR_LENGTH; i++) {
                      /*******************************************************/
                      /* here we will start to loop on the bits of TempBuff  */
                      /* to determine which bit is to be changed. The bit in */
                      /* TempBuff equal to one indicates that a change in    */
                      /* the printers feature corresponding to this bit has  */
                      /* occured.                                            */
                      /* Note that we will start testing on bit 31 and end at*/
                      /* bit 1 since bit 0 has already been tested.          */
                      /*******************************************************/


                 if ((TEMP_BUFF & 0x80000000) == 0){} /*no change in this bit*/
                               /***********************************************/
                               /* test if the last bit in Temp_Buff is a one. */
                               /* if it is equal to one then index indicates  */
                               /* the number of the bit starting from the     */
                               /* LEFT in which a change occured.             */
                               /* We must then determine whether this change  */
                               /* turns on or off a printer feature by testing*/
                               /* the bit number index from the LEFT in       */
                               /* CURR_CHAR_ATTR. A zero bit indicates that   */
                               /* this feature is to be turned off and a one  */
                               /* bit indicates that this feature is to be    */
                               /* turrned on.                                 */
                               /***********************************************/
                 else{
                               /*******************************************/
                               /* this is the case where the last bit in  */
                               /* TEMP_BUFF is equal to one. (i.e a change*/
                               /* in this bit occured)                    */
                               /*******************************************/
                    if ((CURR_CHAR_ATTR &(0x80000000 >> index))==0) {
                                   /*turn off attribute*/
                                /*******************************************/
                                /* anding CURR_CHAR_ATTR with 0x800000000  */
                                /* shifted to the right index times yields */
                                /* zero if the bit number index from the   */
                                /* LEFT in CURR_CHAR_ATTR is a zero and    */
                                /* yields nonzero value if this bit is a   */
                                /* one.                                    */
                                /*******************************************/
                                /*******************************************/
                                /* this is the case were an ESCape sequence*/
                                /* that turns off a printer's feature is to*/
                                /* be embeded.                             */
                                /*******************************************/

                       if (index!=31 && index!=17) 
			{
			 if (index == 16)
				/**********************/
				/* if font is slected */
				/**********************/
			  {
			   if (!((CURR_CHAR_ATTR & 0x02000000) || 
				(CURR_CHAR_ATTR & 0x01000000)))
				/*****************************************/
				/* if not 17/12 cpi then to turn off the */
				/* current selected font, by turning on  */
				/* 10 cpi 				 */
				/*****************************************/
			  	AccProcessChar (18); 
			  }
			else
{
/* send escape to turn off 17cpi and 12 cpi only if no font is selected */
if ((index != 7 && index !=6) 
	|| ((CURR_CHAR_ATTR & 0x00008000) != 0x00008000))  /* IRENE */
                          AccProcessLine((WCHAR)CFG->CODE_OFF_TAB[index].length,
                                        (WCHAR) 1,
                                        (char*) 
				 	CFG->CODE_OFF_TAB[index].sequence);
}
                                   /*******************************************/
                                   /* call AccProcessLine to write the ESCape */
                                   /* sequence found in the CODE_OFF_TAB,     */
                                   /* with an index equal to index, to the    */
                                   /* output buffer.                          */
                                   /*******************************************/
                       }                                         /* endif */

                          if ((index == 6) &&
                             (CURR_CHAR_ATTR & (0x80000000 >> (index+1))) != 0)
                                  /*******************************************/
                                  /* call AccProcessLine to write the ESCape */
                                  /* sequence found in the CODE_OFF_TAB,     */
                                  /* with an index equal to index, to the    */
                                  /* output buffer.                          */
                                  /*******************************************/
{
                             AccProcessLine(
                                        (WCHAR) CFG->CODE_ON_TAB[index+1].length
                                       ,(WCHAR) 1
                                       ,(char*) CFG->CODE_ON_TAB
                                                    [index+1].sequence);
}

                          if ((index==7) &&
                             (CURR_CHAR_ATTR & (0x80000000 >> (index-1))) != 0)
                                  /*******************************************/
                                  /* if 12 CPI feature (index == 7) was      */
                                  /* turrned off, check to see if 17 CPI     */
                                  /* (index == 6) is currently ON or not.    */
                                  /* if yes then send the sequence to turn   */
                                  /* it ON again since ending 12 CPI ends    */
                                  /* 17 CPI also.                            */
                                  /*******************************************/
                          {
                             AccProcessLine(
                               (WCHAR) CFG->CODE_ON_TAB[index-1].length
                               ,(WCHAR) 1
                               ,(char*) CFG->CODE_ON_TAB[index-1].sequence);
                          }
                    }
                    else{
                          /*******************************************/
                          /* this is the case were an ESCape sequence*/
                          /* that turns on a printer's feature is to */
                          /* be embeded.                             */
                          /*******************************************/

                       if (index != 31 && index != 17){
			if (index != 16)
				/********************************************/
				/* if no font selected then turn on printer */
				/* feature using CFG->CODE_OFF_TABLE        */
				/********************************************/
                          AccProcessLine((WCHAR)CFG->CODE_ON_TAB[index].length
                                        ,(WCHAR) 1
                                     ,(char*) CFG->CODE_ON_TAB[index].sequence);
                                   /*******************************************/
                                   /* call AccProcessLine to write the ESCape */
                                   /* sequence found in the CODE_ON_TAB, with */
                                   /* an index equal to index, to the         */
                                   /* output buffer.                          */
                                   /*******************************************/
                       }                                           /* endif */

                       if (index==3){
                                    /******************************************/
                                    /* these are two special cases.           */
                                    /* Turnning on Subscript (index3) emplies */
                                    /* that superscript (index4) is turned off*/
                                    /* Therefore we must skip testing on bit  */
                                    /* number 4 from the LEFT (index4) so as  */
                                    /* not to turn off Subscript which will   */
                                    /* imply turnning off Superscript.        */
                                    /* Turnning on 17 CPI (index6) emplise    */
                                    /* that 12 CPI (index7) is turnned off.   */
                                    /* Therefore we must skip testing on bit  */
                                    /* number 7 from the LEFT (index7) so as  */
                                    /* not to turn off 12 CPI which will imply*/
                                    /* turnning off 12 CPI.                   */
                                    /******************************************/

                          TEMP_BUFF = TEMP_BUFF << 1;
                                    /******************************************/
                                    /* shift TempBuff to the left once to skip*/
                                    /* test on the next bit.                  */
                                    /******************************************/
                          ++i;
                          ++index;
                       }                                              /*end If*/
                    }                                                /* endIF */
                 }                                                   /* endIF */
              index++;
              TEMP_BUFF = TEMP_BUFF << 1;
                         /******************************************/
                         /* shift TempBuff to the left once to skip*/
                         /* test on the next bit.                  */
                         /******************************************/
              }                                                /* endfor */

              if ((CURR_CHAR_ATTR & PRT_PSM_ATTR) != 0
		&& (CURR_CHAR_ATTR & 0x8000) == 0){    
                /******************************************/
		/* case of PSM and no font selected, send */
		/* escape to select PSM font              */
                /******************************************/
                 AccProcessLine( (WCHAR) CFG->CODE_ON_TAB[31-16].length
                                ,(WCHAR) 1
                                ,(char*) CFG->CODE_ON_TAB[31-16].sequence);
                            /*******************************************/
                            /* call AccProcessLine to write the ESCape */
                            /* sequence found in the CODE_ON_TAB, with */
                            /* an index equal to 16, to the            */
                            /* output buffer.                          */
                            /*******************************************/
              }
              if (k < G.CurrPos)
                 count++;
              break;
          }                                                      /* endif */
      }                                                          /* endswitch */
      G.PREV_CHAR_ATTR = CURR_CHAR_ATTR;

    }
   }                                                    /*close  for FOR loop*/


	/* loop to free allocated memory for escapes */
   for(k=0; k <= G.CurrPos; k++) {                       /* open FOR loop */
	if (ProcessBuff [k].escsize > 0) free (ProcessBuff [k].escapes);
	}


 if (count != 0){
             /*********************************************************/
             /* check to see if there are characters to be written to */
             /* the output buffer.                                    */
             /*********************************************************/
             AccProcessLine( (WCHAR) count
                            ,(WCHAR) sizeof(ProcessBuff[start])
                            ,(char*) &ProcessBuff[start].ch);
                   /***********************************************************/
                   /* call AccProcessLine to write the characters of same     */
                   /* attribute to the output buffer.                         */
                   /***********************************************************/
    }                                                                 /*end if*/
  }
}                                                           /*end of procedure*/


/**************************** START OF SPECIFICATIONS ***********************/
/*  SUBROUTINE NAME: AdjustWidth                                            */
/*                                                                          */
/*  DESCRIPTIVE NAME:  It deals with the printer special (minimun) width    */
/*                                                                          */
/*  FUNCTION: This fuction determines the character width according to      */
/*            the attributes and the printer minimum width                  */
/*                                                                          */
/*  ENTRY POINT: AdjustWidth                                                */
/*                                                                          */
/*      LINKAGE: CALL (from  PADD_SPACES_1, UpdatePresStatus)               */
/*                                                                          */
/*  INPUT: ( newpitch, attr )                                               */
/*         newpitch        CHAR                                             */
/*         attr            CHAR                                             */
/*                                                                          */
/*  SIDE EFFECTS: NONE                                                      */
/*                                                                          */
/*  INTERNAL REFERENCES:                                                    */
/*     LOCAL DATA DEFINITION: NONE                                          */
/*                                                                          */
/*     GLOBAL DATA DEFINITIONS:                                             */
/*      CFG->PRT_D_PITCH    WORD    - Default Pitch. (10,12,17..)           */
/*                                                                          */
/*      CFG->PRT_SPEC_PITCH WORD    - Character Width in APELS in case      */
/*                                    of special pitch 17.125 .             */
/*                                                                          */
/*                                                                          */
/*     ROUTINES: NONE                                                       */
/*                                                                          */
/*  EXTERNAL REFERENCES:                                                    */
/*     ROUTINES: NONE                                                       */
/************************** END OF SPECIFICATIONS ***************************/
void AdjustWidth(unsigned long newpitch, unsigned long attr)
{
          G.PITCH = (char) newpitch;
          if (G.CWIDTH == CFG->PRT_SPEC_PITCH)
                   /********************************************************/
                   /* check to see if the current character width is equal */
                   /* to the printer's special pitch width (119).          */
                   /********************************************************/
             ++G.CWIDTH;
                   /**********************************************************/
                   /* in this case G.CWIDTH = 119 which corresponds to pitch */
                   /* 17.125 and not 17. Therefore we increment G.CWIDTH to  */
                   /* be equal to 120 which is the width corresponding to    */
                   /* pitch 17.                                              */
                   /**********************************************************/

          if (G.CWIDTH == 102)
		/***********************************************************/
		/*							   */
		/*							   */
		/***********************************************************/
             G.CWIDTH = 100;

          if ((attr & 0x80) == 0)
                   /**********************************************************/
                   /* if the feature changing the width is to be turned off. */
                   /**********************************************************/
            G.CWIDTH = (G.CWIDTH * (unsigned int)newpitch) / CFG->PRT_D_PITCH;
          else
            G.CWIDTH = (G.CWIDTH * CFG->PRT_D_PITCH) / (unsigned int) newpitch;

          if ((G.CWIDTH-1) == CFG->PRT_SPEC_PITCH)
                   /********************************************************/
                   /* check to see if the current character width is equal */
                   /* to the printer's special pitch width (119).          */
                   /********************************************************/
             --G.CWIDTH;
                   /**********************************************************/
                   /* in this case G.CWIDTH = 120 which corresponds to pitch */
                   /* 17. Therefore we decrement G.CWIDTH to be equal to 119 */
                   /* which is the width corresponding to that special       */
                   /* pitch.                                                 */
                   /**********************************************************/
          if (G.CWIDTH == 100)
            G.CWIDTH = 102;

}




/**************************** START OF SPECIFICATIONS ***********************/
/*  SUBROUTINE NAME: UpdatePresStatus                                       */
/*                                                                          */
/*  DESCRIPTIVE NAME: UpdatePresStatus updates the presentation status      */
/*                    (PRESENTION_STATUS) describing the printers           */
/*                    printing mode whenever a code changing this mode      */
/*                    (attribute type code) occur.                          */
/*                                                                          */
/*  FUNCTION: whenever a code changing the printers printing mode occur     */
/*            (e.g begin DOUBLE STRIKE , end SUBSCRIPT...) the function     */
/*            is called to update the presentation status to reflect        */
/*            this change.                                                  */
/*            Attribute codes can either:                                   */
/*            -Turn on a printer feature (e.g begin UNDERSCORE)             */
/*                 in this case the attribute associated with this code     */
/*                 is in the form 100XXXXX where XXXXX is a count           */
/*                 indicating the bit number corresponding to this          */
/*                 feature. This bit in the presentation status             */
/*                 (PRESENTION_STATUS) is then turned on to indicate        */
/*                 that this feature is now on.                             */
/*                                                                          */
/*            -Turn off a printer feature (e.g end DOUBLE STRIKE)           */
/*                 in this case the attribute associated with this code     */
/*                 is in the form 000XXXXX where XXXXX is a count           */
/*                 indicating the bit number corresponding to this          */
/*                 feature. This bit in the presentation status             */
/*                 (PRESENTION_STATUS) is then turned off to indicate       */
/*                 that this feature is now off.                            */
/*                                                                          */
/*            -Turn on a feature changing the characters width              */
/*                 (e.g begin CONDENSED mode)                               */
/*                 in this case the attribute associated with this code     */
/*                 is in the form 110XXXXX where XXXXX is an index          */
/*                 to a table( WIDTH_TABLE) from which the new attribute    */
/*                 and the new pitch are determined. The new attribute      */
/*                 is in the form 000XXXXX where XXXXX is the bit number    */
/*                 corresponding to this feature. This bit in the attri.    */
/*                 buffer (PRESENTION_STATUS) is then turned on to          */
/*                 indicate that this feature is now on.                    */
/*                 the new pitch is used to determine the characters new    */
/*                 width (PRT_CWIDTH).                                      */
/*                                                                          */
/*            -Turn off a feature changing the characters width             */
/*                 (e.g end DOUBLE WIDTH)                                   */
/*                 in this case the attribute associated with this code     */
/*                 is in the form 010XXXXX where XXXXX is an index          */
/*                 to a table( WIDTH_TABLE) from which the new attribute    */
/*                 and the new pitch are determined. The new attribute      */
/*                 is in the form 000XXXXX where XXXXX is the bit number    */
/*                 corresponding to this feature. This bit in the attri.    */
/*                 buffer (PRESENTION_STATUS) is then turned off to         */
/*                 indicate that this feature is now off.                   */
/*                 the new pitch is used to determine the characters new    */
/*                 width (PRT_CWIDTH).                                      */
/*                                                                          */
/*            -Special codes on                                             */
/*                 these codes are codes turnning on more than one non      */
/*                             features of the printer.                     */
/*                 in this case the attribute associated with this code     */
/*                 is in the form 101XXXXX where XXXXX is an index          */
/*                 to a table (SPECIAL_TABLE) from which the new attrib.    */
/*                 (in this case a 16 bit attribute) is fetched. this       */
/*                 attribute contains the bits to be manipulated.           */
/*                                                                          */
/*            -Special codes off(e.g ESC T  ends both SUB & SUPERSCRIPT)    */
/*                 these codes are codes turnning off more than one         */
/*                 feature of the printer.                                  */
/*                 in this case the attribute associated with this code     */
/*                 is in the form 001XXXXX where XXXXX is an index          */
/*                 to a table (SPECIAL_TABLE) from which the new attrib.    */
/*                 (in this case a 16 bit attribute) is fetched. this       */
/*                 attribute contains the bits to be manipulated.           */
/*                                                                          */
/*  ENTRY POINT: UpdatePresStatus                                           */
/*      LINKAGE: CALL (from Processor)                                      */
/*                                                                          */
/*  INPUT: ( @PRESENTION_STATUS, Attr)                                      */
/*         PRESENTION_STATUS  DWORD    -the current attribute defining      */
/*                                      the printers current status         */
/*                                      (i.e the mode in which the          */
/*                                      character is to be printed).        */
/*                                                                          */
/*         Attr               CHAR     -the attribute associated with       */
/*                                      the ESCape sequence or with the     */
/*                                      Control code.                       */
/*                                                                          */
/*  SIDE EFFECTS: NONE                                                      */
/*                                                                          */
/*  INTERNAL REFERENCES:                                                    */
/*     LOCAL DATA DEFINITION:                                               */
/*                                                                          */
/*         pitch              CHAR     -the new pitch from which the        */
/*                                      character's width is calculated.    */
/*                                                                          */
/*         TempBuff           DWORD    -a dummy variable.                   */
/*                                                                          */
/*         TimesToShift       CHAR     -a dummy variable its value is       */
/*                                      the number of the bit we want       */
/*                                      to change.                          */
/*                                                                          */
/*         tempattr           CHAR     -a dummy variable indexing in the    */
/*                                      WIDTH_TABLE.                        */
/*                                                                          */
/*     GLOBAL DATA DEFINITIONS:                                             */
/*         G                OTHER  -Structure EntryPoint which contains     */
/*                                  the processing context of the current   */
/*                                  input buffer.                           */
/*                                                                          */
/*         SPECIAL_TABLE      @OTHER   -pointer to the special table        */
/*                                                                          */
/*         WIDTH_TABLE        @OTHER   -pointer to the width table          */
/*                                                                          */
/*         EXPANSION_CODE     WORD                                          */
/*                                                                          */
/*     ROUTINES:                                                            */
/*          AdjustWidth                                                     */
/*                                                                          */
/*  EXTERNAL REFERENCES:                                                    */
/*     ROUTINES: NONE                                                       */
/************************** END OF SPECIFICATIONS ***************************/
void UpdatePresStatus(unsigned long *PRESENTION_STATUS, unsigned long attr)
{
unsigned long   Temp_Buff;
unsigned char   pitch, tempattr;
int             TimesToShift;
            switch ((unsigned int)attr & 0xE0) {             /* begin switch */
                 /*************************************************************/
                 /* anding the attribute (attr) associated with the code with */
                 /* 0x0E (11100000B) indicates whether this code:             */
                 /*                            -Turns on a printer feature,   */
                 /*                            -Turns off a printer feature,  */
                 /*                            -Turns on a feature changing   */
                 /*                             the characters width,         */
                 /*                            -Turns off a feature changing  */
                 /*                             the characters width,         */
                 /*                             or                            */
                 /*                            -is a Special code.            */
                 /*************************************************************/

            case TURN_OFF_ATTR:
               TimesToShift = (int)attr;
                 /*********************************************/
                 /* the attr=000XXXXXB therefore              */
                 /* TimesToShift=000XXXXXB which is the number*/
                 /* of the bit we want to turn off.           */
                 /*********************************************/

               Temp_Buff = ~(((long) 1) << TimesToShift);
                 /***********************************************************/
                 /* shift a one to the left number of times equal to the    */
                 /* number of the bit we want to turn off. Now TempBuff has */
                 /* all bits equal zero except the bit corresponding to the */
                 /* bit we want to change in PRESENTION_STATUS which is     */
                 /* equal to one.                                           */
                 /***********************************************************/

               *PRESENTION_STATUS &= Temp_Buff;
                 /*********************************************************/
                 /* anding TempBuff and PRESENTION_STATUS keeps all bits  */
                 /* PRESENTION_STATUS unchanged except the bit we want to */
                 /* change (bit TimesToShift) which is changed to a zero. */
                 /*********************************************************/

               break;

            case TURN_ON_ATTR:
               TimesToShift = (int)attr - 128;
                 /*********************************************/
                 /* since the attr=100XXXXXB therefore        */
                 /* TimesToShift=000XXXXXB which is the       */
                 /* the number of the bit  we want to turn on.*/
                 /*********************************************/

               Temp_Buff = ((long) 1) << TimesToShift;
                 /***********************************************************/
                 /* shift a one to the left number of times equal the number*/
                 /* of the bit we want to turn on. Now TempBuff has all bits*/
                 /* equal zero except the bit corresponding to the bit we   */
                 /* want to change in PRESENTION_STATUS which is equal to   */
                 /* one.                                                    */
                 /***********************************************************/

               *PRESENTION_STATUS |= Temp_Buff;
                 /**********************************************************/
                 /* oring TempBuff and PRESENTION_STATUS keeps all bits in */
                 /* PRESENTION_STATUS unchanged except the bit we want to  */
                 /* change (bit TimesToShift) which is changed to one.     */
                 /**********************************************************/
               break;

            case SPECIAL_TAB:
               Temp_Buff = ~(CFG->SPECIAL_TABLE[attr - 0x20]);
               *PRESENTION_STATUS &= Temp_Buff;
               break;

            case WIDTH_TAB_ON:
            case WIDTH_TAB_OFF:
               tempattr = ((unsigned char)attr & 0x1F);
               pitch    = CFG->WIDTH_TABLE[tempattr].pitch;
                 /*******************************************/
                 /* get the new pitch from the width table. */
                 /*******************************************/

               if (CFG->EXPANSION_CODE == 0x01)
                  attr = CFG->WIDTH_TABLE[tempattr].attr;
               else
                  attr = ((CFG->WIDTH_TABLE[tempattr].attr & 0x1F)
                                        | (attr & 0x80));

               TimesToShift = ((int)attr & 0x1F);

               if ((attr & 0x80) == 0) {
                  if ((*PRESENTION_STATUS & (((long) 1) << TimesToShift)) != 0){
                     switch (TimesToShift) {
                     case 22:
                        if ((*PRESENTION_STATUS &
                        (((long) 1) << (TimesToShift+1))) == 0)
                           AdjustWidth((unsigned long) pitch ,
                                       (unsigned long) attr  );
                        break;
                     case 23:
                        if ((*PRESENTION_STATUS &
                        (((long) 1) << (TimesToShift-1))) == 0)
                           AdjustWidth((unsigned long)pitch,
                           (unsigned long)attr);
                        break;
                     default:
                        AdjustWidth((unsigned long)pitch,(unsigned long) attr);
                     } /* endswitch */
                  }
                  else{
                     if ( (TimesToShift == 23)
                          && ( (*PRESENTION_STATUS
                           & ( ((long) 1) << (TimesToShift-1)) ) != 0) ){
                        *PRESENTION_STATUS &= 0xFFBFFFFF;
                        AdjustWidth((unsigned long)pitch,(unsigned long) attr);
                     }
                  }

               Temp_Buff = ~( ((long) 1) << TimesToShift);
               *PRESENTION_STATUS &= Temp_Buff;

               }
               else {
                  if ((*PRESENTION_STATUS & (((long) 1) << TimesToShift)) == 0){
                     switch (TimesToShift) {
                     case 22:
                        if ((*PRESENTION_STATUS
                        & (((long) 1) << (TimesToShift+1))) == 0)
                           AdjustWidth((unsigned long)pitch,
                           (unsigned long)attr);
                        break;
                     case 23:
                        if ((*PRESENTION_STATUS
                        & (((long) 1) << (TimesToShift-1))) == 0)
                           AdjustWidth((unsigned long)pitch,
                           (unsigned long)attr);
                        else
                           *PRESENTION_STATUS &= 0xFFBFFFFF;
                        break;
                     default:
                        AdjustWidth((unsigned long)pitch,(unsigned long) attr);
                     } /* endswitch */
                  }

               Temp_Buff = ((long) 1) << TimesToShift;
               *PRESENTION_STATUS |= Temp_Buff;
               }                                                    /* endif */
               break;
            }
}                                                          /*end of procedure*/





void FirstInitialize ()
{
   unsigned int    i;
   i = 0;

   while ( i < (unsigned int)MAX_BUFFER_LENGTH){
      G.UpperVowelBuff[i].ch = G.LineBuff[i].ch = G.LowerVowelBuff[i].ch = 0x20;
       /**************************************************/
       /* Initialize character field with blank (space). */
       /**************************************************/
      G.UpperVowelBuff[i].attr = G.LineBuff[i].attr = G.LowerVowelBuff[i].attr = 0;
       /********************************************/
       /* Initialize the presentation field with 0 */
       /********************************************/
      G.UpperVowelBuff[i].escsize = G.LineBuff[i].escsize = G.LowerVowelBuff[i].escsize = 0;
      G.UpperVowelBuff[i].escapes = G.LineBuff[i].escapes = G.LowerVowelBuff[i].escapes = NULL;
      ++i;
   }
}




/**************************** START OF SPECIFICATIONS ***********************/
/*  SUBROUTINE NAME: InitializeBuffer                                       */
/*                                                                          */
/*  DESCRIPTIVE NAME: Initializes the line buffer (LineBuff) with blanks    */
/*                                                                          */
/*  FUNCTION: Initializes the LineBuff with blanks and zero presentation    */
/*            status                                                        */
/*                                                                          */
/*  ENTRY POINT: InitializeBuffer                                           */
/*                                                                          */
/*      LINKAGE: CALL      (from  _CAN_FOUND,                               */
/*                                NewLine,                                  */
/*                                PSM_WriteToBuffer,                        */
/*                                ProcessorInitialize,                      */
/*                                FlushLine)                                */
/*                                                                          */
/*  INPUT: none                                                             */
/*                                                                          */
/*  SIDE EFFECTS: NONE                                                      */
/*                                                                          */
/*  INTERNAL REFERENCES:                                                    */
/*     LOCAL DATA DEFINITION:                                               */
/*         i                  WORD     -dummy variable                      */
/*                                                                          */
/*     GLOBAL DATA DEFINITIONS:                                             */
/*         G                OTHER  -Structure EntryPoint which contains     */
/*                                  the processing context of the current   */
/*                                  input buffer.                           */
/*                                                                          */
/*     ROUTINES: NONE                                                       */
/*                                                                          */
/*  EXTERNAL REFERENCES:                                                    */
/*     ROUTINES: NONE                                                       */
/************************** END OF SPECIFICATIONS ***************************/
void InitializeBuffer(void)
{
   unsigned int  i=0;
    
   while (i < (unsigned int)MAX_BUFFER_LENGTH) 
   { 
     /* Initialize character field with blank (space). */
     G.LineBuff[i].ch = 0x20; 

     /* Initialize the presentation field with 0 */
     G.LineBuff[i].attr = 0;   
 
     if (G.LineBuff[i].escapes != NULL && G.LineBuff[i].escsize > 0)
         free (G.LineBuff[i].escapes);

     G.LineBuff[i].escsize = 0;

     if (TASHKEEL)
     {
       G.UpperVowelBuff[i].ch = G.LowerVowelBuff[i].ch = 0x20;
       G.UpperVowelBuff[i].attr = G.LowerVowelBuff[i].attr = 0;
       if (G.UpperVowelBuff[i].escapes != NULL && G.UpperVowelBuff[i].escsize > 0)
 	  free (G.UpperVowelBuff[i].escapes);
       if (G.LowerVowelBuff[i].escapes != NULL && G.LowerVowelBuff[i].escsize > 0)
	  free (G.LowerVowelBuff[i].escapes);
     }  /* if TASHKEEL */
 
     G.UpperVowelBuff[i].escsize = G.LowerVowelBuff[i].escsize = 0;
     ++i;
   } /* while */
}



/**************************** START OF SPECIFICATIONS ***********************/
/*  SUBROUTINE NAME: WriteToBuffer                                          */
/*                                                                          */
/*  DESCRIPTIVE NAME: used to write printable characters together with      */
/*                    their associated attributes into the line buffer.     */
/*                                                                          */
/*  FUNCTION: whenever a printable character occures the function is        */
/*            called to write this character into the line buffer           */
/*            together with the presentation status (PRESENTION_STATUS)     */
/*            indicating the mode with which this character is to be        */
/*            printed.                                                      */
/*            Before writing the character to the line buffer a check       */
/*            is performed on the line length to see if there is a place    */
/*            to write this character or not.                               */
/*            If there is a place to write the character then both, the     */
/*            character and the corresponding presentation status, are      */
/*            written. The function increments the pointer to position      */
/*            (CurrPos) in buffer (LineBuff) to point to the next empty     */
/*            position.                                                     */
/*            It also increments the line length (LINE_LENGTH) by a         */
/*            value equal to the character width (CWIDTH).                  */
/*            If the line is full then the line is first processed and      */
/*            written to the output buffer before writing the new           */
/*            character to the line buffer.                                 */
/*            The function returns also a return value zero or one to       */
/*            indicate if the character width has been changed              */
/*            returning a zero or it has not been changed returning a 1     */
/*                                                                          */
/*                                                                          */
/*  ENTRY POINT: WriteToBuffer                                              */
/*      LINKAGE: CALL (_PRINTABLE)                                          */
/*                                                                          */
/*  INPUT: (ch, PS)                                                         */
/*         ch               CHAR    -the printable character.               */
/*                                                                          */
/*         PS               DWORD    -the current presentation              */
/*                                    status definning the cuurent          */
/*                                    the printers current status.          */
/*                                    (i.e the mode in which the            */
/*                                    character is to be printed)           */
/*                                                                          */
/*  SIDE EFFECTS: NONE                                                      */
/*                                                                          */
/*  INTERNAL REFERENCES:                                                    */
/*     LOCAL DATA DEFINITION:                                               */
/*          OLD_CW          WORD     - to save the G.CWIDTH value in it     */
/*                                                                          */
/*     GLOBAL DATA DEFINITIONS:                                             */
/*         G                OTHER  -Structure EntryPoint which contains     */
/*                                  the processing context of the current   */
/*                                  input buffer.                           */
/*                                                                          */
/*     ROUTINES:                                                            */
/*         FlushLine                                                        */
/*                                                                          */
/*  EXTERNAL REFERENCES:                                                    */
/*     ROUTINES: NONE                                                       */
/************************** END OF SPECIFICATIONS ***************************/
unsigned long WriteToBuffer(unsigned long ch, unsigned long ps)
{
   unsigned int OLD_CW;
   unsigned char        CurrUpperVowel, CurrLowerVowel;

   OLD_CW = G.CWIDTH;

   /* Maya  4/8/1993 TASHKEEL */ 
 		   /****************************************/
		   /* check to see if character is a Vowel */
 		   /****************************************/
   if (TASHKEEL  &&  ch > 127  &&  Vowels[ch-128])
   {
     ch = Vowels[ch-128]; 
     if ((unsigned char)ch != 0xED && (unsigned char)ch != 0xF0)  /* UPPER VOWEL: not KASRATAN nor KASRA */
     {
       /**********************************************************/
       /* In Visual LTR, Tonal Characters preceed the Consonants */
       /**********************************************************/
       if ((G.BIDI_ATTR | 0xF0F0FFFF) == 0xF0F0FFFF)   /* if Visual LTR */
       {
         G.UpperVowelBuff[G.CurrPos].ch = (unsigned char)ch;
         /*******************************************************************/
         /* In PSM mode, PSM_WriteToBuffer stores consonants in G.LineBuff. */
         /* Therefore, the Vowels' Presentation Status isn't updated.       */
         /*******************************************************************/
         if ((G.PRESENTION_STATUS & PRT_PSM_ATTR) != 0)  
              G.UpperVowelBuff[G.CurrPos].attr = ps & 0xFBDFFFFF;
         return 1;
       }
       else     /* Consonants preceed tonal characters */
       {
         if (G.CurrPos && G.UpperVowelBuff[G.CurrPos-1].ch == 0x20)
         {
           G.UpperVowelBuff[G.CurrPos-1].ch = (unsigned char)ch;
           if ((G.PRESENTION_STATUS & PRT_PSM_ATTR) != 0) 
 	        G.UpperVowelBuff[G.CurrPos-1].attr = ps & 0xFBDFFFFF;
           return 1;
         }
       }
     }
     else  /* Character is a LOWER VOWEL */
     {
       if ((G.BIDI_ATTR | 0xF0F0FFFF) == 0xF0F0FFFF)
       {
         G.LowerVowelBuff[G.CurrPos].ch = (unsigned char)ch;
         if ((G.PRESENTION_STATUS & PRT_PSM_ATTR) != 0)  
 	      G.LowerVowelBuff[G.CurrPos].attr = ps & 0xFBDFFFFF;
         return 1;
       }
       else
       {
         if (G.CurrPos && G.LowerVowelBuff[G.CurrPos-1].ch == 0x20)
         {
           G.LowerVowelBuff[G.CurrPos-1].ch = (unsigned char)ch;
           if ((G.PRESENTION_STATUS & PRT_PSM_ATTR) != 0)
 	        G.LowerVowelBuff[G.CurrPos-1].attr = ps & 0xFBDFFFFF;
           return 1;
         }
       }
     }   /* end if LOWER VOWEL */
   } /* endif VOWEL  */
   else if ((G.LINE_LENGTH+G.CWIDTH) <= G.PRT_LINE_LENGTH)
        {
          /*******************************************************/
          /* check to see if there is a place in the line buffer */
          /* to write the character.                             */
          /*******************************************************/
          G.LineBuff[G.CurrPos].ch = (unsigned char)ch;
          if (TASHKEEL)
              G.UpperVowelBuff[G.CurrPos].attr = G.LowerVowelBuff[G.CurrPos].attr = ps & 0xFBDFFFFF; /* To disable under/over score */ 
          G.LineBuff[G.CurrPos++].attr = ps;
          G.LINE_LENGTH += G.CWIDTH;
          /*********************************************************/
          /* increment the value of the line length by the value   */
          /* of the width of the character just written.           */
          /*********************************************************/
          return 1;
        }
        else {
               /* If TASHKEEL and Visual LTR, save UpperVowel, LowerVowel and put spaces */
               if (TASHKEEL && ((G.BIDI_ATTR | 0xF0F0FFFF) == 0xF0F0FFFF))
               {
                 CurrUpperVowel = G.UpperVowelBuff[G.CurrPos].ch;
                 CurrLowerVowel = G.LowerVowelBuff[G.CurrPos].ch;
                 G.UpperVowelBuff[G.CurrPos].ch = G.LowerVowelBuff[G.CurrPos].ch = 0x20;     
               }

               FlushLine();

               if (TASHKEEL && ((G.BIDI_ATTR | 0xF0F0FFFF) == 0xF0F0FFFF))  /* if Visual LTR */
               {
                 G.UpperVowelBuff[G.CurrPos].ch = CurrUpperVowel;
                 G.LowerVowelBuff[G.CurrPos].ch = CurrLowerVowel;
               } /* end if TASHKEEL */
               G.LineBuff[G.CurrPos].ch = (unsigned char)ch;

               if (TASHKEEL)
                   G.UpperVowelBuff[G.CurrPos].attr = G.LowerVowelBuff[G.CurrPos].attr = G.PRESENTION_STATUS & 0xFBDFFFFF; /* To disable under/over score */ 
               G.LineBuff[G.CurrPos++].attr = G.PRESENTION_STATUS;

               G.LINE_LENGTH += G.CWIDTH;
         
               G.PRT_PSM_W[2] = 0;
               G.PRT_PSM_W[1] = 0;
               if (G.CWIDTH != OLD_CW) return 0;
               else return 1;
        }                                           
}  /* end of procedure */

/**************************** START OF SPECIFICATIONS ***********************/
/*  SUBROUTINE NAME: GET_PSM_CWIDTH                                         */
/*                                                                          */
/*  DESCRIPTIVE NAME: Get the character width                               */
/*                                                                          */
/*  FUNCTION: According to the character pitch and the character            */
/*            presentation status this function returns its width in        */
/*            APLES                                                         */
/*                                                                          */
/*  ENTRY POINT: GET_PSM_CWIDTH                                             */
/*      LINKAGE: CALL (_PSM_WriteToBuffer)                                  */
/*                                                                          */
/*  INPUT: (ch, PS, tempwidth )                                             */
/*         ch               CHAR     -the printable character.              */
/*                                                                          */
/*         PS               DWORD    -the current presentation              */
/*                                    status definning the cuurent          */
/*                                    the printers current status.          */
/*                                    (i.e the mode in which the            */
/*                                    character is to be printed)           */
/*                                                                          */
/*         tempwidth        DWORD    - width of the character               */
/*                                                                          */
/*         RestorFlag       WORD                                            */
/*                                                                          */
/*                                                                          */
/*  SIDE EFFECTS: NONE                                                      */
/*                                                                          */
/*  INTERNAL REFERENCES:                                                    */
/*     LOCAL DATA DEFINITION:                                               */
/*                                                                          */
/*         SAVED_WIDTH      WORD     - to save the character width          */
/*                                                                          */
/*                                                                          */
/*     GLOBAL DATA DEFINITIONS:                                             */
/*         PRT_TABLE        @OTHER   -pointer of an array containing        */
/*                                    all characters psm width factor       */
/*                                                                          */
/*         PRT_OLD_CWIDTH   DWORD - Default Character in APELS.             */
/*                                                                          */
/*         PRT_SPACE_FOR    @OTHER -Used for right-justification.           */
/*                                  and test for 5202 in PSM_WIDTH.         */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*     ROUTINES: NONE                                                       */
/*                                                                          */
/*  EXTERNAL REFERENCES:                                                    */
/*     ROUTINES: NONE                                                       */
/************************** END OF SPECIFICATIONS ***************************/
  unsigned int GET_PSM_CWIDTH(unsigned long ch, unsigned long ps,
                              unsigned long tempwidth)
  {
  unsigned int  SAVED_WIDTH;

    SAVED_WIDTH = (unsigned int)tempwidth;
         /**************************************************************/
         /* save the width of the character in case of PSM not active. */
         /**************************************************************/

    if ((ps & PRT_PSM_ATTR) != 0){
         /******************************************************/
         /* check if the character is/was written in PSM mode. */
         /******************************************************/

       if ( ((CFG->PRT_SPACE_FOR[1]&0x40)==0) && (CFG->PRT_SPACE_FOR[0]!=0) ){
            /***********************************/
            /* check if the printer is  3812 . */
            /***********************************/
          tempwidth = (tempwidth == 255) ? 306L:
                                           tempwidth;
               /************************************************************/
               /* this is a special case of 3812 which has a default pitch */
               /* equal to 8 CPI with default character width equal to 255 */
               /************************************************************/
       }

       else{
            /***************************************/
            /* this is the case of 5202,4208, 4207 */
            /***************************************/
          if ((ps & 0x00C00000) != 0){
               /************************************/
               /* check if DOUBLE WIDTH is active. */
               /************************************/
             tempwidth = tempwidth * 2;
             }
       }

       tempwidth = tempwidth * PsmTable[ch];


       if ( ((CFG->PRT_SPACE_FOR[1] & 0x40) == 0) && (CFG->PRT_SPACE_FOR[0]!=0)) {
            /**************************************/
            /* check if the printer is not 5202 . */
            /**************************************/

             tempwidth = tempwidth / CFG->PRT_OLD_CWIDTH;
            /**************************************************************/
            /* printers other than 5202 support different pitches in case */
            /* of PSM which means that the character width in case of PSM */
            /* is not that read from the PSM_TABLE but is a ratio of that */
            /* value.                                                     */
            /**************************************************************/
       }
       else{
          tempwidth = tempwidth / SAVED_WIDTH;
       }
    }
    return((unsigned int)tempwidth);
 }



/**************************** START OF SPECIFICATIONS ***********************/
/*  SUBROUTINE NAME: PSM_WriteToBuffer                                      */
/*                                                                          */
/*  DESCRIBTIVE NAME: used to write printable characters together with      */
/*                    their associated attributes into the line buffer.     */
/*                    in case of PSM is active                              */
/*                                                                          */
/*  FUNCTION: whenever a printable character occurs the function is         */
/*            called to write this character into the line buffer           */
/*            together with the presentation status (PRESENTION_STATUS)     */
/*            indicating the mode with which this character is to be        */
/*            printed.                                                      */
/*            Before writing the character to the line buffer a check is    */
/*            is performed on the line length to see if there is a place    */
/*            to write this character or not.                               */
/*            If there is a place to write the character then both, the     */
/*            character and the corresponding presentation status, are      */
/*            written. The function increments the pointer to position      */
/*            (CurrPos) in buffer (LineBuff) to point to the next empty     */
/*            position.                                                     */
/*            It also increments the line length (LINE_LENGTH) by a         */
/*            value equal to the character width.                           */
/*            If the line is full then the line is first processed and      */
/*            written to the output buffer before writing the new           */
/*            character to the line buffer.                                 */
/*                                                                          */
/*  ENTRY POINT: PSM_WriteToBuffer                                          */
/*      LINKAGE: CALL (_PRINTABLE)                                          */
/*                                                                          */
/*  INPUT: (ch,ps)                                                          */
/*         ch               CHAR         -the printable character.          */
/*                                                                          */
/*         ps               DWORD        -the current presentation          */
/*                                        status definning the cuurent      */
/*                                        the printers current status.      */
/*                                        (i.e the mode in which the        */
/*                                        character is to be printed)       */
/*                                                                          */
/*  SIDE EFFECTS: NONE                                                      */
/*                                                                          */
/*  INTERNAL REFERENCES:                                                    */
/*     LOCAL DATA DEFINITION: NONE                                          */
/*                                                                          */
/*         i                WORD       - Dummy counter                      */
/*                                                                          */
/*         c                WORD       - Dummy counter                      */
/*                                                                          */
/*         RestoreFlag      DWORD                                           */
/*                                                                          */
/*         TempWidth_1      WORD       - to hold temporary values           */
/*                                                                          */
/*         TempWidth_2      WORD       - to hold temporary values           */
/*                                                                          */
/*         w                @OTHER     - array of three to hold             */
/*                                       psm widths temporarly              */
/*                                                                          */
/*         effect           DWORD      - to be passed to the API            */
/*                                                                          */
/*         RETURN_CODE      DWORD      - to hold the return code of API     */
/*                                                                          */
/*         HANDLE           DWORD      - initialized with zero              */
/*                                                                          */
/*         Line_Buff_Src    @OTHER     - pointer to structure named         */
/*                                       CSDRec   	                    */
/*                                                                          */
/*                                                                          */
/*     GLOBAL DATA DEFINITIONS:                                             */
/*         G                OTHER  -Structure EntryPoint which contains     */
/*                                  the processing context of the current   */
/*                                  input buffer.                           */
/*                                                                          */
/*         HCP              WORD       - to indicate if the code pages      */
/*                                       862 or 856 are supported           */
/*                                                                          */
/*     ROUTINES:                                                            */
/*         AccProcessChar                                                   */
/*         FlushLine                                                        */
/*         GET_PSM_CWIDTH                                                   */
/*         InitializeBuffer                                                 */
/*         layout_object_editshape                                          */
/*         layout_object_transform                                          */
/*         WriteToBuffer                                                    */
/*                                                                          */
/*  EXTERNAL REFERENCES:                                                    */
/*     ROUTINES: NONE                                                       */
/************************** END OF SPECIFICATIONS ***************************/
  void PSM_WriteToBuffer(unsigned long ch, unsigned long ps)
  {
    int i;
    unsigned int         *ToInpBuf;       /* maya 12/5/1993 */

    size_t               InpSize, OutSize;
    struct Buffer        TempLineBuff[MAX_BUFFER_LENGTH];
    size_t               BufferIndex, InpBufferSize, OutBufferSize;

    unsigned long        RestoreFlag = 1, RETURN_CODE;
    unsigned int         W[3], PSM_SAVED_CWIDTH, TempWidth_1, TempWidth_2;
    BooleanValue         effect=EDITREPLACE;
    unsigned char        CurrUpperVowel, CurrLowerVowel;


    PSM_SAVED_CWIDTH =G.CWIDTH;
                             /************************************************/
                             /* Here we need to save the current CWIDTH      */
                             /* because in this function we are going to     */
                             /* change the value of that global variabel     */
                             /* At the end we will restore it back again     */
                             /************************************************/

     if (HCP == 1){
                             /************************************************/
                             /* Check if the code page supported is 862 or   */
                             /* 856 then do not consider the shaping         */
                             /************************************************/

        G.CWIDTH = GET_PSM_CWIDTH (ch, ps, (unsigned long)PSM_SAVED_CWIDTH );

        RestoreFlag = WriteToBuffer (ch, ps);
        if (RestoreFlag == 1){
           G.CWIDTH = PSM_SAVED_CWIDTH ;
        }
        return;
     }

     switch (G.S1){
       case 0xC2:                         /* case of ALEF_MADDA               */
       case 0xC3:                         /* case of ALEF_HAMZA               */
       case 0xC7:                         /* case of ALEF                     */
          if (G.CurrPos != 0){
             G.CWIDTH = GET_PSM_CWIDTH((unsigned long)G.S1,
                                     G.PRESENTION_STATUS,
                                     (unsigned long)G.CWIDTH);
             if ( G.LINE_LENGTH+G.CWIDTH <= G.PRT_LINE_LENGTH ){
                G.LineBuff[G.CurrPos].ch = G.S1;
                G.LineBuff[G.CurrPos].attr = ps;
                G.LINE_LENGTH += G.CWIDTH;
                G.CurrPos++;
                }
             else{
                FlushLine ();
                RestoreFlag = 0;
                G.PRT_PSM_W[2] = 0;
                G.PRT_PSM_W[1] = 0;
                G.PRT_PSM_W[0] = PSM_SAVED_CWIDTH ;
             }
             G.S1 = 0;
             if (RestoreFlag == 1)
                G.CWIDTH = PSM_SAVED_CWIDTH ;
             return;
             }

          else{
             G.S1 = 0;
             }
       break;

       case 0xA6:                         /* case of YEH_HAMZA 1046           */
         if ((G.CurrPos != 0) && (ch == 0x20)){
            TempWidth_1 = GET_PSM_CWIDTH(
                                   (unsigned long)G.LineBuff[G.CurrPos - 1].ch,
                                   G.LineBuff[G.CurrPos - 1].attr,
                                   (unsigned long)G.PRT_PSM_W[1]);
            G.LINE_LENGTH -= TempWidth_1;

            for (i=0; i<G.CurrPos+1; i++)
                 G.CharLineBuff[i] = G.LineBuff[i].ch;

 	    BufferIndex   = (unsigned long)(G.CurrPos-1);
            OutBufferSize = InpBufferSize = (int)(G.CurrPos+1);

            RETURN_CODE = layout_object_editshape(plh, effect,
                                                  &BufferIndex,
                                                  (unsigned char *)G.CharLineBuff,
                                                  &InpBufferSize,
                                                  (unsigned char *)G.CharLineBuff2,
                                                  &OutBufferSize);
            for (i=0; i<OutBufferSize; i++)
                G.CharLineBuff[BufferIndex+i] = G.CharLineBuff2[i];
            for (i=0; i<G.CurrPos+1; i++)
                G.LineBuff[i].ch = G.CharLineBuff[i];

           /* maya 28/2/1993 */
           /* if (RETURN_CODE != 0)
             TRACE("/aix3.2/maya/out", "1  Ret Code from layout_object_editshape = %d\n", RETURN_CODE); */

          G.CWIDTH = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos - 1].ch,
                                    G.LineBuff[G.CurrPos - 1].attr,
                                    (unsigned long)G.PRT_PSM_W[1]);

           RestoreFlag = WriteToBuffer (
                                      (unsigned long)G.LineBuff[G.CurrPos-1].ch,
                                      G.LineBuff[G.CurrPos-1].attr);
           TempWidth_2 = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos].ch,
                                       G.LineBuff[G.CurrPos].attr,
                                       (unsigned long)PSM_SAVED_CWIDTH );

            if ( (G.LINE_LENGTH+TempWidth_2) <= G.PRT_LINE_LENGTH ){
               G.LineBuff[G.CurrPos].attr = ps;
               G.LINE_LENGTH += TempWidth_2;
               G.CurrPos++;
               }
            else{
               G.LineBuff[G.CurrPos].ch = 0x20;
               G.LineBuff[G.CurrPos-1].ch = G.S1;
               G.LINE_LENGTH += TempWidth_1;
               FlushLine();
               RestoreFlag = 0;
               G.PRT_PSM_W[2] = 0;
               G.PRT_PSM_W[1] = 0;
               G.PRT_PSM_W[0] = PSM_SAVED_CWIDTH ;
            }
            G.S1 = 0;
            if (RestoreFlag == 1)
               G.CWIDTH = PSM_SAVED_CWIDTH ;
            return;
            }
         else{
            G.S1 = 0;
         }
       break;
     }                                                           /* endswitch */

     switch ((unsigned char)GetTheByte((char *)&G.BIDI_ATTR, (unsigned long)0)){
       case 128:   /* in case of one cell */
       case 0:
         if (G.CurrPos == 0){

            if (ch == 0xF6) {
               break;
            }
            else {
               G.LineBuff[G.CurrPos].ch   = (unsigned char)ch;
               G.LineBuff[G.CurrPos].attr = ps;

               for (i=0; i<G.CurrPos+2; i++)
                    G.CharLineBuff[i] = G.LineBuff[i].ch;

               BufferIndex   = (unsigned long)(G.CurrPos);
               OutBufferSize = InpBufferSize = (int)(G.CurrPos+2);

               RETURN_CODE = layout_object_editshape(plh, effect,
                                                     &BufferIndex,
                                                     (unsigned char *)G.CharLineBuff,
                                                     &InpBufferSize,
                                                     (unsigned char *)G.CharLineBuff2,
                                                     &OutBufferSize);

               for (i=0; i<OutBufferSize; i++)
                    G.CharLineBuff[BufferIndex+i] = G.CharLineBuff2[i];
               for (i=0; i<G.CurrPos+2; i++)
                    G.LineBuff[i].ch = G.CharLineBuff[i];

           /* maya 28/2/1993 */
           /* if (RETURN_CODE != 0) 
               TRACE("/aix3.2/maya/out", "2  Ret Code from layout_object_editshape = %d\n", RETURN_CODE); */

               W[0] = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos].ch,
                                     G.LineBuff[G.CurrPos].attr,
                                     (unsigned long)G.CWIDTH);

               if ((G.LINE_LENGTH + W[0]) > G.PRT_LINE_LENGTH ){
                  AccProcessChar( (WCHAR) 10);
                  /************************************************************/
                  /* A Line Feed and a Carriage Return are sent to the printer*/
                  /* to start a new line.                                     */
                  /************************************************************/
                  G.LINE_LENGTH = 0;
                  InitializeBuffer ();
                  G.PRT_PSM_W[2] = 0;
                  G.PRT_PSM_W[1] = 0;
                  G.PRT_PSM_W[0] = PSM_SAVED_CWIDTH ;
               }
               G.LINE_LENGTH += W[0];
               ++G.CurrPos;
            }
         }
         else{
            W[1] = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos-1].ch,
                                  G.LineBuff[G.CurrPos-1].attr,
                                  (unsigned long)G.PRT_PSM_W[1]);

            W[2] = (G.CurrPos == 1) ? 0 : GET_PSM_CWIDTH(
                                      (unsigned long)G.LineBuff[G.CurrPos-2].ch,
                                       G.LineBuff[G.CurrPos-2].attr,
                                       (unsigned long)G.PRT_PSM_W[2]);

            G.LINE_LENGTH -=(W[1]+W[2]);

            G.LineBuff[G.CurrPos].ch   = (unsigned char)ch;
            G.LineBuff[G.CurrPos].attr = ps;

            if ((G.LineBuff[G.CurrPos-1].attr & PRT_PSM_ATTR) == 0x00000000)
            {
	      ToInpBuf = (unsigned int *)malloc(sizeof(unsigned int)*G.CurrPos);

              for (i=0; i<G.CurrPos; i++)
                   G.CharLineBuff[i] = G.LineBuff[i].ch;

              OutSize = InpSize = G.CurrPos;

              /* Set layout values so as only to shape. */
              set_new_layout_values((G.BIDI_ATTR & 0x00FF0000) | 0x00000010,
                                    (G.BIDI_ATTR & 0x00FF00FF));

              RETURN_CODE = layout_object_transform (plh,
              		                   (char *) G.CharLineBuff, &InpSize,
                         		   (char *) G.CharLineBuff, &OutSize,
                                	   NULL, ToInpBuf, NULL);

             /***********************************/
             /* Set the original layout values. */
             /***********************************/

             if (G.BIDI_ATTR & 0x00000010)    /* base shapes */
                 set_new_layout_values((G.BIDI_ATTR & 0xFFFF0000),
                                       (G.BIDI_ATTR & 0x0000FFFF));
             else if (G.BIDI_ATTR & 0x00000001)    /* passthrough */
                      set_new_layout_values((G.BIDI_ATTR & 0xFFFF0000),
                                            (G.BIDI_ATTR & 0x0000FFC0));
                  else set_new_layout_values((G.BIDI_ATTR & 0xFFFF0000)|0x00000010,
                                             (G.BIDI_ATTR & 0x0000FFFF));

              /********************************************************/
              /* Put each character with its corresponding attributes */ 
              /********************************************************/
   	      for (i=0; i<OutSize; i++)
     	      {
      	        TempLineBuff[i].ch      = G.CharLineBuff[i];
     	        TempLineBuff[i].attr    = G.LineBuff[ToInpBuf[i]].attr;
     	        TempLineBuff[i].escsize = G.LineBuff[ToInpBuf[i]].escsize;
    	        TempLineBuff[i].escapes = G.LineBuff[ToInpBuf[i]].escapes;
    	      }
              for (i=0; i<OutSize; i++)
              {
 	        G.LineBuff[i].ch      = TempLineBuff[i].ch;
      	        G.LineBuff[i].attr    = TempLineBuff[i].attr;
                G.LineBuff[i].escsize = TempLineBuff[i].escsize;
     	        G.LineBuff[i].escapes = TempLineBuff[i].escapes;
              }

              /* Maya  15/8/1993   Begin TASHKEEL */
              if (TASHKEEL)
              {
                for (i=0; i<OutSize; i++)
                {
                  TempLineBuff[i].ch      = G.UpperVowelBuff[ToInpBuf[i]].ch;
                  TempLineBuff[i].attr    = G.UpperVowelBuff[ToInpBuf[i]].attr;
                  TempLineBuff[i].escsize = G.UpperVowelBuff[ToInpBuf[i]].escsize;
                  TempLineBuff[i].escapes = G.UpperVowelBuff[ToInpBuf[i]].escapes;
                }
                for (i=0; i<OutSize; i++)
                {
                  G.UpperVowelBuff[i].ch      = TempLineBuff[i].ch;
                  G.UpperVowelBuff[i].attr    = TempLineBuff[i].attr;
                  G.UpperVowelBuff[i].escsize = TempLineBuff[i].escsize;
                  G.UpperVowelBuff[i].escapes = TempLineBuff[i].escapes;
                }
                for (i=0; i<OutSize; i++)
                {
                  TempLineBuff[i].ch      = G.LowerVowelBuff[ToInpBuf[i]].ch;
                  TempLineBuff[i].attr    = G.LowerVowelBuff[ToInpBuf[i]].attr;
                  TempLineBuff[i].escsize = G.LowerVowelBuff[ToInpBuf[i]].escsize;
                  TempLineBuff[i].escapes = G.LowerVowelBuff[ToInpBuf[i]].escapes;
                }
                for (i=0; i<OutSize; i++)
                {
                  G.LowerVowelBuff[i].ch      = TempLineBuff[i].ch;
                  G.LowerVowelBuff[i].attr    = TempLineBuff[i].attr;
                  G.LowerVowelBuff[i].escsize = TempLineBuff[i].escsize;
                  G.LowerVowelBuff[i].escapes = TempLineBuff[i].escapes;
                }
              } /* End Tashkeel */

    	      free(ToInpBuf);

              /* maya 28/2/1993 */
              /* if (RETURN_CODE != 0) 
                TRACE("/aix3.2/maya/out", "3   Ret Code from layout_object_transform = %d\n", RETURN_CODE); */
             }
  
             for (i=0; i<G.CurrPos+2; i++)
                  G.CharLineBuff[i] = G.LineBuff[i].ch;
 
             BufferIndex   = (unsigned long)(G.CurrPos);
             OutBufferSize = InpBufferSize = (int)(G.CurrPos+2);
 
             RETURN_CODE = layout_object_editshape(plh, effect,
                                                   &BufferIndex,
                                                   (unsigned char *)G.CharLineBuff,
                                                   &InpBufferSize,
                                                   (unsigned char *)G.CharLineBuff2,
                                                   &OutBufferSize);

            for (i=0; i<OutBufferSize; i++)
                 G.CharLineBuff[BufferIndex+i] = G.CharLineBuff2[i];
            for (i=0; i<G.CurrPos+2; i++)
                 G.LineBuff[i].ch = G.CharLineBuff[i];

            /* maya 28/2/1993 */
            /* if (RETURN_CODE != 0) 
               TRACE("/aix3.2/maya/out", "4  Ret Code from layout_object_editshape = %d\n", RETURN_CODE); */

            if (G.CurrPos != 1){
               W[2] = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos-2].ch,
                                     G.LineBuff[G.CurrPos-2].attr,
                                     (unsigned long)G.PRT_PSM_W[2]);
               G.LINE_LENGTH += W[2];
            }

            switch (G.LineBuff[G.CurrPos-1].ch){
            case 0xE9:                    /* case of ALEF_MAKSURA */
            case 0x96:                    /* case of YEH_FINAL */
              W[1] = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos-1].ch,
                                    G.LineBuff[G.CurrPos-1].attr,
                                    (unsigned long)G.PRT_PSM_W[1]);

              if ( (G.LINE_LENGTH + W[1]) > G.PRT_LINE_LENGTH ){
                 G.LineBuff[G.CurrPos-1].ch = 0xA6;  /* YEH HAMZA *?/
                 W[1] = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos-1].ch,
                                       G.LineBuff[G.CurrPos-1].attr,
                                       (unsigned long)G.PRT_PSM_W[1]);
                 G.LINE_LENGTH += W[1];
                 FlushLine();
                 RestoreFlag = 0;
                 G.PRT_PSM_W[2] = 0;
                 G.PRT_PSM_W[1] = 0;
                 G.PRT_PSM_W[0] = PSM_SAVED_CWIDTH ;
              }
              else{
                 G.LINE_LENGTH += W[1];
              }
            break;
            case 0xF8:
            case 0x9D:
            case 0xFA:
            case 0x9F:
            case 0xF7:
            case 0x9C:
              W[1] = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos-1].ch,
                                    G.LineBuff[G.CurrPos-1].attr,
                                    (unsigned long)G.PRT_PSM_W[1]);
              if ((G.LINE_LENGTH + W[1]) > G.PRT_LINE_LENGTH ){
                 G.LineBuff[G.CurrPos-1].ch = 0xE4;            /*LAM_FINAL */
                 W[1] = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos-1].ch,
                                       G.LineBuff[G.CurrPos-1].attr,
                                       (unsigned long)G.PRT_PSM_W[1]);
                 G.LINE_LENGTH += W[1];
                 FlushLine();
                 RestoreFlag = 0;
                 G.PRT_PSM_W[2] = 0;
                 G.PRT_PSM_W[1] = 0;
                 G.PRT_PSM_W[0] = PSM_SAVED_CWIDTH ;
                 G.LineBuff[G.CurrPos].ch = (unsigned char)ch;
                 G.LineBuff[G.CurrPos].attr = ps;
                 W[0] = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos].ch,
                                       G.LineBuff[G.CurrPos].attr,
                                       (unsigned long)G.PRT_PSM_W[0]);
                 G.LINE_LENGTH += W[0];
                 G.CurrPos++;
                 return;
              }
              else{
                 G.LINE_LENGTH += W[1];
              }
            break;

            default:
              W[1] = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos-1].ch,
                                    G.LineBuff[G.CurrPos-1].attr,
                                    (unsigned long)G.PRT_PSM_W[1]);

              G.LINE_LENGTH += W[1];
            break;
            }                                                  /* endswitch */

            W[0] = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos].ch,
                                  G.LineBuff[G.CurrPos].attr,
                                  (unsigned long)G.PRT_PSM_W[0]);

            if ( (G.LINE_LENGTH + W[0]) > G.PRT_LINE_LENGTH ){
               if (TASHKEEL && (G.BIDI_ATTR | 0xF0F0FFFF == 0xF0F0FFFF))  /* if Visual LTR */
               {
                 CurrUpperVowel = G.UpperVowelBuff[G.CurrPos].ch;
                 CurrLowerVowel = G.LowerVowelBuff[G.CurrPos].ch;
                 G.UpperVowelBuff[G.CurrPos].ch = G.LowerVowelBuff[G.CurrPos].ch = 0x20;
               } /* end if TASHKEEL */

               G.LineBuff[G.CurrPos].ch = 0x20;
               G.LINE_LENGTH -= (W[1]+W[2]);

               for (i=0; i<G.CurrPos+2; i++)
                    G.CharLineBuff[i] = G.LineBuff[i].ch;

               BufferIndex   = (unsigned long)(G.CurrPos);
               OutBufferSize = InpBufferSize = (int)(G.CurrPos+2);

               RETURN_CODE = layout_object_editshape(plh, effect,
                                                     &BufferIndex,
                                                     (unsigned char *)G.CharLineBuff,
                                                     &InpBufferSize,
                                                     (unsigned char *)G.CharLineBuff2,
                                           	     &OutBufferSize);

               for (i=0; i<OutBufferSize; i++)
                    G.CharLineBuff[BufferIndex+i] = G.CharLineBuff2[i];
               for (i=0; i<G.CurrPos+2; i++)
                    G.LineBuff[i].ch = G.CharLineBuff[i];

               /* maya 28/2/1993 */
               /* if (RETURN_CODE != 0) 
	          TRACE("/aix3.2/maya/out", "5   Ret Code from layout_object_editshape = %d\n", RETURN_CODE);  */

               W[1] = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos-1].ch,
                                  G.LineBuff[G.CurrPos-1].attr,
                                  (unsigned long)G.PRT_PSM_W[1]);

               W[2] = (G.CurrPos == 1) ? 0 : GET_PSM_CWIDTH(
                                      (unsigned long)G.LineBuff[G.CurrPos-2].ch,
                                      G.LineBuff[G.CurrPos-2].attr,
                                      (unsigned long)G.PRT_PSM_W[2]);
               G.LINE_LENGTH += (W[1]+W[2]);
               FlushLine();
               RestoreFlag = 0;
               G.PRT_PSM_W[2] = 0;
               G.PRT_PSM_W[1] = 0;
               G.PRT_PSM_W[0] = PSM_SAVED_CWIDTH ;

               if (ch != 0xF6){
                 if (TASHKEEL && (G.BIDI_ATTR | 0xF0F0FFFF == 0xF0F0FFFF))  /* if Visual LTR */
                 {
                   G.UpperVowelBuff[G.CurrPos].ch = CurrUpperVowel;
                   G.LowerVowelBuff[G.CurrPos].ch = CurrLowerVowel;
                   G.UpperVowelBuff[G.CurrPos].attr = G.LowerVowelBuff[G.CurrPos].attr = ps & 0xFBDFFFFF;  /* To disable under/over score */
                 } /* end if TASHKEEL */

                 G.LineBuff[G.CurrPos].ch = (unsigned char)ch;
                 G.LineBuff[G.CurrPos].attr = ps;

		 for (i=0; i<G.CurrPos+1; i++)
                      G.CharLineBuff[i] = G.LineBuff[i].ch;

                 BufferIndex   = (unsigned long)(G.CurrPos-1);
                 OutBufferSize = InpBufferSize = (int)(G.CurrPos+1);

                 RETURN_CODE = layout_object_editshape(plh, effect,
                                                       &BufferIndex,
                                                       (unsigned char *)G.CharLineBuff,
                                                       &InpBufferSize,
                                                       (unsigned char *)G.CharLineBuff2,
                                    		       &OutBufferSize);

                 for (i=0; i<OutBufferSize; i++)
                      G.CharLineBuff[BufferIndex+i] = G.CharLineBuff2[i];
                 for (i=0; i<G.CurrPos+1; i++)
                      G.LineBuff[i].ch = G.CharLineBuff[i];

                 /* maya 28/2/1993 */
                 /* if (RETURN_CODE != 0) 
	            TRACE("/aix3.2/maya/out", "6   Ret Code from layout_object_editshape = %d\n", RETURN_CODE); */

                  W[0] = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos].ch,
                                        G.LineBuff[G.CurrPos].attr,
                                        (unsigned long)G.PRT_PSM_W[0]);
                  G.LINE_LENGTH += W[0];
                  G.CurrPos++;
               }
            }
            else{
               W[0] = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos].ch,
                                     G.LineBuff[G.CurrPos].attr,
                                     (unsigned long)G.PRT_PSM_W[0]);
               G.LINE_LENGTH += W[0];
               G.CurrPos++;
            }
         }
       break;

       case 0x01:
       case 0x11:
       case 0x12:

         for (i=0; i<G.CurrPos+2; i++)
              G.CharLineBuff[i] = G.LineBuff[i].ch;

         BufferIndex   = (unsigned long)(G.CurrPos);
         OutBufferSize = InpBufferSize = (int)(G.CurrPos+2);

         RETURN_CODE = layout_object_editshape(plh, effect,
                                               &BufferIndex,
                                               (unsigned char *)G.CharLineBuff,
                                               &InpBufferSize,
                                               (unsigned char *)G.CharLineBuff2,
                                               &OutBufferSize);

         for (i=0; i<OutBufferSize; i++)
              G.CharLineBuff[BufferIndex+i] = G.CharLineBuff2[i];
         for (i=0; i<G.CurrPos+2; i++)
              G.LineBuff[i].ch = G.CharLineBuff[i];

         /* maya 28/2/1993 */
         /* if (RETURN_CODE != 0) 
           TRACE("/aix3.2/maya/out", "7    Ret Code from layout_object_editshape = %d\n", RETURN_CODE); */

         G.CWIDTH = GET_PSM_CWIDTH(ch,
                                 ps,
                                 (unsigned long)PSM_SAVED_CWIDTH );

         RestoreFlag = WriteToBuffer(ch, ps);
       break;

       case 0x10:
         switch ((unsigned int)ch){
           case 0xF8:
           case 0x9D:
           case 0xFA:
           case 0x9F:
           case 0xF7:
           case 0x9C:
             G.LineBuff[G.CurrPos].ch = (unsigned char)ch;
             G.LineBuff[G.CurrPos].attr = ps;

             for (i=0; i<MAX_BUFFER_LENGTH; i++)
                  G.CharLineBuff[i] = G.LineBuff[i].ch;
  
             BufferIndex   = (unsigned long)(G.CurrPos);
             OutBufferSize = InpBufferSize = (int)(MAX_BUFFER_LENGTH);

             RETURN_CODE = layout_object_editshape(plh, effect,
                                                   &BufferIndex,
                                                   (unsigned char *)G.CharLineBuff,
                                                   &InpBufferSize,
                                                   (unsigned char *)G.CharLineBuff2,
                                           	   &OutBufferSize);

             for (i=0; i<OutBufferSize; i++)
                  G.CharLineBuff[BufferIndex+i] = G.CharLineBuff2[i];
             for (i=0; i<MAX_BUFFER_LENGTH; i++)
                  G.LineBuff[i].ch = G.CharLineBuff[i];

         /* maya 28/2/1993 */
         /* if (RETURN_CODE != 0) 
	   TRACE("/aix3.2/maya/out", "8   Ret Code from layout_object_editshape = %d\n", RETURN_CODE); */

             G.CWIDTH = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos].ch,
                                     G.LineBuff[G.CurrPos].attr,
                                     (unsigned long)PSM_SAVED_CWIDTH );
             G.S1 = G.LineBuff[G.CurrPos+1].ch;
           RestoreFlag = WriteToBuffer((unsigned long)G.LineBuff[G.CurrPos].ch,
                                        ps);
           break;

           default:

             for (i=0; i<G.CurrPos+2; i++)
                  G.CharLineBuff[i] = G.LineBuff[i].ch;

             BufferIndex   = (unsigned long)(G.CurrPos);
             OutBufferSize = InpBufferSize = (int)(G.CurrPos+2);


             RETURN_CODE = layout_object_editshape(plh, effect,
                                                   &BufferIndex,
                                                   (unsigned char *)G.CharLineBuff,
                                                   &InpBufferSize,
                                                   (unsigned char *)G.CharLineBuff2,
                                                   &OutBufferSize);

             for (i=0; i<OutBufferSize; i++)
                  G.CharLineBuff[BufferIndex+i] = G.CharLineBuff2[i];
             for (i=0; i<G.CurrPos+2; i++)
                  G.LineBuff[i].ch = G.CharLineBuff[i];

            /* maya 28/2/1993 */
            /* if (RETURN_CODE != 0) 
               TRACE("/aix3.2/maya/out", "9    Ret Code from layout_object_editshape = %d\n", RETURN_CODE); */

             G.CWIDTH = GET_PSM_CWIDTH(ch,
                                     ps,
                                     (unsigned long)PSM_SAVED_CWIDTH );

             RestoreFlag = WriteToBuffer(ch, ps);
           break;
         }                                                /* endswitch */
       break;

       case 0x13:
       case 0x14:
         switch ((unsigned int)ch){
           case 0xA6:
              G.LineBuff[G.CurrPos].ch = (unsigned char)ch;
              G.LineBuff[G.CurrPos].attr = ps;
              G.CWIDTH = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos].ch,
                                      G.LineBuff[G.CurrPos].attr,
                                      (unsigned long)G.CWIDTH);
              G.S1 = G.LineBuff[G.CurrPos].ch;

            RestoreFlag = WriteToBuffer((unsigned long)G.LineBuff[G.CurrPos].ch,
                                           ps);
           break;

           default:

             for (i=0; i<G.CurrPos+2; i++)
                  G.CharLineBuff[i] = G.LineBuff[i].ch;

	     BufferIndex   = (unsigned long)(G.CurrPos);
             OutBufferSize = InpBufferSize = (int)(G.CurrPos+2);

             RETURN_CODE = layout_object_editshape(plh, effect,
                                                   &BufferIndex,
                                                   (unsigned char *)G.CharLineBuff,
                                                   &InpBufferSize,
                                                   (unsigned char *)G.CharLineBuff2,
    						   &OutBufferSize);

             for (i=0; i<OutBufferSize; i++)
                  G.CharLineBuff[BufferIndex+i] = G.CharLineBuff2[i];
             for (i=0; i<G.CurrPos+2; i++)
                  G.LineBuff[i].ch = G.CharLineBuff[i];

           /* maya 28/2/1993 */
           /* if (RETURN_CODE != 0) 
              TRACE("/aix3.2/maya/out", "10   Ret Code from layout_object_editshape = %d\n", RETURN_CODE); */

              G.CWIDTH = GET_PSM_CWIDTH((unsigned long)G.LineBuff[G.CurrPos].ch,
                                      G.LineBuff[G.CurrPos].attr,
                                      (unsigned long)PSM_SAVED_CWIDTH );

              RestoreFlag = WriteToBuffer(ch, ps);
           break;
                                                                 /* endswitch */
       break;
     }                                                           /* endswitch */

     if (RestoreFlag == 1){
        G.CWIDTH = PSM_SAVED_CWIDTH ;
    }
     return;
  }
}
