/* @(#)16	1.3  src/bos/usr/include/fcs.h, libiconv, bos411, 9428A410j 7/16/91 03:05:28
 *
 * COMPONENT_NAME: (LIBICONV)
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define	False	0
#define	True	1

typedef struct _EscTbl	EscTbl;
typedef struct _EscTbl	{
	char	*name;
	char	*str;
	int	len;
	char	*seg;
	int	seglen;
	int	gl;
};

typedef struct _EscTblTbl	EscTblTbl;
typedef struct _EscTblTbl	{
	char	*name;
	int	netbl;
	int	defgl;
	int	defgr;
	EscTbl	*etbl;
	unsigned char	(*csidx)();
	unsigned char	*isctl;
};

#define	INVALIDCSID	0xff
#define	CONTROLCSID	0xfe
#define	NEEDMORE	0xfd

extern EscTblTbl	_iconv_ct_ett[];
extern EscTblTbl	_iconv_fold7_ett[];
extern EscTblTbl	_iconv_fold8_ett[];
