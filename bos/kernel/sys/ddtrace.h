/* @(#)61	1.11  src/bos/kernel/sys/ddtrace.h, systrace, bos411, 9428A410j 6/16/90 00:26:03 */

/*
 * COMPONENT_NAME:            include/sys/ddtrace.h
 *
 * FUNCTIONS: header file for device driver trace hookwords
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_DDTRACE
#define _H_DDTRACE

#define DDHKWD1(hkid,loc,errno,data) (TRCHKLT( (hkid|(loc<<8)|errno), data ))
#define DDHKWD2(hkid,loc,errno,data1,data2) (TRCHKL2T( (hkid|(loc<<8)|errno), data1, data2 ))
#define DDHKWD3(hkid,loc,errno,data1,data2,data3) (TRCHKL3T( (hkid|(loc<<8)|errno), data1, data2, data3 ))
#define DDHKWD4(hkid,loc,errno,data1,data2,data3,data4) (TRCHKL4T( (hkid|(loc<<8)|errno), data1, data2, data3, data4 ))
#define DDHKWD5(hkid,loc,errno,data1,data2,data3,data4,data5) (TRCHKGT( (hkid|(loc<<8)|errno), data1,data2,data3,data4,data5 ))
#define DDHKGEN(hkid,devno,devln,devnm) (TRCGENKT(0,hkid,devno,devln,devnm))

/* locations 0x00 - 0x6f (0-111) reserved, rest avail to device driver */
#define DD_ENTRY_OPEN		0x01
#define DD_EXIT_OPEN		0x02
#define DD_ENTRY_CLOSE		0x03
#define DD_EXIT_CLOSE		0x04
#define DD_ENTRY_READ		0x05
#define DD_EXIT_READ		0x06
#define DD_ENTRY_WRITE		0x07
#define DD_EXIT_WRITE		0x08
#define DD_ENTRY_IOCTL		0x09
#define DD_EXIT_IOCTL		0x0a
#define DD_ENTRY_SELECT		0x0b
#define DD_EXIT_SELECT		0x0c
#define DD_ENTRY_CONFIG		0x0d
#define DD_EXIT_CONFIG		0x0e
#define DD_ENTRY_STRATEGY	0x0f
#define DD_EXIT_STRATEGY	0x10
#define DD_ENTRY_MPX		0x11
#define DD_EXIT_MPX		0x12
#define DD_ENTRY_REVOKE		0x13
#define DD_EXIT_REVOKE		0x14
#define DD_ENTRY_INTR		0x15
#define DD_EXIT_INTR		0x16
#define DD_ENTRY_BSTART		0x17	/* block devices */
#define DD_EXIT_BSTART		0x18
#define DD_ENTRY_CSTART		0x19	/* character devices */
#define DD_EXIT_CSTART		0x1a
#define DD_ENTRY_IODONE 	0x1b
#define DD_EXIT_IODONE 		0x1c
#define DD_SC_INTR 		0x1d	/* special performance trace, block device */
#define DD_COALESCE 		0x1e	/* special performance trace, scsi disk */
#define DD_SC_IODONE 		0x1f	/* special performance trace, scsi disk */
#define DD_PERF_HOOK 		0x20    /* performance trace, c327dd */
#define DD_C327_HOOK 		0x21    /* generic trace point, c327dd */
#define DD_CONFIG_NAME 		0x40	/* device name trace */

#define	DD_ENTRY_HALT		0x70
#define	DD_EXIT_HALT		0x71
#define	DD_ENTRY_GET_STAT	0x72
#define	DD_EXIT_GET_STAT	0x73
#define DD_EXIT_KREAD		0x74
#define DD_EXIT_KSTAT		0x75
#define DD_EXIT_KTX_FN		0x76	
#define DD_ENTRY_CHGPARM        0x77
#define DD_EXIT_CHGPARM         0x78
#define DD_ENTRY_STARTAR        0x79
#define DD_EXIT_STARTAR         0x7a
#define DD_ENTRY_FLUSHPORT      0x7b
#define DD_EXIT_FLUSHPORT       0x7c
#define DD_ENTRY_ADAPT_QUERY    0x7d
#define DD_EXIT_ADAPT_QUERY     0x7e
#define DD_ENTRY_QUERY_STAT     0x7f
#define DD_EXIT_QUERY_STAT      0x80
#define DD_ENTRY_STOP_AR        0x81
#define DD_EXIT_STOP_AR         0x82
#define DD_ENTRY_TRACEON        0x83
#define DD_EXIT_TRACEON         0x84
#define DD_ENTRY_STOP_PORT      0x85
#define DD_EXIT_STOP_PORT       0x86
#define DD_ENTRY_TRACEOFF       0x87
#define DD_EXIT_TRACEOFF        0x88
#define DD_ENTRY_IOCINFO        0x89
#define DD_EXIT_IOCINFO         0x8a
#define DD_ENTRY_START          0x8b
#define DD_EXIT_START           0x8c
#define DD_ENTRY_QUERY          0x8d
#define DD_EXIT_QUERY           0x8e
#define DD_ENTRY_REJECT_CALL    0x8f
#define DD_EXIT_REJECT_CALL     0x90
#define DD_ENTRY_QUERY_SESSION  0x91
#define DD_EXIT_QUERY_SESSION   0x92
#define DD_ENTRY_DEL_RID        0x93
#define DD_EXIT_DEL_RID         0x94
#define DD_ENTRY_QUERY_RID      0x95
#define DD_EXIT_QUERY_RID       0x96
#define DD_ENTRY_LINK_CON       0x97
#define DD_EXIT_LINK_CON        0x98
#define DD_ENTRY_LINK_DIS       0x99
#define DD_EXIT_LINK_DIS        0x9a
#define DD_ENTRY_LINK_STAT      0x9b
#define DD_EXIT_LINK_STAT       0x9c
#define DD_ENTRY_LOCAL_BUSY     0x9d
#define DD_EXIT_LOCAL_BUSY      0x9e
#define DD_ENTRY_COUNTER_GET    0x9f
#define DD_EXIT_COUNTER_GET     0xa0
#define DD_ENTRY_COUNTER_WAIT   0xa1
#define DD_EXIT_COUNTER_WAIT    0xa2
#define DD_ENTRY_COUNTER_READ   0xa3
#define DD_EXIT_COUNTER_READ    0xa4
#define DD_ENTRY_COUNTER_REM    0xa5
#define DD_EXIT_COUNTER_REM     0xa6
#define DD_ENTRY_DIAG_IO        0xa7
#define DD_EXIT_DIAG_IO         0xa8
#define DD_ENTRY_DIAG_MEM       0xa9
#define DD_EXIT_DIAG_MEM        0xaa
#define DD_EXIT_DIAG_CARD       0xab
#define DD_ENTRY_DIAG_CARD      0xac
#define DD_ENTRY_RESET          0xad
#define DD_EXIT_RESET           0xae
#define DD_ENTRY_DIAG_TASK      0xaf
#define DD_EXIT_DIAG_TASK       0xb0
#define DD_ENTRY_UCODE_TASK     0xb1
#define DD_EXIT_UCODE_TASK      0xb2
#define DD_ENTRY_ADD_RID        0xb3
#define DD_EXIT_ADD_RID         0xb4
#define DD_PERF_TX_DATA_QUEUE   0xb5
#define DD_PERF_TX_DATA_ASSIGN  0xb6
#define DD_PERF_TX_DATA_COMP    0xb7
#define DD_PERF_PACKET_RECEIVE  0xb8
#define DD_PERF_RX_DATA_QUEUE   0xb9
#define DD_PERF_RX_DATA_DELIVER 0xba
#define DD_PERF_RX_DATA_KERNEL  0xbb
#define DD_PERF_RX_DATA_QFULL   0xbc
#define DD_ENTRY_INTR_STAT      0xbd
#define DD_EXIT_INTR_STAT       0xbe
#endif  /* _H_DDTRACE */
