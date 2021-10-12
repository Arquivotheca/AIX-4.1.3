/* @(#)84	1.6  src/bos/usr/include/uinfo.h, sysproc, bos411, 9428A410j 4/2/93 13:48:33 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: usrinfo
 *		
 *
 *   ORIGINS: 9
 *
 *
 * Copyright 1983, INTERACTIVE Systems Corporation
 *
 * RESTRICTED RIGHTS:   Use, duplication or disclosure is subject to
 *                      restrictions stated in your contract with
 *                      Interactive Systems Corp.
 *
 * NAME:        uinfo.h
 *
 * PURPOSE:     This file defines command codes for the usrinfo() system
 *              call and the length of the uinfo buffer.
 *
 */

#ifndef _H_UINFO
#define _H_UINFO

#define UINFO
#define UINFOSIZ        64      /* size of user info buffer             */
#define SETUINFO        1       /* set user info command code           */
#define GETUINFO        2       /* get user info command code           */
typedef char uinfo_t[UINFOSIZ];

#endif /* _H_UINFO */

