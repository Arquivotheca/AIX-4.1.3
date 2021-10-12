/* @(#)69       1.1  src/bos/kernext/dlc/fdl/fdlextcb.h, sysxdlcf, bos411, 9428A410j 7/20/92 18:50:29 */
/*
 * COMPONENT_NAME: (SYSXDLCF) FDDI Data Link Control
 *
 * FUNCTIONS: fdlextcb.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _h_FDLEXTCB
#define _h_FDLEXTCB

#include <sys/types.h>

/*
**      These fields make up the token ring DLC specific parameters for
**      the ioctl call with the DLC_START_LS option
*/

struct   fdl_start_psd
{
     uchar_t     pkt_prty;    /* ring access packet priority          */
     uchar_t     dyna_wnd;    /* dynamic window increment             */
     ushort_t    reserved;    /* currently not used                   */
};

/*
**      These fields make up the token ring DLC specific parameters for
**      the ioctl call with the DLC_ALTER option
*/

#define FDL_ALTER_PRTY 0x80000000  /* alter the packet priority       */
#define FDL_ALTER_DYNA 0x40000000  /* alter the dynamic window incr.  */

struct   fdl_alter_psd
{
     ulong_t     flags;       /* specific alter flags                 */
     uchar_t     pkt_prty;    /* ring access packet priority value    */
     uchar_t     dyna_wnd;    /* dynamic window increment value       */
     ushort_t    reserved;    /* currently not used                   */
};

#define FDL_LSAP_OFFSET		47 /* offset of the local sap in the packet */
#define FDL_ROUTING_LEN    30 /* max. # of bytes for routing information */
#define FDL_ROUTING_OFFSET 16 /* offset to the start of the routing info */

#endif  _H_FDLEXTCB
