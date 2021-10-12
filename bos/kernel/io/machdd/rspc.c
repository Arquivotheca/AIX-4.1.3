static char sccsid[] = "@(#)92	1.15  src/bos/kernel/io/machdd/rspc.c, machdd, bos41J, 9518A_all 5/1/95 11:03:54";
/*
 * COMPONENT_NAME: (MACHDD) Machine Device Driver
 *
 * FUNCTIONS: nvrw_rspc(), nvwr_xlate_rspc(), md_rspc_tod_rw(),
 *            md_restart_block_read(), md_restart_block_upd()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#ifdef _RSPC

#include <sys/errno.h>
#include <sys/lock_def.h>
#include <sys/intr.h>
#include <sys/system_rspc.h>
#include <sys/systemcfg.h>
#include <sys/nvdd.h>
#include <stddef.h>
#include "md_nvram.h"
#include "md_rspc.h"
#include "md_extern.h"

extern ulong NVRAM_swbase;
extern struct nvblock nv_blocks[];
extern char *gea_buf;
extern int gea_length;

int md_nio_bus;
char *md_nv_buf;
Simple_lock md_nv_lock;
HEADER header;
ushort nvram_data;

#define rol(x,y) ( ( ((x)<<(y)) | ((x)>>(16 - (y))) ) & 0xFFFF)
#define ror(x,y) ( ( ((x)>>(y)) | ((x)<<(16 - (y))) ) & 0xFFFF)

/*
 * NAME: nvrw_rspc
 *
 * FUNCTION: Read/Write RSPC NVRAM (within DS1385 RTC)
 *
 * Notes: Gets the lock, then reads or writes to the 
 *	  NVRAM within the DS1385 Real Time Clock module.
 * 
 * Inputs:
 *
 *	dir = NVREAD or NVWRITE
 *	buf = pointer to data buffer
 *	addr = Offset into NVRAM (12 bits)
 *	len = Number of bytes to read/write
 *      flag = lock already held if ne 0.
 *
 * EXECUTION ENVIRONMENT:  Process or interrupt (pinned routine)
 *
 * RETURN VALUE: 0 if successful
 *               ENOMEM if allocation fails, EINVAL if map fails
 *
 */
int
nvrw_rspc(int dir, char *buf, uint addr, uint len, int flag)
{
    int rc=0;
    volatile struct rspcsio *siop;
    int i;
    int oldi;
    char *data_port;

    if (INVALID == (siop = md_mapit(md_nio_bus, IO_RGN, 0, 0x1000)))
	return(EINVAL);
    if (!flag)
        oldi = disable_lock(INTMAX, &md_nv_lock);

    /* get pointer to structure element */
    if (nvram_data == 0x77 ) /* sandalfoot */
        data_port = &siop->nvram_data;
    else /* polo/woodfield and follow ons*/
        data_port = &siop->nvram_data_rspc;

    for (i=0; i<len; i++) {
	siop->nvram_ad_lo = addr & 0xff;
	siop->nvram_ad_hi = (addr >> 8);
	addr++;
	__iospace_sync();
	if (dir == NVWRITE) 
	    *data_port = buf[i];
	else
	    buf[i] = *data_port;
    }
    if (!flag)
        unlock_enable(oldi, &md_nv_lock);
    md_unmapit(md_nio_bus, siop);
    return(rc);
}

/*
 * NAME: md_restart_block_read
 *
 * FUNCTION: Move the restart_block info from the header structure
 *           to the user's buffer.  Put PMMode into md_addr.
 *
 * Inputs:   Pointer to the mdio structure.
 *
 * EXECUTION ENVIRONMENT:  Process or interrupt (pinned routine)
 *
 * RETURN VALUE: 0 -> successful
 *               ENOMEM -> not enough room in user's data buffer.
 *
 */
int md_restart_block_read(MACH_DD_IO *arg)
{

    if (!__rspc())
    {
        return EINVAL;
    }

    if (arg->md_size >= (sizeof(RESTART_BLOCK) && (arg->md_data != 0)))
    {
        bcopy((int) &header + offsetof(HEADER, RestartBlock), arg->md_data, 
              sizeof(RESTART_BLOCK));
        arg->md_addr = (ulong) header.PMMode;
    }
    else
        return ENOMEM;
    return(0);
}

/*
 * NAME: md_restart_block_upd
 *
 * FUNCTION: Move the restart_block from the user's buffer into
 *           the memory resident header.  Compute the checksum
 *           for the restart_block.  Then create Crc1 and write
 *           the header to NVRAM.
 *
 * Inputs:   Pointer to the mdio structure.
 *	     Unsigned char to be stored into PMMode in the header.
 *
 * EXECUTION ENVIRONMENT:  Process or interrupt (pinned routine)
 *
 * RETURN VALUE: 0 -> successful
 *
 */
int md_restart_block_upd(MACH_DD_IO *arg, unsigned char pmmode)
{

    if (!__rspc())
    {
        return EINVAL;
    }

    header.PMMode = pmmode;
    bcopy(arg->md_data, (int) &header + offsetof(HEADER, RestartBlock), 
              sizeof(RESTART_BLOCK));
    /*
     *	Now calculate the restart block checksum
     */
    header.RestartBlock.CheckSum = (uint) (header.RestartBlock.BootMasterId +
               header.RestartBlock.ProcessorId +
               header.RestartBlock.BootStatus +
               header.RestartBlock.RestartAddress +
               header.RestartBlock.Version +
               header.RestartBlock.Revision) +
               (uint) (header.RestartBlock.SaveAreaAddr +
               header.RestartBlock.SaveAreaLength);
    header.RestartBlock.CheckSum = -header.RestartBlock.CheckSum;
    gen_crc1(0,0,0);	/* generate Crc1 but no time/date stamp */
    return(0);
}
/*
 * NAME: nvwr_xlate_rspc
 *
 * FUNCTION: Move vital information from the NVRAM cache to the real
 *	     NVRAM.  The information is reformatted to save space.
 *	     Only the vital fields are saved.
 *
 * Inputs:   None
 *
 * EXECUTION ENVIRONMENT:  Process or interrupt (pinned routine)
 *
 * RETURN VALUE: None
 *
 */
void
nvwr_xlate_rspc()
{
    struct dumpinfo *dp;
    struct short_dump *sd;
    struct erec *ep;
    struct erec *es;
    struct nvblock *nvp;
    int hdr;
    ulong *crc;

    if (md_nv_buf == NULL)
        return;

    /* Extract vital dump information */
    nvp = &nv_blocks[DUMP_NVBLOCK];
    dp = (struct dumpinfo *)(NVRAM_swbase + nvp->base);
    sd = (struct short_dump *)(md_nv_buf + ERRLOG_SIZE);
    bcopy(dp->dm_devicename, sd->dm_devicename, 20);
    sd->dm_size = dp->dm_size;
    sd->dm_timestamp = dp->dm_timestamp;
    sd->dm_status = dp->dm_status;
    sd->dm_flags = dp->dm_flags;
    bcopy(dp->dm_filehandle, sd->dm_filehandle, 16);

    /* Extract vital error log information */
    nvp = &nv_blocks[ERRLOG_NVBLOCK];
    ep = (struct erec *)(NVRAM_swbase + nvp->base);
    es = (struct erec *)(md_nv_buf);
    hdr = sizeof(struct erec) - sizeof(struct err_rec) - sizeof(struct sympt_data);
    if (ep->erec_len > ERRLOG_SIZE) {	/* truncate if necessary */
	es->erec_len = ERRLOG_SIZE;
    } else {
	es->erec_len = ep->erec_len;
    }
    if (ep->erec_rec_len > (ERRLOG_SIZE - hdr)) {
	es->erec_symp_len = 0;		 	/* No room for symp data */
	es->erec_rec_len = ERRLOG_SIZE - hdr;	/* truncate detail data */
    } else {
	es->erec_rec_len = ep->erec_rec_len;	/* room for all detail data */
	if (ep->erec_symp_len > (ERRLOG_SIZE - hdr - es->erec_rec_len))
	    es->erec_symp_len = ERRLOG_SIZE - hdr - es->erec_rec_len; /* trunc */
	else
	    es->erec_symp_len = ep->erec_symp_len; /* room for all symp data */
    }
    es->erec_timestamp = ep->erec_timestamp;
    if (es->erec_rec_len)
	bcopy(&ep->erec_rec, &es->erec_rec, es->erec_rec_len);
    if (es->erec_symp_len)
	bcopy(&ep->erec_rec+ep->erec_rec_len, &es->erec_len+es->erec_symp_len,
	    es->erec_symp_len);
    
    /* Compute CRC */
    crc = (ulong *) &md_nv_buf[NVBUF_SIZE-CRC_SIZE];
    *crc = crc32(md_nv_buf, NVBUF_SIZE-CRC_SIZE);
			
    /* Write info to NVRAM after checking the size, then update time stamp */
    if (NVBUF_SIZE <= header.OSAreaLength ) {
        nvrw_rspc(NVWRITE, md_nv_buf, header.OSAreaAddress, NVBUF_SIZE, 0); 
    }
    /*
     *	Set header up for OS=AIX and os area used.  Then update time of day
     *  and generate new Crc1, then write header out to NVRAM.
     */
    header.OSAreaUsage = Used; /* Used */
    header.LastOS = AIX; /* AIX */
    gen_crc1((char *) &header.OSAreaLastWriteDT, 0, 0); /* no GEA to write */
    
}

/*
 * NAME: gen_crc1
 *
 * FUNCTION: Get TOD and compute Crc1 value for header.  Write volatile
 *	     parts of the header to NVRAM and optionally write the
 *           part of the Global Environment Area that is specified.
 *
 * Inputs:  p1 = pointer to the 8 bytes of tod to be updated if non-zero.
 *          p2 = pointer to the GEA to be written to NVRAM if non-zero.
 *          p3 = number bytes in GEA to be written. 
 *
 * EXECUTION ENVIRONMENT:  Process or interrupt (pinned routine)
 *
 * RETURN VALUE: None
 *
 */
gen_crc1(char *tod, char *gea_addr, int size)
{
    ushort   uiCRC = 0xFFFF;
    unsigned int   uiCRCOffset, uiCRCEndOffset;
    uchar data;
    char *start;
    int i, j, oldi;
    volatile struct rspcsio *siop;
    int tod_offset[] = {0,2,4,7,8,9};
    char *tod_ptr;
    
    oldi = disable_lock(INTMAX, &md_nv_lock);
    if (tod)		/* if tod null, then only Crc1 to be generated */
    {
        /*
         *	Now must get the current time of day to be written to the
         *	OSArea header for time of last write.
         */
        siop = md_mapit(md_nio_bus, IO_RGN, 0, 0x1000);
        if (siop != INVALID)
        {

            /*
             *	If here, we will access the tod from
             *	nvram three times.  The idea is to avoid
             *	a carry from one byte to another such that
             *	the time is inconsistent. 
             */
            tod_ptr = tod + 6;	/* point to the seconds offset */
            data = *tod_ptr ^ 0xff;
            for (i=0; (i < 3) && (*tod_ptr != data); i++)
            {
    	        for(j=0; j < 6; j++)
    	        {
                    /*
                     *	Each byte contains two 4-bit digits of
                     *	tod.
                     */
                    siop->rtc_index = tod_offset[j];
                    __iospace_sync();
                    *tod_ptr = siop->rtc_data;
                    tod_ptr--;
                }
                siop->rtc_index = 0;    /* read seconds again         */
                __iospace_sync();
                data = siop->rtc_data;
                tod_ptr = tod + 6;	/* point to the seconds offset */
            }

    	    md_unmapit(md_nio_bus, siop);

            /*
             *	Set lsb to 0, and determine the century.  If the
             *	year 4-bit digit is a 9, we set this for the 1900s.
             *	Otherwise, we are into the year 2000 and beyond.
             *	NOTE: This fails in the year 2090.
             */
            tod_ptr = tod + 7; 	/* point to lsb	*/
            *tod_ptr = 0;
            tod_ptr = tod + 1;      /* point to year char  */
            if ((*tod_ptr & 0xf0) == 0x90)
                *tod = 0x19;
            else
                *tod = 0x20;
        }
        else
        {
            /*
             *	If here, unable to map nvram space.  Set tod to
             *	0 and continue.
             */
            for (i=0; i<8; i++)
            {
                *tod = 0;
                tod++;
            }
        }
    } /* endif if(tod) */

    uiCRCOffset = sizeof(header.Size) + sizeof(header.Version)
                  + sizeof(header.Revision);
    uiCRCEndOffset = uiCRCOffset + sizeof(header.Crc1) 
                  + sizeof(header.Crc2);
    /* sandalfoot crc has bug that skips one byte */
    if (header.Revision < 4 ) /* sandalfoot */
        uiCRCEndOffset++;

    /* generate crc1 value for header */
    for (start = &header; start < ((int) &header + uiCRCOffset); start++)
        uiCRC = crcgen(uiCRC, *start);

    for (start = (int) &header + uiCRCEndOffset; 
      start < ((int) &header + sizeof(header)); start++) 
        uiCRC = crcgen(uiCRC, *start);

    for (start = gea_buf; start < (gea_buf + gea_length); start++) 
        uiCRC = crcgen(uiCRC, *start);

    /* move CRC1 value to memory resident header */
    header.Crc1 = uiCRC;
    /*
     *    Now write the volatile parts of the header to NVRAM.
     */
    nvrw_rspc(NVWRITE, &header.Crc1, offsetof(HEADER, Crc1), 
        offsetof(HEADER, Security) - offsetof(HEADER, Crc1), 1); 
    nvrw_rspc(NVWRITE, &header.GELastWriteDT, offsetof(HEADER, GELastWriteDT),
        sizeof(header.GELastWriteDT), 1); 
    nvrw_rspc(NVWRITE, &header.OSAreaLastWriteDT, 
        offsetof(HEADER, OSAreaLastWriteDT), 
        sizeof(header.OSAreaLastWriteDT), 1);
    /*
     *    Now check if part of the GEA must be written.
     */
    if (gea_addr != 0)
        nvrw_rspc(NVWRITE, gea_addr, header.GEAddress + (gea_addr - gea_buf),
            size, 1); 
    unlock_enable(oldi, &md_nv_lock);
}

/*************************************************************************/
/* Name:crcgen                                                           */
/*                                                                       */
/* Function: Calculates the next CRC value for the next byte in a stream */
/*           Uses the CCITT Polynomial: x**16 + x**12 + x**5 + 1         */
/*                                                                       */
/* Where                                                                 */
/*    ^        is defined to be the exclusive or function                */
/*    d0-d7    denotes the incoming data bits                            */
/*    c0-c15   denotes the newly calculated crc (n state)                */
/*    p0-p15   denotes the previous crc (n-1 state)                      */
/*    pd0-pd7  denotes p(x) .xor. d(x)                                   */
/*                                                                       */
/*    c(0)  = p(8)  ^ pd(0) ^ pd(4)                                      */
/*    c(1)  = p(9)  ^ pd(1) ^ pd(5)                                      */
/*    c(2)  = p(10) ^ pd(2) ^ pd(6)                                      */
/*    c(3)  = p(11) ^ pd(0) ^ pd(3) ^ pd(7)                              */
/*    c(4)  = p(12) ^ pd(1)                                              */
/*    c(5)  = p(13) ^ pd(2)                                              */
/*    c(6)  = p(14) ^ pd(3)                                              */
/*    c(7)  = p(15) ^ pd(0) ^ pd(4)                                      */
/*    c(8)  = pd(0) ^ pd(1) ^ pd(5)                                      */
/*    c(9)  = pd(1) ^ pd(2) ^ pd(6)                                      */
/*    c(10) = pd(2) ^ pd(3) ^ pd(7)                                      */
/*    c(11) = pd(3)                                                      */
/*    c(12) = pd(0) ^ pd(4)                                              */
/*    c(13) = pd(1) ^ pd(5)                                              */
/*    c(14) = pd(2) ^ pd(6)                                              */
/*    c(15) = pd(3) ^ pd(7)                                              */
/*                                                                       */
/* Note:                                                                 */
/*       Bit 0 is the MSB                                                */
/* Input:                                                                */
/*       oldcrc         previous CRC value                               */
/*       data           next data byte                                   */
/* Returns:                                                              */
/*       new CRC value                                                   */
/*                                                                       */
/*************************************************************************/
int crcgen(unsigned int oldcrc, unsigned char data)
{
   unsigned int pd, crc;

   pd = ((oldcrc>>8) ^ data) << 8;

   crc = 0xFF00 & (oldcrc << 8);
   crc |= pd >> 8;
   crc ^= rol(pd,4) & 0xF00F;
   crc ^= ror(pd,3) & 0x1FE0;
   crc ^= pd & 0xF000;
   crc ^= ror(pd,7) & 0x01E0;
   return crc;
}


/*
 * NAME: md_rspc_tod_rw()
 *
 * FUNCTION: Read or Write the DS1385 Real Time Clock
 *
 * Inputs:   addr = index of register to read or write
 *	     b = pointer to data byte to read or write
 *	     dir = NVREAD or NVWRITE
 *
 * EXECUTION ENVIRONMENT:  Process or interrupt (pinned routine)
 *
 * RETURN VALUE: None
 *
 */
int
md_rspc_tod_rw(uchar addr, uchar *b, int dir)
{
    int rc=0;
    volatile struct rspcsio *siop;
    int oldi;
    uchar data;
    
    if (INVALID == (siop = md_mapit(md_nio_bus, IO_RGN, 0, 0x1000)))
	return(EINVAL);
    data = *b;
    oldi = disable_lock(INTMAX, &md_nv_lock);
    siop->rtc_index = addr;
    __iospace_sync();
    if (dir == NVWRITE)
        siop->rtc_data = data;
    else
        data = siop->rtc_data;
    unlock_enable(oldi, &md_nv_lock);
    md_unmapit(md_nio_bus, siop);
    *b = data;
    return(rc);
}

#endif	/* _RSPC */

