#ifndef rpc__v0_included
#define rpc__v0_included
#include "idl_base.h"
#include "nbase.h"
#include "ncastat.h"
#define RPC_IDL_SUPPORTS_V1 ndr__true
#define rpc__unbound_port socket__unspec_port
#define rpc__mod 469827584
#define rpc__comm_failure nca_status__comm_failure
#define rpc__op_rng_error nca_status__op_rng_error
#define rpc__unk_if nca_status__unk_if
#define rpc__cant_create_sock 469827588
#define rpc__cant_bind_sock 469827589
#define rpc__wrong_boot_time nca_status__wrong_boot_time
#define rpc__too_many_ifs 469827591
#define rpc__not_in_call 469827592
#define rpc__you_crashed nca_status__you_crashed
#define rpc__no_port 469827594
#define rpc__proto_error nca_status__proto_error
#define rpc__too_many_sockets 469827596
#define rpc__illegal_register 469827597
#define rpc__cant_recv 469827598
#define rpc__bad_pkt 469827599
#define rpc__unbound_handle 469827600
#define rpc__addr_in_use 469827601
#define rpc__in_args_too_big 469827602
#define rpc__out_args_too_big nca_status__out_args_too_big
#define rpc__server_too_busy nca_status__server_too_busy
#define rpc__string_too_long nca_status__string_too_long
#define rpc__too_many_objects 469827606
#define rpc__unsupported_type nca_status__unsupported_type
#define rpc__not_authenticated 469827608
#define rpc__invalid_auth_type 469827609
#define rpc__cant_malloc 469827610
#define rpc__cant_nmalloc 469827611
#define rpc__invalid_handle 469827612
typedef ndr__ulong_int rpc__sar_opts_t;
#define rpc__brdcst 1
#define rpc__idempotent 2
#define rpc__maybe 4
#define rpc__drep_int_big_endian 0
#define rpc__drep_int_little_endian 1
#define rpc__drep_float_ieee 0
#define rpc__drep_float_vax 1
#define rpc__drep_float_cray 2
#define rpc__drep_float_ibm 3
#define rpc__drep_char_ascii 0
#define rpc__drep_char_ebcdic 1
typedef ndr__short_float *rpc__short_float_p_t;
typedef ndr__long_float *rpc__long_float_p_t;
typedef rpc__short_float_p_t rpc__short_float_p;
typedef rpc__long_float_p_t rpc__long_float_p;
typedef ndr__char *rpc__char_p_t;
typedef ndr__byte *rpc__byte_p_t;
#define rpc__max_alignment 8
#define rpc__mispacked_hdr 0
#define rpc__max_pkt_size 1024
#define rpc__max_pkt_size_8 128
typedef struct NIDL_tag_1c88 rpc__ppkt_t;
struct NIDL_tag_1c88 {
ndr__long_float d[128];
};
typedef rpc__ppkt_t *rpc__ppkt_p_t;
typedef void (*rpc__server_stub_t)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */rpc__ppkt_p_t ins,
  /* [in] */ndr__ulong_int ilen,
  /* [in] */rpc__ppkt_p_t outs,
  /* [in] */ndr__ulong_int omax,
  /* [in] */rpc__drep_t drep,
  /* [out] */rpc__ppkt_p_t *routs,
  /* [out] */ndr__ulong_int *olen,
  /* [out] */ndr__boolean *must_free,
  /* [out] */status__t *st)
#else
()
#endif
;
typedef rpc__server_stub_t *rpc__epv_t;
typedef void (*rpc__mgr_proc_t)
#ifdef __STDC__
 (
  /* [in] */handle_t h)
#else
()
#endif
;
typedef rpc__mgr_proc_t *rpc__mgr_epv_t;
typedef void (*rpc__generic_server_stub_t)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */rpc__ppkt_p_t ins,
  /* [in] */ndr__ulong_int ilen,
  /* [in] */rpc__ppkt_p_t outs,
  /* [in] */ndr__ulong_int omax,
  /* [in] */rpc__drep_t drep,
  /* [in] */rpc__mgr_epv_t epv,
  /* [out] */rpc__ppkt_p_t *routs,
  /* [out] */ndr__ulong_int *olen,
  /* [out] */ndr__boolean *must_free,
  /* [out] */status__t *st)
#else
()
#endif
;
typedef rpc__generic_server_stub_t *rpc__generic_epv_t;
typedef struct NIDL_tag_97d rpc__if_spec_t;
struct NIDL_tag_97d {
ndr__ulong_int vers;
ndr__ushort_int port[32];
ndr__ushort_int opcnt;
uuid__t id;
};
typedef ndr__boolean (*rpc__shut_check_fn_t)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status__t *st)
#else
()
#endif
;
typedef void (*rpc__auth_log_fn_t)
#ifdef __STDC__
 (
  /* [in] */status__t st,
  /* [in] */socket__addr_t *addr,
  /* [in] */ndr__ulong_int addrlen)
#else
()
#endif
;
typedef ndr__char *rpc__cksum_t;
typedef ndr__char rpc__string_t[256];
extern  void rpc__use_family
#ifdef __STDC__
 (
  /* [in] */ndr__ulong_int family,
  /* [out] */socket__addr_t *saddr,
  /* [out] */ndr__ulong_int *slen,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__use_family_wk
#ifdef __STDC__
 (
  /* [in] */ndr__ulong_int family,
  /* [in] */rpc__if_spec_t *ifspec,
  /* [out] */socket__addr_t *saddr,
  /* [out] */ndr__ulong_int *slen,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__register
#ifdef __STDC__
 (
  /* [in] */rpc__if_spec_t *ifspec,
  /* [in] */rpc__epv_t epv,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__unregister
#ifdef __STDC__
 (
  /* [in] */rpc__if_spec_t *ifspec,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__register_mgr
#ifdef __STDC__
 (
  /* [in] */uuid__t *typ,
  /* [in] */rpc__if_spec_t *ifspec,
  /* [in] */rpc__generic_epv_t sepv,
  /* [in] */rpc__mgr_epv_t mepv,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__register_object
#ifdef __STDC__
 (
  /* [in] */uuid__t *obj,
  /* [in] */uuid__t *typ,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  handle_t rpc__get_handle
#ifdef __STDC__
 (
  /* [in] */uuid__t *actuid,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  handle_t rpc__alloc_handle
#ifdef __STDC__
 (
  /* [in] */uuid__t *obj,
  /* [in] */ndr__ulong_int family,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__set_binding
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */socket__addr_t *saddr,
  /* [in] */ndr__ulong_int slen,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__inq_binding
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */socket__addr_t *saddr,
  /* [out] */ndr__ulong_int *slen,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__clear_server_binding
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__clear_binding
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  handle_t rpc__bind
#ifdef __STDC__
 (
  /* [in] */uuid__t *obj,
  /* [in] */socket__addr_t *saddr,
  /* [in] */ndr__ulong_int slen,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__free_handle
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  handle_t rpc__dup_handle
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__listen
#ifdef __STDC__
 (
  /* [in] */ndr__ulong_int max_calls,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__listen_dispatch
#ifdef __STDC__
 (
  /* [in] */ndr__ulong_int sock,
  /* [in] */rpc__ppkt_t *pkt,
  /* [in] */rpc__cksum_t cksum,
  /* [in] */socket__addr_t *from,
  /* [in] */ndr__ulong_int from_len,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__listen_recv
#ifdef __STDC__
 (
  /* [in] */ndr__ulong_int sock,
  /* [out] */rpc__ppkt_t *pkt,
  /* [out] */rpc__cksum_t *cksum,
  /* [out] */socket__addr_t *from,
  /* [out] */ndr__ulong_int *from_len,
  /* [out] */ndr__ulong_int *ptype,
  /* [out] */uuid__t *obj,
  /* [out] */uuid__t *if_id,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__forward
#ifdef __STDC__
 (
  /* [in] */ndr__ulong_int sock,
  /* [in] */socket__addr_t *from,
  /* [in] */ndr__ulong_int from_len,
  /* [in] */socket__addr_t *taddr,
  /* [in] */ndr__ulong_int to_len,
  /* [in] */rpc__ppkt_t *pkt,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__name_to_sockaddr
#ifdef __STDC__
 (
  /* [in] */rpc__string_t name,
  /* [in] */ndr__ulong_int namelen,
  /* [in] */ndr__ulong_int port,
  /* [in] */ndr__ulong_int family,
  /* [out] */socket__addr_t *saddr,
  /* [out] */ndr__ulong_int *slen,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__sockaddr_to_name
#ifdef __STDC__
 (
  /* [in] */socket__addr_t *saddr,
  /* [in] */ndr__ulong_int slen,
  /* [out] */rpc__string_t name,
  /* [in, out] */ndr__ulong_int *namelen,
  /* [out] */ndr__ulong_int *port,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__inq_object
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */uuid__t *obj,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__shutdown
#ifdef __STDC__
 (
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__allow_remote_shutdown
#ifdef __STDC__
 (
  /* [in] */ndr__ulong_int allow,
  /* [in] */rpc__shut_check_fn_t cproc,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__set_auth_logger
#ifdef __STDC__
 (
  /* [in] */rpc__auth_log_fn_t lproc);
#else
 ( );
#endif
extern  ndr__ulong_int rpc__set_short_timeout
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */ndr__ulong_int on,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  ndr__ulong_int rpc__set_fault_mode
#ifdef __STDC__
 (
  /* [in] */ndr__ulong_int on);
#else
 ( );
#endif
extern  void rpc__set_async_ack
#ifdef __STDC__
 (
  /* [in] */ndr__ulong_int on);
#else
 ( );
#endif
extern  void rpc__sar
#ifdef __STDC__
 (
  /* [in] */handle_t hp,
  /* [in] */rpc__sar_opts_t opts,
  /* [in] */rpc__if_spec_t *ifspec,
  /* [in] */ndr__ulong_int opn,
  /* [in] */rpc__ppkt_t *ins,
  /* [in] */ndr__ulong_int ilen,
  /* [in] */rpc__ppkt_t *outs,
  /* [in] */ndr__ulong_int omax,
  /* [out] */rpc__ppkt_p_t *routs,
  /* [out] */ndr__ulong_int *olen,
  /* [out] */rpc__drep_t *drep,
  /* [out] */ndr__boolean *must_free,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rpc__cvt_short_float
#ifdef __STDC__
 (
  /* [in] */rpc__drep_t src_drep,
  /* [in] */rpc__drep_t dst_drep,
  /* [in] */rpc__short_float_p_t src,
  /* [in] */rpc__short_float_p_t dst);
#else
 ( );
#endif
extern  void rpc__cvt_long_float
#ifdef __STDC__
 (
  /* [in] */rpc__drep_t src_drep,
  /* [in] */rpc__drep_t dst_drep,
  /* [in] */rpc__long_float_p_t src,
  /* [in] */rpc__long_float_p_t dst);
#else
 ( );
#endif
extern  rpc__ppkt_p_t rpc__alloc_pkt
#ifdef __STDC__
 (
  /* [in] */ndr__ulong_int len);
#else
 ( );
#endif
extern  void rpc__free_pkt
#ifdef __STDC__
 (
  /* [in] */rpc__ppkt_p_t p);
#else
 ( );
#endif
extern  void rpc__cvt_string
#ifdef __STDC__
 (
  /* [in] */rpc__drep_t src_drep,
  /* [in] */rpc__drep_t dst_drep,
  /* [in] */rpc__char_p_t src,
  /* [in] */rpc__char_p_t dst);
#else
 ( );
#endif
extern  void rpc__block_copy
#ifdef __STDC__
 (
  /* [in] */rpc__byte_p_t src,
  /* [in] */rpc__byte_p_t dst,
  /* [in] */ndr__ulong_int count);
#else
 ( );
#endif
#endif
