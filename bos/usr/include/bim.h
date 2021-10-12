/* @(#)40	1.1  src/bos/usr/include/bim.h, cmdims, bos411, 9428A410j 7/8/93 19:44:59 */
/*
 * COMPONENT_NAME: (cmdims) SBCS Input Method
 *
 * FUNCTIONS: HEBREW INPUT METHOD
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_bim_h
#define	_bim_h

/* Key masks. These values come from X11/X.h file. (duplicated). */

#define ShiftMask		(1<<0)
#define LockMask		(1<<1)
#define ControlMask		(1<<2)
#define Mod1Mask		(1<<3)
#define Mod2Mask		(1<<4)
#define Mod3Mask		(1<<5)
#define Mod4Mask		(1<<6)
#define Mod5Mask		(1<<7)
#define MetaMask		Mod1Mask
#define AltGraphMask		Mod2Mask

#define BIM_EXTENDSTATE		Mod3Mask

#define BIM_PARSEKEYS		0x10000000	/* bidi parseKeys */
#define	BIMIOC			('B' << 16)

#define	BIM_ChangeLayer		(0 | BIMIOC)
#define	BIM_ChangeParseKeys	(BIM_ChangeLayer + 1)
#define	BIM_QueryLayer		(BIM_ChangeParseKeys + 1)
#define	BIM_QueryParseKeys	(BIM_QueryLayer + 1)

/*-----------------------------------------------------------------------*
*       Bidi IM Query Layer
*-----------------------------------------------------------------------*/
typedef struct {
	unsigned int layer;
}	BIMQueryLayer;

/*-----------------------------------------------------------------------*
*       Bidi IM Query Parse Keys
*-----------------------------------------------------------------------*/
typedef struct {
	unsigned int parseKeys;
}	BIMQueryParseKeys;


#define BIMLayerMask      0x0fff
#define BIMLatinLayer     0x0100
#define BIMNlsLayer       0x0200

#define BIMKeysMask       0xf000
#define BIMParseKeys      0x1000
#define BIMDontParseKeys  0x2000

#define BIMScrReverseKey       "\033[001B"
#define BIMLatinLayerKey       "\033[002B"
#define BIMNlsLayerKey         "\033[003B"
#define BIMStartPushKey        "\033[004B"
#define BIMEndPushKey          "\033[005B"
#define BIMTogglePushKey       "\033[006B"
#define BIMToggleAutoPushKey   "\033[007B"


#define BIMScrReverseCode           1
#define BIMLatinLayerCode           2
#define BIMNlsLayerCode             3
#define BIMStartPushCode            4
#define BIMEndPushCode              5
#define BIMTogglePushCode           6
#define BIMToggleAutoPushCode       7

/*  Shaping Keys, Arabic specific */

#define BIMToggleAsdKey        "\033[008B"
#define BIMInitialCsdKey       "\033[009B"
#define BIMMiddleCsdKey        "\033[010B"
#define BIMFinalCsdKey         "\033[011B"
#define BIMIsolatedCsdKey      "\033[012B"
#define BIMPassThruCsdKey      "\033[013B"
#define BIMColumnHeadingKey    "\033[014B"

#define BIMToggleAsdCode            8
#define BIMInitialCsdCode           9
#define BIMMiddleCsdCode           10
#define BIMFinalCsdCode            11
#define BIMIsolatedCsdCode         12
#define BIMPassThruCsdCode         13
#define BIMColumnHeadingCode       14

/*  End of Arabic specific part */


#define BIMIsSpecialKey(_str) \
                ((_str[0] == '\033') && \
                 (_str[1] == '[') && \
                 (isdigit(_str[2])) && \
                 (isdigit(_str[3])) && \
                 (isdigit(_str[4])) && \
                 (_str[5] == 'B'))

#define BIMGetKeyNumber(_key) \
                ((_key[2] - '0') * 100 + (_key[3] - '0') * 10 + (_key[4] - '0'))

#endif	/* _bim_h */
