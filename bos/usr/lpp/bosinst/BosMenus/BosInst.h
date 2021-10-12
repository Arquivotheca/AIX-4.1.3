/*
 * @(#) 86 1.6 src/bos/usr/lpp/bosinst/BosMenus/BosInst.h, bosinst, bos411, 9428A410j 94/05/31 15:05:27
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

/* BosInst.h
 *
 * This struct is a mapping of the bosinst.data file
 */
struct control_flow
{
    char CONSOLE[30];	/* console path */
    
    char INSTALL_METHOD[20];	/* install method */
    char PROMPT[4];
    char EXISTING_SYSTEM_OVERWRITE[4];
    char INSTALL_X_IF_ADAPTER[4];
    char ERROR_EXIT[72];
    char RUN_STARTUP[72];
    char RM_INST_ROOTS[72];
    char CUSTOMIZATION[80];	/* customization file */
    char TCB[4];		/* Trusted Computing Base */
    char INSTALL_TYPE[10];	/* graphics, ascii, client, personal */
    char BUNDLES[80];		/* bundles files */
} ;
struct locale
{
    char BOSINST_LANG[10];
    char CULTURAL_CONVENTION[10];
    char MESSAGES[10];
    char KEYBOARD[20];		/* keyboard map */
};
struct target_disk_data
{
    struct target_disk_data *next;	/* next target disk */
    char LOCATION[16];
    char HDISKNAME[256];
    char SIZE_MB[8];
};
struct BosInst
{
    struct control_flow control_flow;
    struct target_disk_data *targets; 
    struct target_disk_data *last;	/* next target disk */
    struct locale locale;
};

