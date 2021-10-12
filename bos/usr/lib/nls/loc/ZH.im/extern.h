/* @(#)44	1.1  src/bos/usr/lib/nls/loc/ZH.im/extern.h, ils-zh_CN, bos41B, 9504A 12/19/94 14:38:18  */
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
/*................................................................*/
/*               function tables definition                       */
/*                                                                */
extern char fk_tab[];
extern WORD sfx_table[];

extern struct INPT_BF in;
extern struct INPT_BF kbf;
extern struct W_SLBL wp;
extern struct ATTR msx_area[];
extern struct INDEX ndx;
extern struct INDEX kzk_ndx;
extern struct TBF  cisu;
extern struct PD_TAB pindu;
extern struct FMT now;
extern struct T_REM tmmr;

extern BYTE spbx_tab[];
extern int form[];

extern WORD sfx_table_size;

extern BYTE logging_stack[];
extern WORD logging_stack_size;
						
/*******************************************
 display result area DATA
**********************************************/
	extern int word_long;
    extern int unit_length;          /* single word */
    extern int disp_tail;
    extern int disp_head;
    extern int group_no;
	extern int current_no;
    extern unsigned char space_char[];
    extern BYTE out_svw[];
    extern unsigned char group_counter[];
	
/*********************************************
 input area DATA
**********************************************/
extern int input_cur;
extern int new_no;
extern int jlxw_mode;
extern int jiyi_mode;
extern int leibie;

/******************************************************
 result area DATA
*******************************************************/
extern int result_area_pointer;
extern BYTE result_area[60];
extern BYTE out_bfb[40];
extern int out_pointer;
extern int now_cs;
extern int now_cs_dot;


/********************************************
 biaodian table
*********************************************/
extern unsigned char biaodian_table[];
extern unsigned char cc_biaodian[];
extern int biaodian_pos;
extern WORD biaodian_value;
extern BYTE yinhao_flag;

/*******************************************
 control variabe
********************************************/
extern BYTE bdd_flag;          /* the function of punctuation switch */
extern BYTE bdd_flag_keep;
extern BYTE int_asc_mode;    /* extern int or half character switch */
extern BYTE step_mode;
extern BYTE cp_ajust_flag;
extern BYTE word_back_flag;
extern BYTE msg_type;

/*****************************************
temp memory area
******************************************/
extern BYTE temp_rem_area[512];
extern BYTE rem_area[512];

/******************************************
pindu adjust parameter
********************************************/
extern BYTE auto_mode;
extern BYTE auto_mode_keep;

/**************************************
display buffer parameter
***************************************/
extern int out_length;
extern int last_out_length;
extern int cap_mode;

/******************************************
user_definition parameter
*******************************************/
extern WORD mulu_record_length;
extern WORD data_record_length;
extern WORD mulu_true_length;
extern WORD data_start;
extern WORD mulu_max_length;
extern BYTE user_word_max_length;


extern  BYTE jiyi_pindu;

extern BYTE int_asc_mode;
extern BYTE cp_ajust_flag;
extern BYTE bdd_flag;
extern BYTE cbx_flag;
extern BOOL IfTopMost;

/**************************************
Added by Austin
**************************************/
extern BYTE territory;
extern BYTE out_svw[1100];

#ifndef _DICREC
#define _DICREC
struct DICREC {
              long startp;
              long length;
};
#endif

#ifndef _FHEAD
#define _FHEAD
struct FILEHEAD {
                struct DICREC total;
                struct DICREC csm;
                struct DICREC dyj;
                struct DICREC jf;
                struct DICREC grp;
                BYTE idflag[8];
}head;
#endif

extern struct FILEHEAD head;

extern WORD jp_index[32];
extern WORD qp_index[520];

#ifndef _PYML
#define _PYML
extern struct YJNDX{
               BYTE head;
               BYTE *pt;
               WORD num;
             };
extern struct YJNDX pyml_index[];
extern BYTE pymlfb[];
extern BYTE pymla[];
extern BYTE pymlb[];
extern BYTE pymlc[];
extern BYTE pymld[];
extern BYTE pymle[];
extern BYTE pymlf[];
extern BYTE pymlg[];
extern BYTE pymlh[];
extern BYTE pymlj[];
extern BYTE pymlk[];
extern BYTE pymll[];
extern BYTE pymlm[];
extern BYTE pymln[];
extern BYTE pymlp[];
extern BYTE pymlq[];
extern BYTE pymlr[];
extern BYTE pymls[];
extern BYTE pymlt[];
extern BYTE pymlw[];
extern BYTE pymlx[];
extern BYTE pymly[];
extern BYTE pymlz[];
#endif

extern BYTE slbl_tab[];

