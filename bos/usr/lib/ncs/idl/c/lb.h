#ifndef lb__v0_included
#define lb__v0_included
#include "idl_base.h"
#include "nbase.h"
#include "rpc.h"
#define lb__mod 469893120
#define lb__database_invalid 469893121
#define lb__database_busy 469893122
#define lb__not_registered 469893123
#define lb__update_failed 469893124
#define lb__cant_access 469893125
#define lb__server_unavailable 469893126
#define lb__bad_entry 469893127
typedef ndr__ulong_int lb__server_flag_t;
#define lb__server_flag_local 1
#define lb__server_flag_reserved_02 2
#define lb__server_flag_reserved_04 4
#define lb__server_flag_reserved_08 8
#define lb__server_flag_reserved_10 16
#define lb__server_flag_reserved_20 32
#define lb__server_flag_reserved_40 64
#define lb__server_flag_reserved_80 128
#define lb__server_flag_reserved_0100 256
#define lb__server_flag_reserved_0200 512
#define lb__server_flag_reserved_0400 1024
#define lb__server_flag_reserved_0800 2048
#define lb__server_flag_reserved_1000 4096
#define lb__server_flag_reserved_2000 8192
#define lb__server_flag_reserved_4000 16384
#define lb__server_flag_reserved_8000 32768
#define lb__server_flag_reserved_10000 65536
#define lb__server_flag_reserved_20000 131072
#define lb__server_flag_reserved_40000 262144
#define lb__server_flag_reserved_80000 524288
typedef ndr__ulong_int lb__lookup_handle_t;
#define lb__default_lookup_handle 0
typedef struct NIDL_tag_904 lb__entry_t;
struct NIDL_tag_904 {
uuid__t object;
uuid__t obj_type;
uuid__t obj_interface;
lb__server_flag_t flags;
ndr__char annotation[64];
ndr__ulong_int saddr_len;
socket__addr_t saddr;
};
extern  void lb__register
#ifdef __STDC__
 (
  /* [in] */uuid__t *object,
  /* [in] */uuid__t *obj_type,
  /* [in] */uuid__t *obj_interface,
  /* [in] */lb__server_flag_t flags,
  /* [in] */ndr__char annotation[64],
  /* [in] */socket__addr_t *saddr,
  /* [in] */ndr__ulong_int saddr_len,
  /* [out] */lb__entry_t *xentry,
  /* [out] */status__t *status);
#else
 ( );
#endif
extern  void lb__unregister
#ifdef __STDC__
 (
  /* [in] */lb__entry_t *xentry,
  /* [out] */status__t *status);
#else
 ( );
#endif
extern  void lb__lookup_range
#ifdef __STDC__
 (
  /* [in] */uuid__t *object,
  /* [in] */uuid__t *obj_type,
  /* [in] */uuid__t *obj_interface,
  /* [in] */socket__addr_t *location,
  /* [in] */ndr__ulong_int location_len,
  /* [in, out] */lb__lookup_handle_t *entry_handle,
  /* [in] */ndr__ulong_int max_num_results,
  /* [out] */ndr__ulong_int *num_results,
  /* [out] */lb__entry_t result_entries[1],
  /* [out] */status__t *status);
#else
 ( );
#endif
extern  void lb__lookup_object
#ifdef __STDC__
 (
  /* [in] */uuid__t *object,
  /* [in, out] */lb__lookup_handle_t *entry_handle,
  /* [in] */ndr__ulong_int max_num_results,
  /* [out] */ndr__ulong_int *num_results,
  /* [out] */lb__entry_t result_entries[1],
  /* [out] */status__t *status);
#else
 ( );
#endif
extern  void lb__lookup_object_local
#ifdef __STDC__
 (
  /* [in] */uuid__t *object,
  /* [in] */socket__addr_t *location,
  /* [in] */ndr__ulong_int location_len,
  /* [in, out] */lb__lookup_handle_t *entry_handle,
  /* [in] */ndr__ulong_int max_num_results,
  /* [out] */ndr__ulong_int *num_results,
  /* [out] */lb__entry_t result_entries[1],
  /* [out] */status__t *status);
#else
 ( );
#endif
extern  void lb__lookup_type
#ifdef __STDC__
 (
  /* [in] */uuid__t *obj_type,
  /* [in, out] */lb__lookup_handle_t *entry_handle,
  /* [in] */ndr__ulong_int max_num_results,
  /* [out] */ndr__ulong_int *num_results,
  /* [out] */lb__entry_t result_entries[1],
  /* [out] */status__t *status);
#else
 ( );
#endif
extern  void lb__lookup_interface
#ifdef __STDC__
 (
  /* [in] */uuid__t *obj_interface,
  /* [in, out] */lb__lookup_handle_t *entry_handle,
  /* [in] */ndr__ulong_int max_num_results,
  /* [out] */ndr__ulong_int *num_results,
  /* [out] */lb__entry_t result_entries[1],
  /* [out] */status__t *status);
#else
 ( );
#endif
extern  ndr__ulong_int lb__use_short_timeouts
#ifdef __STDC__
 (
  /* [in] */ndr__ulong_int flag);
#else
 ( );
#endif
#endif
