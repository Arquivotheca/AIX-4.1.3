/* @(#)88	1.4  src/bos/kernext/x25/jsmbuf.h, sysxx25, bos411, 9428A410j 1/10/94 16:52:51 */
/*
 * COMPONENT_NAME: (SYSXX25) X.25 Device handler module
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM corp.
 */
#ifndef _H_JSMBUF
#define _H_JSMBUF

#define BSD_INCLUDES                              /* Pick up CLBYTES         */
#include <sys/types.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/uio.h>

#ifdef QLLC
#include <x25/jsdefs.h>
#include <x25/jsxbuf.h>
#else
#include "jsdefs.h"
#include "jsxbuf.h"
#endif

typedef struct mbuf mbuf_t;

#define NULL_MBUF_PTR ((mbuf_t *)NULL)
#define MAX_MBUF_SIZE ((unsigned)CLBYTES)
#define BIG_MBUF_SIZE MAX_MBUF_SIZE
#define SMALL_MBUF_SIZE ((unsigned)(MLEN))
#define NULL_LIST_PTR (NULL)
#define NULL_LIST ((jsmlist_t *)NULL)
#define READ_NEXT_JSMLIST_ELEMENT(p) ((p)->m_act)
#define SET_NEXT_JSMLIST_ELEMENT(p,val) ((p)->m_act=(val))

struct intl_jsmlist
{
  mbuf_t *first;       /* Pointer to first element in the list               */
  mbuf_t *last;        /* Pointer to last element in the list                */
};

typedef struct intl_jsmlist jsmlist_t;
typedef mbuf_t *jsmlist_iterate_t;

/*  Constants required for list and search functions                         */
enum intl_jsl_rc_t
{
  jsl_found,
  jsl_not_found
};
typedef enum intl_jsl_rc_t jsl_rc_t;


#define READ_MBUF_LENGTH(p) ((p)->m_len)
#define SET_MBUF_LENGTH(p,val) ((p)->m_len=(val))
#define READ_MAX_MBUF_LENGTH(p) ((M_HASCL(p))?BIG_MBUF_SIZE:SMALL_MBUF_SIZE)
#define READ_MBUF_DATA(p) ((p)->m_data)
#define SET_MBUF_DATA(p,val) ((p)->m_data=(val))
#define JSMBUF_MALLOC(size) (x25smbuf_malloc(size))
#define JSMBUF_WMALLOC(size) (x25smbuf_wmalloc(size))
#define JSMBUF_FREE(mbuf_ptr) (x25smbuf_free(mbuf_ptr))
#define JSMBUF_CAT(to,extra) (x25smbuf_cat((to),(extra)))

/*****************************************************************************/
/* The first set of macros are low-level accesses to the M buf structures.   */
/* They provide access to bytes, half-words and words inside an mbuf         */
/*****************************************************************************/
#define JSMBUF_READ_BYTE(mbuf_ptr,offset,type)\
  ((type)x25smbuf_read_byte((mbuf_ptr),(offset)))
#define JSMBUF_READ_HALF_WORD(mbuf_ptr,offset,type)\
  ((type)x25smbuf_read_half_word((mbuf_ptr),(offset)))
#define JSMBUF_READ_WORD(mbuf_ptr,offset,type)\
  ((type)x25smbuf_read_word((mbuf_ptr),(offset)))
#define JSMBUF_READ_BLOCK(mbuf_ptr,offset,from,length)\
  (x25smbuf_read_block((mbuf_ptr),(offset),(from),(length)))

/*****************************************************************************/
/* Now a set of macros to set values within an Mbuf                          */
/*****************************************************************************/
#define JSMBUF_SET_BYTE(mbuf_ptr,offset,value)\
  (x25smbuf_set_byte((mbuf_ptr),(offset),((byte)(value))))
#define JSMBUF_SET_HALF_WORD(mbuf_ptr,offset,value)\
  (x25smbuf_set_half_word((mbuf_ptr),(offset),((ushort)(value))))
#define JSMBUF_SET_WORD(mbuf_ptr,offset,value)\
  (x25smbuf_set_word((mbuf_ptr),(offset),((unsigned)(value))))
#define JSMBUF_SET_BLOCK(mbuf_ptr,offset,from,length)\
  (x25smbuf_set_block((mbuf_ptr),(offset),(from),(length)))

/*****************************************************************************/
/* Finally, a macro to find the address of a value within an Mbuf.  Beware   */
/* when using this one as you now have unchecked access to the Mbuf          */
/*****************************************************************************/
#define JSMBUF_ADDRESS(mbuf_ptr,offset,type)\
  ((type)(x25smbuf_address((mbuf_ptr),(offset))))
#define JSMBUF_LENGTH(mbuf_ptr) ((x25smbuf_length((mbuf_ptr))))
#define JSMBUF_GUARANTEE_SIZE(mbuf_ptr,num_bytes)\
  x25smbuf_guarantee_size ((mbuf_ptr),(num_bytes))
#define JSMBUF_ADJUST_FORWARD(mbuf_ptr,num_bytes)\
  x25smbuf_adjust_forward ((mbuf_ptr),(num_bytes))
#define JSMBUF_TRIM(mbuf_ptr,num_bytes)\
  ((void)x25smbuf_trim((mbuf_ptr),(num_bytes)))
#define JSMBUF_COPYIN(mbuf_ptr,offset,uaddr,num_bytes)\
  (x25smbuf_copyin((mbuf_ptr),(offset),(uaddr),(num_bytes)))
#define JSMBUF_COPYOUT(mbuf_ptr,offset,uaddr,num_bytes)\
  (x25smbuf_copyout((mbuf_ptr),(offset),(uaddr),(num_bytes)))
#define JSMBUF_IOMOVEIN(mbuf_ptr,offset,uiop,num_bytes)\
  (x25smbuf_iomovein((mbuf_ptr),(offset),(uiop),(num_bytes)))
#define JSMBUF_IOMOVEOUT(mbuf_ptr,offset,uiop,num_bytes)\
  (x25smbuf_iomoveout((mbuf_ptr),(offset),(uiop),(num_bytes)))

/* Start of declarations for jsmbuf.c                                        */
#ifdef _NO_PROTO
void x25smbuf_error();
mbuf_t *x25smbuf_malloc();
void x25smbuf_free();
byte x25smbuf_read_byte();
ushort x25smbuf_read_half_word();
unsigned x25smbuf_read_word();
void x25smbuf_read_block();
void x25smbuf_set_byte();
void x25smbuf_set_half_word();
void x25smbuf_set_word();
void x25smbuf_set_block();
int x25smbuf_guarantee_size();
void x25smbuf_cat();
byte *x25smbuf_address();
unsigned x25smbuf_length();
void x25smbuf_adjust_forward();
unsigned x25smbuf_trim();
mbuf_t *x25smbuf_adjust_backward();
int x25smbuf_copyin();
int x25smbuf_copyout();
int emulate_uiomove();
int x25smbuf_iomovein();
int x25smbuf_iomoveout();
void x25smbuf_dump();
void dcheck();
byte *dmalloc();
void dfree();
void jsmlist_init();
void jsmlist_element_init();
void jsmlist_enq();
void jsmlist_add_to_front();
mbuf_t *jsmlist_deq();
jsl_rc_t jsmlist_rem_without_access();
bool jsmlist_empty();
int jsmlist_length();
jsl_rc_t jsmlist_rem();
mbuf_t *jsmlist_read_head();
mbuf_t *jsmlist_iterate_first();
mbuf_t *jsmlist_iterate_next();
#else
extern void x25smbuf_error(char *s,...);

extern mbuf_t *x25smbuf_malloc(
  unsigned size);

extern void x25smbuf_free(
  mbuf_t *x25smbuf_ptr);

extern byte x25smbuf_read_byte(
  mbuf_t *x25smbuf_ptr,
  unsigned offset);

extern ushort x25smbuf_read_half_word(
  mbuf_t *x25smbuf_ptr,
  unsigned offset);

extern unsigned x25smbuf_read_word(
  mbuf_t *x25smbuf_ptr,
  unsigned offset);

extern void x25smbuf_read_block(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  byte *block_ptr,
  unsigned num_bytes);

extern void x25smbuf_set_byte(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  byte value);

extern void x25smbuf_set_half_word(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  ushort value);

extern void x25smbuf_set_word(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  unsigned value);

extern void x25smbuf_set_block(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  byte *block_ptr,
  unsigned num_bytes);

extern int x25smbuf_guarantee_size(
  mbuf_t *x25smbuf_ptr,
  unsigned num_bytes);

extern void x25smbuf_cat(
  mbuf_t *base,
  mbuf_t *extra);

extern byte *x25smbuf_address(
  mbuf_t *x25smbuf_ptr,
  unsigned offset);

extern unsigned x25smbuf_length(
  mbuf_t *x25smbuf_ptr);

extern void x25smbuf_adjust_forward(
  mbuf_t *x25smbuf_ptr,
  unsigned bytes_to_adjust);

extern unsigned x25smbuf_trim(
  mbuf_t *x25smbuf_ptr,
  unsigned bytes_to_trim);

extern mbuf_t *x25smbuf_adjust_backward(
  mbuf_t *x25smbuf_ptr,
  unsigned bytes_to_adjust);

extern int x25smbuf_copyin(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  char *uaddr,
  unsigned num_bytes);

extern int x25smbuf_copyout(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  char *uaddr,
  unsigned num_bytes);

extern int x25smbuf_iomovein(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  struct uio *uiop,
  unsigned num_bytes);

extern int x25smbuf_iomoveout(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  struct uio *uiop,
  unsigned num_bytes);

extern void x25smbuf_dump(
  mbuf_t *x25smbuf_ptr,
  unsigned offset);

extern void dcheck(
  int n);

extern byte *dmalloc(
  unsigned size);

extern void dfree(
  char *p);

extern void jsmlist_init(
  jsmlist_t *list_addr);

extern void jsmlist_element_init(
  mbuf_t *element);

extern void jsmlist_enq(
  jsmlist_t *list_addr,
  mbuf_t *element);

extern void jsmlist_add_to_front(
  jsmlist_t *list_addr,
  mbuf_t *element);

extern mbuf_t *jsmlist_deq(
  jsmlist_t *list_addr);

extern jsl_rc_t jsmlist_rem_without_access(
  jsmlist_t *list_addr,
  mbuf_t *element,
  mbuf_t *next_link);

extern bool jsmlist_empty(
  jsmlist_t *list_addr);

extern int jsmlist_length(
  jsmlist_t *list_addr);

extern jsl_rc_t jsmlist_rem(
  jsmlist_t *list_addr,
  mbuf_t *element);

extern mbuf_t *jsmlist_read_head(
  jsmlist_t *list_addr);

extern mbuf_t *jsmlist_iterate_first(
  jsmlist_iterate_t *list_iter,
  jsmlist_t *list_addr);

extern mbuf_t *jsmlist_iterate_next(
  jsmlist_iterate_t *list_iter);

#endif /* _NO_PROTO */
/* End of declarations for jsmbuf.c                                          */

#endif
