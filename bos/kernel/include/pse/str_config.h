/* @(#)96       1.2  src/bos/kernel/include/pse/str_config.h, sysxpse, bos412, 9446B 11/9/94 16:36:16 */
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * ORIGINS: 63, 71, 83
 * 
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
 */
/** Copyright (c) 1988  Mentat Inc.
 **/

/*
 * STREAMS configuration entry point in/out data structures
 */
#ifndef _STR_CONFIG_H
#define _STR_CONFIG_H

typedef struct str_config {
        uint    sc_version;
        uint    sc_sa_flags;
        char    sc_sa_name[FMNAMESZ+1];
        dev_t   sc_devnum;
} str_config_t;

/*
 * Values for sa_flags (str_config and streamadm)
 */
#define STR_TYPE_MASK   0x00000003
#define STR_IS_DEVICE   0x00000001      /* device */
#define STR_IS_MODULE   0x00000002      /* module */
#define STR_SYSV4_OPEN  0x00000100      /* V.4 open signature/return */
#define STR_QSAFETY     0x00000200      /* Module needs safe callbacks */
#define STR_IS_MPSAFE	0x00000400	/* Module is MP safe or MP efficient */
#define STR_NOTTOSPEC	0x00000800	/* Modules service routine may sleep or fault and
					   must be scheduled. */

struct streamadm {
        uint    sa_version;
        uint    sa_flags;
        char    sa_name[FMNAMESZ+1];
        caddr_t sa_ttys;
        uint    sa_sync_level;
        caddr_t sa_sync_info;
};

extern  dev_t           strmod_add(dev_t, struct streamtab *,
                                struct streamadm *);
extern  int             strmod_del(dev_t, struct streamtab *,
                                struct streamadm *);

#endif /* _STR_CONFIG_H */
