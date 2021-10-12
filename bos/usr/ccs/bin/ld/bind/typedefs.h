/* @(#)27	1.4  src/bos/usr/ccs/bin/ld/bind/typedefs.h, cmdld, bos411, 9431A411a 8/1/94 16:43:13 */
#ifndef Binder_TYPEDEFS
#define Binder_TYPEDEFS
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

typedef int RETCODE;
typedef int FILEDES;

typedef struct ifile IFILE;
typedef		struct object OBJECT;
typedef		struct obj_member_info OBJ_MEMBER_INFO;
typedef		struct object_info OBJECT_INFO;
typedef			struct section SECTION;
typedef			struct srcfile SRCFILE;
typedef				struct csect CSECT;
typedef					struct symbol SYMBOL;
typedef					struct rld RLD;

typedef struct item	ITEM;
typedef union {
    int	heap_index;
    CSECT *csect;
} *CSECT_HEAP;

typedef struct typechk	TYPECHK;
typedef struct str	STR;
typedef struct hash_str	HASH_STR;

typedef int32	X_OFF_T;		/* Signed type used for file offsets
					   in an XCOFF file or for offsets
					   within parts of an XCOFF file.
					   This type can also be used for
					   the size of an XCOFF file.
					   The range of this type determines
					   the maximum XCOFF file size. */
typedef uint32	X_VADDR_T;		/* Virtual address in an XCOFF file.
					   This is an arithmetic type unrelated
					   to the size of a pointer. */

#endif /* Binder_TYPEDEFS */
