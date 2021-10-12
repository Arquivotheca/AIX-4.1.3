#ifndef rproc__v1_included
#define rproc__v1_included
#include "idl_base.h"
#include "rpc.h"
#include "nbase.h"
static rpc__if_spec_t rproc__v1_if_spec = {
  1,
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  1,
  {
  0x33c38289,
  0x8000,
  0,
  0xd,
  {0x0, 0x0, 0x80, 0x9c, 0x0, 0x0, 0x0}
  }
};
#define rproc__cant_run_prog 470155265
#define rproc__cant_create_proc 470155266
#define rproc__internal_error 470155267
#define rproc__too_many_args 470155268
#define rproc__not_allowed 470155269
#define rproc__cant_set_id 470155270
typedef ndr__long_int rproc__t;
typedef ndr__char rproc__arg_t[128];
typedef rproc__arg_t rproc__args_t[1];
extern  void rproc__create_simple
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */ndr__char pname[256],
  /* [in] */ndr__long_int argc,
  /* [in] */ndr__char argv[1][128],
  /* [out] */rproc__t *proc,
  /* [out] */status__t *st);
#else
 ( );
#endif
typedef struct rproc__v1_epv_t {
void (*rproc__create_simple)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */ndr__char pname[256],
  /* [in] */ndr__long_int argc,
  /* [in] */ndr__char argv[1][128],
  /* [out] */rproc__t *proc,
  /* [out] */status__t *st)
#else
()
#endif
;
} rproc__v1_epv_t;
#ifndef vms
extern rproc__v1_epv_t rproc__v1_client_epv;
#else /* vms */
#ifdef NIDL_CSWTCH
GLOBALREF(rproc__v1_epv_t, rproc__v1_client_epv);
#endif
#endif /* vms */
#ifndef vms
extern rpc__generic_epv_t rproc__v1_server_epv;
#else /* vms */
#ifdef NIDL_SSTUB
GLOBALREF(rpc__generic_epv_t, rproc__v1_server_epv);
#endif
#endif /* vms */
#endif
