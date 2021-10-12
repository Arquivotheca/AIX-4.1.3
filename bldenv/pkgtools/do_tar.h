/* @(#)94       1.2  src/bldenv/pkgtools/do_tar.h, pkgtools, bos412, GOLDA411a 1/29/93 17:26:22 */
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 18,27,71
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log$
 *
 */
/* RCSfile Revision (OSF) Date */
/* $Source: /u/mark/src/pax/RCS/pax.h,v $
 *
 * $Revision: 1.2 $
 *
 * pax.h - defnitions for entire program
 *
 * DESCRIPTION
 *
 *	This file contains most all of the definitions required by the PAX
 *	software.  This header is included in every source file.
 *
 * AUTHOR
 *
 *     Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
 *
 * Sponsored by The USENIX Association for public distribution. 
 *
 * Copyright (c) 1989 Mark H. Colburn.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Mark H. Colburn and sponsored by The USENIX Association. 
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _h_DO_TAR
#define _h_DO_TAR

#include "adepackage.h"

#define BLOCKSIZE    512
#define TMAGIC       "ustar"
#define TMAGLEN      6
#define TVERSION     "00"
#define TVERSLEN     2

#define REGTYPE     '0'
#define LNKTYPE     '1'
#define SYMTYPE     '2'
#define CHRTYPE     '3'
#define BLKTYPE     '4'
#define DIRTYPE     '5'
#define FIFOTYPE    '6'
#define CONTTYPE    '7'

#define TUNMLEN     32
#define TGNMLEN     32

/*
 *  Private function prototypes
 */

int write_tar_file ( Tapeinfo * , Fileinfo *);
char *finduname ( int );
char *findgname ( int );
char tartype ( int );

#endif
