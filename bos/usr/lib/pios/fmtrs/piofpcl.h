/* @(#)59       1.4.1.1  src/bos/usr/lib/pios/fmtrs/piofpcl.h, cmdpios, bos411, 9428A410j 2/18/92 11:33:45 */
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
*  piofpcl.h  - Definitions for a Printer Formatter                            *
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

/* Paper Source Drawer (1; drawer  2; manual  3; envelope) */
#define Paper_source (*(_Paper_source + piomode))  /* variable name */
int *_Paper_source;   /* pointer to the variable */

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

/* Font ID.  If blank the the internal fonts will be used. */
#define Font_ID (*(_Font_ID + piomode))  /* variable name */
int *_Font_ID;   /* pointer to the variable */

/* Restore the Printer at the End of the Print Job? (!: no;  +: yes) */
#define Restoreprinter (*(_Restoreprinter + piomode))  /* variable name */
int *_Restoreprinter;   /* pointer to the variable */

/* Wrap Long Lines (!: no;  +: yes) */
#define Wrap (*(_Wrap + piomode))  /* variable name */
int *_Wrap;   /* pointer to the variable */

/* Code Page Name For Print Data Stream (file with same name in dir. "d1") */
#define Codepagename (*(_Codepagename + piomode))  /* variable name */
struct str_info *_Codepagename;   /* pointer to the variable */

/* Issue Form Feed Between Copies & At Job End (!: no;  +: yes) */
#define Do_formfeed (*(_Do_formfeed + piomode))  /* variable name */
int *_Do_formfeed;   /* pointer to the variable */

/* Number of Units Into Which a Horizontal Inch Is Divided */
#define Hres (*(_Hres + piomode))  /* variable name */
int *_Hres;   /* pointer to the variable */

/* Number of Units Into Which a Vertical Inch Is Divided */
#define Vres (*(_Vres + piomode))  /* variable name */
int *_Vres;   /* pointer to the variable */

/* Printer model number */
#define Pr_model_num (*(_Pr_model_num + piomode))  /* variable name */
int *_Pr_model_num;   /* pointer to the variable */

/* Integer Set By Formatter For Use By Various Commands */
#define Workint1 (*(_Workint1 + piomode))  /* variable name */
int *_Workint1;   /* pointer to the variable */

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

/* Work Variable For Typestyle code */
#define Typestyle_code (*(_Typestyle_code + piomode))  /* variable name */
int *_Typestyle_code;   /* pointer to the variable */

/* Work Variable For Style code (0 normal, 1 italic) */
#define Style_code (*(_Style_code + piomode))  /* variable name */
int *_Style_code;   /* pointer to the variable */

/* Work Variable For Stroke Weight code (-7 to 7, where: 0 normal, 3 bold) */
#define Weight_code (*(_Weight_code + piomode))  /* variable name */
int *_Weight_code;   /* pointer to the variable */

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

/* Work Variable For Page Width (in Hres units) */
#define Width (*(_Width + piomode))  /* variable name */
int *_Width;   /* pointer to the variable */

/* Name of Current Vert. Decrement Command */
#define Vdecr_cmd (*(_Vdecr_cmd + piomode))  /* variable name */
struct str_info *_Vdecr_cmd;   /* pointer to the variable */

/* The parameterized character in an escape sequence */
#define Param (*(_Param + piomode))  /* variable name */
int *_Param;   /* pointer to the variable */

/* The group identifier */
#define Group (*(_Group + piomode))  /* variable name */
int *_Group;   /* pointer to the variable */

/* This points to the string which will hold numeric parameters found in the datastream */
#define Num_param (*(_Num_param + piomode))  /* variable name */
struct str_info *_Num_param;   /* pointer to the variable */

/* The command specifier */
#define Term (*(_Term + piomode))  /* variable name */
int *_Term;   /* pointer to the variable */

/* Typestyle */
#define Typestyle (*(_Typestyle + piomode))  /* variable name */
struct str_info *_Typestyle;   /* pointer to the variable */

/* Pitch Value In String Format */
#define Pitch_string (*(_Pitch_string + piomode))  /* variable name */
struct str_info *_Pitch_string;   /* pointer to the variable */

/* Base code page table (list of code pages to use) */
#define Base_table (*(_Base_table + piomode))  /* variable name */
struct str_info *_Base_table;   /* pointer to the variable */


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

/* List of available fonts */
#define FONTLIST       "mU"

/* Path Name of Stage 2 Translate Table */
#define BASE_TABLE     "t0"

/* Path Name of First Alternate Stage 2 Translate Table */
#define TABLE_1        "t1"

/* Path Name of Next Alternate Stage 2 Translate Table */
#define TABLE_2        "t2"

/* ditto */
#define TABLE_3        "t3"

/* ditto */
#define TABLE_4        "t4"

/* ditto */
#define TABLE_5        "t5"

/* ditto */
#define TABLE_6        "t6"

/* ditto */
#define TABLE_7        "t7"

/* ditto */
#define TABLE_8        "t8"

/* ditto */
#define TABLE_9        "t9"

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

/* Select the Roman-8 symbol set */
#define SEL_BASE_SET   "c1"

/* Select the IBM-US symbol set */
#define SEL_ALT1_SET   "c2"

/* Select the ECMA-94 (Latin 1) symbol set */
#define SEL_ALT2_SET   "c3"

/* Set line termination mode */
#define LINE_TERM_MODE "ct"

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

/* PCL Reset - not used by the 4219 */
#define RESET_CMD      "eR"

/* Send out a two by escape sequence */
#define TWO_BYTE_CMD   "eW"

/* Send out a multi byte escape sequence */
#define MULTI_BYTE_CMD "eX"

/* Send out a multi byte escape sequence followed by binary data */
#define MB_W_DATA_CMD  "eY"

/* Select the symbol set to be used */
#define SYM_SET_CMD    "eZ"



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
"_s"   , VAR_STR        , NULL           , (union dtypes *) &_Typestyle      ,
"_t"   , VAR_INT        , NULL           , (union dtypes *) &_Tmarg          ,
"_u"   , VAR_INT        , NULL           , (union dtypes *) &_Paper_source   ,
"_v"   , VAR_INT        , NULL           , (union dtypes *) &_Lpi            ,
"_w"   , VAR_INT        , NULL           , (union dtypes *) &_Pgwidth        ,
"_x"   , VAR_INT        , NULL           , (union dtypes *) &_Auto_crlf      ,
"_z"   , VAR_INT        , NULL           , (union dtypes *) &_Rotation       ,
"_E"   , VAR_INT        , NULL           , (union dtypes *) &_Doublespace    ,
"_I"   , VAR_INT        , NULL           , (union dtypes *) &_Font_ID        ,
"_J"   , VAR_INT        , NULL           , (union dtypes *) &_Restoreprinter ,
"_L"   , VAR_INT        , NULL           , (union dtypes *) &_Wrap           ,
"_X"   , VAR_STR        , NULL           , (union dtypes *) &_Codepagename   ,
"_Z"   , VAR_INT        , NULL           , (union dtypes *) &_Do_formfeed    ,
"mH"   , VAR_INT        , NULL           , (union dtypes *) &_Hres           ,
"mV"   , VAR_INT        , NULL           , (union dtypes *) &_Vres           ,
"t0"   , VAR_STR        , NULL           , (union dtypes *) &_Base_table     ,
"w1"   , VAR_INT        , NULL           , (union dtypes *) &_Workint1       ,
"wD"   , VAR_INT        , NULL           , (union dtypes *) &_Vdecr          ,
"wF"   , VAR_INT        , NULL           , (union dtypes *) &_Hpos_from_prev ,
"wH"   , VAR_INT        , NULL           , (union dtypes *) &_Hincr          ,
"wV"   , VAR_INT        , NULL           , (union dtypes *) &_Vincr          ,
"wf"   , VAR_STR        , NULL           , (union dtypes *) &_Ff_cmd         ,
"wh"   , VAR_STR        , NULL           , (union dtypes *) &_Horz_tabs      ,
"wi"   , VAR_STR        , NULL           , (union dtypes *) &_Vincr_cmd      ,
"wl"   , VAR_INT        , NULL           , (union dtypes *) &_Linespacing    ,
"wp"   , VAR_INT        , NULL           , (union dtypes *) &_Passthru       ,
"ww"   , VAR_INT        , NULL           , (union dtypes *) &_Width          ,
"wy"   , VAR_STR        , NULL           , (union dtypes *) &_Vdecr_cmd      ,
"wP"   , VAR_INT        , NULL           , (union dtypes *) &_Param          ,
"wG"   , VAR_INT        , NULL           , (union dtypes *) &_Group          ,
"wN"   , VAR_STR        , NULL           , (union dtypes *) &_Num_param      ,
"wT"   , VAR_INT        , NULL           , (union dtypes *) &_Term           ,
NULL   , 0              , NULL           , NULL

};

/*
 * For backward compatibility ---
 * attrtable2 has those work variables which are not in the
 * laserjet 2 file, but are used in the laserjet 3.  They
 * won't be loaded unless FONTLIST exists in the file.
 */
struct attrparms attrtable2[] = {
"wt"   , VAR_INT        , NULL           , (union dtypes *) &_Typestyle_code ,
"wu"   , VAR_INT        , NULL           , (union dtypes *) &_Style_code     ,
"wv"   , VAR_INT        , NULL           , (union dtypes *) &_Weight_code    ,
"wz"   , VAR_STR        , NULL           , (union dtypes *) &_Pitch_string   ,
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

#define OPTSTRING  "b:g:i:j:l:p:t:u:v:w:x:z:I:J:L:X:Z:"

