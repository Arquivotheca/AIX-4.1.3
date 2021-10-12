static char sccsid[] = "@(#)19  1.13.1.11  src/bos/kernext/disp/gem/rcm/gem_rcx.c, sysxdispgem, bos411, 9428A410j 4/26/94 19:48:42";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		gem_associate_rcxp
 *		gem_bind_window
 *		gem_command_list
 *		gem_create_rcx
 *		gem_create_rcxp
 *		gem_create_win_attr
 *		gem_create_win_geom
 *		gem_ddf
 *		gem_delete_rcx
 *		gem_delete_rcxp
 *		gem_delete_win_attr
 *		gem_delete_win_geom
 *		gem_disassociate_rcxp
 *		gem_init_rcx
 *		gem_lock_domain
 *		gem_lock_hw
 *		gem_make_gp
 *		gem_set_gp_priority
 *		gem_start_switch
 *		gem_unlock_domain
 *		gem_unlock_hw
 *		gem_unmake_gp
 *		gem_update_win_attr
 *		gem_update_win_geom
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


/*;**********************************************************************/
/*;CHANGE HISTORY                                                       */
/*;                                                                     */
/*;dm 08/20/89  make_gp.segment = base address                       @1 */
/*;LW 08/23/89  Added #ifdefs for RT development                        */
/*;MC 08/24/89  Added COPY_DD_IN and COPY_DD_OUT to routines            */
/*;LW 08/25/89  Changed include files                                   */
/*;MC 08/25/89  Deleted COPY_DD_IN and COPY_DD_OUT macro calls          */
/*;MC 08/28/89  Changed interfaces to conform to new code from Austin	*/
/*;LW 08/29/89  Added vtp def as a global until Austin responds		*/
/*;LW 09/01/89  Changed coercion of (arg->dd_data) 		      @2*/
/*;DM 09/01/89  make_gp has 3 arguments now                             */
/*;LW 09/08/89  Get shmem ptr from arg dd_data                        @3*/
/*;LW 09/13/89  Moved xxxFIFOPTRS macros to gem_ddmac.h                 */
/*;CL 09/15/89  Added gem_ddf()                                       @4*/
/*;LW 09/18/89  Added domain initialization                           @5*/
/*;MC 09/18/89  Moved domain values to phys_displays structure	      @6*/
/*;MC 09/20/89  Modified domain addresses			      @7*/
/*;DM 09/21/89  Added GemrcmPriv initialization from make_gp          @8*/
/*;DM 09/21/89  xmem and kernel stuff                                 @9*/
/*;lw 09/22/89  Added deferred update window                            */
/*;DM 09/22/89  changed call to GETFIFOPTRS, PUTFIFOPTRS                */
/*;lw 10/05/89  changed call interface to gem_init_rcx                  */
/*;lw 10/13/89  Removed gem_set_rcx.  Not called by DI RCM              */
/*;		Eliminated call to empty function iggm_bind_window      */
/*;MC 11/21/89  Added call to real iggm_bind_window		      @A*/
/*;LW 11/27/89  Added more trace info to upd geom & attr	        */
/*;MC 11/29/89  Changed KERNEL to _KERNEL and removed KGNHAK		*/
/*;LW 01/11/90  Fixed unmakegp interface                      		*/
/*;LW 01/17/90  8951 change to handle null rp in upd geom  		*/
/*;LW 02/26/90  Ensure update win attr returns 0           	      @B*/
/*;**********************************************************************/
#define  XMAP_NMULTIMAP 16
#include <sys/syspest.h>

#include "gemincl.h"
#include "gemrincl.h"
#include "rcm_mac.h"
#include "gem_gai.h"




/***********************************************************************/
/* FUNCTION NAME: gem_init_rcx                                         */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_init_rcx(pd, pgd)
struct phys_displays *pd;			/* read only 		*/
rGemDataPtr pgd;				/* vtt local data 	*/
{
 int rc;

#ifdef GEM_DBUG
    printf("Entering gem_init_rcx\n");
#endif GEM_DBUG
  
   rc = 0;
  rc = iggm_init_rcx(pd, pgd);

#ifdef GEM_DBUG
    printf("Exiting gem_init_rcx\n");
#endif GEM_DBUG
    
    return (rc);
}

/***********************************************************************/
/* FUNCTION NAME: gem_make_gp                                          */
/*                                                                     */
/* DESCRIPTION: Returns the segment base start address of the adapter. */
/***********************************************************************/
int gem_make_gp(gdp, datap, length) 
struct _gscDev   *gdp;
char             *datap;
int              length;

{
   ulong        gmbase ;
   int          rc ;
   rGemRCMPrivPtr	pDevP;
   struct 	gem_ddf *ddf;
   struct 	gemini_data *ld;

   struct phys_displays *pd;

   pd = gdp->devHead.display ;
   
   ddf = (struct gem_ddf *) pd->free_area;
   if ( ddf->num_of_process == 0 )   
   {
	ld = (struct gemini_data *) gdp->devHead.vttld;
        ld->Vttenv.vtt_mode = GRAPHICS_MODE;
	vttact( pd->visible_vt );
   }

   
#ifdef GEM_DBUG
   printf("\nEntering gem_make_gp pdp 0x%x datap 0x%x length %d\n",
	  pd, datap, length);

   printf("dds_addr %X\n", pd->odmdds);
   printf("bus_mem_start_ram %X\n",
	  ((struct gem_dds *)pd->odmdds)->io_bus_mem_start);

/*   brkpoint(0x11111111);*/
   printf("Device structure is @ %x\n",gdp);
#endif GEM_DBUG

   rc = 0;
   rc = iggm_make_gp( gdp, datap, length ) ;

   ddf->num_of_process++;

#ifdef GEM_DBUG
	printf("Exiting gem_make_gp \n");
#endif GEM_DBUG

	return(rc) ;
}

/***********************************************************************/
/* FUNCTION NAME: gem_unmake_gp                                          */
/*                                                                     */
/* DESCRIPTION: Returns the segment base start address of the adapter. */
/***********************************************************************/
int gem_unmake_gp( gdp, pproc)
struct _gscDev *gdp;
rcmProcPtr	 pproc;			/* current rcm process pointer    @9 */

{
  int rc;
  struct 	gem_ddf *ddf;
  struct 	phys_displays *pd;
  struct 	gemini_data *ld;

  pd = gdp->devHead.display ;

  rc = 0;
#ifdef GEM_DBUG
  printf("Entering gem_unmake_gp\n");
#endif GEM_DBUG


	rc = iggm_unmake_gp( gdp,pproc) ;

   	ddf = (struct gem_ddf *) pd->free_area;
   	ddf->num_of_process--;
   	if ( ddf->num_of_process == 0 )   
   	{
		ld = (struct gemini_data *) gdp->devHead.vttld;
        	ld->Vttenv.vtt_mode = KSR_MODE ;
  	} 
#ifdef GEM_DBUG
	printf("Exiting gem_unmake_gp \n");
#endif GEM_DBUG

	return(rc);
}

/***********************************************************************/
/* FUNCTION NAME: gem_set_gp_priority                                  */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_set_gp_priority( gdp, arg )
struct _gscDev *gdp;
set_gp_priority      *arg;
 
{
 
	arg->error = iggm_set_gp_priority( gdp, arg ) ;

#ifdef GEM_DBUG
	printf("Exiting gem_set_gp_priority \n");
#endif GEM_DBUG

	return ;
}

/***********************************************************************/
/* FUNCTION NAME: gem_create_rcx                                       */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_create_rcx(gdp, rp, arg, datap)
struct _gscDev     *gdp;
struct _rcx        *rp;
create_rcx         *arg;
char               *datap;

{
	rcmProcPtr	pproc;

#ifdef GEM_DBUG
	printf("Entering gem_create_rcx \n");
#endif GEM_DBUG

	arg->error = iggm_create_rcx( gdp, rp, arg, datap );

#ifdef GEM_DBUG
	printf("Exiting gem_create_rcx.  Returns: %x\n",arg->error) ;
	printf("  pData=%x\n",rp->pData);
#endif GEM_DBUG

	return(arg->error);
}

/***********************************************************************/
/* FUNCTION NAME: gem_delete_rcx                                       */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_delete_rcx(gdp, rp)
struct _gscDev    *gdp;
struct _rcx       *rp;

{
	int		rc;

#ifdef GEM_DBUG
	printf("Entering gem_delete_rcx \n");
#endif GEM_DBUG
	rc = iggm_delete_rcx( gdp, rp ) ;



#ifdef GEM_DBUG
	printf("Exiting gem_delete_rcx - return code = %d \n",rc) ;
#endif GEM_DBUG

	return(rc);
}

/***********************************************************************/
/* FUNCTION NAME: gem_create_rcxp                                      */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_create_rcxp(gdp, pRcxp, arg, datap)
struct _gscDev   *gdp;
rcxpPtr            pRcxp;
create_rcxp      *arg;
rGemrcxPtr         datap;

{
	rcmProcPtr	pproc;
	int		rc;

	rc = iggm_create_rcxp(gdp, pRcxp, arg, datap);

#ifdef GEM_DBUG	
        printf("Exiting gem_create_rcxp \n") ;
#endif GEM_DBUG

	return(rc);
}

/***********************************************************************/
/* FUNCTION NAME: gem_delete_rcxp                                      */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_delete_rcxp( gdp, arg )
struct _gscDev   *gdp ;
delete_rcxp      *arg;

{
	rcmProcPtr	pproc;
	int		rc;

	rc = iggm_delete_rcxp( gdp, arg ) ;

#ifdef GEM_DBUG
        printf("Exiting gem_delete_rcxp \n") ;
#endif GEM_DBUG

	return(rc);
}

/***********************************************************************/
/* FUNCTION NAME: gem_associate_rcxp                                   */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_associate_rcxp( gdp, arg )
struct _gscDev      *gdp ;
associate_rcxp      *arg ;

{
	rcmProcPtr	pproc;
	int		rc;

	 rc = iggm_associate_rcxp( gdp, arg ) ;


#ifdef GEM_DBUG
        printf("Exiting gem_associate_rcxp \n") ;
#endif GEM_DBUG

	return(rc);

}

/***********************************************************************/
/* FUNCTION NAME: gem_disassociate_rcxp                                */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_disassociate_rcxp( gdp, arg )
struct _gscDev         *gdp ;
disassociate_rcxp      *arg ;

{
	rcmProcPtr	pproc;
	int		rc;

	 rc = iggm_disassociate_rcxp( gdp, arg ) ;

#ifdef GEM_DBUG
        printf("Exiting gem_disassociate_rcxp \n") ;
#endif GEM_DBUG

	return(rc);

}

/***********************************************************************/
/* FUNCTION NAME: gem_create_win_attr                                  */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_create_win_attr(gdp, wap, arg)
struct _gscDev   *gdp;
struct _rcmWA    *wap;
create_win_attr  *arg ;

{
	rcmProcPtr	pproc;
#ifdef GEM_DBUG
	printf("Entering gem_create_win_attr: wap = %x\n",wap);
#endif GEM_DBUG


	arg->error = iggm_create_win_attr( gdp, wap, arg ) ;

#ifdef GEM_DBUG
        printf("Exiting gem_create_win_attr: pPriv = %x\n", wap->pPriv) ;
#endif GEM_DBUG

	return(arg->error);
}

/***********************************************************************/
/* FUNCTION NAME: gem_update_win_attr                                  */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_update_win_attr(gdp, rp, newwap, changemask, arg)
struct _gscDev    *gdp;
struct _rcx       *rp;
struct _rcmWA     *newwap;
int               changemask;
update_win_attr   *arg ;

{
#ifdef GEM_DBUG
  printf(
"Start VDD update win_attr:gdp=0x%x rp=0x%x rp.WA=0x%x newWA=0x%x mask=0x%x\n",
	 gdp, rp, rp->pWA, newwap, changemask);
#endif GEM_DBUG
	
  
      iggm_update_win_attr(gdp, rp, newwap, changemask, arg) ;


#ifdef GEM_DBUG
  printf("Done VDD update win_attr\n");
#endif GEM_DBUG
  
  return(0);
}

/***********************************************************************/
/* FUNCTION NAME: gem_delete_win_attr                                  */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_delete_win_attr(gdp, pwa)
struct _gscDev   *gdp ;
struct _rcmWA    *pwa;

{
	rcmProcPtr	pproc;
	int		rc;

#ifdef GEM_DBUG
	printf("Entering gem_delete_win_attr: wap = %x pPriv = %x\n",
	       pwa, pwa->pPriv);
#endif GEM_DBUG

	rc = iggm_delete_win_attr( gdp, pwa ) ;

#ifdef GEM_DBUG
	printf("Exiting gem_delete_win_attr\n");
#endif GEM_DBUG

	return(rc);
}



/***********************************************************************/
/* FUNCTION NAME: gem_create_win_geom                                  */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_create_win_geom(gdp, wgp, arg)
struct _gscDev      *gdp;
struct _rcmWG       *wgp;
struct _create_win_geom     *arg ;

{
	rcmProcPtr	pproc;

#ifdef GEM_DBUG
	printf("Entering gem_create_win_geom: wgp = %x\n", wgp);
#endif GEM_DBUG

	arg->error = iggm_create_win_geom ( gdp, wgp, arg ) ;

#ifdef GEM_DBUG
	printf("Exiting gem_create_win_geom: pPriv = %x\n", wgp->pPriv);
#endif GEM_DBUG

	return(arg->error);
}

/***********************************************************************/
/* FUNCTION NAME: gem_update_win_geom                                  */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_update_win_geom(gdp, rp, newgp, changemask)
struct _gscDev      *gdp;
struct _rcx         *rp;
struct _rcmWG       *newgp;
int                 changemask;

{
  int		i;

#ifdef GEM_DBUG
  printf(
 "Start VDD update geom:gdp=0x%x rp=0x%x rp.WG=0x%x newgp=0x%x mask=0x%x\n",
	 gdp, rp, rp->pWG, newgp, changemask);
#endif
  
      iggm_update_win_geom( gdp, rp, newgp, changemask) ;

#ifdef GEM_DBUG
	printf("Done VDD update geom\n");
#endif GEM_DBUG

	return(0);
}

/***********************************************************************/
/* FUNCTION NAME: gem_delete_win_geom                                  */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_delete_win_geom(gdp, pwg)
struct _gscDev      *gdp;
struct _rcmWG       *pwg;
{
	rcmProcPtr	pproc;
	int             rc;

#ifdef GEM_DBUG
	printf("Entering gem_delete_win_geom: wgp = %x pPriv = %x\n",
	       pwg, pwg->pPriv);
#endif GEM_DBUG

	rc = iggm_delete_win_geom(gdp, pwg);

#ifdef GEM_DBUG
	printf("Exiting gem_delete_win_geom\n");
#endif GEM_DBUG

	return(rc);
}

/***********************************************************************/
/* FUNCTION NAME: gem_bind_window                                      */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_bind_window( gdp, rp, newwgp, newwap )			   /*@A*/
struct _gscDev   *gdp;
struct _rcx      *rp;
struct _rcmWG    *newwgp;
struct _rcmWA    *newwap;

{
	rcmProcPtr	pproc;
	int		rc;


#ifdef GEM_DBUG
	printf("Entering gem_bind_window rp=0x%x, newWG=0x%x, newWA=0x%x\n",
	       rp, newwgp, newwap);
#endif GEM_DBUG
	rc = iggm_bind_window( gdp, rp, newwgp, newwap ) ;

#ifdef GEM_DBUG
	printf("Exiting gem_bind_window \n");
#endif GEM_DBUG

	return(rc);
}


/***********************************************************************/
/* FUNCTION NAME: gem_lock_hw                                          */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_lock_hw( gdp, arg )
struct _gscDev *gdp;
lock_hw      *arg ;
{
	rcmProcPtr	pproc;

	arg->error = iggm_lock_hw( gdp, arg );

#ifdef GEM_DBUG
	printf("Exiting gem_lock_hw \n");
#endif GEM_DBUG

	return(arg->error);
}

/***********************************************************************/
/* FUNCTION NAME: gem_unlock_hw                                        */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_unlock_hw( gdp, arg )
struct _gscDev *gdp;
unlock_hw      *arg ;
{
	rcmProcPtr	pproc;

	arg->error = iggm_unlock_hw( gdp, arg );

#ifdef GEM_DBUG
	printf("Exiting gem_unlock_hw \n");
#endif GEM_DBUG

	return(arg->error);
}

/***********************************************************************/
/* FUNCTION NAME: gem_lock_domain                                      */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_lock_domain( gdp, arg )
struct _gscDev *gdp;
lock_domain      *arg ;
{
	rcmProcPtr	pproc;

	arg->error = iggm_lock_domain( gdp, arg );

#ifdef GEM_DBUG
	printf("Exiting gem_lock_domain \n");
#endif GEM_DBUG

	return(arg->error);
}

/***********************************************************************/
/* FUNCTION NAME: gem_unlock_domain                                    */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_unlock_domain( gdp, arg )
struct _gscDev *gdp;
unlock_domain      *arg ;
{
	rcmProcPtr	pproc;

	arg->error = iggm_unlock_domain( gdp, arg );

#ifdef GEM_DBUG
	printf("Exiting gem_unlock_domain \n");
#endif GEM_DBUG

	return(arg->error);
}

/***********************************************************************/
/* FUNCTION NAME: gem_command_list                                     */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_command_list( gdp, arg )
struct _gscDev *gdp;
command_list      *arg ;
{
	rcmProcPtr	pproc;

	arg->error = iggm_command_list( gdp, arg ) ;


#ifdef GEM_DBUG
	printf("Exiting gem_command_list \n");
#endif GEM_DBUG

	return(arg->error);
}


/*@4*/
/***********************************************************************/
/* FUNCTION NAME: gem_ddf                                              */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/

int gem_ddf(gdp, cmd, ddi, ddi_length) 
struct _gscDev    *gdp ;
int               cmd ;
caddr_t           ddi ;
int               ddi_length ;
{
	int             rc;
	rcmProcPtr	pproc;
#ifdef GEM_DBUG
	printf("Entering gem_ddf gdp=%x gdp->devHead.pProc=%x\n",gdp,gdp->devHead.pProc);
#endif GEM_DBUG

	FIND_GP(gdp,pproc);

#ifdef GEM_DBUG
	printf("pproc=%x gdp=%x\n", pproc,gdp);
	if(pproc != NULL)
	  printf("pproc->pPriv->shmem=%x\n", ((rGemprocPtr)pproc->procHead.pPriv)->shmem);
	printf("gpd->vttld->shmem=%x\n", ((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv.shmem);
#endif

	rc = 0;


	rc = iggm_ddf( gdp, cmd, ddi, ddi_length );

#ifdef GEM_DBUG
	printf("Exiting gem_ddf \n");
#endif GEM_DBUG

	return(rc);
}

/***********************************************************************/
/* FUNCTION NAME: gem_start_switch                                     */
/*                                                                     */
/* DESCRIPTION:                                                        */
/***********************************************************************/
int gem_start_switch(gdp, old_rcx, new_rcx, seq_num)
struct _gscDev        *gdp ;
rcxPtr                old_rcx ;
rcxPtr                new_rcx ;
int                   *seq_num ;

{
	int rc;
#ifdef GEM_DBUG
	printf("Entering gem_start_switch \n");
#endif GEM_DBUG

	rc = iggm_start_switch(gdp, old_rcx, new_rcx, seq_num);

#ifdef GEM_DBUG
	printf("Exiting gem_start_switch\n");
#endif GEM_DBUG

	return(rc);
}
