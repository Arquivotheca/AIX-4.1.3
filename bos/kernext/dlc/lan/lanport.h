/* @(#)94       1.11.1.4  src/bos/kernext/dlc/lan/lanport.h, sysxdlcg, bos411, 9428A410j 1/20/94 17:51:25 */
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: lanport.h
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
#ifndef _h_LANPORT
#define _h_LANPORT

#if defined(TRL) || defined(FDL)
#define TRLORFDDI
#endif

#include "lanintcb.h"
#include "lansapcb.h"
#include "lanstacb.h"
#include "lantxbuf.h"
#include <sys/lanerr.h>

struct lan_sls_arg {
	struct dlc_sls_arg dlc_sls_arg;
#ifdef TRL
	struct trl_start_psd trl_start_psd;
#endif
#ifdef FDL
	struct fdl_start_psd fdl_start_psd;
#endif
};

struct   dcache {
    u_char   name[20];                 /* discovery remote name       */
    u_char   address[6];               /* discovery remote card
                                          address                     */
    u_char   len_name;                 /* length of name              */
    u_char   len_address;              /* length of address           */
};

struct   cache {
    ulong_t  n_entries;                /* number of cache entries     */
    u_char   name_index[100];          /* indices into cache data area*/
    struct dcache cache_data[100];     /* cache data area             */
};

struct cache *p_to_cache;
struct port_dcl {
	struct dlc_port dlc_port;
	int debug;
/* <<< feature CDLI >>> */
	struct ndd      *nddp;
	ulong_t         statfilter_sid;
/* <<< end feature CDLI >>> */
	int sapno;			/* current sap number         */
	ulong_t  stano;                   /* current station list index */
/* defect 115819 removed goodbye_proc */
	int rc;                         /* return code                */
	struct sap_cb *sap_ptr;
	struct station_cb *sta_ptr;
	int op;
	struct dlc_open_ext dlc_open_ext;
	struct dlc_esap_arg dlc_esap_arg;
	struct lan_sls_arg lan_sls_arg;
	struct dlc_io_ext dlc_io_ext;
	struct dlc_qsap_arg dlc_qsap_arg;
	struct dlc_alter_arg dlc_alter_buf;
	struct dlc_qls_arg dlc_qls_arg;
	int operation_result;
	int routing_info;               /* token ring flag            */
	int lpdu_length;
	struct COMMON_CB common_cb;
	struct rcv_data rcv_data;
	struct station_list station_list[MAX_SESSIONS];
	struct sap_list sap_list[128];
	struct send_buf_rsp send_data;
	struct mbuf *m;
        struct mbuf *m0;
	struct STA_CMD_BUF *sta_cmd_buf;
	struct STA_CMD_BUF *rnr_rsp_buf;
	struct STA_CMD_BUF *disc_rsp_buf;
	struct RCV_STA_TBL rcv_sta_tbl[MAX_SESSIONS];
	struct {
	  uint   status;             /* return code                */
	  ushort_t opcode;             /* function to be performed   */
	  ushort_t funa_1;             /* first bytes of functional addr  */
	  ushort_t funa_2;             /* second bytes of functional addr */
	  ushort_t func_3;             /* last bytes of functional addr */
	  } func_addr;
	  
	/* the following unions resolve the old PL/8 */
        /* based pointers */
	union l {
		char *config_ptr;
		struct dlc_sls_arg *ls_config;
		struct lan_sls_arg *lan_config;
		struct dlc_esap_arg *sap_config;
	} l;

	union d {
		char *data_ptr;
		struct rcv_data *rcv_data;
		struct send_ibuf_data *send_ibuf_data;
		struct send_data *send_data;
		struct send_buf_rsp *send_buf_rsp;
		struct send_nbuf_data *send_nbuf_data;
	} d;
	union i {
		char *i_field_ptr;
		struct i_field_frmr *i_field_frmr;
		struct ifield_add_name *ifield_add_name;
		struct ifield_add_name_rsp *ifield_add_name_rsp;
		struct ifield_find_name *ifield_find_name;
		struct ifield_name_found *ifield_name_found;
	} i;
#ifdef TRLORFDDI
	union ri {
		char *ptr_ri;
		struct ri_sd *ri_sd;
		struct ri_sbp *ri_sbp;
		struct ri_rcv *ri_rcv;
	} ri;
#endif  /* TRLORFDDI */
	union loopback {
		char *lb_ring_addr;
		struct ring_queue *loopback_rq;
	} loopback;

	char *errptr;
	char build_err_rec[300];
	struct vector_header vector_header;
	struct correlator_vector correlator_vector;
	struct mac_vector mac_vector;
	struct search_vector search_vector;
	struct nsa_vector nsa_vector;
	struct lsap_vector lsap_vector;
	struct response_vector response_vector;
	struct origin_vector origin_vector;
	struct object_vector object_vector;
/* <<< feature CDLI >>> */
/* <<< removed struct cio_get_fastwrt fastwrt_info; >>> */
/* <<< end feature CDLI >>> */
	char *ptr_corr; 	/* correlator vector          */
	char *ptr_nsa; 		/* nsa vector              */
	char *ptr_mac; 		/* mac vector              */
	char *ptr_lsap; 	/* lsap vector             */
	char *ptr_resp; 	/* response vector         */
	char *ptr_tid; 		/* target id vector        */
	char *ptr_sid; 		/* source id vector        */
	char *ptr_s_nsa; 	/* source id nsa vector    */
	char *ptr_t_nsa; 	/* target id nsa vector    */
	char *ptr_object;       /* object name vector      */
	int  target_name_len;
	char target_name[20];
	int  source_name_len;
	char source_name[20];
        ulong_t fpp_cbytes;     /* number of bytes read/written */
        struct file *fpp_cache;
        u_char *p2_to_cache;    /*address of cache         */
        u_char *p3_to_cache;    /* address of cache data   */
        u_char name_cache[26];      /* path of cache data      */

#define CACHE_SIZE  4096        /* maximum size of cache   */
#define FPP_CMODE   1666        /* mode values of cache file  */
#define SETBIT(field,b) (field |= b)
#define CLRBIT(field,b) (field &= ~(b))

};

#endif /* _h_LANPORT */
