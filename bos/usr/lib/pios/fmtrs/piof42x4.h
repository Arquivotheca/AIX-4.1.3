/* @(#)49       1.3  src/bos/usr/lib/pios/fmtrs/piof42x4.h, cmdpios, bos411, 9428A410j 1/19/90 01:49:34 */
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
*  piof42x4.h - Definitions for a Printer Formatter                            *
*******************************************************************************/



/*******************************************************************************
*                         Integer and String Variables                         *
*******************************************************************************/

/* Bottom Margin (number of blank lines); Sometimes Called Skip Perf. */
#define Bmarg (*(_Bmarg + piomode))  /* variable name */
int *_Bmarg;   /* pointer to the variable */

/* Emphasized Print (!: no;  +: yes) */
#define Emphasized (*(_Emphasized + piomode))  /* variable name */
int *_Emphasized;   /* pointer to the variable */

/* Page Number Where Printing Should Begin */
#define Beginpg (*(_Beginpg + piomode))  /* variable name */
int *_Beginpg;   /* pointer to the variable */

/* Indent Value (characters) */
#define Indent (*(_Indent + piomode))  /* variable name */
int *_Indent;   /* pointer to the variable */

/* Initialize the printer (for each print file)? (!: no;  +: yes) */
#define Init_printer (*(_Init_printer + piomode))  /* variable name */
int *_Init_printer;   /* pointer to the variable */

/* Print Color */
#define Color (*(_Color + piomode))  /* variable name */
int *_Color;   /* pointer to the variable */

/* Page Length (lines; 0 means ignore page length) */
#define Pglen (*(_Pglen + piomode))  /* variable name */
int *_Pglen;   /* pointer to the variable */

/* Pitch (characters per inch)  Initially set to the value implied by the font ID. */
#define Pitch (*(_Pitch + piomode))  /* variable name */
int *_Pitch;   /* pointer to the variable */

/* Print Quality (1: draft;  2: letter) */
#define Quality (*(_Quality + piomode))  /* variable name */
int *_Quality;   /* pointer to the variable */

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

/* Double-Strike Print (!: no;  +: yes) */
#define Doublestrike (*(_Doublestrike + piomode))  /* variable name */
int *_Doublestrike;   /* pointer to the variable */

/* Italics print (!: no;  +: yes) */
#define Italics (*(_Italics + piomode))  /* variable name */
int *_Italics;   /* pointer to the variable */

/* Restore the Printer at the End of the Print Job? (!: no;  +: yes) */
#define Restoreprinter (*(_Restoreprinter + piomode))  /* variable name */
int *_Restoreprinter;   /* pointer to the variable */

/* Condensed Print (!: no;  +: yes) */
#define Condensed (*(_Condensed + piomode))  /* variable name */
int *_Condensed;   /* pointer to the variable */

/* Wrap Long Lines (!: no;  +: yes) */
#define Wrap (*(_Wrap + piomode))  /* variable name */
int *_Wrap;   /* pointer to the variable */

/* Double-Wide Print (!: no;  +: yes) */
#define Cont_dblwide (*(_Cont_dblwide + piomode))  /* variable name */
int *_Cont_dblwide;   /* pointer to the variable */

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

/* Maximum Number of Horizontal Tabs That Can Be Defined */
#define Max_htabs (*(_Max_htabs + piomode))  /* variable name */
int *_Max_htabs;   /* pointer to the variable */

/* Maximum Number of Vertical Tabs That Can Be Defined */
#define Max_vtabs (*(_Max_vtabs + piomode))  /* variable name */
int *_Max_vtabs;   /* pointer to the variable */

/* Percentage of Perf. Skip Value For Top Margin (rest for bottom margin) */
#define Perf_percent (*(_Perf_percent + piomode))  /* variable name */
int *_Perf_percent;   /* pointer to the variable */

/* Printer model number */
#define Pr_model_num (*(_Pr_model_num + piomode))  /* variable name */
int *_Pr_model_num;   /* pointer to the variable */

/* Default overstrike character = '*' */
#define O_strike_char (*(_O_strike_char + piomode))  /* variable name */
int *_O_strike_char;   /* pointer to the variable */

/* Bypass whitespace on overstrike (!: no  +:yes) */
#define O_strike_byp (*(_O_strike_byp + piomode))  /* variable name */
int *_O_strike_byp;   /* pointer to the variable */

/* Bypass whitespace on underscore (!: no  +:yes) */
#define U_score_byp (*(_U_score_byp + piomode))  /* variable name */
int *_U_score_byp;   /* pointer to the variable */

/* Integer Set By Formatter For Use By Various Commands */
#define Workint1 (*(_Workint1 + piomode))  /* variable name */
int *_Workint1;   /* pointer to the variable */

/* Work Variable For Vertical Decrement (in Vres units) */
#define Vdecr (*(_Vdecr + piomode))  /* variable name */
int *_Vdecr;   /* pointer to the variable */

/* Work Variable In Which To Save Horizontal Position From Previous Line */
#define Hpos_from_prev (*(_Hpos_from_prev + piomode))  /* variable name */
int *_Hpos_from_prev;   /* pointer to the variable */

/* Vertical Index Increment Used By ESC J & ESC 3 */
#define Gunits (*(_Gunits + piomode))  /* variable name */
int *_Gunits;   /* pointer to the variable */

/* Work Variable For Horizontal Increment Per Character (in Hres units) */
#define Hincr (*(_Hincr + piomode))  /* variable name */
int *_Hincr;   /* pointer to the variable */

/* Vertical Index Increment Used By ESC A */
#define Tunits (*(_Tunits + piomode))  /* variable name */
int *_Tunits;   /* pointer to the variable */

/* Work Variable For Vertical Increment (in Vres units) */
#define Vincr (*(_Vincr + piomode))  /* variable name */
int *_Vincr;   /* pointer to the variable */

/* Currently In Subscript Mode? (!: no;  +: yes) */
#define Subscr_mode (*(_Subscr_mode + piomode))  /* variable name */
int *_Subscr_mode;   /* pointer to the variable */

/* Currently In "line" Doublewide Mode? (!: no;  +: yes) */
#define Line_dblwide (*(_Line_dblwide + piomode))  /* variable name */
int *_Line_dblwide;   /* pointer to the variable */

/* Currently In Superscript Mode? (!: no;  +: yes) */
#define Superscr_mode (*(_Superscr_mode + piomode))  /* variable name */
int *_Superscr_mode;   /* pointer to the variable */

/* Name of Current Form Feed Command */
#define Ff_cmd (*(_Ff_cmd + piomode))  /* variable name */
struct str_info *_Ff_cmd;   /* pointer to the variable */

/* Graphics Line Spacing Used By SET_GRA_LS_CMD */
#define Graphic_linesp (*(_Graphic_linesp + piomode))  /* variable name */
int *_Graphic_linesp;   /* pointer to the variable */

/* Current Horizontal Tab Settings (terminated by null) */
#define Horz_tabs (*(_Horz_tabs + piomode))  /* variable name */
struct str_info *_Horz_tabs;   /* pointer to the variable */

/* Name of Current Vertical Increment Command */
#define Vincr_cmd (*(_Vincr_cmd + piomode))  /* variable name */
struct str_info *_Vincr_cmd;   /* pointer to the variable */

/* Number of Lines to Space Down For Each Line Feed */
#define Linespacing (*(_Linespacing + piomode))  /* variable name */
int *_Linespacing;   /* pointer to the variable */

/* Current Vertical Spacing Mode (0: ESC 0; 1: ESC 1; 2: ESC 2; 3: ESC 3) */
#define Vertspace_mode (*(_Vertspace_mode + piomode))  /* variable name */
int *_Vertspace_mode;   /* pointer to the variable */

/* Currently In Continuous Overscore Mode? (!: no;  +: yes) */
#define Cont_overscore (*(_Cont_overscore + piomode))  /* variable name */
int *_Cont_overscore;   /* pointer to the variable */

/* Pass-Through Instead of Formatting (i.e., data stream is not modified)? */
#define Passthru (*(_Passthru + piomode))  /* variable name */
int *_Passthru;   /* pointer to the variable */

/* (not used) Currently In One-Direction Print Mode? (!: no;  +: yes) */
#define L_to_r_print (*(_L_to_r_print + piomode))  /* variable name */
int *_L_to_r_print;   /* pointer to the variable */

/* Currently In Proportional Spacing Mode? (!: no;  +; yes) */
#define Prop_spacing (*(_Prop_spacing + piomode))  /* variable name */
int *_Prop_spacing;   /* pointer to the variable */

/* Line Spacing: (value/Tunits) inches per line; used by SET_LS_CMD */
#define Text_linesp (*(_Text_linesp + piomode))  /* variable name */
int *_Text_linesp;   /* pointer to the variable */

/* Currently In Continuous Underscore Mode? (!: no;  +: yes) */
#define Cont_undrscore (*(_Cont_undrscore + piomode))  /* variable name */
int *_Cont_undrscore;   /* pointer to the variable */

/* Current Vertical Tab Settings (terminated by null) */
#define Vert_tabs (*(_Vert_tabs + piomode))  /* variable name */
struct str_info *_Vert_tabs;   /* pointer to the variable */

/* Work Variable For Page Width (in Hres units) */
#define Width (*(_Width + piomode))  /* variable name */
int *_Width;   /* pointer to the variable */

/* Name of Current Vert. Decrement Command */
#define Vdecr_cmd (*(_Vdecr_cmd + piomode))  /* variable name */
struct str_info *_Vdecr_cmd;   /* pointer to the variable */

/* Overstrike (!; off  +; on) */
#define Ovrstrike_mode (*(_Ovrstrike_mode + piomode))  /* variable name */
int *_Ovrstrike_mode;   /* pointer to the variable */

/* Underscore (!; off  +; on) */
#define Undrscore_mode (*(_Undrscore_mode + piomode))  /* variable name */
int *_Undrscore_mode;   /* pointer to the variable */



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

/* (not used) Send Carriage Return & Reverse Line Feed Controls */
#define REV_CRLF_CMD   "cx"

/* Space Horizontally In Hres Units (used for ident, LF w/o CR) */
#define HORZ_SPACE_CMD "ch"

/* (not used) Select Primary (ROM) Font */
#define SEL_PRIFNT_CMD "c1"

/* (not used) Select Alternate (downloaded) Font */
#define SEL_ALTFNT_CMD "c2"

/* Set Vertical Spacing */
#define SET_VERTSP_CMD "cv"

/* Set Pitch */
#define SET_PITCH_CMD  "cp"

/* Select Condensed Mode On/Off */
#define SET_COND_CMD   "cc"

/* Set Proportional Spacing On/Off */
#define SET_PROPSP_CMD "cs"

/* Turn on or off overstike */
#define OVERSTRIKE_CMD "cO"

/* Turn on or off underscore */
#define UNDRSCORE_CMD  "cU"

/* Initialize local font and Select local character set */
#define ILF_SLCS_CMD   "cZ"

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

/* ASCII Control Code to Shift Out (i.e., set double-wide for the line) */
#define SO_CMD         "ao"

/* ASCII Control Code to Shift In (i.e., start condensed printing) */
#define SI_CMD         "ai"

/* ASCII Control Code to Select 10 Char per Inch; Cancels Cond., 12 CPI */
#define DC2_CMD        "at"

/* ASCII Control Code to Cancel Double-Wide Printing by Line */
#define DC4_CMD        "ad"

/* Set Line Spacing to 8 Lines Per Inch */
#define SET_8LPI_CMD   "e0"

/* Set Line Spacing to 7/72 Inch Per Line */
#define SET_7_72LS_CMD "e1"

/* Turn On Line Spacing Specified With SET_LS_CMD */
#define LS_ON_CMD      "e2"

/* Set Graphics Line Spacing */
#define SET_GRA_LS_CMD "e3"

/* Print All Characters */
#define PRTALL_CMD     "e8"

/* (not used; would conflict with formatter processing) Set Code Page */
#define SET_CODEPG_CMD "e9"

/* Command to Set Text Line Spacing */
#define SET_LS_CMD     "eA"

/* Turn On/Off Emphasized Printing */
#define SET_EMPH_CMD   "eE"

/* Turn On/Off Double-Strike Printing */
#define SET_DBLSTR_CMD "eG"

/* Select Quality */
#define SET_MODE_CMD   "eI"

/* Advance the Paper by a Variable Amount */
#define VAR_LS_CMD     "eJ"

/* Print Graphics at 60 dots/inch hor. & 72 dots/inch vert. */
#define GRA_60_CMD     "eK"

/* Print Graphics at 120 dots/inch hor. & 72 dots/inch vert. (low speed) */
#define GRA_120LS_CMD  "eL"

/* Select desired print color */
#define SEL_COLOR_CMD  "eM"

/* Turn On/Off Proportional Spacing */
#define PROP_SPACE_CMD "eP"

/* Turn On/Off Subscript Printing */
#define SET_SUB_CMD    "eS"

/* Turn On/Off Superscript Printing */
#define SET_SUPER_CMD  "eT"

/* (not used) Change From/To Left-to-Right To/From Two-Direction Printing */
#define LR_PRINT_CMD   "eU"

/* Turn On/Off Continuous Double-Wide Printing */
#define CONT_DW_CMD    "eW"

/* Set Left and Right Margins */
#define SET_MAR_CMD    "eX"

/* Print Graphics at 120 dots/inch hor. & 72 dots/inch vert. (normal spd) */
#define GRA_120NS_CMD  "eY"

/* Print Graphics at 240 dots/inch hor. & 72 dots/inch vert. */
#define GRA_240_CMD    "eZ"

/* Print a Single Character From the All Characters Chart */
#define PRTNEXT_CMD    "eb"

/* Set Character Spacing to 12 Characters Per Inch */
#define SET_12CPI_CMD  "ec"

/* Move to the Right n/120 Inch */
#define MOVE_RIGHT_CMD "ed"

/* (not used) Move to the Left n/120 Inch */
#define MOVE_LEFT_CMD  "ee"

/* (not used) Turn On Italics Printing */
#define SET_ITAL_CMD   "ef"

/* Turn On/Off Continuous Overscore */
#define OVERSCORE_CMD  "eh"

/* (not used) Stop Printing */
#define STOP_PRT_CMD   "ej"

/* Turn On/Off Continuous Underscore */
#define UNDERSCORE_CMD "ek"

/* Set Vertical Units for VAR_LS_CMD and SET_GRA_LS_CMD */
#define SET_VUNITS_CMD "em"

/* (not used) Set printer aspect ratio */
#define SET_ASPRAT_CMD "en"

/* (not used) High-Resolution Graphics */
#define HIRES_GRA_CMD  "eo"

/* Download Character Fonts */
#define DOWNLOAD_CMD   "eq"

/* (not used) Reverse line feed */
#define REVERSE_LF_CMD "er"

/* Generalized Pass thru command */
#define PASS_THRU      "eu"

/* (yes - this is used) Null string for ignoring pass thru commands */
#define DONT_PASS_THRU "et"

/* (not used) Alternate Graphics Mode: Print Graphics In Specified Mode */
#define AGMGRAPH_CMD   "ey"



/*******************************************************************************
*                    Table of Variables and Their Attributes                   *
*******************************************************************************/

struct attrparms attrtable[] = {
                                                              /*
 name      data type       lookup table     addr of pointer
------   --------------   --------------   ----------------   */
"_b"   , VAR_INT        , NULL           , (union dtypes *) &_Bmarg          ,
"_e"   , VAR_INT        , NULL           , (union dtypes *) &_Emphasized     ,
"_g"   , VAR_INT        , NULL           , (union dtypes *) &_Beginpg        ,
"_i"   , VAR_INT        , NULL           , (union dtypes *) &_Indent         ,
"_j"   , VAR_INT        , NULL           , (union dtypes *) &_Init_printer   ,
"_k"   , VAR_INT        , colors         , (union dtypes *) &_Color          ,
"_l"   , VAR_INT        , NULL           , (union dtypes *) &_Pglen          ,
"_p"   , VAR_INT        , NULL           , (union dtypes *) &_Pitch          ,
"_q"   , VAR_INT        , NULL           , (union dtypes *) &_Quality        ,
"_t"   , VAR_INT        , NULL           , (union dtypes *) &_Tmarg          ,
"_v"   , VAR_INT        , NULL           , (union dtypes *) &_Lpi            ,
"_w"   , VAR_INT        , NULL           , (union dtypes *) &_Pgwidth        ,
"_x"   , VAR_INT        , NULL           , (union dtypes *) &_Auto_crlf      ,
"_y"   , VAR_INT        , NULL           , (union dtypes *) &_Doublestrike   ,
"_E"   , VAR_INT        , NULL           , (union dtypes *) &_Italics        ,
"_J"   , VAR_INT        , NULL           , (union dtypes *) &_Restoreprinter ,
"_K"   , VAR_INT        , NULL           , (union dtypes *) &_Condensed      ,
"_L"   , VAR_INT        , NULL           , (union dtypes *) &_Wrap           ,
"_W"   , VAR_INT        , NULL           , (union dtypes *) &_Cont_dblwide   ,
"_X"   , VAR_STR        , NULL           , (union dtypes *) &_Codepagename   ,
"_Z"   , VAR_INT        , NULL           , (union dtypes *) &_Do_formfeed    ,
"mH"   , VAR_INT        , NULL           , (union dtypes *) &_Hres           ,
"mV"   , VAR_INT        , NULL           , (union dtypes *) &_Vres           ,
"mx"   , VAR_INT        , NULL           , (union dtypes *) &_Max_htabs      ,
"my"   , VAR_INT        , NULL           , (union dtypes *) &_Max_vtabs      ,
"mP"   , VAR_INT        , NULL           , (union dtypes *) &_Perf_percent   ,
"mN"   , VAR_INT        , NULL           , (union dtypes *) &_Pr_model_num   ,
"m7"   , VAR_INT        , NULL           , (union dtypes *) &_O_strike_char  ,
"m8"   , VAR_INT        , NULL           , (union dtypes *) &_O_strike_byp   ,
"m9"   , VAR_INT        , NULL           , (union dtypes *) &_U_score_byp    ,
"w1"   , VAR_INT        , NULL           , (union dtypes *) &_Workint1       ,
"wD"   , VAR_INT        , NULL           , (union dtypes *) &_Vdecr          ,
"wF"   , VAR_INT        , NULL           , (union dtypes *) &_Hpos_from_prev ,
"wG"   , VAR_INT        , NULL           , (union dtypes *) &_Gunits         ,
"wH"   , VAR_INT        , NULL           , (union dtypes *) &_Hincr          ,
"wT"   , VAR_INT        , NULL           , (union dtypes *) &_Tunits         ,
"wV"   , VAR_INT        , NULL           , (union dtypes *) &_Vincr          ,
"wb"   , VAR_INT        , NULL           , (union dtypes *) &_Subscr_mode    ,
"wd"   , VAR_INT        , NULL           , (union dtypes *) &_Line_dblwide   ,
"we"   , VAR_INT        , NULL           , (union dtypes *) &_Superscr_mode  ,
"wf"   , VAR_STR        , NULL           , (union dtypes *) &_Ff_cmd         ,
"wg"   , VAR_INT        , NULL           , (union dtypes *) &_Graphic_linesp ,
"wh"   , VAR_STR        , NULL           , (union dtypes *) &_Horz_tabs      ,
"wi"   , VAR_STR        , NULL           , (union dtypes *) &_Vincr_cmd      ,
"wl"   , VAR_INT        , NULL           , (union dtypes *) &_Linespacing    ,
"wm"   , VAR_INT        , NULL           , (union dtypes *) &_Vertspace_mode ,
"wo"   , VAR_INT        , NULL           , (union dtypes *) &_Cont_overscore ,
"wp"   , VAR_INT        , NULL           , (union dtypes *) &_Passthru       ,
"wr"   , VAR_INT        , NULL           , (union dtypes *) &_L_to_r_print   ,
"ws"   , VAR_INT        , NULL           , (union dtypes *) &_Prop_spacing   ,
"wt"   , VAR_INT        , NULL           , (union dtypes *) &_Text_linesp    ,
"wu"   , VAR_INT        , NULL           , (union dtypes *) &_Cont_undrscore ,
"wv"   , VAR_STR        , NULL           , (union dtypes *) &_Vert_tabs      ,
"ww"   , VAR_INT        , NULL           , (union dtypes *) &_Width          ,
"wy"   , VAR_STR        , NULL           , (union dtypes *) &_Vdecr_cmd      ,
"wO"   , VAR_INT        , NULL           , (union dtypes *) &_Ovrstrike_mode ,
"wU"   , VAR_INT        , NULL           , (union dtypes *) &_Undrscore_mode ,
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

#define OPTSTRING  "b:e:g:i:j:k:l:p:q:t:v:w:x:y:J:K:L:W:X:Z:"

