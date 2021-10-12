#ifndef conv__v3_included
#define conv__v3_included
#include "idl_base.h"
#include "rpc.h"
#include "nbase.h"
static rpc__if_spec_t conv__v3_if_spec = {
  3,
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  1,
  {
  0x333a2276,
  0x0000,
  0,
  0xd,
  {0x0, 0x0, 0x80, 0x9c, 0x0, 0x0, 0x0}
  }
};
extern  void conv__who_are_you
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */uuid__t *actuid,
  /* [in] */ndr__ulong_int boot_time,
  /* [out] */ndr__ulong_int *seq,
  /* [out] */status__t *st);
#else
 ( );
#endif
typedef struct conv__v3_epv_t {
void (*conv__who_are_you)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */uuid__t *actuid,
  /* [in] */ndr__ulong_int boot_time,
  /* [out] */ndr__ulong_int *seq,
  /* [out] */status__t *st)
#else
()
#endif
;
} conv__v3_epv_t;
#ifndef vms
extern conv__v3_epv_t conv__v3_client_epv;
#else /* vms */
#ifdef NIDL_CSWTCH
GLOBALREF(conv__v3_epv_t, conv__v3_client_epv);
#endif
#endif /* vms */
#ifndef vms
extern rpc__generic_epv_t conv__v3_server_epv;
#else /* vms */
#ifdef NIDL_SSTUB
GLOBALREF(rpc__generic_epv_t, conv__v3_server_epv);
#endif
#endif /* vms */
#endif
