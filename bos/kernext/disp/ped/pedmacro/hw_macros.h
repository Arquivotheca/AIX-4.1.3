/* @(#)16	1.9.1.9  src/bos/kernext/disp/ped/pedmacro/hw_macros.h, pedmacro, bos411, 9428A410j 4/14/94 17:31:47 */

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
/*                PEDERNALES HW MACRO PROGRAMMING INTERFACE                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/


#ifndef _H_MID_HW_MACROS
#define _H_MID_HW_MACROS

#include <string.h>                         /* For memory copies < 64 bytes */
#include <mid/hw_locks.h>                   /* PCB and indirect regs locks  */
#include <mid/hw_seops.h>                   /* Structure element opcodes    */
#include <mid/hw_trace.h>                   /* Trace definitions and macros */
#include <mid/hw_defer.h>                   /* Defer buffer macros */
#include <mid/hw_ind_mac.h>                 /* Indirect register macros */
#include <mid/hw_dd_io_trace.h>             /* New trace definitions */

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                               ABBREVIATIONS                               */
/*                                                                           */
/*      RD      =       read                                                 */
/*      WR      =       write                                                */
/*      GE      =       greater than or equal                                */
/*      LE      =       less than or equal                                   */
/*      EQ      =       equal                                                */
/*      NE      =       not equal                                            */
/*      PCB     =       priority command buffer                              */
/*      IND     =       indirect access                                      */
/*      FIFO    =       circular buffer fifo                                 */
/*      REG     =       register                                             */
/*      CTL     =       control                                              */
/*      ADR     =       address                                              */
/*      DATA    =       data                                                 */
/*      COMO    =       communication                                        */
/*      COMMO   =       communication                                        */
/*      FREE    =       free space                                           */
/*      INTR    =       interrupt                                            */
/*      HOST    =       host                                                 */
/*      CARD    =       card                                                 */
/*      STAT    =       status                                               */
/*      REL     =       relative                                             */
/*      K       =       kernel                                               */
/*      MEM     =       memory                                               */
/*      DSP     =       digital signal processor                             */
/*                                                                           */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                    Macros for Constructing the SE Headers                 */
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define MID_WORDS( num )        (num)   /* Makes reading the macros easier   */

#define MID_E_SE_LEN( words )   ( words << 2 )

#define MID_E_SE_HDR( num_words , opcode )                                    \
        ( ( (MID_E_SE_LEN( num_words )) << 16 ) | opcode )

#define MID_WORDS_SE_DATA( num_struct, struct_name)                           \
        ( ( num_struct * sizeof( struct_name) ) >> 2 )


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                         Macros Which Read Registers                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/* Define General Macro for Reading any Register */
#ifdef MID_CDD
#define _MID_RD_VALUE( host_adr, value )                                      \
{                                                                             \
        MID_ddf->MID_rc = CDD_busgetl(  MID_ddf->MID_cdd,                     \
                                        MID_ddf->MID_seg,                     \
                                        (host_adr),                           \
                                        &(value)        );                    \
}
#else /* not MID_CDD */
#define _MID_RD_VALUE( host_adr, value)                                       \
{                                                                             \
        value = *( (volatile ulong *) (host_adr) );                           \
}
#endif /* not MID_CDD */

/* Define General Macro for Reading and Traceing any Register */
#define _MID_RD_TR_REG( host_adr, bim_adr, value)                             \
{                                                                             \
        _MID_RD_VALUE( host_adr, value )                                      \
        MID_RD_TRACE( bim_adr, value )                                        \
}

#define MID_RD_REG( host_adr, value )  /* Read Register by Address */         \
        _MID_RD_TR_REG( host_adr, _MID_BIM_ADR( host_adr ), value )

#define MID_RD_DSP_CTL( value )         /* Read DSP Control Register */       \
        _MID_RD_TR_REG( MID_PTR_DSP_CONTROL, MID_ADR_DSP_CONTROL, value )

#define MID_RD_IND_CTL( value )         /* Read Indirect Control Register */  \
        _MID_RD_TR_REG( MID_PTR_IND_CONTROL, MID_ADR_IND_CONTROL, value )

#define MID_RD_IND_ADR( value )         /* Read Indirect Address Register */  \
        _MID_RD_TR_REG( MID_PTR_IND_ADDRESS, MID_ADR_IND_ADDRESS, value )

#define MID_RD_IND_DATA( value )        /* Read Indirect Data Register */     \
        _MID_RD_TR_REG( MID_PTR_IND_DATA, MID_ADR_IND_DATA, value )

#define MID_RD_FIFO_FREE( value )       /* Read FIFO Free Space Register */   \
        _MID_RD_TR_REG( MID_PTR_FREE_SPACE, MID_ADR_FREE_SPACE, value )

#define MID_RD_HOST_INTR( value )       /* Read Host Interupt Mask Reg */     \
        _MID_RD_TR_REG( MID_PTR_HOST_INT_MASK, MID_ADR_HOST_INT_MASK, value )

#define MID_RD_HOST_STAT( value )       /* Read Host Status Register */       \
        _MID_RD_TR_REG( MID_PTR_HOST_STATUS, MID_ADR_HOST_STATUS, value )

#define MID_RD_HOST_COMO( value )       /* Read Host Commo Register */        \
        _MID_RD_TR_REG( MID_PTR_HOST_COMMO, MID_ADR_HOST_COMMO, value )

#define MID_RD_CARD_INTR( value )       /* Read Card (DSP) Intr Mask Reg */   \
        _MID_RD_TR_REG( MID_PTR_CARD_INT_MASK, MID_ADR_CARD_INT_MASK, value )

#define MID_RD_CARD_COMO( value )       /* Read Card (DSP) Commo Register */  \
        _MID_RD_TR_REG( MID_PTR_CARD_COMMO, MID_ADR_CARD_COMMO, value )


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                         Macros Which Write Registers                      */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/* Define General Macro for Writeing to any Register */
#ifdef MID_CDD
#define _MID_WR_VALUE( host_adr, value )                                      \
{                                                                             \
        MID_ddf->MID_rc = CDD_busputl(  MID_ddf->MID_cdd,                     \
                                        MID_ddf->MID_seg,                     \
                                        (host_adr),                           \
                                        (value)         );                    \
}
#else /* not MID_CDD */
#define _MID_WR_VALUE( host_adr, value )                                      \
{                                                                             \
        *( (volatile ulong *) (host_adr) ) = ( ulong ) (value);               \
}
#endif /* not MID_CDD */

#define _MID_WR_TR_REG( host_adr, bim_adr, value)                             \
{                                                                             \
        _MID_WR_VALUE( host_adr, value )                                      \
        MID_WR_TRACE( bim_adr, value )                                        \
}

#define MID_WR_REG( host_adr , value )   /* Write Register by Bim Address */  \
        _MID_WR_TR_REG( host_adr, _MID_BIM_ADR( host_adr ), value )

#define MID_WR_DSP_CTL( value )         /* Write to DSP Control Register */   \
        _MID_WR_TR_REG( MID_PTR_DSP_CONTROL, MID_ADR_DSP_CONTROL, value )

#define MID_WR_IND_CTL( value )         /* Write to Indirect Contorl Reg */   \
        _MID_WR_TR_REG( MID_PTR_IND_CONTROL, MID_ADR_IND_CONTROL, value )

#define MID_WR_IND_ADR( value )         /* Write to Indirect Address Reg */   \
        _MID_WR_TR_REG( MID_PTR_IND_ADDRESS, MID_ADR_IND_ADDRESS, value )

#define MID_WR_IND_DATA( value )        /* Write to Indirect Data Register */ \
        _MID_WR_TR_REG( MID_PTR_IND_DATA, MID_ADR_IND_DATA, value )

#define MID_WR_HOST_INTR( value )       /* Write to Host Interrupt Mask Reg */\
        _MID_WR_TR_REG( MID_PTR_HOST_INT_MASK, MID_ADR_HOST_INT_MASK, value )

#define MID_WR_HOST_STAT( value )       /* Write to Host Status Register */   \
        _MID_WR_TR_REG( MID_PTR_HOST_STATUS, MID_ADR_HOST_STATUS, value )

#define MID_WR_HOST_COMO( value )       /* Write to Host Commo Register */    \
        _MID_WR_TR_REG( MID_PTR_HOST_COMMO, MID_ADR_HOST_COMMO, value )


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                         Macros Which Poll Registers                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/* Define Test which must FAIL to Exit Poll Loop */
#define MID_NE_TEST     ==
#define MID_EQ_TEST     !=
#define MID_LE_TEST     >
#define MID_LT_TEST     >=
#define MID_GE_TEST     <
#define MID_GT_TEST     <=

/* Define General Macro for Polling any Register */
#ifdef MID_CDD
#define _MID_POLL_VALUE( host_adr, value, test, test_value )                  \
{                                                                             \
        DEFINE_POLL_COUNT( -1 )                                               \
        do                                                                    \
        {                                                                     \
                _MID_RD_VALUE( host_adr, value )                              \
                INCREMENT_POLL_COUNT                                          \
        }                                                                     \
        while(     ( (value) test (test_value) )                              \
                && ( MID_ddf->MID_rc == 0 )     );                            \
        MID_PC_TRACE(_MID_BIM_ADR( host_adr ))                                \
}
#else /* not MID_CDD */
#define _MID_POLL_VALUE( host_adr, value, test, test_value )                  \
{                                                                             \
        DEFINE_POLL_COUNT( -1 )                                               \
        do                                                                    \
        {                                                                     \
                _MID_RD_VALUE( host_adr , value )                             \
                INCREMENT_POLL_COUNT                                          \
        }                                                                     \
        while ( (value) test (test_value) );                                  \
        MID_PC_TRACE(_MID_BIM_ADR( host_adr ))                                \
}
#endif /* not MID_CDD */


/* Define General Macro for Polling any Register with a Mask */
#ifdef MID_CDD
#define _MID_POLL_VALUE_MASK( host_adr, value, test, test_value, mask )       \
{                                                                             \
        DEFINE_POLL_COUNT( -1 )                                               \
        do                                                                    \
        {                                                                     \
                _MID_RD_VALUE( host_adr, value )                              \
                INCREMENT_POLL_COUNT                                          \
        }                                                                     \
        while(     ( (value & mask) test (test_value) )                       \
                && ( MID_ddf->MID_rc == 0 )     );                            \
        MID_PC_TRACE(_MID_BIM_ADR( host_adr ))                                \
}
#else /* not MID_CDD */
#define _MID_POLL_VALUE_MASK( host_adr, value, test, test_value, mask )       \
{                                                                             \
        DEFINE_POLL_COUNT( -1 )                                               \
        do                                                                    \
        {                                                                     \
                _MID_RD_VALUE( host_adr , value )                             \
                INCREMENT_POLL_COUNT                                          \
        }                                                                     \
        while ( (value & mask) test (test_value) );                           \
        MID_PC_TRACE(_MID_BIM_ADR( host_adr ))                                \
}
#endif /* not MID_CDD */


/* Define General Macro for Polling any Register with a Mask and Timeout */
#ifdef MID_CDD
#define _MID_POLL_VALUE_MASK_TO( host_adr, value, test, test_value, mask,time)\
{                                                                             \
        int     _mac_time = time;                                             \
        DEFINE_POLL_COUNT( -1 )                                               \
        do                                                                    \
        {                                                                     \
                _MID_RD_VALUE( host_adr, value )                              \
                INCREMENT_POLL_COUNT                                          \
        }                                                                     \
        while (   ((value & mask) test (test_value))                          \
                && ( _mac_time-- > 0 )                                        \
                && (MID_ddf->MID_rc == 0 )      );                            \
        MID_PC_TRACE(_MID_BIM_ADR( host_adr ))                                \
}
#elif MID_DD
#define _MID_POLL_VALUE_MASK_TO( host_adr, value, test, test_value, mask,time)\
{                                                                             \
        int     _mac_time = time;                                             \
        DEFINE_POLL_COUNT( 0 )                                                \
        _MID_RD_VALUE( host_adr, value )                                      \
        while ( ((value & mask) test (test_value)) && _mac_time-- )           \
        {                                                                     \
                mid_delay(1);                                                 \
                _MID_RD_VALUE( host_adr, value )                              \
                INCREMENT_POLL_COUNT                                          \
        }                                                                     \
        MID_PC_TRACE(_MID_BIM_ADR( host_adr ))                                \
        if (! _mac_time)                                                      \
        {                                                                     \
                if (! (ddf->hwconfig & MID_BAD_UCODE) )                       \
                {                                                             \
                        char msgbuf[sizeof(__FILE__)+6];                      \
                        ddf->hwconfig |= MID_BAD_UCODE;                       \
                        sprintf(msgbuf, "%s,%d",__FILE__,__LINE__);           \
                        miderr(ddf,NULL,"mid-level","POLL_IND","TIMEOUT",     \
                                NULL, NULL, msgbuf);                          \
                }                                                             \
        }                                                                     \
}
#else /* not MID_CDD and not MID_DD */
#define _MID_POLL_VALUE_MASK_TO( host_adr, value, test, test_value, mask,time)\
{                                                                             \
        int     _mac_time = time;                                             \
        DEFINE_POLL_COUNT( -1 )                                               \
        do                                                                    \
        {                                                                     \
                _MID_RD_VALUE( host_adr, value )                              \
                INCREMENT_POLL_COUNT                                          \
        }                                                                     \
        while ( ((value & mask) test (test_value)) && _mac_time-- );          \
        MID_PC_TRACE(_MID_BIM_ADR( host_adr ))                                \
}
#endif /* not MID_CDD and not MID_DD */

/* Define General Macro for Polling and Traceing any Register */
#define _MID_POLL_TR_REG( host_adr, value, test, test_value, trace_mac )      \
{                                                                             \
        volatile ulong *_mac_host_adr = host_adr;                             \
        trace_mac( _MID_BIM_ADR( _mac_host_adr ), test_value, -1 )            \
        _MID_POLL_VALUE( _mac_host_adr, value, test, test_value )             \
        MID_FV_TRACE( _MID_BIM_ADR( _mac_host_adr ), value )                  \
}

/* Define General Macro for Polling and Traceing any Register with a Mask */
#define _MID_POLL_TR_REG_MASK( host_adr,value,test,test_value,mask,trace_mac )\
{                                                                             \
        volatile ulong *_mac_host_adr = host_adr;                             \
        trace_mac( _MID_BIM_ADR( _mac_host_adr ), test_value, mask )          \
        _MID_POLL_VALUE_MASK( _mac_host_adr, value, test, test_value, mask )  \
        MID_FV_TRACE( _MID_BIM_ADR( _mac_host_adr ), (value & mask) )         \
}

/* Define General Macro for Polling & Traceing any Reg with a Mask & Timeout */
#define _MID_POLL_TR_REG_MASK_TO( host_adr,value,test,test_value,mask,time,   \
        trace_mac )                                                           \
{                                                                             \
        volatile ulong *_mac_host_adr = host_adr;                             \
        trace_mac( _MID_BIM_ADR( _mac_host_adr ), test_value, mask )          \
        _MID_POLL_VALUE_MASK_TO(_mac_host_adr,value,test,test_value,mask,time)\
        MID_FV_TRACE( _MID_BIM_ADR( _mac_host_adr ), (value & mask) )         \
}

/* Poll 'host_adr' until read 'value' is >= 'test_value' */
#define MID_POLL_REG_GE( host_adr, value, test_value )                        \
        _MID_POLL_TR_REG( host_adr, value, MID_GE_TEST, test_value,           \
                MID_GE_TRACE )

/* Poll 'host_adr' until read 'value' is <= 'test_value' */
#define MID_POLL_REG_LE( host_adr, value, test_value )                        \
        _MID_POLL_TR_REG( host_adr, value, MID_LE_TEST, test_value,           \
                MID_LE_TRACE )

/* Poll 'host_adr' until read 'value' is == 'test_value' */
#define MID_POLL_REG_EQ( host_adr, value, test_value )                        \
        _MID_POLL_TR_REG( host_adr, value, MID_EQ_TEST, test_value,           \
                MID_EQ_TRACE )

/* Poll 'host_adr' until read 'value' is != 'test_value' */
#define MID_POLL_REG_NE( host_adr, value, test_value )                        \
        _MID_POLL_TR_REG( host_adr, value, MID_NE_TEST, test_value,           \
                MID_NE_TRACE )

/* Poll 'host_adr' until read 'value & mask' is == 'test_value' */
#define MID_POLL_REG_EQ_MASK( host_adr, value, test_value, mask )             \
        _MID_POLL_TR_REG_MASK( host_adr, value, MID_EQ_TEST, test_value, mask,\
                MID_EQ_TRACE )

/* Poll 'host_adr' until read 'value & mask' is != 'test_value' */
#define MID_POLL_REG_NE_MASK( host_adr, value, test_value, mask )             \
        _MID_POLL_TR_REG_MASK( host_adr, value, MID_NE_TEST, test_value, mask,\
                MID_NE_TRACE )

/* Poll 'host_adr' until read 'value & mask' is == 'test_value' */
#define MID_POLL_REG_EQ_MASK_TO( host_adr, value, test_value, mask, time )    \
        _MID_POLL_TR_REG_MASK_TO( host_adr, value, MID_EQ_TEST, test_value,   \
        mask, time, MID_EQ_TRACE )


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                       Macros Which Write to the FIFO                      */
/*                                                                           */
/*---------------------------------------------------------------------------*/

#ifdef MID_RMS

/* RMS does not write to the fifo */

#elif MID_CDD

#define MID_PIO_BLOCK_TRANSFER_SIZE     16
#define MID_FIFO_FREE_SPACE_SLACK       8

#ifndef MACRO_DEBUG

#define _MID_WAIT_FIFO_FREE( free_space, num_left )                           \
{                                                                             \
        _MID_RD_VALUE( MID_PTR_FREE_SPACE, free_space )                       \
        RETURN_ON_EXCEPTION;                                                  \
                                                                              \
        if ( (free_space < (MID_PIO_BLOCK_TRANSFER_SIZE                       \
                         +  MID_FIFO_FREE_SPACE_SLACK)) &&                    \
             (free_space < ((num_left)                                        \
                         +  MID_FIFO_FREE_SPACE_SLACK)) )                     \
        {                                                                     \
                ulong   _mac_min_space;         /* Minumum space required */  \
                MID_FS_TRACE( )                                               \
                _mac_min_space = ((num_left) < MID_PIO_BLOCK_TRANSFER_SIZE)   \
                                ? (num_left) : MID_PIO_BLOCK_TRANSFER_SIZE;   \
                _mac_min_space += MID_FIFO_FREE_SPACE_SLACK;                  \
                _MID_POLL_VALUE_MASK_TO( MID_PTR_FREE_SPACE,                  \
                        free_space, MID_GE_TEST,                              \
                        _mac_min_space, ~0, MID_APPROX_1_SEC   )              \
        }                                                                     \
        free_space -= MID_FIFO_FREE_SPACE_SLACK;                              \
}

/*---------------------------------------------------------------------------*/
/* This macro will write any number of words into the adapter FIFO           */
/*---------------------------------------------------------------------------*/

#define MID_WR_FIFO( pData, num_words )                                       \
{                                                                             \
        ulong           _mac_num_left = (num_words);                          \
        ulong *         _mac_ptr = ( ulong * ) (pData);                       \
                                                                              \
        while (_mac_num_left > 0)                                             \
        {                                                                     \
                if ( _mac_num_left > ( MID_ddf -> MID_free_space ) )          \
                {                                                             \
                        _MID_WAIT_FIFO_FREE( MID_ddf -> MID_free_space,       \
                                            _mac_num_left )                   \
                }                                                             \
                while( MID_ddf -> MID_free_space > 0 && _mac_num_left > 0)    \
                {                                                             \
                        MID_ddf->MID_rc = CDD_busputl(  MID_ddf->MID_cdd,     \
                                                        MID_ddf->MID_seg,     \
                                                        MID_PTR_FIFO_DATA,    \
                                                        *(_mac_ptr)     );    \
                        MID_WR_TRACE_FIFO( _mac_ptr, 1 )                      \
                        RETURN_ON_EXCEPTION;                                  \
                        _mac_ptr++;                                           \
                        MID_ddf -> MID_free_space--;                          \
                        _mac_num_left--;                                      \
                }                                                             \
        }                                                                     \
}
#endif  /* MACRO_DEBUG */

#elif MID_DD

/*---------------------------------------------------------------------------*/
/* This macro is used to write words to the FIFO.  The input parameter       */
/* 'pData' is the array of words to be written and the input parameter       */
/* 'num_words' is the length of the array in words.                          */
/*                                                                           */
/* This macro writes to the FIFO without checking for Free Space.  The       */
/* process using these macros must stop transferring data when the high      */
/* water mark is reached and restart when the low water mark is reached.     */
/*                                                                           */
/* Only the general version of MID_WR_FIFO is provided for the device driver */
/* since it seldom writes to the FIFO.                                       */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* This macro will write any number of words into the adapter FIFO           */
/*---------------------------------------------------------------------------*/
#define MID_BIM_MAX_STORE_MULT  16

#define MID_WR_FIFO( pData, num_words )                                       \
{                                                                             \
 /*--------------------------------------------------------------------*      \
   The first two variable declared here depend on the variable "MID_DDF"      \
   which is defined to "ddf" in hw_model.h.  This obviously places a          \
   restriction on all users of this macro to have the variable "ddf"          \
   defined to be the pointer to the Ped device dependent structure.           \
 ---------------------------------------------------------------------*/      \
    gscDev          *pGD = MID_DDF->pdev ;                                    \
    mid_rcx_t       *midRCX = MID_DDF->current_context_midRCX;                \
    ulong *         _mac_ptr = (ulong * )(pData);                             \
    int             _mac_num_write;                                           \
    int             _mac_num_left ;                                           \
    int             _mac_old_int ;                                            \
    volatile ulong *_mac_fifo_data = MID_PTR_FIFO_DATA;                       \
                                                                              \
    if (midRCX == NULL)  /* This is NULL if no context is on the adapter */   \
    {                                                                         \
        if (! (ddf->hwconfig & MID_BAD_UCODE) )                               \
        {                                                                     \
            char msgbuf[sizeof(__FILE__)+6];                                  \
            ddf->hwconfig |= MID_BAD_UCODE;                                   \
            sprintf(msgbuf, "%s,%d",__FILE__,__LINE__);                       \
            miderr(ddf,NULL,"mid-level","MID_WR_FIFO","NULL midRCX",          \
                NULL, NULL, msgbuf);                                          \
        }                                                                     \
    }                                                                         \
    else                                                                      \
    {                                                                         \
        _mac_old_int = i_disable (INTMAX) ;                                   \
        for (_mac_num_left = num_words;                                       \
             _mac_num_left > 0;                                               \
             _mac_num_left -= _mac_num_write)                                 \
        {                                                                     \
            if (midRCX->flags.waiting_for_FIFO_low_water_mark)                \
            {                                                                 \
                midRCX->flags.sleeping_for_FIFO = 1 ;                         \
                                                                              \
                if ( MID_DDF->num_graphics_processes != 0 )                   \
            	{                                                             \
                    if (midRCX->flags.domain_guarded_and_current)             \
                    {                                                         \
                        (*pGD->devHead.pCom->rcm_callback->unguard_domain)    \
                                (midRCX-> pRcx-> pDomain) ;                   \
                    }                                                         \
                                                                              \
                    while (midRCX->flags.sleeping_for_FIFO)                   \
                        e_sleep (&(midRCX->context_sleep_event_word), EVENT_SHORT);   \
                    if (midRCX->flags.domain_guarded_and_current)             \
                    {                                                         \
                        (*pGD->devHead.pCom->rcm_callback->make_cur_and_guard_dom)    \
                                    (midRCX-> pRcx) ;                         \
                    }                                                         \
            	}                                                             \
            	else                                                          \
            	{                                                             \
                    i_enable (_mac_old_int) ;                                 \
                    while (midRCX->flags.sleeping_for_FIFO)                   \
                    {                                                         \
                    }                                                         \
                    _mac_old_int = i_disable (INTMAX) ;                       \
            	}                                                             \
            }                                                                 \
                                                                              \
            if (_mac_num_left < MID_BIM_MAX_STORE_MULT)                       \
                    _mac_num_write = _mac_num_left ;                          \
            else                                                              \
                    _mac_num_write = MID_BIM_MAX_STORE_MULT ;                 \
                                                                              \
            MID_DD_IO_TRACE( FIFO, _mac_ptr, _mac_num_write << 2 );           \
            memcpydd( _mac_fifo_data, _mac_ptr, _mac_num_write << 2 );        \
            _mac_ptr += _mac_num_write;                                       \
        }                                                                     \
        i_enable (_mac_old_int) ;                                             \
                                                                              \
        MID_WR_TRACE_FIFO( pData, num_words )                                 \
    }                                                                         \
}

#elif MID_TED

/*---------------------------------------------------------------------------*/
/* This macro is used to poll the FIFO Free Space Register.  It will poll    */
/* until the free space exceeds by the MID_FIFO_FREE_SPACE_SLACK either the  */
/* number of number of words left to send or the MID_PIO_BLOCK_TRANSFER_SIZE.*/
/* The slack is required because there is a finite time following each       */
/* before the free space is updated.                                         */
/*                                                                           */
/* The input parameter 'num_left' specifies the number of words left to      */
/* send.  The amount of free space available is returned in the output       */
/* parameter 'free_space'.                                                   */
/*                                                                           */
/* A trace entry is generated only if sufficient free space is not initially */
/* available.                                                                */
/*---------------------------------------------------------------------------*/

#define MID_FIFO_FREE_SPACE_SLACK        8
#define MID_PIO_BLOCK_TRANSFER_SIZE     16

#define MID_WAIT_FIFO_FREE( free_space, num_left )                            \
{                                                                             \
        _MID_RD_VALUE( MID_PTR_FREE_SPACE, free_space )                       \
        if ( (free_space < (MID_PIO_BLOCK_TRANSFER_SIZE                       \
                         +  MID_FIFO_FREE_SPACE_SLACK)) &&                    \
             (free_space < ((num_left)                                        \
                         +  MID_FIFO_FREE_SPACE_SLACK)) )                     \
        {                                                                     \
                ulong   _mac_min_space;         /* Minumum space required */  \
                MID_FS_TRACE( )                                               \
                _mac_min_space = ((num_left) < MID_PIO_BLOCK_TRANSFER_SIZE)   \
                                ? (num_left) : MID_PIO_BLOCK_TRANSFER_SIZE;   \
                _mac_min_space += MID_FIFO_FREE_SPACE_SLACK;                  \
                _MID_POLL_VALUE( MID_PTR_FREE_SPACE,                          \
                        free_space, MID_GE_TEST,                              \
                        _mac_min_space )                                      \
        }                                                                     \
        free_space -= MID_FIFO_FREE_SPACE_SLACK;                              \
}

/*---------------------------------------------------------------------------*/
/* These macros are used to write words to the FIFO.  The input parameter    */
/* 'pData' is the array of words to be written and the input parameter       */
/* 'num_words' is the length of the array in words.                          */
/*                                                                           */
/* These macros check for sufficient Free Space before writing to the        */
/* adapter in order to avoid overrunning the FIFO.                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* This macro will write any number of words into the adapter FIFO           */
/*---------------------------------------------------------------------------*/

#define MID_WR_FIFO( pData, num_words )                                       \
{                                                                             \
        ulong           _mac_free_space;                                      \
        ulong           _mac_num_left = (num_words);                          \
        ulong *         _mac_ptr = ( ulong * ) (pData);                       \
        ulong           _mac_num_write;                                       \
                                                                              \
        while (_mac_num_left > 0)                                             \
        {                                                                     \
                MID_WAIT_FIFO_FREE( _mac_free_space, _mac_num_left )          \
                _mac_num_write = (_mac_num_left < _mac_free_space)            \
                                ? _mac_num_left : _mac_free_space;            \
                if (_mac_num_write <= 16)                                     \
                {                                                             \
                        memcpydd( MID_PTR_FIFO_DATA, _mac_ptr,                \
                                _mac_num_write << 2 );                        \
                }                                                             \
                else                                                          \
                {                                                             \
                        memcpyw( MID_PTR_FIFO_DATA, _mac_ptr,                 \
                                _mac_num_write );                             \
                }                                                             \
                _mac_ptr           += _mac_num_write;                         \
                _mac_num_left      -= _mac_num_write;                         \
        }                                                                     \
        MID_WR_TRACE_FIFO( pData, num_words )                                 \
}

#define MID_WR_FIFO_1( Data )                                                 \
{                                                                             \
        MID_WAIT_FIFO_FREE( _mac_free_space, 1 )                              \
        _MID_WR_VALUE( MID_PTR_FIFO_DATA, Data )                              \       \
        MID_WR_TRACE_FIFO_1( Data )                                           \
}

#else /* not MID_RMS and not MID_CDD and not MID_DD and not MID_TED */

/*---------------------------------------------------------------------------*/
/* These macros are used to write words to the FIFO.  The input parameter    */
/* 'pData' is the array of words to be written and the input parameter       */
/* 'num_words' is the length of the array in words.                          */
/*                                                                           */
/* These macros write to the FIFO without checking for Free Space.  In       */
/* order to avoid overrunning the FIFO, the process using these macros must  */
/* be put to sleep when the high water mark is reached and awakened when     */
/* the low water mark is reached.                                            */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* This macro will write any number of words into the adapter FIFO           */
/*---------------------------------------------------------------------------*/
#define MID_WR_FIFO( pData, num_words )                                       \
{                                                                             \
        if (num_words <= 16)                                                  \
        {                                                                     \
                memcpydd( MID_PTR_FIFO_DATA, pData, num_words << 2 );         \
        }                                                                     \
        else                                                                  \
        {                                                                     \
                memcpyw( MID_PTR_FIFO_DATA, pData, num_words );               \
        }                                                                     \
        MID_WR_TRACE_FIFO( pData, num_words )                                 \
}

#define MID_WR_FIFO_1( Data )                                                 \
{                                                                             \
        _MID_WR_VALUE( MID_PTR_FIFO_DATA, Data )                              \       \
        MID_WR_TRACE_FIFO_1( Data )                                           \
}

#endif /* not MID_RMS and not MID_CDD and not MID_DD and not MID_TED */


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                       Macros Which Write to the PCB                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define DEFINE_PCB_REGISTER_ADDRESSES                                         \
        volatile ulong *_pcb_data;                                            \
        volatile ulong *_ind_control;                                         \
        volatile ulong *_ind_address;                                         \
        volatile ulong *_ind_data;                                            \
                                                                              \
        _pcb_data = MID_PTR_PCB_DATA;                                         \
        _ind_control = _pcb_data +                                            \
                ((MID_PTR_IND_CONTROL - MID_PTR_PCB_DATA) >> 2);              \
        _ind_address = _pcb_data +                                            \
                ((MID_PTR_IND_ADDRESS - MID_PTR_PCB_DATA) >> 2);              \
        _ind_data    = _pcb_data +                                            \
                ((MID_PTR_IND_DATA    - MID_PTR_PCB_DATA) >> 2);


/*---------------------------------------------------------------------------*/
/* This macro waits for the required amount of space (up to 8 words) in the  */
/* Priority Command buffer.  It determines the amount of space available     */
/* by putting the BIM in wrap mode and reading the Priority Status           */
/* Register via the Indirect Data port.  A trace entry is generated only     */
/* if the PCB is not initially empty.                                        */
/*                                                                           */
/* To guarantee that this polling method will work, the caller must          */
/* guarantee that it will not be interrupted by another process which        */
/* alters the values in the Indirect Control and/or Indirect Address         */
/* registers before returning.                                               */
/*                                                                           */
/* RESTRICTIONS:  It is the user's responsiblity NOT to request more than    */
/*                8 words at a time.  Otherwise the macro will never return. */
/*---------------------------------------------------------------------------*/

#define PCB_WORDS_AVAILABLE(psr_value)                                        \
        ((psr_value == MID_PSR_PCB_EMPTY) ? 8 : 8 - psr_value)

#ifdef MID_DD
#define _MID_WAIT_PCB_SPACE( pcb_words_required )                             \
{                                                                             \
        ulong   _mac_time = 5000000;                                          \
        ulong   _mac_psr_value;                                               \
        DEFINE_POLL_COUNT( 0 )                                                \
        DEFINE_INDIRECT_REGISTER_ADDRESSES                                    \
                                                                              \
        _MID_WR_VALUE( _ind_control, MID_PSR_MODE)                            \
        _MID_WR_VALUE( _ind_address, MID_PSR_OFFSET)                          \
        _MID_RD_VALUE( _ind_data, _mac_psr_value )                            \
        while ((pcb_words_required > PCB_WORDS_AVAILABLE(_mac_psr_value)) &&  \
                 _mac_time--)                                                 \
        {                                                                     \
                mid_delay(1);                                                 \
                _MID_RD_VALUE( _ind_data, _mac_psr_value )                    \
                INCREMENT_POLL_COUNT                                          \
        }                                                                     \
        MID_PC_TRACE(MID_ADR_IND_DATA)                                        \
        if (! _mac_time)                                                      \
        {                                                                     \
                if (! (ddf->hwconfig & MID_BAD_UCODE) )                       \
                {                                                             \
                        char msgbuf[sizeof(__FILE__)+6];                      \
                        ddf->hwconfig |= MID_BAD_UCODE;                       \
                        sprintf(msgbuf, "%s,%d",__FILE__,__LINE__);           \
                        miderr(ddf,NULL,"mid-level","POLL_IND","TIMEOUT",     \
                                NULL, NULL, msgbuf);                          \
                }                                                             \
        }                                                                     \
}
#else /* not MID_DD */
#define _MID_WAIT_PCB_SPACE( pcb_words_required )                             \
{                                                                             \
        ulong   _mac_psr_value;                                               \
        DEFINE_POLL_COUNT( -1 )                                               \
        DEFINE_INDIRECT_REGISTER_ADDRESSES                                    \
                                                                              \
        _MID_WR_VALUE( _ind_control, MID_PSR_MODE)                            \
        _MID_WR_VALUE( _ind_address, MID_PSR_OFFSET)                          \
        do                                                                    \
        {                                                                     \
                _MID_RD_VALUE( _ind_data, _mac_psr_value )                    \
                INCREMENT_POLL_COUNT                                          \
        } while (pcb_words_required > PCB_WORDS_AVAILABLE(_mac_psr_value));   \
        MID_PC_TRACE(MID_ADR_IND_DATA)                                        \
}
#endif /* not MID_DD */

#define _MID_CHECK_PCB_SPACE( pcb_words_available )                           \
{                                                                             \
        DEFINE_INDIRECT_REGISTER_ADDRESSES                                    \
        _MID_WR_VALUE( _ind_control, MID_PSR_MODE)                            \
        _MID_WR_VALUE( _ind_address, MID_PSR_OFFSET)                          \
        _MID_RD_VALUE( _ind_data, pcb_words_available )                       \
}


/*---------------------------------------------------------------------------*/
/* These macros will do a structure to structure copy of the data from the   */
/* array specified by 'pData' into the PCB specified by 'pPCB'.  The         */
/* size of the array is determined by the size of the structure specified    */
/* by 'pTYPE'.  The typecasting causes the structure to be declared volatile,*/
/* which makes the xlc compiler recognize a store string operation.          */
/*                                                                           */
/* The number of words being transferred is specified by 'num_write'.        */
/* The first macro must be used if num_write is from 1 to 8 and the second   */
/* macro must be used if num_write is from 9 to 16.  At this time, no PCB    */
/* commands are greater than 16 words long.                                  */
/*---------------------------------------------------------------------------*/

#define _MID_WR_PCB_DATA( pData, num_bytes )                                  \
        memcpydd(_pcb_data, pData, num_bytes);


#define _MID_WR_PCB_1_8( pData, nbytes, num_write )                           \
{                                                                             \
        DEFINE_PCB_REGISTER_ADDRESSES                                         \
        PROTECT_PCB_ACCESS                                                    \
        _MID_WAIT_PCB_SPACE( num_write )                                      \
        _MID_WR_PCB_DATA( pData, nbytes)                                      \
        MID_WR_TRACE_PCB( pData, num_write )                                  \
        UNPROTECT_PCB_ACCESS                                                  \
}

#define _MID_WR_PCB_9_16( pData, nbytes, num_write )                          \
{                                                                             \
        DEFINE_PCB_REGISTER_ADDRESSES                                         \
        PROTECT_PCB_ACCESS                                                    \
        _MID_WAIT_PCB_SPACE( 8 )                                              \
        _MID_WR_PCB_DATA( pData, 32 )                                         \
        MID_WR_TRACE_PCB( pData, 8 )                                          \
                                                                              \
        _MID_WAIT_PCB_SPACE( num_write )                                      \
        _MID_WR_PCB_DATA( ((ulong *) pData) + 8, nbytes )                     \
        MID_WR_TRACE_PCB( ((ulong *) pData) + 8, num_write )                  \
        UNPROTECT_PCB_ACCESS                                                  \
}

/*---------------------------------------------------------------------------*/
/* This macro will initialize the PCB_FIFO_EMPTY bit in the host status      */
/* register after an adapter reset.  It is important that it NOT poll for    */
/* the PCB to be empty.                                                      */
/*---------------------------------------------------------------------------*/
#define MID_WR_PCB_INIT( )                                                    \
{                                                                             \
        ulong   _mac_nop = 0x00040000 | OP_NO_OPERATION;   /* NOP command */  \
        volatile ulong *_pcb_data = MID_PTR_PCB_DATA;                         \
        _MID_WR_PCB_DATA( &_mac_nop, 4 )                                      \
        MID_WR_TRACE_PCB( &_mac_nop, 1 )                                      \
}

/*---------------------------------------------------------------------------*/
/* These macros are used to write words to the PCB.  The input parameter     */
/* 'pData' is the array of words to be written.  The next 16 macros are of   */
/* the form MID_WR_PCB_X where X represents the number of words to send.     */
/* If then number of words to send is not a constant, is not easily          */
/* determined, or is greater than 16, the routine MID_WR_PCB should be       */
/* used.  MID_WR_PCB will work for any length of PCB command, but is not     */
/* as efficient as the macros which write a fixed number of words.           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* This macro will write 1 words into the adapter PCB                        */
/*---------------------------------------------------------------------------*/
#define MID_WR_PCB_1( pData )                                                 \
        _MID_WR_PCB_1_8( pData, 4, 1 )

/*---------------------------------------------------------------------------*/
/* This macro will write 2 words into the adapter PCB                        */
/*---------------------------------------------------------------------------*/
#define MID_WR_PCB_2( pData )                                                 \
        _MID_WR_PCB_1_8( pData, 8, 2 )

/*---------------------------------------------------------------------------*/
/* This macro will write 3 words into the adapter PCB                        */
/*---------------------------------------------------------------------------*/
#define MID_WR_PCB_3( pData )                                                 \
        _MID_WR_PCB_1_8( pData, 12, 3 )

/*---------------------------------------------------------------------------*/
/* This macro will write 4 words into the adapter PCB                        */
/*---------------------------------------------------------------------------*/
#define MID_WR_PCB_4( pData )                                                 \
        _MID_WR_PCB_1_8( pData, 16, 4 )

/*---------------------------------------------------------------------------*/
/* This macro will write 5 words into the adapter PCB                        */
/*---------------------------------------------------------------------------*/
#define MID_WR_PCB_5( pData )                                                 \
        _MID_WR_PCB_1_8( pData, 20, 5 )

/*---------------------------------------------------------------------------*/
/* This macro will write 6 words into the adapter PCB                        */
/*---------------------------------------------------------------------------*/
#define MID_WR_PCB_6( pData )                                                 \
        _MID_WR_PCB_1_8( pData, 24, 6 )

/*---------------------------------------------------------------------------*/
/* This macro will write 7 words into the adapter PCB                        */
/*---------------------------------------------------------------------------*/
#define MID_WR_PCB_7( pData )                                                 \
        _MID_WR_PCB_1_8( pData, 28, 7 )

/*---------------------------------------------------------------------------*/
/* This macro will write 8 words into the adapter PCB                        */
/*---------------------------------------------------------------------------*/
#define MID_WR_PCB_8( pData )                                                 \
        _MID_WR_PCB_1_8( pData, 32, 8 )

/*---------------------------------------------------------------------------*/
/* This macro will write 9 words into the adapter PCB                        */
/*---------------------------------------------------------------------------*/
#define MID_WR_PCB_9( pData )                                                 \
        _MID_WR_PCB_9_16( pData, 4, 1 )

/*---------------------------------------------------------------------------*/
/* This macro will write 10 words into the adapter PCB                       */
/*---------------------------------------------------------------------------*/
#define MID_WR_PCB_10( pData )                                                \
        _MID_WR_PCB_9_16( pData, 8, 2 )

/*---------------------------------------------------------------------------*/
/* This macro will write 11 words into the adapter PCB                       */
/*---------------------------------------------------------------------------*/
#define MID_WR_PCB_11( pData )                                                \
        _MID_WR_PCB_9_16( pData, 12, 3 )

/*---------------------------------------------------------------------------*/
/* This macro will write 12 words into the adapter PCB                       */
/*---------------------------------------------------------------------------*/
#define MID_WR_PCB_12( pData )                                                \
        _MID_WR_PCB_9_16( pData, 16, 4 )

/*---------------------------------------------------------------------------*/
/* This macro will write 13 words into the adapter PCB                       */
/*---------------------------------------------------------------------------*/
#define MID_WR_PCB_13( pData )                                                \
        _MID_WR_PCB_9_16( pData, 20, 5 )

/*---------------------------------------------------------------------------*/
/* This macro will write 14 words into the adapter PCB                       */
/*---------------------------------------------------------------------------*/
#define MID_WR_PCB_14( pData )                                                \
        _MID_WR_PCB_9_16( pData, 24, 6 )

/*---------------------------------------------------------------------------*/
/* This macro will write 15 words into the adapter PCB                       */
/*---------------------------------------------------------------------------*/
#define MID_WR_PCB_15( pData )                                                \
        _MID_WR_PCB_9_16( pData, 28, 7 )

/*---------------------------------------------------------------------------*/
/* This macro will write 16 words into the adapter PCB                       */
/*---------------------------------------------------------------------------*/
#define MID_WR_PCB_16( pData )                                                \
        _MID_WR_PCB_9_16( pData, 32, 8 )

/*---------------------------------------------------------------------------*/
/* This macro will between 1 and 8 words to the PCB without checking to see  */
/* if there is sufficient room for them.                                     */
/*                                                                           */
/* Prerequisites:                                                            */
/*   User must ensure the PCB has enough space to hold the number of words.  */
/*   User must ensure that either they own the PCB lock or that nobody owns  */
/*     the PCB lock and interrupts are disabled.                             */
/*---------------------------------------------------------------------------*/

#define _MID_WR_PCB_1_8_NOWAIT( pData, num_words )                            \
{                                                                             \
        switch( num_words )                                                   \
        {                                                                     \
                case 1:                                                       \
                        _MID_WR_PCB_DATA( pData, 4 )                          \
                        break;                                                \
                case 2:                                                       \
                        _MID_WR_PCB_DATA( pData, 8 )                          \
                        break;                                                \
                case 3:                                                       \
                        _MID_WR_PCB_DATA( pData, 12 )                         \
                        break;                                                \
                case 4:                                                       \
                        _MID_WR_PCB_DATA( pData, 16 )                         \
                        break;                                                \
                case 5:                                                       \
                        _MID_WR_PCB_DATA( pData, 20 )                         \
                        break;                                                \
                case 6:                                                       \
                        _MID_WR_PCB_DATA( pData, 24 )                         \
                        break;                                                \
                case 7:                                                       \
                        _MID_WR_PCB_DATA( pData, 28 )                         \
                        break;                                                \
                case 8:                                                       \
                        _MID_WR_PCB_DATA( pData, 32 )                         \
                        break;                                                \
        }                                                                     \
        MID_WR_TRACE_PCB( pData, num_words );                                 \
        MID_DD_IO_TRACE( PCB, pData, num_words << 2 );                        \
}

#ifdef MID_INTR

#ifdef MID_DD

/*---------------------------------------------------------------------------*/
/* This macro will write any number of words into the adapter PCB.  If the   */
/* PCB command won't fit in the buffer or if PCB access is protected, then   */
/* the PCB commmand will be put on a stack and the macro will return.        */
/* The stacked commands will be unstacked by a kernel process.               */
/*                                                                           */
/* The MID_INTR is used to distiguish a module that cannot use a locking     */
/* technique to gain access to the PCB, but must, instead, use the stacking  */
/* mechanism.  This is certainly true for any code in the interrupt handling */
/* path.                                                                     */
/*                                                                           */
/* In addition, we use the MID_INTR define in paths which, for any other     */
/* design reason, it is not desireable to wait for the PCB lock to be        */
/* granted.  A path falling in this category is the font path.  Since, the   */
/* font code (primarily) runs under a device independent kernel process, we  */
/* do not want to make all devices wait for the I/O path of a single Ped to  */
/* become available.                                                         */
/*                                                                           */
/* Prerequisites:                                                            */
/*   Interrupts must be disabled to ensure serialization to the stack.       */
/*   MID_FDL_PTR must be defined if using fast domain locks for lock access. */
/*   MID_DDF must be defined to point to the PED device dependent structure. */
/*---------------------------------------------------------------------------*/

#define MID_WR_PCB( pData, num_words )                                        \
{                                                                             \
        int     _mac_num_available;                                           \
                                                                              \
        DEFINE_PCB_REGISTER_ADDRESSES                                         \
        if (MID_PCB_STACK_IS_EMPTY &&                                         \
            PCB_ACCESS_IS_NOT_LOCKED &&                                       \
            num_words <= 8)                                                   \
        {                                                                     \
                _MID_CHECK_PCB_SPACE ( _mac_num_available )                   \
                if (_mac_num_available >= num_words)                          \
                {                                                             \
                        _MID_WR_PCB_1_8_NOWAIT( pData, num_words )            \
                }                                                             \
                else                                                          \
                {                                                             \
                        mid_pcb_stack( MID_DDF, pData, num_words);            \
                }                                                             \
        }                                                                     \
        else                                                                  \
        {                                                                     \
                mid_pcb_stack( MID_DDF, pData, num_words);                    \
        }                                                                     \
}

#endif /* MID_DD */

#else /* not MID_INTR */

/*---------------------------------------------------------------------------*/
/* This macro will write any number of words into the adapter PCB.           */
/*                                                                           */
/* Prerequisites:                                                            */
/*   MID_FDL_PTR must be defined if using fast domain locks for lock access. */
/*---------------------------------------------------------------------------*/

#define MID_WR_PCB( pData, num_words )                                        \
{                                                                             \
        int             _mac_num_8;                                           \
        int             _mac_residue;                                         \
        ulong *         _mac_ptr = ( ulong * ) (pData);                       \
                                                                              \
        DEFINE_PCB_REGISTER_ADDRESSES                                         \
        PROTECT_PCB_ACCESS                                                    \
        _mac_num_8 = (num_words) >> 3;                                        \
        while (_mac_num_8 > 0)                                                \
        {                                                                     \
                _MID_WAIT_PCB_SPACE( 8 )                                      \
                _MID_WR_PCB_DATA( _mac_ptr, 32 );                             \
                MID_WR_TRACE_PCB( _mac_ptr, 8 );                              \
                MID_DD_IO_TRACE( PCB, _mac_ptr, 8 << 2 );                     \
                _mac_ptr   += 8;                                              \
                _mac_num_8 -= 8;                                              \
        }                                                                     \
                                                                              \
        _mac_residue = (num_words) & 7;                                       \
                                                                              \
        if (_mac_residue)                                                     \
        {                                                                     \
                _MID_WAIT_PCB_SPACE( _mac_residue )                           \
                _MID_WR_PCB_1_8_NOWAIT( _mac_ptr, _mac_residue)               \
                                                                              \
        }                                                                     \
        UNPROTECT_PCB_ACCESS                                                  \
}

#endif /* not MID_INTR */

#endif /* _H_MID_HW_MACROS */
