/* @(#)05       1.3  src/bos/kernel/include/pse/str_system.h, sysxpse, bos411, 9428A410j 2/1/94 08:17:30 */
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * ORIGINS: 83
 * 
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _STR_SYSTEM_H
#define _STR_SYSTEM_H

#include <sys/systm.h>
#include <sys/syspest.h>	/* ASSERT */
#include <sys/device.h>

/*
 * Defines found in OSF 1.1 and not present in AIX4.1
 */

/* 
 * fcntl.h
 */
#define      O_DOCLONE DDOCLONE    /* make a cloned device */

/* 
 * ioctl.h
 */
#define FIOFATTACH  _IOW('f', 121, void *)  /* internal: fattach */
#define FIOFDETACH  _IOW('f', 120, void *)  /* internal: fdetach */
#define FIOPIPESTAT _IOR('f', 122, struct stat)     /* pipe|fifo stat */

/*
 * param.h
 */
#ifndef MACHINE_ALIGNMENT               /* optional machine/machparam.h */
#define MACHINE_ALIGNMENT       sizeof (void *)
#endif

#define ALIGNMENT(p)    ((uint)(p) % MACHINE_ALIGNMENT)
#define ALIGN(p)        (void *)((caddr_t)(p) + MACHINE_ALIGNMENT - 1 - \
                         ALIGNMENT((caddr_t)(p) + MACHINE_ALIGNMENT - 1))
/*
 * If PIPSIZ is set to < 4096 experience shows that many applications
 * deadlock. Note that PIPE_BUF is the write atomicity limit.
 */
#if     (PIPE_BUF * 2) < 4096           /* sys/syslimits.h */
#define PIPSIZ  4096
#else
#define PIPSIZ  (PIPE_BUF * 2)
#endif

#ifdef  _KERNEL
/*
 * Macro to replace redundant SEC_BASE code.
 */
#if     SEC_BASE
#define PRIV_SUSER(priv,code,retcode,error)   \
        ((error) = !privileged((priv),(code)) ? (retcode) : 0)
#else
#define PRIV_SUSER(priv,code,retcode,error)  {\
	char	rc;			      \
        if (!suser(&(rc))) error = rc;        \
}
#endif
#endif

/*
 * cmn_err.h
 */
/*
 * STREAMS common error codes.
 */

extern  void    cmn_err(int, char *, ...);

/*
 *      level definitions for cmn_err()
 */
#define CE_CONT         0x0001          /* simple printf */
#define CE_NOTE         0x0002          /* "NOTICE:" */
#define CE_WARN         0x0003          /* "WARNING:" */
#define CE_PANIC        0x0004          /* "PANIC:" */

#endif /* _STR_SYSTEM_H */
