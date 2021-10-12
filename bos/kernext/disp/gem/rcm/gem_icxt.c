static char sccsid[] = "@(#)91	1.8.2.5  src/bos/kernext/disp/gem/rcm/gem_icxt.c, sysxdispgem, bos411, 9428A410j 1/19/93 12:20:19";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		gem_init_imm_cxt
 *		gem_init_trav_cxt
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



/*;
 *;                                                                     
 *; CHANGE HISTORY:                                                     
 *;                                                                     
 *;MC   09/05/89   Created                                            @#
 *;DM   09/05/89   Modified                                             
 *;LW   09/14/89   Changed ctx to cxt, renamed file                     
 *;MC   09/14/89   Changed assignment of hatch data work area offset  @1
 *;CL   11/16/89   Added routine for init Traversal FIFO context      @2
 *;MC	12/11/89   Fixed for new context and fixed return code
 *;		   checking from rMalloc
 *;LW 12/13/89  Restructured hfiles				     
 *;LW 02/21/90  Changed view table to twice the screen size
 *;LW 02/27/90  Changed to use itof for all floating point 
 *;                                                                     
 */

#include "gemincl.h"
#include "gemrincl.h"
#include "gmasl.h"
ulong itof();

int gem_init_imm_cxt( pDevP, new_rcx, pData)
rGemRCMPrivPtr  pDevP;
rcxPtr          new_rcx;
char            *pData;
{
   int          i;
   ulong        gmbase = pDevP->gmbase;
   IMM_CONTEXT  *gmcxt;
   IMM_CONTEXT  *cp;
   rGemrcxPtr   pGemrcx;
   int          slot_num;
   CxtSlot      *pCslot;

   pGemrcx        =  (rGemrcxPtr) new_rcx->pData;
   slot_num    =  pGemrcx->start_slot;
   pCslot      =  &(pDevP->slots[slot_num]);
   gmcxt       =  (IMM_CONTEXT *) pCslot->slot_addr;

   /*
    * Malloc space in kernel for context
    */
#ifdef GEM_DBUG
   printf("%s: Trying to malloc %d bytes for imm cxt\n",__FILE__,pCslot->slot_len);
#endif
   if (
       (cp = (IMM_CONTEXT *) rMalloc(pCslot->slot_len)) == NULL
       ) return(GM_NOMEMORY);
#ifdef GEM_DBUG
HERE_I_AM;
#endif
   
   pGemrcx->pCxt  =  (gem_cxt *) cp;

   /*-------------------------------------------------------------------*/
   /*   Initialize all fields in malloced CONTEXT.                      */
   /*-------------------------------------------------------------------*/

                                                /* Window Plane Masks   */
   cp->rminfo.rmchngind	   = 0;                    /* change indicator flag*/
   cp->rminfo.wfrmbufmsk   = 0;                    /* wndw plne frm bfr msk*/
   cp->rminfo.wcmpmask     = 0;                    /* wndw plne compare msk*/
   cp->rminfo.wcmpval      = 0;                    /* wndw plne compare val*/

   cp->rminfo.polylclr     = GMDefaultColor;       /* polyline color       */
   cp->rminfo.intclr       = GMDefaultColor;       /* interior color       */
   cp->rminfo.wlogop       = 0;       		   /* window logical op    */

   cp->rminfo.windorgx     = 0;                    /* window origin (x)    */
   cp->rminfo.windorgy     = 0;                    /* window origin (y)    */
   cp->rminfo.windwidth	= itof(GM_WIDTH - 1);	/* window width - 1	*/
   cp->rminfo.windheight = itof(GM_HEIGHT - 1);	/* window height - 1	*/
   cp->rminfo.woflgs       = 0;                    /* window origin flags  */
   cp->rminfo.numpikatst   = 0;

                                        /*------------------------------*/
                                        /* ASL initialization           */
                                        /* (adapter state list)         */
                                        /*------------------------------*/
   cp->asl.dsvo      = VME_ADR(&gmcxt->dsv);    /* offset to dsv        */
   cp->asl.nviewtab  = 1;                       /* # view tables        */
   cp->asl.viewato   = VME_ADR(&gmcxt->vat[0]); /* offset to vat        */
   cp->asl.npattbl   = 0;                       /* # pattern tables     */
                                                /* (used by blt)        */
   cp->asl.pattao    = VME_ADR(&gmcxt->pt[0]);  /* offset to pt         */
   cp->asl.nhattbl   = 0;                       /* # hatch tables       */
   cp->asl.htblo     = VME_ADR(&gmcxt->ht[0]);  /* offset to ht         */
   cp->asl.nlntbl    = 0;                       /* # line type tables   */
   cp->asl.lnptbo    = 0;                       /* offset to ltt        */
   cp->asl.lnprtbo   = 0;                       /* offset to line type
                                                   rendering table      */
   cp->asl.hatwkao   = VME_ADR(&((GM_MMAP *)gmbase)->hatwa[0]);
                                                /* offset to hwa      @1*/
   cp->asl.chngind   = 0;                       /* change indicators    */

   cp->asl.lntblchngbm  = 0;                    /* line tbl change bits */
   cp->asl.pattblchngbm = 0;                    /* pat tbl change bits  */
   cp->asl.hattblchngbm = 0;                    /* hat tbl change bits  */

                                                /* Overlay Plane Masks  */
   cp->asl.ofrmbufmsk   = 0;                    /* ovrl plne frm bfr msk*/
   cp->asl.ocmpmsk      = 0;                    /* ovrl plne compare msk*/
   cp->asl.ocmpval      = 0;                    /* ovrl plne compare val*/


   cp->asl.dispmsk      = GMOverlayDisplayMask; /* display mask         */

   cp->asl.ologop	= 0;			/* ovrl plne logical op	*/


   cp->asl.woanntxtratio= 0;
   cp->asl.lwdcwcrat	= itof(1);		/* line wid wind ratio	*/
   cp->asl.prminfo  	= VME_ADR(&gmcxt->rminfo); /* offset to dsv     */
   cp->asl.gtxtculdspmth= 1;			/* cull display method	*/
   cp->asl.gtxtculhgt	= 0;			/* culling height	*/

                                        /*------------------------------*/
                                        /* DSV initialization           */
                                        /* (drawing state vector)       */
                                        /*------------------------------*/
   cp->dsv.lgvtm[0]      = 1;                   /* View Xform Matrix    */
   cp->dsv.lgvtm[1]      = 0;                   /* 4x3 matrix           */
   cp->dsv.lgvtm[2]      = 0;                   /*   100                */
   cp->dsv.lgvtm[3]      = 0;                   /*   010                */
   cp->dsv.lgvtm[4]      = 1;                   /*   001                */
   cp->dsv.lgvtm[5]      = 0;                   /*   000                */
   cp->dsv.lgvtm[6]      = 0;                   /*                      */
   cp->dsv.lgvtm[7]      = 0;                   /*                      */
   cp->dsv.lgvtm[8]      = 1;                   /*                      */
   cp->dsv.lgvtm[9]      = 0;                   /*                      */
   cp->dsv.lgvtm[10]     = 0;                   /*                      */
   cp->dsv.lgvtm[11]     = 0;                   /*                      */
   cp->dsv.plylincoli    = GMDefaultColor;      /* Polyline Color Int   */
   cp->dsv.intcoli       = GMDefaultColor;      /* Interior Color Int   */
   cp->dsv.curlintyp     = LSSolid;             /* Curnt Line Type      */
   cp->dsv.curlinwid     = 1;                   /* Curnt Line Width     */
   cp->dsv.curintsty     = 2;                   /* Curnt Interior Style */
   cp->dsv.curintstyind  = 1;                   /* Curnt Intr Styl Indx */
   cp->dsv.curvwtbli     = 0;                   /* Curnt View Table Indx*/
   cp->dsv.frbufmsk      = 0;                   /* Frame Buffer Mask    */
   cp->dsv.dispmsk       = GMDefaultDisplayMask;/* Dsply Mask(Base Plnes*/
   cp->dsv.dsvlogop      = 0;                   /* Logical Operation    */
   cp->dsv.colpromoddat  = 0;                   /* Color Processor Mode */
   cp->dsv.dsvflgs       = 0;                   /* Flags                */
                                                /*   Bits 31-2 -  Rsvd  */
                                                /*         1-0 -  Color */
                                                /*       Processing Mode*/


                                        /*------------------------------*/
                                        /* VT  initialization           */
                                        /* (view table)                 */
                                        /*------------------------------*/
   cp->vt.vwtindx    = 1;                       /* View Table Index     */
   cp->vt.vopflg     = 0x000E;                  /* View Operation Flag  */

   cp->vt.vtm[0]     = itof(1);     		/* View Trans Matrix    */
   cp->vt.vtm[1]     = 0;     			/*    device coords -   */
   cp->vt.vtm[2]     = 0;     			/*       upper left org */
   cp->vt.vtm[3]     = 0;     			/*                      */
   cp->vt.vtm[4]     = 0;     			/*                      */
   cp->vt.vtm[5]     = itof(-1);    		/*                      */
   cp->vt.vtm[6]     = 0;     			/*                      */
   cp->vt.vtm[7]     = 0;     			/*                      */
   cp->vt.vtm[8]     = 0;     			/*                      */
   cp->vt.vtm[9]     = 0;     			/*                      */
   cp->vt.vtm[10]    = itof(1);     		/*                      */
   cp->vt.vtm[11]    = 0;     			/*                      */
   cp->vt.vtm[12]    = 0;     			/*                      */
   cp->vt.vtm[13]    = itof(400*1023);		/*                      */
   cp->vt.vtm[14]    = 0;     			/*                      */
   cp->vt.vtm[15]    = itof(1);     		/*                      */

   cp->vt.prp[0]     = 0;     			/* Proj Ref Pt (x)      */
   cp->vt.prp[1]     = 0;     			/* Proj Ref Pt (y)      */
   cp->vt.prp[2]     = itof(1);     		/* Proj Ref Pt (z)      */

   cp->vt.vpd        = 0;     			/* View Plane Distance  */

   cp->vt.truvp[0]   = 0;     			/* True Vwport (pixel)  */
   cp->vt.truvp[1]   = itof(400*1279); 		/* True Vwport (pixel)  */
   cp->vt.truvp[2]   = 0;     			/* True Vwport (pixel)  */
   cp->vt.truvp[3]   = itof(400*1023); 		/* True Vwport (pixel)  */
   cp->vt.truvp[4]   = 0;     			/* True Vwport (pixel)  */
   cp->vt.truvp[5]   = 0;     			/* True Vwport (pixel)  */
   cp->vt.truw2vr[0] = itof(1);     		/* True WtoVwport Ratio */
   cp->vt.truw2vr[1] = itof(1);     		/* True WtoVwport Ratio */
   cp->vt.truw2vr[2] = itof(1);     		/* True WtoVwport Ratio */
   cp->vt.truiw2vr[0]= itof(1);     		/* True Invrse WtoVwport*/
   cp->vt.truiw2vr[1]= itof(1);     		/* True Invrse WtoVwport*/
   cp->vt.truiw2vr[2]= itof(1);     		/* True Invrse WtoVwport*/
   cp->vt.truwvc[0]  = 0;     			/* True Window in VC    */
   cp->vt.truwvc[1]  = itof(400*1279); 		/* True Window in VC    */
   cp->vt.truwvc[2]  = 0;     			/* True Window in VC    */
   cp->vt.truwvc[3]  = itof(400*1023); 		/* True Window in VC    */
   cp->vt.truwvc[4]  = 0;     			/* True Window in VC    */
   cp->vt.truwvc[5]  = itof(1);     		/* True Window in VC    */
   cp->vt.npczclip[0] = 0;			/* Z clip bound in NPC	*/
   cp->vt.npczclip[1] = itof(1);		/* Z clip bound in NPC	*/
   cp->vt.shearparm[0] = itof(1279);		/* Shearing Parameters	*/
   cp->vt.shearparm[1] = itof(1023);		/* Shearing Parameters	*/
   cp->vt.shearparm[2] = itof(-1);		/* Shearing Parameters	*/

                                        /*------------------------------*/
                                        /* No Line Types                */
                                        /*------------------------------*/

                                        /*------------------------------*/
                                        /* No Patterns Initialized      */
                                        /*------------------------------*/

                                        /*------------------------------*/
                                        /* No Hatches Initialized       */
                                        /*------------------------------*/

                                        /*------------------------------*/
                                        /* NO Base Planes Color Table   */
                                        /*      initialization          */
                                        /*------------------------------*/

                                        /*------------------------------*/
                                        /* NO Overlay Planes Color Table*/
                                        /*      initialization          */
                                        /*------------------------------*/

                                        /*------------------------------*/
                                        /* View Address Table           */
                                        /*      initialization          */
                                        /*------------------------------*/

   cp->vat[0]        = VME_ADR(&gmcxt->vt);     /* offset to vt         */




   return(0);
};




/* @2 */
/*
 *    Function: gem_init_trav_cxt()
 *    Description: This routine is to initialize Traversal Context
 *    NB: Certain variables are decalred as IMM_CONTEXT ptrs.  This is
 *	  misleading - IMM_CONTEXT is actually a definition of a
 *	  generic context.
 */

int gem_init_trav_cxt( pDevP, new_rcx, pData)
rGemRCMPrivPtr    pDevP;
rcxPtr            new_rcx;
char              *pData;
{
   int            i;
   ulong          gmbase = pDevP->gmbase;
   TRAV_CONTEXT   *gmcxt;
   TRAV_CONTEXT   *cp;
   TRAV_SLOT	  *pTslot;
   rGemrcxPtr     pGemrcx;
   int            slot_num;
   CxtSlot        *pCslot;

   pGemrcx        =  (rGemrcxPtr) new_rcx->pData;
   slot_num       =  pGemrcx->start_slot;
   pCslot         =  &(pDevP->slots[slot_num]);
   gmcxt          =  (TRAV_CONTEXT *) pCslot->slot_addr;

   /*
    * Malloc space in kernel for context
    */
#ifdef GEM_DBUG
   printf("%s: Trying to malloc %d bytes for trav cxt\n",__FILE__,pCslot->slot_len);
#endif
   if ((cp = (TRAV_CONTEXT *) rMalloc(pCslot->slot_len)) == NULL )
      return(GM_NOMEMORY);
#ifdef GEM_DBUG
HERE_I_AM;
#endif
	     
   pTslot	  =  (TRAV_SLOT *)cp;
   pGemrcx->pCxt  =  (gem_cxt *) cp;

   /*-------------------------------------------------------------------*/
   /*   Initialize all fields in malloced CONTEXT.                      */
   /*-------------------------------------------------------------------*/

                                                /* Window Plane Masks   */
   pTslot->rminfo.rmchngind    = 0;                /* change indicator flag*/
   pTslot->rminfo.wfrmbufmsk   = 0;                /* wndw plne frm bfr msk*/
   pTslot->rminfo.wcmpmask     = 0;                /* wndw plne compare msk*/
   pTslot->rminfo.wcmpval      = 0;                /* wndw plne compare val*/

   pTslot->rminfo.polylclr     = GMDefaultColor;   /* polyline color       */
   pTslot->rminfo.intclr       = GMDefaultColor;   /* interior color       */
   pTslot->rminfo.wlogop       = 0;    		   /* window logical op    */

   pTslot->rminfo.windorgx     = 0;                /* window origin (x)    */
   pTslot->rminfo.windorgy     = 0;                /* window origin (y)    */
   pTslot->rminfo.windwidth	= itof(GM_WIDTH - 1);/* window width - 1   */
   pTslot->rminfo.windheight= itof(GM_HEIGHT - 1); /* window height - 1    */
   pTslot->rminfo.woflgs       = 0;                /* window origin flags  */
   if (((rGemrcxPtr)pData)->cxt_type & GL_ENHANCED)
     pTslot->rminfo.woflgs    |= 0x8000;	   /* indicate GL context  */
   pTslot->rminfo.numpikatst   = 0;

                                        /*------------------------------*/
                                        /* ASL initialization           */
                                        /* (adapter state list)         */
                                        /*------------------------------*/
   cp->asl.dsvo      = 0;	                /* offset to dsv        */
   cp->asl.nviewtab  = 1;                       /* # view tables        */
   cp->asl.viewato   = VME_ADR(&gmcxt->vat[0]); /* offset to vat        */

   cp->asl.npylnbtb  = 0;                       /* # Polyline bundle tbl*/
   cp->asl.pylnbtbo  = 0;                       /* Offset to Pyln bun tb*/
   cp->asl.npymkbtb  = 0;                       /* # Polymarker bun tbl */
   cp->asl.pymkbtbo  = 0;                       /* Offset to Pymk bun tb*/
   cp->asl.ntextbtb  = 0;                       /* # Text bundle table  */
   cp->asl.textbtbo  = 0;                       /* Offset to Text bun tb*/
   cp->asl.nintrbtb  = 0;                       /* # Interior bun tbl   */
   cp->asl.intrbtbo  = 0;                       /* Offset to Intr bun tb*/
   cp->asl.nedgebtb  = 0;                       /* # Edge bundle table  */
   cp->asl.edgebtbo  = 0;                       /* Offset to Edge bun tb*/

   cp->asl.npattbl   = 0;                       /* # pattern tables     */
                                                /* (used by blt)        */
   cp->asl.pattao    = 0;                       /* offset to pt         */
   cp->asl.nhattbl   = 0;                       /* # hatch tables       */
   cp->asl.htblo     = 0;                       /* offset to ht         */
   cp->asl.nlntbl    = 0;                       /* # line type tables   */
   cp->asl.lnptbo    = 0;                       /* offset to ltt        */
   cp->asl.lnprtbo   = 0;                       /* offset to line type
                                                   rendering table      */


   cp->asl.nmktytb       = 0;                   /* # Marker type tbl    */ 
   cp->asl.mktytbo       = 0;                   /* Offset to Mk type tb */
   cp->asl.ncullsztb     = 0;                   /* # Cull size table    */
   cp->asl.cullsztbo     = 0;                   /* Offset to Cull sz tb */
   cp->asl.ndepthcutb    = 0;                   /* # Depth cue table    */ 
   cp->asl.depthcutbo    = 0;                   /* Offset to Depth cu tb*/
   cp->asl.nlghtsrctb    = 0;                   /* # Light src table    */
   cp->asl.lghtsrctbo    = 0;                   /* Offset to Lght src tb*/
   cp->asl.nclrproctbnrm = 0;                   /* # Color proc tbl     */
   cp->asl.clrproctbonrm = 0;                   /* Offset to Clr proc tb*/
   cp->asl.nclrproctbech = 0;                   /* # Color Proc tbl     */
   cp->asl.clrproctboech = 0;                   /* Offset to Clr Proc tb*/
   cp->asl.nlogclrtb     = 0;                   /* # Logical color tbl  */
   cp->asl.logclrtbo     = 0;                   /* Offset to Log clr tb */ 
   cp->asl. pkcorcntlblk = 0;                   /* Offset to Pick Correl*/
   cp->asl.pkinvhlghtfl  = 0;                   /* Offset to Pick, Invis*/
                                                /* and Hghlgh Filter tb */
   cp->asl.hatwkao     = 0;			/* offset to hwa	*/

   cp->asl.trsurwkao   = 0;                     /* Offset to Trim Surf  */
   cp->asl.shpwkao     = 0;                     /* Offset to Shp Wk Area*/
   cp->asl.rsvd1       = 0;                     /* Rsvd for Image Cd Sup*/
   cp->asl.rsvd2       = 0;                     /* Rsvd for Image Cd Sup*/
   cp->asl.rsvd3       = 0;                     /* Rsvd for Image Cd Sup*/
   cp->asl.langlktbo   = 0;                     /* Offset to Language Lk*/   
   cp->asl.nalang      = 0;                     /* National Language(Def*/     
   cp->asl.nechvwtb    = 0;                     /* # Echo view table    */
   cp->asl.echvwtbo    = 0;                     /* Offset to Echo vw tb */

   cp->asl.chngind   = 0;                       /* change indicators    */
   cp->asl.clrtblbg  = 0;                       /* clr tbl begin change */ 
   cp->asl.clrtblend = 0;                       /* clr table end change */

   cp->asl.lntblchngbm  = 0;                    /* line tbl change bits */
   cp->asl.pattblchngbm = 0;                    /* pat tbl change bits  */
   cp->asl.hattblchngbm = 0;                    /* hat tbl change bits  */

                                                /* Overlay Plane Masks  */
   cp->asl.ofrmbufmsk   = 0;                    /* ovrl plne frm bfr msk*/
   cp->asl.ocmpmsk      = 0;                    /* ovrl plne compare msk*/
   cp->asl.ocmpval      = 0;                    /* ovrl plne compare val*/

   cp->asl.dispmsk      = GMOverlayDisplayMask; /* display mask         */

   cp->asl.ologop	= 0;			/* ovrl plne logical op	*/

   cp->asl.woanntxtratio= 0;
   cp->asl.vwtbsgmdo    = 0;              /* Offset to View table for   @A*/
   cp->asl.lwdcwcrat	= itof(1);		/* line wid wind ratio	*/

   pTslot		=  (TRAV_SLOT *)gmcxt;
   cp->asl.prminfo  	= VME_ADR(&pTslot->rminfo); /* offset to dsv     */
   cp->asl.gtxtculdspmth= 1;			/* cull display method	*/
   cp->asl.gtxtculhgt	= 0;			/* culling height	*/


                                        /*------------------------------*/
                                        /* VT  initialization           */
                                        /* (view table)                 */
                                        /*------------------------------*/
   cp->vt.vwtindx    = 1;                       /* View Table Index     */
   cp->vt.vopflg     = 0x000E;                  /* View Operation Flag  */

   cp->vt.vtm[0]     = itof(1);     		/* View Trans Matrix    */
   cp->vt.vtm[1]     = 0;     			/*    device coords -   */
   cp->vt.vtm[2]     = 0;     			/*       upper left org */
   cp->vt.vtm[3]     = 0;     			/*                      */
   cp->vt.vtm[4]     = 0;     			/*                      */
   cp->vt.vtm[5]     = itof(1);     		/*                      */
   cp->vt.vtm[6]     = 0;     			/*                      */
   cp->vt.vtm[7]     = 0;     			/*                      */
   cp->vt.vtm[8]     = 0;     			/*                      */
   cp->vt.vtm[9]     = 0;     			/*                      */
   cp->vt.vtm[10]    = itof(1);     		/*                      */
   cp->vt.vtm[11]    = 0;     			/*                      */
   cp->vt.vtm[12]    = 0;     			/*                      */
   cp->vt.vtm[13]    = 0;     			/*                      */
   cp->vt.vtm[14]    = 0;     			/*                      */
   cp->vt.vtm[15]    = itof(1);     		/*                      */

   cp->vt.prp[0]     = 0;     			/* Proj Ref Pt (x)      */
   cp->vt.prp[1]     = 0;     			/* Proj Ref Pt (y)      */
   cp->vt.prp[2]     = itof(1);     		/* Proj Ref Pt (z)      */

   cp->vt.vpd         = 0;     			/* View Plane Distance  */

   cp->vt.truvp[0]   = 0;     			/* True Vwport (pixel)  */
   cp->vt.truvp[1]   = itof(400*1279); 		/* True Vwport (pixel)  */
   cp->vt.truvp[2]   = 0;     			/* True Vwport (pixel)  */
   cp->vt.truvp[3]   = itof(400*1023); 		/* True Vwport (pixel)  */
   cp->vt.truvp[4]   = 0;     			/* True Vwport (pixel)  */
   cp->vt.truvp[5]   = 0;     			/* True Vwport (pixel)  */
   cp->vt.truw2vr[0] = itof(1);     		/* True WtoVwport Ratio */
   cp->vt.truw2vr[1] = itof(1);     		/* True WtoVwport Ratio */
   cp->vt.truw2vr[2] = itof(1);     		/* True WtoVwport Ratio */
   cp->vt.truiw2vr[0]= itof(1);     		/* True Invrse WtoVwport*/
   cp->vt.truiw2vr[1]= itof(1);     		/* True Invrse WtoVwport*/
   cp->vt.truiw2vr[2]= itof(1);     		/* True Invrse WtoVwport*/
   cp->vt.truwvc[0]  = 0;     			/* True Window in VC    */
   cp->vt.truwvc[1]  = itof(400*1279); 		/* True Window in VC    */
   cp->vt.truwvc[2]  = 0;     			/* True Window in VC    */
   cp->vt.truwvc[3]  = itof(400*1023); 		/* True Window in VC    */
   cp->vt.truwvc[4]  = 0;     			/* True Window in VC    */
   cp->vt.truwvc[5]  = itof(1);     		/* True Window in VC    */
   cp->vt.npczclip[0] = 0;			/* Z clip bound in NPC	*/
   cp->vt.npczclip[1] = itof(1);		/* Z clip bound in NPC	*/
   cp->vt.shearparm[0] = itof(1279);		/* Shearing Parameters	*/
   cp->vt.shearparm[1] = itof(1023);		/* Shearing Parameters	*/
   cp->vt.shearparm[2] = itof(-1);		/* Shearing Parameters	*/

                                        /*------------------------------*/
                                        /* View Address Table           */
                                        /*      initialization          */
                                        /*------------------------------*/

   cp->vat[0]        = VME_ADR(&gmcxt->vt);     /* offset to vt         */

   return(0);
};
