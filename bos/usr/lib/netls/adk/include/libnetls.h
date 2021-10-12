/*=============================================================================
 *(c) Copyright 1988, 1990, 1991 Hewlett-Packard Co.  Unpublished Work.
 * All Rights Reserved.
 *
 *                         RESTRICTED RIGHTS LEGEND
 *
 * Use, duplication or disclosure by the U.S. Government is subject to
 * restrictions as set forth in subparagraph (c)(1)(ii) of the Rights in
 * Technical Data and Computer Software clause at DFARS 252.227-7013 for DOD
 * agencies, and subparagraphs (c)(1) and (c)(2) of the Commercial Computer
 * Software Restricted Rights clause at FAR 52.227-19 for other agencies.
 *   
 *                           HEWLETT-PACKARD COMPANY
 *                             3000 Hanover Street
 *                     Palo Alto, California 94304 U.S.A.
 *
 *=============================================================================
 */
/* ===============================================================================
 * Proprietary.  Copyright 1987 by Apollo Computer Inc.,
 * Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
 * Copyright Laws Of The United States.
 * 
 * Apollo Computer Inc. reserves all rights, title and interest with respect 
 * to copying, modification or the distribution of such software programs and
 * associated documentation, except those rights specifically granted by Apollo
 * in a Product Software Program License, Source Code License or Commercial
 * License Agreement (APOLLO NETWORK LICENSE SERVER) between Apollo and 
 * Licensee.  Without such license agreements, such software programs may not
 * be used, copied, modified or distributed in source or object code form.
 * Further, the copyright notice must appear on the media, the supporting
 * documentation and packaging as set forth in such agreements.  Such License
 * Agreements do not grant any rights to use Apollo Computer's name or trademarks
 * in advertising or publicity, with respect to the distribution of the software
 * programs without the specific prior written permission of Apollo.  Trademark 
 * agreements may be obtained in a separate Trademark License Agreement.
 * ===============================================================================
 * libnetls.h - insert file for libnetls routines
 *
 */

/*
 *   NOTE - all output strings are null terminated.
 *   NOTE - must include nbase.h and uuid.h from /usr/include/idl/c before this
 *   NOTE - Several defines must be kept in sync with ls_imp.idl
 */



/*************/ 
/* CONSTANTS */
/*************/
                 
#ifndef ls_errors_h
#define ls_errors_h

#include "idl_base.h"

/* error codes */ 
#define netls_server             0x1D010000   /* server module              */
#define netls_library            0x1D020000   /* library module             */
#define netls_tools              0x1D030000   /* tools libraries (xxx_subr) */
#define um_server                0x1D040000   /* usage metering server */
#define um_library               0x1D050000   /* usage metering library */

#define netls_lic_not_fnd            netls_server + 0x1
#define netls_netls_lic_not_fnd      netls_server + 0x2
#define netls_past_exp_date          netls_server + 0x3
#define netls_netls_past_exp_date    netls_server + 0x4
#define netls_not_started            netls_server + 0x5
#define netls_netls_not_started      netls_server + 0x6
#define netls_no_version             netls_server + 0x7 
#define netls_no_netls_version       netls_server + 0x8
#define netls_not_enough_lics        netls_server + 0x9
#define netls_not_enough_netls_lics  netls_server + 0xA
#define netls_not_authorized         netls_server + 0xB
#define netls_wait_entry_deleted     netls_server + 0xC
#define netls_bad_io                 netls_server + 0xD
#define netls_others_waiting         netls_server + 0xE
#define netls_no_lics                netls_server + 0xF
#define netls_no_netls_lics          netls_server + 0x10
#define netls_still_waiting          netls_server + 0x11
#define netls_non_matching_tid       netls_server + 0x12
#define netls_bad_entry              netls_server + 0x13
#define netls_bad_timestamp          netls_server + 0x14
#define netls_vnd_not_in_db          netls_server + 0x15 
#define netls_duplicate_vendor       netls_server + 0x16
#define netls_duplicate_product      netls_server + 0x17
#define netls_no_such_product        netls_server + 0x18
#define netls_still_has_products     netls_server + 0x19
#define netls_not_valid              netls_server + 0x1A
#define netls_no_such_vendor         netls_server + 0x1B
#define netls_database_corrupt       netls_server + 0x1C
#define netls_new_pname_unexpected   netls_server + 0x1D
#define netls_cant_delete_netls_v    netls_server + 0x1E
#define netls_fatal_error            netls_server + 0x1F
#define netls_invalid_client         netls_server + 0x20
#define netls_server_not_found       netls_server + 0x21
#define netls_old_server             netls_server + 0x22
#define netls_log_corrupted          netls_server + 0x23
#define netls_too_early_to_derive    netls_server + 0x24
#define netls_too_late_to_derive     netls_server + 0x25
#define netls_wrong_dest_type        netls_server + 0x26
#define netls_not_enough_dur         netls_server + 0x27
#define netls_not_expired            netls_server + 0x28
#define netls_bad_lic_annot          netls_server + 0x29
#define netls_attempted_rename       netls_server + 0x2A
                                    
#define netls_no_init                netls_library + 0x01
#define netls_no_queues              netls_library + 0x02
#define netls_not_bound              netls_library + 0x03
#define netls_invalid_svr            netls_library + 0x04
#define netls_no_families            netls_library + 0x05
#define netls_no_svrs_fnd            netls_library + 0x06
#define netls_invalid_vid            netls_library + 0x07
#define netls_bad_param              netls_library + 0x08
#define netls_invalid_job_id         netls_library + 0x09
#define netls_duplicate_vnd          netls_library + 0x0A
#define netls_cant_create_nl         netls_library + 0x0B
#define netls_no_server_handle       netls_library + 0x0C

#define netls_decode_bad_version     netls_tools  + 0x1
#define netls_bad_password           netls_tools  + 0x2
#define netls_wrong_target           netls_tools  + 0x3   
#define netls_bad_pword_ver          netls_tools  + 0x4
#define netls_fatal_err              netls_tools  + 0x5
#define netls_not_bundled            netls_tools  + 0x6
#define netls_cant_add_nl            netls_tools  + 0x7

#define um_vnd_not_supported     um_server  + 0x1
#define um_prod_not_supported    um_server  + 0x2
#define um_vrsn_not_supported    um_server  + 0x3
#define um_rec_not_fnd           um_server  + 0x5
#define um_not_owner             um_server  + 0x6
#define um_buffer_too_small      um_server  + 0x7
#define um_not_deleted           um_server  + 0x8
#define um_some_not_deleted      um_server  + 0x9
#define um_rec_deleted           um_server  + 0xA
#define um_not_implemented       um_server  + 0xB
#define um_sanity_rec_not_fnd    um_server  + 0xC

#define umlib_no_init            um_library + 0x01
#define umlib_not_used           um_library + 0x02  /* to keep libnetls and um_lib in synch */
#define umlib_not_bound          um_library + 0x03
#define umlib_invalid_svr        um_library + 0x04
#define umlib_no_families        um_library + 0x05
#define umlib_no_svrs_fnd        um_library + 0x06
#define umlib_invalid_vid        um_library + 0x07
#define umlib_bad_param          um_library + 0x08
#define umlib_invalid_job_id     um_library + 0x09
#define umlib_duplicate_vnd      um_library + 0x0A
#define umlib_svr_not_fnd        um_library + 0x0B
#define umlib_buf_too_small      um_library + 0x0C
#define umlib_db_modified        um_library + 0x0D

#endif

/* VMS VAXC RTL contains a gmtime function that returns zero */
/* Replace it here with something more usable... */
#if defined(VMS) || defined(vms)
#define gmtime(clock) localtime(clock)
#endif

/* vrsn array length (includes space for null character) */
#define nls_VLEN 12       /* same as LS_VLEN from ls_imp.idl  */

/* product name array length (includes space for null character) */
#define nls_product_len 32 /* same as ls_product_len from ls_imp.idl  */

/* null job id */
#define nls_null_job_id uuid__nil
extern uuid__t uuid__nil;

/* password length */
#define nls_max_pword_len 128

/* comment length for password file */
#define nls_max_comment 128 

/* license types -- not expected to be the same numbering as in ls_imp.idl, */
/* but it is the same set of values, so keep in sync (translations in ls_lib1.c) */
#define NETLS_ANY         0xff
#define NETLS_NODELOCKED  0x01
#define NETLS_CONCURRENT  0x02
#define NETLS_COMPOUND    0x03
#define NETLS_USEONCE     0x04
#define NETLS_USAGE       0x05

/* target types - used in netls_request_compound() and netls_get_target() */
/* THIS MUST BE THE NUMBERING IN ls_imp.idl -- Keep in sync! */
#define NETLS_APOLLO  0
#define NETLS_DOMAIN  0
#define NETLS_OPEN    1
#define NETLS_SUN     2
#define NETLS_VAX     3
#define NETLS_MSDOS   4
#define NETLS_HPUX    5
#define NETLS_ULTRIX  6
#define NETLS_HPOSF   7
#define NETLS_SVR4    	8
#define NETLS_CLIPPER	9
#define	NETLS_SCO	10
#define	NETLS_SGI	11
#define	NETLS_NEXT	12
#define NETLS_AIX       13
#define NETLS_NOVELL    14
#define NETLS_DGUX      15
#define	NETLS_OSF1	16
#define NETLS_OS2	17
#define NETLS_OS400	18
#define NETLS_MVS	19
#define NETLS_WINNT	20


/*********/
/* TYPES */
/*********/
#ifndef ls_imp_included
typedef ndr__long_float trans_id_t;

#ifndef STD_INCLUDED
typedef ndr__long_int status__all_t;
#endif
#endif

/* vendor id */
typedef uuid__string_t nls_vnd_id_t;
                         
/* job id */
typedef uuid__t nls_job_id_t;

/* dates */
typedef ndr__ulong_int nls_time_t; /* Must be same as ls_time_t from ls_imp.idl */

/* license annotation   81 characters plus a null terminator */
typedef char nls_lic_annot_t[82];       /* -- same as NLS_LA_LEN from ls_imp.idl */

/* license transaction */
typedef ndr__long_int nls_lic_trans_id_t;


/**********************************************/
/* n e t l s _ c r e a t e _ j o b _ i d () */
/**********************************************/

/* 
 * creates a job id for the process
 */

extern void netls_create_job_id(
#ifdef __STDC__
nls_job_id_t    *job_id     /* returned job id */
#endif
);   

/*
    OUT:
        job_id - the created job id 
*/



/****************************/
/* n e t l s _ i n i t () */
/****************************/
 
/*
 * locates servers that service this vendor; must be called before 
 * libnetls routines 
 */            

extern void netls_init(
#ifdef __STDC__
nls_vnd_id_t    vnd_id,     /* vendor id */
ndr__long_int   vnd_key,    /* vendor's encoding key */
nls_job_id_t    *job_id,    /* job id of this process */
status__all_t   *status     /* returned status */
#endif
);

/* 
    IN: 
        vnd_id -  the uuid text that is given in lspass.

        vnd_key - two byte key to use in time stamp protection alg;
        same as vendor key in lspass.


    IN/OUT: 
        job_id - job id for this job; if uuid_$nil on input, a job
        id is assigned.
       
    OUT:
        status - set to status_$ok if no error, otherwise see below.


    POSSIBLE STATUS:
        status_$ok - ok.          

        netls_no_svrs_fnd - no license servers were found in the NCS
        locating broker for the particular vendor (vnd_id).
  
        netls_bad_param - one of the paramters to this call was not
        correct.
  
        netls_invalid_vid - bad uuid text for the vendor id

        netls_duplicate_vnd - duplicate vendor 
*/

/****************************************************/
/* n e t l s _ r e q u e s t _ c o m p o u n d () */
/****************************************************/
/*
 * Gets a destination license from a compound license.
 *
 */
extern ndr__long_int netls_request_compound(
#ifdef __STDC__
nls_job_id_t    *job_id,
ndr__long_int    lic_type,       /* type of password to be created.  If not compound, this must match the destination type in the compound password */
ndr__long_int    prod_id,        /* product id */
char             vrsn[nls_VLEN], /* product version */
ndr__long_int    vlen,           /* length of product version */
ndr__long_int    num_lics,       /* number of licenses for new password */
ndr__long_int    target_id,      /* server id of server where licenses will be installed */
ndr__long_int    target_type,    /* type of server */
nls_time_t       start_date,     /* start date for new password (note: cascaded compound licenses may *not* alter the derived start, destination) */
ndr__long_int    duration,       /* duration for new password   */
char             vpassword[nls_max_pword_len],  /* resulting vendor password */
char             ppassword[nls_max_pword_len],  /* resulting product password */
nls_lic_annot_t  lic_annot,      /* license annotation of password */
status__all_t    *status
#endif
);

/*
    IN: 
        job_id - job id for this job.

        lic_type - type of license to be built; one of NETLS_NODELOCKED, NETLS_CONCURRENT, NETLS_COMPOUND, 
        NETLS_USAGE, NETLS_USEONCE.

        prod_id - identifier of the product that is being requested.   
        Identifier is defined by the vendor.

        vrsn - the vendor defined product version text.

        vlen - the length of the version text.

        num_lics - the number of licenses being requested.

        target_id - the id of the target for which these licenses are destined.
           
        target_type - type of the above target; one of NETLS_APOLLO, NETLS_ANY, NETLS_SUN,
        NETLS_VAX, NETLS_MSDOS.

        start_date - date license is to begin.  represented as the number of seconds
        since Jan 1, 1970.
                      
        duration - number of days until this license expires.

    OUT: 
        vpassword - vendor password (see documentation).

        ppassword - product password (see documentation).

        license annotation - annotation of license password created.

    POSSIBLE STATUS:
        status_$ok - ok.
        
        netls_lic_not_fnd - license was not found in license server
        data base.

        netls_past_exp_date - all licenses are out of date.

        netls_not_started - all licenses' start dates have not 
        occurred yet. 

        netls_no_such_vendor - no vendor found for this product

        netls_no_version - all licenses have the wrong version.

        netls_no_lics - no licenses available for this product

        netls_not_enough_lics - there are not enough installed licenses
        for the amount of licenses needed by this request.

        netls_not_authorized - user is not authorized to use requested
        product according to the user file.

        netls_bad_timestamp - license server system's time of day is too 
        different from local system's.

        netls_old_server - talking to an old version of the server

        netls_invalid_client - the client was not recognized as being an
        authentic client 

        netls_too_early_to_derive - the start date of the derived license occurs
        before the compound license start date

        netls_too_late_to_derive - the end date of the derived license occurs after 
        the compound license start date plus the duration

        netls_not_enough_dur - aggregate duration is smaller than compound license
        duration

        netls_wrong_dest_type - the license type declared for the destination must
        be the same as the license type declared in the compound license

        netls_no_init - call netls_init() first.

        netls_invalid_svr - the server was not recognized as being an
        authentic license server.

        netls_bad_param - one of the paramters to this call was not
        correct.

        netls_invalid_job_id - the job id was not valid.  make sure
        you obtained a valid job id via the netls_get_job_id() call.

        netls_no_svrs_fnd - no servers found
*/                     



/**************************************************/
/* n e t l s _ r e q u e s t _ l i c e n s e () */
/**************************************************/

/*
 * contacts each known server servicing this vendor to request num_lics
 * licenses for the product described by <prod_id, vrsn>; the type
 * of license needed is specified.
 * returns true if license retrieved, false otherwise.
 */

extern ndr__long_int netls_request_license(
#ifdef __STDC__
nls_job_id_t          *job_id,        /* job id of this vendor */
ndr__long_int         *lic_type,      /* license type */
ndr__long_int         prod_id,        /* product id */
char                  vrsn[nls_VLEN], /* product version */
ndr__long_int         vlen,           /* length of version */
ndr__long_int         num_lics,       /* amount of licenses requested */
nls_time_t            chk_per,        /* time out period in seconds */
nls_lic_trans_id_t    *lic_trans_id,  /* returned license transaction id */
nls_time_t            *exp_date,      /* expiration date on license */
nls_lic_annot_t       lic_annot,      /* annotation of license */
status__all_t         *status         /* returned status */
#endif
);

/*
    IN: 
        job_id - job id for this job.

        prod_id - identifier of the product that is being requested.

        vrsn - product version text.

        vlen - length of the version text.

        num_lics - number of licenses being requested.

        chk_per - amount of time between check calls in seconds; after 
        this time the license server will return the license to the free pool 
        if the application has not checked in.

    OUT: 
        lic_trans_id - this is a reference to a "black box" which netls uses to 
        store information about a request/wait/release transaction for
        a particular license.  Each request/wait/release for a different 
        license should use a different license transaction id.

        exp_date - the closest expiration date of all records of this
        license being requested; represented as the number of seconds
        since Jan 1, 1970.
                            
        lic_annot - annotation to the license as defined in nlspass at 
        password creation time.

        status - set to status_$ok if no error, otherwise see below.
                                                            
    IN/OUT:
        lic_type - type of license to be requested; one of NETLS_ANY, NETLS_NODELOCKED, 
        NETLS_CONCURRENT, NETLS_USEONCE.  on output, this is the actual type that was
        granted (helpful in the case of NETLS_ANY).



    POSSIBLE STATUS:
        status_$ok - ok.
        
        netls_lic_not_fnd - license was not found in license server
        data base.

        netls_past_exp_date - all licenses are out of date.

        netls_not_started - all licenses' start dates have not 
        occurred yet.

        netls_no_version - all licenses have the wrong version.

        netls_not_enough_lics - there are not enough installed licenses
        for the amount of licenses needed by this request.

        netls_not_authorized - user is not authorized to use requested
        product according to the user file.

        netls_others_waiting - other users are waiting in the wait queue; call
        netls_request_and_wait() to be placed in queue.

        netls_no_lics - no licenses are available at present.

        netls_bad_timestamp - license server system's time of day is too 
        different from local system's.
        
        netls_old_server - talking to an old version of the server

        netls_invalid_client - the client was not recognized as being an
        authentic client

        netls_no_init - call netls_init() first.

        netls_invalid_svr - the server was not recognized as being an
        authentic license server.

        netls_bad_param - one of the paramters to this call was not
        correct.

        netls_invalid_job_id - the job id was not valid.  make sure
        you obtained a valid job id via the netls_get_job_id() call.

        netls_no_svrs_fnd - no servers found
*/                     



/***************************************************/
/* n e t l s _ r e q u e s t_ a n d _ w a i t () */
/***************************************************/

/*
 * contacts each known server servicing this vendor to request num_lics
 * licenses for the product described by <prod_id, vrsn>; if no
 * licenses are available, the user is placed in a wait queue; the
 * type of license needed is specified.
 * returns true if license retrieved, false otherwise.
 */

extern ndr__long_int netls_request_and_wait(
#ifdef __STDC__
nls_job_id_t          *job_id,        /* job id of process */
ndr__long_int         *lic_type,      /* type of license */
ndr__long_int         prod_id,        /* product id */
char                  vrsn[nls_VLEN], /* product version */
ndr__long_int         vlen,           /* length of version */
ndr__long_int         num_lics,       /* amount licenses requested */
nls_time_t            chk_per,        /* time out period in seconds */
ndr__long_int         pos_list_sz,    /* size of queue position list array */
nls_lic_trans_id_t    *lic_trans_id,  /* returned license transaction id */
nls_time_t            *exp_date,      /* expiration date of license */  
nls_lic_annot_t       lic_annot,      /* annotation of license */
ndr__long_int         q_pos_list[],   /* position in queues */
ndr__long_int         *num_queues,    /* number of queues in which user is waiting */
status__all_t         *status         /* returned status */
#endif
);

/*
    IN: 
        job_id - job id for this job.

        prod_id - identifier of the product that is being requested.   
        Identifier is defined by the vendor.

        vrsn - the vendor defined product version text.

        vlen - the length of the version text.

        num_lics - the number of licenses being requested.

        chk_per - the amount of time between check calls in seconds; after 
        this time the license server will return the license to the free pool 
        if the application has not checked in.                                       

        pos_list_sz - the number of elements in pos_list[].

    OUT: 
        lic_trans_id - this is a reference to a "black box" which netls uses to 
        store information about a request/wait/release transaction for
        a particular license.  Each request/wait/release for a different 
        license should use a different license transaction id.  

        exp_date - the closest expiration date of all records of this
        license being requested.  represented as the number of seconds
        since Jan 1, 1970.
                      
        lic_annot - vendor information.  (This is entered into the license
        through the password)                                             

        pos_list - an array of integers; each element is the position 
        of the user on the queue at each license server (starting at 
        one).  

        num_queues - the number of queues that the application is waiting
        in; upon return, this is the number of valid elements in pos_list.

        status - set to status_$ok if no error, otherwise see below.


    IN/OUT:
        lic_type - type of license to be requested; one of NETLS_ANY, NETLS_NODELOCKED, 
        NETLS_CONCURRENT, NETLS_USEONCE.  on output, this is the actual type that was
        granted (helpful in the case of NETLS_ANY).


    POSSIBLE STATUS (NOTE - only placed on wait queue if noted, check
                      num_queues to to see if waiting):
        status_$ok - ok.
        
        netls_lic_not_fnd - license was not found in license server
        data base.

        netls_past_exp_date - all licenses are out of date.

        netls_not_started - all licenses' start dates have not 
        occurred yet.

        netls_no_version - all licenses have the wrong version.

        netls_not_enough_lics - there are not enough installed licenses
        for the amount of licenses needed by this request.

        netls_not_authorized - user is not authorized to use requested
        product according to the user file.

        netls_others_waiting - other users are waiting in the wait queue; call
        netls_request_and_wait() to be placed in queue.

        netls_no_lics - no licenses are available at present.

        netls_bad_timestamp - license server system's time of day is too 
        different from local system's.
        
        netls_old_server - talking to an old version of the server

        netls_invalid_client - the client was not recognized as being an
        authentic client

        netls_no_init - call netls_init() first.

        netls_invalid_svr - the server was not recognized as being an
        authentic license server.

        netls_bad_param - one of the paramters to this call was not
        correct.

        netls_invalid_job_id - the job id was not valid.  make sure
        you obtained a valid job id via the netls_get_job_id() call.

        netls_no_svrs_fnd - no servers found
 
*/



/******************************************/
/* n e t l s _ w a i t _ s t a t u s () */
/******************************************/

/*
 * checks position in each queue; if license ready, it is granted;
 * otherwise, positions in queues are returned 
 * returns true (non-zero) if license retrieved, false (zero) otherwise.
 */

extern ndr__long_int netls_wait_status(
#ifdef __STDC__
nls_lic_trans_id_t  lic_trans_id,   /* id of this license transaction */
ndr__long_int       pos_list_sz,    /* size of queue position list array */
ndr__long_int       q_pos_list[],   /* position in queues */
ndr__long_int       *num_queues,    /* number of queues in which user is waiting */
nls_lic_annot_t     lic_annot,      /* annotation of license */
status__all_t       *status         /* returned status */
#endif
);

/*
    IN: 
        lic_trans_id - this is a reference to a "black box" which netls uses to 
        store information about a request/wait/release transaction for
        a particular license.  Each request/wait/release for a different 
        license should use a different license transaction id.  this transaction 
        id is gotten from netls_request() or netls_request_and_wait().

        pos_list_sz - the number of elements in pos_list[].

    OUT:
        q_pos_list - an array of integers; each element is the position 
        of the user on the queue at each license server (starting at 
        one).  

        num_queues - the number of queues that the application is waiting
        in; upon return, this is the number of valid elements in pos_list.

        lic_annot - vendor information is returned if a license we granted.  
        (This is entered into the license through the password)         

        status - set to status_$ok if no error, otherwise see below.


    POSSIBLE STATUS:
        status_$ok - ok.
        
        netls_lic_not_fnd - license was not found in license server
        data base.

        netls_no_version - all licenses have the wrong version.

        netls_wait_entry_deleted - wait queue entry was deleted, probably
        because application did not wait_stat() within two minutes and
        was removed because it was thought to be dead.

        netls_still_waiting - licenses still not available; user is still
        on waiting queue. 

        netls_bad_timestamp - license server system's time of day is too 
        different from local system's.

        netls_no_queues - user is not waiting on any queues; probably 
        did not call netls_request_and_wait().

        netls_no_init - call netls_init() first.

        netls_invalid_svr - the server was not recognized as being an
        authentic license server.

        netls_bad_param - one of the paramters to this call was not
        correct.                                                  

        netls_invalid_job_id - the job id was not valid.  make sure
        you obtained a valid job id via the netls_get_job_id() call.


*/



/******************************************/
/* n e t l s _ w a i t _ r e m o v e () */
/******************************************/

/*
 * remove user from all queues associated with this transaction
 */

extern void netls_wait_remove(
#ifdef __STDC__
nls_lic_trans_id_t  lic_trans_id,   /* id of this license transaction */
status__all_t       *status         /* returned status */
#endif
);

/*
    IN: 
        lic_trans_id - this is a reference to a "black box" which netls uses to 
        store information about a request/wait/release transaction for
        a particular license.  Each request/wait/release for a different 
        license should use a different license transaction id.  this transaction 
        id is gotten from netls_request() or netls_request_and_wait().

    OUT
        status - set to status_$ok if no error, otherwise see below.


    POSSIBLE STATUS:
        status_$ok - ok.
        
        netls_no_version - all licenses have the wrong version.

        netls_wait_entry_deleted - wait queue entry was deleted, probably
        because application did not wait_stat() within two minutes and
        was removed because it was thought to be dead.

        netls_bad_timestamp - license server system's time of day is too 
        different from local system's.

        netls_no_init - call netls_init() first.

        netls_no_queues - user is not waiting on any queues; probably 
        did not call netls_request_and_wait().

        netls_bad_param - one of the paramters to this call was not
        correct.  

        netls_invalid_job_id - the job id was not valid.  make sure
        you obtained a valid job id via the netls_get_job_id() call.


*/

/**************************************************/
/* n e t l s _ r e l e a s e _ l i c e n s e () */
/**************************************************/
 
/*
 * notifies server that application is finished with license
 */

extern void netls_release_license(
#ifdef __STDC__
nls_lic_trans_id_t  lic_trans_id,   /* id of this license transaction */
status__all_t       *status         /* returned status */
#endif
);

/* 
    IN: 
        lic_trans_id - this is a reference to a "black box" which netls uses to 
        store information about a request/wait/release transaction for
        a particular license.  Each request/wait/release for a different 
        license should use a different license transaction id.  this 
        transaction id is gotten from netls_request() or netls_request_and_wait().

    OUT
        status - set to status_$ok if no error, otherwise see below.


    POSSIBLE STATUS:
        status_$ok - ok.
        
        netls_lic_not_fnd - license was not found in license server
        data base.

        netls_no_version - all licenses have the wrong version.

        netls_non_matching_tid - transaction id did not match in data base;
        probably because license became stale and was removed by another
        user's request.

        netls_bad_timestamp - license server system's time of day is too 
        different from local system's.

        netls_no_init - call netls_init() first.

        netls_not_bound - not bound to any server; probably because release
        called without being granted a license first.

        netls_invalid_svr - the server was not recognized as being an
        authentic license server.

        netls_bad_param - one of the paramters to this call was not
        correct. 

        netls_invalid_job_id - the job id was not valid.  make sure
        you obtained a valid job id via the netls_get_job_id() call.


*/


/**********************************/
/* n e t l s _ c l e a n u p () */
/**********************************/

/* 
 * releases all held licenses and removes user from all queues
 */

extern void netls_cleanup(
#ifdef __STDC__
nls_job_id_t    *job_id,    /* job id of this process */
status__all_t   *status     /* returned status */
#endif
);
/* 
    IN:
        job_id - job id for this job.

    POSSIBLE STATUS:
        status_$ok - ok.
        
        netls_lic_not_fnd - license was not found in license server
        data base.

        netls_no_version - all licenses have the wrong version.

        netls_non_matching_tid - transaction id did not match in data base;
        probably because license became stale and was removed by another
        user's request.

        netls_bad_timestamp - license server system's time of day is too 
        different from local system's.

        netls_no_init - call netls_init() first.

        netls_invalid_job_id - the job id was not valid.  make sure
        you obtained a valid job id via the netls_get_job_id() call.
*/        

/**********************************************/
/* n e t l s _ c h e c k _ l i c e n s e () */
/**********************************************/
        
/*
 * notifies server that appl is still using license.  Should be 
 * called periodically so that server can know which licenses 
 * have been granted to dead applications and return those stale 
 * licenses to the free pool.
 * returns true (non-zero) if license server acknowledges the granting
 * of the license, false (zero) otherwise.
 */

extern ndr__long_int netls_check_license(
#ifdef __STDC__
nls_lic_trans_id_t  lic_trans_id,   /* id of license transaction */
nls_time_t          chk_per,        /* time out period in seconds */
status__all_t       *status         /* returned status */
#endif
);                   

/*
    IN: 
        lic_trans_id - this is a reference to a "black box" which netls uses to 
        store information about a request/wait/release transaction for
        a particular license.  Each request/wait/release for a different 
        license should use a different license transaction id.  this 
        transaction id is gotten from netls_request() or netls_request_and_wait().

        chk_period - the period (in seconds) that the license server should 
        wait before returning this application's license(s) back to the free
        pool if the application does not check in again. 

    OUT
        status - set to status_$ok if no error, otherwise see below.


    POSSIBLE STATUS:
        status_$ok - ok.
        
        netls_lic_not_fnd - license was not found in license server
        data base.

        netls_no_version - all licenses have the wrong version.

        netls_bad_entry - transaction id did not match in data base;
        probably because license became stale and was removed by another
        user's request.

        netls_bad_timestamp - license server system's time of day is too 
        different from local system's.

        netls_no_init - call netls_init() first.

        netls_not_bound - not bound to any server; probably because release
        called without being granted a license first.

        netls_invalid_svr - the server was not recognized as being an
        authentic license server.

        netls_bad_param - one of the paramters to this call was not
        correct.  

        netls_invalid_job_id - the job id was not valid.  make sure
        you obtained a valid job id via the netls_get_job_id() call.

*/



/**********************************************/
/* n e t l s _ g e t _ c u r _ u s e r s () */
/**********************************************/

/*
 * returns information about users who have been granted the 
 * designated license <prod_id, vrsn>.
 */

extern void netls_get_cur_users(
#ifdef __STDC__
nls_job_id_t   *job_id,        /* job id of vendor */
ndr__long_int   prod_id,        /* product id */
char            vrsn[nls_VLEN], /* product version */
ndr__long_int   vlen,           /* length of product version */
ndr__long_int   sz,             /* size of out arrays */
ndr__long_int   *cnt,           /* number of elems returned */
char            u_name[][32],   /* array of user names */
char            n_name[][32],   /* array of node names */
char            g_name[][32],   /* array of group names */
nls_time_t      start_time[],   /* array of start times */
ndr__long_int   num_lics[],     /* array of license amounts */
ndr__long_int   *total_lics,    /* total number of licenses granted */
ndr__long_int   *ptr,           /* looping pointer */
status__all_t   *status         /* returned status */
#endif
);

/*
    IN:                    
        job_id - job id for this job.

        prod_id - identifier of the product that is being querried.   
        Identifier is defined by the vendor.

        vrsn - the vendor defined product version text.

        vlen - the length of the version text.

        sz - the number of elements in ulist;  only that many users will
        be returned.

    OUT:
        cnt - the number of users returned.

        u_name - an array of 32 byte character strings; user names of the
        current users.

        n_name - an array of 32 byte character strings; node names of the
        current users.

        g_name - an array of 32 byte character strings; group names of the
        current users.                                      

        start_time - an array of nls_time_t; start times of the current users,
        expressed as the number of seconds since Jan 1, 1970.

        num_lics - an array of 32 bit integers; number of licenses held by each
        current user.
                  
        total_lics - the total number of licenses

        status - set to status_$ok if no error, otherwise see below.

          
    IN/OUT:
        ptr - used to call netls_get_cur_users() successively.


    POSSIBLE STATUS:
        status_$ok - ok.
        
        netls_lic_not_fnd - license was not found in license server
        data base.

        netls_no_version - all licenses have the wrong version.

        netls_no_init - call netls_init() first.

        netls_bad_param - one of the paramters to this call was not
        correct.

        netls_invalid_job_id - the job id was not valid.  make sure
        you obtained a valid job id via the netls_get_job_id() call.
*/

/************************************************************/
/* n e t l s _ g e t _ a l l _ p r o d u c t _ i n f o () */
/************************************************************/
  
/* 
 * provides information about the products installed for a given 
 * vendor by returning the fields from the product records in the 
 * database.
 * returns number of elements in out arrays
 */                     
                                                      
extern ndr__long_int netls_get_all_product_info(
#ifdef __STDC__
nls_job_id_t      *job_id,                      /* job id of process */
ndr__long_int     size,                         /* size of out arrays */
ndr__long_int     *pntr1,                       /* looping pointer */
ndr__long_int     *pntr2,                       /* looping pointer */
ndr__long_int     prod_id[],                    /* product id */
ndr__long_int     num_lics[],                   /* license installed */
nls_time_t        start_date[],                 /* product start date */
nls_time_t        exp_date[],                   /* product expiration date */
char              prod_name[][nls_product_len], /* product name */
ndr__long_int     pnlen[],                      /* length of product name */
char              vrsn[][nls_VLEN],             /* product version */
ndr__long_int     vlen[],                       /* length of product version */
nls_time_t        ts[],                         /* license timestamp */
ndr__long_int     in_use[],                     /* how many are granted */
nls_lic_annot_t   lic_annot[],                  /* license annotation */
ndr__long_int     *rem,                         /* number of recs remaining */
status__all_t     *status                       /* returned status */
#endif
);

/*
    Note:
        This call made be made iteratively to collect information about all products.  Iterative
        calls are only valid if there are no other intervening netls calls.  On rare occasions
        this call may miss information (this will only happen if the database is being modified
        with LSADMIN between iterations of this call).  For any given product id, there may
        be many records in the database, and therefore many entries in the returned arrays.
      
    IN:
        job_id - job id for this job.

        size - the size of elements in the arrays (num_lics, start_date,...).  Only that many elements
        will be returned.
       
    IN - OUT:
        pntr1 - An internal pointer that allows you to collect all the information by making multiple
        calls.  Set this to zero the first time you call this procedure.  On subsequent calls, pass
        in whatever was passed back on the previous call.

        pntr2 - An internal pointer that allows you to collect all the information by making multiple
        calls.  Set this to zero the first time you call this procedure.  On subsequent calls, pass
        in whatever was passed back on the previous call.
        
          
    OUT:                        
        prod_id - an array of 32 bit integers; the product id for this record.

        num_lics - an array of 32 bit integers; the number of licenses installed with this record.

        start_date -an array of nls_time_t; the start date (in seconds since 1970) for this record.

        exp_date - an array of nls_time_t; the expiration date (in seconds since 1970) for this record.

        prod_name - an array of 32 character arrays; the name of this product.

        pnlen - an array of 32 bit integers; the length of the names.

        version - an array of 9 character arrays; the version string for this product.

        vlen - an array of 32 bit integers; the lengths of the version strings.

        ts - an array of nls_time_t; the timestamp for thsi record. 

        in_use - an array of 32 bit integers; the number of licenses from this record currently in use. 

        lic_annot - an array of 82 character arrays; vendor information for this product.
                    (This is entered into the license through the password)         

        rem - the number of records remaining.  At least rem records remain, there may be more.

        status - set to status_$ok if no error, otherwise see below.

    RETURNS:
        The number of records found - this is the number of elements filled in in the out arrays.

    POSSIBLE STATUS:

        status_$ok - ok.
        
        netls_no_init - call netls_init() first.

        netls_bad_param - one of the paramters to this call was not
        correct.

        netls_invalid_job_id - the job id was not valid.  make sure
        you obtained a valid job id via the netls_get_job_id() call.
*/


/*****************************************************/
/* n l s l i b  _ g e t _ p r o d u c t _ i n f o () */
/*****************************************************/
 
/*
 * provides information about a product installed for a given 
 * vendor by returning the fields from the product records in 
 * the database.
 * returns number of elements in out arrays.
 */                                                        

extern ndr__long_int netls_get_product_info(
#ifdef __STDC__
nls_job_id_t    *job_id,                      /* job id of process */
ndr__long_int   prod_id,                      /* product id to look up */
ndr__long_int   size,                         /* size of out arrays */
ndr__long_int   *pntr1,                       /* looping pointer */
ndr__long_int   *pntr2,                       /* looping pointer */
ndr__long_int   num_lics[],                   /* license installed */
nls_time_t      start_date[],                 /* product start date */
nls_time_t      exp_date[],                   /* product expiration date */
char            prod_name[][nls_product_len], /* product name */
ndr__long_int   pnlen[],                      /* length of product name */
char            vrsn[][nls_VLEN],             /* product version */
ndr__long_int   vlen[],                       /* length of product version */
nls_time_t      ts[],                         /* license timestamp */
ndr__long_int   in_use[],                     /* how many are granted */
nls_lic_annot_t lic_annot[],                  /* license annotation */
ndr__long_int   *rem,                         /* number of recs remaining */
status__all_t   *status                       /* returned status */
#endif
);
/*
    Notes:
        This call made be made iteratively to collect information about all records for all versions.  
        Iterative calls are only valid if there are no other intervening netls calls.  On rare occasions
        this call may miss information (this will only happen if the database is being modified with 
        NLSADMIN between iterations of this call).  For any given product id, there may be many records 
        in the database, and therefore many entries in the returned arrays.
    
    IN:        
        job_id - job id for this job.

        prod_id - the product id of interest.

        size - the size of elements in the arrays (num_Lics, start_date,...).  Only that many elements
        will be returned.
       
    IN - OUT:
        pntr1 - An internal pointer that allows you to collect all the information by making multiple
        calls.  Set this to zero the first time you call this procedure.  On subsequent calls, pass
        in whatever was passed back on the previous call.

        pntr2 - An internal pointer that allows you to collect all the information by making multiple
        calls.  Set this to zero the first time you call this procedure.  On subsequent calls, pass
        in whatever was passed back on the previous call.
        
          
    OUT:                        
        num_lics - an array of 32 bit integers; the number of licenses installed with this record.

        start_date -an array of nls_time_t; the start date (in seconds since 1970) for this record.

        exp_date - an array of nls_time_t; the expiration date (in seconds since 1970) for this record.

        prod_name - an array of 32 character arrays; the name of this product.

        pnlen - an array of 32 bit integers; the length of the names.

        version - an array of 9 character arrays; the version string for this product.

        vlen - an array of 32 bit integers; the lengths of the version strings.

        ts - an array of nls_time_t; the timestamp for thsi record. 

        in_use - an array of 32 bit integers; the number of licenses from this record currently in use. 

        lic_annot - an array of 80 character arrays; vendor information for this product.
                    (This is entered into the license through the password)         

        rem - the number of records remaining.  At least rem records remain, there may be more.

        status - set to status_$ok if no error, otherwise see below.

    RETURNS:
        The number of records found - this is the number of elements filled in in the out arrays.

    POSSIBLE STATUS:

        status_$ok - ok.
        
        netls_no_init - call netls_init() first.

        netls_bad_param - one of the paramters to this call was not
        correct.

        netls_invalid_job_id - the job id was not valid.  make sure
        you obtained a valid job id via the netls_get_job_id() call.
*/



/****************************************/
/* g e t _ n o d e l o c k _ i n f o () */
/****************************************/
    
/* 
 * provides information about installed nodelocked licenses
 * returns number of elements in out arrays 
 */                                        

ndr__long_int netls_get_nodelock_info(
#ifdef __STDC__
nls_job_id_t    *job_id,                      /* job id of process */
ndr__long_int   size,                         /* size of out arrays */
ndr__long_int   *pntr,                        /* looping pointer */
ndr__long_int   prod_id[],                    /* product id */
nls_time_t      start_date[],                 /* product start date */
nls_time_t      exp_date[],                   /* product expiration date */
nls_time_t      ts[],                         /* license timestamp */
nls_lic_annot_t lic_annot[],                  /* license annotation */
ndr__long_int   *rem,                         /* number of records remaining */
status__all_t   *status
#endif
);

/*
    IN:        
        job_id - job id for this job.

        size - the size of elements in the arrays (prod_id, start_date,...).  Only that many elements
        will be returned.
       
    IN - OUT:
        pntr - An internal pointer that allows you to collect all the information by making multiple
        calls.  Set this to zero the first time you call this procedure.  On subsequent calls, pass
        in whatever was passed back on the previous call.
          
    OUT:                        
        prod_id - an array of 32 bit integers; the product ids of the installed products

        start_date -an array of nls_time_t; the start date (in seconds since 1970) for this record.

        exp_date - an array of nls_time_t; the expiration date (in seconds since 1970) for this record.

        ts - an array of nls_time_t; the timestamp for this record. 

        lic_annot - an array of 80 character arrays; vendor information for this product.
                    (This is entered into the license through the password)         

        rem - the number of records remaining.  At least rem records remain, there may be more.

        status - set to status_$ok if no error, otherwise see below.

    RETURNS:
        The number of records found - this is the number of elements filled in in the out arrays.

    POSSIBLE STATUS:
        status_$ok - ok.
        
        netls_no_init - call netls_init() first.

        netls_bad_param - one of the paramters to this call was not
        correct.

        netls_invalid_job_id - the job id was not valid.  make sure
        you obtained a valid job id via the netls_get_job_id() call.
*/

        





/**********************************/                 
/* n e t l s _ l o g _ m s g () */
/**********************************/                 

/*
 * logs msg in the log file of each server found for the transaction 
 * identified by the transaction id.  If license already checked out, 
 * then msg will appear at server where license came from; otherwise, 
 * log msg will appear at each server which supports the vendor.
 */
       
extern void netls_log_msg(
#ifdef __STDC__
nls_job_id_t        *job_id,        /* job id for vendor */
nls_lic_trans_id_t  lic_trans_id,   /* id of license transaction */
char                msg[128],       /* message to log */
ndr__long_int       msgl,           /* length of message */
status__all_t       *status         /* returned status */
#endif
);

/*
    IN:
        job_id - job id for this job.

        lic_trans_id - this is a reference to a "black box" which netls uses to 
        store information about a request/wait/release transaction for
        a particular license.  Each request/wait/release for a different 
        license should use a different license transaction id.  this 
        transaction id is gotten from netls_request() or netls_request_and_wait().

        msg - the message to be printed.  limited to 128 bytes.
    
        msgl - the length of the message to be printed.


    POSSIBLE STATUS:
        status_$ok - ok.
        
        netls_no_init - call netls_init() first.

        netls_bad_param - one of the paramters to this call was not
        correct.

        netls_invalid_job_id - the job id was not valid.  make sure
        you obtained a valid job id via the netls_get_job_id() call.
*/

/******************************************/
/* n e t l s _ g e t _ e r r _ m s g () */
/******************************************/

/* 
 * returns appropriate error message in err_msg depending on 
 * given status code.
 */

extern void netls_get_err_msg(
#ifdef __STDC__
char            proc_name[33],  /* errant procedure name */
ndr__long_int   pnl,            /* length of procedure name */
status__all_t   status,         /* status code */
char            err_msg[128],   /* returned error message */
ndr__long_int   *errl           /* length of error message */
#endif
);

/*
    IN:
        proc_name - the procedure name in which the error occured.

        pnl - the length of the procedure name string.

        status - the returned status from another netls_() call.
    
    OUT:
        err_msg - the error message returned.

        errl - the length of the error message.
*/




/****************************************************/
/* n e t l s _ a d d _ n o d e l o c k e d ()     */
/****************************************************/

/*
 * adds a nodelocked password to the nodelock file
 */

extern void netls_add_nodelocked(
#ifdef __STDC__
nls_job_id_t        *job_id,                    /* my jobid */
char                password[nls_max_pword_len],/* password to add */
nls_lic_annot_t     lic_annot,                  /* annotation of license */
char                comment[nls_max_comment],   /* comment to add */
ndr__long_int       clen,                       /* length of comment */
status__all_t       *status
#endif 
);
           


/* 
    IN:
        job_id - job id for this job.

        password - product password to add to nodelock file.
        
        lic_annot - annotation to the license as defined in nlspass at 
        password creation time.

        comment - comment to add to the nodelock file.

        clen - length of comment.  

    POSSIBLE STATUS:
        status_$ok - ok.
        
        netls_no_init - call netls_init() first.

        netls_bad_param - one of the paramters to this call was not
        correct.

        netls_invalid_job_id - the job id was not valid.  make sure
        you obtained a valid job id via the netls_get_job_id() call.

        netls_can't_create_nl - cannot create nodelocked file


*/



/****************************************/
/* n e t l s _ g e t _ t a r g e t () */
/****************************************/

/*
 * returns server id and type for this node
 */

extern void netls_get_target(
#ifdef __STDC__
ndr__long_int       *target_id,
ndr__long_int       *target_type
#endif
);

/*
    OUT:
        target_id - the target id of this machine or the target id
        currently set if netls_set_target_id() was called.

        target_type - the type of this machine (NLS_APOLLO, NLS_SUN,
        NLS_VAX, NLS_MSDOS, or NLS_OPEN)
*/


/**********************************************/
/* n e t l s _ s e t _ t a r g e t _ i d () */
/**********************************************/

/*
 * allows application to set target id; must call netls_init() after
 * this call to establish node lock list correctly
 */

extern void netls_set_target_id(
#ifdef __STDC__
ndr__long_int   target_id
#endif
);

/*
    IN:
        target_id - the target id to set.
*/

