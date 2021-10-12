/* @(#)51 1.1 src/bos/kernel/sys/sjisioctl.h, sysxtty, bos411, 9428A410j 10/15/93 11:57:52 */
/*
 * COMPONENT_NAME: SYSXTTY
 *
 * FUNCTIONS :
 *
 * ORIGINS: 83
 *
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 1.2
 */

/*
 * @(#)sjisioctl.h      1.2 2/21/91
 */
/************************************************************************
 *                                                                      *
 *                      Copyright (c) 1990 by                           *
 *              Digital Equipment Corporation, Maynard, MA              *
 *                      All rights reserved.                            *
 *                                                                      *
 *   This software is furnished under a license and may be used and     *
 *   copied  only  in accordance with the terms of such license and     *
 *   with the  inclusion  of  the  above  copyright  notice.   This     *
 *   software  or  any  other copies thereof may not be provided or     *
 *   otherwise made available to any other person.  No title to and     *
 *   ownership of the software is hereby transferred.                   *
 *                                                                      *
 *   The information in this software is subject to change  without     *
 *   notice  and should not be construed as a commitment by Digital     *
 *   Equipment Corporation.                                             *
 *                                                                      *
 *   Digital assumes no responsibility for the use  or  reliability     *
 *   of its software on equipment which is not supplied by Digital.     *
 *                                                                      *
 ************************************************************************/

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _H_SJISIOCTL
#define _H_SJISIOCTL

/*
 * EUC-SJIS ioctl's
 */
#define IOC_SJIS_VOID        0x20000000
#define _IO_SJIS(x,y)    (IOC_SJIS_VOID|(x<<8)|y)

#define SJIS_UC_C1SET	_IO_SJIS('J', 0x780)	/* set C1 conversion state for upper */
#define SJIS_UC_C1GET	_IO_SJIS('J', 0x781)	/* get C1 conversion state for upper */
#define SJIS_LC_C1SET	_IO_SJIS('J', 0x782)	/* set C1 conversion state for lower */
#define SJIS_LC_C1GET	_IO_SJIS('J', 0x783)	/* get C1 conversion state for lower */

/*
 * for SJIS_*C_C1SET
 * strioctl.ic_dp should point to an int holding the C1 conversion state
 *
 * for SJIS_*C_C1GET
 * strioctl.ic_dp should point to an int
 * strioctl.ic_len should be size of an int
 */

/*
 * define's for SJIS_*C_C1SET and SJIS_*C_C1GET
 */
#define SJIS_C1_C0	0x01	/* convert C1 to C0 */
#define SJIS_C1_PASS	0x02	/* pass C1 as is */
#define SJIS_C1_THROW	0x03	/* throw away C1 */

#endif /* _H_SJISIOCTL */
