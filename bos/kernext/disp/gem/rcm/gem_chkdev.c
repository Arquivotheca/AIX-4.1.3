static char sccsid[] = "@(#)81	1.6.1.7  src/bos/kernext/disp/gem/rcm/gem_chkdev.c, sysxdispgem, bos411, 9428A410j 1/19/93 12:18:24";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		gem_check_dev
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


/*
 *;                                                                     
 *; CHANGE HISTORY:                                                     
 *;                                                                     
 *;MC   09/12/89   Created                                            @#
 *;LW   09/19/89   Changed pProc->fault_addr assignment		      @1  
 *;LW   09/19/89   Subtracted position_ofst from fault_addr	      @2  
 *;CL   11/01/89   Fixed prolog                                           
 *;MC	11/29/89  Changed KERNEL to _KERNEL and removed KGNHAK
 *;LW   06/11/90   Remove CAN_FIND_GP ifdef code and set private PID
 */

#include "gemincl.h"
#include "gemrincl.h"
#include "rcm_mac.h"

/*
 * FUNCTION: gem_check_dev                                                 
 *
 * DISCRIPTION:                                                            
 *     return the faulting domain number and save the faulting address     
 *                                                                         
 * WARNING:                                                                
 *     This routine runs on the interrupt level.                           
 *                                                                         
 */

int gem_check_dev( gdp, bus_addr, pid)
struct _gscDev	*gdp;		/* device dependent information       */
caddr_t		bus_addr;
pid_t		    pid;
{
  struct phys_displays *pd      = gdp->devHead.display;
  rGemDataPtr	 pgd   		= (rGemDataPtr)(gdp->devHead.vttld);
  rGemRCMPrivPtr pDevP 		= &pgd->GemRCMPriv;
  CxtSlot	 *pSlots;
  rcmProcPtr     pproc;
  rGemprocPtr	 pProcP;
  ulong		 fault_addr;
  int		 domain_num;	/* faulting domain number             */
  int		 i;
  ulong		 domain_start, domain_end, memory_size;

  fault_addr			= (ulong)bus_addr & 0x0FFFFFFF;

#ifdef GEM_DBUG
  printf("%s %d fault_addr=%x pid=0x%x\n",__FILE__,__LINE__,fault_addr,pid);
#endif

/*
 * check if bus address is in the adapter's memory range                    
 */
  if(((struct gem_dds *)pd->odmdds)->features & MEMORY_4_MEG) 
    memory_size = GemMemSize4Meg;
  else
    memory_size = GemMemSize;


  if (( pDevP->position_ofst > fault_addr ) ||
      ( pDevP->position_ofst + memory_size < fault_addr ))
	return(-1);


  /*
   * Find proc structure
   */
    for (pproc = gdp->devHead.pProc; pproc != NULL;
	 pproc = pproc->procHead.pNext)
	 if (pproc->procHead.pid == pid) break;
    if (pproc == NULL) {
	return (GM_NOT_GP);
    }


/*
 * save the pid in a process private area                        
 */
  pProcP = (rGemprocPtr)pproc->procHead.pPriv;
  pProcP->fault_pid = pid;

  fault_addr -=  pDevP->position_ofst;				/*@2*/

#ifdef TRACK_PID
  printf("%s %d fault_addr=0x%x pProcP=0x%x pid=0x%x\n",
	 __FILE__,__LINE__,fault_addr,pProcP,pid);
#endif


#ifdef GEM_DBUG
  printf(" %s %d  fault_addr - position offset=%x pProcP=0x%x pid=0x%x\n",
	 __FILE__,__LINE__,fault_addr,pProcP,pid);

  for (i=0; i<GM_MAX_DOMAINS; ++i)
    printf("   domain[%d].start=%x domain[%d].end=%x\n",i,
	   (ulong)pd->busmemr[i].bus_mem_start_ram & 0x001fffff,i,
	   (ulong)pd->busmemr[i].bus_mem_end_ram & 0x001fffff);

#endif

/*
 * find the domain number fault occurred in				   
 */
  for ( i=0; i < GM_MAX_DOMAINS; i++ )
  {
  
     domain_start = (ulong)pd->busmemr[i].bus_mem_start_ram & 0x001fffff;
     domain_end  = (ulong)pd->busmemr[i].bus_mem_end_ram & 0x001fffff;


     if (( fault_addr >= domain_start ) &&
	    ( fault_addr <= domain_end ))
     {
	   domain_num = i;
	   break;
     }
  }

#ifdef GEM_DBUG
  printf("%s %d Returning domain number %d\n",__FILE__,__LINE__,domain_num);
#endif

  return (domain_num);
}
