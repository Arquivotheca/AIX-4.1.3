/* @(#)17       1.1  src/bos/usr/bin/odmadd/odmtrace.h, cmdodm, bos411, 9428A410j 9/12/89 15:04:17 */

/*
 * COMPONENT_NAME: odmcmd.h
 *
 * ORIGIN: IBM
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */
/* Include file to turn on the trace output */

extern int odmtrace;
#ifdef DEBUG
#define TRC(a,b,c,d,e) if(odmtrace)  print_odm_trace(a,b,c,d,e);
#else
#define TRC(a,b,c,d,e)
#endif
