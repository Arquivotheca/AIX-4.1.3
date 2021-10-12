/* @(#)40	1.1  src/bos/usr/lib/nls/loc/imk/include/deadtable.h, libkr, bos411, 9428A410j 5/25/92 15:36:18 */
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		deadtable.h
 *
 * ORIGINS :		27
 *
 * (C) COPYRIGHT International Business Machines Corp.  1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_deadtable_h
#define	_deadtable_h

typedef struct _DeadTable	{
	unsigned int	alpha;
	unsigned int	diac;
}	DeadTable;

static const DeadTable	abovedot[] = {
	{ XK_C, XK_Cabovedot }, { XK_E, XK_Eabovedot },
	{ XK_G, XK_Gabovedot }, { XK_I, XK_Iabovedot },
	{ XK_Z, XK_Zabovedot }, { XK_c, XK_cabovedot },
	{ XK_e, XK_eabovedot }, { XK_g, XK_gabovedot },
	{ XK_z, XK_zabovedot },
};

static const DeadTable	acute[] = {
	{ XK_A, XK_Aacute }, { XK_C, XK_Cacute },
	{ XK_E, XK_Eacute }, { XK_I, XK_Iacute },
	{ XK_L, XK_Lacute }, { XK_N, XK_Nacute },
	{ XK_O, XK_Oacute }, { XK_R, XK_Racute },
	{ XK_S, XK_Sacute }, { XK_U, XK_Uacute },
	{ XK_Y, XK_Yacute }, { XK_Z, XK_Zacute },
	{ XK_a, XK_aacute }, { XK_c, XK_cacute },
	{ XK_e, XK_eacute }, { XK_i, XK_iacute },
	{ XK_l, XK_lacute }, { XK_n, XK_nacute },
	{ XK_o, XK_oacute }, { XK_r, XK_racute },
	{ XK_s, XK_sacute }, { XK_u, XK_uacute },
	{ XK_y, XK_yacute }, { XK_z, XK_zacute },
};

static const DeadTable	breve[] = {
	{ XK_A, XK_Abreve }, { XK_G, XK_Gbreve },
	{ XK_U, XK_Ubreve }, { XK_a, XK_abreve },
	{ XK_g, XK_gbreve }, { XK_u, XK_ubreve },
};

static const DeadTable	caron[] = {
	{ XK_C, XK_Ccaron }, { XK_D, XK_Dcaron },
	{ XK_E, XK_Ecaron }, { XK_L, XK_Lcaron },
	{ XK_N, XK_Ncaron }, { XK_R, XK_Rcaron },
	{ XK_S, XK_Scaron }, { XK_T, XK_Tcaron },
	{ XK_Z, XK_Zcaron }, { XK_c, XK_ccaron },
	{ XK_d, XK_dcaron }, { XK_e, XK_ecaron },
	{ XK_l, XK_lcaron }, { XK_n, XK_ncaron },
	{ XK_r, XK_rcaron }, { XK_s, XK_scaron },
	{ XK_t, XK_tcaron }, { XK_z, XK_zcaron },
};

static const DeadTable	cedilla[] = {
	{ XK_C, XK_Ccedilla }, { XK_G, XK_Gcedilla },
	{ XK_K, XK_Kcedilla }, { XK_L, XK_Lcedilla },
	{ XK_N, XK_Ncedilla }, { XK_R, XK_Rcedilla },
	{ XK_S, XK_Scedilla }, { XK_T, XK_Tcedilla },
	{ XK_c, XK_ccedilla }, { XK_g, XK_gcedilla },
	{ XK_k, XK_kcedilla }, { XK_l, XK_lcedilla },
	{ XK_n, XK_ncedilla }, { XK_r, XK_rcedilla },
	{ XK_s, XK_scedilla }, { XK_t, XK_tcedilla },
};

static const DeadTable	circumflex[] = {
	{ XK_A, XK_Acircumflex }, { XK_C, XK_Ccircumflex },
	{ XK_E, XK_Ecircumflex }, { XK_G, XK_Gcircumflex },
	{ XK_H, XK_Hcircumflex }, { XK_I, XK_Icircumflex },
	{ XK_J, XK_Jcircumflex }, { XK_O, XK_Ocircumflex },
	{ XK_S, XK_Scircumflex }, { XK_U, XK_Ucircumflex },
	{ XK_a, XK_acircumflex }, { XK_c, XK_ccircumflex },
	{ XK_e, XK_ecircumflex }, { XK_g, XK_gcircumflex },
	{ XK_h, XK_hcircumflex }, { XK_i, XK_icircumflex },
	{ XK_j, XK_jcircumflex }, { XK_o, XK_ocircumflex },
	{ XK_s, XK_scircumflex }, { XK_u, XK_ucircumflex },
};

static const DeadTable	degree[] = {
	{ XK_A, XK_Aring }, { XK_U, XK_Uring },
	{ XK_a, XK_aring }, { XK_u, XK_uring },
};

static const DeadTable	diaeresis[] = {
	{ XK_A, XK_Adiaeresis }, { XK_E, XK_Ediaeresis },
	{ XK_I, XK_Idiaeresis }, { XK_O, XK_Odiaeresis },
	{ XK_U, XK_Udiaeresis }, { XK_a, XK_adiaeresis },
	{ XK_e, XK_ediaeresis }, { XK_i, XK_idiaeresis },
	{ XK_o, XK_odiaeresis }, { XK_u, XK_udiaeresis },
	{ XK_y, XK_ydiaeresis },
};

static const DeadTable	doubleacute[] = {
	{ XK_O, XK_Odoubleacute }, { XK_U, XK_Udoubleacute },
	{ XK_o, XK_odoubleacute }, { XK_u, XK_udoubleacute },
};

static const DeadTable	grave[] = {
	{ XK_A, XK_Agrave }, { XK_E, XK_Egrave },
	{ XK_I, XK_Igrave }, { XK_O, XK_Ograve },
	{ XK_U, XK_Ugrave }, { XK_a, XK_agrave },
	{ XK_e, XK_egrave }, { XK_i, XK_igrave },
	{ XK_o, XK_ograve }, { XK_u, XK_ugrave },
};

static const DeadTable	macron[] = {
	{ XK_A, XK_Amacron }, { XK_E, XK_Emacron },
	{ XK_I, XK_Imacron }, { XK_O, XK_Omacron },
	{ XK_U, XK_Umacron }, { XK_a, XK_amacron },
	{ XK_e, XK_emacron }, { XK_i, XK_imacron },
	{ XK_o, XK_omacron }, { XK_u, XK_umacron },
};

static const DeadTable	ogonek[] = {
	{ XK_A, XK_Aogonek }, { XK_E, XK_Eogonek },
	{ XK_I, XK_Iogonek }, { XK_U, XK_Uogonek },
	{ XK_a, XK_aogonek }, { XK_e, XK_eogonek },
	{ XK_i, XK_iogonek }, { XK_u, XK_uogonek },
};

static const DeadTable	tilde[] = {
	{ XK_A, XK_Atilde }, { XK_I, XK_Itilde },
	{ XK_N, XK_Ntilde }, { XK_O, XK_Otilde },
	{ XK_U, XK_Utilde }, { XK_a, XK_atilde },
	{ XK_i, XK_itilde }, { XK_n, XK_ntilde },
	{ XK_o, XK_otilde }, { XK_u, XK_utilde },
};

#endif	/* _deadtable_h */
