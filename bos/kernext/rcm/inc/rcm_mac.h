/* @(#)37	1.10.3.11  src/bos/kernext/rcm/inc/rcm_mac.h, rcm, bos41J, 9509A_all 2/21/95 14:57:33 */
#ifndef _H_RCM_MAC
#define _H_RCM_MAC

/*
 *
 * COMPONENT_NAME: (rcm) Macros
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989-1995
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*================================================================
                    INCLUDES
  ==============================================================*/
#ifdef _RCM
#include <rcm_trc.h>
#endif


/*================================================================

		    MACROS

  ==============================================================*/

/* -------------------------------------------------------------

    min and max
*/

#define max(a,b)	(ushort) (((ushort) a < (ushort) b) ? b : a)
#define min(a,b)	(ushort) (((ushort) a > (ushort) b) ? b : a)


/* -------------------------------------------------------------

    SET_PDEV
	    Sets the device pointer from the virtual terminal
	    structure.
*/

#define SET_PDEV(pd,pdev)  \
    {pdev = pd->pGSC;}


/* -------------------------------------------------------------

    FIND_COMPROC
	    Determines if the calling process is a graphics
	    process (by pid). If so, sets the variable "pcproc" to
	    point to the RCM common proc structure. If not, it
	    returns with NULL pointer.  THE TID IS NOT CHECKED!
*/

#define FIND_COMPROC(pcproc)  					\
    {  								\
	ulong old_int;						\
        pid_t pid = getpid ();					\
	old_int = i_disable(INTMAX);				\
	for (pcproc = apCom->pProcList; pcproc != NULL; 	\
	     pcproc = pcproc->pNext)				\
	{							\
            if (pcproc->pid == pid)				\
	    {							\
		break;						\
	    } 							\
	}							\
	i_enable(old_int);					\
    }


/* -------------------------------------------------------------

    FIND_GP
	    Determines if the calling process is a graphics
	    process for the indicated device. If so, sets the
	    variable "pproc" to point to the RCM proc structure.
	    If not, it returns with NULL pointer.
	    NULL is also returned if the process is in unmake_gp
	    and the PROC_UNMAKE_GP flag is set.

    NOTE:   
	    FIND_GP_OPT can be called directly for better performance 
	    if interrupts are already disabled.
*/

#define FIND_GP_OPT(pdev,pproc)  					\
    {  									\
	pid_t pid = getpid ();						\
	tid_t tid = thread_self ();					\
	for (pproc = pdev->devHead.pProc; pproc != NULL; 		\
	     pproc = pproc->procHead.pNext)				\
	{								\
	    if  (pproc->procHead.pid == pid               &&		\
	         pproc->procHead.tid == tid               && 		\
		 !(pproc->procHead.flags & PROC_UNMAKE_GP)  )		\
	    { 								\
		break;							\
	    } 								\
	}								\
    }


#define FIND_GP(pdev,pproc)  						\
    {  									\
	ulong old_int;							\
									\
	old_int = i_disable(INTMAX);					\
									\
	FIND_GP_OPT( pdev, pproc );					\
									\
    	i_enable(old_int);						\
    }


/* -------------------------------------------------------------

    FIND_RCX
	    Finds the requested rcx in the list for a graphics
	    process. If found, sets pointer to rcx, else
	    sets pointer to NULL.
*/

#define FIND_RCX(pproc,match,prcx) \
    { \
    for (prcx = pproc->procHead.pRcx; prcx != NULL; prcx = prcx->pNext)   \
	if (prcx == (struct _rcx *) match) break;  \
    }



/* -------------------------------------------------------------

    FIND_WG_OPT
	Serves the same purpose as FIND_WG except that it does not
	disable and enable interrupts.  This should be called when 
	interrupts are already disabled.
*/

#define FIND_WG_OPT(pdev,match,pwg)                                     \
    {                                                                   \
        rcmWG   *hash_top ;                                             \
        ulong   depth;		                                        \
                                                                        \
        DEPTH_INIT(depth);                                              \
        hash_top = (rcmWG *)(pdev->devHead.wg_hash_table->              \
                                entry[RCM_WG_HASH(match)].pWG) ;        \
        pwg = hash_top ;                                                \
                                                                        \
        RCM_FIND_WG_TRACE(0, 0, 0, 0, 0) ; 	                	\
        while (pwg != NULL)                                             \
        {                                                               \
                DEPTH_INC(depth);                                       \
                                                                        \
                if (pwg == (struct _rcmWG *) match) break;              \
                pwg = pwg->pNext ;                                      \
        }                                                               \
        RCM_FIND_WG_TRACE(0, 1, depth, pdev->devHead.window_count,0); 	\
                                                                        \
        CHECK_FOR_LARGE_DEPTH(pdev, depth);                    		\
                                                                        \
    }



/* -------------------------------------------------------------

    FIND_WG
	    Finds the requested window geometry in the list for a
	    device. If found, sets pointer to wg, else
	    sets pointer to NULL.

    NOTE:	
	    Use FIND_WG_OPT instead for situations where 
	    interrupts are already disabled.
*/

#define FIND_WG(pdev,match,pwg)                                         \
    {                                                                   \
        int	old_int;                                                \
                                                                        \
        old_int = i_disable( INTMAX );                                  \
                                                                        \
        FIND_WG_OPT( pdev, match, pwg );                                \
                                                                        \
        i_enable( old_int ) ;                                           \
                                                                        \
    }

/* -------------------------------------------------------------

    FIND_WA
	    Finds the requested window attribute in the list for a
	    process. If found, sets pointer to wa, else
	    sets pointer to NULL.
*/

#define FIND_WA(pproc,match,pwa) \
    { \
    for (pwa = pproc->procHead.pWA; pwa != NULL; pwa = pwa->pNext)   \
	if (pwa == (struct _rcmWA *) match) break;  \
    }

/* -------------------------------------------------------------

    FIND_RCXP
	    Finds the requested context part in the common list.
	    If found, sets pointer to rcxp, else
	    sets pointer to NULL.
*/

#define FIND_RCXP(match,prcxp) \
    { \
	for (prcxp = apCom->pRcxParts; prcxp != NULL; \
				    prcxp = prcxp->pNext)   \
	if (prcxp->glob_id == match) break;  \
    }

/* -------------------------------------------------------------

    FIND_RCXP_BY_HANDLE
	    Finds the requested context part in the common list.
	    If found, sets pointer to rcxp, else
	    sets pointer to NULL.
*/

#define FIND_RCXP_BY_HANDLE(match,prcxp) \
    { \
	for (prcxp = apCom->pRcxParts; prcxp != NULL; \
				    prcxp = prcxp->pNext)   \
	if (prcxp == (struct _rcxp *) match) break;  \
    }

/* -------------------------------------------------------------

    FIND_RCXPH
	    Finds the requested context part header in the list
	    for the context.  If found, sets pointer to rcxph, else
	    sets pointer to NULL.
*/

#define FIND_RCXPH(prcx,match,prcxph) \
    { \
	for (prcxph = prcx->pRcxph; prcxph != NULL; \
				    prcxph = prcxph->pNext)   \
	if (prcxph == (struct _rcxph *) match) break;  \
    }



/* -------------------------------------------------------------
 *  Macros to facilitate usage of the RCM_USR_BUFFER capability by device
 *  drivers.
 *
 *  Interrupt control is the responsibility of the caller.
 *  THERE CAN BE A RACE CONDITION with X going away and invalidating
 *  pdev->devHead.pusrbufhdr.
 * ------------------------------------------------------------- */


    
#define RCM_FIND_RCM_USR_BUFFER(pdevA, setA, srvalA, startA, lengthA, ubufA)  \
{									\
    ubufA = NULL;							\
									\
    if ((unsigned) (setA) < RCM_MAX_USR_BUFFER &&			\
        (pdevA)->devHead.pusrbufhdr != NULL  )				\
    {									\
        ubufA = ((pdevA)->devHead.pusrbufhdr)[setA];			\
									\
        while (ubufA != NULL)						\
        {								\
	    if ((srvalA) == ubufA->srval                            &&	\
	        (startA)             >= ubufA->start                &&	\
	        (startA) + (lengthA) <= ubufA->start + ubufA->length  )	\
	    {								\
	        break;							\
	    }								\
									\
	    ubufA = ubufA->next;					\
        }								\
    }									\
}


#define RCM_WRITE_RCM_USR_BUFFER(ubufA, startA, lengthA, bufferA)	\
    xmemout (bufferA, startA, lengthA, &ubufA->xmemusr);


#define RCM_READ_RCM_USR_BUFFER(ubufA, startA, lengthA, bufferA)	\
    xmemin (startA, bufferA, lengthA, &ubufA->xmemusr);



/* -------------------------------------------------------------------------
	Current Displayed Buffer specific macros for shared memory usage
 * -------------------------------------------------------------------------*/

#define RCM_VALIDATE_CDB_ADDR(pdev_p, new_addr_p, uaddr_p, rc_p)    \
{								      \
    usrbuf_t *	shm_region ;					      \
 								      \
        rc_p = 0 ;					      		\
 								      \
        uaddr_p.eaddr = new_addr_p ;					\
									\
        if (uaddr_p.eaddr == NULL)            /* if not used ... */	\
            uaddr_p.srval = 0;						\
        else  				/* if used, validate buffer */	\
        {								\
            /* save the segment reg content for this effective addr */	\
            uaddr_p.srval = as_getsrval (getadsp (), new_addr_p);	\
									\
    	    RCM_FIND_RCM_USR_BUFFER (pdev_p, gsc_SHM_CurrDispBuff, 	\
					uaddr_p.srval, uaddr_p.eaddr, 	\
					sizeof(uint), shm_region) ;     \
 									\
    	    if (shm_region == NULL)					\
    	    {							      \
            	uaddr_p.eaddr = NULL ;					\
            	uaddr_p.srval = 0;					\
        	rc_p = -1 ;					      \
    	    }							      \
	    else							\
		rc_p = 1;                                             \
        }							      \
}



/* -------------------------------------------------------------------------
	MBX specific macros for shared memory usage
 * -------------------------------------------------------------------------*/

#define RCM_VALIDATE_MBX_ADDR(pdev_p, new_addr_p, uaddr_p, rc_p)    \
{								      \
    usrbuf_t *	shm_region ;					      \
 								      \
        rc_p = 0 ;					      		\
 								      \
        uaddr_p.eaddr = new_addr_p ;					\
									\
        if (uaddr_p.eaddr == NULL){            /* if not used ... */	\
            uaddr_p.srval = 0;						\
        }else  				/* if used, validate buffer */	\
        {								\
            /* save the segment reg content for this effective addr */	\
            uaddr_p.srval = as_getsrval (getadsp (), new_addr_p);	\
									\
    	    RCM_FIND_RCM_USR_BUFFER (pdev_p, gsc_SHM_MBX,       	\
					uaddr_p.srval, uaddr_p.eaddr, 	\
					sizeof(uint), shm_region) ;     \
 									\
    	    if (shm_region == NULL)					\
    	    {							      \
            	uaddr_p.eaddr = NULL ;					\
            	uaddr_p.srval = 0;					\
        	rc_p = -1 ;					      \
    	    }							      \
	    else							\
		rc_p = 1;                                             \
        }							      \
}



#define RCM_UPDATE_CURR_DISP_BUFF(pdev_p, uaddr_p, new_buff, rc_p)    \
{									      \
    usrbuf_t *	shm_region ;					      \
 									      \
    RCM_FIND_RCM_USR_BUFFER (pdev_p, gsc_SHM_CurrDispBuff, uaddr_p.srval,     \
				uaddr_p.eaddr, sizeof(uint), shm_region) ;    \
 									      \
    if (shm_region == NULL)						      \
        rc_p = -1 ;							      \
    else								      \
    {									      \
        rc_p = RCM_WRITE_RCM_USR_BUFFER (shm_region, uaddr_p.eaddr,	      \
					 sizeof(uint), new_buff) ;	      \
    }									      \
}	


#define RCM_UPDATE_MULT_DISP_BUFF(pdev_p, uaddr_p, new_swap, rc_p)            \
{									      \
    usrbuf_t *	shm_region ;					              \
 									      \
    RCM_FIND_RCM_USR_BUFFER (pdev_p, gsc_SHM_MBX, uaddr_p.srval,     	      \
				uaddr_p.eaddr, sizeof(uint), shm_region) ;    \
 									      \
    if (shm_region == NULL)						      \
        rc_p = -1 ;							      \
    else								      \
    {									      \
        rc_p = RCM_WRITE_RCM_USR_BUFFER (shm_region, uaddr_p.eaddr,	      \
					 sizeof(uint), new_swap) ;	      \
    }									      \
}	

/* Return codes for DD start_switch */
#define  RCM_HEAVY_SWITCH	0
#define  RCM_LIGHT_SWITCH	1

/* -------------------------------------------------------------

    RCM_TRACE
	    Produces a RCM trace record.
*/

/* use powers of 2 for speed */
#define  TRACE_2POW	8
#define  TRACE_MAX	(1 << TRACE_2POW)
#define  TRACE_MASK	(TRACE_MAX - 1)

struct _trace_all {
    char	id[8];
    int 	tcount; 	/* debug trace count */
    struct	_trace {	/* trace area */
	int	    type;
	pid_t	    pid;
	int	    a,b;
    } trace[TRACE_MAX];
};

extern struct _trace_all trace_all;

#define RCM_TRACE(mtype,mpid,ma,mb) \
    { \
    int     indx; \
			\
    indx = trace_all.tcount & TRACE_MASK; \
    trace_all.tcount++; \
    trace_all.trace[indx].type = mtype; \
    trace_all.trace[indx].pid = (pid_t) mpid; \
    trace_all.trace[indx].a = (int) ma; \
    trace_all.trace[indx].b = (int) mb; \
    }

#define DRIVER_TRACE(trcptr,mtype,mpid,ma,mb) \
    { \
    int     indx; \
			\
    indx = trcptr->tcount & TRACE_MASK; \
    trcptr->tcount++; \
    trcptr->trace[indx].type = mtype; \
    trcptr->trace[indx].pid = (pid_t) mpid; \
    trcptr->trace[indx].a = (int) ma; \
    trcptr->trace[indx].b = (int) mb; \
    }

/*
	RCM_LOCK/RCM_UNLOCK

	Handles a lock and provides for nesting.
	Ignores signals.  
*/

#define RCM_LOCK(L,S)    { S = lockl (L, LOCK_SHORT); }
#define RCM_UNLOCK(L,S)  { if (S != LOCK_NEST) unlockl (L); }

/*
 *  Flag definitions for rcm_(un)lock_pdev.
 */
#define  PDEV_UNNEST  0x01		/* pop out all nesting */
#define  PDEV_GUARD   0x02		/* guard_dom on outer nest */
#define  PDEV_COND    0x04		/* conditional operation */

#define guard_all_domains(pdev, pproc)				\
{								\
	int i;							\
	for(i = 0; i < pdev->devHead.num_domains; i++)		\
		guard_dom(&pdev->domain[i],pproc,0,GUARD_DEV_LOCK);	\
}

#define unguard_all_domains(pdev)				\
{								\
	int i;							\
	for(i = 0; i < pdev->devHead.num_domains; i++)		\
		unguard_dom(&pdev->domain[i],UNGUARD_DEV_UNLOCK);	\
}

#define GUARD_DOM(pdom, pproc, int, flags)
#define UNGUARD_DOM(pdom, flags)



/*****************************************************************************
 *****************************************************************************
    RCM_ASSERT macro 
	- First define the compile symbols that turn on the asserts
 *****************************************************************************
 *****************************************************************************/

#ifdef DEBUG	/* we want RCM_ASSERT_SWITCH defined when DEBUG is defined */
#	define RCM_ASSERT_SWITCH
#endif

#ifdef RCMDEBUG	/* we want RCM_ASSERT_SWITCH defined when DEBUG is defined */
#	define RCM_ASSERT_SWITCH
#endif

/*---------------------------------------------------------------------------*
    RCM_ASSERT macro itself
 *---------------------------------------------------------------------------*/
#ifdef RCM_ASSERT_SWITCH
#   define RCM_ASSERT(cond, p1, p2, p3, p4, p5)	 			\
    {                                                  		        \
        if(!(cond))							\
        {                                              		        \
                printf("RCM_ASSERT[%s #%d]\n",__FILE__,__LINE__);	\
                brkpoint(0xD06D0D00, p1, p2, p3, p4, p5);		\
        }                                                      		\
    }


/*---------------------------------------------------------------------------*
    assert and ASSERT macros are also redefined
 *---------------------------------------------------------------------------*/
#undef assert
#define assert(p) 		\
{				\
	if(!(p))		\
	{ 			\
		printf("[%s #%d]\n",__FILE__,__LINE__); 	\
		panic("assert(p)");				\
	}			 \
} 

#undef ASSERT
#define ASSERT(p) assert(p)

#else
#	define RCM_ASSERT(cond, p1, p2, p3, p4, p5)
#endif

#endif 

