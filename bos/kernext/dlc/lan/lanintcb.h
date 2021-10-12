/* @(#)93       1.9.1.1  src/bos/kernext/dlc/lan/lanintcb.h, sysxdlcg, bos411, 9428A410j 7/20/92 19:07:07 */
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: lanintcb.h
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
#ifndef _h_LANINTCB
#define _h_LANINTCB
/**********************************************************************/
/*                                                                    */
/*     IIIIIIII  NN    NN  TTTTTTTT  EEEEEEEE  RRRRRR    NN    NN     */
/*        II     NNN   NN     TT     EE        RR    RR  NNN   NN     */
/*        II     NNNN  NN     TT     EE        RR    RR  NNNN  NN     */
/*        II     NN NN NN     TT     EEEEE     RRRRRR    NN NN NN     */
/*        II     NN  NNNN     TT     EE        RR  RR    NN  NNNN     */
/*        II     NN   NNN     TT     EE        RR   RR   NN   NNN     */
/*     IIIIIIII  NN    NN     TT     EEEEEEEE  RR    RR  NN    NN     */
/*                                                                    */
/**********************************************************************/

/*====================================================================*/
/* error log definitions */
/*====================================================================*/

#define FILEN __FILE__
#define LINEN __LINE__

#define ALERT 1
#define NON_ALERT 0

#define DET_MODULE      0x00A2  /* (00A2) Detecting Module           */
#define ADD_SUBVECS     0x8001  /* (8001) Additional Subvectors      */
#define SV_LCS_DATA       0x51  /* Link Connection Subsystem Data SV */
#define SF51_LOCAL_MAC    0x03  /* Local MAC address Subfield        */
#define SF51_REMOTE_MAC   0x04  /* Remote MAC address Subfield       */
#define SF51_ROUTING      0x04  /* Routing Information Subfield      */

#define SV_LCS_CONFIG     0x52  /* Link Connection Subsystem Data SV */
#define SF52_REMOTE_SAP   0x02  /* Remote link address/SAP Subfield  */
#define SF52_LOCAL_SAP    0x04  /* Local link address/SAP Subfield   */

#define SV_LS_DATA        0x8C  /* Link Station Data Subfield        */
#define SF8C_NS_NR        0x01  /* Current NS/NR counts Subfield     */
#define SF8C_FRAMES_OUT   0x02  /* Outstanding frame count Subfield  */
#define SF8C_CNTL_RCVD    0x03  /* Last control received Subfield    */
#define SF8C_CNTL_SENT    0x04  /* Last control sent Subfield        */
#define SF8C_MODULUS      0x05  /* Sequence number modulus Subfield  */
#define SF8C_LS_STATE     0x06  /* Link station state Subfield       */
#define SF8C_LS_LBUSY     0x80  /* Link station state = local busy   */
#define SF8C_LS_RBUSY     0x40  /* Link station state = remote busy  */
#define SF8C_REPOLLS      0x07  /* Number of repolls Subfield        */
#define SF8C_RCVD_NR      0x08  /* Last received Nr Subfield         */

#define SV_RESOURCE_LIST  0x05  /* Hierarchy/Resource List SV        */
#define SF05_NAME_LIST    0x10  /* Name List Subfield                */
#define SE0510_CMPL_IND   0xC0  /* Complete Indicator Sub Element    */
#define SE0510_FLAGS      0x00  /* Flags Sub Element                 */

#ifdef TRL
#define COMP_NAME "SYSXDLCT"
#define DLC_TYPE "Token-Ring DLC  "   /* must be 16 chars */
#endif
#ifdef FDL
#define COMP_NAME "SYSXDLCF"
#define DLC_TYPE "FDDI DLC        "   /* must be 16 chars */
#endif
#ifdef EDL
#define COMP_NAME "SYSXDLCE"
#define DLC_TYPE "Std-Ethernet DLC"   /* must be 16 chars */
#endif
#ifdef E3L
#define COMP_NAME "SYSXDLCI"
#define DLC_TYPE "IEEE-Ether DLC  "   /* must be 16 chars */
#endif
#ifdef SDL
#define COMP_NAME "SYSXDLCS"
#define DLC_TYPE "SDLC DLC        "   /* must be 16 chars */
#endif
#ifdef QLL
#define COMP_NAME "SYSXDLCQ"
#define DLC_TYPE "X.25 QLLC DLC   "   /* must be 16 chars */
#endif
/*====================================================================*/
#define TRACE_SHORT_SIZE 80       /* default link trace entry size */
#define RC_GOOD   0
#define RC_NONE  -1
#define SETBIT(field,b) (field |= b)
#define CLRBIT(field,b) (field &= ~(b))
#define MINUS_ONE -1
#define NO_MATCH -1              /* no-match indicator for scan      */
#define TRUE 1
#define FALSE 0
#define SAP_CORR_MASK 0x80000000 /* SAP correlator mask */
#define MAX_ADDN_REPOLLS 3       /* maximum add name query repolls   */
#define ADDN_RETRY_VAL 3         /* number of 500 msec ticks for add */
				 /*                     name retries */
#define T_BUSY_VAL 40            /* number of 500 msec ticks prior   */
				 /* to reseting local busy.          */
#define ABORTED 0xff9c           /* internal RC = -100 for   */
					/* user aborted SAPs        */
/*********************************************************************/
#define SOFT 0
#define HARD 1
#define FRMR_SENT 2
/**********************************************************************/
#define ECB_RCV_MASK 0x40000000
#define ECB_TIMER_MASK 0x20000000
/**********************************************************************/
#define NO_BUF_AVAIL -1          /* no buffer RC from lanfetch */

					/*----------------------------*/
					/* STATION COMMANDS           */
					/*----------------------------*/
#define WRITENO 		1
#define WRT_SINGLE_CMPLNO 	3
#define RCV_CMPLNO 		4
#define T1_TIMEOUT_CMPLNO 	5
#define T2_TIMEOUT_CMPLNO 	6
#define T3_TIMEOUT_CMPLNO 	7
#define LOW_ON_BUFFERSNO 	8
#define BUFFERS_NOW_AVAILNO 	9
#define OPENNO 			10
#define CALL_CMPLNO 		11
#define LISTEN_CMPLNO 		12
#define CONTROLNO 		13
#define CLOSENO 		14
#define QUERYNO 		15
#define LBUSY_WAKEUPNO 		16
					/*----------------------------*/
					/* STATION MISC.              */
					/*----------------------------*/
#define MAX_SESSIONS 256          /* maximum number of sessions plus 1*/
					/*----------------------------*/
					/* DISCOVERY COMMANDS         */
					/*----------------------------*/
#define ADDNO 1                   /* generate an add name query       */
#define CALLNO 2                  /* generate a call (find remote)    */
#define RCVNO 3                   /* handle the receive packet        */
/*--------------------------------------------------------------------*/
				 /* ERROR LOG ERROR TYPES      */
				 /*----------------------------*/
#define PERM_PLC_ERR 1           /* physical port shutdown (perm) */
#define PERM_SAP_ERR 2           /* SAP shutdown (permanent)      */
#define PERM_STA_ERR 3           /* link station shutdown (perm)  */
#define TEMP_ERR 4               /* temporary error               */
#define INFO_ERR 5               /* information only error        */
/*--------------------------------------------------------------------*/

					/*----------------------------*/
					/* Registered Ethernet "TYPE" */
					/*----------------------------*/
#define SNA_LLC_TYPE 0x80d5
					/*----------------------------*/
					/* DSAP/SSAP                  */
					/*----------------------------*/
#define RESP_ON 0x01             /* response "on" SSAP mask     */
#define RESP_OFF 0xfe            /* response "off" SSAP mask    */
#define GROUP_ON 0x01            /* response "on" DSAP mask     */
#define GROUP_OFF 0xfe           /* response "off" DSAP mask    */
#define DISCOVERY_SAP 0xfc
#define NULL_SAP 0x00
#define RESOLVE_SAP 0x00
					/*----------------------------*/
					/* CONTROL BYTEs              */
					/*----------------------------*/
#define RR 0x01                  /* receive ready byte-1 mask         */
#define RNR 0x05                 /* receive not ready byte-1 mask     */
#define REJ 0x09                 /* reject byte-1 mask                */
#define UI 0x13                  /* unnumbered information            */
#define DM 0x1f                  /* disconnected mode                 */
#define DISC 0x53                /* disconnect                        */
#define UA 0x73                  /* unnumbered acknowledge            */
#define SABME 0x7f               /* set ABM extended mode             */
#define FRMR 0x97                /* frame reject                      */
#define XID 0xbf                 /* exchange ID                       */
#define TEST 0xf3                /* test                              */

#define UI_NO_PF 0x03            /* unnumbered info - no pf bit       */
#define DM_NO_PF 0x0f            /* disconnected mode - no pf bit     */
#define UA_NO_PF 0x63            /* unnumbered acknowledge - no pf bit*/
#define FRMR_NO_PF 0x87          /* frame reject - no pf bit          */
#define XID_NO_PF 0xaf           /* xid - no pf bit                   */

#define PF1_MASK 0x10            /* poll final bit mask byte-1 */
#define PF1_OFF_MASK 0xef        /* poll final off mask byte-1 */
#define PF2_MASK 0x01            /* poll final bit mask byte-2 */
#define PF2_OFF_MASK 0xfe        /* poll final off mask byte-2 */

#define INFO 0x0                 /* information frame bit mask */
#define SUPERVISORY 0x1          /* supervisory frame bit mask */
#define UNNUMBERED 0x3           /* unnumbered frame bit mask  */



					/*----------------------------*/
					/* DH COMMAND MISC.           */
					/*----------------------------*/
#define MIN_PACKET 60            /* minimum packet size -CRC */


	struct rcv_ring {
	    ulong_t rcv_input;            /* Ring Queue IN  pointer     */
	    ulong_t rcv_output;           /* Ring Queue OUT pointer     */
	    ulong_t rcv_end;              /* Ring Queue END pointer     */
	    char *rcv_begin;            /* Ring Queue buffer pointers */
	}; 

#ifdef EDL
#include "edlintcb.h"
#endif

#ifdef TRL
#include "trlintcb.h"
#endif

#ifdef FDL
#include "fdlintcb.h"
#endif

#ifdef E3L
#include "e3lintcb.h"
#endif

	struct i_field_frmr {
	    u_char frmr_ctl1;         /* invalid control byte-1 rcvd   */
	    u_char frmr_ctl2;         /* invalid control byte-2 rcvd   */
	    u_char frmr_vs;           /* current send state variable   */
	    u_char frmr_vr;           /* current receive state variable*/
#define FRMR_RESPONSE 1              /* invalid rcvd cmd/rsp indicator*/
	    u_char frmr_reason;       /* reason code for rejection     */

#define FRMR_INV_CTL_RCVD 	0x01
#define FRMR_INV_IFIELD_RCVD 	0x03
#define FRMR_IFIELD_OFLO 	0x04
#define FRMR_INV_SEQ_NUM 	0x08
#define FRMR_PF_ON 		0x10
#define FRMR_PF_OFF 		0x00
	};


	struct vector_header {
	    ushort_t length;              /* vector length              */
	    ushort_t key;                 /* vector key                 */
	};
#define SIZE_VECTOR_HEADER 4

	struct correlator_vector {
	    ushort_t length;              /* vector length              */
	    ushort_t key;                 /* vector key                 */
	    ulong_t value;                /* correlator value           */
	};
#define SIZE_CORRELATOR_VECTOR 8

	struct mac_vector {
	    ushort_t length;              /* vector length              */
	    ushort_t key;                 /* vector key                 */
	    char value[6];              /* MAC address                */
	};
#define SIZE_MAC_VECTOR 10

	struct search_vector {
	    ushort_t length;              /* vector length              */
	    ushort_t key;                 /* vector key                 */
	};
#define SIZE_SEARCH_VECTOR 4

	struct nsa_vector {
	    ushort_t length;              /* vector length              */
	    ushort_t key;                 /* vector key                 */
	    ushort_t value;               /* type of name supported     */
	};
#define SIZE_NSA_VECTOR 6

	struct lsap_vector {
	    ushort_t length;              /* vector length              */
	    ushort_t key;                 /* vector key                 */
	    u_char value;                /* lsap value                 */
	};
#define SIZE_LSAP_VECTOR 5

	struct response_vector {
	    ushort_t length;              /* vector length              */
	    ushort_t key;                 /* vector key                 */
	    u_char value;                /* response value             */
	};
#define SIZE_RESPONSE_VECTOR 5

	struct origin_vector {
	    ushort_t length;              /* vector length              */
	    ushort_t key;                 /* vector key                 */
	};
#define SIZE_ORIGIN_VECTOR 4

	struct object_vector {
	    ushort_t length;              /* vector length              */
	    ushort_t key;                 /* vector key                 */
	    char value[20];             /* name                       */
	};
#define SIZE_OBJECT_VECTOR 4

/** CONSTANT VECTOR VALUES *******************************************/
#define FIND_VECTOR_KEY 0x0001 /* find vector              */
#define FOUND_VECTOR_KEY 0x0002 /* found vector             */
#define CORRELATOR_KEY 0x4003/* correlator vector        */
#define SEARCH_ID_KEY 0x0004 /* search ID vector         */
#define ORIGIN_ID_KEY 0x000d /* origin ID vector         */
#define OBJECT_KEY 0x4010    /* object name vector       */
#define MAC_KEY 0x4006       /* designated MAC vector    */
#define LSAP_KEY 0x4007      /* designated LSAP vector   */
#define NSA_KEY 0x4011       /* NSA ID vector            */
#define RESP_KEY 0x400b      /* response vector          */

#define SNA_ID 0x0001        /* SNA NSA ID               */
#define RESOURCE_AVAIL 0x01  /* resource available       */
#define MASK_MAJOR_VECTOR 0xf000 /* vector key mask          */
#define MAJOR_VECTOR 0x0000  /* major vector             */
#define LOCAL_NSA_ID 0x8000  /* locally admin. NSA ID    */

#define CORRELATOR 0x00000001/* correlator present          */
#define MAC 0x00000002       /* mac present                 */
#define LLSAP 0x00000004     /* lsap present                */
#define RESP 0x00000008      /* resp present                */
#define SEARCH 0x00000010    /* search id present           */
#define ORIGIN 0x00000020    /* origin id present           */
#define ORIGIN_NAME 0x00000040 /* object name present         */
#define SEARCH_NAME 0x00000080 /* search name present         */
#define SEARCH_NSA 0x00000100/* search NSA present          */
#define ORIGIN_NSA 0x00000200/* origin NSA present          */

#define MAJOR_VECTOR_MODE 1  /* seaching major vector     */
#define MINOR_VECTOR_MODE 0  /* seaching minor vector     */

#endif /* _h_LANINTCB */
