static char sccsid[] = "@(#)18	1.14  src/bos/kernext/aixif/if_ca.c, sysxaixif, bos411, 9428A410j 4/20/94 16:38:16";
/*
 *   COMPONENT_NAME: SYSXAIXIF
 *
 *   FUNCTIONS: ca_attach
 *		ca_detach
 *		ca_init
 *		ca_ioctl
 *		ca_netintr
 *		ca_output
 *		ca_recv
 *		ca_reset
 *		ca_statintr
 *		ca_txintr
 *		config_ca
 *		config_ca_init
 *		config_ca_term
 *		net_start_ca
 *
 *   ORIGINS: 26,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

/*****************************************************************************
 * 
 * 	if_ca.c -  interface to aix 370 channel device handler
 *
 ****************************************************************************/

#define CAMTU   32768		/* 32 K limit */
#define IP_TYPE 0x800		/* IP type packet -- always for ca */

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
#include <sys/fp_io.h>          /* fp_open flags                */
#include <sys/syslog.h>		/* For bsdlog()			*/

#include <net/if.h>		/* ifnet			*/
#include <net/if_types.h>	/* all the interface types	*/

#include <sys/devinfo.h>	/* after if.h so IFF_UP not redef'd */

#include <netinet/in.h>		/* for sockaddr_in ...		*/
#include <netinet/in_netarp.h>  /* arp stuff			*/
#include <netinet/in_var.h>	/* IA_SIN macro			*/
#include <netinet/in_systm.h>	/* needed by ip.h		*/
#include <netinet/ip.h>		/* ip_len ???			*/
#include <sys/time.h>		/* for curtime()...	        */

#include <sys/comio.h>		/* aix comio device driver stuff*/
#include <aixif/net_if.h>	/* network interface to comio 	*/
#include <sys/catuser.h>	/* channel attach related	*/

/*****************************************************************************
 * Channel Attach software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * ca_if, which the routing code uses to locate the interface.
 ****************************************************************************/
struct	ca_softc {
	struct		arpcom ca_ac;	/* Channel Attach common part	*/
	u_short		ca_flags;	/* private flag			*/
	struct file	*netfp;		/* device handler file pointer	*/
	u_short		subchan;	/* subchannel address */
	u_short		link_id;
	u_short		started;  /* flag to indicate if started already  */
};
struct ca_softc *ca_softc = (struct ca_softc *)NULL;

#define	ca_if		ca_ac.ac_if	/* network-visible interface */

int			if_ca_count = 0;/* number of units attached	*/
struct netid_list	ca_netids;	/* channel attach netids to start	*/
int			if_ca_lock = LOCK_AVAIL; /* attach locking	*/
int camtu;	/* mtu passed in device request struct */
int init = 0;
int linkid=0;	/* claw link id */

void		ca_netintr();		/* receive interrupt handler	*/
void		ca_statintr();		/* status interrupt handler	*/
void		ca_txintr();		/* transmit complete handler	*/

/*************************************************************************
 *
 *	config_ca() - channel attach kernel extension entry point
 *
 ************************************************************************/
config_ca(cmd, uio)
int		cmd;
struct uio	*uio;
{
	struct	device_req		device;
	int				error = 0;
	int				unit;
	int				lockt;
	char				*cp;

	lockt = lockl(&if_ca_lock, LOCK_SHORT);

	if (if_ca_count == 0)
		if (error = pincode(config_ca))	
			goto out;
	if (init == 0) {
		if (ifsize <= 0)
			ifsize = IF_SIZE;
		ca_softc = (struct ca_softc *)
		    xmalloc(sizeof(struct ca_softc)*ifsize, 2, pinned_heap);
		if (ca_softc == (struct ca_softc *)NULL) {
		    unpincode(config_ca);
		    unlockl(&if_ca_lock);
		    return(ENOMEM);
		}
		bzero(ca_softc, sizeof(struct ca_softc) * ifsize); 
		init++;
	}

	if (cmd != CFG_INIT) {
		error = EINVAL;
		goto out;
	}

	if (uio) {	
		error = uiomove((caddr_t) &device, (int) sizeof(device), 
                                 UIO_WRITE, uio);
		if (error) 
			goto out;
	
		camtu = CAMTU;
	}

	cp = device.dev_name;
	while(*cp < '0' || *cp > '9') cp++;
	unit = atoi(cp);

	if (unit >= ifsize) {
		error = ENXIO;
		goto out;
	}
	
	error = config_ca_init(&device, unit);
out:
	if (lockt != LOCK_NEST)
		unlockl(&if_ca_lock);
	return(error);
}


/*************************************************************************
 *
 *	config_ca_init() - channel attach load initialisation
 *
 ************************************************************************/
config_ca_init(device, unit)
register struct	device_req	*device;
int				unit;
{
	struct	kopen_ext		kopen_ext;
	int				rc = 0;
	char				driver_path[100];

	if (ca_softc[unit].netfp != NULL)
		return(EALREADY);

	ca_netids.id_count	= 0; 	/* don't do any net_start until the
					   subchannel address is passed in */

	kopen_ext.status = 0;
	kopen_ext.rx_fn = ca_netintr;
	kopen_ext.tx_fn = ca_txintr;
	kopen_ext.stat_fn = ca_statintr;
	kopen_ext.open_id = unit;

        sprintf(driver_path, "/dev/cat%d", unit);
        rc = fp_open(driver_path, O_RDWR|O_NDELAY, NULL, &kopen_ext,
                FP_SYS, &ca_softc[unit].netfp);

	if (!rc) { 
		if_ca_count++;
		rc = ca_attach(unit);
		if (!rc) {
			rc = fp_getdevno(ca_softc[unit].netfp,
				&ca_softc[unit].ca_if.devno,
				&ca_softc[unit].ca_if.chan);
		}
	}

	/* initialize the linkid to zero  */
	ca_softc[unit].link_id = 0;

	return(rc);
}


/*************************************************************************
 *
 *	ca_statintr() - channel attach network status interrupt handler
 *
 ************************************************************************/
void
ca_statintr(unit, sbp)
register u_long			unit;
register struct status_block 	*sbp;
{
	register struct ca_softc 	*cap;
 	register struct ifnet *ifp;


	NETTRC2(HKWD_IFCA|hkwd_statintr_in, &ca_softc[unit].ca_if, sbp->option[0]);

	cap = &ca_softc[unit];
	ifp = &ca_softc[unit].ca_if;

	if (sbp->code == CIO_START_DONE) {
		if (sbp->option[0] != CIO_OK)
			net_error(&ca_softc[unit].ca_if, sbp->option[0], 
                                  ca_softc[unit].netfp);

		else{
			 cap->link_id = sbp->option[2];	/* link id */
			/* Connection has been established, set the running
			 * flag to indicate that
			 */
			ifp->if_flags |= IFF_RUNNING;  
		}
	}

	/* 
	 * Handle the link disconnect
	 * can be disconnected by a HALT, or a link disconnect
	 * link disconnect is indicated by an asynchronous status
      * with the option set to 3A
      */
     if (sbp->code == CIO_HALT_DONE){
          cap->link_id = 0;
          ifp->if_flags &= ~IFF_RUNNING;
     }
     if ((sbp->code == CIO_ASYNC_STATUS) && (sbp->option[0] == 0x3A)){
     /*
      * We recieved a link disconnect notification.
      */
          cap->link_id = 0;
          ifp->if_flags &= ~IFF_RUNNING;

     }
	else
		net_error(&ca_softc[unit].ca_if, sbp->option[0], 
                          ca_softc[unit].netfp);

	NETTRC(HKWD_IFCA|hkwd_statintr_out);
}


/*************************************************************************
 *
 *	ca_netintr() - channel attach network interrupt handler
 *
 ************************************************************************/
void
ca_netintr(unit, status, m)
register u_int			unit;
register u_int			*status;
register struct	mbuf		*m;
{
	NETTRC2(HKWD_IFCA|hkwd_netintr_in, &ca_softc[unit].ca_if, *status);

	if (*status == CIO_OK)
		ca_recv(m, unit);
	else {
		net_error(&ca_softc[unit].ca_if, *status,ca_softc[unit].netfp); 
		if (m)
			m_freem(m);
	}

	NETTRC(HKWD_IFCA|hkwd_netintr_out);
}

/*************************************************************************
 *
 *	ca_txintr() - channel attach transmit complete interrupt handler
 *
 ************************************************************************/
void
ca_txintr(unit)
u_long unit;
{
	net_error(&ca_softc[unit].ca_if, INV_TX_INTR, ca_softc[unit].netfp); 
}


 /****************************************************************************
 * ca_attach -	logically attach the channel attach network interface
 ****************************************************************************/
ca_attach(unit)
unsigned 		unit;
{
	register struct ifnet 		*ifp;
	register struct ca_softc        *cap;
	struct	devinfo			devinfo;
	struct	cio_get_fastwrt		cio_get_fastwrt;
	extern int			ca_init();
	extern int			ca_output();
	extern int			ca_ioctl();
	extern int			ca_reset();
	int				rc;

	NETTRC1(HKWD_IFCA|hkwd_attach_in,  unit);

	if (unit >= ifsize) {
		NETTRC(HKWD_IFCA|hkwd_attach_out);
		return ENXIO;
	}

        cap             = &ca_softc[unit];
	ifp             = &ca_softc[unit].ca_if;

	if (fp_ioctl(cap->netfp, IOCINFO, &devinfo, NULL))
		ifp->if_baudrate = 0;
	else 
	{
		switch (devinfo.un.pca.chnl_speed)
		{
			case CAT_SPD_2_7:
				ifp->if_baudrate = 2700000;
				break;

			case CAT_SPD_4_5:
				ifp->if_baudrate = 4500000;
				break;

			default:
				ifp->if_baudrate = 0;
		}
	}

	ifp->if_mtu	= camtu;
	ifp->if_flags 	= IFF_POINTOPOINT;
	ifp->if_unit	= unit;
	ifp->if_name    = "ca";
	ifp->if_init	= ca_init;
	ifp->if_output	= ca_output;
	ifp->if_ioctl	= ca_ioctl;
	ifp->if_reset	= ca_reset;
        ifp->if_type    = IFT_OTHER;

        ifp->if_hdrlen = 14;
        ifp->if_addrlen = 6;

	if_attach(ifp);
	if_nostat(ifp);

	NETTRC(HKWD_IFCA|hkwd_attach_out);
	return(0);
}

/***************************************************************************** 
 * ca_detach -	logically detach channel attach network interface
 ****************************************************************************/ 
ca_detach(ifp)
struct ifnet	*ifp;
{
	struct cat_set_sub	claw_sess;
	register struct ca_softc        *cap = &ca_softc[ifp->if_unit];
	int	rc;

	NETTRC1(HKWD_IFCA|hkwd_detach_in,  ifp);
      
	/* ifp->if_flags  &= ~(IFF_UP|IFF_RUNNING); */
	/* only turn off the up flag.  Running is changed based off
   	 * status from the device driver 
	 */
	ifp->if_flags  &= ~(IFF_UP);

	/*
	 *	Now issue CIO_HALT for that subchan.
	 */
	bzero(&claw_sess,sizeof(struct cat_set_sub));

	claw_sess.specmode= CAT_CLAW_MOD;
	strcpy(claw_sess.claw_blk.WS_appl,"TCPIP   ");
	strcpy(claw_sess.claw_blk.H_appl,"TCPIP   ");
	strcpy(claw_sess.claw_blk.WS_adap,"PSCA    ");
	strcpy(claw_sess.claw_blk.H_name,"HOST    ");

	claw_sess.sb.netid= cap->subchan;
	claw_sess.claw_blk.linkid= cap->link_id;

	rc=fp_ioctl(cap->netfp,CIO_HALT,&claw_sess,NULL);
	if (rc)
		bsdlog(LOG_ERR, "if_ca: CIO_HALT failed with errno=%d\n",
			rc);

	cap->started = 0;
	rc = net_detach(cap->netfp);
	if (rc)
		bsdlog(LOG_ERR,"if_ca: net_detach failed with errno=%d\n",
			rc);
	if_ca_count--;
	cap->netfp = NULL;

	NETTRC(HKWD_IFCA|hkwd_detach_out);
	return (rc);
}
	
/***************************************************************************** 
 * ca_init -	logically initialize channel attach network interface
 ****************************************************************************/ 
ca_init()
{
	register struct ca_softc *	cap;
	register struct ifnet *		ifp;
	int				s;
	int				unit;

	NETTRC(HKWD_IFCA|hkwd_init_in);

	for ( unit = 0; unit < ifsize; unit ++ ) {
		cap = &ca_softc[unit];
		ifp = &cap->ca_if;

		/* not yet, if address still unknown */
		if (ifp->if_addrlist == (struct ifaddr *)0)
			continue;
		/* Running flag is now set by the completion of
		 * the CIO_START
		 * cap->ca_if.if_flags |= IFF_RUNNING;
		 */
	}

	NETTRC(HKWD_IFCA|hkwd_init_out);
}

/***************************************************************************** 
 * Process an ioctl request.
 *
 * NOTE:
 *	the error code returned from this routine is ignored by the caller.
 ****************************************************************************/ 
ca_ioctl(ifp, cmd, data)
	register struct ifnet *		ifp;
	int				cmd;
	caddr_t				data;
{
	register struct ifaddr *	ifa = (struct ifaddr *)data;
	register struct ifreq *		ifr = (struct ifreq *)data;
	register struct ca_softc *	cap = &ca_softc[ifp->if_unit];
	int				subchan;
	int				error = 0;
	struct timestruc_t		ct;

	NETTRC3(HKWD_IFCA|hkwd_ioctl_in, ifp, cmd, data);

	if (!cap->netfp)
		return(ENODEV);

	switch (cmd) {
	    case SIOCIFATTACH:
		bsdlog(0, "if_ca: SIOCIFATTACH called\n");
		break;

	    case SIOCIFDETACH:
		bsdlog(0, "if_ca: SIOCIFDETACH called\n");
		ca_detach(ifp);
		break;
		
	    case SIOCSIFADDR:
                switch (ifa->ifa_addr->sa_family) {
		    case AF_INET:
			((struct arpcom *)ifp)->ac_ipaddr =
				IA_SIN(ifa)->sin_addr;
			break;
		}
		ca_init();
		ifp->if_flags |= IFF_UP;
		curtime (&ct);
		ifp->if_lastchange.tv_sec = (int) ct.tv_sec;
		ifp->if_lastchange.tv_usec = (int) ct.tv_nsec / 1000;
		break;

	    case SIOCSIFFLAGS:
		ca_init();
		break;

	    case SIOCSIFSUBCHAN:
		/* Only do the start once   for each interface  */
		if ( !(cap->started) ) {
			subchan = ifr->ifr_flags;
			error = net_start_ca(cap->netfp, subchan);
			if (!error) {
			       	cap->link_id=linkid;
			       	cap->subchan=subchan;
				cap->started = 1;   /* flag that it has been*/
						    /* started to prevent   */
						    /* a second start       */
			}
		}
		break;
	
	    default:
		error = EINVAL;
	}


	NETTRC1(HKWD_IFCA|hkwd_ioctl_out, error);

	return (error);
}

/***************************************************************************** 
 * Channel Attach output routine.
 ****************************************************************************/ 
ca_output(ifp, m, dst, rt)
register struct ifnet *ifp;
register struct mbuf *m;
struct sockaddr *dst;
struct rtentry *rt; {
	register struct ca_softc	*cap = &ca_softc[ifp->if_unit];
	int				error = 0;
	cat_write_ext_t			*wrtext;
	struct	mbuf			*m_wrtext;

	PERFTRC(HKWD_NETPERF|25);

	if ((ifp->if_flags & (IFF_UP|IFF_RUNNING)) != (IFF_UP|IFF_RUNNING)) {
		error = ENETDOWN;
		++cap->ca_if.if_snd.ifq_drops;
		goto out;
	}

	/* CLAW connection */
	if (cap->link_id == 0 )  {
		error = ENETDOWN;
		++cap->ca_if.if_snd.ifq_drops;
		goto out;
	}

	cap->ca_if.if_obytes += m->m_pkthdr.len;

	m_wrtext = m_get(M_DONTWAIT, MT_DATA);
	if (m_wrtext == (struct mbuf *)NULL) {
		error = ENOBUFS;
		goto out;
	}
	wrtext = mtod(m_wrtext, cat_write_ext_t *);
	wrtext->cio_ext.flag= 0;
	wrtext->cio_ext.write_id= cap->link_id;
	wrtext->cio_ext.netid= cap->subchan;
	wrtext->attn_int= 0;

	if (!cap->netfp) {
		++cap->ca_if.if_oerrors;
		goto out;
	}
	else error = net_xmit(ifp, m, cap->netfp, m->m_pkthdr.len, m_wrtext);

	if (error)
		++cap->ca_if.if_oerrors;

	++cap->ca_if.if_opackets;
	m = 0;

    out:
	if (m)
		m_freem(m);

	PERFTRC(HKWD_NETPERF|125);

	NETTRC1(HKWD_IFCA|hkwd_output_out, error);
	return (error);
}

/***************************************************************************** 
 * ca_reset() - reset channel attach network interface
 ****************************************************************************/ 
ca_reset()
{
	int	unit;

	NETTRC(HKWD_IFCA|hkwd_reset_in);

	for ( unit = 0; unit < ifsize; unit ++ ) {
		ca_softc[unit].ca_if.if_flags &= ~IFF_RUNNING;
	}
	ca_init();

	NETTRC(HKWD_IFCA|hkwd_reset_out);

}

/***************************************************************************** 
 * ca_recv	-	Process Channel Attach receive completion
 *
 * Input:
 *	m	-	^ to mbuf containing packet
 *	unit	-	channel attach device driver unit number
 *
 * Description:
 *	If input error just drop packet.
 *	Otherwise purge input buffered data path.
 ****************************************************************************/ 
ca_recv(m, unit)
	struct mbuf			*m;
	int				unit;
{
	register struct ca_softc	*cap;
    	register struct mbuf		*m0;
	int				len;

	PERFTRC(HKWD_NETPERF|26);
	NETTRC2(HKWD_IFCA|hkwd_rcv_in, m, &ca_softc[unit].ca_if);

	cap = &ca_softc[unit];
	++cap->ca_if.if_ipackets;

	if ((cap->ca_if.if_flags & IFF_UP) == 0) {
		/*
		 * interface is down, as far as the network code is
		 * concerned.  not much use receiving the packet.
		 */
		m_freem(m);
		++cap->ca_if.if_iqdrops;
		goto out;
	}

        /* determine total length of packet     */
        m0 = m;
        len = m0->m_len;
        while (m0 = m0->m_next)
                len += m0->m_len;

	if ((m0 = m_gethdr(M_DONTWAIT, MT_DATA)) == NULL) {
		m_freem(m);
		cap->ca_if.if_ierrors++;
		return(1);
	}
	else {
		m0->m_next = m;
		m = m0;
		m->m_pkthdr.rcvif = &cap->ca_if;
		m->m_pkthdr.len = len;
		m->m_len = 0;
	}

	if (find_input_type(IP_TYPE, m, &cap->ca_ac, 0, NULL ) != 0) { 
		++cap->ca_if.if_ierrors;
		net_error(&cap->ca_if, INV_INPUT_TYPE, cap->netfp);
	}
	goto out;

   drop:
        ++cap->ca_if.if_iqdrops;
	if (m)
		m_freem(m);

    out:
	PERFTRC(HKWD_NETPERF|126);
	NETTRC(HKWD_IFCA|hkwd_rcv_out);
	return(0);
}

/*
 * NAME:  net_start_ca
 *
 * FUNCTION:  starts netids on an AIX comio style device handler.
 *
 * RETURNS:
 *	0          - ok
 *	EADDRNOTAVAIL  - address not available
 * This routine is specially for channel attach because a claw block is required
 */
net_start_ca(netfp, subchan)
	struct file *netfp;
	int	subchan;
{
	int			rc = 0;
	struct cat_set_sub	claw_sess;

	/*
	 *	Now issue CIO_START for that subchan.
	 */
	bzero(&claw_sess,sizeof(struct cat_set_sub));

	/* claw_sess.specmode= CAT_CLAW_MOD | CAT_FLUSHX_MOD;*/
	claw_sess.specmode= CAT_CLAW_MOD;
	claw_sess.set_default = 1;
	claw_sess.shrtbusy = 3;
	claw_sess.startde = 1;
	strcpy(claw_sess.claw_blk.WS_appl,"TCPIP   ");
	strcpy(claw_sess.claw_blk.H_appl,"TCPIP   ");
	strcpy(claw_sess.claw_blk.WS_adap,"PSCA    ");
	strcpy(claw_sess.claw_blk.H_name,"HOST    ");
	claw_sess.sb.netid= subchan;

	if (rc= fp_ioctl(netfp,CIO_START,&claw_sess,NULL))
		return (rc);

	return(0);
}
