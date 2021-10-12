static char sccsid[] = "@(#)36	1.27  src/bos/kernext/aixif/if_op.c, sysxaixif, bos411, 9437A411a 9/9/94 17:14:16";
/*
 *   COMPONENT_NAME: SYSXAIXIF
 *
 *   FUNCTIONS: collapse
 *		collapse2
 *		config_op
 *		config_op_init
 *		config_op_term
 *		op_attach
 *		op_detach
 *		op_init
 *		op_ioctl
 *		op_netintr
 *		op_output
 *		op_recv
 *		op_reset
 *		op_send
 *		op_statintr
 *		op_txintr
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************
 * 
 * 	if_op.c - Serial optical Network interface to aix device handler
 *
 ****************************************************************************/

#ifndef NSC
#define NSC
#endif  NSC

#include <sys/types.h>		/* u_char, u_long and more	*/
#include <sys/mbuf.h>		/* mbuf structure and macros	*/
#include <sys/socket.h>		/* sockaddrs and such		*/
#include <sys/ioctl.h>		/* SIOC's			*/
#include <sys/errno.h>		/* error values			*/
#include <sys/malloc.h>		/* xmalloc			*/
#include <sys/syspest.h>	/* assert and BUG macros	*/
#include <sys/file.h>		/* file structure and flags	*/
#include <sys/device.h>		/* for CFG_INIT, CFG_TERM	*/
#include <sys/uio.h>		/* uio and iovec structures	*/
#include <sys/lockl.h>		/* for lockl and unlockl	*/
#include <sys/nettrace.h>	/* trace defs			*/
#include <sys/fp_io.h>		/* fp_open flags 		*/
#include <sys/time.h>
#include <sys/syslog.h>		/* For bsdlog			*/

#include <net/if.h>		/* ifnet			*/
#include <net/if_types.h>	/* IF types...useful comment, eh? */

#include <sys/devinfo.h>	/* after if.h so IFF_UP not redef'd */

#include <netinet/in.h>		/* for sockaddr_in ...		*/
#include <netinet/in_var.h>	/* IA_SIN macro			*/
#include <netinet/in_systm.h>	/* needed by ip.h		*/
#include <netinet/ip.h>		/* ip_len ???			*/
#include <netinet/if_op.h>	/* optical IF defines		*/

#include <aixif/net_if.h>	/* network interface to comio 	*/
#include <sys/comio.h>		/* aix comio device driver stuff*/
#include <sys/soluser.h>	/* optical device handler defines */


/*****************************************************************************
 * Optical software status per interface.
 *
 * Each interface is referopced by a network interface structure,
 * op_if, which the routing code uses to locate the interface.
 * This structure contains the hardware address, device handler file ptr...
 ****************************************************************************/
struct	op_softc {
	struct		arpcom op_ac;	/* Optical common part		*/
        struct file     *netfp;         /* device handler file pointer  */
        int     (*fastwrite)();         /* device driver fastwrite entry pt */
	chan_t		chan;		/* device driver channel used for
					   fastwrite */
	int		status;
	u_long		subnetmask;	/* stores ~(netmask of if) */
};
struct op_softc *op_softc = (struct op_softc *)NULL;

#define op_if op_ac.ac_if

int			if_op_count = 0;/* number of units attached	*/
struct netid_list	op_netids;	/* optical netids to start	*/
int			if_op_lock = LOCK_AVAIL; /* attach locking	*/
u_long opmtu;			 /* mtu passed in device request struct */
int init = 0;

void		op_netintr();		/* receive interrupt handler	*/
void		op_statintr();		/* status interrupt handler	*/
void		op_txintr();		/* transmit complete handler	*/
struct mbuf *collapse(
struct mbuf	*mptr,			/* pointer to mbuf chain 	*/
uint		msglen);		/* total length of message	*/

/*
 * NAME: config_op()
 *                                                                    
 * FUNCTION: Serial Optical kernel extension entry point.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * This function gets called when the Optical extension gets loaded 
 * 	or terminated via
 *     	an ifconfig call or the loveley cfgif config method for tcpip.
 *     	config_op initializes the optical socftc struct & calls 
 *     	config_op_init() or config_op_term() to attach/detach to/from the 
 * 	optical device driver.
 *                                                                   
 * (RECOVERY OPERATION:) returns an error if one is received from 
 * 	config_op_init().     
 *
 * (DATA STRUCTURES:) softc gets initialized.  We pin ourselves!
 *
 * RETURNS: 0 or error from config_op_init().
 */  

config_op(cmd, uio)
int		cmd;
struct uio	*uio;
{
	struct	device_req		device;
	int				error = 0;
	int				unit;
	int				lockt;
	char				*cp;

	lockt = lockl(&if_op_lock, LOCK_SHORT);

	if (if_op_count == 0)
		if (error = pincode(config_op))	
			goto out;
	if (init == 0) {
		if (ifsize <= 0)
			ifsize = IF_SIZE;
		op_softc = (struct op_softc *)
		    xmalloc(sizeof(struct op_softc)*ifsize, 2, pinned_heap);
		if (op_softc == (struct op_softc *)NULL) {
		    unpincode(config_op);
		    unlockl(&if_op_lock);
		    return(ENOMEM);
		}
		bzero(op_softc, sizeof(struct op_softc) * ifsize);
		init++;
	}

	if ( cmd != CFG_INIT ) {
		error = EINVAL;
		goto out;
	}

	if (uio) {	
		error = uiomove((caddr_t) &device, (int) sizeof(device), 
                                 UIO_WRITE, uio);
		if (error) 
			goto out;
	
		opmtu = OPMTU;
	}

	cp = device.dev_name;
	while(*cp < '0' || *cp > '9') cp++;
	unit = atoi(cp);

	if (unit >= ifsize) {
		error = ENXIO;
		goto out;
	}

	error = config_op_init(&device, unit);
out:
	if (lockt != LOCK_NEST)
		unlockl(&if_op_lock);
	return(error);
}


/*
 * NAME: config_op_init()
 *                                                                    
 * FUNCTION: Serial Optical interface load initialization.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * This function gets called by config_op.  It sets up the netid structure
 *     	and calls net_attach to attach to the device driver.  If this
 *      succeeds, it then calls op_attach to initialize the interface.
 *
 * (RECOVERY OPERATION:) returns an error if one is received from 
 * 	either net_attach() or op_attach().
 *
 * (DATA STRUCTURES:) op_netids gets initialized.  
 *
 * RETURNS: 0 or error from attach routines.
 */  
config_op_init(device, unit)
register struct	device_req	*device;
int				unit;
{
	struct	kopen_ext		kopen_ext;
	int				rc = 0;
	struct file			*fp;
	char				path[64];

	if (op_softc[unit].netfp != NULL)
		return(EALREADY);
	op_netids.id_count = 1;
	op_netids.id_length = 1;
	op_netids.id[0] = DSAP_INET;

	kopen_ext.status = 0;
	kopen_ext.rx_fn = op_netintr;
	kopen_ext.tx_fn = op_txintr;
	kopen_ext.stat_fn = op_statintr;
	kopen_ext.open_id = unit;

	sprintf(path, "/dev/ops%d", unit);
	rc = fp_open(path, O_RDWR|O_NDELAY, NULL, &kopen_ext, 
		FP_SYS, &fp);
	if (!rc) {
		int chan, devno;

		rc = fp_getdevno(fp, &devno, &chan);
		device->devno = devno;
	}
	if (!rc) {
		rc = net_attach(&kopen_ext, device, &op_netids,
				&op_softc[unit].netfp);
		fp_close(fp);
	}
	if (!rc) { 
		if_op_count++;
		rc = op_attach(unit);
		if (!rc) {
			rc = fp_getdevno(op_softc[unit].netfp,
				&op_softc[unit].op_if.devno,
				&op_softc[unit].op_if.chan);
		}
	}

	return(rc);
}

/*
 * NAME: op_statintr()
 *                                                                    
 * FUNCTION: Serial Optical interface status interrupt handler.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * This function gets called by the device driver.  If the status is 
 * 	notification that the driver start is successful, we call 
 *	net_start_done().  Else, we log an error via net_error().
 *
 * (RECOVERY OPERATION:) errors are logged via net_error().
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: void
 */  
void
op_statintr(unit, sbp)
register u_long			unit;
register struct status_block 	*sbp;
{
	register struct op_softc 	*opp;

	NETTRC2(HKWD_IFEN|hkwd_statintr_in, &op_softc[unit].op_if, sbp->option[0]);

	opp = &op_softc[unit];

	if (sbp->code == CIO_START_DONE) {
		if (sbp->option[0] != CIO_OK)
			net_error(&op_softc[unit].op_if, sbp->option[0], 
                                  op_softc[unit].netfp);
		
		net_start_done(&op_netids, sbp);
	}
	else if (sbp->code == CIO_ASYNC_STATUS &&
		 sbp->option[0] == SOL_NEW_PRID) {
		/* do nothing */
	}
	else
		net_error(&op_softc[unit].op_if, sbp->option[0], 
                          op_softc[unit].netfp);

	NETTRC(HKWD_IFEN|hkwd_statintr_out);
}


/*
 * NAME: op_netintr() 
 *                                                                    
 * FUNCTION: Serial Optical interface data receive intr. handler.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * This function gets called by the device driver when some network action
 * 	is needed by us.  if a data is ready to be received, then we call
 *	call op_recv().  Else error is logged via net_error().
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: void
 */  
void
op_netintr(unit, status, m)
register u_int			unit;
register u_int			*status;
register struct	mbuf		*m;
{
	NETTRC2(HKWD_IFEN|hkwd_netintr_in, &op_softc[unit].op_if, *status);

	if (*status == CIO_OK)
		op_recv(m, unit);
	else {
		net_error(&op_softc[unit].op_if, *status,op_softc[unit].netfp); 
		if (m)
			m_freem(m);
	}

	NETTRC(HKWD_IFEN|hkwd_netintr_out);
}

/*
 * NAME: op_txintr()()
 *                                                                    
 * FUNCTION: Serial Optical interface transmit complete intr. handler.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * This function gets called by device driver.  Since the interface DOES NOT
 * 	request to be interrupted when a xmit is complete, we log this as an
 *	error.
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: void
 */  
void
op_txintr(unit)
u_long unit;
{
	net_error(&op_softc[unit].op_if, INV_TX_INTR, op_softc[unit].netfp); 
}


/*
 * NAME: op_attach()
 *                                                                    
 * FUNCTION: Serial Optical interface logical attach function.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * This function gets called by config_op_init.  It sets up the ifnet
 *	part of the optical softc structure.  It then calls if_attach() and
 *	if_nostat() to initialize the interface.  
 *
 * (RECOVERY OPERATION:) returns an error if one is received from 
 * 	either net_attach() or op_attach().
 *
 * (DATA STRUCTURES:) ifnet part of softc get initialized, ifnet structure 
 *	of the kernel gets optical interface added.
 *
 * RETURNS: 0 
 */  
op_attach(unit)
unsigned 		unit;
{
	register struct ifnet 		*ifp;
	register struct op_softc        *opp; 
	extern int			op_init();
	extern int			op_output();
	struct cio_get_fastwrt		cio_get_fastwrt;
	extern int			op_ioctl();
	extern int			op_reset();
	int				rc;

	NETTRC1(HKWD_IFEN|hkwd_attach_in,  unit);

	if (unit >= ifsize) {
		NETTRC(HKWD_IFEN|hkwd_attach_out);
		return ENXIO;
	}

        opp             = &op_softc[unit];              /* SNMP BASE */
	ifp             = &op_softc[unit].op_if;

	ifp->if_mtu	= opmtu;
	ifp->if_flags 	= 0;
	ifp->if_unit	= unit;
	ifp->if_name    = "so";
	ifp->if_init	= op_init;
	ifp->if_output	= op_output;
	ifp->if_ioctl	= op_ioctl;
	ifp->if_reset	= op_reset;
        ifp->if_type    = IFT_OTHER;
	ifp->if_baudrate= 220000000;

        if (fp_ioctl(opp->netfp, CIO_GET_FASTWRT, &cio_get_fastwrt, 
		NULL) == 0) {
                opp->fastwrite = cio_get_fastwrt.fastwrt_fn;
		opp->chan = cio_get_fastwrt.chan;
	} else 
                opp->fastwrite = NULL;
	

        ifp->if_addrlen = 2;
        ifp->if_hdrlen = 10;

	if_attach(ifp);
	if_nostat(ifp);

	NETTRC(HKWD_IFEN|hkwd_attach_out);
	return(0);
}

/*
 * NAME: op_init()
 *                                                                    
 * FUNCTION: Serial Optical interface logical initialize funtion.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * This function gets called by op_ioctl().  I guess this implies that we 
 *	must call op_ioctl() before usage of the optical interface.  This
 *	happens by ifconfig/cfgif.  op_init simply marks the interface as
 *	"running."
 *
 * (RECOVERY OPERATION:) returns an error if one is received from 
 * 	either net_attach() or op_attach().
 *
 * (DATA STRUCTURES:) softc gets changed.
 *
 * RETURNS: nada
 */  
op_init()
{
	register struct op_softc *	opp;
	register struct ifnet *		ifp;
	int				s;
	int				unit;

	NETTRC(HKWD_IFEN|hkwd_init_in);

	for ( unit = 0; unit < ifsize; unit ++ ) {
		opp = &op_softc[unit];
		ifp = &opp->op_if;

		opp->op_if.if_flags |= IFF_RUNNING;
	}

	NETTRC(HKWD_IFEN|hkwd_init_out);
}

/*
 * NAME: op_ioctl()
 *                                                                    
 * FUNCTION: Serial Optical interface ioctl function.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * This function gets called by ifconfig/cfgif/chgif.  Two basic functions
 *	are supported.  Initialize the interface (via SIFFLAGS) and set
 *	the ip addr associated with this interface (via SIFADDR).
 *
 * (RECOVERY OPERATION:)  Returns an errno if an invalid/unsupported 
 *	request is made.
 *
 * (DATA STRUCTURES:) ifnet struct may get changed (ipaddr added).
 *
 * RETURNS: EINVAL, EAFNOSUPPORT, or 0.
 */  
op_ioctl(ifp, cmd, data)
	register struct ifnet *		ifp;
	int				cmd;
	caddr_t				data;
{
	register struct ifaddr *	ifa = (struct ifaddr *)data;
	register struct op_softc *	opp = &op_softc[ifp->if_unit];
	int				error = 0;
	struct	timestruc_t		ct;

	NETTRC3(HKWD_IFEN|hkwd_ioctl_in, ifp, cmd, data);


	if (!opp->netfp) 
		return(ENODEV);

	switch (cmd) {
	    case (SIOCIFDETACH):
		error = net_detach(opp->netfp);
		if (error)
			bsdlog(LOG_ERR, "if_op: net_detach returned %d\n",
				error);
		opp->netfp = 0;
		break;

            case (SIOCSIFADDR):
                switch (ifa->ifa_addr->sa_family) {
                    case AF_INET:
                        ((struct arpcom *)ifp)->ac_ipaddr =
                                IA_SIN(ifa)->sin_addr;
			opp->subnetmask = ~((struct in_ifaddr *)(ifa))->
				ia_subnetmask;
                        op_init();
                        ifp->if_flags |= IFF_UP;
			curtime(&ct);
			ifp->if_lastchange.tv_sec = (int)ct.tv_sec;
			ifp->if_lastchange.tv_usec = (int)ct.tv_nsec / 1000;
                        break;

                    default:
                        error = EAFNOSUPPORT;
			break;
		}
		break;


	    case (SIOCSIFFLAGS):
		op_init();
		break;

	    default:
		error = EINVAL;
	}

	NETTRC1(HKWD_IFEN|hkwd_ioctl_out, error);

	return (error);
}

/*
 * NAME: op_output()
 *                                                                    
 * FUNCTION: Serial Optical interface output function
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * op_output determines the destination HW addr (process_id for optical) via
 *	masking the last octet of the dest ipaddr with the netmask.  It then
 *	adds the header to the front of the packet and calls op_send.  This
 *	is only done if the AF is inet.  op_output is usually called by 
 *	ip_output or some such beasty.
 *
 * (RECOVERY OPERATION:) returns an EAFNOSUPPORT if AF is ~inet.  If no mbufs
 *	are available and we need one, ENOBUFS is returned.
 *
 * (DATA STRUCTURES:) the mbuf given to us is freed after the xmit is done.
 *
 * RETURNS: 0 or errnos.
 */  
op_output(ifp, m, dst, rt)
register struct ifnet *ifp;
register struct mbuf *m;
struct sockaddr *dst;
struct rtoptry *rt; {
	register struct op_softc	*opp = &op_softc[ifp->if_unit];
	struct in_addr			idst;
	register int			off;
	int				error = 0;
	short				op_addr;
	struct	op_hdr			*op_hdrp;
	struct	ifaddr			*ifa;

	PERFTRC(HKWD_NETPERF|25);

	if ((ifp->if_flags & (IFF_UP|IFF_RUNNING)) != (IFF_UP|IFF_RUNNING)) {
		error = ENETDOWN;
		++opp->op_if.if_snd.ifq_drops;
		goto out;
	}


	/* Figure out the destination address */
	switch (dst->sa_family) {
	case AF_INET:

		idst = ((struct sockaddr_in *)dst)->sin_addr;

		/*
		 * if the dst addr is our own ip addr then call the
		 * loopback output routine.
		 */
		if (idst.s_addr == opp->op_ac.ac_ipaddr.s_addr) {
			error = looutput(ifp, m, dst, (struct rtentry *)NULL);
			return(error);
		}

		op_addr = idst.s_addr & opp->subnetmask & 0xff;

		if ((op_addr < 1) || (op_addr > 254)) {
			error = EADDRNOTAVAIL;
			goto out;
		}

		break;

	default:
		error = EAFNOSUPPORT;
		++opp->op_if.if_noproto;
		goto out;
	}

        /*
         * Add local net header.  If no space in first mbuf,
         * allocate another.
         */
	M_PREPEND(m, sizeof(struct op_hdr), M_DONTWAIT);
	if (m == 0) {
		error = ENOBUFS;
		++opp->op_if.if_snd.ifq_drops;
		goto out;
        }

        op_hdrp = mtod(m, struct op_hdr *);
        ie2_llc(&(op_hdrp->snap), 0x0800);
	op_hdrp->proc_id = op_addr;


	error = op_send(m, ifp, opp);
	++opp->op_if.if_opackets;
	m = 0;

    out:
	if (m)
		m_freem(m);

	PERFTRC(HKWD_NETPERF|125);

	NETTRC1(HKWD_IFEN|hkwd_output_out, error);
	return (error);
}

/*
 * NAME: op_send()
 *                                                                    
 * FUNCTION: Serial Optical interface send routine.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * This function gets called by op_output.  It determines the total packet
 * 	length by walking the mbuf chain.  If the length of each mbuf or 
 *	cluster is not on a word boundary -OR- the mbuf data pointer is not
 *	on a word boundary, then the collapse routines are called to fix it.
 *	THIS IS DONE ONLY TO SUPPORT THE NSC ROUTER!!!!!!  Once the mbuf
 *	chain is in the NSC approved fmt, net_xmit is called to send the
 *	packet.
 *
 * (RECOVERY OPERATION:) returns an error if one is received from 
 * 	net_xmit.  If we run out of mbufs, return ENOBUFS.
 *
 * (DATA STRUCTURES:) the mbuf chain may get messaged to make NSC happy.
 *
 * RETURNS: 0 or error from attach routines.
 */  
int 
op_send(m, ifp, opp)
register struct	mbuf		*m;	/* mbuf to send */
register struct ifnet 		*ifp;	/* IF pointer */
register struct op_softc	*opp;	/* ^ to softc for optical. */
{
	register struct mbuf	*n;	/* tmp pointer to loop on */
	int		error = 0, 
			done=0;

#ifdef NSC
	n = m;
	while (!done && n) {
		if (((u_long)(n->m_data) & 0x3) || 
			(n->m_next && n->m_len & 0x3) ||
			((n->m_flags&M_EXT) && (n->m_ext.ext_size != CLBYTES)))
			if ((n=collapse(m, m->m_pkthdr.len)) == 
				(struct mbuf *)NULL) {
				if (m)
					m_freem(m);
				++opp->op_if.if_snd.ifq_drops;
				return(ENOBUFS);
			} else {
				done = 1;
				m_freem(m);
				m = n;
			}
		n=n->m_next;
	}
#endif /* NSC */
	
#ifdef notdef

/* 
 * I'm using the sol_collapse routine until I can understand that Kati's
 * routines are more efficient and worthwhile...
 */
	/* NSC Kludge...*/
	if (mustcollapse) {
		if (collapse(m) == NULL) 
			return(ENOBUFS);
		mustcollapse = 0;
		n = m;
		while (n = n->m_next) {
			/* NSC kludge... */
			if ((u_long)(n->m_data) & 0x3) {
				mustcollapse = 1;
			}
			if (n->m_next && n->m_len & 0x3) {
				mustcollapse = 1;
			}
		}
		if (mustcollapse) {
			if ((m = collapse2(m)) == NULL)
				return(ENOBUFS);
		}
	}
#endif /* notdef */

	if (ifp->if_flags & IFF_DEBUG)
		net_xmit_trace(ifp, m);

	opp->op_if.if_obytes += m->m_pkthdr.len;
	if (!opp->netfp) {
		m_freem(m);
		++opp->op_if.if_oerrors;
		return(0);
	}
	error = (*(opp->fastwrite))(m, opp->chan);
	if (error) {
		m_freem(m);
		++opp->op_if.if_oerrors;
	}
	return(0);
}



/*
 * NAME: op_reset()
 *                                                                    
 * FUNCTION: Serial Optical interface reset function.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * This function gets called by ifconfig/cfgif/chgif.  It "resets" the 
 *	interface by calling op_init().
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) softc gets changed.
 *
 * RETURNS: nada.
 */  
op_reset()
{
	int	unit;

	NETTRC(HKWD_IFEN|hkwd_reset_in);

	for ( unit = 0; unit < ifsize; unit ++ ) {
		op_softc[unit].op_if.if_flags &= ~IFF_RUNNING;
	}
	op_init();

	NETTRC(HKWD_IFEN|hkwd_reset_out);

}

/***************************************************************************** 
 * op_recv	-	Process Optical receive completion
 *
 * Input:
 *	m	-	^ to mbuf containing packet
 *	unit	-	optical device driver unit number
 *
 * Description:
 *	If input error just drop packet.
 *	Otherwise purge input buffered data path and examine 
 *	packet to determine type.  If can't determine length
 *	from type, then have to drop packet.  Otherwise decapsulate
 *	packet based on type and pass to type-specific higher-level
 *	input routine.
 ****************************************************************************/ 
/*
 * NAME: op_recv()
 *                                                                    
 * FUNCTION: Serial Optical interface receive function.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * This function gets called by op_netintr.  
 *	If input error just drop packet.
 *	Otherwise purge input buffered data path and examine 
 *	packet to determine type.  If can't determine length
 *	from type, then have to drop packet.  Otherwise decapsulate
 *	packet based on type and pass to type-specific higher-level
 *	input routine.
 *     	and calls net_attach to attach to the device driver.  If this
 *      succeeds, it then calls op_attach to initialize the interface.
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 0
 */  
op_recv(m, unit)
	struct mbuf			*m;
	int				unit;
{
	register struct op_softc	*opp;
	register struct ifnet		*ifp;
	register struct ie2_llc_hdr	*llcp;
    	register struct mbuf		*m0;
	register int			len;

	PERFTRC(HKWD_NETPERF|26);
	NETTRC2(HKWD_IFEN|hkwd_rcv_in, m, &op_softc[unit].op_if);

	opp = &op_softc[unit];
	ifp = &op_softc[unit].op_if;
	llcp  = mtod(m, struct ie2_llc_hdr *);
	ifp->if_ipackets++;

	if ((ifp->if_flags & IFF_UP) == 0) {
		/*
		 * interface is down, as far as the network code is
		 * concerned.  Not much use receiving the packet.
		 */
		m_freem(m);
		++opp->op_if.if_iqdrops;
		goto out;
	}

	m->m_len -= sizeof (struct ie2_llc_hdr);
	m->m_data += sizeof(struct ie2_llc_hdr); 

	/* determine total length of packet	*/
	m0 = m;
	len = m0->m_len;
	while (m0 = m0->m_next)
		len += m0->m_len;


	if (m->m_flags & (M_PKTHDR|M_EXT) == 0) {
		if ((m0 = m_gethdr(M_DONTWAIT, MT_DATA)) == NULL) {
			m_freem(m);
			ifp->if_ierrors++;
			goto out;
		}
		else {
			m0->m_next = m;
			m = m0;
			m->m_len = 0;
		}
	}
	else {
		m->m_flags |= M_PKTHDR;
	}

	m->m_pkthdr.len = len;
	m->m_pkthdr.rcvif = ifp;

	if (find_input_type(llcp->type, m, &opp->op_ac, NULL) != 0) { 
		ifp->if_ierrors++;
		net_error(ifp, INV_INPUT_TYPE, opp->netfp);
	}

    out:
	PERFTRC(HKWD_NETPERF|126);
	NETTRC(HKWD_IFEN|hkwd_rcv_out);
	return(0);
}

#ifdef notdef
/* 
 * I'm using the sol_collapse routine until I can understand that Kati's
 * routines are more efficient and worthwhile...
 */
/*
 * NAME: collapse()
 *                                                                    
 * FUNCTION: Serial Optical interface collapse function.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * This function gets called by op_send if the mbuf chain must be NSCafied.
 * collapse - this specialised version of collapse makes sure that a chain
 * contains all clusters.  It leaves existing clusters as is, but attaches
 * a cluster to the first non-cluster mbuf (either in the entire chain or
 * after a cluster) and copies as much of itself and subsequent
 * non_cluster mbufs into the new cluster as will fit.
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) mbuf chain gets massaged
 *
 * RETURNS: 1=ok, NULL=no mbufs avail, the collapse failed.
 */  


int
collapse(m)
	struct mbuf *m;
{
	caddr_t fromptr, toptr;
	int 	fromlen;
	int rlen = 0;
	struct mbuf * m0,*lastm;

	lastm = m;
	m = m->m_next;				/* space past header mbuf */

	while (m) {
		if (!m->m_len) {
			lastm->m_next = m_free(m);
			m = lastm->m_next;
			continue;
		}

		/* just skip over any existing cluster */
		if (M_HASCL(m)) {
			rlen = 0;
			lastm = m;
			m = m->m_next;
		}
		/* copy into current cluster if space, else attach a 
		   cluster to current mbuf */
		else {
			fromptr = mtod(m,caddr_t);
			fromlen = m->m_len;
			if (rlen < fromlen) {			
				if ((m0 = m_getclust(M_WAIT,MT_DATA)) == NULL) {
					return(NULL);
				}
				lastm->m_next = m0;
				lastm = m0;
				rlen = CLBYTES;
				m0->m_len = 0;				

				/* wonder why this is needed */
				toptr = mtod(m0,caddr_t);
			}

			/* copy to cluster and advance pointers */
			bcopy(fromptr,toptr,fromlen);	
			toptr += fromlen;
			m0->m_len += fromlen;
			rlen -= fromlen;

			/* discard "from" mbuf */
			m0->m_next = m_free(m);		
			m = m0->m_next;
		}
	}
	return(1);
}


/*
 * NAME: collapse2()
 *                                                                    
 * FUNCTION: Serial Optical interface 2nd collapse function.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * This function gets called by op_send. 
 * collapse2 - this specialised version of collapse deals with clusters.
 * Either the data is still not aligned or the clusters are too numerous
 * and have to be packed.  It's too bad to have 2 collapse routines, but
 * better than packing when not absolutely essential.  Since we are just
 * shifting the contents of clusters to the left, we never have to get a
 * new one The above statement is false since it turns out the higher
 * levels remember these clusters and retransmit them on timeout.  So
 * better get new ones after all.  Should be able to use m_collapse, but
 * it won't fill up 'gather' cluster with part of a source cluster, so am
 * afraid it won't manage a complete compress.
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:) mbuf chain gets massaged.
 *
 * RETURNS: 1=ok, NULL=no buffs avail, collapse failed
 */  

int
collapse2(m)
	struct mbuf *m;
{
	caddr_t fromptr, toptr;
	int 	fromlen;
	struct mbuf * m0 = NULL, *oldm, *n;
	int	tolen;

	oldm = n = m;
	m = m->m_next;				/* space past header mbuf */

	/* m points to source cluster
	 * m0 points to target cluster
	 */

	while (1) {

		/* need new target */
		if (m0 == NULL) {				
			if ((m0 = m_getclust(M_WAIT,MT_DATA)) == NULL) {

				/* free target chain */
				m_freem(n);

				/* free source chain */
				m_freem(m);
				return(NULL); 
			}
			toptr = mtod(m0, caddr_t);
			m0 -> m_len = 0;
			oldm -> m_next = m0;
			oldm = m0;
		}
		if (CLBYTES - m0 -> m_len < m -> m_len) 
			fromlen = CLBYTES - m0 -> m_len;
		else fromlen = m -> m_len;
		fromptr = mtod(m,caddr_t);
		bcopy(fromptr, toptr, fromlen);
		
		m -> m_len -= fromlen;
		m0 -> m_len += fromlen;

		/* still data in source? */
		if (m -> m_len) {
			m -> m_data += fromlen;
			m0 = NULL;
			continue;
		}
		m = m_free(m);
		if (m == NULL) return(n);
		toptr += fromlen;
	}
}
#endif /* notdef */

#ifdef NSC
/*
 * NAME: collapse
 *
 * FUNCTION: Copies the data from an unacceptable mbuf chain into a normal
 *	chain of clusters.  The pointer of the new chain is returned.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from op_send when either the length or the
 * 	data field of an mbuf in the xmit chain is not word aligned.  This
 * 	is for the NSC router.
 *
 * NOTES:
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	struct mbuf *	- successful completion (new mbuf chain is returned)
 *	NULL		- could not get a cluster
 */
struct mbuf *
collapse(
struct mbuf	*mptr,		/* pointer to mbuf chain 		*/
uint		msglen)		/* total length of message		*/
{
	struct mbuf	*m, *new_mptr, *m_dest, *m_src;
	caddr_t		dest_ptr, src_ptr;
	uint		wait_flag, num_clus, i, dest_resid, src_resid;

	/*
	 *  First compute how many clusters we will need for the data.
	 */
	num_clus = (msglen + CLBYTES -1) / CLBYTES;


	/*
	 *  Get all the clusters we will need (and link them together).
	 */
	if ((new_mptr = m_getclust(M_DONTWAIT, MT_DATA)) == NULL) {
		return NULL;
	}
	m = new_mptr;
	for (i=1 ; i<num_clus ; i++) {
		if ((m->m_next = m_getclust(M_DONTWAIT, MT_DATA)) == NULL) {
			m_freem(new_mptr);
			return NULL;
		}
		m = m->m_next;
	}

	/*
	 *  At this point we have all the buffers we need, so copy the data.
	 */
	m_dest = new_mptr;
	m_src = mptr;
	dest_ptr = MTOD(m_dest, caddr_t);
	src_ptr = MTOD(m_src, caddr_t);
	dest_resid = CLBYTES;
	src_resid = m_src->m_len;
	m_dest->m_len = 0;
	while (TRUE) {
		if (dest_resid >= src_resid) {
			bcopy(src_ptr, dest_ptr, src_resid);
			dest_ptr = (caddr_t) ((uint)dest_ptr + src_resid);
			dest_resid -= src_resid;
			m_dest->m_len += src_resid;
			m_src = m_src->m_next;
			if (m_src == NULL) {
				break;
			} else {
				src_resid = m_src->m_len;
				src_ptr = MTOD(m_src, caddr_t);
			}
		} else {
			bcopy(src_ptr, dest_ptr, dest_resid);
			src_ptr = (caddr_t) ((uint)src_ptr + dest_resid);
			src_resid -= dest_resid;
			m_dest->m_len = CLBYTES;
			m_dest = m_dest->m_next;
			m_dest->m_len = 0;
			dest_resid = CLBYTES;
			dest_ptr = MTOD(m_dest, caddr_t);
		}
	}
	return new_mptr;
}
#endif /* NSC */
