/* @(#)94	1.1  src/gos/2d/MOTIF/lib/Xm/RepType.h, libxm, gos411, 9428A410i 11/10/92 16:49:39 */

/*
 *   COMPONENT_NAME: LIBXM	Motif Toolkit
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 73
 *
 */

/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: RepType.h,v $ $Revision: 1.5 $ $Date: 92/03/13 16:42:22 $ */
/*
*  (c) Copyright 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmRepType_h
#define _XmRepType_h


#include <Xm/Xm.h>


#ifdef __cplusplus
extern "C" {
#endif


#define XmREP_TYPE_INVALID		0x1FFF

typedef unsigned short XmRepTypeId ;

typedef struct
{   
    String rep_type_name ;
    String *value_names ;
    unsigned char *values ;
    unsigned char num_values ;
    Boolean reverse_installed ;
    XmRepTypeId rep_type_id ;
    }XmRepTypeEntryRec, *XmRepTypeEntry, XmRepTypeListRec, *XmRepTypeList ;


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern XmRepTypeId XmRepTypeRegister() ;
extern void XmRepTypeAddReverse() ;
extern Boolean XmRepTypeValidValue() ;
extern XmRepTypeList XmRepTypeGetRegistered() ;
extern XmRepTypeEntry XmRepTypeGetRecord() ;
extern XmRepTypeId XmRepTypeGetId() ;
extern String * XmRepTypeGetNameList() ;
extern void XmRepTypeInstallTearOffModelConverter() ;

#else

extern XmRepTypeId XmRepTypeRegister( 
                        String rep_type,
                        String *value_names,
                        unsigned char *values,
#if NeedWidePrototypes
                        unsigned int num_values) ;
#else
                        unsigned char num_values) ;
#endif /* NeedWidePrototypes */
extern void XmRepTypeAddReverse( 
#if NeedWidePrototypes
                        int rep_type_id) ;
#else
                        XmRepTypeId rep_type_id) ;
#endif /* NeedWidePrototypes */
extern Boolean XmRepTypeValidValue( 
#if NeedWidePrototypes
                        int rep_type_id,
                        unsigned int test_value,
#else
                        XmRepTypeId rep_type_id,
                        unsigned char test_value,
#endif /* NeedWidePrototypes */
                        Widget enable_default_warning) ;
extern XmRepTypeList XmRepTypeGetRegistered( void ) ;
extern XmRepTypeEntry XmRepTypeGetRecord( 
#if NeedWidePrototypes
                        int rep_type_id) ;
#else
                        XmRepTypeId rep_type_id) ;
#endif /* NeedWidePrototypes */
extern XmRepTypeId XmRepTypeGetId( 
                        String rep_type) ;
extern String * XmRepTypeGetNameList( 
#if NeedWidePrototypes
                        int rep_type_id,
                        int use_uppercase_format) ;
#else
                        XmRepTypeId rep_type_id,
                        Boolean use_uppercase_format) ;
#endif /* NeedWidePrototypes */
extern void XmRepTypeInstallTearOffModelConverter( void ) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/



#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmRepType_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
