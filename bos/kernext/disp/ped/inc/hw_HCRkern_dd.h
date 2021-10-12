/* @(#)57	1.3.1.2  src/bos/kernext/disp/ped/inc/hw_HCRkern_dd.h, peddd, bos411, 9428A410j 3/19/93 18:52:29 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
 ***************************************************************
 *
 *  PEDERNALES MACRO PROGRAMMING INTERFACE
 *
 ***************************************************************
 */

#ifndef	_H_HW_HCRKERN
#define	_H_HW_HCRKERN


/* ***************************************************************
   ***************************************************************
     OPCODES FOR COMMANDS INTO THE HOST COMMO REGISTER
   ***************************************************************
   *************************************************************** */

/*  #define	DownloadComplete		0x9001  NO LONGER USED */
#define MID_HCR_SWITCH_CONTEXT		0x9002

#define MID_SC_UPDATES_REQUIRED         0
#define MID_SC_NO_UPDATES_REQUIRED      0x00010000
#define MID_SC_DRAIN_FIFO_TO_WID_CORR	0x00020000
#define MID_SC_WAITING_FOR_LOW_WATER	0x00040000



/* ***************************************************************
   ***************************************************************
      MACROS FOR PROGRAMMING TO THE HOST COMMO REGISTER
   ***************************************************************
   *************************************************************** */



/*-------------------------------------------------------------------------* 

   Set Context structure and macro                       

   This macro: 
       . builds the Context Switch Command Element data, 
       . writes that data to the CCB (Context Command Block),
       . then writes to the Host Commo Register to cause the switch.
       . Finally, it reads the "FIFO available" bit in adapter status.
	  This bit is not valid until the "Context Switch in process" bit 
	  is set so this macro polls (up to 20 microseconds) for CSIP. 
          Then the status is read again because the macros just aren't
          smart enough to pass the status value from one macro to another. 

 --------------------------------------------------------------------------*/

typedef struct MID_SetContext_SE
{

  ushort   old_fate;      /* Old context fate:                    */
#	define    MID_NO_OLD_CTX                  0
#	define    MID_SAVE_OLD_CTX_ON_ADAPTER     1
#	define    MID_REMOVE_OLD_CTX_MAX_LENGTH   2
#	define    MID_REMOVE_OLD_CTX_COMPUTE_LEN  3
#	define    MID_TERMINATE_OLD_CTX           4


  ushort   new_fate;      /* New context fate:                    */
#	define    MID_NO_NEW_CTX                  0
#	define    MID_INITIATE_NEW_CTX            1
#	define    MID_SWITCH_TO_NEW_CTX           2
#	define    MID_RESTORE_NEW_CTX             3


  ulong    old_id ;       /* Old context ID (remove only) */
  ulong    old_adr;       /* Old context mem address(remove only) */
  ulong    old_len;       /* Old context mem length (remove only) */


  ulong    new_id;        /* New context id                       */
  ulong    new_type;      /* New context type:                    */
                                      /*   0 = 2d                             */
                                      /*   1 = 3dm1                           */
                                      /*   3 = 3dm2                           */
				/*  The context types are defined in mid.h. */

  ulong    new_adr;       /* New context mem address(restore only)*/
  ulong    new_len;       /* New context mem length (restore only)*/
  ushort   high_water;    /* High water interrupt point           */
  ushort   low_water;     /* Low water interrupt point            */


} MIDSetContext_SE;




/*----------------------------------------------------------------------------
   Set Context macro 
 *---------------------------------------------------------------------------*/

#define MID_SetContext_SE( ddf_param, flag,	                               \
                           oldfate_param, newfate_param,                       \
                           old_id_param, oldaddr_param, old_len_param,         \
            new_id_param,  newtype_param, newaddr_param, new_len_param,        \
                           fifo_hi_param, fifo_lo_param );                     \
{                                                                              \
  MIDSetContext_SE    SE;                                                      \
                                                                               \
     SE.old_fate   = ( ushort )  oldfate_param;                                \
     SE.new_fate   = ( ushort )  newfate_param;                                \
     SE.old_id     = ( ulong )   old_id_param;                                 \
     SE.old_adr    = ( ulong )   oldaddr_param;                                \
     SE.old_len    = ( ulong )   old_len_param;                                \
     SE.new_id     = ( ulong )   new_id_param;                                 \
     SE.new_type   = ( ulong )   newtype_param;                                \
     SE.new_adr    = ( ulong )   newaddr_param;                                \
     SE.new_len    = ( ulong )   new_len_param;                                \
     SE.high_water = ( ushort )  fifo_hi_param;                                \
     SE.low_water  = ( ushort )  fifo_lo_param;                                \
                                                                               \
     BUGLPR(dbg_SetContext, 4, ("SE is built at %8X\n",&SE));                  \
     BUGLPR(dbg_SetContext, 4, ("fates:  %8X %4X\n",SE.old_fate,SE.new_fate)); \
     BUGLPR(dbg_SetContext, 4, ("old ID:      %8X\n",SE.old_id   ));	       \
     BUGLPR(dbg_SetContext, 4, ("old addr:    %8X\n",SE.old_adr  ));	       \
     BUGLPR(dbg_SetContext, 4, ("old length   %8X\n",SE.old_len  ));	       \
     BUGLPR(dbg_SetContext, 4, ("new ID:      %8X\n",SE.new_id   ));	       \
     BUGLPR(dbg_SetContext, 4, ("new type:    %8X\n",SE.new_type ));	       \
     BUGLPR(dbg_SetContext, 4, ("new addr:    %8X\n",SE.new_adr  ));	       \
     BUGLPR(dbg_SetContext, 4, ("new length   %8X\n",SE.new_len  ));	       \
     BUGLPR(dbg_SetContext, 4, ("hi/lo%11X %4X\n",SE.high_water,SE.low_water));\
     BUGLPR(dbg_SetContext, 3, ("Writing CCB to adapter\n"));                  \
     MID_WR_CCB_ARRAY( 0, &SE,  (sizeof(SE) >> 2));                            \
                                                                               \
     /* Get the switch started */                                              \
                                                                               \
     MID_DD_IO_TRACE (HCR, &(SE), sizeof(SE) ) ;			\
     MID_WR_HOST_COMO( MID_HCR_SWITCH_CONTEXT | flag );			       \
     BUGLPR(dbg_SetContext, 2, ("Writing Host como\n"));                       \
                                                                               \
}    




#endif /* _H_HW_HCRKERN */

