static char sccsid[] = "@(#)30	1.7  src/bos/kernext/xns/ns_init.c, sysxxns, bos411, 9428A410j 3/21/94 15:46:38";
/*
 *   COMPONENT_NAME: SYSXXNS
 *
 *   FUNCTIONS: config_ns
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/uio.h>		
#include <sys/device.h>
#include <sys/domain.h>	
#include <sys/socket.h>
#include <sys/lockl.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include <sys/timer.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/mbuf.h>

#include <sys/cdli.h>
#include <sys/ndd.h>
#include <net/nd_lan.h>

#include <net/if.h>
#include <net/netisr.h>
#include <net/if_types.h>

#include <netns/ns.h>
#include <netns/ns_if.h>

#include <netinet/if_ether.h>

int netns_kmid;		/* kernel mod id, "module name"_kmid */
int extension_loaded = 0;	/* flag indicating extension loaded  */

extern	struct domain nsdomain;
extern	struct ifqueue nsintrq;

extern  add_input_type(), add_domain_af();

/*
 *
 * config_ns - entry point for netns kernel extension
 *
 */
config_ns(cmd, uio)
	int cmd;
	struct uio *uio;
{
	int err, nest;
	struct config_proto config_proto;

	err = 0;
	nest = lockl(&kernel_lock, LOCK_SHORT);

	switch (cmd) {
	case CFG_INIT:
	
		/* check if kernel extension already loaded */	
		if (extension_loaded) 
			goto out;

		ns_lock_init();

		/*
		 * pin the netns kernel extension 
                 */
		 if (err = pincode(config_ns))
			goto out;

		/* Add ns domain */	
		add_domain_af(&nsdomain);
			
                config_proto.loop = nsintr;
                config_proto.loopq = &nsintrq;
                config_proto.netisr = NETISR_NS;
                config_proto.resolve = ns_arpresolve;
                config_proto.ioctl = NULL;
                config_proto.whohas = NULL;
                nd_config_proto(AF_NS, &config_proto);

		extension_loaded++;
	
		break;
	
	case CFG_TERM:
	default:
		err = EINVAL;
	}

out:
	if (nest != LOCK_NEST)
		unlockl(&kernel_lock);
	
	return(err);
}
