/* @(#)59       1.2  src/bos/usr/lib/pios/fmtrs/piofasci.h, cmdpios, bos411, 9428A410j 6/11/91 10:44:16 */
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*******************************************************************************
*  piofasci.h - Print formatter for simple ascii data stream
*******************************************************************************/



/*******************************************************************************
*                         Integer and String Variables                         *
*******************************************************************************/

/* Bottom Margin (number of blank lines); Sometimes Called Skip Perf. */
#define Bmarg (*(_Bmarg + piomode))  /* variable name */
int *_Bmarg;   /* pointer to the variable */

/* Page Number Where Printing Should Begin */
#define Beginpg (*(_Beginpg + piomode))  /* variable name */
int *_Beginpg;   /* pointer to the variable */

/* Indent Value (characters) */
#define Indent (*(_Indent + piomode))  /* variable name */
int *_Indent;   /* pointer to the variable */

/* Initialize the printer (for each print file)? (!: no;  +: yes) */
#define Init_printer (*(_Init_printer + piomode))  /* variable name */
int *_Init_printer;   /* pointer to the variable */

/* Page Length (lines; 0 means ignore page length) */
#define Pglen (*(_Pglen + piomode))  /* variable name */
int *_Pglen;   /* pointer to the variable */

/* Pitch (characters per inch) */
#define Pitch (*(_Pitch + piomode))  /* variable name */
int *_Pitch;   /* pointer to the variable */

/* Top Margin (number of blank lines) */
#define Tmarg (*(_Tmarg + piomode))  /* variable name */
int *_Tmarg;   /* pointer to the variable */

/* Line Density (lines per inch) examples: 6, 8 */
#define Lpi (*(_Lpi + piomode))  /* variable name */
int *_Lpi;   /* pointer to the variable */

/* Page Width (characters) */
#define Pgwidth (*(_Pgwidth + piomode))  /* variable name */
int *_Pgwidth;   /* pointer to the variable */

/* 0: CR->CR, LF->LF, VT->VT;  1: CR->CRLF  2: LF->CRLF, VT->CRVT; */
#define Auto_crlf (*(_Auto_crlf + piomode))  /* variable name */
int *_Auto_crlf;   /* pointer to the variable */

/* Rotate Page (!; no = portrait  +; yes = landscape) */
#define Rotation (*(_Rotation + piomode))  /* variable name */
int *_Rotation;   /* pointer to the variable */

/* Double spacing flag (!; no = single  +; yes = double) */
#define Doublespace (*(_Doublespace + piomode))  /* variable name */
int *_Doublespace;   /* pointer to the variable */

/* Restore the Printer at the End of the Print Job? (!: no;  +: yes) */
#define Restoreprinter (*(_Restoreprinter + piomode))  /* variable name */
int *_Restoreprinter;   /* pointer to the variable */

/* Wrap Long Lines (!: no;  +: yes) */
#define Wrap (*(_Wrap + piomode))  /* variable name */
int *_Wrap;   /* pointer to the variable */

/* Issue Form Feed Between Copies & At Job End (!: no;  +: yes) */
#define Do_formfeed (*(_Do_formfeed + piomode))  /* variable name */
int *_Do_formfeed;   /* pointer to the variable */

/* Number of Units Into Which a Horizontal Inch Is Divided */
#define Hres (*(_Hres + piomode))  /* variable name */
int *_Hres;   /* pointer to the variable */

/* Number of Units Into Which a Vertical Inch Is Divided */
#define Vres (*(_Vres + piomode))  /* variable name */
int *_Vres;   /* pointer to the variable */

/* Integer Set By Formatter For Use By Various Commands */
#define Workint1 (*(_Workint1 + piomode))  /* variable name */
int *_Workint1;   /* pointer to the variable */

/* Integer Set By Formatter For Use By Various Commands */
#define Workint2 (*(_Workint2 + piomode))  /* variable name */
int *_Workint2;   /* pointer to the variable */

/* Work Variable For Vertical Decrement (in Vres units) */
#define Vdecr (*(_Vdecr + piomode))  /* variable name */
int *_Vdecr;   /* pointer to the variable */

/* Work Variable In Which To Save Horizontal Position From Previous Line */
#define Hpos_from_prev (*(_Hpos_from_prev + piomode))  /* variable name */
int *_Hpos_from_prev;   /* pointer to the variable */

/* Work Variable For Horizontal Increment Per Character (in Hres units) */
#define Hincr (*(_Hincr + piomode))  /* variable name */
int *_Hincr;   /* pointer to the variable */

/* Work Variable For Vertical Increment (in Vres units) */
#define Vincr (*(_Vincr + piomode))  /* variable name */
int *_Vincr;   /* pointer to the variable */

/* Code Page Name For Print Data Stream (file with same name in dir. "d1") */
#define Codepagename (*(_Codepagename + piomode)) /* variable name */
struct str_info *_Codepagename;	/* pointer to the variable */

/* Name of Current Form Feed Command */
#define Ff_cmd (*(_Ff_cmd + piomode))  /* variable name */
struct str_info *_Ff_cmd;   /* pointer to the variable */

/* Current Horizontal Tab Settings (terminated by null) */
#define Horz_tabs (*(_Horz_tabs + piomode))  /* variable name */
struct str_info *_Horz_tabs;   /* pointer to the variable */

/* Name of Current Vertical Increment Command */
#define Vincr_cmd (*(_Vincr_cmd + piomode))  /* variable name */
struct str_info *_Vincr_cmd;   /* pointer to the variable */

/* Number of Lines to Space Down For Each Line Feed */
#define Linespacing (*(_Linespacing + piomode))  /* variable name */
int *_Linespacing;   /* pointer to the variable */

/* Pass-Through Instead of Formatting (i.e., data stream is not modified)? */
#define Passthru (*(_Passthru + piomode))  /* variable name */
int *_Passthru;   /* pointer to the variable */

/* Current Vertical Tab Settings (terminated by null) */
#define Vert_tabs (*(_Vert_tabs + piomode))  /* variable name */
struct str_info *_Vert_tabs;   /* pointer to the variable */

/* Work Variable For Page Width (in Hres units) */
#define Width (*(_Width + piomode))  /* variable name */
int *_Width;   /* pointer to the variable */

/* Name of Current Vert. Decrement Command */
#define Vdecr_cmd (*(_Vdecr_cmd + piomode))  /* variable name */
struct str_info *_Vdecr_cmd;   /* pointer to the variable */



/*******************************************************************************
*                               String Constants                               *
*******************************************************************************/

/* Directory Containing Stage 1 Translate Tables (data stream to intermed.) */
#define STG1_XLATE_DIR "d1"

/* Directory Containing Stage 2 Translate Tables (intermediate to printer) */
#define STG2_XLATE_DIR "d2"

/* Path Name of Font File To Be Downloaded (must include download commands) */
#define DOWNLOAD_FONT  "mF"

/* Character To Be Used When Real One Can't Be Printed */
#define SUBSTCHAR      "mS"

/* Path Name of Stage 2 Translate Table */
#define BASE_TABLE	"t0"

/* Path Name of First Alternate Stage 2 Translate Table */
#define TABLE_1		"t1"

/* Path Name of Next Alternate Stage 2 Translate Table */
#define TABLE_2		"t2"

/* ditto */
#define TABLE_3		"t3"

/* ditto */
#define TABLE_4		"t4"

/* ditto */
#define TABLE_5		"t5"

/* ditto */
#define TABLE_6		"t6"

/* ditto */
#define TABLE_7		"t7"

/* ditto */
#define TABLE_8		"t8"

/* ditto */
#define TABLE_9		"t9"

/* Initialize the Printer */
#define INIT_CMD       "ci"

/* Restore the Printer at Job End */
#define REST_CMD       "cr"

/* Command When Switching Between Pri. Mode & Alt. (security label) Mode */
#define SWITCHMODE_CMD "cm"

/* Send Carriage Return & Line Feed Controls */
#define CRLF_CMD       "cl"

/* Space Horizontally In Hres Units (used for ident, LF w/o CR) */
#define HORZ_SPACE_CMD "ch"

/* No command necessary to select the IBM-437 character set */
#define SEL_BASE_SET   "c1"

/* Tell the printer what the Horizontal Motion Index is */
#define SET_SPACE_WDTH "cs"

/* Null */
#define NULL_CMD       "an"

/* ASCII Control Code to Ring the Bell Once */
#define BEL_CMD        "ar"

/* ASCII Control Code to Backspace One Characer */
#define BS_CMD         "ab"

/* ASCII Control Code to Advance the Paper One Line (LF without CR) */
#define LF_CMD         "al"

/* ASCII Control Code to Advance the Paper to Top of Next Page (FF) */
#define FF_CMD         "af"

/* ASCII Control Code to Return to the Left Margin (CR without LF) */
#define CR_CMD         "ac"

/* ASCII Control Code to Shift Out */
#define SO_CMD         "ao"

/* ASCII Control Code to Shift In */
#define SI_CMD         "ai"



/*******************************************************************************
*                    Table of Variables and Their Attributes                   *
*******************************************************************************/

struct attrparms attrtable[] = {
                                                              /*
 name      data type       lookup table     addr of pointer
------   --------------   --------------   ----------------   */
"_b"   , VAR_INT        , NULL           , (union dtypes *) &_Bmarg          ,
"_g"   , VAR_INT        , NULL           , (union dtypes *) &_Beginpg        ,
"_i"   , VAR_INT        , NULL           , (union dtypes *) &_Indent         ,
"_j"   , VAR_INT        , NULL           , (union dtypes *) &_Init_printer   ,
"_l"   , VAR_INT        , NULL           , (union dtypes *) &_Pglen          ,
"_p"   , VAR_INT        , NULL           , (union dtypes *) &_Pitch          ,
"_t"   , VAR_INT        , NULL           , (union dtypes *) &_Tmarg          ,
"_v"   , VAR_INT        , NULL           , (union dtypes *) &_Lpi            ,
"_w"   , VAR_INT        , NULL           , (union dtypes *) &_Pgwidth        ,
"_x"   , VAR_INT        , NULL           , (union dtypes *) &_Auto_crlf      ,
"_E"   , VAR_INT        , NULL           , (union dtypes *) &_Doublespace    ,
"_J"   , VAR_INT        , NULL           , (union dtypes *) &_Restoreprinter ,
"_L"   , VAR_INT        , NULL           , (union dtypes *) &_Wrap           ,
"_X"   , VAR_STR        , NULL           , (union dtypes *) &_Codepagename    ,
"_Z"   , VAR_INT        , NULL           , (union dtypes *) &_Do_formfeed    ,
"mH"   , VAR_INT        , NULL           , (union dtypes *) &_Hres           ,
"mV"   , VAR_INT        , NULL           , (union dtypes *) &_Vres           ,
"w1"   , VAR_INT        , NULL           , (union dtypes *) &_Workint1       ,
"w2"   , VAR_INT        , NULL           , (union dtypes *) &_Workint2       ,
"wD"   , VAR_INT        , NULL           , (union dtypes *) &_Vdecr          ,
"wF"   , VAR_INT        , NULL           , (union dtypes *) &_Hpos_from_prev ,
"wH"   , VAR_INT        , NULL           , (union dtypes *) &_Hincr          ,
"wV"   , VAR_INT        , NULL           , (union dtypes *) &_Vincr          ,
"wf"   , VAR_STR        , NULL           , (union dtypes *) &_Ff_cmd         ,
"wh"   , VAR_STR        , NULL           , (union dtypes *) &_Horz_tabs      ,
"wi"   , VAR_STR        , NULL           , (union dtypes *) &_Vincr_cmd      ,
"wl"   , VAR_INT        , NULL           , (union dtypes *) &_Linespacing    ,
"wp"   , VAR_INT        , NULL           , (union dtypes *) &_Passthru       ,
"wv"   , VAR_STR        , NULL           , (union dtypes *) &_Vert_tabs      ,
"ww"   , VAR_INT        , NULL           , (union dtypes *) &_Width          ,
"wy"   , VAR_STR        , NULL           , (union dtypes *) &_Vdecr_cmd      ,
NULL   , 0              , NULL           , NULL

};



/*******************************************************************************
*                                                                              *
*                   String of Options (Flags) for piogetopt()                  *
*                                                                              *
*   These are all the available formatter command line flags.  These flags     *
*   should be duplicated in the %f[] segment of the pipeline defined in the    *
*   formatter database.                                                        *
*                                                                              *
*******************************************************************************/

#define OPTSTRING  "b:g:i:j:l:t:w:x:J:L:X:Z:"

