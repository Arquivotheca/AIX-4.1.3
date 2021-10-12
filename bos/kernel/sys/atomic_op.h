/* @(#)85	1.4  src/bos/kernel/sys/atomic_op.h, sysproc, bos411, 9428A410j 3/29/94 10:47:44 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#ifndef _H_ATOMIC_OP
#define _H_ATOMIC_OP

#include <sys/types.h>

typedef int	*atomic_p;
typedef ushort	*atomic_h;

#ifdef _NO_PROTO

int  fetch_and_add();
uint fetch_and_and();
uint fetch_and_or();
boolean_t compare_and_swap();

#ifdef _KERNEL
ushort fetch_and_add_h();
boolean_t test_and_set();
int fetch_and_nop();
#endif

#else /* _NO_PROTO */

int  fetch_and_add(atomic_p,int);
uint fetch_and_and(atomic_p,uint);
uint fetch_and_or(atomic_p,uint);
boolean_t compare_and_swap(atomic_p,int *,int);

#ifdef _KERNEL
ushort fetch_and_add_h(atomic_h,int);
boolean_t test_and_set(atomic_p, int);
int fetch_and_nop(atomic_p);
#endif

#endif /* _NO_PROTO */

/* Atomic Locking primitives--User mode routines */

#ifndef _KERNEL
#ifdef _NO_PROTO

boolean_t _check_lock();
void _clear_lock();
int _safe_fetch();

#else /* _NO_PROTO */

boolean_t _check_lock(atomic_p, int, int);
void _clear_lock(atomic_p, int);
int _safe_fetch(atomic_p);

#endif /* _NO_PROTO */

/* Atomic Locking primitive pragmas */

#pragma mc_func _clear_lock { \
 "48003403" /* bla    ._clear_lock */ \
}
#pragma mc_func _check_lock { \
 "48003423" /* bla    ._check_lock */ \
}
#pragma mc_func _safe_fetch {          \
 "80630000" /* l      r3,0(r3)      */ \
 "5463003F" /* rlinm. r3, r3, 0, -1 */ \
 "41820004" /* beq    $+4           */ \
}
#pragma reg_killed_by _safe_fetch cr0,gr3
#endif /* !_KERNEL */

#endif /* _H_ATOMIC_OP */
