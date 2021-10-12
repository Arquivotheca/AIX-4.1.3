/* @(#)78	1.9  src/bos/kernel/db/POWER/vdbfmtu.h, sysdb, bos411, 9428A410j 6/5/91 09:45:23 */

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _h_FMTU
#define _h_FMTU
/*
  * user area display definitions
 */

#define Get_from_memory(a1,a2,a3,a4) 	get_put_data((a1),(a2),(a3),(a4),FALSE)
#define VIRT 	1			/* for Get_from_mem() */

#undef u_sfirst
#undef u_slast
#undef u_srw
#undef u_segbase
#undef u_segoffs
#undef u_ofirst
#undef u_olast
/* segment register defines */
#define u_sfirst(i)        u_sbounds[(i)].u_segbnds.sbu.sbu_sfirst
#define u_slast(i)         u_sbounds[(i)].u_segbnds.sbu.sbu_slast
#define u_srw(i)           u_sbounds[(i)].u_segbnds.sbu.sbu_srw
#define u_segbase(i)       u_sbounds[(i)].u_segbnds.sbs.sbs_sbase
#define u_segoffs(i)       u_sbounds[(i)].u_segbnds.sbs.sbs_segoffs
#define u_ofirst(i)        u_sbounds[(i)].u_segbnds.sbs.sbs_segoffs->so_first
#define u_olast(i)         u_sbounds[(i)].u_segbnds.sbs.sbs_segoffs->so_last
#define u_osrw(i)          u_sbounds[(i)].u_segbnds.sbs.sbs_srw
#define u_segid(i)         u_sbounds[(i)].u_segbnds.sbs.sbs_segoffs->so_segid

#define Ex	x->u_exh.u_exbhdr	/* shorthand for user.a.out structure */

/****************** DEBUG Stuff ******************/
/* #define Debug */			/* to include debug code */

/* Values for DBG_LVL */
#define DBGGENERAL	1
#define DBGSR2		2

#ifdef Debug
extern int jr_debug();
extern int DBG_LVL;
#endif /* Debug */


#endif /* h_FMTU */


/* These defines for cred structure were originally in sys/user.h */

#ifndef u_uid

#define        u_uid      u_cred->cr_uid
#define        u_ruid     u_cred->cr_ruid
#define        u_suid     u_cred->cr_suid
#define        u_luid     u_cred->cr_luid
#define        u_acctid   u_cred->cr_acctid
#define        u_gid      u_cred->cr_gid
#define        u_rgid     u_cred->cr_rgid
#define        u_sgid     u_cred->cr_sgid
#define        u_groups   u_cred->cr_groups
#define        u_epriv    u_cred->cr_epriv
#define        u_ipriv    u_cred->cr_ipriv
#define        u_bpriv    u_cred->cr_bpriv
#define        u_mpriv    u_cred->cr_mpriv

#endif

