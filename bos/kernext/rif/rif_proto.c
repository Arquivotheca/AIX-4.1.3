static char sccsid[] = "@(#)90	1.1  src/bos/kernext/rif/rif_proto.c, sysxrif, bos411, 9428A410j 2/25/91 17:22:36";
/*
 * COMPONENT_NAME: (SYSXRIF) raw interface services 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26, 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/protosw.h>
#include <sys/domain.h>
#include <sys/mbuf.h>

/*
 * RIF protocol family: RAW
 */
int	rif_input(),rif_output(), rif_init();
extern	struct domain rifdomain;
extern	int rif_usrreq();

struct protosw rifsw[] = {
{ SOCK_RAW,	&rifdomain,	0,		PR_ATOMIC|PR_ADDR,
  rif_input,	rif_output,	0,		0,
  rif_usrreq,
  rif_init,	0,		0,		0,
},
{ SOCK_DGRAM,	&rifdomain,	0,		PR_ATOMIC|PR_ADDR,
  rif_input,	rif_output,	0,		0,
  rif_usrreq,
  0,		0,		0,		0,
},
};

struct domain rifdomain =
    { AF_RIF, "rifnet", 0, 0, 0, 
      rifsw, &rifsw[sizeof(rifsw)/sizeof(rifsw[0])] };

