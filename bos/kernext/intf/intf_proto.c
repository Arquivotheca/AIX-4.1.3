static char sccsid[] = "@(#)26	1.4  src/bos/kernext/intf/intf_proto.c, sysxintf, bos411, 9433A411a 8/12/94 10:45:17";
/*
 * COMPONENT_NAME: (SYSXINTF) raw interface services 
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
 * INTF protocol family: RAW
 */
int	intf_init();
extern	struct domain intfdomain;
extern	int intf_usrreq();

struct protosw intfsw[] = {
{ SOCK_RAW,	&intfdomain,	0,		PR_ATOMIC|PR_ADDR|PR_INTRLEVEL,
  0,	0,	0,		0,
  intf_usrreq,	0,		0,
  intf_init,	0,		0,		0,
},
{ SOCK_DGRAM,	&intfdomain,	0,		PR_ATOMIC|PR_ADDR|PR_INTRLEVEL,
  0,	0,	0,		0,
  intf_usrreq,	0,		0,
  0,		0,		0,		0,
},
};

struct domain intfdomain =
    { AF_INTF, "intfnet", 0, 0, 0, 
      intfsw, &intfsw[sizeof(intfsw)/sizeof(intfsw[0])] };

