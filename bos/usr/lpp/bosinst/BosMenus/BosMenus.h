/*
 * @(#) 89 1.13 src/bos/usr/lpp/bosinst/BosMenus/BosMenus.h, bosinst, bos411, 9433B411a 94/08/10 13:42:38
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: include file for BosMenus
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* This include file defines the text of menus used by BOSinstall.
 * The structure definition for the menu is defined in Menus.h
 */

/* Names of various settings... */
#define PRESERVATION 	0
#define OVERWRITE	1
#define MIGRATION	2

/* menu tags defines */
#define BIM_HELP	88
#define BIM_PREV	99

struct BosCC
{
    struct CC *cc;
    short menu_index;
};

struct BosMESSAGES
{
    struct MESSAGES *msg;
    short menu_index;
};

struct BosKEYBOARD
{
    struct KEYBOARD *kb;
    short menu_index;
};

/* Language menu */
extern struct Menu *BOSLangDrvr(struct Menu *, int *); 
extern int BOSLangPre(struct Menu *);
struct Menu BOSLang = 
{
    BOSLangDrvr,
    BOSLangPre,
    24, /* len */
    -1,  /* default line */
    0,  /* animate */
    0,  /* multiple select */
    0,  /* animate string */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "    88  Help ?",
    "",
    "",
    ">>> Choice ",
    0
};

/* Fig 04 - welcome */
extern struct Menu *BOSmainDrvr(struct Menu *, int *);
extern int BOSmainPre(struct Menu *);
struct Menu BosMain = 
{
    BOSmainDrvr,
    BOSmainPre,
    24,
    5,	/* default line */
    0,
    0,
    0,  /* animate string */
    "",
    "",
    "",
    "Type the number of your choice and press Enter. Choice is indicated by >>>.",
    "",
    "    1 Start Install Now with Default Settings",
    "",
    "    2 Change/Show Installation Settings and Install",
    "",
    "    3 Start Maintenance Mode for System Recovery",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "    88  Help ?",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {0, 0, 0, BS_INSTALL_PROMPT, 0, BS_INSTALL_EASY,  /* 0-5 */
     0, BS_INSTALL, 0, BS_MAINT, 0, 0, /* 6-11 */
     0, 0, 0, 0, 0, 0,   /* 12-17 */
     0, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
};
/* Fig 04.1 - warning popup */
extern struct Menu *InstWarnDrvr(struct Menu *, int *);
extern int InstWarnPre(struct Menu *);
struct Menu InstWarn = 
{
    InstWarnDrvr,
    InstWarnPre,
    24,
    7,	/* default line */
    0,
    0,
    0,  /* animate string */
    "                 Installation Warning",
    "",
    "",
    "WARNING:  Base Operating System Installation may destroy or ",
    "impair recovery of data.  Before installing, you should back up",
    "your system.",
    "",
    "    1 Continue with Install",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "    88  Help ?",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {BS_INST_WARN, 0, 0, 0, 0, 0,  /* 0-5 */
     0, BS_INSTALL_CONT, 0, 0, 0, 0, /* 6-11 */
     0, 0, 0, 0, 0, 0,   /* 12-17 */
     0, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
};
/* Confirmation  menu Fig 06 */
extern struct Menu *ConfirmDrvr(struct Menu *, int *);
extern int ConfirmPre(struct Menu *);
struct Menu Confirm = 
{
    ConfirmDrvr,		/* driver */
    ConfirmPre,		/* preformat */
    24,		/* length  */
    17,		/* default line */
    0,		/* animate */
    0,		/* multiple select */
    0,  /* animate string */
    "                     Installation and Settings",
    "",
    "",
    "",
    "",
    "    1  System Settings:",
    "         Method of Installation.............New and Complete Overwrite",
    "         Disk Where You Want to Install.....",
    "",
    "    2  Primary Language Environment Settings (AFTER Install):",
    "         Cultural Convention................",
    "         Language...........................",
    "         Keyboard...........................",
    "",
    "",
    "    3  Install Trusted Computing Base.......",
    "",
    "    0  Install with the settings listed above.",
    "",
    "",
    "    88  Help ?",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {BS_TITLE, 0, 0, 0, 0, BS_CNFRM_SET, 	  /* 0-5 */
     BS_CNFRM_SI, BS_CNFRM_DSK, 0, BS_CNFRM_LNG, BS_CNFRM_CC, BS_CNFRM_MT, 
     BS_CNFRM_KBM, 0, 0, BS_TCB, 0, BS_CNFRM_GO,   /* 12-17 */
     0, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
    0
};

/* Change method of install Fig 07 */
extern struct Menu *ChgMethodDrvr(struct Menu *, int *);
extern int ChgMethodPre(struct Menu *);
struct Menu ChgMethod = 
{
    ChgMethodDrvr,		/* driver */
    ChgMethodPre,		/* preformat */
    24,				/* length  */
    -1,				/* default line */
    0,				/* animate */
    0,		/* multiple select */
    0,  /* animate string */
    "                     Change Method of Installation",
    "",
    "  Type the number of the installation method and press Enter.",
    "",
    "    1 New and Complete Overwrite",
    "",
    "",
    "",
    "",
    "    2 Preservation Install",
    "",
    "",
    "",
    "",
    "",
    "    3 Migration Install",
    "",
    "",
    "",
    "",
    "    88  Help ? ",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {BS_CHG_MTHD, 0, BS_CHG_MTHD_INST, 0, BS_CHG_MTHD_NEW, 0,  /* 0-5 */
     0, 0, 0, BS_CHG_MTD_PSRV, 0, 0, /* 6-11 */
     0, 0, 0, BS_CHG_MTHD_MIG, 0, 0,   /* 12-17 */
     0, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
    0
};

/* Suplemental disk - Fig 09 */
extern struct Menu *CfgSupplementalDrvr(struct Menu *, int *);
extern int CfgSupplementalPre(struct Menu *);
struct Menu CfgSupplemental = 
{
    CfgSupplementalDrvr,		/* driver */
    CfgSupplementalPre,		/* preformat */
    24,		/* length  */
    -1,		/* default line */
    0,		/* animate */
    0,		/* multiple select */
    0,  /* animate string */
    "      Support for Disks not known to Base Operating System Installation",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "    88  Help ?",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {BS_SUPLEMENTAL_TITLE, 0, 0, 0, 0, 0,  /* 0-5 */
     0, 0, 0, 0, 0, 0, /* 6-11 */
     0, 0, 0, 0, 0, 0,   /* 12-17 */
     0, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
    0
};

/* please wait.... Fig 10 */
struct Menu CfgWait = 
{
    0,		/* driver */
    0,		/* preformat */
    9,		/* length  */
    -1,		/* default line */
    1,		/* animate */
    0,		/* multiple select */
    "|/-\\",
    "",		/* 0 */
    "",		/* 1 */
    "",		/* 2 */
    "",		/* 3 */
    "",		/* 4 */
    "",		/* 5 */
    "",		/* 6 */
    "          Reading diskette.",
    "          Please Wait...",
    "                             ",
    "",		/* 10 */
    "",
    "",		/* 12 */
    "",
    "",		/* 14 */
    "",
    "",		/* 16 */
    "",
    "",		/* 18 */
    "",
    "",		/* 20 */
    "",
    "",		/* 22 */
    "",
    {0, 0, 0, 0, 0, 0,  /* 0-5 */
     0, BS_SUPLEMENTAL_READ, BS_SUPLEMENTAL_WAIT, 0, 0, 0, /* 6-11 */
     0, 0, 0, 0, 0, 0,   /* 12-17 */
     0, 0, 0, 0, 0, 0},   /* 18-23 */
};

/* change disks - rootvg ( preserve & migration) Fig 11 */
extern struct Menu *ChgRootVGDrvr(struct Menu *, int *);
extern int ChgRootVGPre(struct Menu *);
struct Menu ChgRootVG = 
{
    ChgRootVGDrvr,
    ChgRootVGPre,
    24,
    -1,
    0,
    0,		/* multiple select */
    0,  /* animate string */
    "          Change Disks Where You Want to Install",
    "",
    "Type the number for the disks to be used for installation and press Enter.",
    "",
    "       Level    Disks In Rootvg       Location Code      Size(MB)",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "    88  Help ?",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {BS_CHGDSK_TITLE, 0, BS_CHGDSK_INST, 0, BS_CHGDSK_HDG, 0,  /* 0-5 */
     0, 0, 0, 0, 0, 0, /* 6-11 */
     0, 0, 0, 0, 0, 0,   /* 12-17 */
     0, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
    0
};

/* Fig 11.5 -15 - Primary language environment */
extern struct Menu *NLSMenuDrvr(struct Menu *, int *);
extern int NLSMenuPre(struct Menu *);
struct Menu NLSMenu = 
{
    NLSMenuDrvr,
    NLSMenuPre,
    24,		/* length */
    -1,		/* default line */
    0,		/* animate  */
    0,		/* multiple select */
    (char *)0,  /* animate string */
    "                 Set Primary Language Environment",
    "",
    "",
    "",
    "",
    "",
    "        Cultural Convention    Language               Keyboard ",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "    88  Help ?",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {BS_NLS_TITLE, 0, 0, 0, 0, 0,  /* 0-5 */
     BS_NLS_HDG, 0, 0, 0, 0, 0, /* 6-11 */
     0, 0, 0, 0, 0, 0,   /* 12-17 */
     0, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
    0
};

/* Cultural Convention Fig 16 */
extern struct Menu *NLSCultureDrvr(struct Menu *, int *);
extern int NLSCulturePre(struct Menu *);
struct Menu NLSCulture = 
{
    NLSCultureDrvr,
    NLSCulturePre,
    24,
    9,
    0,
    0,		/* multiple select */
    0,          /* animate string  */
    "                   Set Primary Cultural Convention",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "    88  Help ?",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {BS_CC_TITLE, 0, 0, 0, 0, 0,  /* 0-5 */
     0, 0, 0, 0, 0, 0, /* 6-11 */
     0, 0, 0, 0, 0, 0,   /* 12-17 */
     0, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
    0
};
/* Default message translation  Fig 18?*/
extern struct Menu *NLSMsgTransDrvr(struct Menu *, int *);
extern int NLSMsgTransPre(struct Menu *);
struct Menu NLSMsgTrans = 
{
    NLSMsgTransDrvr,
    NLSMsgTransPre,
    24,
    9,
    0,
    0,		/* multiple select */
    0,          /* animate string  */
    "                   Set Primary Language ",
    "",
    "Type the number for the Primary Language for this system and press Enter.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "    88  Help ?",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {BS_MSG_TITLE, 0, BS_MSG_INST, 0, 0, 0,  /* 0-5 */
     0, 0, 0, 0, 0, 0, /* 6-11 */
     0, 0, 0, 0, 0, 0,   /* 12-17 */
     0, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
    0
};
/* Keyboard Map Fig 21 */
extern struct Menu *NLSKeyboardDrvr(struct Menu *, int *);
extern int NLSKeyboardPre(struct Menu *);
struct Menu NLSKeyboard = 
{
    NLSKeyboardDrvr,
    NLSKeyboardPre,
    24,
    9,
    0,
    0,		/* multiple select */
    0,          /* animate string  */
    "                   Set Keyboard ",
    "",
    "Type the number for the Keyboard for this system and press Enter.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "    88  Help ?",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {BS_KB_TITLE, 0, BS_KB_INST, 0, 0, 0,  /* 0-5 */
     0, 0, 0, 0, 0, 0, /* 6-11 */
     0, 0, 0, 0, 0, 0,   /* 12-17 */
     0, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
    0
};
/* Keyboard id Fig 21 */
extern struct Menu *NLSKeyboardIDDrvr(struct Menu *, int *);
extern int NLSKeyboardIDPre(struct Menu *);
struct Menu NLSKeyboardID = 
{
    NLSKeyboardIDDrvr,
    NLSKeyboardIDPre,
    24,
    5,
    0,
    0,		/* multiple select */
    0,          /* animate string  */
    "",		/* Dynamic - KB type title */
    "",
    "",         /* Dynamic - instruction line */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "    88  Help ?",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {0, 0, 0, 0, 0, 0,  /* 0-5 */
     0, 0, 0, 0, 0, 0, /* 6-11 */
     0, 0, 0, 0, 0, 0,   /* 12-17 */
     0, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
    0
};

/* mksysb Confirmation  Fig 27 */
extern struct Menu *MksysbConfirmDrvr(struct Menu *, int *);
extern int MksysbConfirmPre(struct Menu *);
struct Menu MksysbConfirm = 
{
    MksysbConfirmDrvr,		/* driver */
    MksysbConfirmPre,		/* preformat */
    24,		/* length  */
    12,		/* default line */
    0,		/* animate */
    0,		/* multiple select */
    0,          /* animate string  */
    "                   System Backup Installation and Settings",
    "",
    "",
    "",
    "",
    "      Setting:                                 Current Choice(s):",
    "",
    "    1 Disk(s) where you want to install ......",
    "         Use Maps.............................",
    "    2 Shrink File Systems.....................",
    "",
    "",
    "    0 Install with the settings listed above.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "    88  Help ?",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {BS_MKSYSB_CONFIRM_TITLE, 0, 0, 0, 0, BS_MKSYSB_CONFIRM_HDG,  /* 0-5 */
     0, BS_MKSYSB_CONFIRM_DSK, BS_MKSYSB_CONFIRM_MAP, BS_MKSYSB_CONFIRM_SHRK, 0, 0, /* 6-11 */
      BS_MKSYSB_CONFIRM_GO, 0, 0, 0, 0,0,   /* 12-17 */
     0, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
    0
};

/* mksysb disk selection - Fig 28 */
extern struct Menu *MksysbChoiceDrvr(struct Menu *, int *);
extern int MksysbChoicePre(struct Menu *);
/* Select Source Disk */
struct Menu MksysbChoice = 
{
    MksysbChoiceDrvr,		/* driver */
    MksysbChoicePre,		/* preformat */
    24,		/* length  */
    16,		/* default line */
    0,		/* animate */
    1,		/* multiple select */
    0,          /* animate string  */
    "                  Change Disk(s) Where You Want to Install",
    "",
    "",
    "",
    "",
    "",
    "",
    "        Name     Location Code    Size(MB)   VG Status   Bootable   Maps",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "     0   Continue with choices indicated above",
    "",
    "    66  Disks not known to Base Operating System Installation",
    "",
    "    88  Help ?",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {BS_MKSYSB_DSK_TITLE, 0, 0, 0, 0, 0,  /* 0-5 */
     0, BS_MKSYSB_DSK_HDG, 0, 0, 0, 0, /* 6-11 */
     0, 0, 0, 0, BS_MKSYSB_DSK_GO, 0,   /* 12-17 */
     BS_CFGDSK, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
    0
};

/* mksysb maps dialog - Fig 28.5 */
extern struct Menu *MksysbMapDrvr(struct Menu *, int *);
extern int MksysbMapPre(struct Menu *);
struct Menu MksysbMap = 
{
    MksysbMapDrvr,		/* driver */
    MksysbMapPre,		/* preformat */
    24,		/* length  */
    7,		/* default line */
    0,		/* animate */
    0,		/* multiple select */
    0,          /* animate string  */
    "                      Use Maps",
    "",
    "",
    "",
    "",
    " Type the number of your choice and press Enter.",
    "",
    "    1  Use maps for installation",
    "",
    "    2  Do not use maps for installation",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "    88  Help ?",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {BS_MAPS_TITLE, 0, 0, 0, 0, BS_MAPS_PROMPT,  /* 0-5 */
     0, BS_MAPS_YES, 0, BS_MAPS_NO, 0, 0, /* 6-11 */
     0, 0, 0, 0, 0, 0,   /* 12-17 */
     0, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
    0
};

/* Maintenance utilites  Fig 29 */
extern struct Menu *MaintenanceDrvr(struct Menu *, int *);
extern int MaintenancePre(struct Menu *);
struct Menu Maintenance = 
{
    MaintenanceDrvr,		/* driver */
    MaintenancePre,		/* preformat */
    24,		/* length  */
    4,		/* default line */
    0,		/* animate */
    0,		/* multiple select */
    0,          /* animate string  */
    "                      Maintenance ",
    "",
    "Type the number of your choice and press Enter.",
    "",
    "    1 Access a Root Volume Group ",
    "    2 Copy a System Dump to Removable Media",
    "    3 Access Advanced Maintenance Functions",
    "    4 Install from a System Backup",
    "",
    ""
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "    88  Help ?",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {BS_MAINT_TITLE, 0,BS_MAINT_INST, 0, BS_MAINT_RVG, BS_MAINT_DUMP,  /* 0-5 */
     BS_MAINT_SHELL, BS_MAINT_MKSYSB, 0, 0, 0, 0, /* 6-11 */
     0, 0, 0, 0, 0, 0,   /* 12-17 */
     0, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
    0
};
/* Disk warning Fig 30 */
extern struct Menu *RVGWarnDrvr(struct Menu *, int *);
extern int RVGWarnPre(struct Menu *);
struct Menu RVGWarn = 
{
    RVGWarnDrvr,		/* driver */
    RVGWarnPre,		/* preformat */
    24,		/* length  */
    21,		/* default line */
    0,		/* animate */
    0,		/* multiple select */
    0,          /* animate string  */
    "                     Warning:",
    "",
    "",
    "",
    "",
    "",
    "Type the number of your choice and press Enter.",
    "",
    "    0 Continue",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "    88  Help ?",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {BS_ROOTVG_WARN_TITLE, 0, 0, 0, 0, 0,  /* 0-5 */
     BS_MAINT_INST, 0, BS_ROOTVG_GO, 0, 0, 0, /* 6-11 */
     0, 0, 0, 0, 0, 0,   /* 12-17 */
     0, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
    0
};
/* Shell Info Fig 37 */
extern struct Menu *ShellInfoDrvr(struct Menu *, int *);
extern int ShellInfoPre(struct Menu *);
struct Menu ShellInfo = 
{
    ShellInfoDrvr,		/* driver */
    ShellInfoPre,		/* preformat */
    24,		/* length  */
    10,		/* default line */
    0,		/* animate */
    0,		/* multiple select */
    0,          /* animate string  */
    "               Information for Advanced Maintenance Functions",
    "",
    "-------------------------------------------------------------------------------",
    "",
    "",
    "",
    "-------------------------------------------------------------------------------",
    "",
    " Type the number of your choice and press Enter.",
    "",
    "    0 Enter the Limited Function Maintenance Shell",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "    88  Help ?",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {BS_SHELL_TITLE, 0, 0, 0, 0, 0,  /* 0-5 */
     0, 0, BS_SHELL_PROMPT, 0, BS_SHELL_GO, 0, /* 6-11 */
     0, 0, 0, 0, 0, 0,   /* 12-17 */
     0, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
    0
};

/* Mksysb tape drive selection - Fig 39 */
extern struct Menu *MksysbTapeDrvr(struct Menu *, int *);
extern int MksysbTapePre(struct Menu *);
/* Select Source Tape*/
struct Menu MksysbTape = 
{
    MksysbTapeDrvr,		/* driver */
    MksysbTapePre,
    24,		/* length  */
    -1,		/* default line */
    0,		/* animate */
    0,		/* multiple select */
    0,          /* animate string  */
    "               Choose Tape Drive",
    "",
    "",
    "",
    "",
    "      Tape Drive                     Path Name",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "    88  Help ?",
    "    99  Previous Menu",
    "",
    ">>> Choice ",
    {BS_MKSYSB_TAPE_TITLE, 0, 0, 0, 0, BS_MKSYSB_TAPE_HDG,  /* 0-5 */
     0, 0, 0, 0, 0, 0, /* 6-11 */
     0, 0, 0, 0, 0, 0,   /* 12-17 */
     0, 0, BS_HELP, BS_PREV_MENU, 0, BS_CHOICE},   /* 18-23 */
    0
};
