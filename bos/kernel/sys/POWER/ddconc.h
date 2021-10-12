/* @(#)08       1.4  src/bos/kernel/sys/POWER/ddconc.h, sysxdisk, bos411, 9428A410j 9/12/93 18:04:21 */
#ifndef _H_DDCONC
#define _H_DDCONC
/*
 * COMPONENT_NAME: (SYSXDISK) Serial Dasd Subsytem Device Driver
 *
 * FUNCTIONS:  Header File for concurrent mode
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Ioctl Operation Defines */

#define DD_CONC_REGISTER        0x80    /* Register for concurrent mode      */ 
#define DD_CONC_UNREGISTER      0x81    /* Unregister for concurrent mode    */ 

/* Concurrent mode commands */

#define DD_CONC_SEND_REFRESH    0x01    /* Issue a refresh to notify other   */
                                        /* hosts that data has been  updated */
                                        /* on the device.                    */
#define DD_CONC_LOCK            0x02    /* Issue a lock to prevent other     */
                                        /* hosts from modifying data         */
#define DD_CONC_UNLOCK          0x03    /* Issue an unlock to allow other    */
                                        /* hosts to modify data              */
#define DD_CONC_TEST            0x04    /* Issue a test device command to    */
                                        /* verify that the device is still   */
                                        /* accessable to this host.          */
#define DD_CONC_RECV_REFRESH    0x05    /* A refresh command has been        */
                                        /* received from another host.       */
#define DD_CONC_RESET           0x06    /* A device  has been reset and any  */
                                        /* pending messages have beem flshed */

/* 
 * Registration structure 
 */

struct dd_conc_register 
{
    int (*conc_func_addr)(struct conc_cmd *);    /* Concurrent mode entry     */
                                                 /* point address             */
    int (*conc_intr_addr)(struct conc_cmd *,
			  uchar,uchar,dev_t);    /* Concurrent mode Function  */
                                                 /* address for interrupts    */
};

/* 
 * Concurrent mode command structure
 */

struct conc_cmd {
    struct conc_cmd   *prev;                    /* Previous one in list      */
    struct conc_cmd   *next;                    /* Next one in list          */
    uchar             error;                    /* Error after I/O           */
    uchar             cmd_op;                   /* Command operation         */
    uchar             message;                  /* Message code              */
    dev_t             devno;                    /* Major + minor             */
    int               work;                     /* Work area                 */
    int               resvd;                    /* Reserved for future       */
};

#endif
