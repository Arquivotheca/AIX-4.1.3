/* @(#)71       1.3  src/bos/kernext/dlc/fdl/fdlintcb.h, sysxdlcf, bos411, 9428A410j 1/20/94 17:40:40 */
/*
 * COMPONENT_NAME: (SYSXDLCF) FDDI Data Link Control
 *
 * FUNCTIONS: fdlintcb.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#ifndef _h_FDDIINTCB
#define _h_FDDIINTCB
/*******************************************************************/
/*  This module contains Token Ring specific definitions only.     */
/*******************************************************************/

#define BRIDGE_BUF_SIZE0 0x00  /* bridge buffer =  516       */
#define BRIDGE_BUF_SIZE1 0x01  /* bridge buffer = 1500       */
#define BRIDGE_BUF_SIZE2 0x02  /* bridge buffer = 2052       */
#define BRIDGE_BUF_SIZE3 0x03  /* bridge buffer = 4472       */
/* LEHb defect 44499 */
#define BRIDGE_BUF_SIZE4 0x04  /* bridge buffer = 8144       */
#define BRIDGE_BUF_SIZE5 0x05  /* bridge buffer = 11407      */
#define BRIDGE_BUF_SIZE6 0x06  /* bridge buffer = 17800      */
/* LEHe */

/* <<< feature CDLI >>> */
#define GROUP_ADDR_MASK 0x80     /* Group address indicator mask      */
/* <<< end feature CDLI >>> */
					/*----------------------------*/
					/* PACKET HEADER LENGTHS      */
					/*----------------------------*/
#define UN_HDR_LENGTH  19          /* unnumbered header length w/o routing   */
#define NORM_HDR_LENGTH 20        /* normal I/S header length w/o routing    */
#define MAX_PACKET     4096      /* max packet size            */

	struct ri_control_field {           /* routing control fields */
		unsigned all_route     :1;    /* all rings broadcast    */
		unsigned single_route  :1;    /* limited route broadcast*/
		unsigned unknown       :1;    /* unused                 */
		unsigned ri_lth        :5;    /* length of ri field     */
		unsigned direction     :1;    /* scan direction         */
		unsigned largest_field :3;    /* largest field          */
		unsigned fill          :4;    /* reserved               */
	};

	struct rcv_data {
				u_char reserved[3];
       		u_char phy_ctl_1;    	/* physical control byte 1    */
       		u_char laddr[6];     	/* destination address(local) */
       		u_char raddr[6];     	/* source address(remote)     */
#define RI_PRESENT 0x80     		/* routing info present       */
#define DIRECTION 0x80
       		char ri_field[FDL_ROUTING_LEN]; /* routing information, always*/
       		u_char lsap;		/* DSAP(local)                */
#define GROUP_SAP   1   		/* Group SAP indicator        */
       		u_char rsap;            	/* SSAP(remote)               */
#define RESPONSE    1   		/* response indicator         */
       		u_char ctl1;            	/* control byte #1            */
#define POLL_FINAL_1  0x10       	/* poll-final indicator       */
#define FRAME_TYPE    0x03      	/* type of frame              */
       		u_char ctl2;      	/* control byte #2            */
	};

	struct ri_sd {
       		u_char rsap;		/* DSAP(local)                */
#define GROUP_SAP   1   		/* Group SAP indicator        */
       		u_char lsap;            	/* SSAP(remote)               */
#define RESPONSE    1   		/* response indicator         */
       		u_char ctl1;            	/* control byte #1            */
#define POLL_FINAL_1  0x10       	/* poll-final indicator       */
#define FRAME_TYPE    0x03      	/* type of frame              */
       		u_char ctl2;      	/* control byte #2            */
		u_char ack_slot;
	};

	struct ri_sbp {
       		u_char rsap;		/* DSAP(local)                */
#define GROUP_SAP   1   		/* Group SAP indicator        */
       		u_char lsap;            	/* SSAP(remote)               */
#define RESPONSE    1   		/* response indicator         */
       		u_char ctl1;            	/* control byte #1            */
#define POLL_FINAL_1  0x10       	/* poll-final indicator       */
#define FRAME_TYPE    0x03      	/* type of frame              */
       		u_char ctl2;      	/* control byte #2            */
	};

	struct ri_rcv {
       		u_char lsap;		/* DSAP(local)                */
#define GROUP_SAP   1   		/* Group SAP indicator        */
       		u_char rsap;            	/* SSAP(remote)               */
#define RESPONSE    1   		/* response indicator         */
       		u_char ctl1;            	/* control byte #1            */
#define POLL_FINAL_1  0x10       	/* poll-final indicator       */
#define FRAME_TYPE    0x03      	/* type of frame              */
       		u_char ctl2;      	/* control byte #2            */
		u_char ack_slot;
	};
 
	struct send_data {
				u_char reserved[3];
       		u_char phy_ctl_1;    	/* physical control byte 1    */
       		u_char raddr[6];     	/* destination address(remote)*/
       		u_char laddr[6];     	/* source address(local)      */
#define RI_PRESENT 0x80     		/* routing info present       */
       		char ri_field[FDL_ROUTING_LEN]; /* routing information, always*/
       		u_char rsap;         	/* DSAP(remote)               */
#define GROUP_SAP       1      		/* Group SAP indicator        */
       		u_char lsap;            	/* SSAP(local)                */
#define RESPONSE        1      		/* response indicator         */
       		u_char ctl1;            	/* control byte #1            */
#define POLL_FINAL_1    0x10   		/* poll-final indicator       */
       		u_char ctl2;           	/* start of unnumbered info   */
#define POLL_FINAL_2    1      		/* poll-final indicator       */
       		u_char ack_stano;       	/* WRITE ACK STATION SLOT#    */
	};
 
	struct send_buf_rsp {
				u_char reserved[3];
       		u_char phy_ctl_1;    	/* physical control byte 1    */
       		u_char raddr[6];     	/* destination address(remote)*/
       		u_char laddr[6];     	/* source address(local)      */
#define RI_PRESENT 0x80     		/* routing info present       */
       		char ri_field[FDL_ROUTING_LEN]; /* routing information, always*/
       		u_char rsap;         	/* DSAP(remote)               */
       		u_char lsap;            	/* SSAP(local)                */
       		u_char ctl1;            	/* control byte #1            */
       		u_char ctl2;           	/* start of unnumbered info   */
       		u_char ack_stano;       	/* WRITE ACK STATION SLOT#    */
	};
 
	struct send_nbuf_data {
				u_char reserved[3];
       		u_char phy_ctl_1;    	/* physical control byte 1    */
       		u_char raddr[6];     	/* destination address(remote)*/
       		u_char laddr[6];     	/* source address(local)      */
#define RI_PRESENT 0x80     		/* routing info present       */
       		char ri_field[FDL_ROUTING_LEN]; /* routing information, always*/
       		u_char rsap;         	/* DSAP(remote)               */
       		u_char lsap;            	/* SSAP(local)                */
       		u_char ctl1;            	/* control byte #1            */
       		u_char ctl2;           	/* start of unnumbered info   */
       		u_char ack_stano;       	/* WRITE ACK STATION SLOT#    */
	};
#endif /* _h_FDDIINTCB */
