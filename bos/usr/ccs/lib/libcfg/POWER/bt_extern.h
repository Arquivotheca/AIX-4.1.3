/* @(#)24	1.4  src/bos/usr/ccs/lib/libcfg/POWER/bt_extern.h, libcfg, bos41J, 9520A_all 5/9/95 13:49:26 */
/*
 * COMPONENT_NAME: (LIBCFG) BUILD TABLES HEADER FILE
 *
 * FUNCTIONS: EXTERNAL FUNCTION DECLARATION & PROTOTYPES FOR
 *            COMMON BUILD TABLES FUNCTIONS
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/* prevent multiple inclusion */
#ifndef _H_BT_EXTERN
#define _H_BT_EXTERN

extern adapter_t *build_adapter_list(int *, char *, char *);
extern attribute_t *new_attribute(int *, adapter_t *, struct PdAt *, 
                                  struct CuAt *);
extern int destroy_lists(adapter_t **, attribute_t **);
extern unsigned long get_modelcode(int *);
extern value_list_t *strtovlist(int *, char *);
extern void apply_share_attributes(int *);
extern void convert_to_list(int *, attribute_t *);
extern void destroy_attribute_list(int *, attribute_t *);
extern void eliminate_value_list_element(attribute_t *, int);
extern void process_bus_resource_attributes(int *, adapter_t *, struct PdAt *, 
                                            int, struct CuAt *, int);
extern void process_modifier_attributes(int *, adapter_t *, struct PdAt *, int,
                                        struct CuAt *, int);
extern void setup_list_attribute(int *, adapter_t *, attribute_t *, char *, 
                                 bus_resource_e, unsigned long, unsigned long, 
                                 unsigned long, unsigned long);
extern void sync_group_attribute(attribute_t *);
extern void sync_list_attribute(int *, attribute_t *, int);
extern void verify_bus_resource_attributes(int *, adapter_t *, int, void());
extern void destroyvlist(int *, value_list_t *);
extern void destroyrlist(int *, value_range_t *);
extern void share_algorithm_cb_234(int *, inttbl_t *, int, attribute_t *,
                                   unsigned long *, unsigned long *);

#include <cfgresid.h> /* For CFG_bax_descriptor_t */

extern void get_bax_and_invert(int *, adapter_t *, int *, CFG_bax_descriptor_t **,
                               int *, CFG_bax_descriptor_t **);
extern void reserve_bax_range(int *, adapter_t *, CFG_bax_descriptor_t *, int);
extern void reserve_range(int *, attribute_t *, unsigned long, unsigned long, char *);
extern int build_resid_attribute_list(unsigned long, adapter_t *, char *);

#endif /* _H_BT_EXTERN */

