/* @(#)56       1.1  src/bldenv/sbtools/include/ode/private/odexm.h, bldprocess, bos412, GOLDA411a 1/19/94 17:36:33
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * HISTORY
 * $Log: odexm.h,v $
 * Revision 1.1.6.2  1993/11/10  22:28:25  damon
 * 	CR 463. Swapped ep and sp args of check_arglist()
 * 	[1993/11/10  22:28:11  damon]
 *
 * Revision 1.1.6.1  1993/11/09  16:53:36  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/09  16:52:36  damon]
 * 
 * Revision 1.1.4.2  1993/05/18  18:45:11  damon
 * 	CR 515. Brought ODEXM_HEADER def up to date
 * 	[1993/05/18  18:41:33  damon]
 * 
 * Revision 1.1.2.3  1992/12/03  19:14:12  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:43:19  damon]
 * 
 * Revision 1.1.2.2  1992/09/21  19:46:14  damon
 * 	CR 240. Common place for header
 * 	[1992/09/21  19:45:52  damon]
 * 
 * $EndLog$
 */
#define OXMBUFSIZE 16384
typedef struct {
  unsigned short count;
  int status;
} ODEXM_HEADER;

void
setup_connections(void);
void
authenticate_client(void);
void
read_config_info (void);
void
read_arglist(void);
void
transmit_status(void);
BOOLEAN get_exists ( char * exists );
BOOLEAN get_perm ( u_short * perm );
BOOLEAN get_count ( unsigned int * count );
void
get_buffer ( char * buffer, int * len, int readsize, int fd );
void transmit_status2 (void);
void
handle_io ( int in_fd, int out_fd );
BOOLEAN put_exists ( char * exists );
BOOLEAN put_perm ( u_short * perm );
BOOLEAN put_count ( unsigned int * count );
void
put_buffer ( char * buffer, int len, int * len2, int fd );
void
check_arglist ( int count, char * ep, char * sp, unsigned int checksum );
