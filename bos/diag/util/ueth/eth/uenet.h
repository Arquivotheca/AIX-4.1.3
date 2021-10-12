/* @(#)37	1.1  src/bos/diag/util/ueth/eth/uenet.h, dsaueth, bos411, 9428A410j 12/6/93 08:21:12 */
/*
 *   COMPONENT_NAME: DSAUETH
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define DEVNAMESIZE 4			/* size of device name entx */

#define CAT_NAME "uenet.cat"

/* connector types to be chosen from menu */
#define DIX 1
#define BNC 2

/* menu numbers */
#define DESCRIP_NUM 0x802050
#define ACTION_NUM 0x802051
#define WC_NUM 0x802052
#define STANDBY_NUM 0x802053
#define RESULTS_NUM 0x802054

#define CONFIG_ERR 0x99

/***** riser card return code *****/

#define TWISTED_PAIR_RISER_CARD    0x8880
#define THICK_RISER_CARD           0x8881
#define THIN_RISER_CARD            0x8882
