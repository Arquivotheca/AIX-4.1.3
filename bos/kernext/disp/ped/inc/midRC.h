/* static char sccsid[] = "@(#)29	1.4.1.3  src/bos/kernext/disp/ped/inc/midRC.h, peddd, bos411, 9428A410j 3/19/93 18:52:57"; */
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


#ifndef _H_MIDRC  
#define _H_MIDRC   


/*----------------------------------------------------------------------------*
    General request codes 
 *----------------------------------------------------------------------------*/
#define MID_NULL_REQUEST    0 

/*----------------------------------------------------------------------------*
    General return codes 
 *----------------------------------------------------------------------------*/
#define MID_RC_OK          0     
#define MID_ERROR         -1     

/*----------------------------------------------------------------------------*
    Return codes from create_win_geom
 *----------------------------------------------------------------------------*/
#define MID_PARM_ERROR     MID_ERROR
#define MID_NO_MEMORY      MID_ERROR


/*----------------------------------------------------------------------------*
    Request codes for get_WID and get_unused_WID
 *----------------------------------------------------------------------------*/
#define MID_GET_UNSHARED_WID    1     
#define MID_GET_WID_FOR_RENDER  MID_GET_UNSHARED_WID         
#define MID_GET_GUARDED_WID     2
#define MID_GET_WID_FOR_SWAP    MID_GET_GUARDED_WID      
#define MID_GET_WID_FOR_PI      3      
#define MID_GET_WID_FOR_SWITCH  4      

/*----------------------------------------------------------------------------*
    Return codes from get_WID
 *----------------------------------------------------------------------------*/
#define MID_OLD_WID        0     
#define MID_NEW_WID        1     

/*----------------------------------------------------------------------------*
    Return codes from get_unused_WID
 *----------------------------------------------------------------------------*/
#define MID_WID_ASSIGNED      MID_RC_OK
#define MID_WID_NOT_AVAILABLE MID_ERROR




/*----------------------------------------------------------------------------*
    Return codes from start_switch   These should go in rcm.h some day.     
 *----------------------------------------------------------------------------*/
#define MID_LIGHT_SWITCH   1     
#define MID_HEAVY_SWITCH   0     
#define MID_PREVIOUS_NOT_DONE   2     



#endif /* _H_MIDRC */
