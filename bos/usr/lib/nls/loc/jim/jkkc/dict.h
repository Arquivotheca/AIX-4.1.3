/* @(#)82	1.1  src/bos/usr/lib/nls/loc/jim/jkkc/dict.h, libKJI, bos411, 9428A410j 7/23/92 00:37:00	*/
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 * 
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       dict.h
 *
 * DESCRIPTIVE NAME:  STRUCTURE DEFINITION FOR DICTIONARY INFORMATION
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:
 *
 ******************** END OF SPECIFICATIONS *****************************/

#ifndef _jkkc_DICT
#define	_jkkc_DICT

#define	SDICT_NUM	16		/* System Dict Max Count	*/
#define	SDICT_LEN	80		/* System Dict Max Length	*/

/*-----------------------------------------------------------------------*
*	System Dictionary Information structure
*-----------------------------------------------------------------------*/
typedef struct	_Sdictinfo {
	char	*sdictname;	/* system dictionaty name */
	int	dsyfd;		/* system dictionary file descriptor */
	char	*dsyseg;	/* address of shared memory segment */
	char	*sxesxe;	/* allocated addr of sys dict to be freed */
}	Sdictinfo, *SDICTINFO;

/*-----------------------------------------------------------------------*
*	System Dictionary Information Data structure
*-----------------------------------------------------------------------*/
typedef struct	_Sdictdata {
	int		shmatcnt;	/* the number of the mapped files */
	Sdictinfo	sdictinfo[SDICT_NUM];	/* system dict info */
}	Sdictdata, *SDICTDATA;

/*-----------------------------------------------------------------------*
*	User Dictionary Information structure
*-----------------------------------------------------------------------*/
typedef struct	_Udictinfo {
	char	*udictname;	/* user dictionaty name */
	int	dusfd;		/* user dictionary file descriptor */
	long	indlen;		/* record length of user dictionary */
	char	*mdemde;	/* address of user dictionary */
	char	*uxeuxe;	/* address of user dictionary */
	long	dumtime;	/* Time of last data modification */
	long	ductime;	/* Time of last file status change */
}	Udictinfo, *UDICTINFO;

/*-----------------------------------------------------------------------*
*	FUZOKUGO Dictionary Information structure
*-----------------------------------------------------------------------*/
typedef struct	_Fdictinfo {
	char	*fdictname;	/* FUZOKUGO dictionaty name */
	int	dfzfd;		/* FUZOKUGO dictionary file descriptor */
	char	*dfgdfg;	/* address of FUZOKUGO dictionary */
}	Fdictinfo, *FDICTINFO;

#endif	/* _jkkc_DICT */
