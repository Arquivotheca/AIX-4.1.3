/* @(#)20	1.3  src/bos/usr/ccs/bin/ld/bind/objects.h, cmdld, bos411, 9428A410j 1/28/94 11:33:37 */
#ifndef Binder_OBJECTS
#define Binder_OBJECTS
/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

extern void		init_objects(void);
extern void		reserve_new_objects(int);
extern void		allocate_object_info(OBJECT *, FILHDR *, off_t);
extern void		allocate_ifile_objects(IFILE *, HEADERS *);
extern char		*get_object_file_name(OBJECT *);
extern IFILE_TYPE_T	set_ifile_type(IFILE *, HEADERS **);
extern OBJECT		*first_object(void);
extern OBJECT		*last_object(void);
extern OBJECT		*new_init_object(IFILE *);
extern OBJECT		*new_init_member_objects(IFILE *, OBJECT *, int);
extern OBJECT_TYPE_T	read_magic_number(IFILE*,HEADERS**,OBJECT*,off_t,long);

#endif /* Binder_OBJECTS */
