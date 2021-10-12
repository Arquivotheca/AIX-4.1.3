/* @(#)19       1.7  src/bos/usr/include/iconvP.h, libiconv, bos411, 9428A410j 12/15/93 11:32:41
 *
 *   COMPONENT_NAME: LIBICONV
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef __ICONVP_H
#define __ICONVP_H

#include <sys/types.h>


/* Internal use macros ------------------------------------------------------*/

#define ICONV_METHOD_PATH	"/iconv/"
#define ICONV_TABLE_PATH	"/iconvTable/"
#define	UCONV_METHOD_PATH	"/uconv/"
#define	UCONV_TABLE_PATH	"/uconvTable/"
#define	UCONV_ALIAS_FILE	"ucs.alias"
#define	CCSID_PATH		"/ccsid/"
#define	CCSID_TABLE		"ccsid.tbl"
#define DEF_LOCPATH		"/usr/lib/nls/loc:/etc/nls/loc"
#define	DEF_UCS_NAME		"UCS-2"

#define	TRUE			1
#define	FALSE			0

#define	FROM			0
#define	TO			1

/* Substitution characters for codepages ------------------------------------*/

#define	HOST_SUBCHAR		0x3f
#define	ISO88591_SUBCHAR	0x1a
#define	IBM850_SUBCHAR		0x7f
#define	IBM932_SUBCHAR		0x7f
#define	IBM932_D_SUBCHAR_1	0xfc
#define	IBM932_D_SUBCHAR_2	0xfc
#define	EUCJP_SUBCHAR		0x1a
#define	EUCJP_D_SUBCHAR_1	0xf4
#define	EUCJP_D_SUBCHAR_2	0xfe
#define	HOST_D_SUBCHAR_1	0xfe
#define	HOST_D_SUBCHAR_2	0xfe

/* Control code names -------------------------------------------------------*/

#define EUCSS2			0x8e		/* EUC single shift 2        */
#define EUCSS3			0x8f		/* EUC single shift 3        */
#define SO			(uchar_t)0x0e	/* EBCDIC shift out          */
#define SI			(uchar_t)0x0f	/* EBCDIC shift in           */

/* _LC_object_t VERSION number in minor part --------------------------------*/

#define	_LC_ICONV_MODIFIER	0x0001		/* Version for modifier      */

/* Prototypes internal sub routines -----------------------------------------*/

extern	size_t		_ascii_exec (
	uchar_t		**inbuf,	/* Input buffer                      */
	size_t		*inbytesleft,	/* #bytes left in the input buffer   */
	uchar_t		**outbuf,	/* Output buffer                     */
	size_t		*outbytesleft);	/* #bytes left in the output buffer  */

#endif /*!__ICONVP_H*/
