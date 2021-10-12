#ifndef socket__v0_included
#define socket__v0_included
#include "idl_base.h"
#include "nbase.h"
#define socket__wk_fwd ((ndr__ushort_int) 0x0)
typedef ndr__ushort_int socket__wk_ports_t;
typedef ndr__char socket__string_t[100];
typedef socket__addr_t socket__addr_list_t[1];
typedef ndr__ulong_int socket__len_list_t[1];
typedef ndr__char socket__local_sockaddr_t[50];
#define socket__eq_hostid 1
#define socket__eq_netaddr 2
#define socket__eq_port 4
#define socket__eq_network 8
#define socket__addr_module_code 268566528
#define socket__buff_too_large 268566529
#define socket__buff_too_small 268566530
#define socket__bad_numeric_name 268566531
#define socket__cant_find_name 268566532
#define socket__cant_cvrt_addr_to_name 268566533
#define socket__cant_get_local_name 268566534
#define socket__cant_create_socket 268566535
#define socket__cant_get_if_config 268566536
#define socket__internal_error 268566537
#define socket__family_not_valid 268566538
#define socket__invalid_name_format 268566539
extern  ndr__boolean socket__valid_family
#ifdef __STDC__
 (
  /* [in] */ndr__ulong_int family,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void socket__valid_families
#ifdef __STDC__
 (
  /* [in, out] */ndr__ulong_int *num,
  /* [out] */socket__addr_family_t families[1],
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  ndr__ulong_int socket__inq_port
#ifdef __STDC__
 (
  /* [in] */socket__addr_t *saddr,
  /* [in] */ndr__ulong_int slen,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void socket__set_port
#ifdef __STDC__
 (
  /* [in, out] */socket__addr_t *saddr,
  /* [in, out] */ndr__ulong_int *slen,
  /* [in] */ndr__ulong_int port,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void socket__set_wk_port
#ifdef __STDC__
 (
  /* [in, out] */socket__addr_t *saddr,
  /* [in, out] */ndr__ulong_int *slen,
  /* [in] */ndr__ulong_int port,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  ndr__boolean socket__equal
#ifdef __STDC__
 (
  /* [in] */socket__addr_t *saddr1,
  /* [in] */ndr__ulong_int slen1,
  /* [in] */socket__addr_t *saddr2,
  /* [in] */ndr__ulong_int slen2,
  /* [in] */ndr__ulong_int flags,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void socket__from_name
#ifdef __STDC__
 (
  /* [in] */ndr__ulong_int family,
  /* [in] */socket__string_t name,
  /* [in] */ndr__ulong_int namelen,
  /* [in] */ndr__ulong_int port,
  /* [out] */socket__addr_t *saddr,
  /* [in, out] */ndr__ulong_int *slen,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  ndr__ulong_int socket__family_from_name
#ifdef __STDC__
 (
  /* [in] */socket__string_t name,
  /* [in] */ndr__ulong_int namelen,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void socket__family_to_name
#ifdef __STDC__
 (
  /* [in] */ndr__ulong_int family,
  /* [out] */socket__string_t name,
  /* [in, out] */ndr__ulong_int *namelen,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void socket__to_name
#ifdef __STDC__
 (
  /* [in] */socket__addr_t *saddr,
  /* [in] */ndr__ulong_int slen,
  /* [out] */socket__string_t name,
  /* [in, out] */ndr__ulong_int *namelen,
  /* [out] */ndr__ulong_int *port,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void socket__to_numeric_name
#ifdef __STDC__
 (
  /* [in] */socket__addr_t *saddr,
  /* [in] */ndr__ulong_int slen,
  /* [out] */socket__string_t name,
  /* [in, out] */ndr__ulong_int *namelen,
  /* [out] */ndr__ulong_int *port,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void socket__set_broadcast
#ifdef __STDC__
 (
  /* [in, out] */socket__addr_t *saddr,
  /* [in, out] */ndr__ulong_int *slen,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  ndr__ulong_int socket__max_pkt_size
#ifdef __STDC__
 (
  /* [in] */ndr__ulong_int family,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void socket__inq_my_netaddr
#ifdef __STDC__
 (
  /* [in] */ndr__ulong_int family,
  /* [out] */socket__net_addr_t *naddr,
  /* [in, out] */ndr__ulong_int *nlen,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void socket__inq_netaddr
#ifdef __STDC__
 (
  /* [in] */socket__addr_t *saddr,
  /* [in] */ndr__ulong_int slen,
  /* [out] */socket__net_addr_t *naddr,
  /* [in, out] */ndr__ulong_int *nlen,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void socket__set_netaddr
#ifdef __STDC__
 (
  /* [in, out] */socket__addr_t *saddr,
  /* [in, out] */ndr__ulong_int *slen,
  /* [in] */socket__net_addr_t *naddr,
  /* [in] */ndr__ulong_int nlen,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void socket__inq_hostid
#ifdef __STDC__
 (
  /* [in] */socket__addr_t *saddr,
  /* [in] */ndr__ulong_int slen,
  /* [out] */socket__host_id_t *hid,
  /* [in, out] */ndr__ulong_int *hlen,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void socket__set_hostid
#ifdef __STDC__
 (
  /* [in, out] */socket__addr_t *saddr,
  /* [in, out] */ndr__ulong_int *slen,
  /* [in] */socket__host_id_t *hid,
  /* [in] */ndr__ulong_int hlen,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void socket__inq_broad_addrs
#ifdef __STDC__
 (
  /* [in] */ndr__ulong_int family,
  /* [in] */ndr__ulong_int port,
  /* [out] */socket__addr_list_t brd_addrs,
  /* [out] */socket__len_list_t brd_lens,
  /* [in, out] */ndr__ulong_int *len,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void socket__to_local_rep
#ifdef __STDC__
 (
  /* [in] */socket__addr_t *saddr,
  /* [in, out] */socket__local_sockaddr_t lcl_saddr,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  void socket__from_local_rep
#ifdef __STDC__
 (
  /* [in, out] */socket__addr_t *saddr,
  /* [in] */socket__local_sockaddr_t lcl_saddr,
  /* [out] */status__t *st);
#else
 ( );
#endif
#endif
