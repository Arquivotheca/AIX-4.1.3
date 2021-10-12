/*
 * @(#) 96 1.1 src/bos/usr/lpp/bosinst/BosMenus/Menus.h, bosinst, bos411, 9428A410j 93/07/01 12:10:07
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

/* Menus.h
 * Description: defines the data structure for menus.
 *
 * Menus Conventions:
 * 	line 0 is the title line
 *	line 2-4 are for instructions
 *	line 6-23 are menu choices
 */

struct MessageBox
{
    char *Text;		/* message text - up to 3 lines, embeded newlines */
    int MsgNum;		/* message number                     */
};

struct Menu
{
    struct Menu *(*driver )(struct Menu *, int *);  /* driver to call to 
						     handle choices   */
    int (*preformat)(struct Menu *);  /* function to call to preformat menu */
    int Length;			/* lenght if menu is shorter than 24  */
    int DefaultLine;		/* menu line of default (CR) value    */
    int Animate;		/* do animation flag                  */
    int MultipleSelect;		/* allow multiple numbers enterd      */
    char *AnimationString;	/* sequence of animation characters   */
    char *Text[24];		/* Menu text                          */
    int MsgNum[24];		/* Message number for this menu       */
    struct MessageBox *Message; /* Advisory message                   */
};

