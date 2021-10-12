#ifndef llb__v4_included
#define llb__v4_included
#include "idl_base.h"
#include "rpc.h"
#include "nbase.h"
#include "lb.h"
static rpc__if_spec_t llb__v4_if_spec = {
  4,
  {0, 0, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  3,
  {
  0x333b33c3,
  0x0000,
  0,
  0xd,
  {0x0, 0x0, 0x87, 0x84, 0x0, 0x0, 0x0}
  }
};
extern  void llb__insert
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */lb__entry_t *xentry,
  /* [out] */status__t *status);
#else
 ( );
#endif
extern  void llb__delete
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */lb__entry_t *xentry,
  /* [out] */status__t *status);
#else
 ( );
#endif
#define llb__max_lookup_results 6
extern  void llb__lookup
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */uuid__t *object,
  /* [in] */uuid__t *obj_type,
  /* [in] */uuid__t *obj_interface,
  /* [in, out] */lb__lookup_handle_t *entry_handle,
  /* [in] */ndr__ulong_int max_num_results,
  /* [out] */ndr__ulong_int *num_results,
  /* [out] */lb__entry_t result_entries[6],
  /* [out] */status__t *status);
#else
 ( );
#endif
typedef struct llb__v4_epv_t {
void (*llb__insert)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */lb__entry_t *xentry,
  /* [out] */status__t *status)
#else
()
#endif
;
void (*llb__delete)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */lb__entry_t *xentry,
  /* [out] */status__t *status)
#else
()
#endif
;
void (*llb__lookup)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */uuid__t *object,
  /* [in] */uuid__t *obj_type,
  /* [in] */uuid__t *obj_interface,
  /* [in, out] */lb__lookup_handle_t *entry_handle,
  /* [in] */ndr__ulong_int max_num_results,
  /* [out] */ndr__ulong_int *num_results,
  /* [out] */lb__entry_t result_entries[6],
  /* [out] */status__t *status)
#else
()
#endif
;
} llb__v4_epv_t;
#ifdef NIDL_CSWTCH
GLOBALREF(llb__v4_epv_t, llb__v4_client_epv);
#endif
#ifdef NIDL_SSTUB
GLOBALREF(rpc__generic_epv_t, llb__v4_server_epv);
#endif
#endif
