/* @(#)34  1.1  src/bos/diag/da/artic/dartic_ext.h, daartic, bos41J, 9511A_all 3/3/95 11:02:39 */
/*
 *   COMPONENT_NAME: DAARTIC
 *
 *   FUNCTIONS: EXTERNL REFERENCES TO GLOBAL VARIABLES
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
extern struct tu_fru_pair
{
        short   tu;             /* test unit */
        short   fru;       /* index into ... frus[] */
} tus_test[];

extern short core_pm_frus [];

extern struct mpqp_tu_fru_pair
{
        short   tu;             /* test unit */
        short   fru;
} mpqp_pm_test[];

extern struct x25_tu_fru_pair
{
        short   tu;             /* test unit */
        short   fru;
} x25_mp_2_test[];

extern struct fru_bucket artic_frus[];
extern struct fru_bucket mpqp_frus[];
extern struct fru_bucket x25_mp2_frus[];
extern struct fru_bucket t1e1j1_frus[];

/* FRU for cable of MPQP        */
extern struct fru_bucket mpqp_cable_frus[];

/*
 *       Menus to be used by MPQP
 */




extern struct msglist have_wrap_plug[];
extern ASL_SCR_INFO asi_have_wrap_plug [];

/* menu xxxxxx --- plugging wrap plug */
extern struct msglist put_wrap_plug[];
extern ASL_SCR_INFO asi_put_wrap_plug [];

/* menu XXXXXX --- unplug  wrap plug */
extern struct msglist unplug_wrap[];
extern ASL_SCR_INFO asi_unplug_wrap[];

/* this message will ask the user if he wants to test cable     */

extern struct msglist testing_cable_t1_yes_no[];
extern ASL_SCR_INFO asi_testing_cable_t1_yes_no[];

extern struct msglist testing_cable_e1_yes_no[];
extern ASL_SCR_INFO asi_testing_cable_e1_yes_no[];

/* this message will ask the user to plug the cable to be tested in */

extern struct msglist t1_cable_plug_in[];
extern ASL_SCR_INFO asi_t1_cable_plug_in[];

extern struct msglist e1_cable_plug_in[];
extern ASL_SCR_INFO asi_e1_cable_plug_in[];

extern struct msglist unplug_wrap_and_cable[];
extern ASL_SCR_INFO asi_unplug_wrap_and_cable[];

extern struct msglist unplug_cable[];
extern ASL_SCR_INFO asi_unplug_cable[];


/* return code of test units */
extern int tu1[];

extern int tu2[];

extern int tu3[];

extern int tu4[];

extern int tu5[];

extern int tu6[];

extern int tu7[];

extern int tu8[];

extern int tu18[];

extern int tu19[];

extern int tu51[];

extern int tu52[];

extern int *tu_rctbl[];

extern int *mpqp_pm_rctbl[];
extern int *x25_mp_2_rctbl[];

extern int tu020[];
extern int tu021[];

/* -------------------------------------------------------------------- */
/*              This is for MPQP advanced tests                         */
/* -------------------------------------------------------------------- */

extern struct mpqp_internal_fru_pair
{
        short   tu;             /* test unit */
        short   fru;
} mpqp_internal_tests[];

extern struct fru_bucket mpqp_advanced_frus[];

/* return code of test units */

extern int tu99[];
extern int tu32[] ;

extern int tu33[] ;

extern int tu34[] ;

extern int tu35[] ;

extern int tu36[] ;

extern int tu37[] ;

extern int tu38[] ;

extern int tu39[] ;

extern int tu40[] ;

extern int tu41[] ;

extern int tu42[] ;

extern int tu43[] ;

extern int tu44[] ;

extern int tu45[] ;

extern int tu46[] ;

extern int tu47[] ;

extern int tu48[] ;

extern int tu49[] ;

extern int tu50[] ;
extern int tu53[] ;
extern int tu54[] ;
extern int tu55[] ;
extern int tu56[] ;
extern int tu57[] ;
extern int tu58[] ;
extern int tu59[] ;
extern int tu60[] ;

extern int *mpqp_advanced_rctbl[] ;

/* -------------------------------------------------------------------- */
/*                 End  for MPQP advanced tests                         */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*                 This is for  Portmaster advanced tests               */
/* -------------------------------------------------------------------- */


extern struct portmaster_internal_fru_pair
{
        short   tu;             /* test unit */
        short   fru;
} portmaster_internal_tests[] ;

extern int *portmaster_advanced_rctbl[] ;

extern struct fru_bucket pm_advanced_frus[] ;

/* -------------------------------------------------------------------- */
/*              This is for MP2 advanced tests                          */
/* -------------------------------------------------------------------- */

extern struct mp2_internal_fru_pair
{
        short   tu;             /* test unit */
        short   fru;
} mp2_internal_tests[];

extern struct fru_bucket mp2_advanced_frus[] ;

/* return codes of test units */

extern int tu20[];

extern int tu21[];

extern int tu22[];

extern int tu23[];

extern int tu24[];

extern int tu25[];

extern int tu26[];

extern int tu27[];

extern int tu28[];

extern int tu29[];

extern int tu30[];

extern int *mp2_advanced_rctbl[];

