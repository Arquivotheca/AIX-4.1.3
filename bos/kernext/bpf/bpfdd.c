static char sccsid[] = "@(#)85	1.4  src/bos/kernext/bpf/bpfdd.c, sysxbpf, bos411, 9437A411a 9/9/94 16:00:41";
/*
 *   COMPONENT_NAME: SYSXBPF
 *
 *   FUNCTIONS: 
 *		bpfconfig
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/user.h>
#include <sys/mbuf.h>
#include <net/if.h>

#include <net/bpf.h>
#include <net/bpfdesc.h>

extern struct bpf_d	bpf_dtab[];

static int	load = 0;
static dev_t	devno[NBPFILTER];
lock_t		cfg_lock = LOCK_AVAIL;

bpfconfig(cmd, uio)
	int cmd;
	struct uio *uio;
{
	struct bpf_config bpf_config;
	register struct ifnet *ifp;
	int		error;
	int		i;
	int		s;
	struct devsw	devsw;
	extern int	bpfopen();
	extern int	bpfclose();
	extern int	bpfread();
	extern int	bpfioctl();
	extern int	bpfselect();
	extern		int nodev();

	devsw.d_open     = (int(*)())bpfopen;
	devsw.d_close    = (int(*)())bpfclose;
	devsw.d_read     = (int(*)())bpfread;
	devsw.d_write    = nodev;
	devsw.d_ioctl    = (int(*)())bpfioctl;
	devsw.d_strategy = nodev;
	devsw.d_ttys     = NULL;
	devsw.d_select   = (int(*)())bpfselect;
	devsw.d_config   = nodev;
	devsw.d_print    = nodev;
	devsw.d_dump     = nodev;
	devsw.d_mpx      = nodev;
	devsw.d_revoke   = nodev;
	devsw.d_dsdptr   = NULL;
	devsw.d_selptr   = NULL;
	devsw.d_opts     = 0;

        if ( ((cmd != CFG_INIT) || (uio == NULL)) )
		return(EINVAL);
	
	if (!load)
		bzero((char *)devno, (sizeof(dev_t) * NBPFILTER));

	if (error = uiomove(&bpf_config, sizeof(bpf_config), UIO_WRITE, uio))
		return(error);

	lockl(&cfg_lock, LOCK_SHORT);
	if (!(error = (load) ? 0 : pincode(bpfconfig))) {
	/* keep track of the devices we add to the device switch table */
	/* and don't add the same one twice.                           */
		for (i = 0; i < NBPFILTER; ++i) {
			if (devno[i] == 0) {
				if (error = devswadd(bpf_config.devno,&devsw)) {
					unlockl(&cfg_lock);
					return (error);
				}
				else
					devno[i] = bpf_config.devno;
				break;
			} else 
				if (devno[i] == bpf_config.devno)
					break;
		}
	}
	else {
		unlockl(&cfg_lock);
		return (error);
	}

	if (!load) {
		load++;
		for (i=0; i<NBPFILTER; i++) 
			BPF_LOCKINIT((struct bpf_d *)&bpf_dtab[i]);
	}
	for (ifp = ifnet; ifp; ifp = (struct ifnet *)ifp->if_next) {
		if ((ifp->if_flags & IFF_BPF) && !ifp->if_tapctl)
			bpfattach(&ifp->if_tapctl, ifp, ifp->if_type, 
				ifp->if_hdrlen);
	}

	unlockl(&cfg_lock);
	return(0);
}
