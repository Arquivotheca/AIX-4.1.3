/* @(#)11	1.1  src/bos/usr/ccs/lib/libcfg/POWER/pt_extern.h, libcfg, bos41J, bai15 4/11/95 15:53:07 */
/*
 * COMPONENT_NAME: (LIBCFG) BUILD TABLES HEADER FILE
 *
 * FUNCTIONS: EXTERNAL FUNCTION DECLARATION & PROTOTYPES FOR
 *            COMMON BUS.out LOGGING FUNCTIONS
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/* prevent multiple inclusion */
#ifndef _H_PT_EXTERN
#define _H_PT_EXTERN

extern void log_resource_summary(int, attribute_t *, adapter_t *, int);
extern void log_message(int, char *, ...);
extern void log_error(int, int, char *, char *, char *);
extern void log_ok_value(int, attribute_t *);
extern void log_conflict(int, attribute_t *, attribute_t *);
extern void log_postpone(int, attribute_t *);
extern void log_wrap(int, attribute_t *);
extern void log_increment(int, attribute_t *, unsigned long);
extern void log_share(int, attribute_t *);
extern void log_resid_summary(int, adapter_t *);

#endif /* _H_PT_EXTERN */




