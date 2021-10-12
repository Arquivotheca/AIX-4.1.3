/* @(#)71       1.7.1.5  src/bos/kernext/disp/gem/inc/gem_ddmac.h, sysxdispgem, bos411, 9428A410j 1/19/93 12:41:43 */
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		BEGIN_CRIT
 *		END_CRIT
 *		GETFIFOPTRS
 *		GM_LOCK_DEV
 *		GM_UNLOCK_DEV
 *		LOCK
 *		LOCK_CXTSLOT
 *		LOCK_SLOTS
 *		PUTFIFOPTRS
 *		UNLOCK_CXTSLOT
 *		UNLOCK_SLOTS
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/***********************************************************************/
/*;CHANGE HISTORY                                                      */
/*;LW 09/13/89  Added xxxFIFOPTRS macros                               */
/*;             Deleted COPY_DD_IN/OUT macros                          */
/*;MC 09/14/89  Added "stubs" for BEGIN_ and END_CRIT                @1*/
/*;LW 10/11/89  Fixed size used in GET/PUT FIFOPTRS macros             */
/*;jg 10/12/89  BUSACC defined as 0                                  @2*/
/*;MC 11/29/89  Changed KERNEL to _KERNEL and removed KGNHAK	       */
/***********************************************************************/

#define GM_LOCK_DEV(PDEV,PPROC) \
{				\
gscDevPtr pdev = PDEV;		\
rcmProcPtr pproc = PPROC;	\
RCM_LOCK_DEV;			\
}

#define GM_UNLOCK_DEV(PDEV)  unlockl(&(PDEV)->devHead.rlock);

#ifndef _KERNEL
#define GETFIFOPTRS(gdp, pproc)						  \
  ((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv.shmem =	  \
   ((rGemprocPtr)gdp->devHead.pProc->procHead.pPriv)->shmem

#define PUTFIFOPTRS 

#endif /* #ifndef _KERNEL */

#ifdef _KERNEL
#define GETFIFOPTRS(gdp, pproc)					       	       \
   copyin(								       \
     ((rGemprocPtr)pproc->procHead.pPriv)->shmem->fi,			       \
     ((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv.shmem->fi,        \
     sizeof(unsigned short) * sizeof(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv.shmem->fi)			                                         \
   )
 
#define PUTFIFOPTRS(gdp, pproc)					       	       \
   copyout(								       \
     ((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv.shmem->fi,        \
     ((rGemprocPtr)pproc->procHead.pPriv)->shmem->fi,			       \
     sizeof(unsigned short) * sizeof(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv.shmem->fi)			                                         \
   )
#endif
 

  
#define BEGIN_CRIT()	/* still needed???	*/

#define END_CRIT()	/* still needed???	*/

#define LOCK_SLOTS()	/* locks entire slots array	*/

#define UNLOCK_SLOTS()	/* unlocks slots array		*/

#define LOCK_CXTSLOT(p)	/* locks specific slot	*/			\
 LOCK(p->slot_lock)

#define UNLOCK_CXTSLOT(p)	/* unlocks specific slot	*/	\
 UNLOCK(p->slot_lock)

#define LOCK(l) 
#define UNLOCK(l) 

#define HERE_I_AM printf("file: %s at line %d\n",__FILE__,__LINE__)
