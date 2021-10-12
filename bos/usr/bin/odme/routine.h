/* @(#)76	1.5  src/bos/usr/bin/odme/routine.h, cmdodm, bos411, 9428A410j 6/15/90 22:36:15 */

/*
 * COMPONENT_NAME: (ODME) ROUTINE.H - routine declarations
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*-----------------------------------------------------------*/
/*  This include file defines all external routines used     */
/*-----------------------------------------------------------*/


int odminit();
int odmcreate();
int odmadd();
int odmdeleteete();
int ObjectChange();
int odmclose();
int odmopen();
int ObjectGen();
int ObjectCheckpt();
int ObjectURdo();
int odmdrop();
int odmget();
int ObjrepeatInfo();

int trcbit();
struct Class *mount_class();
struct Class *open_class();
void odmcf_perms();
int initialize_odm();
