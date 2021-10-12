/* @(#)49	1.1  src/bos/usr/include/pse/cpp.h, sysxpse, bos411, 9428A410j 5/7/91 14:30:34 */
/*
 *   COMPONENT_NAME: LIBCPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _CPP_
#define _CPP_

/** Copyright (c) 1990  Mentat Inc.
 ** cpp.c 2.1, last change 11/14/90
 **/

extern	void	cpp_blast(   char ** cpp   );

extern	char **	cpp_del(   char ** cpp, int position   );

extern	char **	cpp_dup(   char ** cpp   );

extern	char **	cpp_ins(   char ** orig_cpp, int position, char * str   );

extern	int	cpp_len(   char ** cpp   );

extern	void	cpp_print(   char ** cpp   );

#endif
