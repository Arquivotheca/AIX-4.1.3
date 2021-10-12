/* @(#)90       1.3  src/bos/kernel/io/dump_pg.h, sysdump, bos411, 9428A410j 3/3/94 17:42:47  */  

/*
 * COMPONENT_NAME: sysdump
 *
 * FUNCTIONS:  header file for dumpdd_pg.c
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/intr.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <sys/syspest.h>
#include <sys/priv.h>
#include <sys/trchkid.h>
#include <sys/errids.h>
#include <sys/xmem.h>
#include <sys/fp_io.h>
#include <sys/sleep.h>
#include <sys/mstsave.h>
#include <sys/low.h>
#include <sys/vmker.h>
#include <sys/nvdd.h>
#include <sys/mbuf.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <string.h>
#include <sys/comio.h>
#include <netdb.h>
#include <net/if.h>
#include <net/netisr.h>
#include <net/af.h>
/* define GATEWAY needed for get_arp_entry to work correctly        */
#define  GATEWAY
#include <net/route.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/in_netarp.h>

#include <memory.h>


#define MDEV_DUMP    0
#define MDEV_DUMPCTL 1
#define MDEV_NULL    2
#define MDEV_FILE    3
#define HOST_LEN         256


#define CALL 0
#define REPLY  1
#define NFS_PROGRAM ((u_long) 100003)
#define NFS_VERSION ((u_long) 2)
#define RFS_WRITE   8
#define AUTH_UNIX   1
#define AUTH_NULL   0
#define RPC_MSG_VERSION ((u_long) 2)
#define UDP_TTL     30
#define IP_TYPE     0x0800
#define ARP_TYPE    0x0806
#define REV_ARP_TYPE    0x8035
#define NS_TYPE     0x0600
#define FILEHNDL_LEN     32
#define HOST_LEN         256
#define TOTALWTIME       60000
#define MAXTIMEOUT       60000
#define MILSEC           1000
#define MAX_RETRIES      10

/* The end of remote dump includes */
#define DUMPINIT2 16            /* pre-init dump_op for secondary dump device */

#define SERIALIZE(lp)    lockl(lp,LOCK_SIGWAKE)
#define UNSERIALIZE(lp)  unlockl(lp)
#define DMP_BUF_MAX (4 * 1024)          /* max size of single d_dump transfer */
#define VCPY(from,to_array) strncpy(to_array,from,sizeof(to_array))
#define jfree(ptr) xmfree(ptr,pinned_heap)
#define jmalloc(n) xmalloc(n,2,pinned_heap)

#include <sys/devinfo.h>
#include <sys/dump.h>

#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/mdio.h>

#define Debug (void)sizeof
#define Jprintf         (void)sizeof
extern dmpopen();
extern dmpdump();
extern dmpinit();
extern dmpclose();
extern idmp_do();
extern isdmpfile();



extern dev_t null_mmdev;
extern int Mdev;
extern struct dumpinfo nv_dumpinfo;
extern int ddd_namelen;    /* length of the remote host name */

extern struct mdt_entry {               /* master dump table is an array */
        struct cdt *((*mdt_func)());    /* of function pointers that return a */
};                                      /* pointer to a cdt structure */

extern struct dmp {
        lock_t            dmp_lockword;        /* serialization */
        lock_t            dmp_lockword1;       /* serialization, DMPD_PRIM */
        lock_t            dmp_lockword2;       /* serialization, DMPD_SEC */
        int               dmp_haltondump;      /* dmp_halt after dmp_do ioctl */
        int               dmp_count;           /* current max cap of mdt list */
        struct mdt_entry *dmp_mdt;             /* start of mdt list */
        int               dmp_priopenflg;      /* primary dump device is held open */
        struct file      *dmp_oprimfp;         /* old primary dump device file pointer */
        struct file      *dmp_primfp;
        struct file      *dmp_secfp;
} dmp;

extern void   ddd_rx_fn();
extern void   ddd_tx_fn();
extern void   ddd_stat_fn();
extern void   ddd_freem();


extern struct dmpio {
	dev_t             dmp_dev;		/* major/minor dump device */
        struct in_addr    dmp_remoteIP;         /* remote IP address       */
        struct in_addr    dmp_localIP;          /* local IP address        */
        int               dmp_netflg;           /* remote dump flag       */
	int               dmp_blocksize;	/* blocksize of dump device */
	char             *dmp_buf;		/* buffer for dump_op() */
	struct mbuf      *dmp_bufwrite;		/* write buffer for remote dump*/
	struct mbuf      *dmp_bufread;		/* read buffer for remote dump */
        struct ifnet     *dmp_ifp;              /* the address of interface if struct */
        struct sockaddr_in      dmp_dest;       /* the address of destination */
       	int               dmp_bufsize;		/* buffer size for dump_op() */
	int               dmp_idx;		/* index into dmp_buf */
	int               dmp_idxn;		/* length of the device dependence headers*/
	int               dmp_offset;		/* current offset into dump device */
	struct uio        dmp_uio;		/* uio for dump_op() */
	struct iovec      v;			/* iovec for dump_op() */
	struct dumpinfo   dmp_dumpinfo;		/* dumpinfo structure */
	
}dmpio[2]; 


extern struct NFS_hdr {
	long              xid;			/* Transmit sequence number */
	long              direction;		/* request direction */
	long              rpc_version;		/* rpc_version */
	long              program;		/* NFS_PROGRAM */
	long              version;		/* NFS_VERSION */
	long              proc_num;		/* NFS_WRITE */
	long              verf;		        /* AUTH_UNIX */
	long              cred_size;		/*  */
	long              time;		        /* time of the request */
	long              hostnamelen;		/* length */
	char              *hostname;	        /* host name */
	long              uid;		        /* 0 */
	long              gid;		        /* 0 */
	long              gidlen;	        /* 0 */
        long              glist;                /* 0 */
	long              verf_flavor;	        /* AUTH_NULL */
	long              verf_len;	        /* 0 */
        /* NFS write arg */
	char              *filehandle;	        /* 32byte file handle */
	long              begoff;	        /* not use */
	long              offset;	        /* current offset in the file */
	long              totcnt;	        /* not use */
	long              cnt;	                /* size of write data */
	
};

extern struct NFS_hdr NFS_hdr[2];

extern int get_arp_entry();
extern dump_op();
extern dmpnow();
extern struct dmpio *typeto_dmpio();
extern struct NFS_hdr *typeto_NFS_hdr();
extern set_NFS_hdr();

extern void dump_func();
extern dmpfile();
extern dmpnull();
typedef struct cdt *((*CDTFUNC)());
extern struct mdt_entry *getmdt();
extern build_IP_header();
extern caddr_t pinned_heap;
extern int ras_privflg;    /* non-zero: check privileges. global=symtab */
struct arptab *ddd_arptab;
