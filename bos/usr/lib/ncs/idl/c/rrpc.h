#ifndef rrpc__v0_included
#define rrpc__v0_included
#include "idl_base.h"
#include "rpc.h"
#include "nbase.h"
#include "rpc.h"
static rpc__if_spec_t rrpc__v0_if_spec = {
  0,
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  4,
  {
  0x36ce399d,
  0x4000,
  0,
  0xd,
  {0x0, 0x0, 0xc3, 0x66, 0x0, 0x0, 0x0}
  }
};
#define rrpc__mod 470024192
#define rrpc__shutdown_not_allowed 470024193
typedef rpc__if_spec_t rrpc__interface_vec_t[1];
typedef ndr__long_int rrpc__stat_vec_t[1];
#define rrpc__sv_calls_in 0
#define rrpc__sv_rcvd 1
#define rrpc__sv_sent 2
#define rrpc__sv_calls_out 3
#define rrpc__sv_frag_resends 4
#define rrpc__sv_dup_frags_rcvd 5
#define rrpc__sv_n_calls rrpc__sv_calls_in
#define rrpc__sv_n_pkts_rcvd rrpc__sv_rcvd
#define rrpc__sv_n_pkts_sent rrpc__sv_sent
extern  void rrpc__are_you_there
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rrpc__inq_stats
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */ndr__ulong_int max_stats,
  /* [out] */rrpc__stat_vec_t stats,
  /* [out] */ndr__long_int *l_stat,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rrpc__inq_interfaces
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */ndr__ulong_int max_ifs,
  /* [out] */rrpc__interface_vec_t ifs,
  /* [out] */ndr__long_int *l_if,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void rrpc__shutdown
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status__t *st);
#else
 ( );
#endif
typedef struct rrpc__v0_epv_t {
void (*rrpc__are_you_there)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status__t *st)
#else
()
#endif
;
void (*rrpc__inq_stats)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */ndr__ulong_int max_stats,
  /* [out] */rrpc__stat_vec_t stats,
  /* [out] */ndr__long_int *l_stat,
  /* [out] */status__t *st)
#else
()
#endif
;
void (*rrpc__inq_interfaces)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */ndr__ulong_int max_ifs,
  /* [out] */rrpc__interface_vec_t ifs,
  /* [out] */ndr__long_int *l_if,
  /* [out] */status__t *st)
#else
()
#endif
;
void (*rrpc__shutdown)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status__t *st)
#else
()
#endif
;
} rrpc__v0_epv_t;
#ifndef vms
extern rrpc__v0_epv_t rrpc__v0_client_epv;
#else /* vms */
#ifdef NIDL_CSWTCH
GLOBALREF(rrpc__v0_epv_t, rrpc__v0_client_epv);
#endif
#endif /* vms */
#ifndef vms
extern rpc__generic_epv_t rrpc__v0_server_epv;
#else /* vms */
#ifdef NIDL_SSTUB
GLOBALREF(rpc__generic_epv_t, rrpc__v0_server_epv);
#endif
#endif /* vms */
#endif
