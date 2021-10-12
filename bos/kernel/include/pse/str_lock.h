/* @(#)01       1.2  src/bos/kernel/include/pse/str_lock.h, sysxpse, bos411, 9428A410j 10/4/93 09:00:01 */
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * ORIGINS: 83
 * 
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef	_STR_LOCK_H
#define _STR_LOCK_H

#include <sys/lockl.h>
#include <net/net_globals.h>

#define simple_lock_addr(lock)          (&(lock)

#define	EMPTY_FIELD
#define decl_simple_lock_data(class,name)	class simple_lock_data_t name;

#include <sys/i_machine.h>
#define splstr()	i_disable(INTMAX)

#define DISABLE_LOCK_DECL	int _ssavpri;
#define DISABLE_LOCK(l)    do {                          \
        _ssavpri = disable_lock(INTMAX, (l));            \
} while (0)
#define DISABLE_UNLOCK(l)  	unlock_enable(_ssavpri, (l))

#define SIMPLE_LOCK(l)		simple_lock(l)
#define SIMPLE_UNLOCK(l)	simple_unlock(l)

#define LOCK_QUEUE(sqh)   DISABLE_LOCK(&((sqh)->sqh_lock))
#define UNLOCK_QUEUE(sqh) DISABLE_UNLOCK(&((sqh)->sqh_lock))

#endif	/* _STR_LOCK_H */
