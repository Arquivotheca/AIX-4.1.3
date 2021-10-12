static char sccsid[] = "@(#)93	1.11  src/bos/kernel/io/machdd/rspcinit.c, machdd, bos41J, 9516B_all 4/21/95 11:42:01";
/*
 * COMPONENT_NAME: (MACHDD) Machine Device Driver
 *
 * FUNCTIONS: nvrd_xlate_rspc(), get_port_num(), findekw()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#ifdef _RSPC

#include <sys/errno.h>
#include <sys/lock_def.h>
#include <sys/nvdd.h>
#include <sys/residual.h>
#include <sys/iplcb.h>
#include <sys/vmker.h>
#include <sys/syspest.h>
#include <sys/iocc.h>
#include "md_nvram.h"
#include "md_rspc.h"
#include "md_extern.h"

extern ulong NVRAM_swbase;
extern struct nvblock nv_blocks[];
extern char *md_nv_buf;
extern HEADER header;
extern char *gea_buf;
extern int gea_thresh;
extern int gea_used;
extern int gea_length;
extern int fw_keycase;
extern ushort nvram_data;

/*
 * NAME: get_port_num
 *
 * FUNCTION: Look into the residual data to get the port number to 
 *	     be used for a data port.  RSPC type boxes should use
 *	     0x76, but sandalfoot uses 0x77. 
 *
 * Notes:    This routine should only be called once at init time.
 *           It initializes the global variable nvram_data.
 *
 * Inputs:   None
 *
 * EXECUTION ENVIRONMENT:  Initialization
 *
 * RETURN VALUE: None
 *
 */
void
get_port_num()
{
    int i;
    int oldi;
    struct ipl_cb *iplcb;
    struct ipl_directory *idir_ptr;
    RESIDUAL        *rp;
    struct _S9_Pack	*s9_packet;
    struct _S8_Pack	*s8_packet;

    nvram_data = 0xffff;

    /* GET rp POINTER: pointer to RESIDUAL structure at residual_offset 
 	in the IPL control block. */
    iplcb = (struct ipl_cb *)vmker.iplcbptr;
    idir_ptr = (struct ipl_directory *)(&iplcb->s0);
    rp = (RESIDUAL *)((char *)iplcb + idir_ptr->residual_offset);


    for (i=0;i<rp->ActualNumDevices;i++) {
    	if ((rp->Devices[i].DeviceId.BaseType == SystemPeripheral) &&
	    (rp->Devices[i].DeviceId.SubType == NVRAM)) {
		/*
		 *  Found the device table entry for NVRAM.  Go look in
		 *  the residual heap for an S9 packet that contains the
		 *  address and data port numbers.
		 */
		if (rp->Devices[i].AllocatedOffset) {
			s9_packet = (struct _S9_Pack *)
			    &rp->DevicePnPHeap[rp->Devices[i].AllocatedOffset];
			while (s9_packet->Tag != 0x47) {
				/*
				 *  Look through all packets until
				 *  the 1st S9 packet is reached.
				 */
				if (s9_packet->Tag & 0x80) {
					s9_packet = (struct _S9_Pack *)
					    ((caddr_t)s9_packet +
					    (((struct _L1_Pack *) s9_packet)->
						Count1 << 8) +
					    ((struct _L1_Pack *) s9_packet)->
						Count0 + 3);
				} else if ((s9_packet->Tag & 0xf8) == 0x78) {
					break;
				} else {
					s9_packet = (struct _S9_Pack *)
					    ((caddr_t)s9_packet +
					    (s9_packet->Tag & 0x07) + 1);
				}
			}
			if ((s9_packet->Tag & 0xf8) == 0x78) {
				/*
				 *  No packet found, assume 0x74 and 0x77
				 */
				nvram_data = 0x77;
			} else {
				s8_packet = (struct _S8_Pack *) s9_packet;
				s8_packet++;
				nvram_data = (s8_packet->RangeMin[1] << 8) +
				    s8_packet->RangeMin[0];
			}
		} else {
			/*
			 * Assume addr = 0x74, data = 0x77 (Sandalfoot)
			 */
			nvram_data = 0x77;
		}
	}
    }

    if (nvram_data == 0xffff) {
	/*
	 * Old Sandalfoot ROS, assume data =0x77
	 */
	nvram_data = 0x77;
    }
assert((nvram_data == 0x77) || (nvram_data == 0x76));

}

/*
 * NAME: findekw
 *
 * FUNCTION: Search the Global Environment Area of NVRAM to look
 *           for the specified input string.
 *
 * Notes:    This routine should only be called once at init time.
 *
 * Inputs:   Pointer to the string (fw_keycase=) used for firmware
 *           to identify the key position electronically.
 *
 * EXECUTION ENVIRONMENT:  Initialization
 *
 * RETURN VALUE: 0 -> string not found.
 *               pointer to string on right side of the equal sign.
 *
 */
char *
findekw(char *iodata)
{
    char *gea_buf_ptr;
    int string_len;
    char *tiodata;
    int i;

    /*
     *    The input string is terminated with an equal sign.
     *    Determine the length of the string including the
     *    equal sign.
     */
    tiodata = iodata;
    for (i = 0; (*tiodata != 0) && (*tiodata != '='); i++)
        tiodata++;

    if (*tiodata == '=')
        string_len = tiodata - iodata + 1;
    else
        return 0;

    for (gea_buf_ptr = gea_buf;
       gea_buf_ptr < (gea_buf + gea_used - string_len + 1); gea_buf_ptr++)
    {
        if (!strncmp(iodata, gea_buf_ptr, string_len))
        /*
         *   If here, we have a hit.  Compute pointer
         *   to the string after the equal sign in the Global Env Area.
         */
            return (gea_buf_ptr + string_len);
        else /* if (!strncmp(iodata, gea_buf ... */
        /*
         *      If here, string did not compare; must
         *      step to next variable in GEA for next
         *      compare.
         */
        {
            while (*gea_buf_ptr != 0)
                gea_buf_ptr++;
        } /* if (!strncmp(iodata, gea_buf ... */

    } /* for (gea_buf_ptr = gea_buf; gea_buf_pt ... */
    return 0;;
}


/*
 * NAME: nvrd_xlate_rspc
 *
 * FUNCTION: Move vital information from the real NVRAM to the
 *	     NVRAM cache.  The information is munged back to its
 *	     original format.
 *
 * Notes:    This routine should only be called once at init time.
 *	     It allocates a pinned buffer for later use by
 *           nvwr_xlate_rspc().
 *
 * Inputs:   None
 *
 * EXECUTION ENVIRONMENT:  Initialization
 *
 * RETURN VALUE: None
 *
 */
void
nvrd_xlate_rspc()
{
    struct dumpinfo *dp;
    struct erec *ep;
    struct nvblock *nvp;
    struct short_dump *sd;
    struct erec *es;
    int hdr;
    char *gea_buf_ptr;
    char *keyptr;

    /* Zero blocks */
    nvblock_zero(&nv_blocks[DUMP_NVBLOCK]);
    nvblock_zero(&nv_blocks[ERRLOG_NVBLOCK]);

    md_nv_buf = MD_PIN_ALLOC(NVBUF_SIZE);
    if (md_nv_buf == NULL)
	return;
    
    /* get nvram_data value */
    get_port_num();

    /* read in the nvram header */
    nvrw_rspc(NVREAD, &header, 0, sizeof(header), 0);

    /*
     *	Allocate space for the Global Env Area
     */
    gea_buf = MD_PIN_ALLOC(header.GELength);
    if (gea_buf == NULL)
	return;
    gea_thresh = 0;		/* housekeep threshhold */
    
    /*
     *	Read GEA from NVRAM into memory.
     */
    nvrw_rspc(NVREAD, gea_buf, header.GEAddress, header.GELength, 0);
    /*
     *	Null bytes at the end of the GEA are unused.  Calculate
     *	the size which is the maximum threshhold.
     */
    for (gea_buf_ptr = (gea_buf + header.GELength - 1); 
	(gea_buf_ptr >= gea_buf) && (*gea_buf_ptr == 0); gea_buf_ptr--)
    	;
    gea_used = gea_buf_ptr - gea_buf + 2;	
    gea_length = header.GELength;

    /*
     *   Determine electronic key position: normal or service.
     */
    keyptr = findekw("fw-keyswitch=");
    /*
     *   Set the default key postion to normal.
     */
    fw_keycase = KEY_POS_NORMAL;
   
    if (keyptr)
    {
        /* 
         *   If here, the string was found in GEA.  Determine if
         *   position is service.
         */
        if (!strcmp(keyptr, "service"))
            fw_keycase = KEY_POS_SERVICE;
    }
	
    /* Read info from NVRAM */
    nvrw_rspc(NVREAD, md_nv_buf, header.OSAreaAddress, NVBUF_SIZE, 0);

    /* Check CRC */
    if (crc32(md_nv_buf, NVBUF_SIZE)) {	/* bad crc? */
	return;			/* no point in continuing */
    }

    /* Extract vital dump information */
    nvp = &nv_blocks[DUMP_NVBLOCK];
    dp = (struct dumpinfo *)(NVRAM_swbase + nvp->base);
    sd = (struct short_dump *)(md_nv_buf + ERRLOG_SIZE);
    bcopy(sd->dm_devicename, dp->dm_devicename, 20);
    dp->dm_size = sd->dm_size;
    dp->dm_timestamp = sd->dm_timestamp;
    dp->dm_status = sd->dm_status;
    dp->dm_flags = sd->dm_flags;
    bcopy(sd->dm_filehandle, dp->dm_filehandle, 16);

    /* Extract vital error log information */
    nvp = &nv_blocks[ERRLOG_NVBLOCK];
    ep = (struct erec *)(NVRAM_swbase + nvp->base);
    es = (struct erec *)(md_nv_buf);
    ep->erec_len = es->erec_len;
    ep->erec_rec_len = es->erec_rec_len;
    ep->erec_symp_len = es->erec_symp_len;
    ep->erec_timestamp = es->erec_timestamp;
    if (es->erec_rec_len && (es->erec_rec_len < ERRLOG_SIZE))
        bcopy(&es->erec_rec, &ep->erec_rec, es->erec_rec_len);
    if (es->erec_symp_len && (es->erec_symp_len < ERRLOG_SIZE))
	bcopy(&es->erec_rec+es->erec_rec_len, &ep->erec_len+ep->erec_symp_len,
	    es->erec_symp_len);

    /* Update CRCs */
    nvblock_crc(&nv_blocks[DUMP_NVBLOCK]);
    nvblock_crc(&nv_blocks[ERRLOG_NVBLOCK]);
}

#endif	/* _RSPC */

