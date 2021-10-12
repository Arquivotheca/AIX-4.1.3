/* static char sccsid[] = "@(#)43  1.9  src/bos/usr/include/rpc/xdr.h, libcrpc, bos411, 9428A410j 3/3/94 14:50:57"; */
/*
 *   COMPONENT_NAME: libcrpc
 *
 *   FUNCTIONS: IXDR_GET_BOOL
 *		IXDR_GET_ENUM
 *		IXDR_GET_LONG
 *		IXDR_GET_SHORT
 *		IXDR_GET_U_LONG
 *		IXDR_GET_U_SHORT
 *		IXDR_PUT_BOOL
 *		IXDR_PUT_ENUM
 *		IXDR_PUT_LONG
 *		IXDR_PUT_SHORT
 *		IXDR_PUT_U_LONG
 *		IXDR_PUT_U_SHORT
 *		RNDUP
 *		XDR_DESTROY
 *		XDR_GETBYTES
 *		XDR_GETLONG
 *		XDR_GETPOS
 *		XDR_INLINE
 *		XDR_PUTBYTES
 *		XDR_PUTLONG
 *		XDR_SETPOS
 *		xdr_destroy
 *		xdr_getbytes
 *		xdr_getlong
 *		xdr_getpos
 *		xdr_inline
 *		xdr_putbytes
 *		xdr_putlong
 *		xdr_setpos
 *		xdrrec_create
 *
 *   ORIGINS: 24,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *      @(#)xdr.h 1.22 88/02/08 SMI 
 */

/*	@(#)xdr.h	1.2 90/07/19 4.1NFSSRC SMI	*/

/*
 * xdr.h, External Data Representation Serialization Routines.
 */

#ifndef _RPC_XDR_H
#define	_RPC_XDR_H

#include <rpc/types.h>

#ifndef _KERNEL
#include <stdio.h>
#endif
/*
 * XDR provides a conventional way for converting between C data
 * types and an external bit-string representation.  Library supplied
 * routines provide for the conversion on built-in C data types.  These
 * routines and utility routines defined here are used to help implement
 * a type encode/decode routine for each user-defined type.
 *
 * Each data type provides a single procedure which takes two arguments:
 *
 *	bool_t
 *	xdrproc(xdrs, argresp)
 *		XDR *xdrs;
 *		<type> *argresp;
 *
 * xdrs is an instance of a XDR handle, to which or from which the data
 * type is to be converted.  argresp is a pointer to the structure to be
 * converted.  The XDR handle contains an operation field which indicates
 * which of the operations (ENCODE, DECODE * or FREE) is to be performed.
 *
 * XDR_DECODE may allocate space if the pointer argresp is null.  This
 * data can be freed with the XDR_FREE operation.
 *
 * We write only one procedure per data type to make it easy
 * to keep the encode and decode procedures for a data type consistent.
 * In many cases the same code performs all operations on a user defined type,
 * because all the hard work is done in the component type routines.
 * decode as a series of calls on the nested data types.
 */

/*
 * Xdr operations.  XDR_ENCODE causes the type to be encoded into the
 * stream.  XDR_DECODE causes the type to be extracted from the stream.
 * XDR_FREE can be used to release the space allocated by an XDR_DECODE
 * request.
 */
enum xdr_op {
	XDR_ENCODE=0,
	XDR_DECODE=1,
	XDR_FREE=2
};

/*
 * This is the number of bytes per unit of external data.
 */
#define BYTES_PER_XDR_UNIT	(4)
#define RNDUP(x)  ((((x) + BYTES_PER_XDR_UNIT - 1) / BYTES_PER_XDR_UNIT) \
		    * BYTES_PER_XDR_UNIT)

/*
 * A xdrproc_t exists for each data type which is to be encoded or decoded.
 *
 * The second argument to the xdrproc_t is a pointer to an opaque pointer.
 * The opaque pointer generally points to a structure of the data type
 * to be decoded.  If this pointer is 0, then the type routines should
 * allocate dynamic storage of the appropriate size and return it.
 * bool_t	(*xdrproc_t)(XDR *, caddr_t *);
 */
typedef	bool_t (*xdrproc_t)();

/*
 * The XDR handle.
 * Contains operation which is being applied to the stream,
 * an operations vector for the paticular implementation (e.g. see xdr_mem.c),
 * and two private fields for the use of the particular impelementation.
 */
typedef struct XDR {
	enum xdr_op	x_op;		/* operation; fast additional param */
	struct xdr_ops {
#ifdef _NO_PROTO
		bool_t	(*x_getlong)();	/* get a long from underlying stream */
		bool_t	(*x_putlong)();	/* put a long to " */
		bool_t	(*x_getbytes)();/* get some bytes from " */
		bool_t	(*x_putbytes)();/* put some bytes to " */
		u_int	(*x_getpostn)();/* returns bytes off from beginning */
		bool_t  (*x_setpostn)();/* lets you reposition the stream */
		long *	(*x_inline)();	/* buf quick ptr to buffered data */
		void	(*x_destroy)();	/* free privates of this xdr_stream */
#else /* _NO_PROTO */
		bool_t	(*x_getlong)(struct XDR *, long *);
		bool_t	(*x_putlong)(struct XDR *, long *);
		bool_t	(*x_getbytes)(struct XDR *, caddr_t, u_int);
		bool_t	(*x_putbytes)(struct XDR *, caddr_t, u_int);
		u_int	(*x_getpostn)(struct XDR *);
		bool_t  (*x_setpostn)(struct XDR *, u_int);
		long *	(*x_inline)(struct XDR *, u_int);
		void	(*x_destroy)(struct XDR *);
#endif /* _NO_PROTO */
	} *x_ops;
	caddr_t 	x_public;	/* users' data */
	caddr_t		x_private;	/* pointer to private data */
	caddr_t 	x_base;		/* private used for position info */
	int		x_handy;	/* extra private word */
} XDR;


/*
 * Operations defined on a XDR handle
 *
 * XDR		*xdrs;
 * long		*longp;
 * caddr_t	 addr;
 * u_int	 len;
 * u_int	 pos;
 */
#define XDR_GETLONG(xdrs, longp)			\
	(*(xdrs)->x_ops->x_getlong)(xdrs, longp)
#define xdr_getlong(xdrs, longp)			\
	(*(xdrs)->x_ops->x_getlong)(xdrs, longp)

#define XDR_PUTLONG(xdrs, longp)			\
	(*(xdrs)->x_ops->x_putlong)(xdrs, longp)
#define xdr_putlong(xdrs, longp)			\
	(*(xdrs)->x_ops->x_putlong)(xdrs, longp)

#define XDR_GETBYTES(xdrs, addr, len)			\
	(*(xdrs)->x_ops->x_getbytes)(xdrs, addr, len)
#define xdr_getbytes(xdrs, addr, len)			\
	(*(xdrs)->x_ops->x_getbytes)(xdrs, addr, len)

#define XDR_PUTBYTES(xdrs, addr, len)			\
	(*(xdrs)->x_ops->x_putbytes)(xdrs, addr, len)
#define xdr_putbytes(xdrs, addr, len)			\
	(*(xdrs)->x_ops->x_putbytes)(xdrs, addr, len)

#define XDR_GETPOS(xdrs)				\
	(*(xdrs)->x_ops->x_getpostn)(xdrs)
#define xdr_getpos(xdrs)				\
	(*(xdrs)->x_ops->x_getpostn)(xdrs)

#define XDR_SETPOS(xdrs, pos)				\
	(*(xdrs)->x_ops->x_setpostn)(xdrs, pos)
#define xdr_setpos(xdrs, pos)				\
	(*(xdrs)->x_ops->x_setpostn)(xdrs, pos)

#define	XDR_INLINE(xdrs, len)				\
	(*(xdrs)->x_ops->x_inline)(xdrs, len)
#define	xdr_inline(xdrs, len)				\
	(*(xdrs)->x_ops->x_inline)(xdrs, len)

#define	XDR_DESTROY(xdrs)				\
	(*(xdrs)->x_ops->x_destroy)(xdrs)
#define	xdr_destroy(xdrs) XDR_DESTROY(xdrs)

/*
 * Support struct for discriminated unions.
 * You create an array of xdrdiscrim structures, terminated with
 * a entry with a null procedure pointer.  The xdr_union routine gets
 * the discriminant value and then searches the array of structures
 * for a matching value.  If a match is found the associated xdr routine
 * is called to handle that part of the union.  If there is
 * no match, then a default routine may be called.
 * If there is no match and no default routine it is an error.
 */
#define NULL_xdrproc_t ((xdrproc_t)0)
struct xdr_discrim {
	int	value;
	xdrproc_t proc;
};

/*
 * In-line routines for fast encode/decode of primitve data types.
 * Caveat emptor: these use single memory cycles to get the
 * data from the underlying buffer, and will fail to operate
 * properly if the data is not aligned.  The standard way to use these
 * is to say:
 *	if ((buf = XDR_INLINE(xdrs, count)) == NULL)
 *		return (FALSE);
 *	<<< macro calls >>>
 * where ``count'' is the number of bytes of data occupied
 * by the primitive data types.
 *
 * N.B. and frozen for all time: each data type here uses 4 bytes
 * of external representation.
 */
#define IXDR_GET_LONG(buf)		((long)ntohl((u_long)*(buf)++))
#define IXDR_PUT_LONG(buf, v)		(*(buf)++ = (long)htonl((u_long)v))

#define IXDR_GET_BOOL(buf)		((bool_t)IXDR_GET_LONG(buf))
#define IXDR_GET_ENUM(buf, t)		((t)IXDR_GET_LONG(buf))
#define IXDR_GET_U_LONG(buf)		((u_long)IXDR_GET_LONG(buf))
#define IXDR_GET_SHORT(buf)		((short)IXDR_GET_LONG(buf))
#define IXDR_GET_U_SHORT(buf)		((u_short)IXDR_GET_LONG(buf))

#define IXDR_PUT_BOOL(buf, v)		IXDR_PUT_LONG((buf), ((long)(v)))
#define IXDR_PUT_ENUM(buf, v)		IXDR_PUT_LONG((buf), ((long)(v)))
#define IXDR_PUT_U_LONG(buf, v)		IXDR_PUT_LONG((buf), ((long)(v)))
#define IXDR_PUT_SHORT(buf, v)		IXDR_PUT_LONG((buf), ((long)(v)))
#define IXDR_PUT_U_SHORT(buf, v)	IXDR_PUT_LONG((buf), ((long)(v)))

/*
 * These are the "generic" xdr routines.
 */
#ifdef _NO_PROTO
extern bool_t	xdr_void();
extern bool_t	xdr_int();
extern bool_t	xdr_u_int();
extern bool_t	xdr_long();
extern bool_t	xdr_u_long();
extern bool_t	xdr_short();
extern bool_t	xdr_u_short();
extern bool_t	xdr_bool();
extern bool_t	xdr_enum();
extern bool_t	xdr_array();
extern bool_t	xdr_bytes();
extern bool_t	xdr_opaque();
extern bool_t	xdr_string();
extern bool_t	xdr_union();
#ifndef _KERNEL
extern void	xdr_free();
extern bool_t	xdr_char();
extern bool_t	xdr_u_char();
extern bool_t	xdr_vector();
extern bool_t	xdr_float();
extern bool_t	xdr_double();
extern bool_t	xdr_reference();
extern bool_t	xdr_pointer();
extern bool_t	xdr_wrapstring();
#endif /* !_KERNEL*/
#else /* _NO_PROTO */
extern bool_t	xdr_void(void);
extern bool_t	xdr_int(XDR *, int *);
extern bool_t	xdr_u_int(XDR *, u_int *);
extern bool_t	xdr_long(XDR *, long *);
extern bool_t	xdr_u_long(XDR *, u_long *);
extern bool_t	xdr_short(XDR *, short *);
extern bool_t	xdr_u_short(XDR *, u_short *);
extern bool_t	xdr_bool(XDR *, bool_t *);
extern bool_t	xdr_enum(XDR *, enum_t *);
extern bool_t	xdr_array(XDR *, caddr_t *, u_int *, u_int, u_int, xdrproc_t);
extern bool_t	xdr_bytes(XDR *, char **, u_int *, u_int);
extern bool_t	xdr_opaque(XDR *, caddr_t, u_int);
extern bool_t	xdr_string(XDR *, char **, u_int);
extern bool_t	xdr_union(XDR *, enum_t *, char *, struct xdr_discrim *, xdrproc_t);
#ifndef _KERNEL
extern void	xdr_free(xdrproc_t, char *);
extern bool_t	xdr_char(XDR *, char *);
extern bool_t	xdr_u_char(XDR *, u_char *);
extern bool_t	xdr_vector(XDR *, char *, u_int, u_int, xdrproc_t);
extern bool_t	xdr_float(XDR *, float *);
extern bool_t	xdr_double(XDR *, double *);
extern bool_t	xdr_reference(XDR *, caddr_t *, u_int, xdrproc_t);
extern bool_t	xdr_pointer(XDR *, char **, u_int, xdrproc_t);
extern bool_t	xdr_wrapstring(XDR *, char **);
#endif /* !_KERNEL*/
#endif /* _NO_PROTO */
/*
 * Common opaque bytes objects used by many rpc protocols;
 * declared here due to commonality.
 */
#define MAX_NETOBJ_SZ 1024 
struct netobj {
	u_int	n_len;
	char	*n_bytes;
};
typedef struct netobj netobj;
#ifdef _NO_PROTO
extern bool_t   xdr_netobj();
#else
extern bool_t   xdr_netobj(XDR *, netobj *);
#endif
/*
 * These are the public routines for the various implementations of
 * xdr streams.
 */
#ifdef _NO_PROTO
extern void   xdrmem_create();		/* XDR using memory buffers */
#ifndef _KERNEL
extern void   xdrstdio_create();	/* XDR using stdio library */
extern void   xdrrec_create();		/* XDR pseudo records for tcp */
extern bool_t xdrrec_endofrecord();	/* make end of xdr record */
extern bool_t xdrrec_skiprecord();	/* move to beginning of next record */
extern bool_t xdrrec_eof();		/* true if no more input */
#else
extern void xdrmbuf_init();		/* XDR using kernel mbufs */
#endif /* !_KERNEL*/
#else /* _NO_PROTO */
extern void   xdrmem_create(XDR *, caddr_t, u_int, enum xdr_op);
#ifndef _KERNEL
extern void   xdrstdio_create(XDR *, FILE *, enum xdr_op);
extern void   xdrrec_create(XDR *, u_int, u_int, caddr_t, int (*)(void *, caddr_t, int), int (*)(void *, caddr_t, int));
/*
extern bool_t xdrrec_endofrecord(XDR *, boot_t);
*/
extern bool_t xdrrec_skiprecord(XDR *);
extern bool_t xdrrec_eof(XDR *);
#else
extern void xdrmbuf_init(XDR *, struct mbuf *, enum xdr_op);
#endif /* !_KERNEL*/
#endif /* _NO_PROTO */
#endif /*!_RPC_XDR_H*/
