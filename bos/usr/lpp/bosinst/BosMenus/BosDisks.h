/*
 * @(#) 85 1.5 src/bos/usr/lpp/bosinst/BosMenus/BosDisks.h, bosinst, bos411, 9433B411a 94/08/09 15:34:06
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

/* disk info */
struct disk_info
{
    struct disk_info *next;	/* ptr to next node             */
    char vgid[16];              /* volume goup id               */
    char level[6];              /* version/release/mod level    */
    char name[256];              /* name                         */
    char location[20];          /* location code                */
    char size[7];               /* yes, this is ascii           */
    int vgstat;                 /* 0=no vg, 1=rootvg, 2=othervg */
    int bootable;               /* boolean bootable flag        */
    int selected;               /* this disk is selected        */
    int maps;			/* maps used by this disk (mksysb) */ 
    int text_num;               /* Menu line of text            */
};

