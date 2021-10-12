/* @(#)00	1.3  src/bos/usr/bin/odmget/odmroutine.h, cmdodm, bos411, 9428A410j 6/15/90 22:34:27 */
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
#ifndef _H_ODMROUTINE
#define _H_ODMROUTINE

char *convert_to_hex_ascii();
char *get_value_from_string();
struct Class *odm_mount_class();

#endif /* _H_ODMROUTINE */
