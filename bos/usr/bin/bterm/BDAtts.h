/* @(#)75	1.1  src/bos/usr/bin/bterm/BDAtts.h, libbidi, bos411, 9428A410j 8/26/93 13:34:41 */
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: is_att_arabic_nss
 *		is_att_auto_csd
 *		is_att_basic_csd
 *		is_att_bilingual_nss
 *		is_att_explicit
 *		is_att_final_csd
 *		is_att_hindu_nss
 *		is_att_implicit
 *		is_att_initial_csd
 *		is_att_isolated_csd
 *		is_att_middle_csd
 *		is_att_passthru_csd
 *		is_att_passthru_nss
 *		is_att_visual
 *		set_att_arabic_nss
 *		set_att_auto_csd
 *		set_att_basic_csd
 *		set_att_bilingual_nss
 *		set_att_explicit
 *		set_att_final_csd
 *		set_att_hindu_nss
 *		set_att_implicit
 *		set_att_initial_csd
 *		set_att_isolated_csd
 *		set_att_middle_csd
 *		set_att_passthru_csd
 *		set_att_passthru_nss
 *		set_att_visual
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
#ifndef	 _BDAtts_
#define  _BDAtts_

struct BDAtts
{
	unsigned char text;	/* Visual, Implicit, Explicit    */
        unsigned char nss;	/* Numeral Shape Selection       */
	unsigned char csd;	/* Character Shape Determination */
	unsigned char spare;	/* Spare character               */
};

typedef	struct BDAtts	BDAtts_st;
# define	BDATTSNULL	((BDAtts_st *) NULL)

/* defines for Text Level bits */
#define BDAtts_visual	0x1
#define BDAtts_implicit	0x2
#define BDAtts_explicit	0x4

/* defines for CSD bits */
#define BDAtts_auto_csd		0x1
#define BDAtts_pass_csd		0x2
#define	BDAtts_isolated_csd	0x4
#define BDAtts_initial_csd	0x8
#define BDAtts_middle_csd	0x10
#define	BDAtts_final_csd	0x20
#define	BDAtts_basic_csd	0x40

/* defines for NSS bits */
#define BDAtts_bilingual_nss	0x1
#define BDAtts_pass_nss		0x2
#define	BDAtts_hindu_nss	0x4
#define BDAtts_arabic_nss	0x8

/*  defines for status properties */
#define SCREEN_PROPERTY             10
#define LANG_PROPERTY               20
#define LEFT_AUTOPUSH_PROPERTY      30
#define RIGHT_AUTOPUSH_PROPERTY     40
#define SYMMETRIC_PROPERTY          50
#define TEXT_PROPERTY               60
#define BIDI_FUNCTIONAL_PROPERTY    99

/* macros for checking Text Level */
#define is_att_visual(att)	(att.text & BDAtts_visual)
#define is_att_implicit(att)	(att.text & BDAtts_implicit)
#define is_att_explicit(att)	(att.text & BDAtts_explicit)

/* macros for setting Text Level */
#define set_att_visual(att)	att.text = BDAtts_visual
#define set_att_implicit(att)	att.text = BDAtts_implicit
#define set_att_explicit(att)	att.text = BDAtts_explicit

/* macros for checking Charater Shapes */
#define is_att_auto_csd(att)	  (att.csd & BDAtts_auto_csd)
#define is_att_passthru_csd(att)  (att.csd & BDAtts_pass_csd)
#define is_att_isolated_csd(att)  (att.csd & BDAtts_isolated_csd)
#define is_att_initial_csd(att)	  (att.csd & BDAtts_initial_csd)
#define is_att_middle_csd(att)	  (att.csd & BDAtts_middle_csd)
#define is_att_final_csd(att)	  (att.csd & BDAtts_final_csd)
#define is_att_basic_csd(att)	  (att.csd & BDAtts_basic_csd)

/* macros for setting Charater Shapes */
#define set_att_auto_csd(att)	  att.csd = BDAtts_auto_csd
#define set_att_passthru_csd(att) att.csd = BDAtts_pass_csd
#define set_att_isolated_csd(att) att.csd = BDAtts_isolated_csd
#define set_att_initial_csd(att)  att.csd = BDAtts_initial_csd
#define set_att_middle_csd(att)	  att.csd = BDAtts_middle_csd
#define set_att_final_csd(att)	  att.csd = BDAtts_final_csd
#define set_att_basic_csd(att)	  att.csd = BDAtts_basic_csd


/* macros for checking Numerals Shapes */
#define is_att_passthru_nss(att)  (att.nss & BDAtts_pass_nss)
#define is_att_bilingual_nss(att) (att.nss & BDAtts_bilingual_nss)
#define is_att_hindu_nss(att)	  (att.nss & BDAtts_hindu_nss)
#define is_att_arabic_nss(att)	  (att.nss & BDAtts_arabic_nss)

/* macros for setting Numerals Shapes */
#define set_att_passthru_nss(att)  att.nss = BDAtts_pass_nss
#define set_att_bilingual_nss(att) att.nss = BDAtts_bilingual_nss
#define set_att_hindu_nss(att)	   att.nss = BDAtts_hindu_nss
#define set_att_arabic_nss(att)	   att.nss = BDAtts_arabic_nss

#endif /* _BDAtts_ */
