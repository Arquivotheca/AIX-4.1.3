/* @(#)23       1.4  src/bos/kernext/dlc/trl/trlextcb.h, sysxdlct, bos411, 9428A410j 1/20/94 17:54:05 */
/*
 * COMPONENT_NAME: SYSXDLCT
 *
 * FUNCTIONS: trlextcb.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp.  1987, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _h_TRLEXTCB
#define _h_TRLEXTCB

#include <sys/types.h>

/*
**      These fields make up the token ring DLC specific parameters for
**      the ioctl call with the DLC_START_LS option
*/

struct   trl_start_psd
{
     uchar_t     pkt_prty;    /* ring access packet priority          */
     uchar_t     dyna_wnd;    /* dynamic window increment             */
     ushort_t    reserved;    /* currently not used                   */
};

/*
**      These fields make up the token ring DLC specific parameters for
**      the ioctl call with the DLC_ALTER option
*/

#define TRL_ALTER_PRTY 0x80000000  /* alter the packet priority       */
#define TRL_ALTER_DYNA 0x40000000  /* alter the dynamic window incr.  */

struct   trl_alter_psd
{
     ulong_t     flags;       /* specific alter flags                 */
     uchar_t     pkt_prty;    /* ring access packet priority value    */
     uchar_t     dyna_wnd;    /* dynamic window increment value       */
     ushort_t    reserved;    /* currently not used                   */
};

#define TRL_ROUTING_LEN    18 /* max. # of bytes for routing information */
#define TRL_ROUTING_OFFSET 14 /* offset to the start of the routing info */

#endif /*  _H_TRLEXTCB  */
