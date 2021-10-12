/* @(#)23	1.37  src/bos/kernel/vmm/POWER/vmpfhdata.h, sysvmm, bos411, 9428A410j 7/11/94 10:11:34 */

#ifndef	_h_VMPFHDATA
#define	_h_VMPFHDATA

/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:
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

#include "mplock.h"

/*
 * control variables for VMM.
 */

struct pfhdata
{
	int		firstnf;	/* first non pinned page	    */
	int		sidfree;	/* first free sidtable entry	    */
	int		sidxmem;	/* first delete pending entry       */
	int		hisid;		/* max index + 1 of sids ever used  */
	int		numpout;	/* number of fblru page-outs        */
	int		numremote;	/* number of fblru remote page-outs */
	int		pfavail;	/* frames not pinned	           */
	int		lruptr; 	/* next candidate for replacement   */
	int		syncptr;	/* v_sync cursor		    */
	int		iotail; 	/* last pdt on i/o list             */
	int		npgspaces;	/* number of paging spaces */
	int		pdtlast;	/* PDT index last allocated from    */
	int		pdtmaxpg;	/* max index in PDT of paging device */
	int		pdtserver;      /* index in PDT of ds server          */
	int	 	minfree;	/* minimun pages free list (fblru) */
	int	 	maxfree;	/* maxfree pages free list (fblru) */
	uint	 	nxtscbnum;	/* next scb serial number	*/
	int	 	rpgcnt[RPTYPES];    /* repaging cnt */
	int	 	nreplaced[RPTYPES]; /* number of pages replaced */
	int	 	nrepaged[RPTYPES];  /* number of pages repaged */
	int		minperm;	/* no fileonly page steals
					 * if numperm < minperm */
	int		minpgahead;	/* min number of pageahead pages */
	int		maxpgahead;	/* max number of pageahead pages */
	int		kerkey;		/* key for system part of a segment */
	int	 	numpermio;	/* number of fblru non-w.s. pageouts */

	/* The following are the VMM wait lists, used to wait processes on
	 * specific events.
	 */
	struct thread  *freewait;	/* a free page frame		*/
	struct thread  *devwait;	/* all i/o on device to complete*/
	struct thread  *extendwait;	/* to extend an XPT		*/
	struct thread  *bufwait;	/* a free buf-struct to unhide	*/
	struct thread  *deletewait;	/* inherit/delete of segment	*/

	/* The following variables are used in enforcing paging space
	 * thresholds.
	 */
	int		npswarn;	/* free ps threshold before SIGDANGER */
	int		npskill;	/* free ps threshold before SIGKILL */
	int		nextwarn;	/* moving warn threshold */
	int		nextkill;	/* moving kill threshold */
	int		adjwarn;	/* warn threshold adjustment value */
	int		adjkill;	/* kill threshold adjustment value */
	int		npdtblks;	/* seq. blks alloc'd from pdtlast */
	int		maxpdtblks;	/* max blks to alloc from one pdt */
	int		numsched;	/* i/o's to schedule at one time  */
	int		freewake;	/* wake up free frame waitors     */

	struct thread  *dqwait;		/* wait list for f.s. disk quotas */

	/* The following variables are for managing Address Map Entries (AMEs).
	 */
	int		amefree;	/* first free ame entry		    */
	int		amexmem;	/* first delete pending ame         */
	int		hiame;		/* max index + 1 of ames ever used  */

	struct thread  *pgspwait;	/* wait list for paging space free  */

	/* For tracking memory holes during page-replacement.
	 */
	int		lruidx;		/* index in interval array	    */
	int		skiplru;	/* start of next memory hole	    */

	/* The following variables are for managing the Alias Page Table (APT).
	 */
	ushort		aptfree;	/* first free apt entry	*/
	ushort		aptlru;		/* next apt entry to replace */

	/* where the logs are
	 */
	int		logsidx[MAXLOGS];/* sid index of logs */

#ifdef _VMM_MP_EFF
	int		lrurequested;	/* type of lru to be run by the daemon */
	struct thread	*lrudaemon;	/* lru daemon anchor */
	union {
	int	cache_line[16];			/* we want only one lock per cache line */
                Simple_lock     pdtlock;        /* pdt global lock */
                }       l1;
        union {
                int             cache_line[16]; /* we want only one lock per cache line */
                Simple_lock     vmaplock;       /* vmaplock global lock */
                }       l2;
        union {
                int             cache_line[16]; /* we want only one lock per cache line */
                Simple_lock     amelock;        /* amelock global lock */
                }       l3;
        union {
                int             cache_line[16]; /* we want only one lock per cache line */
                Simple_lock     rptlock;        /* rptlock global lock */
                }       l4;
        union {
                int             cache_line[16]; /* we want only one lock per cache line */
                Simple_lock     alloclock;      /* scb free list and xpt alloc lock */
                }       l5;
        union {
                int             cache_line[16]; /* we want only one lock per cache line */
                Simple_lock     aptlock;	/* apt free list lock */
                }       l6;
#endif /* _VMM_MP_EFF */
};

/*
 * macros for referencing variables in pfhdata. it resides in vmmdseg
 */

#define pf_firstnf	vmmdseg.pf.firstnf
#define pf_sidfree	vmmdseg.pf.sidfree
#define pf_sidxmem   	vmmdseg.pf.sidxmem
#define pf_hisid	vmmdseg.pf.hisid
#define pf_numpout	vmmdseg.pf.numpout
#define pf_numremote	vmmdseg.pf.numremote
#define pf_pfavail	vmmdseg.pf.pfavail
#define pf_lruptr	vmmdseg.pf.lruptr
#define pf_minfree	vmmdseg.pf.minfree
#define pf_maxfree	vmmdseg.pf.maxfree
#define pf_syncptr	vmmdseg.pf.syncptr
#define pf_npgspaces	vmmdseg.pf.npgspaces
#define pf_pdtlast	vmmdseg.pf.pdtlast
#define pf_pdtmaxpg	vmmdseg.pf.pdtmaxpg
#define pf_pdtserver	vmmdseg.pf.pdtserver
#define pf_iotail	vmmdseg.pf.iotail
#define pf_logsidx	vmmdseg.pf.logsidx
#define pf_iopfh	vmmdseg.pf.iopfh
#define pf_freewait	vmmdseg.pf.freewait
#define pf_extendwait	vmmdseg.pf.extendwait
#define pf_bufwait	vmmdseg.pf.bufwait
#define pf_devwait	vmmdseg.pf.devwait
#define pf_deletewait	vmmdseg.pf.deletewait
#define pf_pgspwait  	vmmdseg.pf.pgspwait
#define pf_nreplaced	vmmdseg.pf.nreplaced
#define pf_nrepaged	vmmdseg.pf.nrepaged
#define	pf_npswarn	vmmdseg.pf.npswarn
#define	pf_npskill	vmmdseg.pf.npskill
#define	pf_nextwarn	vmmdseg.pf.nextwarn
#define	pf_nextkill	vmmdseg.pf.nextkill
#define	pf_adjwarn	vmmdseg.pf.adjwarn
#define	pf_adjkill	vmmdseg.pf.adjkill
#define	pf_minperm	vmmdseg.pf.minperm
#define	pf_minpgahead	vmmdseg.pf.minpgahead
#define	pf_maxpgahead	vmmdseg.pf.maxpgahead
#define	pf_kerkey	vmmdseg.pf.kerkey
#define	pf_numpermio	vmmdseg.pf.numpermio
#define	pf_npdtblks	vmmdseg.pf.npdtblks
#define	pf_maxpdtblks	vmmdseg.pf.maxpdtblks
#define	pf_numsched	vmmdseg.pf.numsched
#define	pf_freewake	vmmdseg.pf.freewake
#define	pf_dqwait	vmmdseg.pf.dqwait
#define	pf_nxtscbnum	vmmdseg.pf.nxtscbnum
#define	pf_rpgcnt	vmmdseg.pf.rpgcnt
#define pf_amefree	vmmdseg.pf.amefree
#define pf_amexmem   	vmmdseg.pf.amexmem
#define pf_hiame	vmmdseg.pf.hiame
#define	pf_lruidx	vmmdseg.pf.lruidx
#define	pf_skiplru	vmmdseg.pf.skiplru
#define pf_aptfree	vmmdseg.pf.aptfree
#define pf_aptlru	vmmdseg.pf.aptlru
#ifdef _VMM_MP_EFF
#define lru_requested	vmmdseg.pf.lrurequested
#define lru_daemon	vmmdseg.pf.lrudaemon
#define pdt_lock	vmmdseg.pf.l1.pdtlock
#define vmap_lock	vmmdseg.pf.l2.vmaplock
#define ame_lock	vmmdseg.pf.l3.amelock
#define rpt_lock	vmmdseg.pf.l4.rptlock
#define alloc_lock	vmmdseg.pf.l5.alloclock
#define apt_lock	vmmdseg.pf.l6.aptlock
#endif /* _VMM_MP_EFF */

#endif	/* _h_VMPFHDATA */
