/* @(#)76       1.9.1.6  src/bos/kernext/dlc/lan/lancomcb.h, sysxdlcg, bos411, 9430C411a 7/22/94 14:28:50 */
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS:  lancomcb.h
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
#ifndef _h_LANCOMCB
#define _h_LANCOMCB
#if defined(TRL) || defined(FDL)
#define TRLORFDDI
#endif
/**********************************************************************/
/* common control block declaration  */
/**********************************************************************/

/**********************************************************************/
/*  IBM CONFIDENTIAL                                                  */
/*  Copyright International Business Machines Corp. 1985, 1988        */
/*  Unpublished Work                                                  */
/*  All Rights Reserved                                               */
/*  Licensed Material -- Program Property of IBM                      */
/*                                                                    */
/*  Use, Duplication or Disclosure by the Government is subject to    */
/*  restrictions as set forth in paragraph (b)(3)(B) of the Rights in */
/*  Technical Data and Computer Software clause in DAR 7-104.9(a).    */
/**********************************************************************/
#define  TRACE1(x, a)		dlc_trace(x, 1,a)
#define  TRACE2(x, a,b)    	dlc_trace(x, 2,a,b)
#define  TRACE3(x, a,b,c)  	dlc_trace(x, 3,a,b,c)
#define  TRACE4(x, a,b,c,d)	dlc_trace(x, 4,a,b,c,d)
#define  M_TRC_ENA 	0xBADFADFF	/* monitor trace enabled      */

struct COMMON_CB {
        /*----------------------------*/
        /* monitor trace              */
        /*----------------------------*/
       	ulong_t 	ena_word;       	/* monitor trace enable word  */
#define TMAX		400 
	ulong_t		*tptr;
        ulong_t 	*top;
	ulong_t		*tend;
        ulong_t		trace[TMAX]; 

        /*----------------------------*/
        /* physical link parms        */
        /*----------------------------*/
	int		plc_retcode;            /* plc return code            */
	int		sap_retcode;            /* sap failure indicator      */
       	ushort_t 	last_cmd_dh;            /* last command to the dh     */
       	u_char		plc_state;      	/* physical link state        */
#define  PLC_CLOSED		0x00    /* physical link reset        */
#define  OPENING_DEVICE 	0x01    /* open dev issued to the BIOM*/
#define  STARTING_DEVICE 	0x02    /* start dev issued to the DH */
#define  PLC_CLOSING            0x03    /* closing the physical link  */
#define  PLC_OPENED		0xFE    /* physical link open         */
 
	u_char resolve_started;         /* resolve sap started indicator */
	u_char discovery_started;       /* discovery sap started indicator */
        u_char type_80d5_started;       /* type field 80d5 started indicator */

      	/*----------------------------*/
      	/* rcv hashing table stuff    */
      	/*----------------------------*/
      	ulong_t 	hashno;                 /* current rcv sta list index */
	union {
      		char 	hash_string[8]; /* hashing string             */
       		struct {
         	char 	hash_string_raddr[6];   /* remote address     */
         	u_char 	hash_string_lsap;       /* local sap          */
         	u_char 	hash_string_rsap;       /* remote sap         */
		} s_h;
	} u_h; 

        /*----------------------------*/
        /* ADD NAME COUNTER           */
        /*----------------------------*/
        ulong_t   addn_ctr;               /* add name timer enable ctr  */

	/*----------------------------*/
	/* misc.                      */
	/*----------------------------*/
	ulong_t         lbusy_ctr;              /* netd local busy counter*/
  	ulong_t		rc;		        /* general purpose return code*/
       	ushort_t 	maxif;                  /* maximum i-field receivable */
       	ulong_t 	vector_status;          /* discovery vector status    */
       	ulong_t 	nsa_count;              /* discovery nsa count        */
       	ulong_t 	object_count;           /* discovery object count     */
	uchar_t		local_addr[6];          /* local address              */

#ifdef TRLORFDDI
	struct {                            /* routing control fields */
		unsigned all_route     :1;    /* all rings broadcast    */
		unsigned single_route  :1;    /* limited route broadcast*/
		unsigned unknown       :1;    /* unused                 */
		unsigned ri_lth        :5;    /* length of ri field     */
		unsigned direction     :1;    /* scan direction         */
		unsigned largest_field :3;    /* largest field          */
		unsigned fill          :4;    /* reserved               */
#ifdef TRL
		u_char   routes[16];     /* fills out the 18 bytes of RI */
#elif FDL
					 /* fills out the 30 bytes of RI */
		u_char   routes[FDL_ROUTING_LEN-2];
#endif
	} ri_field;

        ulong_t   ri_length;              /* current length of ri field */
#endif  /* TRLORFDDI */
					      /* LM/X RIPL addresses      */
	u_char   ripl_grp_addr[6];            /* Group Address            */

#ifdef TRLORFDDI
					      /* FDDI group address and   */
#define RIPL_GRP_ADDR_MASK_0  0xC0            /* Token functional addr    */
#define RIPL_GRP_ADDR_MASK_1  0x00            /* value = 0xC000_4000_0000 */
#define RIPL_GRP_ADDR_MASK_2  0x40
#define RIPL_GRP_ADDR_MASK_3  0x00
#define RIPL_GRP_ADDR_MASK_4  0x00
#define RIPL_GRP_ADDR_MASK_5  0x00
#endif  /* TRLORFDDI */

#ifndef TRLORFDDI
#define RIPL_GRP_ADDR_MASK_0   0x03           /* Ether or 8023 multicast  */
#define RIPL_GRP_ADDR_MASK_1   0x00           /* value = 0x0300_0200_0000 */
#define RIPL_GRP_ADDR_MASK_2   0x02
#define RIPL_GRP_ADDR_MASK_3   0x00
#define RIPL_GRP_ADDR_MASK_4   0x00
#define RIPL_GRP_ADDR_MASK_5   0x00
#endif /* not TRLORFDDI */

};
#endif /* _h_LANCOMCB */
