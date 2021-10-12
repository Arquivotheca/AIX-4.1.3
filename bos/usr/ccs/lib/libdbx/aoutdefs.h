/* @(#)27	1.2  src/bos/usr/ccs/lib/libdbx/aoutdefs.h, libdbx, bos411, 9428A410j 6/15/90 20:35:46 */
#ifndef _h_aoutdefs
#define _h_aoutdefs
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: (macros) BTYPE, DECREF, ISARGV, ISARY, ISFCN, ISPTR, ISREGV,
 *		       ISTELT, ISTRTYP
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */

/*		common object file format #include's */

/*	 added for convenience */
#define ISTELT(c)	((((c)&N_CLASS)==C_MOS) || (((c)&N_CLASS)==C_MOU) || (((c)&N_CLASS)==C_MOE) || (((c)&N_CLASS)==C_FIELD))
#define ISREGV(c)	((((c)&N_CLASS)==C_REG) || (((c)&N_CLASS)==C_REGPARM))
#define ISARGV(c)	((((c)&N_CLASS)==C_ARG) || (((c)&N_CLASS)==C_REGPARM))
#define ISTRTYP(c)	((c==T_STRUCT) || (c==T_UNION) || (c==T_ENUM))
/*
 #define ISSTTAG(c)	((((c)&N_CLASS)==C_STRTAG) || (((c)&N_CLASS)==C_UNTAG) || (((c)&N_CLASS)==C_ENTAG))
*/
#define MAXAUXENT	1	/* max number auxilliary entries */

#define SYMENT	struct syment

/* This stuff should probably be put in a.out.h */

#ifndef FLEX
#define SYMNMLEN 8		/* length of symbol name */
#endif
/*
#define N_BTMASK 0xf
*/
#define  N_TMASK      060
#define  N_BTSHFT     4
#define  N_TSHIFT     2

#define  BTYPE(x)  ((x) & N_BTMASK)
#define  ISARY(x)  (((x) & N_TMASK) == (DT_ARY << N_BTSHFT))
#define  ISFCN(x)  (((x) & N_TMASK) == (DT_FCN << N_BTSHFT))
#define  ISPTR(x)  (((x) & N_TMASK) == (DT_PTR << N_BTSHFT))
/*
 #define ISTAG(x)  ((((x)&N_CLASS))==C_STRTAG || (((x)&N_CLASS))==C_UNTAG || (((x)&N_CLASS))==C_ENTAG)
 #define  INCREF(x) ((((x)&~N_BTMASK)<<N_TSHIFT)|(DT_PTR<<N_BTSHFT)|\
			(x&N_BTMASK))
*/
#define  DECREF(x) ((((x)>>N_TSHIFT)&~N_BTMASK)|((x)&N_BTMASK))

#endif /* _h_aoutdefs */
