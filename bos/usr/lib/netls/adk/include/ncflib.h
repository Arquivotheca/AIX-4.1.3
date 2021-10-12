#if !defined(hpux) && !defined(AIX3_2)
#ident "@(#)ncflib.h:3.2  File Date:93/03/25 Last Delta:92/05/21 14:31:20"
#endif
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
 * ncflib.h
 *
 */

/*
 *   NOTE - all output strings are null terminated.
 *   NOTE - must include nbase.h and uuid.h from /usr/include/idl/c before this
 */



/*************/ 
/* CONSTANTS */
/*************/ 
/* NetLS error codes */

#ifndef ls_errors_h
#define ls_errors_h

#define netls_subsys               0x1d         /* subsystem code             */

#define netls_server               0x1d010000   /* server module              */
#define netls_library              0x1d020000   /* library module             */
#define netls_tools                0x1d030000   /* tools libraries            */

#define netls_lic_not_fnd          netls_server + 0x1
#define netls_netls_lic_not_fnd      netls_server + 0x2
#define netls_past_exp_date        netls_server + 0x3
#define netls_netls_past_exp_date    netls_server + 0x4
#define netls_not_started          netls_server + 0x5
#define netls_netls_not_started      netls_server + 0x6
#define netls_no_version           netls_server + 0x7 
#define netls_no_netls_version       netls_server + 0x8
#define netls_not_enough_lics      netls_server + 0x9
#define netls_not_enough_netls_lics  netls_server + 0xA
#define netls_not_authorized       netls_server + 0xB
#define netls_wait_entry_deleted   netls_server + 0xC
#define netls_bad_io               netls_server + 0xD
#define netls_others_waiting       netls_server + 0xE
#define netls_no_lics              netls_server + 0xF
#define netls_no_netls_lics          netls_server + 0x10
#define netls_still_waiting        netls_server + 0x11
#define netls_non_matching_tid     netls_server + 0x12
#define netls_bad_entry            netls_server + 0x13
#define netls_bad_timestamp        netls_server + 0x14
#define netls_vnd_not_in_db        netls_server + 0x15 
#define netls_duplicate_vendor     netls_server + 0x16
#define netls_duplicate_product    netls_server + 0x17
#define netls_no_such_product      netls_server + 0x18
#define netls_still_has_products   netls_server + 0x19
#define netls_not_valid            netls_server + 0x1A
#define netls_no_such_vendor       netls_server + 0x1B
#define netls_database_corrupt     netls_server + 0x1C
#define netls_new_pname_unexpected netls_server + 0x1D
#define netls_cant_delete_netls_v    netls_server + 0x1E
#define netls_fatal_error          netls_server + 0x1F 
#define netls_invalid_client       netls_server + 0x20

#define netls_no_init           netls_library + 0x01
#define netls_no_queues         netls_library + 0x02
#define netls_not_bound         netls_library + 0x03
#define netls_invalid_svr       netls_library + 0x04
#define netls_no_families       netls_library + 0x05
#define netls_no_svrs_fnd       netls_library + 0x06
#define netls_invalid_vid       netls_library + 0x07
#define netls_bad_param         netls_library + 0x08
#define netls_invalid_job_id    netls_library + 0x09

#define netls_decode_bad_version   netls_tools  + 0x1
#define netls_bad_password         netls_tools  + 0x2
#define netls_wrong_target         netls_tools  + 0x3   
#define netls_bad_pword_ver        netls_tools  + 0x4

#endif

/* vrsn array length (includes space for null character) */
#define ncf_VLEN 12      

/* product name array length (includes space for null character) */
#define ncf_product_len 32





/*********/
/* TYPES */
/*********/

/* needed typedefs */
typedef long ncf_time_t;
typedef uuid__string_t ncfuuid_string_t;
typedef uuid__t ncf_job_id_t;


/****************************/
/* n c f l i b _ i n i t () */
/****************************/
            
extern void ncflib_init(
#ifdef __STDC__
ncfuuid_string_t vnd_id,   
long             vnd_key,
ncf_job_id_t     *job_id,
long             *status
#endif
);

/* 
    ACTION: creates a job id for the process.  Locates servers that service
            this vendor's objects. 

    IN: 
        vnd_id -  the uuid text that is given in nlspass.

        vnd_key - two byte key to use in time stamp protection alg.


    OUT: 
        job_id - job id for this job. 

        status - set to status_$ok if no error, otherwise see below.


    POSSIBLE STATUS:
        status_$ok - ok.          

        netls_no_families - fatal error in trying to find the socket 
        family on the client host.
  
        netls_no_svrs_fnd - no license servers were found in the NCS
        locating broker for the particular vendor (vid_str).
  
        netls_bad_param - one of the paramters to this call was not
        correct.
  
        netls_invalid_vid - bad uuid text for the vendor id
*/


/*******************************/
/* n c f l i b _ r e q u e s t */
/*******************************/

extern long ncflib_request(
#ifdef __STDC__
ncf_job_id_t *job_id,
long         prod_id,
char         vrsn[ncf_VLEN],
long         num_lics,
long         queue,
ncf_time_t   chk_per,
long         *lic_trans_id,
long         *status
#endif
);
/*
    ACTION: tries to obtain a license for the specified application.

    RETURNS: the license handle for this transaction.

    IN: 
        job_id - job id for this job.

        prod_id - identifier of the product that is being requested.   
        Identifier is defined by the vendor.

        vrsn - the vendor defined product version text.

        num_lics - the number of licenses being requested.

        queue - indicates:
                 TRUE: return immediately and queue request if the specified
                 number of license units are not available

                 FALSE: if the requested number of license units are available,
                 return immediately, but do not queue the request.

        chk_per - the amount of time between check calls in seconds; after 
        this time the license server will return the license to the free pool 
        if the application has not checked in.                                       

    OUT: 
        lic_trans_id - this is a reference to a "black box" which nlslib uses to 
        store information about a request/wait/release transaction for
        a particular license.  Each request/wait/release for a different 
        license should use a different license transaction id.

        status - set to status_$ok if no error, otherwise see below.
                                                            

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
        netls_req_and_wait() to be placed in queue.

        netls_no_lics - no licenses are available at present.

        netls_bad_io - fatal error - should not occur; check log file
        for details.

        netls_bad_timestamp - license server system's time of day is too 
        different from local system's.

        netls_no_init - call netls_init() first.

        netls_invalid_svr - the server was not recognized as being an
        authentic license server.

        netls_bad_param - one of the paramters to this call was not
        correct.

*/                     



/*****************************************/
/* n c f l i b _ w a i t _ s t a t u s() */
/*****************************************/

extern long ncflib_wait_status(
#ifdef __STDC__
long        lic_trans_id,
ncf_time_t  chk_per,
long        *queue_pos,
long        *status
#endif
);
/*
    ACTION: checks reservation queues to see if license is available.
    RETURNS: the smallest number of users ahead of the caller in a queue.

    IN: 
        lic_trans_id - this is a reference to a "black box" which nlslib uses to 
        store information about a request/wait/release transaction for
        a particular license.  Each request/wait/release for a different 
        license should use a different license transaction id.

        chk_per - the amount of time (in seconds) before the license system
        will consider the application to have died.

    OUT:
        queue_pos - denotes the smallest number of users ahead of the caller
        in a queue. Note that in the case where multiple queues exist, the
        shortest value is returned.
        
        status - set to status_$ok if no error, otherwise see below.


    POSSIBLE STATUS:
        status_$ok - ok.
        
        netls_lic_not_fnd - license was not found in license server
        data base.

        netls_no_version - all licenses have the wrong version.

        netls_wait_entry_deleted - wait queue entry was deleted, probably
        because application did not wait_stat() within two minutes and
        was removed because it was thought to be dead.

        netls_bad_io - fatal error - should not occur; check log file
        for details.

        netls_still_waiting - licenses still not available; user is still
        on waiting queue. 

        netls_bad_timestamp - license server system's time of day is too 
        different from local system's.

        netls_no_queues - user is not waiting on any queues; probably 
        did not call netls_req_and_wait().

        netls_no_init - call netls_init() first.

        netls_invalid_svr - the server was not recognized as being an
        authentic license server.

        netls_bad_param - one of the paramters to this call was not
        correct.

*/








/******************************************/
/* n c f l i b _ w a i t _ r e m o v e () */
/******************************************/

extern void ncflib_wait_remove(
#ifdef __STDC__
long lic_trans_id,
long *status
#endif
);
/*
    ACTION: removes a specific request from the reservation queue.

    IN: 
        lic_trans_id - this is a reference to a "black box" which nlslib uses to 
        store information about a request/wait/release transaction for
        a particular license.  Each request/wait/release for a different 
        license should use a different license transaction id.

    OUT
        status - set to status_$ok if no error, otherwise see below.


    POSSIBLE STATUS:
        status_$ok - ok.
        
        netls_lic_not_fnd - license was not found in license server
        data base.

        netls_no_version - all licenses have the wrong version.

        netls_wait_entry_deleted - wait queue entry was deleted, probably
        because application did not wait_stat() within two minutes and
        was removed because it was thought to be dead.

        netls_bad_io - fatal error - should not occur; check log file
        for details.

        netls_bad_timestamp - license server system's time of day is too 
        different from local system's.

        netls_no_init - call netls_init() first.

        netls_no_queues - user is not waiting on any queues; probably 
        did not call netls_req_and_wait().

        netls_bad_param - one of the paramters to this call was not
        correct.

*/

/************************************/
/* n c f l i b _ r e l e a s e () */
/************************************/


extern void ncflib_release(
#ifdef __STDC__
long lic_trans_id,
long *status
#endif
);
/* 
    ACTION: informs the license system that an application is finished  
    with a license. Invalidates the license handle.
          

    IN: 
        lic_trans_id - this is a reference to a "black box" which nlslib uses to 
        store information about a request/wait/release transaction for
        a particular license.  Each request/wait/release for a different 
        license should use a different license transaction id.

    OUT
        status - set to status_$ok if no error, otherwise see below.


    POSSIBLE STATUS:
        status_$ok - ok.
        
        netls_lic_not_fnd - license was not found in license server
        data base.

        netls_no_version - all licenses have the wrong version.

        netls_bad_io - fatal error - should not occur; check log file
        for details.

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

*/

/**********************************/
/* n c f l i b _ c o n f i r m () */
/**********************************/
        
extern long ncflib_confirm(
#ifdef __STDC__
long       lic_trans_id,
ncf_time_t chk_per,
long       *status
#endif
);                   

/*
    ACTION: informs the lisence system that a lisence is still in use,
    and resets the check period. The next concurrent usage call must
    be issued with chk_per seconds.

    RETURNS: true if license server acknowledges affirmatively, false 
    otherwise.  Software vendor can decide on appropriate action for 
    a return of false.

    IN: 
        lic_trans_id - this is a reference to a "black box" which nlslib uses to 
        store information about a request/wait/release transaction for
        a particular license.  Each request/wait/release for a different 
        license should use a different license transaction id.

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

        netls_bad_io - fatal error - should not occur; check log file
        for details.

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

*/


/**********************************************/
/* n c f l i b _ g e t _ c u r _ u s e r s () */
/**********************************************/

extern void ncflib_get_cur_users(
#ifdef __STDC__
ncf_job_id_t *job_id,
long         prod_id,
char         vrsn[ncf_VLEN],
long         *next,
long         max_entries,
long         *total_lics,
long         *number_of_entries,
char         u_name[][32],
char         n_name[][32],
char         g_name[][32],
ncf_time_t   start_time[],
long         num_of_lic_units[],
long         *status
#endif
);

/* ACTION: returns the current users of a product, or optionally,
   the users queued to use the product.

    IN:                    
        job_id - unique identifier for vendor.

        prod_id - the product id of the product.

        vrsn - the version of the product.

        max_entries - the maximum number of entries which may be 
        retrieved by this call.

    OUT:
        total_lics - the total number of licences installed for the
        specified product.

        number_of_entries - the number of entries retrieved by this call.
        If zero or less than maximum entries, the end of list has been reached.

        u_name - an array of 32 byte character strings; user names of the
        current users.

        n_name - an array of 32 byte character strings; node names of the
        current users.

        g_name - an array of 32 byte character strings; group names of the
        current users.                                      

        start_time - the time the given user began using the product.

        num_of_lic_units - the number of license units held by this user.

        status - set to status_$ok if no error, otherwise see below.

          
    IN/OUT:
        next - indicates the position in the set of returning values.

    POSSIBLE STATUS:
        status_$ok - ok.
        
        netls_lic_not_fnd - license was not found in license server
        data base.

        netls_no_version - all licenses have the wrong version.

        netls_bad_io - fatal error - should not occur; check log file
        for details.

        netls_no_init - call netls_init() first.

        netls_bad_param - one of the paramters to this call was not
        correct.

*/

/*****************************************/                 
/* n c f l i b _ l o g _ m e s s a g e ()*/
/*****************************************/                 

extern void ncflib_log_message(
#ifdef __STDC__
ncf_job_id_t *job_id,
long         length, 
char         text[128],
long         *status
#endif
);
/*  ACTION: logs a vendor specific message in a special license system
    log file.
       
    IN:
        job_id - job id of the calling program.

        length - the number of bytes of text to be written in this message.

        text - the actual message that is to be logged. 

    OUT:
        status - code indicating level of success of call.

    POSSIBLE STATUS:
        status_$ok - ok.
        
        netls_bad_io - fatal error - should not occur; check log file
        for details.

        netls_vnd_not_in_db - vendor was not found in data base at server;
        message not printed at that server.

        netls_no_init - call netls_init() first.

        netls_bad_param - one of the paramters to this call was not
        correct.

*/

/**************************************/
/* n c f l i b _ t e r m i n a t e () */
/**************************************/

extern void ncflib_terminate(
#ifdef __STDC__
ncf_job_id_t *job_id,
long *status
#endif
);

