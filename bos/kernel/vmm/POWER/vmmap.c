static char sccsid[] = "@(#)21	1.16  src/bos/kernel/vmm/POWER/vmmap.c, sysvmm, bos411, 9428A410j 3/28/94 17:04:53";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: 	vm_map
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * code to implement vm_map. runs at normal kernel process
 * level.
 */

#include "vmsys.h"
#include <sys/inline.h>
#include <sys/errno.h>

/*
 * vm_map(saddr,taddr,nbytes)
 *
 * maps onto an interval of pages specified by taddr and
 * nbytes an equal size interval of pages in the source
 * specified by saddr. saddr and taddr are 32-bit byte
 * addresses . the pages which are mapped onto are those
 * which intersect the byte range [taddr, taddr+nbytes -1].
 * the first page of the source is the page containing
 * saddr. all of the pages in the target must be in the
 * same segment and similarly for the source.
 *
 * the target segment must be a working storage segment.
 * the source segment may be any kind of segment.
 *
 * the previous contents ot the target pages are released
 * (see vm_release). a subsequent reference to a  target
 * page causes a copy of the page to be made from the
 * source page at the time of the reference. if the target
 * page is modified, the value of the target page is as
 * modified; otherwise any subsequent page fault is satisfied
 * by making a copy of the source page at that time.
 *
 * INPUT PARAMETERS
 *
 * (1) saddr is the address of a byte in the first source page.
 *
 * (2) taddr is the address of a byte in the first target page.

 * (3) nbytes is the number of bytes starting at taddr. pages
 *     which intersect the interval [taddr, taddr + nbytes -1]
 *     are mapped onto.
 *
 *  RETURN  VALUES
 *
 *      0       - ok
 *
 *      EINVAL  -  no segment or i/o segment or all pages not
 *                 in the same segment or page limits exceeded.
 *                 or nbytes < 0.
 *
 *      ENOMEM -   no memory for allocating external page tables.
 */

vm_map(saddr,taddr,nbytes)
uint saddr;     /* address of first byte in source */
uint taddr;     /* address of first byte in target */
int  nbytes;    /* number of bytes to map          */
{

        int npages,p,rc,srsave,mapx,first,last;
        int s,soff,sfirst,slast,ssid, ssidx;
        int toff,tfirst,tlast,tsid,tsidx;

        /* check for silly values of nbytes
         */
        if(nbytes <= 0)
        {
                rc = (nbytes < 0) ? EINVAL : 0;
                return(rc);
        }

        /* calculate last pages and npages.
         */
        toff = taddr & SOFFSET;
        tfirst = toff >> L2PSIZE;
        tlast = (toff + nbytes - 1) >> L2PSIZE;
        npages = tlast - tfirst + 1;
        soff = saddr & SOFFSET;
        sfirst = (soff & SOFFSET) >> L2PSIZE;
        slast = sfirst + npages - 1;

        /* all pages must be same segment
         */
        if(slast >= SEGSIZE/PSIZE || tlast >= SEGSIZE/PSIZE )
                return(EINVAL);

        /* get srvals. check validity of sids.
         */
        tsid = SRTOSID(mfsri(taddr));
        ssid = SRTOSID(mfsri(saddr));
        if (ssid == INVLSID || tsid == INVLSID)
                return(EINVAL);

        /* check that source segment is not i/o segment or is
         * present
         */
        if (mfsri(saddr) & IOSEGMENT)
                return(EINVAL);

        /* make vmmdsseg addressable.
         */
        srsave = chgsr(VMMSR,vmker.vmmsrval);

        /* check that target is working storage segment and that
         * limits aren't exceeded.
         */
        tsidx = STOI(tsid);

	if (!scb_wseg(tsidx) || tlast > scb_uplim(tsidx))
        {
                (void)chgsr(VMMSR,srsave);
                return(EINVAL);
        }
        (void)chgsr(VMMSR,srsave);

        /* allocate a vmapblk
         */
        if ((mapx = vcs_getvmap()) == 0)
                return(ENOMEM);

        /* make ptaseg addressable. fill in vmap block.
         * note that when the source and target segments are the
         * same the value in the vmapblk is set to INVLSID. this
         * is a conventional value used for self-referential maps
         * which allows xpt to be inherited/transferred in fork
         * processing.
	 * No need to acquire the vmap lock since the just got
	 * a new vmap block which is not yet associated w/ an
	 * xpt and thus you not be access by another thread.
         */

        srsave = chgsr(PTASR,vmker.ptasrval);
        pta_vmap[mapx].torg = tfirst;
        pta_vmap[mapx].sorg = sfirst;
        pta_vmap[mapx]._u.sid = (ssid != tsid) ? ssid : INVLSID ;
        pta_vmap[mapx].count = 0;
        (void)chgsr(PTASR,srsave);

        /*
         * do the mapping.
         * pages are processed in groups of pages covered by
         * a single xpt direct block so that mapping operation
         * appears to be atomic. 
         */

        for(p = tfirst; p <= tlast; p += npages)
        {
                /* calculate number of pages from p to end of
                 * interval covered by same direct xpt block.
                 */
                first  = p & (~(XPTENT - 1));
                last   = MIN(first + XPTENT - 1,tlast);
                npages = last - p + 1;

                /* map the pages
                 */
                if( rc = vcs_mapv(tsid,mapx,p,npages))
			{
			vcs_release(tsid,tfirst,p - tfirst,V_NOKEEP);
                        return(rc);
			}
        }

        return 0;

}
