/* @(#)05	1.2  src/bos/usr/lib/piostw/fmtrsTW/piof5577d.h, cmdpiostw, bos411, 9428A410j 7/31/92 23:44:35 */

/*
 * COMPONEMT_NAME: (CMDPIOSTW) Printer Backend for Traditinal Chinese Printers
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTAIL -- (IBM  Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991
 * All Right Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*******************************************************************************
*                         Integer and String Variables                         *
*******************************************************************************/

/* Bottom Margin (number of blank lines); Somtimes Called Skip Perf. */
#define Bmarg (*(_Bmarg + piomode))  /* variable name */
int *_Bmarg;   /* pointer to the variable */

/* Emphasized Print */
#define Emphasized (*(_Emphasized + piomode))  /* variable name */
int *_Emphasized;   /* pointer to the variable */

/* Page Number Where Printing Should Begin */
#define Beginpg (*(_Beginpg + piomode))  /* variable name */
int *_Beginpg;   /* pointer to the variable */

/* Indent Value (character) */
#define Indent (*(_Indent + piomode))  /* variable name */
int *_Indent;   /* pointer to the variable */

/* Initalize the printer (for each print file)? (!:no; +:yes) */
#define Init_printer (*(_Init_printer + piomode))  /* variable name */
int *_Init_printer;   /* pointer to the variable */

/* Page Length (lines; 0 means ignore page length) */
#define Pglen (*(_Pglen + piomode))  /* variable name */
int *_Pglen;   /* pointer to the variable */

/* Pitch (characters per inch) */
#define Pitch (*(_Pitch + piomode))  /* variable name */
int *_Pitch;   /* pointer to the variable */

/* Type Style (m10, m12, courier, elite, gothic) */
#define Typestyle (*(_Typestyle + piomode))  /* variable name */
int *_Typestyle;   /* pointer to the variable */

/* Top Margin (number of blank lines) */
#define Tmarg (*(_Tmarg + piomode))  /* variable name */
int *_Tmarg;   /* pointer to the variable */

/* Line Density (lines per inch) */
#define Lpi (*(_Lpi + piomode))  /* variable name */
int *_Lpi;   /* pointer to the variable */

/* Page Width (characters) */
#define Pgwidth (*(_Pgwidth + piomode))  /* variable name */
int *_Pgwidth;   /* pointer to the variable */

/* 0: CR->CR, LF->LF, VT->VT;  1: CR->CRLF  2: LF->CRLF, VT->CRVT; */
#define Auto_crlf (*(_Auto_crlf + piomode))  /* variable name */
int *_Auto_crlf;   /* pointer to the variable */

/* Double-Strike Print (!: no; +: yes) */
#define Doublestrike (*(_Doublestrike + piomode))  /* variable name */
int *_Doublestrike;   /* pointer to the variable */

/* Double-High Print (!: no; +: yes) */
#define Doublehigh (*(_Doublehigh + piomode))  /* variable name */
int *_Doublehigh;   /* pointer to the variable */

/* Use-defined Font File Name */
#define User_font (*(_User_font + piomode))  /* variable name */
struct str_info *_User_font;   /* pointer to the variable */

/* Restore the Printer at the End of the Print Job ? */
#define Restoreprinter (*(_Restoreprinter + piomode))  /* variable name */
int *_Restoreprinter;   /* pointer to the variable */

/* Condensed Print (!: no; +: yes) */
#define Condensed (*(_Condensed + piomode))  /* variable name */
int *_Condensed;   /* pointer to the variable */

/* Wrap Long Lines (!: no; +: yes) */
#define Wrap (*(_Wrap + piomode))  /* variable name */
int *_Wrap;   /* pointer to the variable */

/* Type of Input Paper Handling (1: manual, 2: pinwheel, 3: sheetfeed) */
#define Paper_handle (*(_Paper_handle + piomode))  /* variable name */
int *_Paper_handle;   /* pointer to the variable */

/* High Speed Print (!: no; +: yes) */
#define Highspeed (*(_Highspeed + piomode))  /* variable name */
int *_Highspeed;   /* pointer to the variable */

/* Unidirectional Printing (!: no; +: yes) */
#define Unidirect (*(_Unidirect + piomode))  /* variable name */
int *_Unidirect;   /* pointer to the variable */

/* Vertical Printing (!: no; +: yes) */
#define Vertical (*(_Vertical + piomode))  /* variable name */
int *_Vertical;   /* pointer to the variable */

/* Double-Wide Print (!; no; +: yes) */
#define Doublewide (*(_Doublewide + piomode))  /* variable name */
int *_Doublewide;   /* pointer to the variable */

/* Issue Form Feeds Between Copies & At Job End (!: no; +: yes) */
#define Do_formfeed (*(_Do_formfeed + piomode))  /* variable name */
int *_Do_formfeed;   /* pointer to the variable */

/* Number of Units Into Which a Horizontal Inch Is Divided */
#define Hres (*(_Hres + piomode))  /* variable name */
int *_Hres;   /* pointer to the variable */

/* Number of Units Into Which a Vertical Inch Is Diviced */
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

/* Pass-Through Instead of Formatting (i.e., data streamis not modified) ? */
#define Passthru (*(_Passthru + piomode))  /* variable name */
int *_Passthru;   /* pointer to the variable */

/* Number of Lines to Space Down For Each Line Feed */
#define Linespacing (*(_Linespacing + piomode))  /* variable name */
int *_Linespacing;   /* pointer to the variable */

/* Work Variable For Horizontal Increment Per Character (in Hres units) */
#define Hincr (*(_Hincr + piomode))  /* variable name */
int *_Hincr;   /* pointer to the variable */

/* Image Transmission Mode (3: 3-byte transmission, 2: 2-byte transmission) */
#define img_h (*(_img_h + piomode))  /* variable name */
int *_img_h;   /* pointer to the variable */

/* Image Data Width (used by FS Command) */
#define img_w (*(_img_w + piomode))  /* variable name */
int *_img_w;   /* pointer to the variable */

/* The Remaining of Image Data */
#define Image_rem (*(_Image_rem + piomode))  /* variable name */
int *_Image_rem;   /* pointer to the variable */

/* The Remaining of Characaters for Print All Command */
#define All_char (*(_All_char + piomode))  /* variable name */
int *_All_char;   /* pointer to the variable */

/* Work Variable For Vertical Increment (in Vres units) */
#define Vincr (*(_Vincr + piomode))  /* variable name */
int *_Vincr;   /* pointer to the variable */

/* Work Variable For Vertical Decrement (in Vres units) */
#define Vdecr (*(_Vdecr + piomode))  /* variable name */
int *_Vdecr;   /* pointer to the variable */

/* Work Variable For Page Width (in Hres units) */
#define pgwidth (*(_pgwidth + piomode))  /* variable name */
int *_pgwidth;   /* pointer to the variable */

/* Left Margin in Hres Unit */
#define lmarg (*(_lmarg + piomode))  /* variable name */
int *_lmarg;   /* pointer to the variable */

/* Work Variable In Which To Save Horizontal Position From Previous Line */
#define Hpos_from_prev (*(_Hpos_from_prev + piomode))  /* variable name */
int *_Hpos_from_prev;   /* pointer to the variable */

/* Work Integer */
#define Workint (*(_Workint + piomode))  /* variable name */
int *_Workint;   /* pointer to the variable */

/* Currently In Subscript Mode? (!:no; +:yes) */
#define Subscr_mode (*(_Subscr_mode + piomode))  /* variable name */
int *_Subscr_mode;   /* pointer to the variable */

/* Currently In Superscript Mode? (!:no; +:yes) */
#define Superscr_mode (*(_Superscr_mode + piomode))  /* variable name */
int *_Superscr_mode;   /* pointer to the variable */

/* Current Horizontal Tab Settings (terminated by null) */
#define Horz_tabs (*(_Horz_tabs + piomode))  /* variable name */
struct str_info *_Horz_tabs;   /* pointer to the variable */

/* Currently In Continuous Underscore Mode ? (B'0000 00nm; n: 1:on 0:off; m: 1:blank */
#define Cont_undrscore (*(_Cont_undrscore + piomode))  /* variable name */
int *_Cont_undrscore;   /* pointer to the variable */

/* Current Vertical Tab Settings (terminated by null) */
#define Vert_tabs (*(_Vert_tabs + piomode))  /* variable name */
struct str_info *_Vert_tabs;   /* pointer to the variable */

/* Name of Current Vertical Increment Command */
#define Vincr_cmd (*(_Vincr_cmd + piomode))  /* variable name */
struct str_info *_Vincr_cmd;   /* pointer to the variable */

/* Name of Forward Vertical Step Feed */
#define Vincr_cmd2 (*(_Vincr_cmd2 + piomode))  /* variable name */
struct str_info *_Vincr_cmd2;   /* pointer to the variable */

/* Name of Particial Index Down Command */
#define Vincr_cmd3 (*(_Vincr_cmd3 + piomode))  /* variable name */
struct str_info *_Vincr_cmd3;   /* pointer to the variable */

/* Name of Current Vert. Decrement Command */
#define Vdecr_cmd (*(_Vdecr_cmd + piomode))  /* variable name */
struct str_info *_Vdecr_cmd;   /* pointer to the variable */

/* Name of Backward Vertical Step Feed */
#define Vdecr_cmd2 (*(_Vdecr_cmd2 + piomode))  /* variable name */
struct str_info *_Vdecr_cmd2;   /* pointer to the variable */

/* Name of Particial Index Up Command */
#define Vdecr_cmd3 (*(_Vdecr_cmd3 + piomode))  /* variable name */
struct str_info *_Vdecr_cmd3;   /* pointer to the variable */

/* Name of Current Form Feed Command */
#define Ff_cmd (*(_Ff_cmd + piomode))  /* variable name */
struct str_info *_Ff_cmd;   /* pointer to the variable */

/* Character Code for Over Strike Command (1byte code: 0x0000XX00; 2byte code: 0x0000XXXX) */
#define Ovrstrike_code (*(_Ovrstrike_code + piomode))  /* variable name */
int *_Ovrstrike_code;   /* pointer to the variable */

/* Character Flag for Over Strike Command (3: Hankaku; 4: Zenkaku; 1: Cancel) */
#define Ovrstrike_flag (*(_Ovrstrike_flag + piomode))  /* variable name */
int *_Ovrstrike_flag;   /* pointer to the variable */

/* Mode Flag for Over Strike Command (B'0000 00nm) */
#define Ovrstrike_mode (*(_Ovrstrike_mode + piomode))  /* variable name */
int *_Ovrstrike_mode;   /* pointer to the variable */

#define	Codepagename	(*(_Codepagename + piomode ))
struct str_info	*_Codepagename;

/*******************************************************************************
*                               String Constants                               *
*******************************************************************************/

/* Commands To Initialize the Printer */
#define INIT_CMD       "ci"

/* Restore the Printer at Job End */
#define REST_CMD       "cr"

/* Command When Switching Between Pri. Mode & Alt. (security label) Mode */
#define SWITCHMODE_CMD "cm"

/* Command To Send Carrier Return & Line Feed Controls */
#define CRLF_CMD       "cl"

/* Command To Space Horizontally In Hres Units (used for ident, LF w/o CR) */
#define HORZ_SPACE_CMD "ch"

/* Initialize Character Scale */
#define INIT_SCALE_CMD "cs"

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

/* ASCII Control Code to Enable the Printer (Device Control 1) */
#define DC1_CMD        "ae"

/* ASCII Control Code to Disable the Printer (Device Control 3) */
#define DC3_CMD        "ad"

/* ASCII Control Code to Cancel the Print Job (Cancel) */
#define CAN_CMD        "at"

/* Set DBCS Data Stream Mode (used only for init/restore) */
#define SET_DBCS_CMD   "eA"

/* Initialize Printers (used only for init/restore) */
#define INIT_PRT_CMD   "eC"

/* Set/Reset Uni-directional Print Mode */
#define SET_DIR_CMD    "eD"

/* Set/Reset High Speed Print Mode */
#define SET_SPD_CMD    "eE"

/* Set/Reset Condenced Print Mode */
#define SET_CND_CMD    "eF"

/* Set/Reset Double Wide Print Mode */
#define SET_ENLG_CMD   "eG"

/* Set/Reset Viertical Print Mode */
#define SET_VERT_CMD   "eH"

/* Set/Reset Emphasized Print Mode */
#define SET_ENPH_CMD   "eI"

/* Set/Reset Double Strike Print Mode */
#define SET_DBLSTR_CMD "eJ"

/* Set/Reset Under Score */
#define UNDERSCORE_CMD "eK"

/* Set/Reset Over Strike */
#define OVERSTRIKE_CMD "eL"

/* Set Character Per Inch */
#define CHG_CPI_CMD    "eO"

/* Set Lines Per Inch */
#define CHG_LPI_CMD    "eP"

/* Set Page Length in 1/6 unit */
#define CHG_PGLEN_CMD  "eQ"

/* Change Font Style */
#define CHG_FONT_CMD   "eR"

/* Change Character Scaling */
#define CHG_SCALE_CMD  "eS"

/* Cut Form Insert */
#define CUT_FORM_INS   "eT"

/* Cut Form Eject */
#define CUT_FORM_EJT   "eU"

/* Set/Reset Super/Sub Script Mode */
#define SET_SCRIPT_CMD "eV"

/* Image Transmission */
#define IMG_TRSMIT_CMD "ea"

/* Image Transmission and Enlarge */
#define IMG_TRSENL_CMD "eb"

/* Forward Horizontal Skip */
#define FRD_HORSKP_CMD "ec"

/* Backward Horizontal Skip */
#define BWD_HORSKP_CMD "ed"

/* Forward Vertical Step Feed */
#define FWD_VRTSTP_CMD "ee"

/* CR To Specified Dot Position */
#define CR_TO_DOT_CMD  "ef"

/* Backward Vertical Step Feed */
#define BWD_VRTSTP_CMD "eg"

/* Set Line Feed Increment Value */
#define SET_LININC_CMD "eh"

/* Change Form Handling */
#define CHG_FORM_CMD   "ei"

/* Print All Characters */
#define PRTALL_CMD     "ej"

/* Partial Index Up */
#define INDEX_UP_CMD   "ek"

/* Partial Index Down */
#define INDEX_DOWN_CMD "el"

/* Set 3/2 Bytes Image Transmission Mode */
#define IMG_HEIGHT_CMD "em"

/* Set Grid Line Attribute */
#define GRID_LINE_CMD  "en"

/* Gaiji Load Command */
#define GAIJI_LOAD_CMD "eo"

/* Horizontal Column Skip */
#define COLUMN_SKIP    "eq"

/* Vertical Line Skip */
#define LINE_SKIP_CMD  "er"



/*******************************************************************************
*                    Table of Variables and Their Attributes                   *
*******************************************************************************/

struct attrparms attrtable[] = {
							      /*
 name      data type       lookup table     addr of pointer
------   --------------   --------------   ----------------   */
"_b"   , VAR_INT        , NULL           , (union dtypes *) &_Bmarg          ,
"_e"   , VAR_INT        , lkup_bool      , (union dtypes *) &_Emphasized     ,
"_g"   , VAR_INT        , NULL           , (union dtypes *) &_Beginpg        ,
"_i"   , VAR_INT        , NULL           , (union dtypes *) &_Indent         ,
"_j"   , VAR_INT        , lkup_bool      , (union dtypes *) &_Init_printer   ,
"_l"   , VAR_INT        , NULL           , (union dtypes *) &_Pglen          ,
"_p"   , VAR_INT        , lkup_cpi       , (union dtypes *) &_Pitch          ,
"_s"   , VAR_INT        , lkup_fnt       , (union dtypes *) &_Typestyle      ,
"_t"   , VAR_INT        , NULL           , (union dtypes *) &_Tmarg          ,
"_v"   , VAR_INT        , lkup_lpi       , (union dtypes *) &_Lpi            ,
"_w"   , VAR_INT        , NULL           , (union dtypes *) &_Pgwidth        ,
"_x"   , VAR_INT        , lkup_crlf      , (union dtypes *) &_Auto_crlf      ,
"_y"   , VAR_INT        , lkup_bool      , (union dtypes *) &_Doublestrike   ,
"_E"   , VAR_INT        , lkup_bool      , (union dtypes *) &_Doublehigh     ,
"_F"   , VAR_STR        , NULL           , (union dtypes *) &_User_font      ,
"_J"   , VAR_INT        , lkup_bool      , (union dtypes *) &_Restoreprinter ,
"_K"   , VAR_INT        , lkup_bool      , (union dtypes *) &_Condensed      ,
"_L"   , VAR_INT        , lkup_bool      , (union dtypes *) &_Wrap           ,
"_O"   , VAR_INT        , lkup_phd       , (union dtypes *) &_Paper_handle   ,
"_S"   , VAR_INT        , lkup_bool      , (union dtypes *) &_Highspeed      ,
"_U"   , VAR_INT        , lkup_bool      , (union dtypes *) &_Unidirect      ,
"_V"   , VAR_INT        , lkup_bool      , (union dtypes *) &_Vertical       ,
"_W"   , VAR_INT        , lkup_bool      , (union dtypes *) &_Doublewide     ,
"_X"   , VAR_STR        , NULL           , (union dtypes *) &_Codepagename   ,
"_Z"   , VAR_INT        , lkup_bool      , (union dtypes *) &_Do_formfeed    ,
"mH"   , VAR_INT        , NULL           , (union dtypes *) &_Hres           ,
"mV"   , VAR_INT        , NULL           , (union dtypes *) &_Vres           ,
"mx"   , VAR_INT        , NULL           , (union dtypes *) &_Max_htabs      ,
"my"   , VAR_INT        , NULL           , (union dtypes *) &_Max_vtabs      ,
"mP"   , VAR_INT        , NULL           , (union dtypes *) &_Perf_percent   ,
"wp"   , VAR_INT        , NULL           , (union dtypes *) &_Passthru       ,
"wl"   , VAR_INT        , NULL           , (union dtypes *) &_Linespacing    ,
"wH"   , VAR_INT        , NULL           , (union dtypes *) &_Hincr          ,
"wI"   , VAR_INT        , NULL           , (union dtypes *) &_img_h          ,
"wJ"   , VAR_INT        , NULL           , (union dtypes *) &_img_w          ,
"wB"   , VAR_INT        , NULL           , (union dtypes *) &_Image_rem      ,
"wA"   , VAR_INT        , NULL           , (union dtypes *) &_All_char       ,
"wV"   , VAR_INT        , NULL           , (union dtypes *) &_Vincr          ,
"wD"   , VAR_INT        , NULL           , (union dtypes *) &_Vdecr          ,
"ww"   , VAR_INT        , NULL           , (union dtypes *) &_pgwidth        ,
"wM"   , VAR_INT        , NULL           , (union dtypes *) &_lmarg          ,
"wF"   , VAR_INT        , NULL           , (union dtypes *) &_Hpos_from_prev ,
"wX"   , VAR_INT        , NULL           , (union dtypes *) &_Workint        ,
"wb"   , VAR_INT        , NULL           , (union dtypes *) &_Subscr_mode    ,
"we"   , VAR_INT        , NULL           , (union dtypes *) &_Superscr_mode  ,
"wh"   , VAR_STR        , NULL           , (union dtypes *) &_Horz_tabs      ,
"wu"   , VAR_INT        , NULL           , (union dtypes *) &_Cont_undrscore ,
"wv"   , VAR_STR        , NULL           , (union dtypes *) &_Vert_tabs      ,
"wi"   , VAR_STR        , NULL           , (union dtypes *) &_Vincr_cmd      ,
"wd"   , VAR_STR        , NULL           , (union dtypes *) &_Vincr_cmd2     ,
"wx"   , VAR_STR        , NULL           , (union dtypes *) &_Vincr_cmd3     ,
"wy"   , VAR_STR        , NULL           , (union dtypes *) &_Vdecr_cmd      ,
"wt"   , VAR_STR        , NULL           , (union dtypes *) &_Vdecr_cmd2     ,
"wz"   , VAR_STR        , NULL           , (union dtypes *) &_Vdecr_cmd3     ,
"wf"   , VAR_STR        , NULL           , (union dtypes *) &_Ff_cmd         ,
"wc"   , VAR_INT        , NULL           , (union dtypes *) &_Ovrstrike_code ,
"wg"   , VAR_INT        , NULL           , (union dtypes *) &_Ovrstrike_flag ,
"wm"   , VAR_INT        , NULL           , (union dtypes *) &_Ovrstrike_mode ,
NULL   , 0              , NULL           , NULL

};


/*******************************************************************************
*                   String of Options (Flags) for piogetopt()                  *
*******************************************************************************/

#ifdef OPTSTRING
#undef OPTSTRING
#endif
#define OPTSTRING  "b:e:g:i:j:l:p:s:t:v:w:x:y:E:F:J:K:L:O:S:U:V:W:Z:"
