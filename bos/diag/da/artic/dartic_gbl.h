/* @(#)35  1.1  src/bos/diag/da/artic/dartic_gbl.h, daartic, bos41J, 9511A_all 3/3/95 11:02:53 */
/*
 *   COMPONENT_NAME: DAARTIC
 *
 *   FUNCTIONS: GLOBAL VARIABLES
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

/*--------------------------------------------------------------*/
/*      Messages and menus                                      */
/*--------------------------------------------------------------*/

extern ASL_SCR_TYPE     menutypes=DM_TYPE_DEFAULTS;
char *msgstr;
char *msgstr1;

struct tu_fru_pair
{
        short   tu;             /* test unit */
        short   fru;       /* index into ... frus[] */
} tus_test[] = {
        { 01, FRU_101 },
        { 18, FRU_118 },
        { 19, FRU_119 },
        { 03, FRU_103 },
        { 04, FRU_104 },
        { 05, FRU_105 },
        { -1, -1 }
 };

short core_pm_frus [] = {7, 6, 6, 7, 7, 8, 9};

struct mpqp_tu_fru_pair
{
        short   tu;             /* test unit */
        short   fru;
} mpqp_pm_test[] = {
        { 52, FRU_152   },
        { 51, FRU_151   },
        { 8, FRU_8      },
        { -1, -1 }
 };

struct x25_tu_fru_pair
{
        short   tu;             /* test unit */
        short   fru;
} x25_mp_2_test[] = {
        { 6, FRU_6      },
        { 7, FRU_7      },
        { 8, FRU_8      },
        { -1, -1 }
 };

struct fru_bucket artic_frus[] = {
        /* fail from tu001; Power on self-test  */
        {"", FRUB1, 0x851, 0x101, ARTIC_101,
                {
                        {90, "", "", 0, DA_NAME, EXEMPT},
                        {10, "", "",  0, PARENT_NAME, NONEXEMPT},

                },
        },
        /* fail from tu018 General registers test       */
        {"", FRUB1, 0x851, 0x103, ARTIC_118,
                {
                        {90, "", "", 0, DA_NAME, EXEMPT},
                        {10, "", "",  0, PARENT_NAME, NONEXEMPT},
                },
        },
        /* fail from tu019 Adapter download diagnostics */
        {"", FRUB1, 0x851, 0x104, ARTIC_119,
                {
                        {90, "", "", 0, DA_NAME, EXEMPT},
                        {10, "", "",  0, PARENT_NAME, NONEXEMPT},
                },
        },
        /* fail from tu003 Adapter Interrupt test */
        {"", FRUB1, 0x851, 0x105, ARTIC_103,
                {
                        {90, "", "", 0, DA_NAME, EXEMPT},
                        {10, "", "",  0, PARENT_NAME, NONEXEMPT},
                },
        },
        /* fail from tu004 Advanced CPU test */
        {"", FRUB1, 0x851, 0x106, ARTIC_104,
                {
                        {90, "", "", 0, DA_NAME, EXEMPT},
                        {10, "", "",  0, PARENT_NAME, NONEXEMPT},
                },
        },
        /* fail from tu005 Advanced DRAM test */
        {"", FRUB1, 0x851, 0x107, ARTIC_105,
                {
                        {90, "", "", 0, DA_NAME, EXEMPT},
                        {10, "", "",  0, PARENT_NAME, NONEXEMPT},
                },
        },

        /* portmaster failure from tu 18 or tu 19  */
        {"", FRUB1, 0x855, 0x503, ARTIC_108,
                {
                        {90, "", "", 0, DA_NAME, EXEMPT},
                        {10, "", "",  0, PARENT_NAME, NONEXEMPT},
                },
        },
        /* portmaster failure from tu 1, 3, or 4 */
        {"", FRUB1, 0x855, 0x504, ARTIC_108,
                {
                        {100, "", "", 0, DA_NAME, EXEMPT},
                },
        },
        /* portmaster failure from tu 5 */
        {"", FRUB1, 0x855, 0x516, ARTIC_108,
                {
                       {90, " ", "", F_X25_DRAM ,
                                        NOT_IN_DB, EXEMPT},
                        {10, "", "",  0, DA_NAME, NONEXEMPT},
                },
        },
        /* portmaster failure from tu 1  (rc = 0x11, 0x12)  */
        {"", FRUB1, 0x855, 0x517, ARTIC_108,
                {
                        {90, "", "",  0, DA_NAME, NONEXEMPT},
                        {10, " ", "", F_X25_DRAM ,
                                        NOT_IN_DB, EXEMPT},
                },
        },
};
struct fru_bucket mpqp_frus[] =
{
        /* fail from tu052 DMA Busmaster Test */
        {"", FRUB1, 0x851, 0x108, ARTIC_108,
                {
                        {90, "", "", 0, DA_NAME, EXEMPT},
                        {10, "", "",  0, PARENT_NAME, NONEXEMPT},
                },
        },
        /* fail from tu051 Advanced CIO test */
        {"", FRUB1, 0x851, 0x109, ARTIC_107,
                {
                        {100, "", "", 0, DA_NAME, EXEMPT},
                },
        },

        /* fail from tu08 Serial Communication Controller test */
        {"", FRUB1, 0x851, 0x113, ARTIC_SCC,
                {
                        {100, "", "", 0, DA_NAME, EXEMPT},
                },
        }
};

struct fru_bucket x25_mp2_frus[] =
{
        /* fail from tu06  Gate Array test*/
        {"", FRUB1, 0x849, 0x114, ARTIC_GATE_ARRAY,
                {
                        {90, "", "", 0, DA_NAME, EXEMPT},
                        {10, "", "",  0, PARENT_NAME, NONEXEMPT},
                },
        },
        /* fail from tu07 Advanced CIO test */
        {"", FRUB1, 0x849, 0x109, ARTIC_107,
                {
                        {100, "", "", 0, DA_NAME, EXEMPT},
                },
        },

        /* fail from tu08 Serial Communication Controller test */
        {"", FRUB1, 0x849, 0x113, ARTIC_SCC,
                {
                        {100, "", "", 0, DA_NAME, EXEMPT},
                },
        }
};
struct fru_bucket t1e1j1_frus[] =
{
        /* fail from tu020; Adapter download diagnostics  */
        {"", FRUB1, 0x851, 0x110, ARTIC_110,
                {
                        {100, "", "", 0, DA_NAME, EXEMPT},

                },
        },
        /* fail from tu021; Adapter download diagnostics  */
        {"", FRUB1, 0x851, 0x111, ARTIC_111,
                {
                        {100, "", "", 0, DA_NAME, EXEMPT},

                },
        },
        /* fail from cable testing      */
        {"", FRUB1, 0x851, 0x112, ARTIC_112,
                {
                        {100, " ", "", ARTIC_112, NOT_IN_DB , EXEMPT},

                },
        },
        /* fail from configurration     */
        {"", FRUB1, 0x851, 0x150, ARTIC_150,
                {
                       {80, "", "", 0, DA_NAME, NONEXEMPT},
                       {10, " ", "", ARTIC_SOFTWARE_ERROR ,
                                        NOT_IN_DB, EXEMPT},
                       {10, "", "", 0, PARENT_NAME, NONEXEMPT},

                },
        },
        {"", FRUB1, 0x851, 0x151, ARTIC_151,
                {
                       {80, "", "", 0, DA_NAME, NONEXEMPT},
                       {10, " ", "", ARTIC_SOFTWARE_ERROR ,
                                        NOT_IN_DB, EXEMPT},
                       {10, "", "", 0, PARENT_NAME, NONEXEMPT},

                },
        },
        /* this fru for Cannot read VPD and there is a problem with     */
        /* the adapter                                                  */
        {"", FRUB1, 0x851, 0x152, ARTIC_152,
                {
                       {100, " ", "", T1_J1, NOT_IN_DB, EXEMPT},
                       {100, " ", "", E1, NOT_IN_DB, EXEMPT},

                },
        }
};


/* FRU for cable of MPQP        */
struct fru_bucket mpqp_cable_frus[] = {
        /* V.35 Cable is BAD            */
        {"", FRUB1, 0x855, 0x118, MPQP_V_35_CABLE_FAILURE,
                {
                        {100, " ", "", V35_cable,
                                 NOT_IN_DB, EXEMPT},

                },
        },
        {"", FRUB1, 0x855, 0x119, MPQP_X_21_CABLE_FAILURE,
                {
                        {100, " ", "", X21_cable,
                                 NOT_IN_DB, EXEMPT},

                },
        },
        {"", FRUB1, 0x855, 0x116, MPQP_RS_232_CABLE_FAILURE,
                {
                        {100, " ", "", cable_232,
                                 NOT_IN_DB, EXEMPT},

                },
        },

        {"", FRUB1, 0x855, 0x117, MPQP_RS_422_CABLE_FAILURE,
                {
                        {100, " ", "", cable_422,
                                 NOT_IN_DB, EXEMPT},

                },
        }
};
/*
*       Menus to be used by MPQP
*/




struct msglist have_wrap_plug[]=
        {
                { 1, ADVANCED_NO_STANDBY},
                { 1, YES_OPTION         },
                { 1, NO_OPTION          },
                { 1, HAVE_WRAP_PLUG     },
                {       0, 0            }
        };

ASL_SCR_INFO asi_have_wrap_plug [DIAG_NUM_ENTRIES(have_wrap_plug)];

/* menu xxxxxx --- plugging wrap plug */
struct msglist put_wrap_plug[]=
        {
                { 1, ADVANCED_NO_STANDBY},
                { 1, PLUG_WRAP          },
                { 1, FINISHED           },
                0
        };

ASL_SCR_INFO asi_put_wrap_plug [DIAG_NUM_ENTRIES(put_wrap_plug)];

/* menu XXXXXX --- unplug  wrap plug */
struct msglist unplug_wrap[]=
        {
                { 1, ADVANCED_NO_STANDBY        },
                { 1, UNPLUG_WRAP                },
                { 1, FINISHED                   },
                0
        };
ASL_SCR_INFO asi_unplug_wrap[DIAG_NUM_ENTRIES(unplug_wrap)];

/* this message will ask the user if he wants to test cable     */

struct msglist testing_cable_t1_yes_no[]=
        {
                { 1, ADVANCED_NO_STANDBY        },
                { 1, YES_OPTION                 },
                { 1, NO_OPTION                  },
                { 1, T1_CABLE_REQUIRE           },
                0
        };
ASL_SCR_INFO asi_testing_cable_t1_yes_no[DIAG_NUM_ENTRIES(testing_cable_t1_yes_no)];

struct msglist testing_cable_e1_yes_no[]=
        {
                { 1, ADVANCED_NO_STANDBY        },
                { 1, YES_OPTION                 },
                { 1, NO_OPTION                  },
                { 1, E1_CABLE_REQUIRE           },
                0
        };
ASL_SCR_INFO asi_testing_cable_e1_yes_no[DIAG_NUM_ENTRIES(testing_cable_e1_yes_no)];

/* this message will ask the user to plug the cable to be tested in */

struct msglist t1_cable_plug_in[]=
        {
                { 1, ADVANCED_NO_STANDBY        },
                { 1, PLUG_T1_CABLE              },
                { 1, FINISHED                   },
                0
        };
ASL_SCR_INFO asi_t1_cable_plug_in[DIAG_NUM_ENTRIES(t1_cable_plug_in)];

struct msglist e1_cable_plug_in[]=
        {
                { 1, ADVANCED_NO_STANDBY        },
                { 1, PLUG_E1_CABLE              },
                { 1, FINISHED                   },
                0
        };
ASL_SCR_INFO asi_e1_cable_plug_in[DIAG_NUM_ENTRIES(e1_cable_plug_in)];

struct msglist unplug_wrap_and_cable[]=
        {
                { 1, ADVANCED_NO_STANDBY        },
                { 1, UNPLUG_WRAP_AND_CABLE      },
                { 1, FINISHED                   },
                0
        };
ASL_SCR_INFO asi_unplug_wrap_and_cable[DIAG_NUM_ENTRIES(unplug_wrap_and_cable)];

struct msglist unplug_cable[]=
        {
                { 1, ADVANCED_NO_STANDBY        },
                { 1, UNPLUG_CABLE               },
                { 1, FINISHED                   },
                0
        };
ASL_SCR_INFO asi_unplug_cable[DIAG_NUM_ENTRIES(unplug_cable)];


/* return code of test units */
int tu1[] = {   0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
                0x1A, 0x341, 0x343, 0x352, 0x403, 0x405, 0x406, 0 };


int tu2[] = {   0xB1, 0 };

int tu3[] = {   0x387, 0x1, 0 };

int tu4[] = {   0x41, 0 };

int tu5[] = {   0x51, 0x52 ,0 };

int tu6[] = {   0x61, 0 };

int tu7[] = {   0x71, 0 };

int tu8[] = {   0x81, 0 };

int tu18[] = {  0x125, 0x346, 0x501, 0 };

int tu19[] = {  0xFFFE, 0 };

int tu51[] = {  0x71, 0 };

int tu52[] = {  0x61, 0x802, 0 };

int *tu_rctbl[] = {
        tu1, tu18, tu19, tu3, tu4, tu5
};

int *mpqp_pm_rctbl[] = {
        tu52, tu51, tu8
};
int *x25_mp_2_rctbl[] = {
        tu6, tu7, tu8
};

int tu020[] = {
0x131,
0 };

int tu021[] = {
0x5100, 0x5205, 0x5206,0x5301, 0x5302,
0x5401, 0x5402, 0x5403, 0x5404, 0x5405,
0x5500, 0x5502, 0x5504, 0x5506, 0x550c,
0x5600, 0x5701, 0x5702, 0x5705, 0x5709, 0x570A, 0x570B, 0x5715,
0x5717, 0x5801, 0x5802, 0x5803, 0x5804, 0x5805, 0x5806, 0x5807, 0x5808,0x5809,
0x581A, 0x571B, 0x571C, 0x571D, 0x571E, 0x571F,
0x5820, 0x5821, 0x5822, 0x5823, 0x5824, 0x5825, 0x5826, 0x5827, 0x5828, 0x5829, 0x582A, 0x582B, 0x582C, 0x582D, 0x582E, 0x582D,
0x585C, 0x5F01, 0x5F08,
0x5F48, 0x5F09, 0x5F2A, 0x5F3A, 0x5F4A, 0x5F0B,
0x5F0C, 0x5F1C, 0x5F0D, 0x5F0E, 0x5F00,
0 };

/* -------------------------------------------------------------------- */
/*              This is for MPQP advanced tests                         */
/* -------------------------------------------------------------------- */

#define         FRU_MPQP_INTERNAL       0
#define         FRU_MPQP_WRAP_PLUG_TEST 1

struct mpqp_internal_fru_pair
{
        short   tu;             /* test unit */
        short   fru;
} mpqp_internal_tests[] = {
        { 32, FRU_MPQP_INTERNAL},
        { 33, FRU_MPQP_INTERNAL},
        { 34, FRU_MPQP_INTERNAL},
        { 35, FRU_MPQP_INTERNAL},
        { 36, FRU_MPQP_INTERNAL},
        { 37, FRU_MPQP_INTERNAL},
        { 38, FRU_MPQP_INTERNAL},
        { 39, FRU_MPQP_INTERNAL},
        { 40, FRU_MPQP_INTERNAL},
        { 41, FRU_MPQP_INTERNAL},
        { -1, -1 }
 };

struct fru_bucket mpqp_advanced_frus[] =
{
        /* Fail from MPQP internal wrap tests   */

        {"", FRUB1, 0x855, 0x503, MPQP_INTERNAL_TESTS,
                {
                        {90, "", "", 0, DA_NAME, EXEMPT},
                        {10, "", "",  0, PARENT_NAME, NONEXEMPT},
                },
        },
        /* Wrap test failure            */
        {"", FRUB1, 0x855, 0x121, MPQP_78PIN_CABLE,
                {
                        {80, " ", "", MPQP_INTERFACE_CARD,
                                NOT_IN_DB, EXEMPT},
                        {15, "", "",  0, DA_NAME, NONEXEMPT},
                        { 5, " ", "", MPQP_JUMPER_CABLE,
                                NOT_IN_DB, EXEMPT},
                },
        },

};

/* return code of test units */

int tu99[] = {  0x300, 0x0 };
int tu32[] = {  0x300, 0x0 };

int tu33[] = {  0xC2, 0 };

int tu34[] = {  0xC3, 0 };

int tu35[] = {  0xC4, 0 };

int tu36[] = {  0xC5, 0 };

int tu37[] = {  0xC6, 0 };

int tu38[] = {  0xC7, 0 };

int tu39[] = {  0xC8, 0 };

int tu40[] = {  0xC9, 0 };

int tu41[] = {  0xCA, 0 };

int tu42[] = {  0xC2, 0 };

int tu43[] = {  0xC3, 0 };

int tu44[] = {  0xC4, 0 };

int tu45[] = {  0xC5, 0 };

int tu46[] = {  0xC6, 0 };

int tu47[] = {  0xC7, 0 };

int tu48[] = {  0xC8, 0 };

int tu49[] = {  0xC9, 0 };

int tu50[] = {  0xCA, 0 };
int tu53[] = {  0xB2, 0 };
int tu54[] = {  0xB3, 0 };
int tu55[] = {  0xB4, 0 };
int tu56[] = {  0xB5, 0 };
int tu57[] = {  0xB6, 0 };
int tu58[] = {  0xB7, 0 };
int tu59[] = {  0xB8, 0 };
int tu60[] = {  0xB9, 0 };

int *mpqp_advanced_rctbl[] = {  tu32, tu33, tu34, tu35, tu36,
                                tu37, tu38, tu39, tu40, tu41 };

/* -------------------------------------------------------------------- */
/*                 End  for MPQP advanced tests                         */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*                 This is for  Portmaster advanced tests               */
/* -------------------------------------------------------------------- */


struct portmaster_internal_fru_pair
{
        short   tu;             /* test unit */
        short   fru;
} portmaster_internal_tests[] = {
        { 53, FRU_MPQP_INTERNAL},
        { 54, FRU_MPQP_INTERNAL},
        { 55, FRU_MPQP_INTERNAL},
        { 56, FRU_MPQP_INTERNAL},
        { 57, FRU_MPQP_INTERNAL},
        { 58, FRU_MPQP_INTERNAL},
        { 59, FRU_MPQP_INTERNAL},
        { 60, FRU_MPQP_INTERNAL},
        { -1, -1 }
 };

int *portmaster_advanced_rctbl[] = {    tu53, tu54, tu55, tu56, tu57,
                                        tu58, tu59, tu60 };

struct fru_bucket pm_advanced_frus[] =
{
        {"", FRUB1, 0x855, 0x507, ARTIC_108,
                {
                        {90, " ", "", EIB_6P_V35,
                                NOT_IN_DB, EXEMPT},
                        {10, "", "", 0, DA_NAME, EXEMPT},
                },
        },
        {"", FRUB1, 0x855, 0x508, ARTIC_108,
                {
                        {90, " ", "", EIB_6P_X21,
                                NOT_IN_DB, EXEMPT},
                        {10, "", "", 0, DA_NAME, EXEMPT},
                },
        },
        {"", FRUB1, 0x855, 0x505, ARTIC_108,
                {
                        {80, " ", "", EIB_8P_232,
                                NOT_IN_DB, EXEMPT},
                        {20, "", "", 0, DA_NAME, EXEMPT},
                },
        },
        {"", FRUB1, 0x855, 0x506, ARTIC_108,
                {
                        {90, " ", "", EIB_8P_422,
                                NOT_IN_DB, EXEMPT},
                        {10, "", "", 0, DA_NAME, EXEMPT},
                },
        }
};
/* -------------------------------------------------------------------- */
/*              This is for MP2 advanced tests                          */
/* -------------------------------------------------------------------- */

struct mp2_internal_fru_pair
{
        short   tu;             /* test unit */
        short   fru;
} mp2_internal_tests[] = {
        { 20, 0},
        { 21, 0},
        { 22, 0},
        { 23, 0},
        { 24, 0},
        { 25, 0},
        { 26, 0},
        { 27, 0},
        { 28, 0},
        { 29, 0},
        { 30, 0},
        { -1, -1 }
 };

struct fru_bucket mp2_advanced_frus[] =
{
      {"", FRUB1, 0x849, 0x511, ARTIC_108,
                {
                        {90, " ", "", MP2_EIB,
                                NOT_IN_DB, EXEMPT},
                        {10, "", "", 0, DA_NAME, EXEMPT},
                },
        },

      {"", FRUB1, 0x849, 0x514, ARTIC_108,
                {
                        {90, " ", "", MP2_EIB,
                                NOT_IN_DB, EXEMPT},
                        {10, "", "", 0, DA_NAME, EXEMPT},
                },
        },

      {"", FRUB1, 0x849, 0x512, ARTIC_108,
                {
                        {90, " ", "", MP2_EIB,
                                NOT_IN_DB, EXEMPT},
                        {10, "", "", 0, DA_NAME, EXEMPT},
                },
        },
      {"", FRUB1, 0x849, 0x515, ARTIC_108,
                {
                        {90, " ", "", MP2_EIB,
                                NOT_IN_DB, EXEMPT},
                        {10, "", "", 0, DA_NAME, EXEMPT},
                },
        },
      {"", FRUB1, 0x849, 0x513, ARTIC_108,
                {
                        {90, " ", "", MP2_EIB,
                                NOT_IN_DB, EXEMPT},
                        {10, "", "", 0, DA_NAME, EXEMPT},
                },
        },

        {"", FRUB1, 0x849, 0x102, ARTIC_GATE_ARRAY,
                {
                        {100, "", "", 0, DA_NAME, EXEMPT},
                },
        },

};

/* return code of test units */

int tu20[] = {  0xA4, 0 };

int tu21[] = {  0xA5, 0 };

int tu22[] = {  0x701, 0x702, 0xB2, 0 };

int tu23[] = {  0x701, 0x702, 0xB2, 0 };

int tu24[] = {  0xB4, 0 };

int tu25[] = {  0xB5, 0 };

int tu26[] = {  0xB6, 0 };

int tu27[] = {  0xB7, 0 };

int tu28[] = {  0xB8, 0 };

int tu29[] = {  0xB9, 0 };

int tu30[] = {  0x135, 0x701, 0x702, 0 };

int *mp2_advanced_rctbl[] = {   tu20, tu21, tu22, tu23, tu24, tu25,
                                tu26, tu27, tu28, tu29, tu30};

