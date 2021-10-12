/* @(#)63       1.2.1.1  src/bos/kernext/dlc/lan/lantxbuf.h, sysxdlcg, bos411, 9428A410j 7/20/92 19:04:03 */
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS:  lantxbuf.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#ifndef _h_LANTXBUF
#define _h_LANTXBUF

#if defined(TRL) || defined(FDL)
#define TRLORFDDI
#endif

/**********************************************************************/
/* link station special transmit buffers  */
/**********************************************************************/
#ifdef EDL
/***************************************
*     Standard Ethernet Version        *
***************************************/
struct STA_CMD_BUF {
       	u_char 	raddr[6];		/* destination address(remote)*/
       	u_char	laddr[6];		/* source address(local)      */
       	ushort_t 	llc_type;		/* llc type field             */
       	ushort_t	lpdu_length;		/* length of the lpdu         */
       	u_char	lpad;			/* leading pad                */
       	u_char 	rsap;			/* dsap(remote)               */
       	u_char 	lsap;			/* ssap(local)                */
       	u_char 	ctl1;			/* control byte #1            */
       	u_char	ctl2;           	/* control byte #2            */
        unsigned  slot1 :24;      	/* reserved		      */
       	ulong_t	ack_slot;       	/* write ack station slot#    */
};
#endif

#ifdef E3L
/***************************************
*     802.3 Ethernet Version           *
***************************************/
struct STA_CMD_BUF {
       	u_char 	raddr[6];		/* destination address(remote)*/
       	u_char	laddr[6];		/* source address(local)      */
       	ushort_t	lpdu_length;		/* length of the lpdu         */
       	u_char 	rsap;			/* dsap(remote)               */
       	u_char 	lsap;			/* ssap(local)                */
       	u_char 	ctl1;			/* control byte #1            */
       	u_char	ctl2;           	/* control byte #2            */
        unsigned  slot1 :24;      	/* reserved		      */
       	ulong_t	ack_slot;       	/* write ack station slot#    */
};
#endif

#ifdef TRL
/********************************
*     Token Ring Version        *
********************************/
struct STA_CMD_BUF {
			u_char phy_ctl_1;   	 	/* physical control byte 1    */
       	u_char phy_ctl_2;    		/* physical control byte 2    */
       	u_char raddr[6];     		/* destination address(remote)*/
       	u_char laddr[6];     		/* source address(local)      */
#define RI_PRESENT 0x80     		/* routing info present       */
       	char ri_field[18];    		/* routing information, always*/
       	u_char 	rsap;			/* dsap(remote)               */
       	u_char 	lsap;			/* ssap(local)                */
       	u_char 	ctl1;			/* control byte #1            */
       	u_char	ctl2;           	/* control byte #2            */
       	ulong_t	ack_slot;      		/* write ack station slot#    */
};
#endif /* TRL */
#ifdef FDL
/********************************
*           FDDI Version        *
********************************/
struct STA_CMD_BUF {
			u_char reserved[3];
			u_char phy_ctl_1;   	 	/* physical control byte 1    */
       	u_char raddr[6];     	/* destination address(remote)*/
       	u_char laddr[6];     	/* source address(local)      */
#define RI_PRESENT 0x80     		/* routing info present       */
       	char ri_field[FDL_ROUTING_LEN]; 	/* routing information, always*/
       	u_char 	rsap;				/* dsap(remote)               */
       	u_char 	lsap;				/* ssap(local)                */
       	u_char 	ctl1;				/* control byte #1            */
       	u_char	ctl2;         	/* control byte #2            */
       	ulong_t	ack_slot;     	/* write ack station slot#    */
};
#endif /* FDDI */
#endif /* _h_LANTXBUF */
