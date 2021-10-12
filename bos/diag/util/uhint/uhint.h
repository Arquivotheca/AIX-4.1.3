/* @(#)44	1.2  src/bos/diag/util/uhint/uhint.h, dutil, bos411, 9428A410j 10/5/93 08:52:51 */
/*
 *   COMPONENT_NAME: DUTIL
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/stat.h> 
#include "uhint_msg.h"

struct ceread{
	char path[512];
	struct stat info;
	};

#define MAXFILES 100
#define CAT_NAME MF_UHINT
#define SELECT 1
#define DISPLAYCE 2
#define SELMENU 0x802010
#define DISPCE  0x802011

void int_hand(int);
void all_init();
void close_all();
int select_readme(struct dirent *);
int disp_menu(int,int);
char * read_file(int);
