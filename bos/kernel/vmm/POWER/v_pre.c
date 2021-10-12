static char sccsid[] = "@(#)94	1.33  src/bos/kernel/vmm/POWER/v_pre.c, sysvmm, bos41J, 9524D_all 6/13/95 15:59:47";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:   v_prexxx
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * wrap routines to takes locks for vmm critical section routines 
 * that can be called both from base level (in vcs_xxx) and directly
 * from a critical section where the locks are already held.
 */

#ifdef _POWER_MP

#include "vmsys.h"
#include "mplock.h"

/*
 * NAME:	v_preallocfree(sid, frag, option)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_preallocfree(sid, frag, option)
int sid;
frag_t frag;
int option;
{
        int rc;
	int pdtx = scb_devid(STOI(sid));

        VMM_MPSAFE_LOCK();
	FS_MPLOCK_S(pdtx);
        rc = v_allocfree(sid,frag,option);
	FS_MPUNLOCK_S(pdtx);
        VMM_MPSAFE_UNLOCK();

        return rc;
}

/*
 * NAME:	v_prefrealloc(nb, nfr, nfrags)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prefrealloc(nb, nfr, nfrags)
int nb;
int * nfr;
int * nfrags;
{
        int rc;

        VMM_MPSAFE_LOCK();
        rc = v_frealloc(nb, nfr, nfrags);
        VMM_MPSAFE_UNLOCK();

        return rc;
}

/*
 * NAME:	v_pregetcio(nmax, array, wait)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_pregetcio(nmax, array, wait)
int nmax;
int * array;
int wait;
{
        int rc;

        VMM_MPSAFE_LOCK();
        rc = v_getcio(nmax, array, wait);
        VMM_MPSAFE_UNLOCK();

        return rc;
}

/*
 * NAME:	v_prerequeio(nframes, array)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prerequeio(nframes, array)
int nframes;
int * array;
{
        int rc;

        VMM_MPSAFE_LOCK();
        rc = v_requeio(nframes, array);
        VMM_MPSAFE_UNLOCK();

        return rc;
}

/*
 * NAME:	v_prefreefrags(ip)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prefreefrags(ip)
struct inode * ip;
{
        int rc;

        VMM_MPSAFE_LOCK();
	LW_MPLOCK_S();
        rc = v_freefrags(ip);
	LW_MPUNLOCK_S();
        VMM_MPSAFE_UNLOCK();

        return rc;
}

/*
 * NAME:	v_premovedaddr(bsid, pagex, oldfrag, newfrag)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_premovedaddr(bsid, pagex, oldfrag, newfrag)
int bsid;
int pagex;
frag_t oldfrag;
frag_t newfrag;
{
        int rc;
	int bsidx = STOI(bsid);

        VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(bsidx);
        rc = v_movedaddr(bsid, pagex, oldfrag, newfrag);
	SCB_MPUNLOCK_S(bsidx);
        VMM_MPSAFE_UNLOCK();

        return rc;
}

/*
 * NAME:	v_precleardata(sid,lastp)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_precleardata(sid,lastp)
int	sid; 	/* segment id */
int	lastp;	/* value for scb_minvpn */
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	ALLOC_MPLOCK_S();
	rc = v_cleardata(sid,lastp);
	ALLOC_MPUNLOCK_S();
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_precreate(sid,type,device,size)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_precreate(sid,type,device,size)
int * sid;      /* set to sid of created segment */
int   type;     /* type of segment to create     */
union {
        dev_t   dev;    /* dev_t of block device */
        int ( *ptr)();  /* pointer to strategy routine */
        } device;
int   size;     /* user region size or size of initial xpt */
{
	int rc;

	VMM_MPSAFE_LOCK();
	ALLOC_MPLOCK_S();
	rc = v_create(sid,type,device,size);
	ALLOC_MPUNLOCK_S();
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_predelete(sid,pfirst,npages)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_predelete(sid,pfirst,npages)
int sid;        /* segment id */
int pfirst;     /* first page */
int npages;     /* number of pages */
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	rc = v_delete(sid,pfirst,npages);
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_predeletelws(sid)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_predeletelws(sid)
int sid;
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	LW_MPLOCK_S();
	rc = v_deletelws(sid);
	LW_MPUNLOCK_S();
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_preextendag(p0, nfrags, fragno, pdtx)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_preextendag(p0, nfrags, fragno, pdtx)
struct vmdmap *p0;
uint  nfrags;
uint  fragno;
int pdtx;
{
	int rc;

	VMM_MPSAFE_LOCK();
	FS_MPLOCK_S(pdtx);
	rc = v_extendag(p0, nfrags, fragno, pdtx);
	FS_MPUNLOCK_S(pdtx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prefreeseg(sidx,xmem)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prefreeseg(sidx,xmem)
int     sidx;   /* index in scb table */
int     xmem;   /* xmem parameter     */
{
	int rc;

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	ALLOC_MPLOCK_S();
	rc = v_freeseg(sidx,xmem);
	ALLOC_MPUNLOCK_S();
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_predqlock(dp)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_predqlock(dp)
struct dquot *dp;
{
	int rc;

	VMM_MPSAFE_LOCK();
	rc = v_dqlock(dp);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prefrlock (tid)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prefrlock (tid)
int tid;          
{
	int rc;

	VMM_MPSAFE_LOCK();
	rc = v_frlock (tid);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_pregetxpt (blk_size,touch)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_pregetxpt (blk_size,touch)
uint blk_size;    /* specifies free list to use  */
uint touch;   /* 0 ==> DO NOT touch XPT blk. 1 ==> touch XPT blk */
{
	int rc;

	VMM_MPSAFE_LOCK();
	ALLOC_MPLOCK_S();
	rc = v_getxpt (blk_size,touch);
	ALLOC_MPUNLOCK_S();
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_pregettblk (tid)
 *
 * FUNCTION:	wrapper routine
 */

v_pregettblk (tid)
int *tid;                       /* tid value and index of tblk */
{
	int rc;

	VMM_MPSAFE_LOCK();
	LW_MPLOCK_S();
	rc = v_gettblk (tid);
	LW_MPUNLOCK_S();
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_pregetvmap()
 *
 * FUNCTION:	wrapper routine
 *
 */

v_pregetvmap()
{
	int rc;

	VMM_MPSAFE_LOCK();
	VMAP_MPLOCK_S();
	rc = v_getvmap();
	VMAP_MPUNLOCK_S();
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_pregrowxpt(sidx, pno)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_pregrowxpt(sidx, pno)
uint   sidx;   /* index of scb */
uint   pno;    /* virtual page number to cover.   */
{
	int rc;

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	rc = v_growxpt(sidx, pno);
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_preinherit(psid,pfirst,npages)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_preinherit(psid,pfirst,npages)
int     psid;   /* parent segment id */
int     pfirst; /* first page number */
int     npages; /* number of pages to release */
{
	int rc;

	VMM_MPSAFE_LOCK();
	rc = v_inherit(psid,pfirst,npages);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_premakep(sid , pagex)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_premakep(sid , pagex)
int	sid;
int	pagex;
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	rc = v_makep(sid , pagex);
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_premakelogp(sid, nextpno, lpage)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_premakelogp(sid, nextpno, lpage)
int	sid;
int	nextpno;
int	lpage;
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	rc = v_makelogp(sid, nextpno, lpage);
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prewritelogp(sid, cpno, ppongpno, release)
 *
 * FUNCTION:	wrapper routine
 *
 */
v_prewritelogp(int sid, int cpno, int ppongpno, int release)
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	rc = v_writelogp(sid, cpno, ppongpno, release);
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_premapd(sid,daddr)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_premapd(sid,daddr)
int	sid;
int	daddr;
{
	int rc;
	int pdtx = scb_devid(STOI(sid));

	VMM_MPSAFE_LOCK();
	FS_MPLOCK_S(pdtx);
	rc = v_mapd(sid,daddr);
	FS_MPUNLOCK_S(pdtx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_premapv(tsid,mapx,tfirst,npages)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_premapv(tsid,mapx,tfirst,npages)
int     tsid;
int     mapx;
int     tfirst;
int     npages;
{
	int rc;
	int tsidx = STOI(tsid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(tsidx);
	rc = v_mapv(tsid,mapx,tfirst,npages);
	SCB_MPUNLOCK_S(tsidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_premvfork(psid,fsid,ssid)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_premvfork(psid,fsid,ssid)
int     psid;   /* segment to which pages are moved to. */
int     fsid;   /* segment from which pages are moved from */
int     ssid;   /* sibling of forker (child) */
{
	int rc;
	int fsidx = STOI(fsid);
	int psidx = STOI(psid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK(fsidx);
	SCB_MPLOCK(psidx);
	rc = v_mvfork(psid,fsid,ssid);
	SCB_MPUNLOCK(psidx);
	SCB_MPUNLOCK(fsidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prepromote(psidx)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prepromote(psidx)
int psidx;      /* index in scb of parent segment */
{
	int rc;

	VMM_MPSAFE_LOCK();
	rc = v_promote(psidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prepdtdelete(devtype,devid,bp)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prepdtdelete(devtype,devid,bp)
int  devtype; 
union {
	dev_t   dev;  /* dev_t of block device */
	int ( *ptr)();  /* pointer to strategy routine */
	} devid;
struct buf **bp; /* set to pointer to first buf-struct */
{
	int rc;

	VMM_MPSAFE_LOCK();
	rc = v_pdtdelete(devtype,devid,bp);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prepbitfree(mapsid,ptr,logage,option)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prepbitfree(mapsid,ptr,logage,option)
int mapsid;     	/* segment id of map */
struct vmdlist *ptr; 	/* pointer to list of blocks to free */
int logage;     	/* applicable logage */ 
int option;     	/* V_PWMAP to update work map too */
{
	int rc;
	int pdtx = scb_devid(STOI(mapsid));

	VMM_MPSAFE_LOCK();
	FS_MPLOCK_S(pdtx);
	rc = v_pbitfree(mapsid,ptr,logage,option);
	FS_MPUNLOCK_S(pdtx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prepbitalloc(mapsid,ptr,logage)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prepbitalloc(mapsid,ptr,logage)
int mapsid;     	/* segment id of map */
struct vmdlist *ptr; 	/* pointer to list of blocks to free */
int logage;     	/* applicable logage */ 
{
	int rc;
	int pdtx = scb_devid(STOI(mapsid));

	VMM_MPSAFE_LOCK();
	FS_MPLOCK_S(pdtx);
	rc = v_pbitalloc(mapsid,ptr,logage);
	FS_MPUNLOCK_S(pdtx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prepin(sid,pno,typ)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prepin(sid,pno,typ)
int     sid;    /* segment id */
int     pno;    /* page number in segment */
int     typ;	/* pin type (short, long term) */
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	rc = v_pin(sid,pno,typ);
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_preallociblk(sid, ppage)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_preallociblk(sid, ppage)
int 	sid;
int	*ppage;
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	FS_MPLOCK_S(scb_devid(sidx));
	rc = v_allociblk(sid, ppage);
	FS_MPUNLOCK_S(scb_devid(sidx));
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prefreeiblk(sid, indblk)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prefreeiblk(sid, indblk)
int sid;
int indblk;
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	FS_MPLOCK_S(scb_devid(sidx));
	rc = v_freeiblk(sid, indblk);
	FS_MPUNLOCK_S(scb_devid(sidx));
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prerelease(sid,pfirst,npages,flags)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prerelease(sid,pfirst,npages,flags)
int     sid;    /* segment id */
int     pfirst; /* first page number */
int     npages; /* number of pages to release */
int     flags;  /* state handling flags */
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	rc = v_release(sid,pfirst,npages,flags);
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_presetsize(ip, newsize)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_presetsize(ip, newsize)
struct inode * ip;
uint newsize;
{
	int rc;

	VMM_MPSAFE_LOCK();
	rc = v_setsize(ip, newsize);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prewrite(sid,pfirst,npages,force)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prewrite(sid,pfirst,npages,force)
int     sid;    /* base segment id */
int     pfirst; /* first page number */
int     npages; /* number of pages */
int     force;  /* 1 to force i/o for journalled segments */
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	rc = v_write(sid,pfirst,npages,force);
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_preprotect(sid,pno,key)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_preprotect(sid,pno,key)
int     sid;    /* segment id */
int     pno;    /* page number in segment */
int     key;    /* key to be used for page */
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	rc = v_protect(sid,pno,key);
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_preprotectp(sid,pagex,key)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_preprotectp(sid,pagex,key)
int     sid;    /* base segment id */
int     pagex;  /* page number in segment */
int     key;    /* key to be used for page */
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK(sidx);
	rc = v_protectp(sid,pagex,key);
	SCB_MPUNLOCK(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_precpfork(fsid,ssid,psid,pno,pscount)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_precpfork(fsid,ssid,psid,pno,pscount)
int     fsid;   /* segment id of forker */
int     ssid;   /* segment id of sibling */
int	psid;	/* segment id of parent  */
int     pno;    /* page number to copy   */
int *	pscount; /* count of paging space blocks moved */
{
	int rc;
	int fsidx = STOI(fsid);
	int ssidx = STOI(ssid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(fsidx);
	SCB_MPLOCK_S(ssidx);
	rc = v_cpfork(fsid,ssid,psid,pno,pscount);
	SCB_MPUNLOCK_S(ssidx);
	SCB_MPUNLOCK_S(fsidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_preflush(sid,pfirst,npages)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_preflush(sid,pfirst,npages)
int     sid;    /* segment id */
int     pfirst; /* first page number */
int     npages; /* number of pages to flush */
{
	int rc;
	int sidx = STOI(sid);

        /* nothing to do ?
         */
        if (npages <= 0)
                return(0);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	rc = v_flush(sid,pfirst,npages);
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_premovep(sid,sp,dest)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_premovep(sid,sp,dest)
uint sid;
struct vmsrclist *sp;
uint dest;
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	rc = v_movep(sid,sp,dest);
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prefreedq(ip,nfrags,fperpage)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prefreedq(ip,nfrags,fperpage)
struct inode *ip; 
int nfrags;
int fperpage;
{
	int rc;
	int type, pdtx = 0;
	struct dquot *dp;

	VMM_MPSAFE_LOCK();

	for (type = 0; type < MAXQUOTAS; type++)
        {
                if ((dp = ip->i_dquot[type]) == NODQUOT)
                        continue;

		pdtx = v_devpdtx(D_FILESYSTEM,dp->dq_jmp->jm_dev);
		if (pdtx > 0)
			FS_MPLOCK_S(pdtx);
		break;
	}

	rc = v_freedq(ip,nfrags,fperpage);

	if(pdtx)
		FS_MPUNLOCK_S(pdtx);

	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_preunlockdq (dp)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_preunlockdq (dp)
struct dquot *dp; 
{
	int rc;

	VMM_MPSAFE_LOCK();
	rc = v_unlockdq (dp);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_preconfig(cfgp)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_preconfig(cfgp)
struct vmconfig *cfgp;
{
	int rc;

	VMM_MPSAFE_LOCK();
	rc = v_config(cfgp);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_preclrfrag(sid,offset,nbytes)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_preclrfrag(sid,offset,nbytes)
uint sid;
uint offset;
int nbytes;
{
	int rc;

	VMM_MPSAFE_LOCK();
	rc = v_clrfrag(sid,offset,nbytes);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_predirfrag(sid,pno,nfrags)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_predirfrag(sid,pno,nfrags)
int sid;
int pno;
int nfrags;
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	rc = v_dirfrag(sid,pno,nfrags);
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_premovefrag(sid,sp,dest,nbytes,touched)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_premovefrag(sid,sp,dest,nbytes,touched)
uint sid;
struct vmsrclist *sp;
uint dest;
int nbytes;
uint *touched;
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	rc = v_movefrag(sid,sp,dest,nbytes,touched);
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prerelfrag(sid,pno,nfrags,oldfrags)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prerelfrag(sid,pno,nfrags,oldfrags)
int sid;
int pno;
int nfrags;
frag_t *oldfrags;
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	rc = v_relfrag(sid,pno,nfrags,oldfrags);
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_predevpdtx(devtype,devid)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_predevpdtx(devtype,devid)
int devtype; 
union {
	dev_t   dev;  /* dev_t of block device */
	int ( *ptr)();  /* pointer to strategy routine */
	} devid;
{
	int rc;

	VMM_MPSAFE_LOCK();
	rc = v_devpdtx(devtype,devid);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prefreescb(sidx,xmem)
 *
 * FUNCTION:	wrapper routine
 *
 * 	we must take the vmm mpsafe lock spin since
 *	v_freescb() can return VM_WAIT and thus we cannot block.
 *	This special use of VM_WAIT is handled by the fst assembler path.
 */

v_prefreescb(sidx,xmem)
int     sidx;   /* index in scb table */
int	xmem;	/* xmem flag	*/
{
	int rc;

	VMM_MPSAFE_SLOCK();
	SCB_MPLOCK(sidx);
	ALLOC_MPLOCK();
	rc = v_freescb(sidx,xmem);
	ALLOC_MPUNLOCK();
	SCB_MPUNLOCK(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prefrtblk(tid)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prefrtblk(tid)
int     tid;  /* index in tblk array = tid reg value */
{
	int rc;

	VMM_MPSAFE_LOCK();
	LW_MPLOCK();
	rc = v_frtblk(tid);
	LW_MPUNLOCK();
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_preiowait(sid)
 *
 * FUNCTION:	wrapper routine
 *
 * 	we must take the vmm mpsafe lock spin since
 *	v_iowait() can return VM_WAIT and thus we cannot block.
 *	This special use of VM_WAIT is handled by the fst assembler path.
 */

v_preiowait(sid)
int	sid;
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_SLOCK();
	SCB_MPLOCK(sidx);
	rc = v_iowait(sid);
	SCB_MPUNLOCK(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_preinterrupt(pt)
 *
 * FUNCTION:	wrapper routine
 *
 * 	v_interrupt is a real fst critical section
 *	and does not need any serialization
 */

v_preinterrupt(pt)
struct thread *pt;
{
	int rc;

	rc = v_interrupt(pt);

	return(rc);
}

/*
 * NAME:	v_prelockseg (sid)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prelockseg (sid)
int sid;
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK(sidx);
	rc = v_lockseg (sid);
	SCB_MPUNLOCK(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prepdtinsert(devtype,devid,bp,fragsize,comptype)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prepdtinsert(devtype,devid,bp,fragsize,comptype)
int	devtype;
union {
	dev_t   dev;  	/* dev_t of block device */
	int ( *ptr)();  /* pointer to strategy routine */
	} devid;
struct buf *bp;         /* pointer to first of list of buf structs */
int fragsize;		/* device fragment size in bytes */
ushort comptype;	/* compression type */
{
	int rc;

	VMM_MPSAFE_LOCK();
	PDT_MPLOCK();
	rc = v_pdtinsert(devtype,devid,bp,fragsize,comptype);
	PDT_MPUNLOCK();
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_preqmodify(sid)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_preqmodify(sid)
int sid;	/* segment id */
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK(sidx);
	rc = v_qmodify(sid);
	SCB_MPUNLOCK(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_preqpages(sid)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_preqpages(sid)
int sid;	/* segment id */
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK(sidx);
	rc = v_qpages(sid);
	SCB_MPUNLOCK(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_presetlog(sid, pno, logage, dirty)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_presetlog(sid, pno, logage, dirty)
int sid;
int pno;
int logage;
int dirty;
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK(sidx);
	rc = v_setlog(sid, pno, logage, dirty);
	SCB_MPUNLOCK(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_preunlockseg (sid,dreset)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_preunlockseg (sid,dreset)
int sid;
int dreset;
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK(sidx);
	rc = v_unlockseg (sid,dreset);
	SCB_MPUNLOCK(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_predefer(sid)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_predefer(sid)
int sid;
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	rc = v_defer(sid);
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prepscount(sid)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prepscount(sid)
int	sid;
{
	int rc;

	VMM_MPSAFE_LOCK();
	rc = v_pscount(sid);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_preinactive(sid)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_preinactive(sid)
int     sid;    /* segment id */
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK(sidx);
	rc = v_inactive(sid);
	SCB_MPUNLOCK(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prepowait(sidx)
 *
 * FUNCTION:	wrapper routine
 *
 * 	we must take the vmm mpsafe lock spin since
 *	v_powait() can return VM_WAIT and thus we cannot block.
 *	This special use of VM_WAIT is handled by the fst assembler path.
 */

v_prepowait(sidx)
uint sidx;
{
	int rc;

	VMM_MPSAFE_SLOCK();
	SCB_MPLOCK(sidx);
	rc = v_powait(sidx);
	SCB_MPUNLOCK(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

caddr_t
/*
 * NAME:	v_pregetame()
 *
 * FUNCTION:	wrapper routine
 *
 */

v_pregetame()
{
	int rc;

	VMM_MPSAFE_LOCK();
	AME_MPLOCK_S();
	rc = v_getame();
	AME_MPUNLOCK_S();
	VMM_MPSAFE_UNLOCK();

	return((caddr_t)rc);
}

/*
 * NAME:	v_prefreeame(ame)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prefreeame(ame)
caddr_t ame;
{
	int rc;

	VMM_MPSAFE_LOCK();
	AME_MPLOCK();
	rc = v_freeame(ame);
	AME_MPUNLOCK();
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prerelalias(asid,afirst,ssid,sfirst,slast)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_prerelalias(asid,afirst,ssid,sfirst,slast)
int     asid;   /* alias sid */
int     afirst; /* alias starting page number */
int     ssid;   /* source sid */
int     sfirst; /* first page number -- pagex */
int     slast;  /* last page number -- pagex */
{
	int rc;
	int sidx = STOI(ssid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK(sidx);
	rc = v_relalias(asid,afirst,ssid,sfirst,slast);
	SCB_MPUNLOCK(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_preprotectap(asid,afirst,ssid,sfirst,npages,key)
 *
 * FUNCTION:	wrapper routine
 * 
 * NOTES:     
 *		MPSAFE:-
 *		This function has 6 parameters and and we can
 *              not use FSTCALL as it will set up for bactracking
 *		and currently backtrack calls can only have 
 *      	5 parameters. So, We use FSTRCALL and take
 *		the MPSAFE spin lock. 
 *
 */

v_preprotectap(asid,afirst,ssid,sfirst,npages,key)
int     asid;   /* alias sid */
int     afirst; /* alias first page number */
int     ssid;   /* source sid */
int     sfirst; /* first page number -- pagex */
int     npages; /* number of pages  */
int     key;    /* page protect key */
{
	int rc;
	int ssidx = STOI(ssid);

	VMM_MPSAFE_SLOCK();
	SCB_MPLOCK(ssidx);
	rc = v_protectap(asid,afirst,ssid,sfirst,npages,key);
	SCB_MPUNLOCK(ssidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_preinvalidate(sid,pfirst,npages)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_preinvalidate(sid,pfirst,npages)
int     sid;    /* segment id */
int     pfirst; /* first page number */
int     npages; /* number of pages to release */
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	rc = v_invalidate(sid,pfirst,npages);
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_pregettlock(sid, vaddr, length)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_pregettlock(sid, vaddr, length)
int sid;                   
int vaddr;               
int length;            
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	SCB_MPLOCK_S(sidx);
	LW_MPLOCK_S();
	rc = v_gettlock(sid, vaddr, length);
	LW_MPUNLOCK_S();
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}

/*
 * NAME:	v_prevmm_lock(struct vmmlock *lock, uint baselevel)
 *
 * FUNCTION:	wrapper routine
 *
 *	Serialize internally by the vmm_lock_lock
 */

v_prevmm_lock(struct vmmlock *lock, uint baselevel)
{
	int rc;

	rc = v_vmm_lock(lock, baselevel);

	return(rc);
}

/*
 * NAME:	v_prevmm_unlock(struct vmmlock *lock, uint baselevel)
 *
 * FUNCTION:	wrapper routine
 *
 *	Serialize internally by the vmm_lock_lock
 */

v_prevmm_unlock(struct vmmlock *lock, uint baselevel)
{
	int rc;

	rc = v_vmm_unlock(lock, baselevel);

	return(rc);
}

/*
 * NAME:	v_presync(ip, iplog)
 *
 * FUNCTION:	wrapper routine
 *
 */

v_presync(ip, iplog)
struct inode *ip;
struct inode *iplog;
{
	int rc;

	VMM_MPSAFE_LOCK();
	rc = v_sync(ip, iplog);
	VMM_MPSAFE_UNLOCK();

	return(rc);
}


/*
 * NAME:	v_prelimits(sidx, uplim, downlim)
 *
 * FUNCTION:	wrapper routine
 *
 */
v_prelimits(sidx, uplim, downlim)
int sidx, uplim, downlim;
{
	int rc;
	VMM_MPSAFE_LOCK();
	SCB_MPLOCK(sidx);
	rc = v_limits(sidx, uplim, downlim);
	SCB_MPUNLOCK(sidx);
	VMM_MPSAFE_UNLOCK();

	return rc;
}

/*
 * NAME:	v_prepsearlyalloc(sidx)
 *
 * FUNCTION:	wrapper routine
 *
 */
v_prepsearlyalloc(sidx)
int sidx;
{
	int rc;
	VMM_MPSAFE_LOCK();
	SCB_MPLOCK(sidx);
	rc = v_psearlyalloc(sidx);
	SCB_MPUNLOCK(sidx);
	VMM_MPSAFE_UNLOCK();

	return rc;
}

/*
 * NAME:	v_prepslatealloc(sidx)
 *
 * FUNCTION:	wrapper routine
 *
 */
v_prepslatealloc(sidx)
int sidx;
{
	int rc;
	VMM_MPSAFE_LOCK();
	SCB_MPLOCK(sidx);
	rc = v_pslatealloc(sidx);
	SCB_MPUNLOCK(sidx);
	VMM_MPSAFE_UNLOCK();

	return rc;
}

/*
 * NAME:	v_preunpin(sid,pno,typ)
 *
 * FUNCTION:	wrapper routine
 *
 */
v_preunpin(sid,pno,typ)
int     sid;    /* segment id */
int     pno;    /* page number in segment */
int     typ;	/* pin type (short, long term) */
{
	int rc;
	int sidx = STOI(sid);

	VMM_MPSAFE_LOCK();
	/* called in v_dalloc which can fault */
	SCB_MPLOCK_S(sidx);
	rc = v_unpin(sid,pno,typ);
	SCB_MPUNLOCK_S(sidx);
	VMM_MPSAFE_UNLOCK();

	return rc;
}

/*
 * NAME:	v_pregetfree()
 *
 * FUNCTION:	wrapper routine
 *
 */
v_pregetfree()
{
	int rc;

	VMM_MPSAFE_LOCK();
	rc = v_getfree();
	VMM_MPSAFE_UNLOCK();

	return rc;
}

/*
 * NAME:	v_preinsfree(nfr)
 *
 * FUNCTION:	wrapper routine
 *
 */
v_preinsfree(nfr)
int nfr;
{
	VMM_MPSAFE_LOCK();
	v_insfree(nfr);
	VMM_MPSAFE_UNLOCK();

	return 0;
}

/*
 * NAME:        v_presyncwait()
 *
 * FUNCTION:    wrapper routine
 *
 */

v_presyncwait()
{
	int rc;
        VMM_MPSAFE_LOCK();
        LW_MPLOCK();
        rc = v_syncwait();
        LW_MPUNLOCK();
        VMM_MPSAFE_UNLOCK();
	return rc;
}

/*
 * NAME:	v_hfblru()
 *
 * FUNCTION:	wrapper routine
 *
 */
v_prehfblru(newfree, criteria)
int newfree, criteria;
{
	int rc;

	VMM_MPSAFE_LOCK();
	rc = v_hfblru(newfree, criteria);
	VMM_MPSAFE_UNLOCK();

	return rc;
}

/*
 * NAME:	v_prelruwait()
 *
 * FUNCTION:	wrapper routine
 *
 */
v_prelruwait()
{
	VMM_MPSAFE_LOCK();
	v_lruwait();
	VMM_MPSAFE_UNLOCK();

	return 0;
}

#ifdef _VMM_MP_EFF
/*
 * NAME:	v_prelru()
 *
 * FUNCTION:	wrapper routine
 *
 */
v_prelru()
{
	int rc;

	VMM_MPSAFE_LOCK();
	PPDA->lru = VM_INLRU;
	rc = v_lru();
	PPDA->lru = VM_NOBACKT;
	VMM_MPSAFE_UNLOCK();

	return rc;
}

#endif /* _VMM_MP_EFF */

#endif /* _POWER_MP */
