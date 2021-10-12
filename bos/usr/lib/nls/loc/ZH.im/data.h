/* @(#)42	1.1  src/bos/usr/lib/nls/loc/ZH.im/data.h, ils-zh_CN, bos41B, 9504A 12/19/94 14:38:15  */
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* VK from the keyboard driver */
#define VK_KANA             0x15        /*1993.4.22 append from windows.h*/
#define VK_ROMAJI           0x16
#define VK_ZENKAKU          0x17
#define VK_HIRAGANA         0x18
#define VK_KANJI            0x19


#define VK_DANYINHAO 0xc0      /* [,]  char = 0x60 */
#define VK_JIANHAO   0xbd      /* [-]  char = 0x2d */
#define VK_DENGHAO   0xbb      /* [=]  char = 0x3d */
#define VK_ZUOFANG   0xdb      /* "["  char = 0x5b */
#define VK_YOUFANG   0xdd      /* "]"  char = 0x5d */
#define VK_FENHAO    0xba      /* [;]  char = 0x3b */
#define VK_ZUODAN    0xde      /* [']  char = 0x27 */
#define VK_DOUHAO    0xbc      /* [,]  char = 0x2c */
#define VK_JUHAO     0xbe      /* [.]  char = 0x2d */
#define VK_SHANGXIE  0xbf      /* [ ]  char = 0x2f */
#define VK_XIAXIE    0xdc      /* [\]  char = 0x5c */

#define WM_NEW_WORD 1992+0x400

#define STC FALSE
#define CLC TRUE
#define REINPUT 2
#define RECALL  3
#define BACKWORD_KEY  0x802d
#define FORWORD_KEY   0x803d
#define BIAODIAN_ONLY -2


/*  Definitions of input step_mode (STD, SD) */

#define START       0    /* the step_mode before pinyin inputing */
#define SELECT      1    /* after convert                        */
#define RESELECT    2    /* after select and can be reselect by  */
                         /* FORCE SELECT KEY.                    */
#define ONINPUT     3    /* During Inputing Progress.            */
#define ONCOVERT    4    /* While Converting.                    */
#define SETOPTION   10   /* Setting ABC Input Method Options     */

/* input information (in.info_flag) definitions */

#define BY_RECALL 1
#define BY_WORD  0x80
#define BY_CHAR  0x81


/* Input msg type definitions (STD,SD) */

#define NORMAL 0        /* Normal pinyin string */
#define ABBR   1        /* First letter is capital */
#define CPAPS_NUM 2     /* Capital Chinese number (identifer is "I") */
#define CSMALL_NUM 3    /* Small chinese number (identifer is "i") */
#define USER_extern WORDS 4    /* Look for user words */
#define BACK_extern WORDS 12   /* Reduce convert poextern inter for a word. */
#define CONTINUE   13   /* Continue converting. */

/* Converitng return msg definitions(STD and SD) */

#define NO_RESULT -1    /* Un-successful converting */
#define SUCCESS   1     /* Converting has results. */


#define EXPAND_TABLE_LENGTH       0x0BBE0


#define BX_LIB_START_POINTER      0L
#define BX_LIB_LENGTH             0x5528

#define DTKB_START_POINTER_LOW    0x05600
#define DTKB_START_POINTER_HI     0
#define DTKB_LENGTH               0x0A00
#define DTKB_CHECK_VALUE          0x55EB

#define HELP_LOW                  0x06000H
#define HELP_HI                   0
#define HELP_LENGTH               0x600

#define BHB_START_POINTER_LOW     0x6780
#define BHB_START_POINTER_HI      0
#define BHB_LENGTH                0x54A0

#define BHB_CX_LOW                0x0A1c0
#define BHB_CX_HI                 0
#define BHB_CX_LENGTH             0x1A20
#define BHB_CHECK_VALUE           0x049FC


#define PTZ_LIB_START_POINTER      0x0BBE0L
#define PTZ_LIB_LENGTH             0x4430
#define PTZ_LIB_LONG               0x400l

#define PD_START_POINTER           0x10010l
#define PD_LENGTH                  0x1160

#define SPBX_START_POINTER         0x111E0l
#define SPBX_LENGTH                6784

/* TOTAL LENGTH OF THE OVERLAY FILE=12CA0H */

#define TMMR_LIB_LENGTH             0x1800
#define TMMR_REAL_LENGTH            0x1800

#define PAREMETER_LENGTH            0x10

#define FRONT_LIB_START_POINTER_HI   0
#define FRONT_LIB_START_POINTER_LOW  0
#define FRONT_LIB_LENGTH             TMMR_LIB_LENGTH

#define MIDDLE_REM                   0x1400
#define BHB_PROC_OFFSET              0

#define LENGTH_OF_USER            0x0A000l

#define NDX_REAL_LENGTH              0x510

#define CHECK_POINT  1024+2048-4
#define CHECK_POINT2 48-4
/*
#define ABCOPTION_AUXROWMAX           9
*/

#define input_msg_disp                0


struct INPUT_TISHI {
			unsigned char buffer[6];
			};

/*                 1994.1.7
struct INPT_BF{
		WORD max_length;
		WORD true_length;
		BYTE info_flag;
		BYTE buffer[40];
		};
*/
struct INPT_BF{
		WORD max_length;
		WORD true_length;
		BYTE info_flag;
		BYTE buffer[60];
		};

struct W_SLBL{
			BYTE dw_stack[20];
			WORD dw_count;
			WORD yj[20];
			 BYTE syj[20];
			WORD tone[20];
			BYTE bx_stack[20];
			BYTE cmp_stack[20];
			WORD yj_ps[20];
			int yjs;
			int xsyjs;
			int xsyjw;
			int syyjs;
			};

struct ATTR{
			BYTE pindu;
			BYTE from;
			WORD addr;
		   };


struct STD_LIB_AREA{
					 WORD two_end;
					 WORD three_end;
					 WORD four_end;
					 BYTE buffer[0x800-6];
					};

struct INDEX{
				WORD body_start;
				WORD ttl_length;
				WORD body_length;
				WORD index_start;
				WORD index_length;
				WORD unused1;
				WORD ttl_words;
				WORD two_words;
				WORD three_words;
				WORD four_words;
				WORD fiveup_words;
				WORD unused2[13 ];
				WORD dir[((23*27)+7)/8*8];
			};

struct USER_LIB_AREA{
					 WORD two_end;
					 WORD three_end;
					 WORD four_end;
					 BYTE buffer[0x400-6];
					};

struct TBF{
			WORD *t_bf_start;
			WORD *t_bf1;
			WORD *t_bf2;
			WORD *bx;
			BYTE *attr;
		  };

struct PD_TAB{
				WORD pd_bf0[8];
				BYTE pd_bf1[((55-16+1)*94+15)/16*16];
				BYTE pd_bf2[0x4f0];
			 };


struct FMT{
			WORD fmt_group;
			WORD fmt_ttl_len;
			WORD fmt_start;
			};


struct T_REM{
				WORD stack1[512];
				WORD stack2[1024];
				WORD stack3[512];
				WORD temp_rem_area[512];
				WORD rem_area[512];
			};

struct M_NDX{
	      WORD mulu_start_hi;
	      WORD mulu_start_low;
	      WORD mulu_length_max;
	      WORD mulu_true_length;
	      WORD mulu_record_length;
	      WORD data_start_hi;
	      WORD data_start_low;
	      WORD data_record_length;
	     };

struct S_HEAD{
	     BYTE flag;
	     BYTE name;
	     WORD start_pos;
	     WORD item[25];

	     };

struct DEX{
				WORD body_start;
				WORD ttl_length;
				WORD body_length;
				WORD index_start;
				WORD index_length;
				WORD unused1;
				WORD ttl_words;
				WORD two_words;
				WORD three_words;
				WORD four_words;
				WORD fiveup_words;
				WORD unused2[13 ];
				struct S_HEAD dex[23];
				WORD  unuserd2[0x510/2-23*27-24];
			};

