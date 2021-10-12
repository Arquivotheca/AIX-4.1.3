static char sccsid[] = "@(#)89        1.9  src/bos/kernel/io/dumpdd_pg.c, sysdump, bos411, 9433A411a 8/8/94 10:58:18";
/*
 * COMPONENT_NAME: SYSDUMP    /dev/sysdump pseudo-device driver
 *
 * FUNCTIONS: dmpinit, dmp_add, dmp_del, dmp_prinit
 * 
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* This file contains routines for dump that exist in page, not
 * real memory
 * dump device driver, /dev/sysdump.
 *    It contains:
 *    1. ioctl routine that the commands sysdumpstart and sysdumpdev talks to.
 *    2. a default devsw.d_dump routine to testing dmp_do().
*/

#include "dump_pg.h"
#include <sys/ndd.h>

#ifdef _POWER_MP
extern mpc_msg_t dmp_mpc_msg_intiodone;
extern mpc_msg_t dmp_mpc_msg_intmax;
extern void dmpmpc();
#endif /* _POWER_MP */

pid_t sec_dump_pid = -1;
struct ndd *prim_nddp;
struct ndd *sec_nddp;

dmpioctl(mmdev,cmd,arg,mode)
dev_t mmdev;
{
    int rv,rv_error,s,i;
    static initflg;

    if(!initflg) {
        initflg++;
        Mdev = major(mmdev);
        devsw[Mdev].d_open  = dmpopen;
        devsw[Mdev].d_close = dmpclose;
        devsw[Mdev].d_ioctl = dmpioctl;
        devsw[Mdev].d_dump  = dmpdump;
        null_mmdev = makedev(Mdev,MDEV_NULL);
        }
    rv_error = 0;
    switch(cmd) {
    case IOCINFO:
    case DMP_IOCSTAT:
    case DMP_IOCSTAT2:
    case DMP_SIZE:
        break;
    case DMP_IOCINFO:
    case DMPSET_PRIM:
    case DMPSET_SEC:
    case DMPNOW_AUTO:
    case DMPNOW_PRIM:
    case DMPNOW_SEC:
    case DMP_IOCHALT:
        if(ras_privchk() < 0) {
            rv_error = EACCES;
            goto exit;
        }
        break;
    default:
        rv_error = EINVAL;
        goto exit;
    }
    switch(cmd) {
    case IOCINFO:
        rv_error = iocinfo(arg);
        break;
    case DMP_IOCHALT:
        dmp.dmp_haltondump = arg ? 1 : 0;
        break;
    case DMP_IOCSTAT: /* get statistical data from NVRAM */
        rv_error = dmp_iocstat(arg);
        break;
    case DMP_IOCSTAT2: /* used by "sysdumpdev -z" */
        rv_error = dmp_iocstat2(arg);
        break;
    case DMP_IOCINFO:
        rv_error = dmp_iocinfo(arg);
        break;
     case DMP_SIZE:
        rv_error = dmp_size(arg);
                break;
    case DMPSET_PRIM: /* sysdumpdev -p <device name> */
        rv_error = dmpset_prim(arg);
        break;
    case DMPSET_SEC:
        rv_error = dmpset_sec(arg);
        break;
    case DMPNOW_PRIM: /* is used by sysdumpstart -p */
        SERIALIZE(&dmp.dmp_lockword1);
        if(isdmpfile(DMPD_PRIM)) {
            rv = dmp_do(DMPD_PRIM_HALT);
        } else {
            rv = dmpnow(DMPD_PRIM_HALT);
        }
        rv_error = dmpdo_to_errno(rv);
        UNSERIALIZE(&dmp.dmp_lockword1);
        break;

    case DMPNOW_SEC: /* is used by sysdumpstart -s */
        SERIALIZE(&dmp.dmp_lockword2);
        if(isdmpfile(DMPD_SEC)) {
            rv = dmp_do(DMPD_SEC_HALT);
        } else {
            rv = dmpnow(DMPD_SEC_HALT);
        }
        rv_error = dmpdo_to_errno(rv);
        UNSERIALIZE(&dmp.dmp_lockword2);
        break;
    }
exit:
    if(rv_error)
        DMP_TRCHKL(DMPIOCTL,rv_error,mmdev << 16 | cmd);
    return(rv_error);
}
/********************************************************************
Set the secondary dump device
********************************************************************/
dmpset_sec(arg)
{

        /* the following group of declarations are for remote dump */
        struct sockaddr whereto;   /* temp structure for IP address */
        struct sockaddr_in *addr = (struct sockaddr_in *)&whereto;
        struct arptab *at;
        char   *next,*hostIP;
        int    pad,i,name_len;
        int    netflg = 0;
        chan_t channel;
        struct dmpio *dp;
        struct NFS_hdr *np;
        struct ifnet *ifp;
        struct in_ifaddr *if_addr;
        struct route      route;
        struct route     *ro;
        struct sockaddr_in *dst;
        char   hbuff[HOST_LEN];
        char   devname[32];
        char   fhbuff[FILEHNDL_LEN];
        char   *remotename = (char *)&hbuff;
        char   *filehandle = (char *)&fhbuff;
        /* the end for remote dump declarations                    */

        struct devinfo info;
        int rv_error;
        dev_t d_dev;
        int s;
        int rv;
        struct dumpinfo dumpinfo;


        rv_error = 0;

        if(copyin(arg,&dumpinfo,sizeof(dumpinfo))) 
            {
            rv_error = EFAULT;
            return(rv_error);
            }
        bzero( (char *)devname, 32);
        /* It is a remote dump, if dumpinfo->dm_devicename has the following   */
        /* format: <remotename>:<pathname>                  */

        if ( ( next = strchr((char *) arg,':')) != NULL )
            {
            netflg = 1;

            /* calculate ddd_namelen */
            ddd_namelen = next - (char *)arg;
            name_len = ddd_namelen;

            /* get the remote hostname */
            strncpy(remotename, (char *)arg, ddd_namelen);
               
            /* pad the remote name with zero so the name length 
               is a multiple of 4 
             */
            pad = ddd_namelen % 4;
            if  ( pad )
                {
                   pad = 4 - pad;
                bzero(remotename+ddd_namelen,pad);
                ddd_namelen += pad;
                } 
      
             /* Now we are going to get the address of the interface */
             /* Pseudo: zero out the route structure
                fill in the destination with remoteIP
                rtalloc kernel service is used to locate
                the required entry in the routine table.
                Examine the routine entry to see if the destination 
                is reachable, if GW is involved then save the 
                GW address for later use, save the IP address of
                the interface, address of the ifnet struct. 
             */ 
                              
             ro = &route;
             bzero( (char *)ro, sizeof(struct route) );
             dst = (struct sockaddr_in *)&ro->ro_dst;
             dst->sin_family = AF_INET;
             dst->sin_len = sizeof(* dst);
             dst->sin_addr.s_addr = dumpinfo.dm_hostIP;
             rtalloc(ro);
             if ( ro->ro_rt == 0 )
                {
                rv = ENETUNREACH;
                return(rv_error);
                }

             /* Get the IP of the network interface */
             if_addr = (struct in_ifaddr *)ro->ro_rt->rt_ifa;

             /* Get the address of the if structure of the network interface */
             ifp = ro->ro_rt->rt_ifp;

             /* If GW is used, save its IP address to use in if_output call */
             if ( ro->ro_rt->rt_flags & RTF_GATEWAY )
                 dst = (struct sockaddr_in *) ro->ro_rt->rt_gateway;

             /* Get the device name from the if structure. */        
             sprintf(devname,"%s%d",ifp->if_name,ifp->if_unit);    

            /* Allocate an instance of the network driver we want 
               to dump to.  If the driver is not already opened,
               this will open the driver, and allow us to access it.
               Save the old pointer in case the ns_alloc() fails. */
             if (rv_error = ns_alloc(devname, &sec_nddp))
                    return(rv_error);

             dumpinfo.dm_mmdev = 0;
             }
        else 
             bcopy(dumpinfo.dm_devicename,devname, 32);

                 
        SERIALIZE(&dmp.dmp_lockword2);
        d_dev = dumpinfo.dm_mmdev;
        /*  For local dump, dm_mmdev is already filled with valid 
         ** value by "sysdumpdev".
        */

        dp = typeto_dmpio(DMPD_SEC);
        if ( netflg )
            {
            /* mark the entry permanent
             */
            if ( rv_error = get_arp_entry(&at,dst) )
                {
		ns_free(sec_nddp);
                UNSERIALIZE(&dmp.dmp_lockword2);
                return(rv_error);
                }
            else
                  at->at_flags |= ATF_PERM;

            dp->dmp_netflg = 1;
            /* The following fields need to be set here because build_IP_header 
               ** use them (synchronization problem).
            */
            dp->dmp_idxn = ifp->if_hdrlen;
            dp->dmp_remoteIP.s_addr = dumpinfo.dm_hostIP;
            dp->dmp_localIP = if_addr->ia_addr.sin_addr;
            dp->dmp_ifp = ifp;
            /*  dmp_bufsize is the maximum size of the packet.
             **  The data part of the packet should be divisible
             **  by 4 for the fragment offset of the IP header.
             */

	    /* Make sure that the maximum packet size is not greater
	       than 4096.  */		
	    if (ifp->if_mtu > 4096)
		ifp->if_mtu = 4096;
	
            i = (ifp->if_mtu - (dp->dmp_idxn + sizeof(struct ip))) % 8;
            dp->dmp_bufsize = ifp->if_mtu - i;
            /* dst can either be the IP of the remote host or G/W's IP */
            bcopy(dst,&dp->dmp_dest,sizeof(struct sockaddr_in));
                          
            /* build the NFS header */
            np = typeto_NFS_hdr(DMPD_SEC);
            bcopy(&(dumpinfo.dm_filehandle),filehandle,FILEHNDL_LEN);
            set_NFS_hdr(np,remotename,ddd_namelen,filehandle,name_len);
            }
        else /* it's a local dump device */
            {
            /* Save the location code */
            bcopy(&(dumpinfo.dm_filehandle),dp->dmp_dumpinfo.dm_filehandle,
                FILEHNDL_LEN);
            dp->dmp_netflg = 0;
            }

        rv_error = dump_op(DUMPINIT2,DMPD_SEC,dumpinfo.dm_devicename,d_dev,0);
        if ((0 == rv_error) && (d_dev != null_mmdev) && (-1 == sec_dump_pid))
            {
            sec_dump_pid = creatp(); /* creating 2ndary dump process */
            if (((pid_t) -1 != sec_dump_pid) && 
               (-1 == initp(sec_dump_pid, dump_func, NULL, 0, "dump")))
                sec_dump_pid = -1;
            }

        UNSERIALIZE(&dmp.dmp_lockword2);
        return(rv_error);
}


/********************************************************************
Set the primary dump device
********************************************************************/

dmpset_prim(arg)
{

        /* the following group of declarations are for remote dump */
        struct sockaddr whereto;   /* temp structure for IP address */
        struct sockaddr_in *addr = (struct sockaddr_in *)&whereto;
        struct arptab *at;
        char   *next,*hostIP;
        int    pad,i,name_len;
        int    netflg = 0;
        chan_t channel;
        struct dmpio *dp;
        struct NFS_hdr *np;
        struct ifnet *ifp;
        struct in_ifaddr *if_addr;
        struct route      route;
        struct route     *ro;
        struct sockaddr_in *dst;
        char   hbuff[HOST_LEN];
        char   devname[32];
        char   fhbuff[FILEHNDL_LEN];
        char   *remotename = (char *)&hbuff;
        char   *filehandle = (char *)&fhbuff;
	struct ndd *old_dmp_nddp;
        /* the end for remote dump declarations                    */

        struct devinfo info;
        int rv_error;
        dev_t d_dev;
        int s;
        int rv;
        struct dumpinfo dumpinfo;


    	rv_error = 0;
        /* Save the argument from sysdumpdev command */
        if(copyin((char *)arg,(char *)&dumpinfo,sizeof(dumpinfo))) 
	    {
            rv_error = EFAULT;
            return(rv_error);
            }

        bzero( (char *)devname, 32);
        /* It is a remote dump, if dumpinfo->dm_devicename has the following   */
        /* format: <remotename>:<pathname>                  */

        if ( ( next = strchr((char *) arg,':')) != NULL )
            {
            netflg = 1;

            /* calculate ddd_namelen */
            ddd_namelen = next - (char *)arg;
            name_len = ddd_namelen;

            /* get the remote hostname */
            strncpy(remotename, (char *)arg, ddd_namelen);
               
            /* pad the remote name with zero so the name length 
               is a multiple of 4 
             */
            pad = ddd_namelen % 4;
            if  ( pad )
                {
                pad = 4 - pad;
                bzero(remotename+ddd_namelen,pad);
                ddd_namelen += pad;
                } 
      
            /* Now we are going to get the address of the interface */
            /* Pseudo: zero out the route structure
               fill in the destination with remoteIP
               rtalloc kernel service is used to locate
               the required entry in the routine table.
               Examine the routine entry to see if the destination 
               is reachable, if GW is involved then save the 
               GW address for later use, save the IP address of
               the interface, address of the ifnet struct. 
            */ 
                              
            ro = &route;
            bzero( (char *)ro, sizeof(struct route) );
            dst = (struct sockaddr_in *)&ro->ro_dst;
            dst->sin_family = AF_INET;
            dst->sin_len = sizeof(* dst);
            dst->sin_addr.s_addr = dumpinfo.dm_hostIP;
            rtalloc(ro);
            if ( ro->ro_rt == 0 )
                { 
                rv = ENETUNREACH;
                return(rv_error);
                }

            /* Get the IP of the network interface */
            if_addr = (struct in_ifaddr *)ro->ro_rt->rt_ifa;

            /* Get the address of the if structure of the network interface */
            ifp = ro->ro_rt->rt_ifp;

            /* If GW is used, save its IP address to use in if_output call */
            if ( ro->ro_rt->rt_flags & RTF_GATEWAY )
                dst = (struct sockaddr_in *) ro->ro_rt->rt_gateway;

            /* Get the name of the device from the if structure. */        
            sprintf(devname,"%s%d",ifp->if_name,ifp->if_unit);    
          
            /* Allocate an instance of the network driver we want 
               to dump to.  If the driver is not already opened,
               this will open the driver, and allow us to access it.
               Save the old pointer in case the ns_alloc() fails. */
	    old_dmp_nddp = prim_nddp;	
            if (rv_error = ns_alloc(devname, &prim_nddp))
		{
		prim_nddp = old_dmp_nddp;
                return(rv_error);        
		}
                
            dumpinfo.dm_mmdev = 0;
            }
        else 
	    bcopy(dumpinfo.dm_devicename,devname, 32);

                 
        SERIALIZE(&dmp.dmp_lockword1);
        dmp.dmp_oprimfp = dmp.dmp_primfp;
        d_dev = dumpinfo.dm_mmdev;
        /* for local dump, dm_mmdev is already filled with valid 
         ** value by "sysdumpdev".
         ** in case of remote dump, use fp_getdevno to obtain the 
         ** devno of the interface.
        */

        if (netflg)
            {
            /* mark the arp entry permanent */
            if ( (rv_error = get_arp_entry(&at,dst)) == 0 )
                at->at_flags |= ATF_PERM;
            else
                {
		ns_free(prim_nddp);
                UNSERIALIZE(&dmp.dmp_lockword1);
                return(rv_error);
                }
            dp = typeto_dmpio(DMPD_PRIM);
            /* dp->dmp_netflg = 1; */
            /* The following fields need to be set here because build_IP_header 
             ** use them (synchronization problem).
            */
            dp->dmp_idxn = ifp->if_hdrlen;
            dp->dmp_remoteIP.s_addr = dumpinfo.dm_hostIP;
            dp->dmp_localIP = if_addr->ia_addr.sin_addr;
            }
        else
            {    
            if(rv_error = fp_open(devname,O_RDWR,0,0,SYS_ADSPACE,&dmp.dmp_primfp))
                {
                dmp.dmp_primfp = dmp.dmp_oprimfp;
                UNSERIALIZE(&dmp.dmp_lockword1);
                return(rv_error);
                }
            }

        /* device is opened successfully */
        /* terminate the previous dump device */
        if(typetodev(DMPD_PRIM) >= 0)
            {
            dump_op(DUMPTERM,DMPD_PRIM,0,0,0);
            if(dmp.dmp_priopenflg) 
                {
                if (dmp.dmp_oprimfp)
                    fp_close(dmp.dmp_oprimfp);
                dmp.dmp_priopenflg = 0;
                }
            }
        /* initialize the new dump device which includes:
         **  - pin the dump support device driver part
         **  - query the device driver for max and min transmission units
         **    and dumpwrite function pointer in case of remote dump.
         **  - allocate the transmission buffer.
         **  - build the IP header in case of remote dump.
        */
	if (netflg)
		dp->dmp_netflg = 1;
        dp = typeto_dmpio(DMPD_PRIM);
        if (rv_error= dump_op(DUMPINIT,DMPD_PRIM,dumpinfo.dm_devicename,d_dev,0))
            {
            /* clean up before exit */
            if (netflg)
                {
                at->at_flags &= ~ATF_PERM;
                ns_free(prim_nddp);
                }
	    else
		fp_close(dmp.dmp_primfp);	
            dp->dmp_netflg = 0;
            UNSERIALIZE(&dmp.dmp_lockword1);
            return(rv_error);
            }
        else   /* everything is going well up till now.  For remote
                  dump save various pieces of data to use at dump time */
            {
            dmp.dmp_priopenflg++;
            if ( netflg )
                {
                dp->dmp_ifp = ifp;
                /*  dmp_bufsize is the maximum size of the packet.
                 **  The data part of the packet should be divisible
                 **  by 4 for the fragment offset of the IP header.
                */

		if (ifp->if_mtu > 4096)
			ifp->if_mtu = 4096;

                i = (ifp->if_mtu - (dp->dmp_idxn + sizeof(struct ip))) % 8;
                dp->dmp_bufsize = ifp->if_mtu - i;
                /* dst can either be the IP of the remote host or G/W's IP */
                bcopy(dst,&dp->dmp_dest,sizeof(struct sockaddr_in));
                       
                /* build the NFS header */
                np = typeto_NFS_hdr(DMPD_PRIM);
                bcopy(&(dumpinfo.dm_filehandle),filehandle,FILEHNDL_LEN);
                set_NFS_hdr(np,remotename,ddd_namelen,filehandle,name_len);
                dmp.dmp_primfp = NULL;    
                dmp.dmp_priopenflg = 0;
                }
            else
                {
                bcopy(&(dumpinfo.dm_filehandle),dp->dmp_dumpinfo.dm_filehandle,
                    FILEHNDL_LEN);
                dp->dmp_netflg = 0;
                }
            }

        UNSERIALIZE(&dmp.dmp_lockword1);
        return(rv_error);
}

dmpinit()
{
#ifdef _POWER_MP
        /* Register our dmpmpc() function to be called
           by each processor at dump time when a mpc_send()
           is issued. */
        dmp_mpc_msg_intiodone = mpc_register(INTIODONE, dmpmpc);
	dmp_mpc_msg_intmax = mpc_register(INTMAX, dmpmpc);
#endif /* _POWER_MP */

    if(dmp_nvread(&nv_dumpinfo) <= 0)
        bzero(&nv_dumpinfo,sizeof(nv_dumpinfo));
}

dmpopen(mmdev)
dev_t mmdev;
{
        int mdev;
        int rv_error;

        Debug("dmpopen(%x)\n",mmdev);
        mdev = minor(mmdev);
        rv_error = 0;
        switch(mdev) {
        case MDEV_DUMP:
        case MDEV_DUMPCTL:
        case MDEV_NULL:
        case MDEV_FILE:
                goto exit;
        default:
                rv_error = ENODEV;
        }
exit:
        if(rv_error)
                DMP_TRCHKL(DMPOPEN,rv_error,mmdev);
        return(rv_error);
}

dmpclose(mmdev)
dev_t mmdev;
{
        int mdev;

        mdev = minor(mmdev);
        switch(mdev) {
        case MDEV_NULL:
        case MDEV_FILE:
                return(0);
        default:
                return(0);
        }
}

iocinfo(arg)
{
    struct devinfo info;

    info.devtype           = DD_DUMP;
    info.flags             = 0;
    info.un.dump.primary   = typetodev(DMPD_PRIM);
    info.un.dump.secondary = typetodev(DMPD_SEC);
    info.un.dump.mdt_size  = dmp.dmp_count;
    if(copyout(&info,arg,sizeof(info)))
        return(EFAULT);
    return(0);
}

dmp_iocstat(arg)    /* get statistical data from NVRAM */
{
    if(copyout(&nv_dumpinfo,arg,sizeof(nv_dumpinfo)))
        return(EFAULT);

    /*  If the DMPFL_NEEDLOG bit is on, then the error hasn't
     ** been logged.  It is the responsibility of the calling
     ** program to log the error.  We go ahead and set this bit
     ** off, and then write it back to nvram.  The error logging
     ** is done by sysdumpdev, and since sysdumpdev gets called
     ** while the system is comming up, the dump will get logged.
    */
    if (nv_dumpinfo.dm_flags & DMPFL_NEEDLOG) {
        nv_dumpinfo.dm_flags &= ~DMPFL_NEEDLOG;
            dmp_nvwrite(&nv_dumpinfo);
            }
    return(0);
}

dmp_iocstat2(arg)
{

    /*  If the DMPFL_NEEDCOPY bit is on, then the dump hasn't
     ** been copied to the server.  Return with the statistical
     ** info of the new dump.  We go ahead and set this bit
     ** off, and then write it back to nvram. If "sysdumpdev -z"
     ** is invoked again, there will be nothing returned
     ** unless there is a new dump.
     */

    if (nv_dumpinfo.dm_flags & DMPFL_NEEDCOPY)
        {
            if (copyout(&nv_dumpinfo,arg,sizeof(nv_dumpinfo)))
                return(EFAULT);
            else
                {
                    nv_dumpinfo.dm_flags &= ~DMPFL_NEEDCOPY;
                    dmp_nvwrite(&nv_dumpinfo);
                    }
            }

    return(0);
}

dmp_iocinfo(arg)
{

    if(copyout(&nv_dumpinfo,arg,sizeof(nv_dumpinfo)))
        return(EFAULT);
    return(0);
}



dmp_size(arg)
{

        int rv_error;
        int dump_size;

    struct devinfo info;

    rv_error = 0;

    /* The size of vmm dump is 2.5% of the real memory */
        dump_size = (int)0.025 * (vmker.nrpages - vmker.badpages);
        dump_size = dump_size * PAGESIZE;
        /* Calculate the memory that's allocated to vmm segment.
           This segment contains the information which is used
           by Virtual Memory Manager.
        */
        dump_size +=  (vms_rusage(vmker.vmmsrval)) * PAGESIZE;

        /* Calculate the memory that's allocated to kernel ext. segment.
           This segment contains the kernel extension information.
        */
        dump_size +=  (vms_rusage(vmker.kexsrval)) * PAGESIZE;

        /* Calculate the memory that's allocated to kernel segment.
           This segment contains the kernel information.
        */
        dump_size +=  (vms_rusage(vmker.kernsrval)) * PAGESIZE;
        info.devtype           = DD_DUMP;
        info.flags             = 0;
        info.un.dump.mdt_size  = dump_size;
        if(copyout(&info,arg,sizeof(info)))
            rv_error = EFAULT;

    return(rv_error);
}
dmpdo_to_errno(rv)
{

        switch(rv) {
        case DMPDO_SUCCESS:  return(0);
        case DMPDO_PART:     return(EIO);
        case DMPDO_DISABLED: return(ENODEV);
        case DMPDO_FAIL:     return(ENOTREADY);
        }
        return(rv);
}
/*
 * Internal d_dump routine.
 * Called by dmp_op() through devdump()
 */
dmpdump(devno,uiop,op,arg,chan,ext)
dev_t devno;
{
        int rv,s;

        DMP_TRCHKL(DMPDUMP,op,devno);
        switch(minor(devno)) {
        case MDEV_NULL:
                return(dmpnull(devno,uiop,op,arg,chan,ext));
        case MDEV_FILE:
                return(dmpfile(devno,uiop,op,arg,chan,ext));
        default:
                return(0);
        }
}

#if 0
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
    int i;
    int rv_error;

        /* assert if this service is called with
           interrupt disabled
        */
        assert(csa->intpri == INTBASE);

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
    if((new_mdt = getmdt(dmp.dmp_count + DMP_GRW_SZ)) == 0) {
        rv_error = -2;
        goto exit;
    }
    bcopy(dmp.dmp_mdt,new_mdt,dmp.dmp_count * sizeof(struct mdt_entry));
    bzero(&new_mdt[dmp.dmp_count],DMP_GRW_SZ * sizeof(struct mdt_entry));
    if(dmp.dmp_mdt)
        freemdt(dmp.dmp_mdt);
    dmp.dmp_mdt = new_mdt;
    dmp.dmp_mdt[dmp.dmp_count].mdt_func = func;
    dmp.dmp_count += DMP_GRW_SZ;
    Debug("dmp_add: allocate. %x\n",new_mdt);
exit:
    UNSERIALIZE(&dmp.dmp_lockword);
    if(rv_error)
        DMP_TRCHK(DMPADDEXIT,rv_error);
    return(rv_error);
}

#endif

/*
 * NAME:     dmp_del
 * FUNCTION: delete 'func' from the cdt component dump table
 * INPUTS:   func   function pointer to caller-supplied routine which
 *                  was added by dmp_add.
 * RETURNS:  0   if deleted successfully
 *           -1  if function was not in the table
 *
 * Compaction is done on the table after deleteing. However, no attempt
 * is made to xfree unused space.
 */
dmp_del(func)
CDTFUNC func;
{
    int i;
    struct mdt_entry *mp, *dp;
    int rv_error;

        /* assert if this service is called with
           interrupt disabled
        */

        assert(csa->intpri == INTBASE);

    rv_error = 0;
    DMP_TRCHKL(DMPDEL,0,func);
    Debug("dmpdel(%x)\n",func);
    SERIALIZE(&dmp.dmp_lockword);

/*  locate the entry to be delete  */

    for(i = 0; i < dmp.dmp_count; i++) {
        mp = &dmp.dmp_mdt[i];
        if(mp->mdt_func == func)
            break;
    }

/*  if the entry is not found, return an error  */

    if(i == dmp.dmp_count) {
        rv_error = -1;
        goto exit;
    }

/*  otherwise, delete the entry and compress the table   */

    dp = &dmp.dmp_mdt[i]; 
    dp->mdt_func  = 0;                /* this covers the case   */
                           /* of the last element    */
                           /* of a full table delete */

    for( ; i < dmp.dmp_count-1; i++) {         /* compress the table to  */
        dmp.dmp_mdt[i] = dmp.dmp_mdt[i+1]; /* keep non-zero elements */
        dp = &dmp.dmp_mdt[i+1];
        if(dp->mdt_func == 0)           /* contiguous, making sure*/
            break;                     /* to zero each element   */
        dp->mdt_func = 0;                  /* after copying it so we */
        }                                  /* don't end up with dups */

exit:
    UNSERIALIZE(&dmp.dmp_lockword);
    if(rv_error)
      DMP_TRCHK(DMPDELEXIT,rv_error);
        
    return(rv_error);
}

/*
 * NAME:     dmp_prinit
 * FUNCTION: Initialize remote dump protocol
 * INPUTS: 
 *           dmp_proto   identifies the protocol
 *           proto_info  points to a protocol specfic structure 
 *                       containing information required by the system
 *                       dump service.
*/
void dmp_prinit(dmp_proto, proto_info)
int dmp_proto;
void *proto_info;
{
 
  /* assert if this service is called with
     interrupt disabled
  */
  assert(csa->intpri == INTBASE);
  SERIALIZE(&dmp.dmp_lockword);
  switch (dmp_proto) {
    case AF_INET:
         ddd_arptab = (struct arptab *) proto_info;
         break;
    default:
         break;
  }
  UNSERIALIZE(&dmp.dmp_lockword);
}

#if 0
/*
 * NAME:     getmdt
 * FUNCTION: allocated master dump table (mdt) entries
 * INPUTS:   n      number of mdt entries to allocate
 * RETURNS:  pointer to xmallo-ed buffer
 */
/* extern */ struct mdt_entry *getmdt(n)
{
        struct mdt_entry *mp;

        mp = (struct mdt_entry *)jmalloc(n*sizeof(struct mdt_entry));
        return(mp);
}

/*
 * NAME:     freemdt
 * FUNCTION: free previously allocated master dump table (mdt) entries
 * INPUTS:   pointer to xmallo-ed buffer
 * RETURNS:  none
 */
freemdt(mp)
struct mdt_entry *mp;
{

        jfree(mp);
}

#endif

/*
 * dmpfile has the same interface as a devsw.d_dump routine,
 * but outputs to a file "dump" instead of a hardware device
 * by using the fp_write kernel routine.
 */
/*ARGSUSED*/
dmpfile(devno,uiop,op,arg,chan,ext)
dev_t devno;
struct uio *uiop;
{
    static struct file *fp;
    char *base;
    int count;
    int rv_count;
    int rv;

    DMP_TRCHK(DMPFILE,op);
    switch(op) {
    case DUMPINIT:
        return(0);
    case DUMPTERM:
        return(0);
    case DUMPSTART:
        if(fp)
            return(EBUSY);
        rv = fp_open("/tmp/dump",O_CREAT|O_TRUNC|O_WRONLY,0644,0,SYS_ADSPACE,&fp);
        if(rv)
            return(rv);
        return(0);
    case DUMPEND:
        if(fp) {
            fp_close(fp);
            fp = 0;
        }
        return(0);
    case DUMPQUERY:
      {
        struct dmp_query *qp;

        if(arg == 0)
            return(EINVAL);
        qp = (struct dmp_query *)arg;
        qp->min_tsize = 8*512;
        qp->max_tsize = 36*512;
        return(0);
      }
    case DUMPWRITE:
        if(fp == 0)
            return(EEXIST);
        count = uiop->uio_resid;
        base  = uiop->uio_iov->iov_base;
        Debug("dmpfile: calling fp_write(%x,%x,%x)\n",fp,base,count);
        rv = fp_write(fp,base,count,ext,UIO_SYSSPACE,&rv_count);
        if(rv || count != rv_count)
            Jprintf("fp_write(%x,%x,%x) rv=%d %d\n",fp,base,count,rv,rv_count);
        if(rv)
            return(rv);
        uiop->uio_resid         -= rv_count;
        uiop->uio_iov->iov_base += rv_count;
        return(0);
    default:
        return(EIO);
    }
}
#if 0

/*
 * NAME:     jmalloc
 * FUNCTION: interface to xmalloc() to allocate from pinned_heap
 * INPUT:    number of bytes to allocate
 * RETURNS:  pointer to pinned allocated buffer
 */
char *jmalloc(n)
{

    char *ptr;

    Debug("jmalloc(%x)\n",n);
    ptr = xmalloc(n,2,pinned_heap);
    if(ptr == 0)
        Jprintf("dumpdd: error on xmalloc of %d bytes\n",n);
    return(ptr);
}

/*
 * NAME:     jfree
 * FUNCTION: interface to xmfree()
 * INPUT:    pointer returned by jmalloc
 * RETURNS:  none
 */
jfree(ptr)
char *ptr;
{

    Debug("jfree(%x)\n",ptr);
    xmfree(ptr,pinned_heap);
}
#endif

dmp_nvread(infp)
struct dumpinfo *infp;
{
        int rv;
        int size;
        static printflg;

        size = sizeof(*infp);
        rv = nvread(DUMP_NVBLOCK,(uchar *)infp,0,size);
        if(rv < 0) {
                return(-1);
        }
        if(rv != size) {
                return(0);
        }
        return(1);
}


ras_privchk()
{

    if(ras_privflg == 0 || privcheck(RAS_CONFIG) == 0)
        return(0);
    return(-1);
}

/*  Pseudo: the real NFS write request is a lot more generic and 
            complicated than what I need.  In order to accomplish what
            are needed for remote dump, the simple structure is defined    
            with all the needed fields for an NFS write request.
            Most of the fields are static.  This header will be filled
            in at DUMPINIT time.
*/

set_NFS_hdr(NFS_hdr,remotename,ddd_namelen, filehandle,name_len)
struct NFS_hdr *NFS_hdr;
char *remotename;
int ddd_namelen;
char *filehandle;
int name_len;
{
    NFS_hdr->xid = 0;
    NFS_hdr->direction = CALL;
    NFS_hdr->rpc_version = RPC_MSG_VERSION;
    NFS_hdr->program = NFS_PROGRAM;
    NFS_hdr->version = NFS_VERSION;
    NFS_hdr->proc_num = RFS_WRITE;
    NFS_hdr->verf = AUTH_UNIX;
    NFS_hdr->uid = 0;
    NFS_hdr->gid = 0;
    NFS_hdr->gidlen = 1;
    NFS_hdr->glist = 0;
    NFS_hdr->verf_flavor = AUTH_NULL;
    /* credsize = sizeof time +  sizeof hostnamelen + hostnamelen +
           + sizeof uid + sizeof gid + sizeof glist + sizeof gidinlist
    */
    NFS_hdr->cred_size = 4 + 4 + ddd_namelen + 4 + 4 + 4 + 4;
    NFS_hdr->verf_len = 0;
    NFS_hdr->hostnamelen = name_len;
    NFS_hdr->hostname = jmalloc(ddd_namelen);
    NFS_hdr->filehandle = jmalloc(FILEHNDL_LEN);
    bcopy(remotename,NFS_hdr->hostname, ddd_namelen);
    bcopy(filehandle,NFS_hdr->filehandle, FILEHNDL_LEN);
    NFS_hdr->begoff = 0;
    NFS_hdr->totcnt = 0;
}



build_IP_header(dp)
struct dmpio *dp;
{

    struct ip *ip;
    struct ether_header *eth;

    dp->dmp_idx = dp->dmp_idxn;
    ip = (struct ip *)(dp->dmp_buf + dp->dmp_idx);
    ip->ip_hl = sizeof(struct ip) >> 2;  /* see NFS code */
    ip->ip_v = IPVERSION;
    ip->ip_tos = 0;
    ip->ip_len = 0;
    ip->ip_id = 0;
    ip->ip_off = 0;
    ip->ip_ttl = UDP_TTL;
    ip->ip_p = IPPROTO_UDP;
    ip->ip_sum = 0;
    ip->ip_src.s_addr = dp->dmp_localIP.s_addr;
    ip->ip_dst.s_addr = dp->dmp_remoteIP.s_addr;
    dp->dmp_idx += sizeof(struct ip);
}

int get_arp_entry(at,dst)
struct arptab **at;
struct sockaddr_in *dst;

{
    int n;

    if ( ddd_arptab == NULL )
        {
            errno = EAGAIN;
             return(EAGAIN);
          }
    *at = &ddd_arptab[ARPTAB_HASH(dst->sin_addr.s_addr) * arptabbsiz];
    for ( n = 0; n < arptabbsiz; n++ , (*at)++ )
        {
             if ((*at)->at_iaddr.s_addr == dst->sin_addr.s_addr )
                   break;
        }
 
    if ( n >= arptabbsiz )
        {
             errno = EAGAIN;
             return(EAGAIN);
          }
      else return(0);
}

void   ddd_freem(m)
struct mbuf *m;
{
    m->m_type = MT_DATA;
}

