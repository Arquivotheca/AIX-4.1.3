/* @(#)93       1.5.1.3 9/14/93 01:02:41 */

/*
 *   COMPONENT_NAME: (CMDPIOSK) Printer Backend for Japanese Printers
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*******************************************************************************
*                         Integer and String Variables                         *
*******************************************************************************/
#define	Beginpg	(*(_Beginpg + piomode ) )
int	*_Beginpg;

#define	Init_printer	(*(_Init_printer + piomode))
int	*_Init_printer;

#define	Restoreprinter	(*(_Restoreprinter + piomode))
int	*_Restoreprinter;

#define	Do_formfeed	(*(_Do_formfeed + piomode))
int	*_Do_formfeed;

#define	Hres	(*(_Hres + piomode ))
int	*_Hres;

#define	Vres	(*(_Vres + piomode ))
int	*_Vres;

#define	Max_htabs	(*(_Max_htabs + piomode ))
int	*_Max_htabs;

#define	Max_vtabs	(*(_Max_vtabs + piomode ))
int	*_Max_vtabs;

#define	Perf_percent	(*(_Perf_percent + piomode ))
int	*_Perf_percent;

#define	Command_level	( *(_Command_level + piomode ) )
int	*_Command_level;

#define	Dpi	(*(_Dpi + piomode ) )
int	*_Dpi;

#define	Codepagename   (*(_Codepagename + piomode))
struct str_info *_Codepagename;   /* pointer to the variable */

#define	KO_code	(*(_KO_code + piomode ) )
struct str_info	*_KO_code;

#define	KI_code	(*(_KI_code + piomode ) )
struct str_info	*_KI_code;

#define	Pagelength	(*(_Pagelength + piomode))
int	*_Pagelength;

#define	Pagewidth	(*(_Pagewidth + piomode ))
int	*_Pagewidth;

#define	Pageorient	(*(_Pageorient + piomode ))
int	*_Pageorient;

#define	Lmargin	(*(_Lmargin + piomode ) )
int	*_Lmargin;

#define	Rmargin	(*(_Rmargin + piomode ) )
int	*_Rmargin;

#define	Tmargin	(*(_Tmargin + piomode ) )
int	*_Tmargin;
int	Tmargin_dummy = 0;

#define	Bmargin	(*(_Bmargin + piomode ) )
int	*_Bmargin;
int	Bmargin_dummy;

#define	Pageend	(*(_Pageend + piomode ))
int	*_Pageend;

#define	VMI	(*(_VMI + piomode ) )
int	*_VMI;

#define	HMI	(*(_HMI + piomode ))
int	*_HMI;

#define	FixVMI	(*(_FixVMI + piomode ) )
int	*_FixVMI;

#define	FixHMI	(*(_FixHMI + piomode ) )
int	*_FixHMI;

#define	Htabval	(*(_Htabval + piomode ) )
int	*_Htabval;

#define	Vtabval	(*(_Vtabval + piomode ) )
int	*_Vtabval;

#define	Charwidth	(*(_Charwidth + piomode ) )
int	*_Charwidth;

#define	Charheight	(*(_Charheight + piomode ))
int	*_Charheight;

#define	Stroke	(*(_Stroke + piomode ) )
int	*_Stroke;

#define	Fstyle	(*(_Fstyle + piomode ) )
int	*_Fstyle;

#define	Vertical	(*(_Vertical + piomode ) )
int	*_Vertical;

#define	Paper	(*(_Paper + piomode ) )
int	*_Paper;

#define	Codepageval	(*(_Codepageval + piomode ) )
int	*_Codepageval;

#define	GRsetval	(*(_GRsetval + piomode ) )
int	*_GRsetval;

#define	Dot_offset	(*(_Dot_offset + piomode ) )
int	*_Dot_offset;

#define	Wrap	(*(_Wrap + piomode ))
int	*_Wrap;

#define	FF_explicit	(*(_FF_explicit + piomode ) )
int	*_FF_explicit;

#define	CAP_nomove	(*(_CAP_nomove + piomode ) )
int	*_CAP_nomove;

#define	CRaddLF	(*(_CRaddLF + piomode ) )
int	*_CRaddLF;

#define	LFaddCR	(*(_LFaddCR + piomode ) )
int	*_LFaddCR;

#define	FFaddCR	(*(_FFaddCR + piomode ) )
int	*_FFaddCR;

#define	VTaddCR	(*(_VTaddCR + piomode ) )
int	*_VTaddCR;

#define	No_PSpacing	(*(_No_PSpacing + piomode ) )
int	*_No_PSpacing;

#define	Goto_tmargin	(*(_Goto_tamrgin + piomode ) )
int	*_Goto_tmargin;

#define	HMI_explicit	(*(_HMI_explicit + piomode ) )
int	*_HMI_explicit;

#define	Mesh_overwrite	(*(_Mesh_overwrite + piomode ) )
int	*_Mesh_overwrite;

#define	Auto_emphasize	(*(_Auto_emphasize + piomode ) )
int	*_Auto_emphasize;

#define	Restore_charattr	(*(_Restore_charattr + piomode ) )
int	*_Restore_charattr;

#define	String_expand	(*(_String_expand + piomode ) )
int	*_String_expand;

#define	Vertical_char_tr	(*(_Vertical_char_tr + piomode ) )
int	*_Vertical_char_tr;

#define	Sizeunitformove	(*(_Sizeunitformove + piomode ))
int	*_Sizeunitformove;

#define	Maxpagewidth	(*(_Maxpagewidth + piomode ))
int	*_Maxpagewidth;

#define	Maxpagelength	(*(_Maxpagelength + piomode ) )
int	*_Maxpagelength;

#define	Minpagewidth	(*(_Minpagewidth + piomode ) )
int	*_Minpagewidth;

#define	Minpagelength	(*(_Minpagelength + piomode ) )
int	*_Minpagelength;

#define	MaxVMI	(*(_MaxVMI + piomode ) )
int	*_MaxVMI;

#define	MaxHMI	(*(_MaxHMI + piomode ) )
int	*_MaxHMI;

#define	MaxX	(*(_MaxX + piomode ))
int	*_MaxX;

#define	MaxY	(*(_MaxY + piomode ))
int	*_MaxY;

#define	SPosN	(*(_SPosN + piomode ))
int	*_SPosN;

#define	Pgspace	(*(_Pgspace + piomode ))
int	*_Pgspace;

#define	Iconvnames	(*(_Iconvnames + piomode ) )
struct str_info	*_Iconvnames;

#define	Tmpcode	(*(_Tmpcode + piomode ) )
struct str_info	*_Tmpcode;

#define	Iconvouts	(*(_Iconvouts + piomode ) )
struct str_info	*_Iconvouts;

#define	Localenames	(*(_Localenames + piomode ) )
struct str_info	*_Localenames;

#define	Iconvindex	(*(_Iconvindex + piomode ) )
struct str_info	*_Iconvindex;

#define	Statevals	(*(_Statevals + piomode ) )
struct str_info	*_Statevals;

#define	GRcodeset	(*(_GRcodeset + piomode ) )
struct str_info	*_GRcodeset;

#define	UDCcodeset	(*(_UDCcodeset + piomode ) )
struct str_info	*_UDCcodeset;

#define	Max_csize	(*(_Max_csize + piomode ) )
int	*_Max_csize;

#define	Int0	(*(_Int0 + piomode ) )
int	*_Int0;

#define	Int1	(*(_Int1 + piomode ) )
int	*_Int1;

#define	Int2	(*(_Int2 + piomode ) )
int	*_Int2;

#define	Int3	(*(_Int3 + piomode ) )
int	*_Int3;

#define	Int4	(*(_Int4 + piomode ) )
int	*_Int4;

#define	Int5	(*(_Int5 + piomode ) )
int	*_Int5;

#define	Int6	(*(_Int6 + piomode ) )
int	*_Int6;

#define	Int7	(*(_Int7 + piomode ) )
int	*_Int7;

#define	Int8	(*(_Int8 + piomode ) )
int	*_Int8;

#define	Int9	(*(_Int9 + piomode ) )
int	*_Int9;

#define	Str0   (*(_Str0 + piomode))
struct str_info *_Str0;   /* pointer to the variable */

#define	Str1   (*(_Str1 + piomode))
struct str_info *_Str1;   /* pointer to the variable */

#define	Str2   (*(_Str2 + piomode))
struct str_info *_Str2;   /* pointer to the variable */

#define	Str3   (*(_Str3 + piomode))
struct str_info *_Str3;   /* pointer to the variable */

#define	Str4   (*(_Str4 + piomode))
struct str_info *_Str4;   /* pointer to the variable */

#define	Str5   (*(_Str5 + piomode))
struct str_info *_Str5;   /* pointer to the variable */

#define	Str6   (*(_Str6 + piomode))
struct str_info *_Str6;   /* pointer to the variable */

#define	Str7   (*(_Str7 + piomode))
struct str_info *_Str7;   /* pointer to the variable */

#define	Str8   (*(_Str8 + piomode))
struct str_info *_Str8;   /* pointer to the variable */

#define	Str9   (*(_Str9 + piomode))
struct str_info *_Str9;   /* pointer to the variable */

#define	Pageform	(*(_Pageform + piomode ))
int	*_Pageform;

#define	Fontlist	(*(_Fontlist + piomode ) )
struct str_info	*_Fontlist;

#define Font_path (*(_Font_path + piomode))  
struct str_info *_Font_path;   

int	*_Doublehigh;
int	*_Doublewide;
#define	Emphasize	(*(_Emphasize + piomode ))
int	*_Emphasize;

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

#define	NUL_CNTL_CMD	"an"
#define	ETX_CNTL_CMD	"ax"
#define	ACK_CNTL_CMD	"aa"
#define	BEL_CNTL_CMD	"ar"
#define	BS_CNTL_CMD	"ab"
#define	HT_CNTL_CMD	"ah"
#define	LF_CNTL_CMD	"al"
#define	VT_CNTL_CMD	"av"
#define	FF_CNTL_CMD	"af"
#define	FF_CMD	"af"
#define	CR_CNTL_CMD	"ac"
#define	SO_CNTL_CMD	"ao"
#define	SI_CNTL_CMD	"ai"
#define	XON_CNTL_CMD	"ae"
#define	XOF_CNTL_CMD	"ad"
#define	EM_CNTL_CMD	"am"
#define	GS_CNTL_CMD	"ag"
#define	SP_CNTL_CMD	"as"
#define	DEL_CNTL_CMD	"az"
#define	NEL_CNTL_CMD	"aN"
#define	HTS_CNTL_CMD	"aH"
#define	VTS_CNTL_CMD	"aV"
#define	PLD_CNTL_CMD	"aL"
#define	PLU_CNTL_CMD	"aU"
#define	RI_CNTL_CMD	"aR"
#define	SS2_CNTL_CMD	"a2"
#define	SS3_CNTL_CMD	"a3"
#   define	SS2_VALUE	0x8e
#   define	SS3_VALUE	0x8f

#define	TEXT_START_CMD	"eA"
#define	VECTOR_START_CMD	"eB"
#define	EMU_START_CMD	"eC"
#define	SJIS_START_CMD	"eD"
#define	JOB_START_CMD	"eE"
#define	JOB_END_CMD	"eF"
#define	HARD_RESET_CMD	"eG"
#define	SOFT_RESET_CMD	"eH"
#define	PARA_RESET_CMD	"eI"
#define	MEM_FREE_CMD	"eJ"
#define	SIZE_SELECT_CMD	"eK"
#define	PAGE_SELECT_CMD	"eL"
#define	PAGE_ROTATE_CMD	"eM"
#define	MARGIN_SET_CMD	"eN"
#define	MARGIN_CLR_CMD	"eO"
#define	LPI_SELECT_CMD	"eP"
#define	SPACING_CMD	"eQ"
#define	P_OFFSET_CMD	"eR"
#define	OFFSET_CMD	"eS"
#define	ABS_XYMOVE_CMD	"eT"
#define	ABS_YMOVE_CMD	"eU"
#define	ABS_XMOVE_CMD	"eV"
#define	REL_YMOVE_CMD	"eW"
#define	REL_XMOVE_CMD	"eX"
#define	SR_POS_CMD	"eY"
#define	TAB_CLR_CMD	"eZ"

#define	WRAP_MODE_CMD	"ea"
#define	AUTO_FF_CMD	"eb"
#define	CAP_MODE_CMD	"ec"
#define	CR_MODE_CMD	"ed"
#define	LF_MODE_CMD	"ee"
#define	FF_MODE_CMD	"ef"
#define	VT_MODE_CMD	"eg"
#define	NO_PS_CMD	"eh"
#define	TM_MODE_CMD	"ei"
#define	HMI_MODE_CMD	"ej"
#define	MESH_MODE_CMD	"ek"
#define	EMPH_MODE_CMD	"el"
#define	RCHAR_MODE_CMD	"em"
#define	EXPAND_MODE_CMD	"en"
#define	VERTICAL_MODE_CMD	"eo"
#define	SIZE_MODE_CMD	"ep"

#define	LS2_CNTL_CMD	"EA"
#define	LS3_CNTL_CMD	"EB"
#define	LS1R_CNTL_CMD	"EC"
#define	LS2R_CNTL_CMD	"ED"
#define	LS3R_CNTL_CMD	"EE"
#define	PAIR_MODE_CMD	"EF"
#define	G0_GR_CMD	"EG"
#define	G1_GR_CMD	"EH"
#define	G2_GR_CMD	"EI"
#define	G3_GR_CMD	"EJ"
#define	G0_GR2_CMD	"EK"
#define	G1_GR2_CMD	"EL"
#define	G2_GR2_CMD	"EM"
#define	G3_GR2_CMD	"EN"
#define	CPI_SEL_CMD	"EO"
#define	CHAR_SIZ_CMD	"EP"
#define	CHAR_TYPE_CMD	"EQ"
#define	CS_NAME_SEL_CMD	"ER"
#define	CS_AGN_NUM_CMD	"ES"
#define	CS_AGN_1_CMD	"ET"
#define	CS_RECORD_B_CMD	"EU"
#define	CS_RECORD_H_CMD	"EV"
#define	C1_RECORD_B_CMD	"EW"
#define	C1_RECORD_H_CMD	"EX"
#define	CS_COPY_CMD	"EY"
#define	C1_COPY_CMD	"EZ"
#define	EMPH_CMD	"Ea"	/*****/
#define	CS_REC_HLP_CMD	"Eb"
#define	CS_DELETE_CMD	"Ec"
#define	CTRL_PR_B_CMD	"Ed"
#define	CTRL_PR_H_CMD	"Ee"
#define	TATE_YOKO_CMD	"Ef"
#define	COMPOSED_CMD	"Eg"
#define	UL_ATTR_CMD	"Eh"
#define	DEC_CHAR_CMD	"Ei"
#define	MESH_PAT_CMD	"Ej"
#define	EX_CHAR_CMD	"Ek"
#define	CHAR_ROT_CMD	"El"
#define	FR_MESH_ST_CMD	"Em"
#define	FR_MESH_EN_CMD	"En"
#define	M_PAT_REC_B_CMD	"Eo"
#define	M_PAT_REC_H_CMD	"Ep"
#define	GRID_ST_CMD	"Eq"
#define	GRID_EN_CMD	"Er"
#define	DGRID_ST_CMD	"Es"
#define	DGRID_EN_CMD	"Et"
#define	DGRID_REC_B_CMD	"Eu"
#define	DGRID_REC_H_CMD	"Ev"
#define	RAS_DRAW_B_CMD	"Ew"
#define	RAS_DRAW_H_CMD	"Ex"
#define	ALLOC_AREA_CMD	"Ey"
#define	WD_IMG_D1_B_CMD	"Ez"
#define	WD_IMG_D1_H_CMD	"E0"
#define	WD_IMG_D2_B_CMD	"E1"
#define	WD_IMG_D2_H_CMD	"E2"
#define	OL_REC_CMD	"E3"
#define	OL_PR_CMD	"E4"
#define	MAC_REC_B_CMD	"E5"
#define	MAC_REC_H_CMD	"E6"
#define	MAC_EXE_CMD	"E7"
#define	STUP_MAC_CMD	"E8"
#define	BKUP_ENV_CMD	"E9"
#define	REST_ENV_CMD	"e0"
#define	UNDO_CMD	"e1"
#define	PAPER_SUP_CMD	"e2"
#define	COPY_NUM_CMD	"e3"
#define	COM_DISP_CMD	"e4"
#define	COM_DISP3_CMD	"e5"
#define	STAT_REQ_CMD	"e6"
#define	CS_SEARCH_CMD	"e7"
#define	CW_REQ_CMD	"e8"

#define	Trans_in	(*(_Trans_in + piomode ))
struct str_info	*_Trans_in;

#define	Trans_out	(*(_Trans_out + piomode ))
struct str_info	*_Trans_out;

/*******************************************************************************
*                    Table of Variables and Their Attributes                   *
*******************************************************************************/

#define	dt	typedef union dtypes *

struct attrparms attrtable[] = {
							      /*
 name      data type       lookup table     addr of pointer
------   --------------   --------------   ----------------   */
"_g"   , VAR_INT        , NULL           , (union dtypes *) &_Beginpg        ,
"_j"   , VAR_INT        , lkup_rest      , (union dtypes *) &_Init_printer   ,
"_s",	VAR_INT,	lkup_fnt,	(dt)&_Fstyle,
"_u",	VAR_INT,	lkup_src,	(dt)&_Paper,
"_z",	VAR_INT,	lkup_page,	(dt)&_Pageform,
"_F",	VAR_STR,	NULL,	(dt)&_Fontlist,
"_I",   VAR_STR,        NULL             , (union dtypes *) &_Font_path      ,

"_J"   , VAR_INT        , lkup_bool      , (union dtypes *) &_Restoreprinter ,
"_V",	VAR_INT,	NULL,	(dt)&_Vertical,
"_X",	VAR_STR,	NULL,	(dt)&_Codepagename,
"_Z",	VAR_INT,	NULL,	(dt)&_Do_formfeed,
"_E",	VAR_INT,	lkup_bool,	(dt)&_Doublehigh,
"_W",	VAR_INT,	lkup_bool,	(dt)&_Doublewide,
"_e",	VAR_INT,	lkup_emph,	(dt)&_Emphasize,
"mH"   , VAR_INT        , NULL           , (union dtypes *) &_Hres           ,
"mV"   , VAR_INT        , NULL           , (union dtypes *) &_Vres           ,
"mx"   , VAR_INT        , NULL           , (union dtypes *) &_Max_htabs      ,
"my"   , VAR_INT        , NULL           , (union dtypes *) &_Max_vtabs      ,
"mP"   , VAR_INT        , NULL           , (union dtypes *) &_Perf_percent   ,
"wA",	VAR_INT,	NULL,	(dt)&_Command_level,
"wB",	VAR_INT,	NULL,	(dt)&_Dpi,
"wC",	VAR_INT,	NULL,	(dt)&_Pagewidth,
"wD",	VAR_INT,	NULL,	(dt)&_Pagelength,
"wE",	VAR_INT,	NULL,	(dt)&_Pageorient,
"wF",	VAR_INT,	NULL,	(dt)&_Lmargin,
"wG",	VAR_INT,	NULL,	(dt)&_Rmargin,
"wH",	VAR_INT,	NULL,	(dt)&_Tmargin,
"wI",	VAR_INT,	NULL,	(dt)&_Bmargin,
"wJ",	VAR_INT,	NULL,	(dt)&_Pageend,
"wK",	VAR_INT,	NULL,	(dt)&_VMI,
"wL",	VAR_INT,	NULL,	(dt)&_HMI,
"wM",	VAR_INT,	NULL,	(dt)&_Charwidth,
"wN",	VAR_INT,	NULL,	(dt)&_Charheight,
"wO",	VAR_INT,	NULL,	(dt)&_Stroke,
"wP",	VAR_INT,	NULL,	(dt)&_Htabval,
"wQ",	VAR_INT,	NULL,	(dt)&_Vtabval,
"wR",	VAR_INT,	NULL,	(dt)&_Codepageval,
"wT",	VAR_INT,	NULL,	(dt)&_GRsetval,
"wa",	VAR_INT,	NULL,	(dt)&_Wrap,
"wb",	VAR_INT,	NULL,	(dt)&_FF_explicit,
"wc",	VAR_INT,	NULL,	(dt)&_CAP_nomove,
"wd",	VAR_INT,	NULL,	(dt)&_CRaddLF,
"we",	VAR_INT,	NULL,	(dt)&_LFaddCR,
"wf",	VAR_INT,	NULL,	(dt)&_FFaddCR,
"wg",	VAR_INT,	NULL,	(dt)&_VTaddCR,
"wh",	VAR_INT,	NULL,	(dt)&_No_PSpacing,
"wi",	VAR_INT,	NULL,	(dt)&_Goto_tmargin,
"wj",	VAR_INT,	NULL,	(dt)&_HMI_explicit,
"wk",	VAR_INT,	NULL,	(dt)&_Mesh_overwrite,
"wl",	VAR_INT,	NULL,	(dt)&_Auto_emphasize,
"wm",	VAR_INT,	NULL,	(dt)&_Restore_charattr,
"wn",	VAR_INT,	NULL,	(dt)&_String_expand,
"wo",	VAR_INT,	NULL,	(dt)&_Vertical_char_tr,
"wp",	VAR_INT,	NULL,	(dt)&_Sizeunitformove,
"WA",	VAR_STR,	NULL,	(dt)&_KO_code,
"WB",	VAR_STR,	NULL,	(dt)&_KI_code,
"WC",	VAR_INT,	NULL,	(dt)&_FixVMI,
"WD",	VAR_INT,	NULL,	(dt)&_FixHMI,
"WE",	VAR_INT,	NULL,	(dt)&_Dot_offset,
"WF",	VAR_INT,	NULL,	(dt)&_Maxpagewidth,
"WG",	VAR_INT,	NULL,	(dt)&_Maxpagelength,
"WH",	VAR_INT,	NULL,	(dt)&_Minpagewidth,
"WI",	VAR_INT,	NULL,	(dt)&_Minpagelength,
"WJ",	VAR_INT,	NULL,	(dt)&_MaxVMI,
"WK",	VAR_INT,	NULL,	(dt)&_MaxHMI,
"WL",	VAR_INT,	NULL,	(dt)&_MaxX,
"WM",	VAR_INT,	NULL,	(dt)&_MaxY,
"WN",	VAR_INT,	NULL,	(dt)&_SPosN,
"WO",	VAR_INT,	NULL,	(dt)&_Pgspace,
"WP",	VAR_STR,	NULL,	(dt)&_Iconvnames,
"WQ",	VAR_STR,	NULL,	(dt)&_Localenames,
"WR",	VAR_STR,	NULL,	(dt)&_Tmpcode,
"WS",	VAR_STR,	NULL,	(dt)&_Statevals,
"WT",	VAR_STR,	NULL,	(dt)&_Iconvindex,
"WU",	VAR_STR,	NULL,	(dt)&_Iconvouts,
"WV",	VAR_STR,	NULL,	(dt)&_GRcodeset,
"WW",	VAR_STR,	NULL,	(dt)&_UDCcodeset,
"WX",	VAR_INT,	NULL,	(dt)&_Max_csize,
"w0",	VAR_INT,	NULL,	(dt)&_Int0,
"w1",	VAR_INT,	NULL,	(dt)&_Int1,
"w2",	VAR_INT,	NULL,	(dt)&_Int2,
"w3",	VAR_INT,	NULL,	(dt)&_Int3,
"w4",	VAR_INT,	NULL,	(dt)&_Int4,
"w5",	VAR_INT,	NULL,	(dt)&_Int5,
"w6",	VAR_INT,	NULL,	(dt)&_Int6,
"w7",	VAR_INT,	NULL,	(dt)&_Int7,
"w8",	VAR_INT,	NULL,	(dt)&_Int8,
"w9",	VAR_INT,	NULL,	(dt)&_Int9,
"W0",	VAR_STR,	NULL,	(dt)&_Str0,
"W1",	VAR_STR,	NULL,	(dt)&_Str1,
"W2",	VAR_STR,	NULL,	(dt)&_Str2,
"W3",	VAR_STR,	NULL,	(dt)&_Str3,
"W4",	VAR_STR,	NULL,	(dt)&_Str4,
"W5",	VAR_STR,	NULL,	(dt)&_Str5,
"W6",	VAR_STR,	NULL,	(dt)&_Str6,
"W7",	VAR_STR,	NULL,	(dt)&_Str7,
"W8",	VAR_STR,	NULL,	(dt)&_Str8,
"W9",	VAR_STR,	NULL,	(dt)&_Str9,
"Ti",   VAR_STR,        NULL,   (dt)&_Trans_in,
"To",   VAR_STR,        NULL,   (dt)&_Trans_out,

NULL   , 0              , NULL           , NULL

};

/*******************************************************************************
*                   String of Options (Flags) for piogetopt()                  *
*******************************************************************************/

#ifdef OPTSTRING
#undef OPTSTRING
#endif
#define OPTSTRING  "b:e:g:i:j:l:p:s:t:u:v:w:x:z:E:F:I:J:K:L:V:W:X:Z:"

int	Job_in;
int	Emulation_mode;
int	Vector_mode;

int	Userunit;
int	Userunit_div;

int	CAP_x;
int	CAP_y;
int	CAP_y_dummy;
int	T_pageend;

int	Hmi[4];

int	PS_offset;
int	Pre_offset;
int	Post_offset;
int	V_offset;

int	*Spos_x;
int	*Spos_y;

int	Htab_l;
int	*Htabs;
int	Vtab_l;
int	*Vtabs;

#define	GL	GGG
int	GGG = 0;
int	Save_GL = -1;	/* used by ss2, ss3 */
int	GR = 1;
int	Userunit = 10;
int	Userunit_div = 1;

char	Processcodeset[256];
char	Tmpcodeset[256];
int	Codestate;	/* Kanji-in(1), out(0), or not-used(-1) ? */
int	Incodepageval;
int	Hard_reset = 0;
int	Zeroval = 0;
int	Vpagelength;
int	Rvmi;
struct shar_vars	sharevars;

int	Underline;
int	Mesh;
int	Reverse;
int	Secret;
int	Pattfill;
int	Shadow;
int	Outline;


