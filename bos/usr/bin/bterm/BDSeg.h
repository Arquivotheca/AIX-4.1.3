/* @(#)76	1.1  src/bos/usr/bin/bterm/BDSeg.h, libbidi, bos411, 9428A410j 8/26/93 13:34:43 */
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: is_LTR_mode
 *		is_RTL_mode
 *		is_aix_mode
 *		is_arabic
 *		is_arabic_nss
 *		is_asd_on
 *		is_basic_csd
 *		is_bidi_mode
 *		is_bilingual_nss
 *		is_explicit_text
 *		is_fASD_on
 *		is_fAutoPush_on
 *		is_fEndPush_on
 *		is_fFldRev_on
 *		is_fLTR_on
 *		is_fPush_on
 *		is_fRTL_on
 *		is_fScrRev_on
 *		is_fShapeF_on
 *		is_fShapeIN_on
 *		is_fShapeIS_on
 *		is_fShapeM_on
 *		is_fShapeP_on
 *		is_final_csd
 *		is_hebrew
 *		is_hindu_nss
 *		is_host_mode
 *		is_implicit_text
 *		is_in_autopush
 *		is_initial_csd
 *		is_isolated_csd
 *		is_latin_kbd
 *		is_left_autopush
 *		is_middle_csd
 *		is_nl_kbd
 *		is_nonulls_mode
 *		is_not_bidi_mode
 *		is_nulls_mode
 *		is_onecell_mode
 *		is_passthru_csd
 *		is_passthru_nss
 *		is_push_mode
 *		is_right_autopush
 *		is_screen_reverse
 *		is_symetric_mode
 *		is_visual_text
 *		reset_bidi_mode
 *		reset_in_autopush
 *		reset_left_autopush
 *		reset_onecell_mode
 *		reset_push_mode
 *		reset_right_autopush
 *		reset_symetric_mode
 *		set_LTR_mode
 *		set_RTL_mode
 *		set_aix_mode
 *		set_arabic_lang
 *		set_arabic_nss
 *		set_asd_on
 *		set_basic_csd
 *		set_bidi_mode
 *		set_bilingual_nss
 *		set_explicit_text
 *		set_fASD_off
 *		set_fASD_on
 *		set_fAutoPush_off
 *		set_fAutoPush_on
 *		set_fEndPush_off
 *		set_fEndPush_on
 *		set_fFldRev_off
 *		set_fFldRev_on
 *		set_fLTR_off
 *		set_fLTR_on
 *		set_fPush_off
 *		set_fPush_on
 *		set_fRTL_off
 *		set_fRTL_on
 *		set_fScrRev_off
 *		set_fScrRev_on
 *		set_fShapeF_off
 *		set_fShapeF_on
 *		set_fShapeIN_off
 *		set_fShapeIN_on
 *		set_fShapeIS_off
 *		set_fShapeIS_on
 *		set_fShapeM_off
 *		set_fShapeM_on
 *		set_fShapeP_on
 *		set_final_csd
 *		set_hebrew_lang
 *		set_hindu_nss
 *		set_host_mode
 *		set_implicit_text
 *		set_in_autopush
 *		set_initial_csd
 *		set_isolated_csd
 *		set_latin_kbd
 *		set_left_autopush
 *		set_middle_csd
 *		set_nl_kbd
 *		set_nonulls_mode
 *		set_nulls_mode
 *		set_onecell_mode
 *		set_passthru_csd
 *		set_passthru_nss
 *		set_push_mode
 *		set_right_autopush
 *		set_symetric_mode
 *		set_visual_text
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *   OBJECT CODE ONLY SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _BDSEG_
#define _BDSEG_

typedef	unsigned char	ui1;

#define FALSE 0
#define TRUE 1

int  reversely_selected_region = FALSE;

#include	"BDAtts.h"

struct BDSeg
{
	struct	BDSeg	*next;	/* Next Saved Bidi Segment        */
	long	Length;		/* length of control block        */
        long	SessionId;	/* Session Identifier             */
	long	BDFkeyFlags;	/* 1 bit to enable each Bidi fkey */
	ui1	bidi_mode;        /* whether bidi enabled or disabled */
	ui1	keys_enabled;     /* whether bidi keys enabled or disabled */
	ui1	natural_lang_kbd; /* keyboard latin or national lang */
	ui1	field_reverse;    /* not used in bterm */
	ui1	screen_reverse;   /* screen orientation LTR=0 RTL=1*/
	ui1	symetric_mode;    /* symmetric swapping on or off */
        ui1     current_lang;     /* current language 1=Arabic */
        ui1     OSflag;           
        ui1     onecell;
        ui1     nonulls;
	ui1	left_autopush_enb;
	ui1	right_autopush_enb;
	ui1	in_autopush_mode;
	ui1	push_mode;
	short	push_start_loc;
	short	push_cur_loc;
	short	push_count;
	BDAtts_st  BDAtts;	/* BiDi Attributes           	*/
	char	   Maps[128];	/* path to Bidi Map  directory  */
};

typedef	struct BDSeg	BDSeg_st;
# define	BDSEGNULL	((BDSeg_st *) NULL)
# define	BDSEGSIZE	(sizeof(BDSeg_st))

/*  define for BDSeg variables */
#define	BDLength		BDCurSeg->Length
#define	BDSessionId		BDCurSeg->SessionId
#define	BDMaps			BDCurSeg->Maps
#define BD_keys_enabled		BDCurSeg->keys_enabled
#define BD_push_start_loc	BDCurSeg->push_start_loc
#define BD_push_cur_loc		BDCurSeg->push_cur_loc
#define BD_push_mode		BDCurSeg->push_mode
#define BD_push_count		BDCurSeg->push_count

/* defines for checking Bidi State */
#define	is_bidi_mode()		(BDCurSeg->bidi_mode)
#define	is_not_bidi_mode()	(!is_bidi_mode())
#define	is_nl_kbd()		(BDCurSeg->natural_lang_kbd)
#define	is_latin_kbd()		(! is_nl_kbd())
#define	is_push_mode()		(BDCurSeg->push_mode)
#define	is_left_autopush()	(BDCurSeg->left_autopush_enb)
#define	is_right_autopush()	(BDCurSeg->right_autopush_enb)
#define	is_in_autopush()	(BDCurSeg->in_autopush_mode)
#define is_symetric_mode()	(BDCurSeg->symetric_mode)
#define is_LTR_mode()		(!is_RTL_mode())
#define is_RTL_mode()		(BDCurSeg->screen_reverse)
#define	is_screen_reverse()	(BDCurSeg->screen_reverse)
#define is_host_mode()          (BDCurSeg->OSflag)
#define is_aix_mode()           (! is_host_mode())
#define is_onecell_mode()       (BDCurSeg->onecell)
#define is_arabic()             (BDCurSeg->current_lang)
#define is_hebrew()             (!is_arabic())
#define is_nonulls_mode()       (BDCurSeg->nonulls)
#define is_nulls_mode()         (! is_nonulls_mode())

#define	set_bidi_mode()		(BDCurSeg->bidi_mode = 1)
#define	reset_bidi_mode()	(BDCurSeg->bidi_mode = 0)

#define set_LTR_mode()		(BDCurSeg->screen_reverse = 0)
#define set_RTL_mode()		(BDCurSeg->screen_reverse = 1)
#define	set_latin_kbd()		(BDCurSeg->natural_lang_kbd = 0)
#define	set_nl_kbd()		(BDCurSeg->natural_lang_kbd = 1)
#define set_left_autopush()     (BDCurSeg->left_autopush_enb = 1)
#define reset_left_autopush()   (BDCurSeg->left_autopush_enb = 0)
#define set_right_autopush()    (BDCurSeg->right_autopush_enb = 1)
#define reset_right_autopush()  (BDCurSeg->right_autopush_enb = 0)
#define set_symetric_mode()	(BDCurSeg->symetric_mode = 1)
#define reset_symetric_mode()	(BDCurSeg->symetric_mode = 0)

#define	set_push_mode()		(BDCurSeg->push_mode = 1)
#define	reset_push_mode()	(BDCurSeg->push_mode = 0)
#define	set_in_autopush()	(BDCurSeg->in_autopush_mode = 1)
#define	reset_in_autopush()	(BDCurSeg->in_autopush_mode = 0)
#define set_host_mode()         (BDCurSeg->OSflag = 1)
#define set_aix_mode()          (BDCurSeg->OSflag = 0)
#define set_onecell_mode()      (BDCurSeg->onecell = 1)
#define reset_onecell_mode()    (BDCurSeg->onecell = 0)
#define set_arabic_lang()       (BDCurSeg->current_lang = 1)
#define set_hebrew_lang()       (BDCurSeg->current_lang = 0)
#define set_nonulls_mode()      (BDCurSeg->nonulls = 1)
#define set_nulls_mode()        (BDCurSeg->nonulls = 0)


/* defines for BDFkeyFlags */
#define BD_LTR_flag		0x1	/* fRTL       1=ON, 0=OFF*/
#define BD_RTL_flag		0x2	/* fLTR       1=ON, 0=OFF*/
#define	BD_Spare_flag		0x4	/* reserved   1=ON, 0=OFF*/
#define BD_Push_flag		0x8	/* fPush      1=ON, 0=OFF*/
#define BD_EndPush_flag		0x10	/* fPush      1=ON, 0=OFF*/
#define	BD_AutoPush_flag	0x20	/* fAutoPush  1=ON, 0=OFF*/
#define BD_FldRev_flag		0x40	/* fFldRev    1=ON, 0=OFF*/
#define BD_ScrRev_flag		0x80	/* fScrRev    1=ON, 0=OFF*/
#define BD_ASD_flag		0x100	/* fASD       1=ON, 0=OFF*/
#define BD_ShapeIN_flag		0x200	/* fShapeIN   1=ON, 0=OFF*/
#define BD_ShapeM_flag		0x400	/* fShapeM    1=ON, 0=OFF*/
#define BD_ShapeF_flag		0x800	/* fShapeF    1=ON, 0=OFF*/
#define BD_ShapeIS_flag		0x1000	/* fShapeIS   1=ON, 0=OFF*/
#define BD_ShapeP_flag		0x2000	/* Passthru Shape, reserved*/

/* macros for checking Fkey Flags  */
#define is_fLTR_on()		(BDCurSeg->BDFkeyFlags & BD_LTR_flag)
#define is_fRTL_on()		(BDCurSeg->BDFkeyFlags & BD_RTL_flag)
#define is_fPush_on()		(BDCurSeg->BDFkeyFlags & BD_Push_flag)
#define is_fEndPush_on()	(BDCurSeg->BDFkeyFlags & BD_EndPush_flag)
#define is_fAutoPush_on()	(BDCurSeg->BDFkeyFlags & BD_AutoPush_flag)
#define is_fFldRev_on()		(BDCurSeg->BDFkeyFlags & BD_FldRev_flag)
#define is_fScrRev_on()		(BDCurSeg->BDFkeyFlags & BD_ScrRev_flag)
#define is_fASD_on()		(BDCurSeg->BDFkeyFlags & BD_ASD_flag)
#define is_fShapeIN_on()	(BDCurSeg->BDFkeyFlags & BD_ShapeIN_flag)
#define is_fShapeM_on()		(BDCurSeg->BDFkeyFlags & BD_ShapeM_flag)
#define is_fShapeF_on()		(BDCurSeg->BDFkeyFlags & BD_ShapeF_flag)
#define is_fShapeIS_on()	(BDCurSeg->BDFkeyFlags & BD_ShapeIS_flag)
#define is_fShapeP_on()		(BDCurSeg->BDFkeyFlags & BD_ShapeP_flag)

/* macros for setting key Flags  */
#define set_fLTR_on()		(BDCurSeg->BDFkeyFlags |= BD_LTR_flag)
#define set_fLTR_off()          (BDCurSeg->BDFkeyFlags &= ~BD_LTR_flag)
#define set_fRTL_on()		(BDCurSeg->BDFkeyFlags |= BD_RTL_flag)
#define set_fRTL_off()          (BDCurSeg->BDFkeyFlags &= ~BD_RTL_flag)
#define set_fPush_on()		(BDCurSeg->BDFkeyFlags |= BD_Push_flag)
#define set_fPush_off()         (BDCurSeg->BDFkeyFlags &= ~BD_Push_flag)
#define set_fEndPush_on()	(BDCurSeg->BDFkeyFlags |= BD_EndPush_flag)
#define set_fEndPush_off()      (BDCurSeg->BDFkeyFlags &= ~BD_EndPush_flag)
#define set_fAutoPush_on()	(BDCurSeg->BDFkeyFlags |= BD_AutoPush_flag)
#define set_fAutoPush_off()     (BDCurSeg->BDFkeyFlags &= ~BD_AutoPush_flag)
#define set_fFldRev_on()	(BDCurSeg->BDFkeyFlags |= BD_FldRev_flag)
#define set_fFldRev_off()       (BDCurSeg->BDFkeyFlags &= ~BD_FldRev_flag)
#define set_fScrRev_on()	(BDCurSeg->BDFkeyFlags |= BD_ScrRev_flag)
#define set_fScrRev_off()       (BDCurSeg->BDFkeyFlags &= ~BD_ScrRev_flag)
#define set_fASD_on()		(BDCurSeg->BDFkeyFlags |= BD_ASD_flag)
#define set_fASD_off()          (BDCurSeg->BDFkeyFlags &= ~BD_ASD_flag)
#define set_fShapeIN_on()	(BDCurSeg->BDFkeyFlags |= BD_ShapeIN_flag)
#define set_fShapeIN_off()      (BDCurSeg->BDFkeyFlags &= ~BD_ShapeIN_flag)
#define set_fShapeM_on()	(BDCurSeg->BDFkeyFlags |= BD_ShapeM_flag)
#define set_fShapeM_off()       (BDCurSeg->BDFkeyFlags &= ~BD_ShapeM_flag)
#define set_fShapeF_on()	(BDCurSeg->BDFkeyFlags |= BD_ShapeF_flag)
#define set_fShapeF_off()       (BDCurSeg->BDFkeyFlags &= ~BD_ShapeF_flag)
#define set_fShapeIS_on()       (BDCurSeg->BDFkeyFlags |= BD_ShapeIS_flag)
#define set_fShapeIS_off()      (BDCurSeg->BDFkeyFlags &= ~BD_ShapeIS_flag)
#define set_fShapeP_on()	(BDCurSeg->BDFkeyFlags |= BD_ShapeP_flag)

/* macros for checking Text Level */
#define is_visual_text()	is_att_visual(BDCurSeg->BDAtts)
#define is_implicit_text()	is_att_implicit(BDCurSeg->BDAtts)
#define is_explicit_text()	is_att_explicit(BDCurSeg->BDAtts)

/* macros for setting Text Level */
#define set_visual_text()	set_att_visual(BDCurSeg->BDAtts)
#define set_implicit_text()	set_att_implicit(BDCurSeg->BDAtts)
#define set_explicit_text()	set_att_explicit(BDCurSeg->BDAtts)

/* macros for checking Charater Shapes */
#define is_asd_on()		is_att_auto_csd(BDCurSeg->BDAtts)
#define is_passthru_csd()	is_att_passthru_csd(BDCurSeg->BDAtts)
#define is_isolated_csd()	is_att_isolated_csd(BDCurSeg->BDAtts)
#define is_initial_csd()	is_att_initial_csd(BDCurSeg->BDAtts)
#define is_middle_csd()		is_att_middle_csd(BDCurSeg->BDAtts)
#define is_final_csd()		is_att_final_csd(BDCurSeg->BDAtts)
#define is_basic_csd()		is_att_basic_csd(BDCurSeg->BDAtts)

/* macros for setting Charater Shapes */
#define set_asd_on()		set_att_auto_csd(BDCurSeg->BDAtts)
#define set_passthru_csd()	set_att_passthru_csd(BDCurSeg->BDAtts)
#define set_isolated_csd()	set_att_isolated_csd(BDCurSeg->BDAtts)
#define set_initial_csd()	set_att_initial_csd(BDCurSeg->BDAtts)
#define set_middle_csd()	set_att_middle_csd(BDCurSeg->BDAtts)
#define set_final_csd()		set_att_final_csd(BDCurSeg->BDAtts)
#define set_basic_csd()		set_att_basic_csd(BDCurSeg->BDAtts)


/* macros for checking Numerals Shapes */
#define is_passthru_nss()	is_att_passthru_nss(BDCurSeg->BDAtts)
#define is_bilingual_nss()	is_att_bilingual_nss(BDCurSeg->BDAtts)
#define is_hindu_nss()		is_att_hindu_nss(BDCurSeg->BDAtts)
#define is_arabic_nss()		is_att_arabic_nss(BDCurSeg->BDAtts)

/* macros for setting Numerals Shapes */
#define set_passthru_nss()	set_att_passthru_nss(BDCurSeg->BDAtts)
#define set_bilingual_nss()	set_att_bilingual_nss(BDCurSeg->BDAtts)
#define set_hindu_nss()		set_att_hindu_nss(BDCurSeg->BDAtts)
#define set_arabic_nss()	set_att_arabic_nss(BDCurSeg->BDAtts)

extern BDSeg_st	*BDCurSeg;
extern BDSeg_st	*BDSegAlloc();
extern	char	*BDGetBidiPath();

#endif _BDSEG_
