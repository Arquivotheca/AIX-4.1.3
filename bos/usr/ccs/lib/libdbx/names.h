/* @(#)65	1.2.1.1  src/bos/usr/ccs/lib/libdbx/names.h, libdbx, bos411, 9428A410j 2/19/92 14:24:24 */
#ifndef _h_names
#define _h_names
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: (macros) ident
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */
typedef struct Name *Name;

/*
 * Inline (for speed) function to return the identifier (string)
 * associated with a name.  Because C cannot support both inlines
 * and data hiding at the same time, the Name structure must be
 * publicly visible.  It is not used explicitly, however, outside of this file.
 */

struct Name {
    char *identifier;
    Name chain;
};

extern cases symcase;

#define ident(n) ((n == nil) ? "(noname)" : n->identifier)
extern Name identname(/* s, isallocated */);
extern names_free(/*  */);


/*
 * This struct records the demangled text of a C++ name.
 */

typedef struct DemangledName *DemangledName;

struct DemangledName {
    void *dName;         /* the class instance returned by the demangler */

    /* the following pointers point into the demangled name structure */
    Name name; 		 /* the function name, alone */
    String qualName;	 /* the qualified name */
    String params;	 /* the parameter list of the name */
    String shortName;    /* name || params */
    String fullName;     /* qualifier || name || params */
    Name mName;		 /* the original mangled name */
};

DemangledName Demangle(/* Name *mangledName */);
void EraseDemangledName(/* DemangledName */);

#endif /* _h_names */
