/* @(#)15	1.4  src/bos/usr/bin/ate/menu.h, cmdate, bos411, 9437A411a 8/26/94 16:03:20 */
/* 
 * COMPONENT_NAME: BOS menu.h
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 9  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


struct list{
	int len;
	char **items;
};

extern struct list *mmenu, *altrmenu, *modmenu, *alterdesc, *moddesc; 
extern char *off, *on;

#define MAIN_LEN	11
#define ALTER_LEN	11
#define MOD_LEN		 7
