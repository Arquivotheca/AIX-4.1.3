/* @(#)31	1.2  src/bos/usr/bin/bprt/prt-proc.h, libbidi, bos411, 9428A410j 11/4/93 15:33:45 */
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
/*****************************HEADER FILE HEADER*******************************/
/*                                                                            */
/* HEADER FILE NAME: PRT-PROC.H (Processor)                                   */
/*                                                                            */
/* FUNCTION:   This module contains the bidirectional processing routines     */
/*             that are called by the dispatcher to process a certains SFN.   */
/*                                                                            */
/******************************************************************************/

struct Buffer {
        unsigned char  	ch;
        unsigned long  	attr;
	unsigned char 	*escapes;	/* like esc [ I and esc [ @ */
	int 	        escsize;
        };

#define MAX_BUFFER_LENGTH 255L

static char DOUBLE_HEIGHT_END_ESC[]={27,'[','@',0x04,0x00,0x00,0x00,0x01,0x01};
/* 4/8/1993  TASHKEEL */
/* These three ESCapes move the printer head vertically to correctly */
/* position the vowels and consonants on top of each other.	     */	
static char UPPER_ESC[3] = {27, 74, 4};
static char CONSONANT_ESC[3] = {27, 74, 10};
static char LOWER_ESC[3] = {27, 74, 9};
/* 4/8/1993  TASHKEEL */



typedef struct{
        unsigned char 	CharLineBuff[MAX_BUFFER_LENGTH];     /* These are working buffers used in the */ 
                                                             /* layout_object_transform() and the */ 
        unsigned char 	CharLineBuff2[MAX_BUFFER_LENGTH];    /* layout_object_editshape() functions */
        struct Buffer   UpperVowelBuff[MAX_BUFFER_LENGTH];   /* 4/8/1993 TASHKEEL */
        struct Buffer   LineBuff[MAX_BUFFER_LENGTH];
        struct Buffer   LowerVowelBuff[MAX_BUFFER_LENGTH];   /* 4/8/1993 TASHKEEL */
        unsigned long   PRESENTION_STATUS,
                        PREV_CHAR_ATTR,
                        GRAPHICS_CHAR_COUNT,
                        IGNORE_COUNT,
			SAVED_GRAPHICS,
                        BIDI_ATTR;
        unsigned int    PRT_LINE_LENGTH,
 			INSERT_SPACES_START,   		     /* 4/8/1993 TASHKEEL */
 			INSERT_SPACES_END,   		     /* 4/8/1993 TASHKEEL */
                        LINE_LENGTH,
                        CWIDTH,
                        PRT_PSM_W[3],
                        CurrPos,
                        SELECT_FLAG,
                        COUNT_DH,
                        PRT_DFLT_LENGTH,
                        START_FUNC,
			OTHER_PITCH;
        unsigned char   PRT_TABS[28],
                        ORIENTATION,
                        PRT_CURR_TAB,
                        ZERO_TERM_FLAG,
                        PRT_ESC_OFFSET,
                        END_LINE_FLAG,
                        InCompFlag,
                        ESC_OFFSET,
                        COUNT[4],
                        PRT_L_MARGIN,
                        PRT_R_MARGIN,
                        SFN_PRT_LAST,
                        COND_EMP,
                        DOUBLE_HEIGHT_FLAG,
                        DOUBLE_HEIGHT_ESC[9],
			ESC_I_FLAG,
                        ESC_I_SEQ[3],
			ESC_BRKT_I [13],
			BRACKET_FUNCTIONS,
			PRT_NUM,
                        PITCH;
        char            S1,
                        DEF_ATTR,
                        ESC_SubCode ;
              } EntryPoint;


extern void ProcessorInitialize ( void );
extern void Processor           ( void );

#define   DEFAULT_BIDI_ATTRIBUTES 0x00001101L
#define CP864  0
#define CP1046 1
#define CP862  2

unsigned long WriteToBuffer (unsigned long ch, unsigned long ps);
unsigned int  GET_PSM_CWIDTH( unsigned long ch, unsigned long ps,
                              unsigned long tempwidth);
void          InsertSpaces(void);
void          ProcessData(void);
void 	      PostProcessTashkeel(void);
void          PostProcess(struct Buffer *ProcessBuff);
void          InitializeBuffer(void);
void          FirstInitialize(void);
void          PSM_WriteToBuffer(unsigned long ch,
                                unsigned long ps);
unsigned long GetTheByte ( char *AttStr,
                           unsigned long ByteNumber );
void          PutByte ( char *AttStr,
                        unsigned long ByteNumber,
                        unsigned long AttByte );
void          UpdatePresStatus(unsigned long *PRESENTION_STATUS,
                               unsigned long attr);


void          AdjustWidth(unsigned long newpitch, unsigned long attr);



