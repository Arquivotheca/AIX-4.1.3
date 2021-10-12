static char sccsid[] = "@(#)09	1.40.2.25  src/bos/kernel/io/machdd/md.c, machdd, bos41J 6/2/95 09:51:13";
/*
 * COMPONENT_NAME: (MACHDD) Machine Device Driver
 *
 * FUNCTIONS: mdopen, mdclose, mdioctl, mdread, mdmpx,
 *            md_check_buid
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */


#ifndef _MD_C
#   define _MD_C
#endif
				/* Includes */
#include <sys/file.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/adspace.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/iplcb.h>
#include <sys/vmker.h>
#include <sys/priv.h>
#include <sys/except.h>
#include <sys/iocc.h>
#include <sys/mdio.h>
#include <sys/systemcfg.h>
#include <sys/sys_resource.h>
#include <sys/rtc.h>
#include <sys/user.h>
#include <errno.h>
#include <sys/nvdd.h>
#include "md_extern.h"
#include "md_nvram.h"

			/* Defines */
#define IOCC	0x00400000
#define IOCC2	0x00010000

#define USERMASK   0x00040060

Simple_lock md_lock;

#define BASECUST -3

extern ulong NVRAM_size;
extern ulong NVRAM_base;
extern HEADER header;

char *gea_buf;
int gea_used;
int gea_thresh;
int gea_length;

caddr_t BaseCustom;
long Base_size;

extern struct md_bus_info md_bus[];
ulong md_valid_buid[MD_MAX_BUID];	/* I/O BUID array */

extern int enter_dbg;                   /* debugger enable flag */


/*
 * NAME: mdopen
 *
 * FUNCTION:  Device open routine - check privilege
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES:
 *    devno : major and minor device numbers
 *   rwflag : FREAD -or- FWRITE
 *            FREAD  = open file for reading
 *            FWRITE = read/write
 *     chan : channel number
 *      ext : extended system call param (NOT USED)
 *
 * RETURNS:
 *        0 : successful open()
 *    EPERM : insufficient privilege
 *
 */
int
mdopen(dev_t devno, ulong rwflag, int chan, int ext)
{

    if (!priv_chk(DEV_CONFIG)) {
	/* not super-user and does not have DEV_CONFIG privlege */
	return EPERM;
    }

    return 0;
} /* mdopen() */


/* 
 * NAME: mdclose
 *
 * FUNCTION:  Device close routine
 *            Free BaseCustom buffer if chan=BASECUST
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES:
 *    devno : major and minor device numbers
 *     chan : channel number
 *      ext : extended system call param (NOT USED)
 * 
 * RETURNS
 *
 *    always returns 0
 *
 */
int
mdclose(dev_t devno, int chan, int ext)
{
    if (chan == BASECUST) {
	free(BaseCustom);	/* Free this buffer at first close */
	BaseCustom = 0;		/* Opens will fail now */
    }
    return 0;
} /* mdclose() */


/*
 * NAME: mdioctl
 *
 * FUNCTION:  Device ioctl routine
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES:
 *
 *    devno : major and minor device numbers
 *       op : operation code
 *      arg : address of parameter block
 *     flag : file parameter word 
 *            DKERNEL = called from KERNEL
 *            DREAD   = open for reading
 *            DWRITE  = open for writing
 *            DAPPEND = open for appending
 *     chan : the read/write character offset 
 *      ext : extended system call param: used to specify BUID
 *
 * RETURNS:
 *
 *   EFAULT : cannot transfer data between KERNEL mem and USER mem
 *   ENOMEM : no KERNEL memory to allocate temp storage
 *   EACCES : SECURITY VIOLATION
 *
 */
int
mdioctl(dev_t devno, int op, int arg, ulong flag, int chan, int ext)
{
    int fBus;
    int fkern;
    ulong ioaddr;
    struct devinfo dinfo;
    MACH_DD_IO ioarg;
    char *iodata;
    char *old_ad;
    ulong iosize;
    int rc = 0;
    ulong psr;
    ulong buid;
    uchar b;
    uint w;
    caddr_t addr;
    ulong md_disable_pipe();
    ulong md_arbcntl_rw();
    struct buc_info *md_select_slot();
    int busnum;		/* index into md_bus from minor num */
    int region;		/* region of bus */
#ifdef _RS6K_SMP_MCA
    uint length;
    char vpd_desc[2];
#endif /* _RS6K_SMP_MCA */

#ifdef _RSPC
    int string_len;
    int gea_string_len;
    int search_len;
    int gea_avail;
    char *found;
    char *tiodata;
    char *tmd_addr;
    char *gea_buf_ptr;
    int gea_used_org;
    struct gea_attrib geastuff;
#endif /* _RSPC */

    MD_LOCK();
    fkern = flag & DKERNEL;
    fBus = minor(devno);
        /* NVRAM and BUS DEVICES */
            /* minor(devno) == 0   -> dev/nvram & /dev/bus0 DEFAULT BUS */
            /* minor(devno) == 16  -> dev/bus1 */
            /* minor(devno) == 32  -> dev/bus2 */
            /* minor(devno) == 48  -> dev/bus3 */
	    /* etc... */
    busnum = fBus >> 4;
    if (busnum > MD_MAX_BUS) {
	MD_UNLOCK();
	return EINVAL;
    }

    if (ext) {       /* ioctlx can specify arbitrary buid */
	if (md_check_buid(ext)) { /* Make sure buid is valid */
		MD_UNLOCK();
		return EINVAL;
	}
	if ((op == MIOBUSGET) || (op == MIOBUSPUT)) {
		md_bus[MD_EXT_IDX].bid[IO_RGN] = ext ;
		md_bus[MD_EXT_IDX].map = MAP_T1;
		md_bus[MD_EXT_IDX].io_excpt = TRUE;
		busnum = MD_EXT_IDX;
	} else {
		MD_UNLOCK();
		return EINVAL;
	}
    }

    switch (op) {
    case IOCINFO:  
    case MIONVLED:
    case MIONVCHCK:
#ifdef _RSPC
    case MIOGEAST:
#endif /* _RSPC */
	break;

#ifdef _RS6K_SMP_MCA

	/* new ioctls for PEGASUS */

    case MIOKEYCONNECT:
    case MIOKEYDISCONNECT:
	if(!__rs6k_smp_mca()) {
		MD_UNLOCK();
		return EINVAL;
	}
	break;

    case MIOSETKEY:
    case MIONVSTRLED:
    case MEEPROMGET:
    case MEEVPDGET:
    case MEEVPDPUT:
    case MFEPROMPUT:
    case MPOWERGET:
    case MPOWERSET:
    case MRDSSET:
    case MRDSGET:
    case MSURVSET:
    case MSURVRESET:
    case MDINFOGET:
    case MDINFOSET:
#ifdef _POWER_MP
    case MCPUSET:
    case MCPUGET:
#endif /* _POWER_MP */
	    if(!__rs6k_smp_mca()) {
		    MD_UNLOCK();
		    return EINVAL;
	    }
	    /* fall through */
#endif /* _RS6K_SMP_MCA */

    case MIONVGET:
    case MIONVPUT:
    case MIOBUSGET:
    case MIOBUSPUT:
    case MIOCCGET:
    case MIOCCPUT:
#ifdef _POWER_RS
    case MSLAGET:
    case MSLAPUT:
#endif	/* _POWER_RS */
    case MIOIPLCB:
    case MIOAIPLCB:
    case MIOGETPS:
    case MIOGETKEY:
    case MIOTODGET:
    case MIOTODPUT:
    case MIOVPDGET:
    case MIOCFGGET:
    case MIOCFGPUT:
    case MIORESET:
#ifdef _RSPC_UP_PCI
    case MIOPCFGET:
    case MIOPCFPUT:
    case MIOMEMGET:
    case MIOMEMPUT:
#endif	/* _RSPC_UP_PCI */
#ifdef HTXLVL
    case MIOBUSMEM:
    case MIOBUSIO:
#endif
/*
 *	The following three options are used
 *	for reading and updating the Global
 *	Environment Area.
 */
#ifdef _RSPC
    case MIOGEARD:
    case MIOGEARDA:
    case MIOGEAUPD:
#endif  /* _RSPC */

        if (!fkern) {
	    if (copyin(arg, &ioarg, sizeof(MACH_DD_IO))) {
		MD_UNLOCK();
		return EFAULT;
	    }
        } else
	    bcopy(arg, &ioarg, sizeof(MACH_DD_IO));

	switch(ioarg.md_incr) {
	    case MV_BYTE:  iosize = ioarg.md_size;
			   break;
	    case MV_SHORT: iosize = ioarg.md_size * sizeof(short);
			   break;
	    default:
	    case MV_WORD:  iosize = ioarg.md_size * sizeof(int);
			   break;
	}
        break;
    default :
	MD_UNLOCK();
        return EINVAL;
    }
    if ((MIOGEARD == op) || (MIOGEAUPD == op) || (MIOGEARDA == op) || 
       (MIOGEAST == op)) 
    {
	if (__rspc())
        {
                if ((MIOGEAUPD == op) && !(flag & DWRITE))
                {
                     MD_UNLOCK();
                     return EACCES;
                }
		iosize = ioarg.md_size;
        }
	else
	{
		MD_UNLOCK();
		return EINVAL;
	}
    }
	
#ifdef _POWER_RS
    if ((MSLAGET == op) || (MSLAPUT == op)) {
	switch (ioarg.md_buid) {
	case MSLA0:
	case MSLA1:
	case MSLA2:
	case MSLA3:
	case MSLA4:
	case MSLA5:
	case MSLA6:
	case MSLA7:
	    buid = (ioarg.md_buid << 20)|BUID_T;
	    if (md_check_buid(buid)) { 	/* Make sure buid is valid */
		MD_UNLOCK();
		return EINVAL;
	    }
	    md_bus[MD_EXT_IDX].bid[IO_RGN] = buid;
	    md_bus[MD_EXT_IDX].map = MAP_T1;
	    md_bus[MD_EXT_IDX].io_excpt = TRUE;
	    busnum = MD_EXT_IDX;
	    break;
	default:
	    MD_UNLOCK();
	    return EINVAL;
	}
    }
#endif	/* _POWER_RS */

#ifdef _RS6K_UP_MCA
    switch(op) {	/* platform check */
    case MIOVPDGET:	/* these only supported for PowerPC */
    case MIOCFGGET:
    case MIOCFGPUT:
    case MIORESET:
	if (__rs6k_up_mca()) {	
	    if (md_slot_bucp(ioarg.md_sla))	/* valid slot? */
	        break;
	}
	MD_UNLOCK();
 	return EINVAL;
    }
#endif	/* _RS6K_UP_MCA */

    switch (op) {

    /* these are default codes for every device */
    case IOCINFO    : /* return struct that describes device */
        dinfo.devtype = DD_PSEU;
        dinfo.flags = 0;                    /* correct number of flags ????? */
        if (!fkern) {
	    if (copyout(&dinfo, arg, sizeof(struct devinfo)))
		rc = EFAULT;
	} else
	    bcopy(&dinfo, arg, sizeof(struct devinfo));
        break;
    case MIONVGET   : /* MIONVGET   addr, size, data(r), incr */
        ioaddr = NVRAM_base + (ioarg.md_addr & NVRAMMASK);
	if ((ioaddr + iosize) > (NVRAM_base + NVRAM_size)) {
		rc = EINVAL;
		break;
	}
	if (NULL == (iodata =
	    mdget(MD_NVRAM_IDX, IOCC_RGN, ioaddr, iosize, 0, 
              ioarg.md_incr, &rc))) {
		rc = ENOMEM;
		break;
	} 
	if (!fkern) {
	    if (copyout(iodata, ioarg.md_data, iosize))
	        rc = EFAULT;
	} else
	    bcopy(iodata, ioarg.md_data, iosize);
	MD_PG_FREE(iodata);
        break;
    case MIONVPUT   : /* MIONVPUT   addr, size, data(p), incr */
        ioaddr = NVRAM_base + (ioarg.md_addr & NVRAMMASK);
	if ((ioaddr + iosize) > (NVRAM_base + NVRAM_size)) {
		rc = EINVAL;
		break;
	}
	if (NULL == (iodata = MD_PG_ALLOC(iosize))) {
	    rc = ENOMEM;
	    break;
	}
        if (!fkern) {
	    if (copyin(ioarg.md_data, iodata, iosize)) {
		MD_PG_FREE(iodata);
		rc = EFAULT;
		break;
	    }
        } else
	    bcopy(ioarg.md_data, iodata, iosize);
	mdput(MD_NVRAM_IDX, IOCC_RGN, ioaddr, iosize, iodata, MV_BYTE, &rc);
	MD_PG_FREE(iodata);

	if (ioaddr < (NVRAM_base + LED_ADDRESS)) {	/* ROS AREA MODIFIED */
	    struct ros_cb *nv_ros;
	    nv_ros = (struct ros_cb *)mdget(MD_NVRAM_IDX, IOCC_RGN, NVRAM_base, 
		sizeof(struct ros_cb), 0, MV_BYTE, &rc);
	    if (nv_ros && crc32(nv_ros, sizeof(struct ros_cb))) {
		nv_ros->ros_crc = crc32(nv_ros, 
			sizeof(struct ros_cb)-sizeof(ulong));
		mdput(MD_NVRAM_IDX, IOCC_RGN, NVRAM_base, sizeof(struct ros_cb),
			nv_ros, MV_BYTE, &rc);
	    }
	    MD_PG_FREE(nv_ros);
	}
        break;
#ifdef _RSPC
    case MIOGEARD   : /* MIOGEARD   addr, size, data, length */
/*
 *	This option is used to get the string following
 *	a Global Environment Area variable returned.
 *	An example is the bootpath variable.  This variable
 *	is used by ROS as the search order for boot devices.
 *	The format of an entry is:
 *		variablename=<string>, where
 *	string is a character string with each entry delimited
 *	by colons (:).  The MIOGEARD option expects the md_addr
 *	to point to a null-terminated string which is the variable
 *	name to be searched for.  The last non-null  character in the 
 *	string must be an equal sign (=).  md_data points to a buffer
 *	to return the entry in the GEArea.  If the md_length
 *	input is non-zero, the length of the returned string
 *	will be placed in that target.
 */
 
        /*
         *   Allocate space for maximum length string.
         */
        if (NULL == (iodata = MD_PG_ALLOC(gea_length))) {
	    rc = ENOMEM;
	    break;
	}
        /*
         *   Copy the input string into the allocated buffer
         */
        if (!fkern)
        {
	    if (copyinstr(ioarg.md_addr, iodata, gea_length, &string_len))
            {
	        MD_PG_FREE(iodata);
	    	rc = EFAULT;
            	break;
	    }
        } 
        else /* if (!fkern) */
        {
            tmd_addr = (uchar *) ioarg.md_addr;
            for (tiodata = iodata; ((tiodata-iodata) < gea_length) &&
               (*tmd_addr != 0x0); tiodata++)
	    {
		*tiodata = *tmd_addr;
		tmd_addr++;
	    } /* for (tiodata = iodata; ((tiodata-iodata) ... */
            string_len = tiodata - iodata + 1; /* fudge for common code */
	    if (string_len != gea_length)
            {
	        *tiodata = 0;
                tiodata++;
            } /* if (string_len != ... */
	} /* if (!fkern) */    
        /*
         *   Insure that the input string is valid; i.e., null-
         *   terminated with '=' as last non-null character.
         */
	string_len--; /* adjust so that null is not included */
        tiodata = iodata + string_len;
        tmd_addr = tiodata - 1;
	if ((*tiodata == 0x0) && (*tmd_addr == '='))
	{
            /*
             *   If here, valid input, loop thru GEArea looking for match.
             */
            found = 0;	/* used as a flag to indicate hit */
	    for (gea_buf_ptr = gea_buf; 
               gea_buf_ptr < (gea_buf + gea_used - string_len + 1);
               gea_buf_ptr++)
	    {
                if ((*iodata == '*') || 
                    (!strncmp(iodata, gea_buf_ptr, string_len)))
		{
                    /*
                     *   If here, we have a hit.  Determine the length
                     *   of the string in the GEArea and move it to
                     *   the user's buffer.
                     */
		    found = gea_buf_ptr;
                    if (*iodata == '*')
                        string_len = gea_used;  /* wild card for all used */
                    else /* if (*iodata == '*') */
                    {
		        while (*gea_buf_ptr != 0)
		            gea_buf_ptr++;
                        string_len = gea_buf_ptr - found + 1;
                    } /* if (*iodata == '*') */
		    if((string_len) <= ioarg.md_size)
		    {
        	        if (!fkern) 
			{
			    if (copyout(found, ioarg.md_data, string_len)) 
			        rc = EFAULT;
			} /* if (!fkern) */
		        else /* if (!fkern) */
			    bcopy(found, ioarg.md_data, string_len);
		    }
		    else /* if(string_len =< ioarg.md_size) */
                        /*
                         *   If here, not enough memory in user's 
                         *   buffer.  Return ENOMEM.
                         */
		        rc = ENOMEM;
                    /*
                     *   Set to exit for loop.
                     */
		    gea_buf_ptr = gea_buf + gea_used;
                    /*
                     *    Must update the md_length if present.
                     */
                    if (ioarg.md_length) 
                    {
                        if (!fkern) 
                        {
                            if (copyout(&string_len, ioarg.md_length, 
                               sizeof(int)))
                                rc = EFAULT;
                        } /* if (!fkern) */
                        else /* if (!fkern) */
                            bcopy(&string_len, ioarg.md_length, sizeof(int)); 
                    } /* if (ioarg.md_length) */

                } /* if (!strncmp(iodata, gea_buf ... */
                else
                /*
                 *	If here, string did not compare; must
                 *	step to next variable in gea for next
                 *	compare.
                 */
                {
                    while (*gea_buf_ptr != 0)
                        gea_buf_ptr++;
                } /* if (!strncmp(iodata, gea_buf ... */ 

            } /* for (gea_buf_ptr = gea_buf; gea_buf_pt ... */
            if (!found)
                rc = EINVAL;
        }
        else /* if ((*tiodata == 0x0) && (*tmd_addr == '=')) */
	    rc = EINVAL;
					
	MD_PG_FREE(iodata);
        break;
    case MIOGEAUPD   : /* MIOGEAUPD   addr, size, data, length */
/*
 *	The MIOGEAUPD option is used to update the GEArea of
 *	NVRAM.  md_addr points to the null-terminated string 
 *	which is of the form: 
 *		variablename=<string> where,
 *	variablename is an ASCII string to be changed, added, or
 *	deleted to/from NVRAM GEArea.  The string is composed
 *	of logical entities delimited via colons (:).  If 
 *	string does not exist, this implies that the 
 *	specified variablename is to be deleted.  If the 
 *	variablename is not found in the GEArea, the input
 *	will be added to the end.  If it is found, it will
 *	replace the existing string in its existing position
 *	with adjustments of any data following that entry.
 */

        /*
         *   Allocate space for maximum length string.
         */
        if (NULL == (iodata = MD_PG_ALLOC(gea_length))) 
        {
	    rc = ENOMEM;
	    break;
	}
        /*
         *   Copy the input string into the allocated buffer
         */
        if (!fkern)
        {
	    if (copyinstr(ioarg.md_addr, iodata, gea_length, &string_len))
            {
	        MD_PG_FREE(iodata);
	    	rc = EFAULT;
            	break;
	    }
        } 
        else /* if (!fkern) */
        {
            tmd_addr = (uchar *) ioarg.md_addr;
            for (tiodata = iodata; ((tiodata-iodata) < gea_length) &&
               (*tmd_addr != 0x0); tiodata++)
	    {
		*tiodata = *tmd_addr;
		tmd_addr++;
	    } /* for (tiodata = iodata; ((tiodata-iodata) ... */
            string_len = tiodata - iodata + 1; /* fudge for common code */
	    if (string_len != gea_length)
            {
	        *tiodata = 0;
                tiodata++;
            } /* if (string_len != ... */
	} /* if (!fkern) */    
        /*
         *	Now must locate the equal sign.
         */
        for (tiodata = iodata; (((tiodata-iodata) < string_len) &&
           (*tiodata != '=')); tiodata++);
        if (*tiodata != '=')
        {
            MD_PG_FREE(iodata);
            rc = EINVAL;
            break;
        }
        search_len = tiodata - iodata + 1;
        /*
         *	If here, loop thru GEArea looking for match.
         */
        found = 0;	/* used as a flag to indicate hit */
        for (gea_buf_ptr = gea_buf; 
           gea_buf_ptr < (gea_buf + gea_used - search_len + 1); gea_buf_ptr++)
        {
            if (!strncmp(iodata, gea_buf_ptr, search_len))
            {
                /*
                 *   If here, we have a hit.  Determine the length
                 *   of the string in the GEArea.
                 */
                found = gea_buf_ptr;
                while (*gea_buf_ptr != 0)
                    gea_buf_ptr++;
                gea_string_len = gea_buf_ptr - found + 1;
                /*
                 *   Set to exit for loop.
                 */
                gea_buf_ptr = gea_buf + gea_used;

            } /* if (!strncmp(iodata, gea_buf ... */
            else
            /*
             *	If here, string did not compare; must
             *	step to next variable in gea for next
             *	compare.
             */
            {
                while (*gea_buf_ptr != 0)
                    gea_buf_ptr++;
            } /* if (!strncmp(iodata, gea_buf ... */ 

        } /* for (gea_buf_ptr = gea_buf; gea_buf_pt ... */
        /*
         *	If here, GEArea has been searched for match.
         *	If match occurred, determine if update or
         *	delete.  If match not found and delete not
         *	requested, add specified string to end of
         *	if room permits.
         */
        gea_used_org = gea_used;	/* remember to reduce amount to move */
        if (found)
        {
            if ((string_len - search_len) == 1) 
            {
                /*
                 *	If here, this is a delete request.  If
                 *	the entry to be deleted is the last in
                 *	the GEArea, only need to update gea_used
                 *	and zero the entry.
                 */
                if ((found - gea_buf) + gea_string_len == gea_used)
                    bzero(found, gea_string_len);
                else /* if ((found - gea_buf) + gea_st ... */
                {
                    bcopy((found + gea_string_len), found,
                       gea_used - (found - gea_buf + gea_string_len));
                    bzero(gea_buf+gea_used-gea_string_len, gea_string_len);
                } /* if ((found - gea_buf) + gea_st ... */
                gea_used = gea_used - gea_string_len; /* update used */
            }
            else /* if ((string_len - search_len) == 1) */
            {
                /*
                 *	If here, this is an update request.
                 *	Determine, if this is the last entry.
                 *	If so, and there is sufficient room
                 *	in the GEArea, move the update in.
                 *	If not the last entry, must put in
                 *	place after adjusting following data
                 *	to accomodate the length of the new
                 *	string.
                 */
                if ((found - gea_buf) + gea_string_len < gea_used)
                {
                    /*
                     *	If here, the string to be replaced
                     *	is not the last string in the GEA.
                     */
                    if (string_len > gea_string_len)
                    {
                        if (gea_used + (string_len - gea_string_len)
                            <= (gea_length - gea_thresh))
                        {
                            bcopy(found + gea_string_len,
                                  found + string_len,
                                  (gea_used + gea_buf) - (found + gea_string_len));
                        }
                        else /* if (gea_used + (string_len ... */
                        {
                            MD_PG_FREE(iodata);
                            rc = ENOMEM;
                            break;
                        } /* if (gea_used + (string_len ... */
                    }
                    else if (string_len < gea_string_len)
                    {
                        bcopy (found + gea_string_len,
                               found + string_len,
                               (gea_used + gea_buf) - (found + gea_string_len));
                        bzero(gea_buf+gea_used-(gea_string_len-string_len),
                              gea_string_len - string_len);
                    } 
                }
                else /* if ((found - gea_buf) + gea_string_len < ... */
                {
                    /*
                     *	If here, this is replacing the last
                     *	entry in the GEA.
                     */
                    if (string_len > gea_string_len)
                    {
                        if (gea_used + (string_len - gea_string_len)
                            > (gea_length - gea_thresh))
                        {
                            MD_PG_FREE(iodata);
                            rc = ENOMEM;
                            break;
                        } /* if (gea_used + (string_len ... */
                    }
                    else if (string_len < gea_string_len)
                        bzero(found + string_len, gea_string_len - string_len);

                } /* if ((found - gea_buf) + gea_string_len != ... */

                gea_used = gea_used + (string_len - gea_string_len);
                bcopy (iodata, found, string_len);

            } /* if ((string_len - search_len) == 1) */ 
        }
        else /* if (found) */
        {
            /*
             *	If here, name not found in GEA.  Must add to the
             *	end if there is sufficient room.  If this is a
             *	delete request, return success.
             */
            if ((string_len - search_len) != 1)
            {
                /*
                 *      If here, this is not a delete request.
                 */
                if ((gea_used + string_len) <= (gea_length - gea_thresh))
                {
                    bcopy (iodata, gea_buf + gea_used, string_len);
                    gea_used = gea_used + string_len;
                }
                else /* else ((gea_used + string_len) <= (gea_leng ... */
		{
                            MD_PG_FREE(iodata);
                            rc = ENOMEM;
                            break;
                } /* endif ((gea_used + string_len) <= (gea_leng ... */
            } /* if ((string_len - search_len) != 1) */
        } /* if (found) */
        /*
         *    Must update the md_length if present.
         */
        if (ioarg.md_length) 
        {
            gea_avail = gea_length - gea_used - gea_thresh;
            if (!fkern) 
            {
                if (copyout(&gea_avail, ioarg.md_length, sizeof(int)))
                    rc = EFAULT;
            } /* if (!fkern) */
            else /* if (!fkern) */
                bcopy(&gea_avail, ioarg.md_length, sizeof(int)); 
        } /* if (ioarg.md_length) */

        if (!found)
        {
            gea_string_len = 0;
            found = gea_buf + gea_used_org;
        }
        if (gea_used > gea_used_org)
            gen_crc1((char *) &header.GELastWriteDT, found, 
                gea_used - (found - gea_buf));
        else if (gea_used < gea_used_org)
            gen_crc1((char *) &header.GELastWriteDT, found,
                gea_used_org - (found - gea_buf));
        else if (gea_string_len)
            gen_crc1((char *) &header.GELastWriteDT, found,
                gea_string_len);

	MD_PG_FREE(iodata);
        break;

    case MIOGEAST   : /* MIOGEAST arg      */
        /*
         *	If here, this is a request to set the threshhold.  Must
         *	insure that there is sufficient room and that the
         *	threshhold is greater than zero.
         */
        if (arg < 0)
            rc = EINVAL;
        else
        {
            if (gea_used + arg <= gea_length)
                gea_thresh = arg;
            else
                rc = ENOMEM;
        }
        break;

    case MIOGEARDA  : /* data, size   */
        /*
         *	If here, this is a request to get the GEA attributes.
         */
        if (ioarg.md_size < sizeof(geastuff))
        {
            rc = ENOMEM;
            break;
        }
            
        geastuff.gea_length = gea_length;
        geastuff.gea_used = gea_used;
        geastuff.gea_thresh = gea_thresh;
        if (!fkern) 
        {
            if (copyout(&geastuff, ioarg.md_data, sizeof(geastuff)))
                rc = EFAULT;
        }
        else /* if (!fkern) */
            bcopy(&geastuff, ioarg.md_data, sizeof(geastuff));
        break;

#endif /* _RSPC */
    case MIOBUSGET  : /* MIOBUSGET  addr, size, data(r), incr */
        ioaddr = ioarg.md_addr;
        if (NULL == (iodata =
	    mdget(busnum, IO_RGN, ioaddr, iosize, 0, ioarg.md_incr, &rc))) {
	    rc = ENOMEM;
            break;
	}
        if (!fkern) {
	    if (copyout(iodata, ioarg.md_data, iosize))
		rc = EFAULT;
	} else
	    bcopy(iodata, ioarg.md_data, iosize);
        MD_PG_FREE(iodata);
        break;
    case MIOBUSPUT  : /* MIOBUSPUT  addr, size, data(r), incr */
        if (NULL == (iodata = MD_PG_ALLOC(iosize))) {
            rc = ENOMEM;
            break;
        }
        if (!fkern) {
	    if (copyin(ioarg.md_data, iodata, iosize)) {
		MD_PG_FREE(iodata);
		rc = EFAULT;
		break;
	    }
        } else
	    bcopy(ioarg.md_data, iodata, iosize);
        ioaddr = ioarg.md_addr;
        mdput(busnum, IO_RGN, ioaddr, iosize, iodata, ioarg.md_incr, &rc);
        MD_PG_FREE(iodata);
        break;
    case MIOCCGET   : /* MIOCCGET   addr, size, data(r), incr */
	if (__rs6k() && ((ioarg.md_addr & 0xff) > 7)) {
	    ioaddr = IOCC2 | (ioarg.md_addr & ADDRMASK); /* IOCC */
	    region = IO_RGN;
	} else {
            ioaddr = IOCC | (ioarg.md_addr & ADDRMASK);  /* POS/IOCC */
	    region = IOCC_RGN;
	}
        if (NULL == (iodata =
          mdget(busnum, region, ioaddr, iosize, 0, ioarg.md_incr, &rc))) {
	    rc = ENOMEM;
            break;
	}
        if (!fkern) {
	    if (copyout(iodata, ioarg.md_data, iosize))
		rc = EFAULT;
	} else
	    bcopy(iodata, ioarg.md_data, iosize);
        MD_PG_FREE(iodata);
        break;
    case MIOCCPUT   : /* MIOCCPUT   addr, size, data(p), incr */
	if (__rs6k() && ((ioarg.md_addr & 0xff) > 7)) {
	    ioaddr = IOCC2 | (ioarg.md_addr & ADDRMASK); /* IOCC */
	    region = IO_RGN;
	} else {
            ioaddr = IOCC | (ioarg.md_addr & ADDRMASK);  /* POS/IOCC */
	    region = IOCC_RGN;
	}
        if (NULL == (iodata = MD_PG_ALLOC(iosize))) {
            rc = ENOMEM;
            break;
        }
        if (!fkern) {
	    if (copyin(ioarg.md_data, iodata, iosize)) {
		MD_PG_FREE(iodata);
		rc = EFAULT;
		break;
	    }
        } else
	    bcopy(ioarg.md_data, iodata, iosize);
        mdput(busnum, region, ioaddr, iosize, iodata, ioarg.md_incr, &rc);
        MD_PG_FREE(iodata);
        break;
#ifdef HTXLVL
    case MIOBUSMEM  : /* MIOBUSMEM  */
	buid |= (USERMASK & (ulong)ioarg.md_data);
        ioaddr = as_att(&u.u_adspace, buid|BYPASS_TCW, ioarg.md_addr);
        ioarg.md_addr = ioaddr;
        if (!fkern) {
	    if (copyout(&ioarg, arg, sizeof(MACH_DD_IO)))
		rc = EFAULT;
	} else
	    bcopy(&ioarg, arg, sizeof(MACH_DD_IO));
        break;

    /* this ioctl code for Segreg access to PC Mono Card. */
    case MIOBUSIO   : /* MIOBUSIO   addr, size, data(r), incr */
        ioaddr = as_att(&u.u_adspace, buid|USERMASK, 0);
        ioarg.md_addr = ioaddr;
        if (!fkern) {
	    if (copyout(&ioarg, arg, sizeof(MACH_DD_IO)))
		rc = EFAULT;
	} else
	    bcopy(&ioarg, arg, sizeof(MACH_DD_IO));
        break;
#endif	/* HTXLVL */
    case MIONVLED :
	if (enter_dbg) {	/* debugging support */
	    printf(" %s ", u.u_comm);
	}
        nvled(arg);
        break;
    case MIOIPLCB :
    case MIOAIPLCB :	/* Aux IPLCB */
	if (op == MIOAIPLCB) {
	    if (!vmker.iplcbxptr) {		/* Is it there? */
		rc = ENODEV;
		break;
	    }
	    ioaddr = vmker.iplcbxptr + ioarg.md_addr;
	} else {
	    ioaddr = vmker.iplcbptr + ioarg.md_addr;
	}
        if (!fkern) {
	    if (copyout(ioaddr, ioarg.md_data, ioarg.md_size))
		rc = EFAULT;
	} else
	    bcopy(ioaddr, ioarg.md_data, ioarg.md_size);
	break;
    case MIONVCHCK :
	if (!fkern) {
	    if (copyout(&nvstatus, arg, sizeof(int)))
		rc = EFAULT;
	} else
	    bcopy(&nvstatus, arg, sizeof(int));
	break;
#ifdef _POWER_RS
    case MSLAGET :
        if (NULL == (iodata =
            mdget(busnum, IO_RGN, ioarg.md_addr, iosize, 0, ioarg.md_incr, &rc))) {
	    rc = ENOMEM;
            break;
	}
        if (!fkern) {
	    if (copyout(iodata, ioarg.md_data, iosize))
		rc = EFAULT;
	} else
	    bcopy(iodata, ioarg.md_data, iosize);
        MD_PG_FREE(iodata);
	ioarg.md_slaerr = rc;
	break;
    case MSLAPUT :
        if (NULL == (iodata = MD_PG_ALLOC(iosize))) {
            rc = ENOMEM;
            break;
        }
        if (!fkern) {
	    if (copyin(ioarg.md_data, iodata, iosize)) {
		MD_PG_FREE(iodata);
		rc = EFAULT;
		break;
	    }
        } else
	    bcopy(ioarg.md_data, iodata, iosize);
        mdput(busnum, IO_RGN, ioarg.md_addr, iosize, iodata, ioarg.md_incr, &rc);
	ioarg.md_slaerr = rc;
	break;
#endif	/* _POWER_RS */
    case MIOGETPS:	/* get Power Status register */
    case MIOGETKEY:	/* get Key Lock register */
	if (op == MIOGETPS)
		psr = get_pksr() & 0xfffc0000;	/* mask bits 14-31 for PSR */
	else
		psr = get_pksr() & 0xf;		/* mask bits 0-27 for Key */
	if (!fkern) {
	    if (copyout(&psr, ioarg.md_data, sizeof(psr))) {
		rc = EFAULT;
		break;
	    }
	} else
	    bcopy(&psr, ioarg.md_data, sizeof(psr));
	break;
    case MIOTODGET:	/* get time-of-day register */
#ifdef _RSPC
	if (__rspc()) {
	    rc = md_rspc_tod_rw(ioarg.md_addr, &b, NVREAD);
	}
#endif	/* _RSPC */

#ifdef _RS6K
	if (__rs6k()) {
	    iodata = (volatile char *) 
		    &sys_resource_ptr->sys_regs.time_of_day 
			+ (ioarg.md_addr & RTC_ADDRMASK);
	    __iospace_sync();
	    b = (uchar) *iodata;
	}
#endif	/* _RS6K */

#ifdef _POWER_RS
	if (__power_rs()) {
            ioaddr = RTC_OFFSET + (ioarg.md_addr & RTC_ADDRMASK);
            if (NULL == (iodata =
                mdget(busnum, IOCC_RGN, ioaddr, sizeof(b), 0, MV_BYTE, &rc))) {
                    rc = ENOMEM;
                    break;
            }
	    b = (uchar) *iodata;
            MD_PG_FREE(iodata);
	}
#endif /* _POWER_RS */

        if (!fkern) {
	    if (subyte(ioarg.md_data, b))
		rc = EFAULT;
        } else
	    *(uchar *)ioarg.md_data = b;
	break;    
    case MIOTODPUT:	/* put time-of-day register */
        if (!fkern) {
	    if (copyin(ioarg.md_data, &b, sizeof(b))) {
	        rc = EFAULT;
		break;
	    }
        } else
	    b = *(uchar *)ioarg.md_data;

#ifdef _RSPC
	if (__rspc()) {
	    rc = md_rspc_tod_rw(ioarg.md_addr, &b, NVWRITE);
	}
#endif	/* _RSPC */

#ifdef _RS6K
	if (__rs6k()) {
	    iodata = (volatile char *) 
		    &sys_resource_ptr->sys_regs.time_of_day 
			+ (ioarg.md_addr & RTC_ADDRMASK);
	    *(uchar *)iodata = b;
	    __iospace_sync();
	}
#endif	/* _RS6K */

#ifdef _POWER_RS
	if (__power_rs()) {
            ioaddr = RTC_OFFSET + (ioarg.md_addr & RTC_ADDRMASK);
	    mdput(busnum, IOCC_RGN, ioaddr, sizeof(b), &b, MV_BYTE, &rc);
	}
#endif	/* _POWER_RS */
        break;

#ifdef _RS6K_UP_MCA
    case MIOVPDGET:	/* get VPD/Feature ROM data */
	if (ioarg.md_incr != MV_BYTE) {
	    rc = EINVAL;
	    break;
	}
	rc = md_read_vpd(ioarg.md_sla, ioarg.md_addr, ioarg.md_size,
		ioarg.md_data, fkern);
	break;    
    case MIOCFGGET:	/* read configuration register */
	if (ioarg.md_size != 1) {
	    rc = EINVAL;
	    break;
	}
	rc = md_read_cfg(ioarg.md_sla, ioarg.md_addr, ioarg.md_incr,
		ioarg.md_data, fkern);
	break;    
    case MIOCFGPUT:	/* write configuration register */
	if (ioarg.md_size != 1) {
	    rc = EINVAL;
	    break;
	}
	rc = md_write_cfg(ioarg.md_sla, ioarg.md_addr, ioarg.md_incr,
		ioarg.md_data, fkern);
        break;
    case MIORESET:	/* Reset a bus slot */
	rc = md_reset_slot(ioarg.md_sla, ioarg.md_addr);
	break;
#endif	/* _RS6K_UP_MCA */

#ifdef _RSPC_UP_PCI
    case MIOPCFGET:
        if (NULL == (iodata = MD_PG_ALLOC(iosize))) {
		rc = ENOMEM;
		break;
        }
	old_ad = ioarg.md_data;
	ioarg.md_data = iodata;
	rc = pci_cfgrw(md_bus[busnum].bid[IO_RGN], &ioarg, 0);
        if (!fkern) {
		if (copyout(iodata, old_ad, iosize)) {
			MD_PG_FREE(iodata);
			rc = EFAULT;
			break;
		}
        } else
		bcopy(iodata, old_ad, iosize);
        MD_PG_FREE(iodata);
	break;
    case MIOPCFPUT:
        if (NULL == (iodata = MD_PG_ALLOC(iosize))) {
		rc = ENOMEM;
		break;
        }
        if (!fkern) {
		if (copyin(ioarg.md_data, iodata, iosize)) {
			MD_PG_FREE(iodata);
			rc = EFAULT;
			break;
		}
        } else
		bcopy(ioarg.md_data, iodata, iosize);
	ioarg.md_data = iodata;
	rc = pci_cfgrw(md_bus[busnum].bid[IO_RGN], &ioarg, 1);
	MD_PG_FREE(iodata);
	break;
    case MIOMEMGET  : /* MIOMEMGET  addr, size, data(r), incr */
        ioaddr = ioarg.md_addr;
        if (NULL == (iodata =
	    mdget(busnum, MEM_RGN, ioaddr, iosize, 0, ioarg.md_incr, &rc))) {
	    rc = ENOMEM;
            break;
	}
        if (!fkern) {
	    if (copyout(iodata, ioarg.md_data, iosize))
		rc = EFAULT;
	} else
	    bcopy(iodata, ioarg.md_data, iosize);
        MD_PG_FREE(iodata);
        break;
    case MIOMEMPUT  : /* MIOMEMPUT  addr, size, data(r), incr */
        if (NULL == (iodata = MD_PG_ALLOC(iosize))) {
            rc = ENOMEM;
            break;
        }
        if (!fkern) {
	    if (copyin(ioarg.md_data, iodata, iosize)) {
		MD_PG_FREE(iodata);
		rc = EFAULT;
		break;
	    }
        } else
	    bcopy(ioarg.md_data, iodata, iosize);
        ioaddr = ioarg.md_addr;
        mdput(busnum, MEM_RGN, ioaddr, iosize, iodata, ioarg.md_incr, &rc);
        MD_PG_FREE(iodata);
        break;
#endif	/* _RSPC_UP_PCI */

#ifdef _RS6K_SMP_MCA

    case MIONVSTRLED:
        if (NULL == (iodata = MD_PG_ALLOC(iosize))) {
		rc = ENOMEM;
		break;
        }
        if (!fkern) {
		if (copyin(ioarg.md_data, iodata, iosize)) {
			MD_PG_FREE(iodata);
			rc = EFAULT;
			break;
		}
        } else
		bcopy(ioarg.md_data, iodata, iosize);
	mdnvstrled(iodata);
        MD_PG_FREE(iodata);
	break;
	
    case MIOSETKEY:
{
	uint key;
	
        if (!fkern) {
		if (copyin(ioarg.md_data, &key, sizeof(key))) {
			rc = EFAULT;
			break;
		}
        } else
		bcopy(ioarg.md_data, &key, sizeof(key));
	mdsetkey(key);
	break;
}

    case MRDSSET:
{
	uint rds_state;
	if (!fkern) {
		if (copyin(ioarg.md_data, &rds_state, sizeof(rds_state))) {
			rc = EFAULT;
			break;
		}
	} else
		bcopy(ioarg.md_data, &rds_state, sizeof(rds_state));
	rc = mdrdsset(ioarg.md_cbnum, ioarg.md_dknum, rds_state);
	break;
}
    case MRDSGET:
{
	uint rds_state;
		
	rc = mdrdsget(ioarg.md_cbnum, ioarg.md_dknum, &rds_state);

	if (!fkern) {
		if (copyout(&rds_state, ioarg.md_data, sizeof(rds_state))) {
			rc = EFAULT;
			break;
		}
	} else
		bcopy(&rds_state, ioarg.md_data, sizeof(rds_state));
	break;
}
    case MEEVPDPUT:
        if (!fkern) {
		if (copyin(ioarg.md_addr, vpd_desc, sizeof(vpd_desc))) {
			rc = EFAULT;
			break;
		}
        } else
        	bcopy(ioarg.md_addr, vpd_desc, sizeof(vpd_desc));
        /* fall through */
    case MDINFOSET:
    case MFEPROMPUT:
    case MPOWERSET:
        if (NULL == (iodata = MD_PG_ALLOC(iosize))) {
		rc = ENOMEM;
		break;
	}
	if (!fkern) {
		if (copyin(ioarg.md_data, iodata, iosize)) {
			MD_PG_FREE(iodata);
			rc = EFAULT;
			break;
		}
	} else
		bcopy(ioarg.md_data, iodata, iosize);
	switch (op) {
	case MDINFOSET:
		rc = mdinfoset(ioarg.md_type, iodata, iosize, &length);
		break;
	case MEEVPDPUT:
		rc = mdeevpdput(ioarg.md_eenum, vpd_desc, iodata, iosize,
				&length);
		break;
	case MFEPROMPUT:
		rc = mdfepromput(iodata, iosize, &length);
		break;
	case MPOWERSET:
		rc = mdpowerset(ioarg.md_cbnum, ioarg.md_cmd, iodata, iosize,
				&length);
		break;
	}
	if (rc != 0) {
		MD_PG_FREE(iodata);
		break;
	}
	if (ioarg.md_length) {
		if (!fkern) {
			if (copyout(&length, ioarg.md_length, sizeof(length)))
			{
				MD_PG_FREE(iodata);
				rc = EFAULT;
				break;
			}
		} else {
			bcopy(&length, ioarg.md_length, sizeof(length));
		}
	}
	MD_PG_FREE(iodata);
	break;

    case MEEVPDGET:
        if (!fkern) {
		if (copyin(ioarg.md_addr, vpd_desc, sizeof(vpd_desc))) {
			rc = EFAULT;
			break;
		}
        } else
		bcopy(ioarg.md_addr, vpd_desc, sizeof(vpd_desc));
        /* fall through */
    case MDINFOGET:
    case MEEPROMGET:
    case MPOWERGET:
        if (NULL == (iodata = MD_PG_ALLOC(iosize))) {
		rc = ENOMEM;
		break;
	}
	switch (op) {
	case MDINFOGET:
		rc = mdinfoget(ioarg.md_type, iodata, iosize, &length);
		break;
	case MEEPROMGET:
		rc = mdeepromget(ioarg.md_eenum, ioarg.md_addr, iodata,
				 iosize, &length);
		break;
	case MEEVPDGET:
		rc = mdeevpdget(ioarg.md_eenum, vpd_desc, iodata, iosize,
				&length);
		break;
	case MPOWERGET:
		rc = mdpowerget(ioarg.md_cbnum, iodata, iosize, &length);
		break;
	}
	if (rc != 0) {
		MD_PG_FREE(iodata);
		break;
	}
	if (!fkern) {
		if (copyout(&length, ioarg.md_length, sizeof(length))
		    || copyout(iodata, ioarg.md_data, length)) {
			MD_PG_FREE(iodata);
			rc = EFAULT;
			break;
		}
	} else {
		bcopy(&length, ioarg.md_length, sizeof(length));
		bcopy(iodata, ioarg.md_data, length);
	}
	MD_PG_FREE(iodata);
	break;

#ifdef _POWER_MP	
    case MCPUSET:
{
	int cpu_status;
	
	if (!fkern) {
		if (copyin(ioarg.md_data, &cpu_status, sizeof(cpu_status))) {
			rc = EFAULT;
			break;
		}
	} else
		bcopy(ioarg.md_data, &cpu_status, sizeof(cpu_status));
	rc = mdcpuset(ioarg.md_cpunum, cpu_status);
	break;
}
    case MCPUGET:
{	
	int cpu_status;
	
	rc = mdcpuget(ioarg.md_cpunum, &cpu_status);

	if (rc != 0)
		break;
		
	if (!fkern) {
		if (copyout(&cpu_status, ioarg.md_data, sizeof(cpu_status))) {
			rc = EFAULT;
			break;
		}
	} else
		bcopy(&cpu_status, ioarg.md_data, sizeof(cpu_status));
	break;
}
#endif /* _POWER_MP */

    case MSURVSET:
	rc = mdsurvset(ioarg.md_mode, ioarg.md_delay);
	break;

    case MSURVRESET:
	rc = mdsurvreset();
	break;

    case MIOKEYCONNECT:
	rc = mdkeyconnect(arg);
	break;

    case MIOKEYDISCONNECT:
	rc = mdkeydisconnect();
	break;

#endif /* _RS6K_SMP_MCA */

    default :
        rc = EINVAL;
        break;
    }
    MD_UNLOCK();

    return rc;
} /* mdioctl() */


/*
 * NAME: mdread
 *
 * FUNCTION:  Device read routine
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES:
 *    devno : major and minor device numbers
 *     uiop : pointer to uio structure
 *     chan : channel number
 *
 * RETURN VALUE DESCRIPTION: 
 *         0 : if no error
 *    ENOENT : if chan is not a valid channel number
 *
 */
int
mdread(dev_t devno, struct uio *uiop, int chan)
{ /* only for base */
    int tsize, cbytes, rderr = 0;
    char *dblock;
    caddr_t base;

    /* check devno validity */
    switch(chan) {
	case BASECUST:
	    base = BaseCustom + uiop->uio_offset;
	    cbytes = Base_size - uiop->uio_offset;
	    break;
	default:
	    return ENOENT;
    }
    rderr = uiomove(base, cbytes, UIO_READ, uiop);
    return rderr;
} /* mdread() */


/*
 * NAME: mdmpx
 *
 * FUNCTION: Provides channel number for /dev/nvram/xxx
 *           where xxx can be only base
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES:    No extension (/dev/nvram) results in channel 0.
 *
 * RETURN VALUE: Return ENOENT if the channel is invalid or if
 *		 buffer for /dev/nvram/base has been freed.
 *
 *
 */
int
mdmpx(dev_t devno, int *chanp, char *channame)
{
    int rc=0;

    if (channame) {     /* ALLOCATE CHANNEL */
	    if (0 == strcmp(channame, "base")) {
	        if (BaseCustom)	/* Does the buffer still exist? */
	            *chanp = BASECUST;
	        else
		    rc = ENOENT; /* Buffer must have been freed */
	    }
	    else {
		if (*channame == (char) 0)
		    *chanp = 0;		/* channel 0 */
		else
		    rc = ENOENT; 	/* invalid channel */
	    }
    } 
    return rc;
} /* mdmpx() */


/*
 * NAME: md_check_buid
 *
 * FUNCTION: Checks for valid buid by scanning valid buid table.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * RETURN VALUE: Return 0 if buid is valid, else 1. 
 *
 */
static int
md_check_buid(ulong buid)
{
    int i;

#ifdef _POWER_RS
    if (__power_rs())		/* No way to check it */
	return(0);
#endif	/* _POWER_RS */
#ifdef _POWER_601
    if (__power_601() && ((buid & 0xfff00000) == (REAL_601|BUID_T)))
	return(0);
#endif	/* _POWER_601 */
    for (i = 0; i < MD_MAX_BUID; i++)	/* scan the table */
        if ((buid & 0x1ff00000) == md_valid_buid[i])
	    return(0);
    return(1);
} /* md_check_buid() */
