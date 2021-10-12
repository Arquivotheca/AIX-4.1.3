/* @(#)42	1.9  src/bos/usr/include/cur03.h, libcurses, bos411, 9428A410j 6/10/91 18:36:22 */
#ifndef _H_CUR03
#define _H_CUR03

/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: cur03.h
 *
 * ORIGINS: 10, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:                cur03.h
 *
 * FUNCTION: This file contains #defined constants
 *      which are translated later to unique bit patterns for the speci-
 *      fication of attributes, colors, and fonts.  (see also attrib.c)
 */

#define         _dNORMAL        0

#define         _dREVERSE       -1
#define         _dBOLD          -2
#define         _dBLINK         -3
#define         _dUNDERSCORE    -4
#define         _dDIM           -5
#define         _dINVISIBLE     -6
#define         _dPROTECTED     -7
#define         _dSTANDOUT      -8

#define         _dF_BLACK       -11
#define         _dF_RED         -12
#define         _dF_GREEN       -13
#define         _dF_BROWN       -14
#define         _dF_BLUE        -15
#define         _dF_MAGENTA     -16
#define         _dF_CYAN        -17
#define         _dF_WHITE       -18

#define         _dB_BLACK       -21
#define         _dB_RED         -22
#define         _dB_GREEN       -23
#define         _dB_BROWN       -24
#define         _dB_BLUE        -25
#define         _dB_MAGENTA     -26
#define         _dB_CYAN        -27
#define         _dB_WHITE       -28

#define         _dFONT0         -31
#define         _dFONT1         -32
#define         _dFONT2         -33
#define         _dFONT3         -34
#define         _dFONT4         -35
#define         _dFONT5         -36
#define         _dFONT6         -37
#define         _dFONT7         -38

#define		_dTOPLINE	-40
#define		_dBOTTOMLINE	-41
#define		_dRIGHTLINE	-42
#define		_dLEFTLINE	-43

#endif				/* _H_CUR03 */
