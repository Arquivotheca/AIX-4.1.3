/* @(#)12	1.1  src/bos/usr/include/sys/lc_layoutP.h, libi18n, bos411, 9428A410j 8/25/93 11:06:57 */
/*
 *   COMPONENT_NAME: LIBI18N
 *
 *   FUNCTIONS: None
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _Lc_LayoutP_
#define _Lc_LayoutP_

#include <sys/lc_core.h>

#define _LC_LAYOUT 1

/**********************************************************************/
/*  Structure LayoutObjectP contains variables to hold the addresses of
/*  all the API calls that are exported by libi18n.a .
/*  The initialization routine of the library (LayoutOpen) loads
/*  the relevant  module from LOCPATH, depending on the
/*  locale specified by the application programmer, then it executes
/*  the initialization routine of that module. (The address of	
/*  the initilization routine of the module is found in variable
/*  "init" of structure LC_struct that is returned from lc_load.)
/*  The initilization routine of the module allocates a structure
/*  of type La, fills it and returns a pointer to it. This pointer
/*  must be passed to all subsequent API calls. Finally a call to
/*  LayoutClose will free this structure.
/*  The initialization routine of the module fills the LayoutObjectP
/*  structure as follows : It sets the address of all the API calls, so that 
/*  later when an API call is requested from the library, all the library does
/*  is to call the address of that function. 
/**********************************************************************/
typedef struct __lc_layout_methods {
    struct __lc_layout *(*Open)();            
    int (*Transform)();
    int (*EditShape)();
    int (*wcsTransform)();
    int (*wcsEditShape)();
    int (*ShapeBoxChars)();
    int (*SetValues)();
    int (*GetValues)();
    int (*Close)();            
    } LayoutMethodsRec, *LayoutMethods;


typedef struct {
    struct _lc_layout* class;
    BooleanValue Bidirection;
    BooleanValue ShapeEditing;
    char *Values;
    _LC_object_t *locale;
    char *extension;
    } LayoutObjectCoreRec, *LayoutObjectCore;

typedef struct {
    LayoutObjectCore core;
    LayoutMethods methods;
    }LayoutObjectPRec, *LayoutObjectP;

typedef struct _lc_layout{
    _LC_object_t lc_hdr; 
    LayoutObjectP (*initialize)();            
} lc_layout;

#endif _Lc_LayoutP_
