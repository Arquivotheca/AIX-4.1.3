/* @(#)05	1.10  src/bos/usr/include/IN/ERdefs.h, libIN, bos411, 9428A410j 3/22/93 11:33:18 */
/*
 * COMPONENT_NAME: LIBIN
 *
 * FUNCTIONS:
 *
 * ORIGINS: 9,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_ERDEFS
#define _H_ERDEFS

#ifdef MSG
#define ER_S_CREATE     "Cannot create \"%s\"."
#define ER_CREATE(f)    NLgetamsg(MF_LIBIN, MS_LIBIN, M_ER_S_CREATE, ER_S_CREATE), f
#define ER_S_EXEC       "Cannot execute \"%s\"."
#define ER_EXEC(f)      NLgetamsg(MF_LIBIN, MS_LIBIN, M_ER_S_EXEC, ER_S_EXEC), f
#define ER_S_LINK       "Cannot link \"%s\" to \"%s\"."
#define ER_LINK(f1,f2)  NLgetamsg(MF_LIBIN, MS_LIBIN, M_ER_S_LINK, ER_S_LINK), f1, f2
#define ER_S_MEMORY     "Out of memory."
#define ER_MEMORY()     NLgetamsg(MF_LIBIN, MS_LIBIN, M_ER_S_MEMORY, ER_S_MEMORY)
#define ER_S_OPEN       "Cannot open \"%s\"."
#define ER_OPEN(f)      NLgetamsg(MF_LIBIN, MS_LIBIN, M_ER_S_OPEN, ER_S_OPEN), f
#define ER_S_READ       "Cannot read \"%s\"."
#define ER_READ(f)      NLgetamsg(MF_LIBIN, MS_LIBIN, M_ER_S_READ, ER_S_READ), f
#define ER_S_SEEK       "Cannot seek on \"%s\"."
#define ER_SEEK(f)      NLgetamsg(MF_LIBIN, MS_LIBIN, M_ER_S_SEEK, ER_S_SEEK), f
#define ER_S_STAT       "Cannot get status of \"%s\"."
#define ER_STAT(f)      NLgetamsg(MF_LIBIN, MS_LIBIN, M_ER_S_STAT, ER_S_STAT), f
#define ER_S_UNLINK     "Cannot unlink \"%s\"."
#define ER_UNLINK(f)    NLgetamsg(MF_LIBIN, MS_LIBIN, M_ER_S_UNLINK, ER_S_UNLINK), f
#define ER_S_WRITE      "Cannot write \"%s\"."
#define ER_WRITE(f)     NLgetamsg(MF_LIBIN, MS_LIBIN, M_ER_S_WRITE, ER_S_WRITE), f
#define ER_S_WOPEN      "Cannot open \"%s\" for writing."
#define ER_WOPEN(f)     NLgetamsg(MF_LIBIN, MS_LIBIN, M_ER_S_WOPEN, ER_S_WOPEN), f
#else
#define ER_S_CREATE     "Cannot create \"%s\"."
#define ER_CREATE(f)    ER_S_CREATE, f
#define ER_S_EXEC       "Cannot execute \"%s\"."
#define ER_EXEC(f)      ER_S_EXEC, f
#define ER_S_LINK       "Cannot link \"%s\" to \"%s\"."
#define ER_LINK(f1,f2)  ER_S_LINK, f1, f2
#define ER_S_MEMORY     "Out of memory."
#define ER_MEMORY()     ER_S_MEMORY
#define ER_S_OPEN       "Cannot open \"%s\"."
#define ER_OPEN(f)      ER_S_OPEN, f
#define ER_S_READ       "Cannot read \"%s\"."
#define ER_READ(f)      ER_S_READ, f
#define ER_S_SEEK       "Cannot seek on \"%s\"."
#define ER_SEEK(f)      ER_S_SEEK, f
#define ER_S_STAT       "Cannot get status of \"%s\"."
#define ER_STAT(f)      ER_S_STAT, f
#define ER_S_UNLINK     "Cannot unlink \"%s\"."
#define ER_UNLINK(f)    ER_S_UNLINK, f
#define ER_S_WRITE      "Cannot write \"%s\"."
#define ER_WRITE(f)     ER_S_WRITE, f
#define ER_S_WOPEN      "Cannot open \"%s\" for writing."
#define ER_WOPEN(f)     ER_S_WOPEN, f
#endif

void    ERcmderr();
void    ERvcmderr();
char *  ERsysmsg();

#endif /* _H_ERDEFS */
