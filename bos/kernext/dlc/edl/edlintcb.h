/* @(#)74       1.6  src/bos/kernext/dlc/edl/edlintcb.h, sysxdlce, bos411, 9428A410j 1/20/94 17:39:44 */
/*
 * COMPONENT_NAME: (SYSXDLCE)  Standard Ethernet Data Link Control
 *
 * FUNCTIONS: edlintcb.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#ifndef _h_EDLINTCB
#define _h_EDLINTCB
/*******************************************************************/
/*  This module contains Standard Ethernet specific defines only.  */
/*******************************************************************/

/* <<< feature CDLI >>> */
#define GROUP_ADDR_MASK 0x01     /* Group address indicator mask      */
/* <<< end feature CDLI >>> */

					/*----------------------------*/
					/* PACKET HEADER LENGTHS      */
					/*----------------------------*/
#define UN_HDR_LENGTH 20         /* unnumbered header length   */
#define NORM_HDR_LENGTH 21       /* normal I/S header length   */
#define MAX_PACKET     1514      /*  max packet size, -CRC bytes */
	struct rcv_data {
	    char laddr[6];          /* destination address(local) */
	    char raddr[6];          /* source address(remote)     */
	    ushort_t llc_type;        /* LLC type field             */
	    ushort_t lpdu_length;     /* length of the LPDU         */
	    u_char lpad;             /* leading pad                */
	    u_char lsap;             /* DSAP(local)                */
	    u_char rsap;             /* SSAP(remote)               */
	    u_char ctl1;             /* control byte #1            */
	    u_char ctl2;             /* control byte #2            */
	    char normal_info;      /* start of normal information*/
	};


	struct send_ibuf_data {
	    ulong_t w1;                   /* 1st word of packet header  */
	    ulong_t w2;                   /* 2nd word of packet header  */
	    ulong_t w3;                   /* 3rd word of packet header  */
	    ulong_t w4;                   /* 4th word of packet header  */
	    ulong_t w5;                   /* 5th word of packet header  */
	    ushort_t hw6;                 /* #6- h/word of packet header*/
	    u_char b7;                   /* #7- byte of packet header  */
	    char normal_info;           /* start of normal information*/
	};


	struct send_data {
	    char raddr[6];              /* destination address(remote)*/
	    char laddr[6];              /* source address(local)      */
	    ushort_t llc_type;            /* LLC type field             */
	    ushort_t lpdu_length;         /* length of the LPDU         */
	    u_char lpad;                 /* leading pad                */
	    u_char rsap;                 /* DSAP(remote)               */
#define  GROUP_SAP 1			/* Group SAP indicator        */
	    u_char lsap;             	/* SSAP(local)                */
#define  RESPONSE 1 			/* response indicator         */
	    u_char ctl1;                 /* control byte #1            */
#define  POLL_FINAL_1 0x10 		/* poll-final indicator       */
	    u_char ctl2;                 /* control byte #2            */
#define  POLL_FINAL_2 1 		/* poll-final indicator       */
	    unsigned  :24;              /* reserved                   */
	    ulong_t ack_stano;            /* write ack station station# */
	}; 


	struct send_buf_rsp {
	    char raddr[6];          	/* destination address(remote)*/
	    char laddr[6];          	/* source address(local)      */
	    ushort_t llc_type;            /* LLC type field             */
	    ushort_t lpdu_length;         /* length of the LPDU         */
	    u_char lpad;                 /* leading pad                */
	    u_char rsap;             	/* DSAP(remote)               */
	    u_char lsap;             	/* SSAP(local)                */
	    u_char ctl1;             	/* control byte #1            */
	    u_char ctl2;     		/* start of unnumbered info   */
	}; 


	struct send_nbuf_data {
	    char raddr[6];          	/* destination address(remote)*/
	    char laddr[6];          	/* source address(local)      */
	    ushort_t llc_type;            /* LLC type field             */
	    ushort_t lpdu_length;         /* length of the LPDU         */
	    u_char lpad;                 /* leading pad                */
	    u_char rsap;             	/* DSAP(remote)               */
	    u_char lsap;             	/* SSAP(local)                */
	    u_char ctl1;             	/* control byte #1            */
	    u_char ctl2;             	/* control byte #2            */
	    unsigned  :24;              /* reserved                   */
	    ulong_t ack_stano;            /* write ack station station# */
	}; 
#endif /* _h_EDLINTCB */
