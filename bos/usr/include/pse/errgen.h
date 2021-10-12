/*
 *   COMPONENT_NAME: SYSXPSE errgen.h
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

struct errblk {
	short mid;
	short sid;
	char level;
	short flags;
	char mess[LOGMSGSZ];
	int arg1;
	int arg2;
	int arg3;
};
