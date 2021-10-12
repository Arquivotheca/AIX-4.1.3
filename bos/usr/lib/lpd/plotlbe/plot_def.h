/* @(#)31	1.1  src/bos/usr/lib/lpd/plotlbe/plot_def.h, cmdplot, bos411, 9428A410j 4/16/91 18:11:23 */
/*
 * COMPONENT_NAME: (CMDPLOT) ported RT plotter code
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/***********************************************************************/
/*     plot_def.h                                                      */
/*                                                                     */
/* (c) Copyright 1985, IBM Corporation                                 */
/*                                                                     */
/* NAME:        plot_def.h                                             */
/*                                                                     */
/* DATE:  09/03/85                                                     */
/*                                                                     */
/* PURPOSE:     This file defines message codes user by plotter backend*/
/***********************************************************************/
 
#define VIEW        1                   /* device is in view  mode    */
#define NOPAPER     2                   /* device needs paper         */
#define OPEN_PORT   3                   /* open port error            */
#define BAD_OPTION  4                   /* bad print cmd option       */
#define BAD_FR      5                   /* bad frame option           */
#define ORG_ERR     6                   /* error setting origin       */
#define OPEN_FILE   7                   /* open file error            */
#define WR_PORT     8                   /* error writing to port      */
#define BAD_ACK     9                   /* unrecognized ack           */
#define RD_FILE     10                  /* error reading file         */
#define REMOVE_PAPER 11                 /* remove paper from device   */
#define RS232_ERROR  12                 /* RS232 error                */
#define CLOSE        13                 /* pen cover open             */
#define CLOSE_VIEW   14                 /* pen cover open - view mode */
#define NOPAPER_CLOSE 15                /* nopaper and pen cover open */
#define BAD_STATUS    16                /* unrecognized device status */
#define RD_PORT       17                /* error reading from port    */
#define TIME_OUT      18                /* time out message           */
#define NO_RESPONSE   19                /* no response from device    */
#define CANCEL        20                /* job canceled               */
#define ENA_QERR      21                /* enable queue error         */
#define BAD_SPEED     22                /* speed operand is invalid   */
#define CONTINUE      23                /* continue plotting          */
#define STAT_ERR      24                /* file status error          */
 
