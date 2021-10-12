#ifndef nbase__v0_included
#define nbase__v0_included
#include "idl_base.h"
typedef ndr__short_int binteger;
typedef ndr__short_int pinteger;
typedef ndr__long_int linteger;
typedef struct NIDL_tag_1e9f status__t;
struct NIDL_tag_1e9f {
ndr__long_int all;
};
#define status__ok 0
typedef struct NIDL_tag_4e9 uuid__t;
struct NIDL_tag_4e9 {
ndr__ulong_int time_high;
ndr__ushort_int time_low;
ndr__ushort_int reserved;
ndr__byte family;
ndr__byte host[7];
};
#ifdef __STDC__
handle_t uuid__t_bind(uuid__t h);
void uuid__t_unbind(uuid__t uh, handle_t h);
#else
handle_t uuid__t_bind();
void uuid__t_unbind();
#endif
#define socket__unspec_port 0
#define socket__unspec ((ndr__ushort_int) 0x0)
#define socket__unix ((ndr__ushort_int) 0x1)
#define socket__internet ((ndr__ushort_int) 0x2)
#define socket__implink ((ndr__ushort_int) 0x3)
#define socket__pup ((ndr__ushort_int) 0x4)
#define socket__chaos ((ndr__ushort_int) 0x5)
#define socket__ns ((ndr__ushort_int) 0x6)
#define socket__nbs ((ndr__ushort_int) 0x7)
#define socket__ecma ((ndr__ushort_int) 0x8)
#define socket__datakit ((ndr__ushort_int) 0x9)
#define socket__ccitt ((ndr__ushort_int) 0xa)
#define socket__sna ((ndr__ushort_int) 0xb)
#define socket__decnet ((ndr__ushort_int) 0xc)
#define socket__dds ((ndr__ushort_int) 0xd)
typedef ndr__ushort_int socket__addr_family_t;
#define socket__num_families 32
#define socket__sizeof_family 2
#define socket__sizeof_data 14
#define socket__sizeof_ndata 12
#define socket__sizeof_hdata 12
typedef struct NIDL_tag_7f4 socket__addr_t;
struct NIDL_tag_7f4 {
socket__addr_family_t family;
ndr__byte data[14];
};
typedef struct NIDL_tag_1701 socket__net_addr_t;
struct NIDL_tag_1701 {
socket__addr_family_t family;
ndr__byte data[12];
};
typedef struct NIDL_tag_976 socket__host_id_t;
struct NIDL_tag_976 {
socket__addr_family_t family;
ndr__byte data[12];
};
#endif
