#ifndef base__v0_included
#define base__v0_included
#include "idl_base.h"
#include "nbase.h"
#include "timebase.h"
typedef ndr__long_int status__all_t;
#define proc1__n_user_processes 56
#define name__long_complen_max 255
#define name__long_pnamlen_max 1023
#define name__pnamlen_max 256
#define name__complen_max 32
typedef ndr__char name__pname_t[256];
typedef ndr__char name__name_t[32];
typedef ndr__char name__long_name_t[256];
typedef ndr__char name__long_pname_t[1024];
#define ios__max 127
#define ios__stdin 0
#define ios__stdout 1
#define ios__stderr 2
#define ios__errin 2
#define ios__errout 2
#define stream__stdin ios__stdin
#define stream__stdout ios__stdout
#define stream__stderr ios__stderr
#define stream__errin stream__stderr
#define stream__errout stream__stderr
typedef ndr__short_int ios__id_t;
typedef struct NIDL_tag_17ce ios__seek_key_t;
struct NIDL_tag_17ce {
ndr__long_int rec_adr;
ndr__long_int byte_adr;
};
typedef ndr__short_int stream__id_t;
typedef struct NIDL_tag_14f0 uid__t;
struct NIDL_tag_14f0 {
ndr__long_int high;
ndr__long_int low;
};
#ifdef __STDC__
handle_t uid__t_bind(uid__t h);
void uid__t_unbind(uid__t uh, handle_t h);
#else
handle_t uid__t_bind();
void uid__t_unbind();
#endif
typedef struct NIDL_tag_7d1 xoid__t;
struct NIDL_tag_7d1 {
ndr__long_int rfu1;
ndr__long_int rfu2;
uid__t uid;
};
typedef struct NIDL_tag_1947 ec2__eventcount_t;
struct NIDL_tag_1947 {
ndr__long_int value;
pinteger awaiters;
};
typedef ec2__eventcount_t *ec2__ptr_t;
#endif
