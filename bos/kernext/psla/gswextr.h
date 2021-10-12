/* @(#)11	1.9  10/12/93 11:37:38 */
/*
 *   COMPONENT_NAME: SYSXPSLA
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/************************************************************************/
/* PURPOSE:     Device driver extern routines                           */
/*                                                                      */
/*;bb 030690    Add init_free(), remove init_vars(),getport().          */
/*;bb 032090    Eliminated gswdefine().                                 */
/*                                                                      */
/************************************************************************/


/* from gsw13.c */
extern int    gswconfig();
extern int    gswopen(),gswclose(),gswread(),gswwrite(),gswioctl();

/* from gsw23.c */
extern int    gswio();
extern int    gfil_ccb();
extern char  *gget_spac();
extern int    gfree_spac();
extern q_qel *gget_qel();
extern q_qel *gfnd_qel();
extern IQT   *gfnd_iqt();
extern void   gfil_iqt();
extern void   gcln_iqt();
extern int    gint_io();
extern IQT   *gfnd_qual();
extern int    gcln_intr();
extern int    gfil_gcb();
extern int    gopn_gsw();
extern int    gio_ioctl();
extern void   gerr_log();
extern int    gopn_x();
extern int    loadpos(),loaducode();
extern void   gswtimer();
extern void   init_free();
extern int    com_pin(), com_unpin();
extern void   acc_regs();
extern int    usrunpin();
#ifdef FPGICOMPILE
extern int    fpgibufin(),fpgiclose();
#endif FPGICOMPILE

/* from gsw33.c */
extern int    gswintr();
extern void   ipl_start(), ipl_msla();
extern int    setup_sio();
extern void   fpsetup_sio();
extern void   stat_accp(), send_uns(), send_sol(),fpsend_sol();
extern void   stop_strt();
extern int    get_sense();
extern void   com_errlog();
extern void   cancel_sio();
extern int    undo_dma();
extern void   do_dma();

/* from the system */
extern void   errsave();
extern void   copyin();
extern void   copyout();
extern void   wakeup();
extern int    nodev();

