/* @(#)95       1.8  src/bos/kernext/dlc/lan/lansapcb.h, sysxdlcg, bos411, 9428A410j 5/28/93 14:14:06 */
/*
 * COMPONENT_NAME: SYSXDLCG
 *
 * FUNCTIONS:  lansapcb.h
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
#ifndef _h_LANSAPCB
#define _h_LANSAPCB
/**********************************************************************/
/* station control block declaration */
/**********************************************************************/

struct  sap_cb {
	/*----------------------------*/
        /* sap configuration          */
        /*----------------------------*/
       	struct dlc_esap_arg sap_profile; /* from externals            */
       	ulong_t	lname_length_8;         /* length of local name       */
       	char	lname_8[8];             /* local name field           */

	/*----------------------------*/
        /* call handling              */
        /*----------------------------*/
       	ulong_t	conflict_user_corr;     /* conflicting user correlator*/

	/*----------------------------*/
        /* listen handling            */
        /*----------------------------*/
	struct station_cb *loop_sta_ptr;
       	ulong_t	loop_stano;           /* sta# for pending listen    */
       	ulong_t	listen_stano;           /* sta# for pending listen    */
       	ulong_t	listen_rname_length;    /* remote name length         */
       	char	listen_rname[20];       /* remote name                */
       	char	listen_raddr[6];        /* remote physical address    */
       	u_char	incomming_rsap;         /* incomming remote sap value */
       	u_char	listen_lsap;            /* local sap value            */

	/*----------------------------*/
        /* misc.                      */
        /*----------------------------*/
       	ulong_t	user_sap_path;          /* user's sap path            */
	struct dlc_chan *user_sap_channel;   /* sap channel ID        */
       	ulong_t	num_sta_opened;         /* number of stations opened  */
       	ulong_t	rcv_ringq_addr;         /* user's receive ringq addr  */
	int     sap_retcode;            /* sap closing reason code    */
	u_char  sap_limbo;              /* sap limbo mode - can't wrt */
/* LEHb defect 43788 */
	struct mbuf *retry_rcvn_buf;    /* store busy network data ptr */
	struct dlc_io_ext retry_rcvn_ext; /* store retry netd extension */
/* LEHe */

	/*----------------------------*/
        /* sap state                  */
        /*-----------rcv valid--------*/
   	u_char	sap_state;      	/* physical link state        */
#define SAP_RCV_VALID 0x80         /* bit 8 valid rcv state indicator */

/* defect 82006 */
        /* multicast address list definitions    */
        struct linktype {
          struct linktype    *next;     /* next group address in list */
          struct linktype    *previous; /* previous group address in list */
          char               grpaddr[6]; /* group address                 */
        } linkentry;

        struct linklist {
          struct linktype    *head;      /* first address in list     */
          struct linktype    *tail;      /* last address in list      */
          struct linktype    *clp;       /* current address pointer   */
          int                listlength; /* number of addresses in list */
          int                itemlength; /* length of each item in list */
        } mc_addr;


/* end defect 82006 */


};
 
                                        /*-----------rcv not valid----*/
#define   SAP_CLOSE_STATE    0x00       /* sap closed         */
#define   SAP_CLOSE_PEND     0x01       /* sap closed         */
#define   SAP_ABORTED        0x02       /* sap aborted        */
                                        /*-----------rcv valid--------*/
#define   ADDING_NAME        0xFF       /* adding sap name to */
                                        /*        the network */
#define   SAP_OPEN_STATE     0xFE       /* sap opened         */
#endif /* _h_LANSAPCB */
