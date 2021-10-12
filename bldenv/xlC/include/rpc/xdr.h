/* @(#)97       1.1  src/bldenv/xlC/include/rpc/xdr.h, ade_build, bos41J, 9509A_all 2/27/95 13:38:17 */
/*
 *   COMPONENT_NAME: xlC
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
extern "C" {
#define xdr_int ____xdr_int
#define xdr_u_int ____xdr_u_int
#define xdr_long ____xdr_long
#define xdr_u_long ____xdr_u_long
#define xdr_short ____xdr_short
#define xdr_u_short ____xdr_u_short
#define xdr_bool ____xdr_bool
#define xdr_enum ____xdr_enum
#define xdr_array ____xdr_array
#define xdr_bytes ____xdr_bytes
#define xdr_opaque ____xdr_opaque
#define xdr_string ____xdr_string
#define xdr_union ____xdr_union
#define xdr_char ____xdr_char
#define xdr_u_char ____xdr_u_char
#define xdr_vector ____xdr_vector
#define xdr_float ____xdr_float
#define xdr_double ____xdr_double
#define xdr_free   ____xdr_free
#define xdr_reference ____xdr_reference
#define xdr_pointer ____xdr_pointer
#define xdr_wrapstring ____xdr_wrapstring
#define xdr_netobj ____xdr_netobj
#define xdrmem_create ____xdrmem_create
#define xdrstdio_create ____xdrstdio_create
#define xdrrec_create ____xdrrec_create
#define xdrrec_endofrecord ____xdrrec_endofrecord
#define xdrrec_skiprecord ____xdrrec_skiprecord
#define xdrrec_eof ____xdrrec_eof
#define xdrproc_t ____xdrproc_t
#include </usr/include/rpc/xdr.h>
#undef XDR_DESTROY
#define       XDR_DESTROY(xdrs)                               \
      (*(void (*)(XDR *))((xdrs)->x_ops->x_destroy))(xdrs)
#undef xdr_int
#undef xdr_u_int
#undef xdr_long
#undef xdr_u_long
#undef xdr_short
#undef xdr_u_short
#undef xdr_bool
#undef xdr_enum
#undef xdr_array
#undef xdr_bytes
#undef xdr_opaque
#undef xdr_string
#undef xdr_union
#undef xdr_char
#undef xdr_free
#undef xdr_u_char
#undef xdr_vector
#undef xdr_float
#undef xdr_double
#undef xdr_reference
#undef xdr_pointer
#undef xdr_wrapstring
#undef xdr_netobj
#undef xdrmem_create
#undef xdrstdio_create
#undef xdrrec_create
#undef xdrrec_endofrecord
#undef xdrrec_skiprecord
#undef xdrrec_eof
#undef xdrproc_t
#include <stdio.h>
#include <sys/types.h>
typedef bool_t	(*xdrproc_t)(XDR *, caddr_t *);
extern bool_t	xdr_int(XDR*, int*);
extern bool_t	xdr_u_int(XDR*, u_int*);
extern bool_t	xdr_long(XDR*, long*);
extern bool_t	xdr_u_long(XDR*, u_long*);
extern bool_t	xdr_short(XDR*, short*);
extern bool_t	xdr_u_short(XDR*, u_short*);
extern bool_t	xdr_bool(XDR*, bool_t*);
extern bool_t	xdr_enum(XDR*, int*);
extern bool_t	xdr_array(XDR*, char **, u_int *, u_int, u_int, xdrproc_t);
extern bool_t	xdr_bytes(XDR*, char **, u_int *, u_int);
extern bool_t	xdr_opaque(XDR*, char *, u_int);
extern bool_t	xdr_string(XDR*, char **, u_int);
extern bool_t	xdr_union(XDR*, int *, char *, struct xdr_discrim *,
			  xdrproc_t *);
extern bool_t	xdr_char(XDR*, char *);
extern bool_t	xdr_u_char(XDR*, u_char *);
extern bool_t	xdr_vector(XDR*, char*, u_int, u_int, xdrproc_t);
extern bool_t	xdr_float(XDR*, float*);
extern bool_t	xdr_double(XDR*, double*);
extern bool_t	xdr_reference(XDR*, char **, u_int, xdrproc_t);
extern bool_t	xdr_pointer(XDR*, char **, u_int, xdrproc_t);
extern bool_t	xdr_wrapstring(XDR*, char **);
extern bool_t   xdr_netobj(XDR*, struct netobj *);
extern void   xdrmem_create(XDR*, char*, u_int, enum xdr_op);
extern void   xdrstdio_create(XDR*, FILE*, enum xdr_op);
extern void   xdrrec_create(XDR*, u_int, u_int, char *,
			    int (*read)(int, void *, size_t),
			    int (*write)(int, const void *, size_t));
extern bool_t xdrrec_endofrecord(XDR*, bool_t);
extern bool_t xdrrec_skiprecord(XDR*);
extern bool_t xdrrec_eof(XDR*);
extern void xdr_free (xdrproc_t, char *);
}
