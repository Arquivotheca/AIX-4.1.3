/* @(#)17       1.7.1.5  src/bos/kernext/disp/ped/pedmacro/hw_model.h, pedmacro, bos411, 9428A410j 4/21/94 10:48:11 */

/*
 * COMPONENT_NAME: PEDMACRO
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                 PEDERNALES HARDWARE PROGRAMMING INTERFACE                 */
/*                                                                           */
/*---------------------------------------------------------------------------*/

#ifndef _H_MID_HW_MODEL
#define _H_MID_HW_MODEL

#include   <sys/types.h>                     /* General type definitions     */
#include   <sys/rcm_win.h>                   /* RCM window definitions       */
#include   <sys/aixgsc.h>                    /* Subr aixgsc definitions      */
#include   <mid/hw_addrs.h>                  /* BIM hardware addresses       */

/*---------------------------------------------------------------------------*/
/*                  Define Terms Used Throughout Macro Layer                 */
/*---------------------------------------------------------------------------*/

#ifndef   MID_PRODUCTION_HW
#define   MID_PRODUCTION_HW       TRUE       /* For production hardware      */
#endif

#ifdef MID_RMS
   /* RMS does not need a MID_DATA_PTR */
#ifdef MID_RMS_PADAPTER_PTR
#  define MID_BASE_PTR    (*(MID_RMS_PADAPTER_PTR->pAdapter->pAttr->address))
#  define MID_FDL_PTR     (&(((pMID_rms_model_t)                               \
	                      (MID_RMS_PADAPTER_PTR->pAdapter->pExt))->fdl))
#  define MID_GSC_HANDLE  (MID_RMS_PADAPTER_PTR->pAdapter->pAttr->gsc_handle)
#endif /* MID_RMS_PADAPTER_PTR
   /* RMS does not use a defer buffer */
#elif MID_CDD
   /* CDD does not need a MID_DATA_PTR */
   /* CDD defines MID_BASE_PTR within each module that needs it. */
   /* CDD does not need fast domain locks */
   /* CDD does not use a defer buffer */
#elif MID_2D
#  define MID_DATA_PTR        (pGaiGC->gai.model_data_ptr)
#  define MID_BASE_PTR        (pGaiGC->gai.adapter_addr)
#  define MID_FDL_PTR         (MID_DATA_PTR->fdl_ptr)
#  define MID_GSC_HANDLE      (MID_DATA_PTR->gsc_handle)
#  define MID_DEFERBUF_HD     (MID_DATA_PTR->defer_buf)
#  define MID_DEFERBUF        (MID_DATA_PTR->next_defer_wd)
#  define MID_DEFERBUF_LEN    (MID_DATA_PTR->defer_buf_len)
#  define MID_DEFER_CAPACITY  (16000) /* changed from 64000...parag */
#elif MID_3DM1
#  define MID_DATA_PTR        (GFLptr->pMID_data_struc)
#  define MID_BASE_PTR        (GFLptr->base_addr)
#  define MID_FDL_PTR         (MID_DATA_PTR->fdl_ptr)
#  define MID_GSC_HANDLE      (MID_DATA_PTR->gsc_handle)
#  define MID_DEFERBUF_HD     (MID_DATA_PTR->defer_buf)
#  define MID_DEFERBUF        (MID_DATA_PTR->next_defer_wd)
#  define MID_DEFERBUF_LEN    (MID_DATA_PTR->defer_buf_len)
#  define MID_DEFER_CAPACITY  (65536) /* bytes = 16 k words */
#elif MID_3DM2
#  define MID_DATA_PTR        (GC->ddstuff.ml_model_data)
#  define MID_BASE_PTR        (GC->ddstuff.baseaddr)
#  define MID_FDL_PTR         (MID_DATA_PTR->fdl_ptr)
#  define MID_GSC_HANDLE      (GC->ddstuff.gsc_handle)
#  define MID_DEFERBUF_HD     (MID_DATA_PTR->defer_buf)
#  define MID_DEFERBUF        (MID_DATA_PTR->next_defer_wd)
#  define MID_DEFERBUF_LEN    (MID_DATA_PTR->defer_buf_len)
#  define MID_DEFER_CAPACITY  (16384) /* words */
#  define LOCALBUF_HD         (GC->ddstuff.local_buf)
#  define LOCALBUF            (GC->ddstuff.next_local_word)
#  define LOCALBUF_CAPACITY   (32768)  /* bytes = 8k words */
#elif MID_DD
#  define MID_DDF             (ddf)
#  define MID_DATA_PTR        (MID_DDF->model_data)
#  define MID_BASE_PTR        (MIDHWP)
   /* MID_DD does not need a fast domain lock structure */
#elif MID_TED
   /* Ted does not need a MID_DATA_PTR */
#  define MID_BASE_PTR        (card_adr)
#  define MID_FDL_PTR         (&fdl)
#  define MID_GSC_HANDLE      (gsc_handle)
   /* Ted does not use a defer buffer */
#endif

/*---------------------------------------------------------------------------*/
/*               Type Definitions Used Throughout Macro Layer                */
/*---------------------------------------------------------------------------*/

typedef struct  DomainLock fdl_t;

typedef struct _MID_model_data
{
	fdl_t *         fdl_ptr;              /* fast domain lock            */
	gHandle         gsc_handle;           /* gsc handle                  */

	/*-------------------------------------------------------------------*/
	/*               Things defining the defer buf                       *
	/*-------------------------------------------------------------------*/
	ulong *         defer_buf;            /* pointer to the defer buffer */
	ulong *         next_defer_wd;        /* used while adding to the buf*/
	ulong           defer_buf_len;        /* how much is in it?          */
} MID_model_data,
  *pMID_model_data;

typedef struct                                /* RMS extension structure     */
{
	fdl_t           fdl;                  /* fast domain lock            */
} MID_rms_model_t, *pMID_rms_model_t;

/*---------------------------------------------------------------------------*/
/*                  Initialization and Termination Macros                    */
/*---------------------------------------------------------------------------*/

#ifdef MID_RMS

#define MID_INIT_PCB_CONTEXT(rc,rms_pExt,gsc_handle)                          \
{                                                                             \
	create_rcx          _MID_PCB_Create_RCX;                              \
	mid_create_rcx_t    _MID_PCB_Create_RCX_Private;                      \
	                                                                      \
	create_win_geom     _MID_PCB_Create_Win_Geom;                         \
	gWinGeomAttributes  _MID_PCB_Win_Geom_Attr;                           \
	                                                                      \
	create_win_attr     _MID_PCB_Create_Win_Attr;                         \
	gWindowAttributes   _MID_PCB_Win_Attr;                                \
	                                                                      \
	bind_window         _MID_PCB_Bind_Win;                                \
	                                                                      \
	set_rcx             _MID_Set_RCX;                                     \
	                                                                      \
	rc = 0;                                                               \
	                                                                      \
	if (rms_pExt == NULL) /* Only malloc 1st time adapter is created */   \
	{                                                                     \
	        rms_pExt = (gPointer) gMalloc(sizeof( MID_rms_model_t ));     \
	                                                                      \
	        if (rms_pExt)                                                 \
	        {                                                             \
	                memset(rms_pExt, 0, sizeof( MID_rms_model_t ));       \
	        }                                                             \
	        else                                                          \
	        {                                                             \
	                rc = -1;                                              \
	        }                                                             \
	}                                                                     \
	                                                                      \
	if (rc == 0)                                                          \
	{                                                                     \
	        /* Create Rendering Context */                                \
	        memset(&_MID_PCB_Create_RCX_Private, 0,                       \
	                sizeof(_MID_PCB_Create_RCX_Private));                 \
	        _MID_PCB_Create_RCX_Private.rcx_type = RCX_PCB;               \
	        _MID_PCB_Create_RCX.error       = 0;                          \
	        _MID_PCB_Create_RCX.domain      = MID_PCB_DOMAIN;             \
	        _MID_PCB_Create_RCX.pDomainLock =                             \
	                &(((pMID_rms_model_t)(rms_pExt))->fdl);               \
	        _MID_PCB_Create_RCX.pData       =                             \
	                (genericPtr) &_MID_PCB_Create_RCX_Private;            \
	        _MID_PCB_Create_RCX.length      =                             \
	                sizeof(_MID_PCB_Create_RCX_Private);                  \
	        _MID_PCB_Create_RCX.rcx_handle  = 0;                          \
	        rc = aixgsc (gsc_handle, CREATE_RCX,                          \
	                &_MID_PCB_Create_RCX);                                \
	}                                                                     \
	                                                                      \
	if (rc == 0)                                                          \
	{                                                                     \
	        /* Create Window Geometry */                                  \
	        memset(&_MID_PCB_Win_Geom_Attr, 0,                            \
	                sizeof(_MID_PCB_Win_Geom_Attr));                      \
	        _MID_PCB_Create_Win_Geom.error = 0;                           \
	        _MID_PCB_Create_Win_Geom.pWG = &_MID_PCB_Win_Geom_Attr;       \
	        _MID_PCB_Create_Win_Geom.wg = 0;                              \
	        rc = aixgsc(gsc_handle, CREATE_WIN_GEOM,                      \
	                &_MID_PCB_Create_Win_Geom);                           \
	}                                                                     \
	                                                                      \
	if (rc == 0)                                                          \
	{                                                                     \
	        /* Create Window Attributes */                                \
	        memset(&_MID_PCB_Win_Attr, 0, sizeof(_MID_PCB_Win_Attr));     \
	        _MID_PCB_Create_Win_Attr.error = 0;                           \
	        _MID_PCB_Create_Win_Attr.pWA = &_MID_PCB_Win_Attr;            \
	        _MID_PCB_Create_Win_Attr.wa = 0;                              \
	        rc = aixgsc(gsc_handle, CREATE_WIN_ATTR,                      \
	                &_MID_PCB_Create_Win_Attr);                           \
	}                                                                     \
	                                                                      \
	if (rc == 0)                                                          \
	{                                                                     \
	        /* Bind Window */                                             \
	        _MID_PCB_Bind_Win.error = 0;                                  \
	        _MID_PCB_Bind_Win.rcx   = _MID_PCB_Create_RCX.rcx_handle;     \
	        _MID_PCB_Bind_Win.wg    = _MID_PCB_Create_Win_Geom.wg;        \
	        _MID_PCB_Bind_Win.wa    = _MID_PCB_Create_Win_Attr.wa;        \
	        rc = aixgsc(gsc_handle, BIND_WINDOW, &_MID_PCB_Bind_Win);     \
	}                                                                     \
	                                                                      \
	if (rc == 0)                                                          \
	{                                                                     \
	        /* Set Rendering Context */                                   \
	        _MID_Set_RCX.error = 0;                                       \
	        _MID_Set_RCX.rcx   = _MID_PCB_Create_RCX.rcx_handle;          \
	        rc = aixgsc(gsc_handle, SET_RCX, &_MID_Set_RCX);              \
	}                                                                     \
}

  /* RMS does not need a MID_INIT */
  /* RMS does not need a MID_DESTROY */

#elif MID_CDD

  /* CDD does not need a MID_INIT */
  /* CDD does not need a MID_DESTROY */

#elif MID_2D

#define MID_INIT(rc,fdl_ptr,gsc_handle)                                       \
{                                                                             \
	MID_DATA_PTR = (pMID_model_data) xalloc (                             \
	        sizeof( MID_model_data ) +                                    \
	        (MID_DEFER_CAPACITY<<2));                                     \
	                                                                      \
	if (MID_DATA_PTR)                                                     \
	{                                                                     \
	        memset(MID_DATA_PTR, 0,                                       \
	                sizeof( MID_model_data ) +                            \
	                (MID_DEFER_CAPACITY<<2));                             \
	                                                                      \
	        MID_FDL_PTR    = fdl_ptr;                                     \
	        MID_GSC_HANDLE = gsc_handle;                                  \
	                                                                      \
	        MID_DEFERBUF_HD = (ulong *)                                   \
	                (((char *) (MID_DATA_PTR)) +                          \
	                sizeof(MID_model_data));                              \
	        MID_DEFERBUF     = MID_DEFERBUF_HD;                           \
	        MID_DEFERBUF_LEN = 0;                                         \
	                                                                      \
	        rc = 0;                                                       \
	}                                                                     \
	else                                                                  \
	{                                                                     \
	        rc = -1;                                                      \
	}                                                                     \
}

#define MID_DESTROY                                                           \
{                                                                             \
	if (MID_DATA_PTR)                                                     \
	        xfree(MID_DATA_PTR);                                          \
}

#elif MID_3DM1

#define MID_INIT(rc,fdl_ptr,gsc_handle)                                       \
{                                                                             \
	MID_DATA_PTR = (pMID_model_data) calloc (1,                           \
	        sizeof( MID_model_data ) +                                    \
	        (MID_DEFER_CAPACITY<<2));                                     \
	                                                                      \
	if (MID_DATA_PTR)                                                     \
	{                                                                     \
	        MID_FDL_PTR    = fdl_ptr;                                     \
	        MID_GSC_HANDLE = gsc_handle;                                  \
	                                                                      \
	        MID_DEFERBUF_HD = (ulong *)                                   \
	                (((char *) (MID_DATA_PTR)) +                          \
	                sizeof(MID_model_data));                              \
	        MID_DEFERBUF     = MID_DEFERBUF_HD;                           \
	        MID_DEFERBUF_LEN = 0;                                         \
	                                                                      \
	        rc = 0;                                                       \
	}                                                                     \
	else                                                                  \
	{                                                                     \
	        rc = -1;                                                      \
	}                                                                     \
}

#define MID_DESTROY                                                           \
{                                                                             \
	if (MID_DATA_PTR)                                                     \
	        free( MID_DATA_PTR );                                         \
}

#elif MID_3DM2

#define MID_INIT(rc,fdl_ptr,gsc_handle)                                       \
{                                                                             \
	MID_DATA_PTR = (pMID_model_data) calloc (1,                           \
	        sizeof( MID_model_data ) +                                    \
	        (MID_DEFER_CAPACITY<<2)  +                                    \
	        (LOCALBUF_CAPACITY));                                         \
	                                                                      \
	if (MID_DATA_PTR)                                                     \
	{                                                                     \
	        MID_FDL_PTR    = fdl_ptr;                                     \
	        MID_GSC_HANDLE = gsc_handle;                                  \
	                                                                      \
	        MID_DEFERBUF_HD = (ulong *)                                   \
	                (((char *) (MID_DATA_PTR)) +                          \
	                sizeof(MID_model_data));                              \
	        MID_DEFERBUF     = MID_DEFERBUF_HD;                           \
	        MID_DEFERBUF_LEN = 0;                                         \
	                                                                      \
	        LOCALBUF_HD = (ulong *)                                       \
	                (((char *) (MID_DEFERBUF_HD)) +                       \
	                (MID_DEFER_CAPACITY<<2));                             \
	        LOCALBUF = LOCALBUF_HD;                                       \
	                                                                      \
	        rc = 0;                                                       \
	}                                                                     \
	else                                                                  \
	{                                                                     \
	        rc = -1;                                                      \
	}                                                                     \
}

#define MID_DESTROY                                                           \
{                                                                             \
	if (MID_DATA_PTR)                                                     \
	        free( MID_DATA_PTR );                                         \
}

#elif MID_DD

  /* The device driver does not need a MID_INIT */
  /* The device driver does not need a MID_DESTROY */
  /* This define can be removed when the call is removed from mid_close */
#define MID_DESTROY

#elif MID_TED

#define MID_INIT(rc,fdl_ptr,gsc_handle)                                       \
{                                                                             \
	MID_INIT_HW_PTRS();                                                   \
	rc = 0;                                                               \
}

  /* Ted does not need a MID_DESTROY */

#endif

/*---------------------------------------------------------------------------*/
/*                     BIM Register Pointer Definitions                      */
/*                                                                           */
/* The following terms may be used to directly define the addresses for each */
/* of the BIM registers that can be directly accessed by the host.  If they  */
/* are not defined by the model, they are computed as offsets from the base  */
/* address each time the host accesses the adapter.                          */
/*---------------------------------------------------------------------------*/

#ifdef MID_TED

   extern  fdl_t                   fdl;
   extern  gHandle                 gsc_handle;

   extern  volatile char  *        pos_adr;
   extern  volatile ulong *        card_adr;
   extern  volatile ulong *        mid_ptr_fifo_data;
   extern  volatile ulong *        mid_ptr_free_space;
   extern  volatile ulong *        mid_ptr_pcb_data;
   extern  volatile ulong *        mid_ptr_ind_control;
   extern  volatile ulong *        mid_ptr_ind_address;
   extern  volatile ulong *        mid_ptr_ind_data;
   extern  volatile ulong *        mid_ptr_dsp_control;
   extern  volatile ulong *        mid_ptr_card_int_mask;
   extern  volatile ulong *        mid_ptr_host_int_mask;
   extern  volatile ulong *        mid_ptr_host_status;
   extern  volatile ulong *        mid_ptr_card_commo;
   extern  volatile ulong *        mid_ptr_host_commo;

#  define MID_PTR_FIFO_DATA     (mid_ptr_fifo_data)
#  define MID_PTR_FREE_SPACE    (mid_ptr_free_space)
#  define MID_PTR_PCB_DATA      (mid_ptr_pcb_data)
#  define MID_PTR_IND_CONTROL   (mid_ptr_ind_control)
#  define MID_PTR_IND_ADDRESS   (mid_ptr_ind_address)
#  define MID_PTR_IND_DATA      (mid_ptr_ind_data)
#  define MID_PTR_DSP_CONTROL   (mid_ptr_dsp_control)
#  define MID_PTR_CARD_INT_MASK (mid_ptr_card_int_mask)
#  define MID_PTR_HOST_INT_MASK (mid_ptr_host_int_mask)
#  define MID_PTR_HOST_STATUS   (mid_ptr_host_status)
#  define MID_PTR_CARD_COMMO    (mid_ptr_card_commo)
#  define MID_PTR_HOST_COMMO    (mid_ptr_host_commo)

#endif /* MID_TED */

/*---------------------------------------------------------------------------*/
/* The following are the default definitions of hardware addresses           */
/* that are used, if the model has not defined them.  The assumption         */
/* is that all or none will be defined.  Therefore, only one is              */
/* tested before creating the defaults                                       */
/*---------------------------------------------------------------------------*/

#define   MID_MAKE_PTR(adr) (volatile ulong *)((((int) (MID_BASE_PTR)) + (adr)))

#ifndef MID_PTR_FIFO_DATA
#define   MID_PTR_FIFO_DATA     MID_MAKE_PTR( MID_ADR_FIFO_DATA )
#define   MID_PTR_FREE_SPACE    MID_MAKE_PTR( MID_ADR_FREE_SPACE )
#define   MID_PTR_PCB_DATA      MID_MAKE_PTR( MID_ADR_PCB_DATA )
#define   MID_PTR_IND_CONTROL   MID_MAKE_PTR( MID_ADR_IND_CONTROL )
#define   MID_PTR_IND_ADDRESS   MID_MAKE_PTR( MID_ADR_IND_ADDRESS )
#define   MID_PTR_IND_DATA      MID_MAKE_PTR( MID_ADR_IND_DATA )
#define   MID_PTR_DSP_CONTROL   MID_MAKE_PTR( MID_ADR_DSP_CONTROL )
#define   MID_PTR_CARD_INT_MASK MID_MAKE_PTR( MID_ADR_CARD_INT_MASK )
#define   MID_PTR_HOST_INT_MASK MID_MAKE_PTR( MID_ADR_HOST_INT_MASK )
#define   MID_PTR_HOST_STATUS   MID_MAKE_PTR( MID_ADR_HOST_STATUS )
#define   MID_PTR_CARD_COMMO    MID_MAKE_PTR( MID_ADR_CARD_COMMO )
#define   MID_PTR_HOST_COMMO    MID_MAKE_PTR( MID_ADR_HOST_COMMO )
#endif /* not MID_PTR_FIFO_DATA */

/*---------------------------------------------------------------------------*/
/* The following macro initialized the hardware addresses for those models   */
/* which have defined them directly.                                         */
/*---------------------------------------------------------------------------*/

#define MID_INIT_HW_PTRS( )                                                   \
{                                                                             \
	MID_PTR_FIFO_DATA     = MID_MAKE_PTR( MID_ADR_FIFO_DATA     );        \
	MID_PTR_FREE_SPACE    = MID_MAKE_PTR( MID_ADR_FREE_SPACE    );        \
	MID_PTR_PCB_DATA      = MID_MAKE_PTR( MID_ADR_PCB_DATA      );        \
	MID_PTR_IND_CONTROL   = MID_MAKE_PTR( MID_ADR_IND_CONTROL   );        \
	MID_PTR_IND_ADDRESS   = MID_MAKE_PTR( MID_ADR_IND_ADDRESS   );        \
	MID_PTR_IND_DATA      = MID_MAKE_PTR( MID_ADR_IND_DATA      );        \
	MID_PTR_DSP_CONTROL   = MID_MAKE_PTR( MID_ADR_DSP_CONTROL   );        \
	MID_PTR_CARD_INT_MASK = MID_MAKE_PTR( MID_ADR_CARD_INT_MASK );        \
	MID_PTR_HOST_INT_MASK = MID_MAKE_PTR( MID_ADR_HOST_INT_MASK );        \
	MID_PTR_HOST_STATUS   = MID_MAKE_PTR( MID_ADR_HOST_STATUS   );        \
	MID_PTR_CARD_COMMO    = MID_MAKE_PTR( MID_ADR_CARD_COMMO    );        \
	MID_PTR_HOST_COMMO    = MID_MAKE_PTR( MID_ADR_HOST_COMMO    );        \
}

#endif /* _H_MID_HW_MODEL */
