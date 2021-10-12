static char sccsid[] = "@(#)93        1.56.1.25  src/bos/kernel/io/dumpdd.c, sysdump, bos41J, 9516B_all 4/18/95 13:43:32";
/*
 * COMPONENT_NAME: SYSDUMP    /dev/sysdump pseudo-device driver
 *
 * FUNCTIONS: dmp_do, dmp_add, dmp_del
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

extern void dump_func();

/*
 * SYSDUMP:
 *   dmpinit         device driver init
 *   dmp_add         system service
 *   dmp_del         system service
 *   dmp_prinit      system service
 *   dmp_do          system service
 *
 * dump device driver and dump system services
 * Everything in this file is pinned.
 * None of these routines run on a fixed stack.
 * However, for dmp_do(), which is called by panic and ctl-alt-1,
 *   ctl-alt-2,the stack must be pre-pinned by the caller,
 *   since page faulting at dump time should not occur.
 *
 * There are 3 parts to this file.
 * A. dump device driver, /dev/sysdump.
 *    It contains:
 *    1. ioctl routine that the commands sysdumpstart and sysdumpdev talks to.
 *    2. a default devsw.d_dump routine to testing dmp_do().
 * B. dump table management routines, dmp_add() and dmp_del()
 *    These are called by the rest of the system to add their data areas to the
 *      dump.
 * C. dmp_do()
 *    This routine is called by the sysdumpstart command though ioctl, or
 *      directly by panic and ctrl-alt-1.
 *    It main function is to call the d_dump routine of the configured device
 *      driver.
 *    It goes through the steps:
 *      INIT/QUERY, START, WRITE, END, TERM
 *    Most of the code of dmp_do() is byte-to-device_block buffering.
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
#include <sys/var.h>
#include <sys/nvdd.h>
#include <fcntl.h>
#include <sys/systemcfg.h>
#include <sys/ndd.h>
#include <sys/cdli.h>

/* These includes are added for POWER_PC */
#include <sys/systemcfg.h>
#include <sys/sys_resource.h>

#ifdef _POWER_MP
#include <sys/ppda.h>
#include <sys/processor.h>
#include <sys/lock_def.h>
#include <sys/mpc.h>
#include <sys/atomic_op.h>
#endif /* _POWER_MP */

/* The following includes are added for remote dump implementation */


#include <sys/socket.h>
#include <net/net_globals.h>
#include <sys/mbuf.h>
#include <net/netisr.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#define REPLY  1
#define NFS_PROGRAM ((u_long) 100003)
#define NFS_VERSION ((u_long) 2)
#define RFS_WRITE   8
#define AUTH_UNIX   1
#define AUTH_NULL   0
#define RPC_MSG_VERSION ((u_long) 2)
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

#include <sys/devinfo.h>
#include <sys/dump.h>

#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/mdio.h>
#include <unistd.h>

#define Debug (void)sizeof

#ifndef DMPNOW_AUTO
#define DMPNOW_AUTO 999
#endif

#define DUMPINIT2 16                /* pre-init dump_op for secondary dump device */

#define SERIALIZE(lp)    lockl(lp,LOCK_SHORT)
#define UNSERIALIZE(lp)  unlockl(lp)

#define MDEV_DUMP    0
#define MDEV_DUMPCTL 1
#define MDEV_NULL    2
#define MDEV_FILE    3
#define jfree(ptr) xmfree(ptr,pinned_heap)
#define jmalloc(n) xmalloc(n,2,pinned_heap)

int (* remote_dump)(struct ndd *, int, caddr_t);
extern struct ndd *prim_nddp;
extern struct ndd *sec_nddp;

void   ddd_rx_fn();
void   ddd_tx_fn();
void   ddd_stat_fn();
extern void   ddd_freem();

extern nodev();
extern int errno;
extern int ddd_IP_input();
extern int ddd_other_input();
extern struct cdt_head *errcdtf();
extern struct cdt_head *trccdtf();

/* trace is a subsystem */
extern int (*_trc_trcofffp)();
extern int (*_trc_trconfp)();

extern idmp_do();
static wr_cdt();
static dmp_ccstatus();
static dmp_complete();
extern isdmpfile();
static jwrite();
static jflush();
static jwrite_io();
extern dmpfile();
static dmp_led();
static dmp_scroll();
extern build_IP_header();
static in_cksum();
static process_response();
static process_packet();
extern int dmpnow();

extern caddr_t xmalloc();
extern caddr_t pinned_heap;
extern time_t time;
extern pid_t sec_dump_pid;

#ifdef _POWER_MP
mpc_msg_t	dmp_mpc_msg_intiodone;
mpc_msg_t	dmp_mpc_msg_intmax;
int    		num_of_cpus;        /* number of running cpus */
int		intmax_mpc = 0;     /* did we have to send the mpc twice? */
volatile char   proc_state[MAXCPU]; /* state of processors */
void            dmpmpc();           /* function to be called by mpc_send() */
static          proc_not_stopped(); /* returns true if all processors not stopped */
int             proc_started_dump;  /* the processor that initiated the dump */
atomic_p        test = 0;           /* used to test if dmp_do() gets called more than once */
#endif /* _POWER_MP */

volatile int    dump_started = 0;   /* entering idmp_do() to begin the dump */

/* The following declarations are added for remote dump implementation */

int ddd_namelen;    /* length of the remote host name */
static int ddd_led;
static int ddd_timeout = 50;    /* time out for read   */
static int ddd_waittime = 70;    /* time out for read   */
static int ddd_nofrag;
static int ddd_moredata;
static int ddd_fragoff;
static int ddd_finish;
static int ddd_mbuf_free = 0;
struct arptab *ddd_arptab;
/* 
   999 is the source port number. Assumption is made that at dump time, sysdump is        
   the only process trying to send thing out to the network, hence any port number 
   will do.  I arbitrarily pick 999.  
   2049 is the destination port number.  This is a well known NFS service port   
*/
struct udphdr udp = {999,2049,0,0};             /* udp header   */

/*  end of remote dump declarations  */

int ras_privflg = 1;          /* non-zero: check privileges. global=symtab */

int Mdev;                /* major device number set on first open */
dev_t null_mmdev;        /* major and minor devno of sysdumpnull  */

struct dumpinfo nv_dumpinfo;        /* dumpinfo from previous dump */

#define VCPY(from,to_array) strncpy(to_array,from,sizeof(to_array))
#define CALTIMENOW(now,cur) \
   curtime(&cur); now = ((cur.tv_sec * MILSEC) + cur.tv_nsec / 1000000)
#define CALTIMELEFT(now,end,left) \
   left = (( now < end ) ? end - now : 0)
#define CALTIMEOUT(t)  t = (((t << 2) > MAXTIMEOUT ) ? MAXTIMEOUT : t << 2) 

#define DMP_BUF_MAX (4 * 1024)       /* max size of single d_dump transfer */

#define DMP_GRW_SZ  100              /* grow mdt table 100 entries at a time */


typedef struct cdt *((*CDTFUNC)());

/*
 * led codes
 * 'A' to an led digit looks like a lower-case c
 */
#define LED_DUMPSUCCESS     0x0A0
#define LED_DUMPINPROGRESS  0x0A2
#define LED_DUMPFAILED      0x0A4
#define LED_FAILEDSTART     0x0A5
#define LED_SECREADY        0x0A6
#define LED_WAIT4RESP       0x0A7       /* Network dump  */
#define LED_DISABLED        0x0A8
#define LED_AUTOINPROGRESS  0x0A9

/* change to extern later */
extern struct mdt_entry *getmdt();        /* allocate 1 mdt entry */
extern freemdt();                         /* free mdt entry */
extern dmpdump();                         /* internal dd_dump routine */

struct mdt_entry {                        /* master dump table is an array */
        struct cdt *((*mdt_func)());      /* of function pointers that return a */
};                                                                        /* pointer to a cdt structure */

/*static*/ struct dmp {
        lock_t            dmp_lockword;         /* serialization */
        lock_t            dmp_lockword1;        /* serialization, DMPD_PRIM */
        lock_t            dmp_lockword2;        /* serialization, DMPD_SEC */
        int               dmp_haltondump;       /* dmp_halt after dmp_do ioctl */
        int               dmp_count;            /* current max cap of mdt list */
        struct mdt_entry *dmp_mdt;              /* start of mdt list */
        int               dmp_priopenflg;       /* primary dump device is held open */
        struct file      *dmp_oprimfp;          /* old primary dump device file pointer */
        struct file      *dmp_primfp;
        struct file      *dmp_secfp;
} dmp = { LOCK_AVAIL, LOCK_AVAIL, LOCK_AVAIL, 0, 0 };






struct dmpio {
        dev_t             dmp_dev;              /* major/minor dump device */
        struct in_addr    dmp_remoteIP;         /* remote IP address       */
        struct in_addr    dmp_localIP;          /* local IP address        */
        int               dmp_netflg;           /* remote dump flag       */
        int               dmp_blocksize;        /* blocksize of dump device */
        char             *dmp_buf;              /* buffer for dump_op() */
        struct mbuf      *dmp_bufwrite;         /* write buffer for remote dump*/
        struct mbuf      *dmp_bufread;          /* read buffer for remote dump */
        struct ifnet     *dmp_ifp;              /* the address of interface if struct */
        struct sockaddr_in      dmp_dest;       /* the address of destination */
        int               dmp_bufsize;          /* buffer size for dump_op() */
        int               dmp_idx;              /* index into dmp_buf */
        int               dmp_idxn;             /* length of the device dependence headers*/
        int               dmp_offset;           /* current offset into dump device */
        struct uio        dmp_uio;              /* uio for dump_op() */
        struct iovec      v;                    /* iovec for dump_op() */
        struct dumpinfo   dmp_dumpinfo;         /* dumpinfo structure */
        
};
/* An array of 2 struct dmpio elements are allocated for pertinent */
/* information about primary and secondary dump devices.           */
/* The first entry is for primary and the second entry is for secondary */

static struct dmpio dmpio[2] = { {-1,}, {-1,}, }; 

struct NFS_hdr {
        long              xid;                  /* Transmit sequence number */
        long              direction;            /* request direction */
        long              rpc_version;          /* rpc_version */
        long              program;              /* NFS_PROGRAM */
        long              version;              /* NFS_VERSION */
        long              proc_num;             /* NFS_WRITE */
        long              verf;                 /* AUTH_UNIX */
        long              cred_size;            /*  */
        long              time;                 /* time of the request */
        long              hostnamelen;          /* length */
        char              *hostname;            /* host name */
        long              uid;                  /* 0 */
        long              gid;                  /* 0 */
        long              gidlen;               /* 0 */
        long              glist;                /* 0 */
        long              verf_flavor;          /* AUTH_NULL */
        long              verf_len;             /* 0 */
        /* NFS write arg */
        char              *filehandle;          /* 32byte file handle */
        long              begoff;               /* not use */
        long              offset;               /* current offset in the file */
        long              totcnt;               /* not use */
        long              cnt;                  /* size of write data */
      
};

static struct NFS_hdr NFS_hdr[2];

struct dmpio *typeto_dmpio(dmptype)
{

        switch(dmptype) {
        case DMPD_AUTO: 
        case DMPD_PRIM: 
        case DMPD_PRIM_HALT: 
                return(&dmpio[0]);
        case DMPD_SEC:  
        case DMPD_SEC_HALT:  
                return(&dmpio[1]);
        }
        assert(dmptype == DMPD_PRIM);
        return(0);
}

struct NFS_hdr *typeto_NFS_hdr(dmptype)
{

        switch(dmptype) {
        case DMPD_AUTO: 
        case DMPD_PRIM: 
        case DMPD_PRIM_HALT: 
                return(&NFS_hdr[0]);
        case DMPD_SEC:  
        case DMPD_SEC_HALT:  
                return(&NFS_hdr[1]);
        }
        assert(dmptype == DMPD_PRIM);
        return(0);
}
/*
 * NAME:     dmp_add
 * FUNCTION: add 'func' to the cdt component dump table
 * INPUTS:   func   function pointer to caller-supplied routine which
 *                  will be called at dmp_do time to return a pointer to
 *                  its filled-in component dump table.
 * RETURNS:  0   if installed successfully
 *           -1  if function is already added
 *           -2  if no more memory can be xalloc-ed
 *
 * Space is allocated in increments of DMP_GRW_SZ (100) elements at a
 * time. When the table must be grown, the old table is copied to the
 * beginning of the newly allocated table and then freed.
 */
dmp_add(func)
CDTFUNC func;
{

        struct mdt_entry *mp;
        struct mdt_entry *mpsave;
        struct mdt_entry *new_mdt;
        int i,n;
        int rv_error;
        int intpri;

        mpsave = 0;
        rv_error = 0;
        DMP_TRCHKL(DMPADD,0,func);
        SERIALIZE(&dmp.dmp_lockword);
        for(i = 0; i < dmp.dmp_count; i++) {
                mp = &dmp.dmp_mdt[i];
                if(mp->mdt_func == func) {
                        Debug("dmp_add: already there\n");
                        rv_error = -1;
                        goto exit;
                }
                if(mp->mdt_func == 0 && mpsave == 0)
                        mpsave = mp;
        }

        if(mpsave) {
                mpsave->mdt_func = func;
                Debug("dmp_add: mpsave=%x\n",mpsave);
                goto exit;
        }

        /*
         * no empty entries.
         */
        n = dmp.dmp_count + DMP_GRW_SZ;
        if ((new_mdt = (struct mdt_entry *)jmalloc(n*sizeof(struct mdt_entry))) == 0 )
        {
                rv_error = -2;
                goto exit;
        }
        bcopy(dmp.dmp_mdt,new_mdt,dmp.dmp_count * sizeof(struct mdt_entry));
        bzero(&new_mdt[dmp.dmp_count],DMP_GRW_SZ * sizeof(struct mdt_entry));
        /* free the old mdt table */
        if(dmp.dmp_mdt)
                jfree(dmp.dmp_mdt);
        dmp.dmp_mdt = new_mdt;
        dmp.dmp_mdt[dmp.dmp_count].mdt_func = func;
        dmp.dmp_count += DMP_GRW_SZ;
exit:
        UNSERIALIZE(&dmp.dmp_lockword);
        if(rv_error)
                DMP_TRCHK(DMPADDEXIT,rv_error);
        return(rv_error);

}

int
dmpnow(dmptype)
int dmptype;
{
        int s,i,rv;

        s = i_disable(INTMAX);
	swtch();
        /* if this is invoked from the process level,
         * i.e. csa->prev == NULL, then touch the
         * stack pointer to avoid the stack overflow problem.
         * In the interrupt environment, the interrupt stack,
         * which is bigger than process stack, is used.
        */
        if ( csa->prev == NULL )
          i = *(volatile *)(get_stkp() - PAGESIZE);
        rv = dmp_do(dmptype);
        i_enable(s);
        return(rv);
}

void 
dump_func(flag, init, parms)
int flag;
void *init;
int parms;
{
    volatile int ecode;

    sec_dump_pid = thread_self();
        
    for(;;) {
        ecode = et_wait(EVENT_SYNC, 0, EVENT_SHORT);
        if (ecode == EVENT_SYNC)
            dmp_do(DMPD_SEC_HALT); /* won't return if successful */
        ecode = et_wait(EVENT_NDELAY, ecode, 0); /* clear event */
    }
}


/*
 * 3. dmp_do
 *    This routine is called by sysdumpstart command through ioctl,
 *    directly by panic and ctrl_alt_numpad.
 */
dmp_do(dmptype)
{
        int rv, i;
        int trc_rv;
        int intpri;
#ifdef _POWER_MP
        struct timestruc_t time, newtime; 
	struct dmpio *dp;	
	int    fetch_return;       /* return from fetch_and_add() */
#endif /* _POWER_MP */

        trc_rv = _trc_trcofffp(); /* modified due to trace becomes 
                                     kernel extension                  */
        /* if the dump device is /dev/sysdumpfile,i.e. dump to
           to /tmp/dump, don't disable interrupt (since fp_*
           services are used to write the dump to the file).
        */

        if (isdmpfile(dmptype))
                rv = idmp_do(dmptype);
        else
                {
                intpri = i_disable(INTMAX);
#ifdef _POWER_MP

		if ((dmptype == DMPD_SEC) || (dmptype == DMPD_SEC_HALT))
			{
			dp = typeto_dmpio(DMPD_SEC);
			if ( ! dp->dmp_netflg)
				{ 
				i_enable(INTBASE);
				fp_opendev(dp->dmp_dev,DWRITE,0,0,&dmp.dmp_secfp);
				}
			else
				{
				i_enable(INTBASE);
				dump_readp = (struct dump_read *)xmalloc(sizeof(dump_readp),4,pinned_heap);
				if (dump_readp == NULL)
					{
					rv = ENOMEM;
					i_enable(intpri);
					return(rv);
					}
				}
			i_disable(INTMAX);
			if (rv = dump_op(DUMPINIT,DMPD_SEC,dp->dmp_dumpinfo.dm_devicename,dp->dmp_dev,0))
				{
				dmp_led(LED_FAILEDSTART);
				dmp_complete(dmptype, DMPDO_FAIL);	
                		i_enable(intpri);
				return(rv);
                		}
			}

        	/* Make sure that the dump code only gets executed once on
           	a MP system.  If it gets called more than once, check to
           	see which processor is calling.  If it's the processor
           	that initiated the dump, let the code continue and the
           	led 0C5 will be shown.  If it's not the processor that
           	initiated the dump, spin forever in that processor.  */

        	fetch_return = fetch_and_add(&test, 1);

        	if (fetch_return == 0)
			{
            		proc_started_dump = CPUID;
			}
        	else
            		if (CPUID != proc_started_dump)
				{
                		while (TRUE);
				}

                /* Get current number of running cpus.  The cpus are
                   allocated a logical cpuid upon boot.  This number
                   always starts at 0.  */        
                num_of_cpus = _system_configuration.ncpus;

                /* Set state of processor that's initiating the dump. */
                proc_state[CPUID] = 1;

                /* Send a mpc to all other processors.  This will cause
                      all other processors to finish any critical sections 
                   they might be in, and then call our dmpmpc() function. */        
                mpc_send(MPC_BROADCAST, dmp_mpc_msg_intiodone);

                curtime(&time);
                curtime(&newtime);

                /* Let's wait either until all other processors
                   are stopped or 3 seconds has passed. */
                while ((proc_not_stopped()) &&
                        (time.tv_sec+4 > newtime.tv_sec))
                        {
                        curtime(&newtime);
                        }

                /* If all processors are not stopped, then resend
                   the mpc at INTMAX, and wait 3 seconds.  Then
                   take the dump no matter what has happened.  */
                if (proc_not_stopped())
                        {
                        mpc_send(MPC_BROADCAST, dmp_mpc_msg_intmax);
			/* This is just set to see if we had to
			   send the mpc twice.  */
                        intmax_mpc = 1;
                        curtime(&time);
                        curtime(&newtime);
                        while ((proc_not_stopped) &&
                               (time.tv_sec+4 > newtime.tv_sec))
                               {
                                curtime(&newtime);
                               }
                        }
#endif /* _POWER_MP */

		rv = idmp_do(dmptype);
                
                i_enable(intpri);
                }

        if(trc_rv)
                _trc_trconfp(); /* modified due to trace becomes 
                                   kernel extension                */
        return(rv);
}

/*
 * NAME:     idmp_do
 * FUNCTION: Perform step for doing a dump with reporting to LEDs
 * INPUTS:   dmptype    dump 'type'  DMPD_PRIM, DMPD_SEC,
 *           flags     
 * RETURNS:  0                      dump completed successfully
 *           DMPDO_DISABLED         dump device not configured
 *           DMPDO_FAIL             dump failed
 *           DMPDO_PART             dump failed. partial dump occurred
 *
 * 1. DUMPSTART
 * 2. For each entry in the master dump table, call the cdt function
 *    and to get the component dump table (cdt) for that component.
 *    Then call wr_cdt to output the cdt and the data it describes.
 * 3. DUMPEND
 */

idmp_do(dmptype)
{
        int i;
        int rv = 0;
        int autoflg;
        int rv_dmpdo = 0;
        dev_t dump_dev;
        CDTFUNC func;
        struct cdt *cdp;

        i_disable(INTMAX);

         /* Set dump_started so that error logging will be stopped
            when dump processing begins.  This allows the first
            error in nvram to be preserved instead of overwritten.
            We are doing this because the first error is probably
            more indicative of the problem than any others, and if
            for some reason the dump fails, we will have the initial
            error that hopefully gives us something to go on. */

        dump_started = 1;
        DMP_TRCHK(DMPDO,dmptype);
        Debug("dmp_do(%x) count=%d\n",dmptype,dmp.dmp_count);
        if(dmptype == DMPD_AUTO) {
                dmptype = DMPD_PRIM;
                autoflg = 1;
        } else {
                autoflg = 0;
        }
        switch(dmptype) {
        case DMPD_PRIM_HALT:
        case DMPD_PRIM:
                dump_dev = typetodev(DMPD_PRIM);
                /* If no dump device is configured or dump is disabled 
                ** (sysdumpnull) then exit with LED_DISABLED displayed.
                */
                if( ((int)dump_dev < 0) || ((int)dump_dev == null_mmdev) ) {
                        dmp_led(LED_DISABLED);
                        if(autoflg)
                                dmp_scroll();
                        rv_dmpdo = DMPDO_DISABLED;
                        dmp_complete(dmptype, rv_dmpdo);        
			return(rv_dmpdo);
                }
                break;
        case DMPD_SEC_HALT:
        case DMPD_SEC:
                dump_dev = typetodev(DMPD_SEC);
                /* If no dump device is configured or dump is disabled 
                ** (sysdumpnull) then exit with LED_DISABLED displayed.
                */
                if( ((int)dump_dev < 0) || ((int)dump_dev == null_mmdev) ) {
                        dmp_led(LED_DISABLED);
                        rv_dmpdo = DMPDO_DISABLED;
                        dmp_complete(dmptype, rv_dmpdo);        
			return(rv_dmpdo);	
                }
                break;
        }
        /* Everything is going well, continue to proceed */
        if(autoflg) {
                ddd_led = LED_AUTOINPROGRESS;
                dmp_led(LED_AUTOINPROGRESS);  /* 0C9 */
                dmp_scroll();
        } else {
                ddd_led = LED_DUMPINPROGRESS;
                dmp_led(LED_DUMPINPROGRESS);  /* 0C2 */
        }
        /* Start the dump.  If unable to start then exit with 
        ** LED_FAILEDSTART(0C5) displayed.
        */
        if(rv = dump_op(DUMPSTART,dmptype,0,0,0)) {
                dmp_led(LED_FAILEDSTART);
                rv_dmpdo = DMPDO_FAIL;
                dmp_complete(dmptype, rv_dmpdo);        
		return(rv_dmpdo);	
        }
        /* Dump the component dump table, invoke the component provided
        ** dump routine to dump the component dump data.  If anything 
        ** goes wrong, exit out of the loop. The failing LED will be displayed by
        ** the routine where failure occurs.
        */ 
        rv = 0;
        for(i = 0; i < dmp.dmp_count; i++) {
                if((func = dmp.dmp_mdt[i].mdt_func) && (cdp = (*func)(1))) {
                        rv = wr_cdt(cdp,dmptype);
                        (*func)(2); 
                        if(rv)
                                break;
                }
        }
       
        /* Finish the dump, signal the device driver */
        dump_op(DUMPEND,dmptype,0,0,0);
        if(rv)
		{
                Debug("wr_cdt returns %d\n",rv);
                dmp_led(LED_DUMPFAILED);
                rv_dmpdo = DMPDO_PART;
                dmp_complete(dmptype, rv_dmpdo);        
        	}
	else
		{
        	dmp_led(LED_DUMPSUCCESS);
        	dmp_complete(dmptype, rv_dmpdo);
		}

        if(rv_dmpdo)
                DMP_TRCHK(DMPDOEXIT,rv_dmpdo);
        return(rv_dmpdo);
}

/*
 * NAME:     wr_cdt
 * FUNCTION: process a component dump table (cdt).
 * INPUTS:   cdt      pointer to cdt
 *           dmptype  PRIMARY/SECONDARY/FILE type value. It is not used
 *                    here but passed on to dump_op().
 * RETURNS:  0         cdt got output to dump device successfully
 *           non-zero  errno code either from dump_op or from the d_dump
 *                     portion of the dump device. 
 *
 * The cdt is an array of one or more cdt_entry structures, with a fixed header.
 * Each cdt_entry describes a data area with an 8 byte name, a byte length,
 *   and a byte pointer. A non-zero segment value 'd_segval' can be supplied
 *   to access data located in other than the kernel segment.
 *
 * The format of the dump is a series of:
 *    a. cdt table                  This will have N cdt_entries
 *    b. N data areas:
 *       i.  bitmap of data area
 *       ii. in-memory data of data area
 * The cdt_table is written out as is.
 * The bitmap is calculated by wr_cdt using the isinmem(ptr) boolean function.
 * The data is written byte-aligned for in-memory regions. Portions of the
 *   data area that lies on a non-inmemory page get skipped, with the
 *   corresponding bitmap bit cleared.
 *   
 * wr_cdt calls dump_op to buffer byte-oriented component data areas into
 *   block (queried from the d_dump device but usually 512 byte) io
 *   transfers.
 *
 * These bitmap macros are defined in dump.h and are also used by formatting
 *   routines:
 * BTOP(addr)          virtual page containing address 'addr'
 * ISBITMAP(page)      true is bitmap bit for 'page' is set. (is in memory)
 * SETBITMAP(page)     set the bitmap bit for 'page'
 * BITMAPSIZE(ptr,len) return the number of bytes needed for the bitmap for 
 *                     data area ptr,len. (Note that ptr is needed because of
 *                     possible page overlap)
 * The parameter 'page' is 0-based, so that 0 is the start of the data area,
 * not virtual address 0.
 * The typedef 'bitmap_t' defines the data type of the bitmap. In this case,
 *   it is unsigned char.
 */
static bitmap_t bm[DMP_BMBITS / BITS_BM];

static 
wr_cdt(cdp,dmptype)
struct cdt *cdp;
{
        int i,ie;
        int num_entries;
        int bitmapsize;
        int npages;
        int page0;
        int count;
        char *addr;
        int tcount;
        int offset;
        int rv;
        int len;
        struct cdt_entry *cep;
        struct xmem xmem, *xmemp;
        int pg;

        Debug("wr_cdt() cdt=%x\n",cdp);
        rv = dump_op(DUMPWRITE,dmptype,cdp,cdp->cdt_len,0); /* comp dump table */
        if(rv)
          {
                dmp_led(LED_DUMPFAILED);
                return(rv);
          }
        /*
         * initialization of the xmem structure is machine dependent
         * For the R2, routines use the d_segval part of this union.
         * Other implementations may the xmem part of the union, which means
         *  they must allocate an xmem structure.
         * The reason for not putting the xmem structure in the cdt_ structure
         *  is to keep the cdt_ structure a fixed length so that dumps have
         *  the exact same format for any V3 platform.
         */
        num_entries = NUM_ENTRIES(cdp);        /* (cdt_len-cdt_head)/sizeof(cdt_entry) */
        Debug("wr_cdt: n=%d\n",num_entries);
        for(ie = 0; ie < num_entries; ie++) {
                cep = &cdp->cdt_entry[ie];
                Debug("wr_cdt: cep=%x ptr=%x len=%x\n",cep,cep->d_ptr,cep->d_len);
                if(cep->d_len == 0)
                        continue;
                len = MIN(cep->d_len,DMP_MAXPAGES * PAGESIZE);

                /* Currently, RT and R2 callers aren't using the
                 * proper interface (they set d_segval to an srval
                 * rather than initializing a cross-memory descriptor
                 * so we must manufacture the xmem descriptor here.
                 */
                if ( SRTOSID(cep->d_segval) )        /* non-zero means not a kernel segval */
                        /* Initialize the xmem descriptor.  The aspace_id
                         * just needs to be some value other than
                         * XMEM_GLOBAL.
                         */
                        xmem.aspace_id = ~XMEM_GLOBAL;
                else 
                        /* The field d_ptr is assumed to be a 32-bit
                         * address that is valid in the current
                         * address space.
                         */
                        xmem.aspace_id = XMEM_GLOBAL;
                xmem.subspace_id = cep->d_segval;
                xmemp = &xmem;

                /*
                 * Construct the bitmap showing in/out status of pages.
                 */
                bitmapsize = BITMAPSIZE(cep->d_ptr,len);
                page0      = BTOP(cep->d_ptr);
                npages     = NPAGES(cep->d_ptr,len);
                Debug("page0 of %x is %x. bitmapsize=%x. npages=%x\n",
                        cep->d_ptr,page0,bitmapsize,npages);
                DMP_TRCHKG(DMPWRCDT,0,cep->d_ptr,len,0,0,0);
                i = 0;
                for(pg = 0; pg < npages;) {
                        bzero(bm,sizeof bm);
                        pg += DMP_BMBITS;
                        if (pg > npages)
                                pg = npages;
                        while (i < pg) {
                                if(isinmem(xmemp,PAGESIZE * (page0+i)))
                                SETBITMAP(bm, i & DMP_BMMASK);
                                ++i;
                        }
                        Debug("bitmap for '%s' is %x\n", cep->d_name,bm[0]);
                        rv = dump_op(DUMPWRITE, dmptype, bm,
                                     bitmapsize >= sizeof bm ?
                                                sizeof bm : bitmapsize,
                                     0);
                        if(rv)
                                return(rv);
                        bitmapsize -= sizeof bm;
                }

                /*
                 * dump data area page by page
                 */
                addr  = cep->d_ptr;
                count = len;
                for(i = 0; i < npages; i++) {
                        offset = (unsigned)addr % PAGESIZE;
                        tcount = MIN(count,PAGESIZE-offset);
                        if(isinmem(xmemp, PAGESIZE * (page0+i))) {
                                /* in-memory parts of data area. */
                                rv = dump_op(DUMPWRITE,dmptype,addr,tcount,xmemp);
                                if(rv)
                                        return(rv);
                        }
                        addr  += tcount;
                        count -= tcount;
                }
        }
        Debug("return from wr_ctd\n");
        return(0);
}

/*
 * Return true if the virtual address xmem.ptr is in memory.
 * This routine machine dependent, although the interface to the
 *   routine is machine independent.
 */
isinmem(xp,ptr)
struct xmem *xp;
char *ptr;
{
        extern char *vm_att();
        char *ptr2;
        int rv;

        if (xp) {
            ptr2 = vm_att(xp->subspace_id,ptr);
            rv = lra(ptr2);
            vm_det(ptr2);
        } else {
            rv = lra(ptr);
        }
        return(rv == -1 ? 0 : 1);
}

typetodev(dmptype)
{

        switch(dmptype) {
        case DMPD_AUTO: 
        case DMPD_PRIM:
        case DMPD_PRIM_HALT:
                return(dmpio[0].dmp_dev);
        case DMPD_SEC:  
        case DMPD_SEC_HALT:  
                return(dmpio[1].dmp_dev);
        }
        assert(dmptype == DMPD_PRIM);
        return(-1);
}

/*
 * Fill in the status of the dump and write it out to nvram.
 */
static dmp_ccstatus(dmptype,status)
{
        struct dmpio *dp;


        dp = typeto_dmpio(dmptype);
        dp->dmp_dumpinfo.dm_status = status;
        dp->dmp_dumpinfo.dm_flags = DMPFL_NEEDLOG | DMPFL_NEEDCOPY;
        nv_dumpinfo = dp->dmp_dumpinfo;
        dmp_nvwrite(&dp->dmp_dumpinfo);
}

/*
 * true if dump device is sysdumpfile
 */
isdmpfile(dmptype)
{
        int mmdev;

        mmdev = typetodev(dmptype);
        return(major(mmdev) == Mdev && minor(mmdev) == MDEV_FILE);
}


/*
 * NAME:     dump_op
 * FUNCTION: interface between dmp_do/wr_cdt and d_dump routines.
 * INPUTS:   op       DUMPINIT,DUMPTERM,DUMPSTART,DUMPSTOP,DUMPWRITE,DUMPQUERY
 *           dmptype  PRIMARY/SECONDARY/FILE
 *           buffer   buffer for DUMPWRITE
 *           count    byte length of 'buffer' for DUMPWRITE
 *                    dev_t dump device for DUMPINIT
 *           xmem     xmem descriptor passed down from wr_cdt() to be passed 
 *                    to jwrite()
 * RETURNS:  0        operation performed successfully
 *           EIO      no configured device for type 'dmptype'
 *           ENOMEM   cannot allocate pinned memory for internal dump buffer
 *                    (DUMPINIT)
 *           ENOMEM   minimum blocksize for d_dump device is greater than the
 *                    preset maximum of DMP_BUF_MAX.  (DUMPINIT)
 *           non-zero errno code either from dump_op or from the d_dump
 *                    portion of the dump device. 
 *
 * The first part of dump_op converts 'dmptype' to a pointer to a dmpio
 * structure.
 */


dump_op(op,dmptype,buffer,count_mmdev,xp)
char *buffer;
struct xmem *xp;
{
        int nblocks;
        int count,i,p_len;
        struct dmp_query dmp_query;
        struct dmpio *dp;
        struct NFS_hdr *NFS_hdr;
        struct ip   *ip;
        int rv_error;
        dev_t d_dev;

        int num_tries;
        int mypacket;
        int timeend, timenow, timeleft;
        struct timestruc_t c_time;
        int saved_idx, saved_dumpsize, saved_offset;
        struct arptab *at;
        struct ndd *dmp_nddp;

        rv_error = 0;
        
        if ((dmptype == DMPD_SEC) || (dmptype == DMPD_SEC_HALT))
                dmp_nddp = sec_nddp;
        else
                dmp_nddp = prim_nddp;
        /* get the address to the dmpio structure */
        dp = typeto_dmpio(dmptype);
        Debug("dump_op(%x,%x,%x,%x) dp=%x\n",op,dmptype,buffer,count,dp);
        switch(op) {
        case DUMPINIT:
                d_dev = count_mmdev;
                dp->dmp_dev = -1;
                /* Signal the chosen dump device driver to init and 
                ** pin itself.
                */
                dmp_query.min_tsize = 0;
                dmp_query.max_tsize = 0;
                if (dp->dmp_netflg)   /* network dump device */
                        {
			/* We cannot call xmalloc() from the interrupt level. */
			/* So we need to do this xmalloc() for secondary dump */
			/* in the DUMPSTART routine.  */
			if ((dmptype != DMPD_SEC) && (dmptype != DMPD_SEC_HALT))
				{
				dump_readp = (struct dump_read *)xmalloc(sizeof(dump_readp),4,pinned_heap);
				if (dump_readp == NULL)
					{
					rv_error = ENOMEM;
					break;
					}
				}

                        /* For network dump, get the address of the network device's dump
                              routine. */
                               if (rv_error = (* (dmp_nddp->ndd_ctl))(dmp_nddp, NDD_DUMP_ADDR, &remote_dump))
                                       break;
                        /* Call the network device's dump routine to initialize itself as
                              the dump device. */
                               if (rv_error = (* remote_dump)(dmp_nddp, DUMPINIT, 0))
                                       break;
                               if (rv_error = (* remote_dump)(dmp_nddp, DUMPQUERY, &dmp_query))
                                       break;
                        if(dmp_query.max_tsize == 0) 
                                dmp_query.max_tsize = 512;
                        dp->dmp_blocksize = dmp_query.min_tsize - (dmp_query.min_tsize % 8) + 8;
                        } 
                else  /* local dump device */
                        {
                        if(rv_error = devdump(d_dev,0,DUMPINIT,0,0,0))
                                break;
        
                        /* Query the dump device driver for the max and min 
                           transmission units. */ 
                        dmp_query.min_tsize = 0;
                        dmp_query.max_tsize = 0;
                        if(rv_error = devdump(d_dev,0,DUMPQUERY,&dmp_query,0,0))
                                break;
                
                        if(dmp_query.max_tsize == 0) 
                                {
                                Debug("dump_op: DUMPQUERY: no max_tsize\n");
                                dmp_query.max_tsize = 512;
                                }

                        if(dmp_query.min_tsize == 0)
                                dmp_query.min_tsize = 512;
                        if(dmp_query.min_tsize > DMP_BUF_MAX) 
                                {
                                rv_error = ENOMEM;
                                break;
                                }        

                        nblocks = MIN(dmp_query.max_tsize,DMP_BUF_MAX)/dmp_query.min_tsize;
                        if(nblocks == 0)
                                nblocks = 1;
                        dp->dmp_blocksize = dmp_query.min_tsize;
                        dp->dmp_bufsize   = nblocks * dp->dmp_blocksize;
                        } /* end local dump */

                dp->dmp_dev = d_dev;
                /* Free the old buf and allocate the new buf */
                if ((dp->dmp_netflg) || (dmptype == DMPD_PRIM) || (dmptype == DMPD_PRIM_HALT))
                        {
                        if(dp->dmp_buf) 
                                {
                                jfree(dp->dmp_buf);
                                dp->dmp_buf = 0;
                                }

                        if (dp->dmp_netflg)
                                {
                                dp->dmp_bufwrite = m_getclust(M_WAIT,MT_DATA);
                                dp->dmp_bufread = m_getclust(M_WAIT,MT_DATA);
                                dp->dmp_buf = dp->dmp_bufwrite->m_ext.ext_buf;
                                /* prebuild IP header */
                                build_IP_header(dp);
                                dp->dmp_bufwrite->m_data = dp->dmp_buf + dp->dmp_idxn;
                                }
                        else
                                dp->dmp_buf = jmalloc(dp->dmp_bufsize);

                        if(dp->dmp_buf == 0) 
                                {
                                rv_error = ENOMEM;
                                break;
                                }
                        VCPY(buffer ? buffer : "",dp->dmp_dumpinfo.dm_devicename);
                        }
                break;
        case DUMPINIT2:
                d_dev = count_mmdev;
                if(dp->dmp_buf) 
                        {
                        jfree(dp->dmp_buf);
                        dp->dmp_buf = 0;
                        }
                if (!dp->dmp_netflg)
                        {
                        if(devsw[major(d_dev)].d_dump && devsw[major(d_dev)].d_dump != nodev)
                                {
                                dp->dmp_dev = d_dev;
                                VCPY(buffer ? buffer : "",dp->dmp_dumpinfo.dm_devicename);
                                dp->dmp_buf = jmalloc(DMP_BUF_MAX);
                                }
                        else
                                rv_error = ENODEV;
                        }
		else
			{
			dp->dmp_dev = d_dev;
                        VCPY(buffer ? buffer : "",dp->dmp_dumpinfo.dm_devicename);
			}
                break;
        case DUMPTERM:
                if(dp->dmp_dev == -1) 
                        {
                        rv_error = EIO;
                        break;
                        }
                if (dp->dmp_netflg)
                        {
                        dp->dmp_bufwrite->m_extfree = NULL;
                        dp->dmp_bufread->m_extfree = NULL;
                        m_freem(dp->dmp_bufwrite);
                        m_freem(dp->dmp_bufread);
                        dp->dmp_bufwrite = 0;
                        dp->dmp_bufread = 0;
                        /* I commented this out to fix defect 46545
                        if ( get_arp_entry(&at,&dp->dmp_dest) == 0 )
                                at->at_flags &= ~ATF_PERM;
                        */
			xmfree(dump_readp,pinned_heap);
                        }
                else
                        {
                        if(dp->dmp_buf) 
                                {
                                jfree(dp->dmp_buf);
                                dp->dmp_buf = 0;
                                dp->dmp_bufsize = 0;
                                }
                        }
                dp->dmp_blocksize = 0;
                dp->dmp_buf       = 0;
                if (dp->dmp_netflg)
                        {
                        dp->dmp_netflg = 0;
                               rv_error = (* remote_dump)(dmp_nddp,DUMPTERM,0);
                               ns_free(dmp_nddp);
                        }
                else
                        rv_error = devdump(dp->dmp_dev,0,DUMPTERM,0,0,0);
                break;
        case DUMPSTART:
#ifndef _POWER_MP
                if ((dmptype == DMPD_SEC) || (dmptype == DMPD_SEC_HALT)) 
                        {
                        int s;

                        s = i_disable(INTMAX);
                        dp = typeto_dmpio(DMPD_SEC);
                        if ( ! dp->dmp_netflg )
                                {
                                i_enable(INTBASE);
                                fp_opendev(dp->dmp_dev,DWRITE,0,0,&dmp.dmp_secfp);
                                } 
			else
				{	
				i_enable(INTBASE);
				dump_readp = (struct dump_read *)xmalloc(sizeof(dump_readp),4,pinned_heap);
				if (dump_readp == NULL)
					{
					rv_error = ENOMEM;
					break;
					}
				}

                        i_disable(s);
                        if(rv_error = dump_op(DUMPINIT,DMPD_SEC,dp->dmp_dumpinfo.dm_devicename,dp->dmp_dev,0))
                                {        
                                dmp_led(LED_FAILEDSTART);
                                goto exit;
                                }
                        }
#endif
                if(dp->dmp_dev == -1) 
                        {
                        rv_error = EIO;
                        break;
                        }
                if(dp->dmp_buf == 0) 
                        {
                        rv_error = ENOMEM;
                        break;
                        }
                if ( !dp->dmp_netflg ) /* dump to filesystem or tape */
                        {
                        dp->dmp_idx      = 0;
                        /* now skip sector 0 which contains lvm control block info */
                        dp->dmp_offset   = 512;
                        }
		curtime(&c_time);
                /* Initialize the dumpinfo with data */
                dp->dmp_dumpinfo.dm_mmdev     = dp->dmp_dev;
                dp->dmp_dumpinfo.dm_size      = 0;
                dp->dmp_dumpinfo.dm_timestamp = c_time.tv_sec;
                dp->dmp_dumpinfo.dm_type      = dmptype;

                /* start the dump */
                if (dp->dmp_netflg)
                        rv_error = (* remote_dump)(dmp_nddp,DUMPSTART,0);
                else
                        rv_error = devdump(dp->dmp_dev,0,DUMPSTART,0,0,0);
                break;
        case DUMPEND:
                jflush(dp);
                if (dp->dmp_netflg)
                        rv_error = (* remote_dump)(dmp_nddp,DUMPEND,0);
                else
                        rv_error = devdump(dp->dmp_dev,0,DUMPEND,0,0,0);
                break;
        case DUMPWRITE:
                count = count_mmdev;
                if (dp->dmp_netflg) /* remote dump */
                /* Pseudo: Each section of the dump is packaged as
                **         an NFS write request RPC call. In general, the 
                **         format of the packet is:
                **         - Device driver specific headers.
                **         - IP header.
                **         - UDP header.
                **         - NFS header.
                **         - dump data.
                ** The space is reserved in the buffer for the Device 
                ** specific headers, IP header is already filled in 
                ** at DUMPINIT time.
                */
                {
                  NFS_hdr = typeto_NFS_hdr(dmptype);
                  NFS_hdr->xid++;
                  /* increment the id of the ip packet. */
                  ip = (struct ip *) ( dp->dmp_buf + dp->dmp_idxn);
                  ip->ip_id++;
                  /* save the index and count for retry purpose */
                  saved_idx = dp->dmp_idx;
                  saved_dumpsize = dp->dmp_dumpinfo.dm_size;
                  saved_offset = NFS_hdr->offset;
                  num_tries = 0;
                  mypacket = 0;

                  ddd_namelen = NFS_hdr->hostnamelen;
                  i = ddd_namelen % 4;
                  if  ( i )
                    {
                      i = 4 - i;
                      ddd_namelen += i;
                    } 
                  /* calculate the packet length */
                  p_len = sizeof(udp) + sizeof(struct NFS_hdr) - 8 + ddd_namelen + FILEHNDL_LEN + count;

                  while ( !mypacket && num_tries <= MAX_RETRIES )
                  {
                    ddd_timeout = 50;
                    dmp_led(ddd_led);
                    dp->dmp_idx = saved_idx;
                    dp->dmp_dumpinfo.dm_size = saved_dumpsize;
                    NFS_hdr->offset = saved_offset;
                    NFS_hdr->time = time;
                    if ( num_tries > 0 )
                      ddd_waittime = (((ddd_waittime << 2) > TOTALWTIME) ? TOTALWTIME : ddd_waittime << 2 );
                    num_tries++;
                    ddd_fragoff = 0;  /* static external will be used by jwrite_io */
                    ddd_moredata = 1;

                    if ((dp->dmp_bufsize-dp->dmp_idx) > p_len)
                    {
                    /* The following flags are needed for the IP fragmentation.*/
                    /* See jwrite_io for the more details.                     */
                       ddd_nofrag = 1;
                    }
                    else ddd_nofrag = 0;
   

                    /* write out UDP header */
                    udp.uh_ulen = htons(p_len);
                    if (rv_error = jwrite(dp,&udp,sizeof(udp),0))
                      break;

                    /* write out NFS header */

                    i = (char *)(&NFS_hdr->hostname) - (char *)NFS_hdr;
                    if (rv_error = jwrite(dp,NFS_hdr,i,0))
                      break;

                    if (rv_error = jwrite(dp,NFS_hdr->hostname,ddd_namelen,0))
                      break;

                    i = (char *)(&NFS_hdr->filehandle) - (char *)(&NFS_hdr->uid);

                    if (rv_error = jwrite(dp,&NFS_hdr->uid,i,0))
                      break;

                    if (rv_error = jwrite(dp,NFS_hdr->filehandle,FILEHNDL_LEN,0))
                      break;

                    NFS_hdr->cnt = count; 
                    i = (char *)(&NFS_hdr->cnt) - (char *)(&NFS_hdr->begoff) + sizeof(int);

                    if (rv_error = jwrite(dp,&NFS_hdr->begoff,i,0))
                      break;

                    NFS_hdr->offset += count;
                    /* Set the flags that are used for setting the ip_off of the IP header */
                    /* This is mean no more data but not finish yet                        */
                    ddd_moredata = 0;
                    ddd_finish = 0;
                    if ( rv_error = jwrite(dp,buffer,count,xp))
                      break;
                    CALTIMENOW(timenow,c_time);
                    timeend = timenow + ddd_waittime;
                    CALTIMELEFT(timenow, timeend, timeleft);
                    dmp_led(LED_WAIT4RESP);
                    /* Signal the device driver to poll for the 
                    ** NFS/RPC acknowlegment until timeout or get the 
                    ** packet.
                    */
                    while ( timeleft && !mypacket)
                    {
                      rv_error = dump_op(DUMPREAD,dmptype, 0,dp->dmp_dev,0);
                      switch (rv_error)
                                      {
                                case 0:
                                        if (!(mypacket = process_packet(dp->dmp_bufread,NFS_hdr->xid)))
                                                {
                                                /* reset the bufread */
                                                dp->dmp_bufread->m_data = dp->dmp_bufread->m_ext.ext_buf;
                                                CALTIMEOUT(ddd_timeout);
                                                } 
                                        CALTIMENOW(timenow,c_time);
                                        CALTIMELEFT(timenow,timeend,timeleft);
                                        break;
                                case ETIMEDOUT:
					timeleft = 0;
                                        break;
                                case ENETDOWN:
					timeleft = 0;
                                        dmp_led(LED_DUMPFAILED);
                                        break;
                                }
                    }
         
                  } /* end outer while loop */
                  if (rv_error)
                    break;
                  if ( !mypacket )
                  {
                    rv_error = -1;
                    break;
                  }
                  dmp_led(ddd_led);
                  rv_error = process_response(dp->dmp_bufread);
                  if ( rv_error == 0)
                  {
                     /* calculate the dump size for remote dump here
                     ** because we don't want to include the size of NFS
                     ** header and UPD header in the size of the dump.
                     */
                     dp->dmp_dumpinfo.dm_size += count;
                  } 
                }
                else
		 rv_error = jwrite(dp,buffer,count,xp);
                break;
        case DUMPREAD:
                dump_readp->dump_bufread = dp->dmp_bufread;
                dump_readp->wait_time = ddd_timeout;        
                rv_error = (* remote_dump)(dmp_nddp,DUMPREAD,dump_readp);
                break;
        default:
                assert(op == DUMPINIT);
                return(-1);
        }
exit:
        if(rv_error)
                DMP_TRCHKL(DMPOPEXIT,rv_error,op);
        Debug("return from dump_op. error=%d\n",rv_error);
        return(rv_error);
}

/*
 * NAME:     jwrite
 * FUNCTION: byte-to-block buffering routine for dump_op/DUMPWRITE
 * INPUTS:   dp        pointer to dmpio structure
 *           ubuf      target buffer address
 *           ucount    size in bytes of 'ubuf'
 *           xp        xmem descriptor passed down from wr_cdt()
 *                     If 0, use bcopy. (use current segments)
 * RETURNS:  0         operation performed successfully
 *           otherwise error code from d_dump routine
 *
 * Byte-oriented ubuf (wr_ctdt data) is written to a dmp_buf buffer
 * allocated at DUMPINIT time. The size of the buffer is a multiple of
 * the min_tsize (blocksize) as returned by the DUMPQUERY command.
 * When the dmp_buf buffer fills, the d_dump is called to write the
 * buffer out. The flush routine is called at the end of dmp_do
 * to flush the unwritten dmp_buf buffer.
 */

static jwrite(dp,ubuf,ucount,xp)
struct dmpio *dp;
char *ubuf;
int ucount;
struct xmem *xp;
{
        int tcount;
        int uidx;
        int rv;

        Debug("jwrite(%x,%x,%x,%x)\n",dp,ubuf,ucount,xp);
        uidx = 0;
loop:
        if(ucount == 0)
                return(0);
        tcount = MIN(ucount,dp->dmp_bufsize - dp->dmp_idx);
        /* Because the device driver throw away small packet, I need to
        ** adjust the last packet of the fragmented request to make sure
        ** it is greater than the smallest packet allowed by the network
        ** device driver
        */
        if ( dp->dmp_netflg  &&
             !(ddd_moredata) &&
             ( (tcount - ucount) != 0)  &&
             ( (tcount - ucount) < dp->dmp_blocksize)) 
          tcount -= dp->dmp_blocksize;
        if(xp)
                xmemin(ubuf+uidx,dp->dmp_buf+dp->dmp_idx,tcount,xp);
        else
                bcopy(ubuf+uidx,dp->dmp_buf+dp->dmp_idx,tcount);
        dp->dmp_idx += tcount;
        uidx += tcount;
        ucount -= tcount;
        if( (ucount > 0 ) || (dp->dmp_idx == dp->dmp_bufsize )) {
                if (dp->dmp_netflg && !(ddd_moredata) && (ucount == 0))
                  ddd_finish = 1;
                rv = jwrite_io(dp,dp->dmp_buf,dp->dmp_idx);
                /*
                dp->dmp_idx = 0;
                */
                if (rv)
                        return(rv);
        }
         
        else if (dp->dmp_netflg && !(ddd_moredata) && (ucount == 0))
               {
                  ddd_finish = 1;
                  rv = jflush(dp);
                  if (rv)
                    return(rv);
               }
        goto loop;
}

static jflush(dp)
struct dmpio *dp;
{
   int n;
   int tcount;

   if (dp->dmp_netflg)
     if (dp->dmp_idx == (dp->dmp_idxn + sizeof (struct ip)))
       return(0);
     else 
       return(jwrite_io(dp,dp->dmp_buf,dp->dmp_idx));
   else 
     if(dp->dmp_idx > 0) {
                tcount = dp->dmp_idx;
                n = dp->dmp_idx % dp->dmp_blocksize;        /* multiple of a block */
                if(n) {
                        n = dp->dmp_blocksize - n;
                        bzero(dp->dmp_buf + dp->dmp_idx,n);
                        tcount += n;
                }
                return(jwrite_io(dp,dp->dmp_buf,tcount));
     }
     /* dp->dmp_idx = 0; */
  return(0);
}

static jwrite_io(dp,buf,count)
struct dmpio *dp;
char *buf;
{
        int rv;
        struct ip* ip;

        Debug("jwrite_io(%x,%x) offset %x\n",buf,count,dp->dmp_uio.uio_offset);
        /* fill in the ip_off and ip_len in case of remote dump */
        if (dp->dmp_netflg) /* remote dump */
        {
          /* does not use the uio structure */
          dp->dmp_uio.uio_resid = 0;
          dp->dmp_uio.uio_offset = 0;

          /* locate the address of the IP header */
          ip = (struct ip *) ( buf + dp->dmp_idxn);
          ip->ip_len = htons(count - dp->dmp_idxn);
          /* In general, the logic to figure out what to put in ip_off is
          ** if the whole thing fits in one mtu,i.e no IP fragment is 
          ** needed, ip_off should be 0. On the other hand, ip_off will be the
          ** accummulative length in words of the previous IP fragments logical
          ** and with IP_MF(more IP fragment) bit pattern.  But for the last 
          ** packet don't do the "logical and".
          */
          if (ddd_nofrag)
            ip->ip_off = 0;
          else
          {
            if (ddd_moredata || !(ddd_finish) )
              ip->ip_off = htons((u_short)((ddd_fragoff >> 3) | IP_MF));
            else ip->ip_off = htons((u_short) (ddd_fragoff >> 3));
          }
          ip->ip_sum = 0;
          ip->ip_sum = in_cksum(ip, sizeof(struct ip));
          ddd_fragoff += count - (dp->dmp_idxn + sizeof (struct ip));
          /* invoke the if_output routine here to dump the dump data 
           * to the network                                            */
          dp->dmp_bufwrite->m_len = dp->dmp_idx - dp->dmp_idxn;
          rv = (*dp->dmp_ifp->if_output)(dp->dmp_ifp,dp->dmp_bufwrite, 
                (struct sockaddr *)&dp->dmp_dest, (struct rtentry *)0 );
          if ( ddd_mbuf_free )
            rv = EIO;
          dp->dmp_bufwrite->m_data = dp->dmp_buf + dp->dmp_idxn;
          dp->dmp_idx = dp->dmp_idxn + sizeof(struct ip);
        }
        else /* local dump */
        {
          dp->dmp_uio.uio_resid  = count;
          dp->v.iov_len    = count;
          dp->v.iov_base   = buf;
          dp->dmp_uio.uio_offset = dp->dmp_offset;
          dp->dmp_uio.uio_segflg = UIO_SYSSPACE;
          dp->dmp_uio.uio_iov    = &dp->v;
          dp->dmp_uio.uio_iovcnt = 1;
          rv = devdump(dp->dmp_dev,&dp->dmp_uio,DUMPWRITE,0,0,0);
          dp->dmp_idx = 0;
          if(rv == 0) {
                dp->dmp_dumpinfo.dm_size += count - dp->dmp_uio.uio_resid;
                dp->dmp_offset += (count - dp->dmp_uio.uio_resid);
                if(dp->dmp_uio.uio_resid != 0)
                        rv = EIO;
          }
        }
        return(rv);
}

dmpnull(devno,uiop,op,arg,chan,ext)
dev_t devno;
struct uio *uiop;
{

        Debug("dmpnull(%x,%x,%d)\n",devno,uiop,op);
        DMP_TRCHK(DMPNULL,op);
        switch(op) {
        case DUMPINIT:
                return(0);
        case DUMPTERM:
                return(0);
        case DUMPSTART:
                return(0);
        case DUMPEND:
                return(0);
        case DUMPQUERY:
          {
                struct dmp_query *qp;

                if(arg == 0)
                        return(EINVAL);
                qp = (struct dmp_query *)arg;
                qp->min_tsize = 512;
                qp->max_tsize = 512;
                return(0);
          }
        default:
                return(EIO);
        }
}

/*
 * NAME:     dmp_led
 * FUNCTION: interface to mdio led routine
 * INPUTS:   led value encoded in lower three digits
 * RETURNS:  none
 *
 * The call to nvled() displays the led on the led panel.
 * The rest of the routine is writing the led to the led
 * scroll area in nvram.  This area is reserved for the leds
 * that need to be scrolled through when an 888 (panic) occurs.
 * The led scroll area starts at 0x0320 in nvram.  We write at
 * 0x0326 because we are writing the fourth led. (0c0, 0c4, ..)
 */
static dmp_led(n)
{
        char *base;
        ulong nvram_base;
        volatile ushort *led;

        nvled(n);

#ifdef _RS6K
        if (__rs6k())
                {
                nvram_base = &sys_resource_ptr->nvram;
                led = (ushort *)(nvram_base + 0x00000326);
                *led = n << 4;
                __iospace_sync();
                }
#endif
#ifdef _POWER_RS
	if (__power_rs())
                {
                if(base = io_att(IOCC_BID,0))
                        {
                        *(short *)(base + 0xA00326) = (n << 4);
                        io_det(base);
                        }
                }
#endif
}

/*
 * Called at DMPD_AUTO time to turn on the scrollable bit in nvram.
 * This allows you to scroll through the leds by using the yellow
 * button when a panic occurs.  We are writing to offset 0x037C in
 * nvram.  This is the ocs command word.
 */
static dmp_scroll()
{
        char *base;
        ulong nvram_base;
        volatile ulong *ocs;


#ifdef _RS6K
        if (__rs6k())
                {
                nvram_base = &sys_resource_ptr->nvram;
                ocs = (ulong *)(nvram_base + 0x0000037C);
                *ocs |= 0x01000000;
                __iospace_sync();
                }
#endif
#ifdef _POWER_RS
	if (__power_rs())
                {
                if(base = io_att(IOCC_BID,0))
                        {
                        if (__power_rsc()) 
                                base[0xA00315] = 0x01;
                        else 
                                base[0xA0037C] |= 0x01;
                        io_det(base);
                        }
                }
#endif
}

dmp_nvwrite(infp)
struct dumpinfo *infp;
{
        int rv;
        int size;
        static printflg;

        size = sizeof(*infp);
        rv = nvwrite(DUMP_NVBLOCK,(uchar *)infp,0,size);
        if(rv != size) {
                return(-1);
        }
        return(1);
}

static in_cksum(addr, len)
u_short *addr;
int len;
{
        register int nleft = len;
        register u_short *w = addr, tmp;
        register u_short answer;
        register int sum = 0;

        /*
         *  Our algorithm is simple, using a 32 bit accumulator (sum),
         *  we add sequential 16 bit words to it, and at the end, fold
         *  back all the carry bits from the top 16 bits into the lower
         *  16 bits.
         */
        while( nleft > 1 )  {
                sum += *w++;
                nleft -= 2;
        }

        /* mop up an odd byte, if necessary */
        if( nleft == 1 ) {
                tmp = *(u_char *)w;
                sum += (tmp << 8);
        }

        /*
         * add back carry outs from top 16 bits to low 16 bits
         */
        sum = (sum >> 16) + (sum & 0xffff);        /* add hi 16 to low 16 */
        sum += (sum >> 16);                        /* add carry */
        answer = ~sum;                                /* truncate to 16 bits */
        return (answer);
}
struct RPC_reply {
                   int          xid;
                   int          direction;
                   int          rp_stat;
                   int          rp_flavor;
                   int          len;
                   char         rest[1];
                 };

static process_response(response)
struct mbuf *response;
{
  struct RPC_reply  *RPC_response;
  int               *ar_stat;
  int               *NFS_stat;

  RPC_response = (struct RPC_reply *)(response->m_data + sizeof(struct ip) +
                 sizeof(struct udphdr));
  response->m_data = response->m_ext.ext_buf;

  /* return -1 if it's not a reply RPC with a good RPC message status */
  if ((RPC_response->direction != REPLY) || 
      (RPC_response->rp_stat != 0 ))         /* 0 == MSG_ACCEPTED */
     return(-1);
  else
  {
    ar_stat = (int *)(&RPC_response->rest + RPC_response->len);
    if (*ar_stat != 0 )   /* 0 == SUCCESS */
      return(-1);
    else 
    {
      NFS_stat = (int *)((char *)ar_stat + sizeof(int));
      return(*NFS_stat);
    }
  }
}

static process_packet(packet,xid)
struct mbuf *packet;
int xid;

{
  struct udphdr *udp_hdr;
  struct RPC_reply *RPC_rep;

  /* assumption: ext_buf points to the beginning of the buffer;
                 m_data points to start of data section 
  */
  if ( packet->m_data == packet->m_ext.ext_buf )
	{
    return(FALSE);
	}
  /* Get the address of the UDP header.
     m_data points to IP header. Walk the size of IP header, I should
     have the address of the UDP header 
  */
  udp_hdr = (struct udphdr *) (packet->m_data + sizeof(struct ip));
  RPC_rep = (struct RPC_reply *) ((char *)udp_hdr + sizeof(struct udphdr));
  if (udp_hdr->uh_sport != 2049 ||
      udp_hdr->uh_dport != 999  ||
      RPC_rep->xid != xid)
	{
     return(FALSE);
	}
 else
	{
 return(TRUE);
	}
}

#ifdef _POWER_MP
/*
 * NAME: dmpmpc
 *
 * FUNCTION: This function is called by all processors at dump time,
 *              except the processor that's initiating the dump.
 *           It's function is to stop all processors, and allow the         
 *		so that we can take the dump.
 *
 * INPUTS: none
 *
 * RETURNS: none
 */
void
dmpmpc()
{
        int i;
        struct timestruc_t time, newtime;

        /* Make sure nothing else can run on this processor. */
        i_disable(INTMAX);

        /* Set the state value of this cpuid to show that it's stopped. */ 
        proc_state[CPUID] = 1;

        while (TRUE);
}

/*
 * NAME: proc_not_stopped
 *
 * FUNCTION: This function checks the value of the processor
 *           states in the proc_state array to see if any processor
 *           has not been stopped yet.
 *
 * INPUTS: none
 *
 * RETURNS:
 *      0: all processors have been stopped
 *      1: all processors have not been stopped
 */
static
proc_not_stopped()
{
int i;

        for (i = 0; i < num_of_cpus; i++)
                {
                if (!proc_state[i])
                        return (1);
                }

        return (0);
}

#endif /* _POWER_MP */

/*
 * NAME: dmp_complete 
 *
 * FUNCTION: This function puts the dump status into nvram, and then 
 *              reboots the system if the autorestart variable has been
 *              set to  true.  Otherwise, we sleep waiting for someone to
 *              manually reboot the system.
 *
 * INPUTS:
 *         type:   the type of dump that was taken (panic, forced, ...)
 *        status: status of the dump (failed, partial, ...) 
 *
 * RETURNS:  
 *      none        
 * 
 */ 
static
dmp_complete(type, status)
int type;
int status;
{
        /* record status to nvram */
        dmp_ccstatus(type, status);

        /* If this is a forced dump, then check to see if the */
        /* autorestart variable is set to true.  If so, then  */
        /* call sr_slih() to automatically reboot the system. */
        /* Otherwise, you sleep until someone hits the power  */
        /* switch. */ 
        if ((DMPD_PRIM_HALT == type) || (DMPD_SEC_HALT == type))
                {
#ifdef _RSPC
		/* This is only set by a system reset interrupt on
		 * 2 kinds of RSPC models */
		extern uint sr_push_rspc;

		if (__rspc()) {
			/* If called from sr_flih then check auto restart if 
			 * not set then power down the box, well, at least
			 * try.  Later this abstraction will be in residual
			 * data but for now hard code power off or just spin.
			 */
			if (sr_push_rspc && !v.v_autost) {

				extern struct io_map nio_map;
				volatile char	*ioseg;

				ioseg = (char *)iomem_att( &nio_map );
			
				/* Turn power off */
				*(ioseg + 0x856) = 1;
				__iospace_sync();
	
				/* Might not get through this but give 
				 * it a go */
				iomem_det((void *)ioseg);

				/* Wait for power to go off */
				while(1);
			}
		}
#endif /* _RSPC */
                if (v.v_autost)
                        sr_slih(1);
                while (TRUE);
                } 
	
	if (status)
               	DMP_TRCHK(DMPDOEXIT,status);
}        

