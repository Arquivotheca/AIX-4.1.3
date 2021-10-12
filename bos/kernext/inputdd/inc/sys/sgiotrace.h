/* @(#)59   1.3 src/bos/kernext/inputdd/inc/sys/sgiotrace.h, inputdd, bos41J, 9516B_all 4/18/95 16:10:11 */
/*
 * COMPONENT_NAME: (INPUTDD) SGIO DD - sgiotrace.h
 *
 * FUNCTIONS: none
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

#ifndef _H_SGIOTRACE
#define _H_SGIOTRACE

#define GS_USE_SYS_TRACE_TIMESTAMP  /* enable time stamps                   */
#include <graphics/gs_trace.h>

GS_MODULE(sgiodd);                 /* create external reference to         */
                                    /* variables used by trace macros       */

/*  note: "sgiodd" is the module name and represents a string that is      */
/*        appended to other characters to form a label. Normally one would  */
/*        use a #define for this but the compiler will not expand it        */
/*        correctly. "sgiodd" is not used outside of this header file.     */


/*--------------------------------------------------------------------------*/
/* routine and subfunction ID's                                             */
/*                                                                          */
/*  note: routine ID's are listed first followed (indented) by              */
/*        subfunction ID's for that routine                                 */
/*                                                                          */
/*        don't code "hkwd_GS_" string in trace macro, this is appended     */
/*        automatically                                                     */
/*                                                                          */
/*        don't code "hkwd_GS_" or routine ID label for subfunction ID's    */
/*        (eg: code "send" instead of "hkwd_GS_sgio_config_send")           */
/*--------------------------------------------------------------------------*/

enum  gs_sgiodd_hkwd {

    hkwd_GS_sgio_config                         = 0x001,
        hkwd_GS_sgio_config_enter                    = hkwd_GS_ENTER,
        hkwd_GS_sgio_config_exit                     = hkwd_GS_EXIT,
    hkwd_GS_sgio_open                           = 0x002,
        hkwd_GS_sgio_open_enter                      = hkwd_GS_ENTER,
        hkwd_GS_sgio_open_exit                       = hkwd_GS_EXIT,
    hkwd_GS_sgio_close                          = 0x003,
        hkwd_GS_sgio_close_enter                     = hkwd_GS_ENTER,
        hkwd_GS_sgio_close_exit                      = hkwd_GS_EXIT,
    hkwd_GS_sgio_ioctl                          = 0x004,
        hkwd_GS_sgio_ioctl_enter                     = hkwd_GS_ENTER,
        hkwd_GS_sgio_ioctl_exit                      = hkwd_GS_EXIT,
        hkwd_GS_sgio_ioctl_iocinfo                   = 1,
        hkwd_GS_sgio_ioctl_regring                   = 2,
        hkwd_GS_sgio_ioctl_rflush                    = 3,
        hkwd_GS_sgio_ioctl_dialsetgrand              = 4,
        hkwd_GS_sgio_ioctl_lpfklight                 = 5,
    hkwd_GS_add_dev                             = 0x005,
        hkwd_GS_add_dev_enter                        = hkwd_GS_ENTER,
        hkwd_GS_add_dev_exit                         = hkwd_GS_EXIT,
    hkwd_GS_remove_dev                          = 0x006,
        hkwd_GS_remove_dev_enter                     = hkwd_GS_ENTER,
        hkwd_GS_remove_dev_exit                      = hkwd_GS_EXIT,
    hkwd_GS_find_dev                            = 0x007,
        hkwd_GS_find_dev_enter                       = hkwd_GS_ENTER,
        hkwd_GS_find_dev_exit                        = hkwd_GS_EXIT,
    hkwd_GS_create_sikproc                      = 0x008,
        hkwd_GS_create_sikproc_enter                 = hkwd_GS_ENTER,
        hkwd_GS_create_sikproc_exit                  = hkwd_GS_EXIT,
    hkwd_GS_term_sikproc                        = 0x009,
        hkwd_GS_term_sikproc_enter                   = hkwd_GS_ENTER,
        hkwd_GS_term_sikproc_exit                    = hkwd_GS_EXIT,
    hkwd_GS_sikproc                             = 0x00a,
        hkwd_GS_sikproc_enter                        = hkwd_GS_ENTER,
        hkwd_GS_sikproc_exit                         = hkwd_GS_EXIT,
    hkwd_GS_poll_update                         = 0x00b,
        hkwd_GS_poll_update_enter                    = hkwd_GS_ENTER,
        hkwd_GS_poll_update_exit                     = hkwd_GS_EXIT,
    hkwd_GS_perform_io                          = 0x00c,
        hkwd_GS_perform_io_enter                     = hkwd_GS_ENTER,
        hkwd_GS_perform_io_exit                      = hkwd_GS_EXIT,
    hkwd_GS_dials_io                            = 0x00d,
        hkwd_GS_dials_io_enter                       = hkwd_GS_ENTER,
        hkwd_GS_dials_io_exit                        = hkwd_GS_EXIT,
    hkwd_GS_dials_read                          = 0x00e,
        hkwd_GS_dials_read_enter                     = hkwd_GS_ENTER,
        hkwd_GS_dials_read_exit                      = hkwd_GS_EXIT,
    hkwd_GS_lpfks_io                            = 0x00f,
        hkwd_GS_lpfks_io_enter                       = hkwd_GS_ENTER,
        hkwd_GS_lpfks_io_exit                        = hkwd_GS_EXIT,
    hkwd_GS_lpfks_read                          = 0x010,
        hkwd_GS_lpfks_read_enter                     = hkwd_GS_ENTER,
        hkwd_GS_lpfks_read_exit                      = hkwd_GS_EXIT,
    hkwd_GS_sikproc_query                       = 0x011,
        hkwd_GS_sikproc_query_enter                  = hkwd_GS_ENTER,
        hkwd_GS_sikproc_query_exit                   = hkwd_GS_EXIT,
    hkwd_GS_sikproc_retry                       = 0x012,
        hkwd_GS_sikproc_retry_enter                  = hkwd_GS_ENTER,
        hkwd_GS_sikproc_retry_exit                   = hkwd_GS_EXIT,
    hkwd_GS_sgio_log                            = 0x013,
        hkwd_GS_sgio_log_enter                       = hkwd_GS_ENTER,
        hkwd_GS_sgio_log_exit                        = hkwd_GS_EXIT,
    hkwd_GS_sgio_uring                          = 0x014,
        hkwd_GS_sgio_uring_enter                     = hkwd_GS_ENTER,
        hkwd_GS_sgio_uring_exit                      = hkwd_GS_EXIT,
    hkwd_GS_sgio_rring                          = 0x015,
        hkwd_GS_sgio_rring_enter                     = hkwd_GS_ENTER,
        hkwd_GS_sgio_rring_exit                      = hkwd_GS_EXIT,
        hkwd_GS_sgio_rring_header                    = 1,
    hkwd_GS_sgio_putring                        = 0x016,
        hkwd_GS_sgio_putring_enter                   = hkwd_GS_ENTER,
        hkwd_GS_sgio_putring_exit                    = hkwd_GS_EXIT,
        hkwd_GS_sgio_putring_header                  = 1,
    hkwd_GS_sgio_rflush                         = 0x017,
        hkwd_GS_sgio_rflush_enter                    = hkwd_GS_ENTER,
        hkwd_GS_sgio_rflush_exit                     = hkwd_GS_EXIT,
    hkwd_GS_reg_pm                              = 0x018,
        hkwd_GS_reg_pm_enter                         = hkwd_GS_ENTER,
        hkwd_GS_reg_pm_exit                          = hkwd_GS_EXIT,
    hkwd_GS_ureg_pm                             = 0x019,
        hkwd_GS_ureg_pm_enter                        = hkwd_GS_ENTER,
        hkwd_GS_ureg_pm_exit                         = hkwd_GS_EXIT,
    hkwd_GS_sgio_pm_handler                     = 0x01a,
        hkwd_GS_sgio_pm_handler_enter                = hkwd_GS_ENTER,
        hkwd_GS_sgio_pm_handler_exit                 = hkwd_GS_EXIT,
    hkwd_GS_sgio_tty_open                       = 0x01b,
        hkwd_GS_sgio_tty_open_enter                  = hkwd_GS_ENTER,
        hkwd_GS_sgio_tty_open_exit                   = hkwd_GS_EXIT,

    hkwd_GS_last_valid                          = 0x3ff
};

/*--------------------------------------------------------------------------*/
/* macros                                                                   */
/*--------------------------------------------------------------------------*/

#ifdef GS_DEBUG

#define panic(d) printf(d); printf("\n")

#define VERIFY(p) {if(!(p)){printf("[%s #%d]\n",__FILE__,__LINE__);         \
       panic("assert(p)");}}

#else

/* #define VERIFY(p) {if (!(p)) panic("assert(p)");} */

#define VERIFY(p)

#endif

/* trace point priorities                                                   */
#define TRC_PRIORITY 1

/* shipped trace macros                                                     */
/*                                                                          */
/*   routine = routine ID                                                   */
/*   func    = sub function ID                                              */
/*   p1-p5   = trace data                                                   */

#define SGIOTRACE0(routine, func)                                            \
       GS_PARM_TRC0(HKWD_GS_SGIO_DD, sgiodd, TRC_PRIORITY, routine,        \
                   routine ## _ ## func); 

#define SGIOTRACE1(routine, func, p1)                                        \
       GS_PARM_TRC1(HKWD_GS_SGIO_DD, sgiodd, TRC_PRIORITY, routine,        \
                   routine ## _ ## func, ((ulong) p1)); 

#define SGIOTRACE(routine, func, p1, p2, p3, p4, p5)                         \
       GS_PARM_TRC(HKWD_GS_SGIO_DD, sgiodd, TRC_PRIORITY, routine,         \
                   routine ## _ ## func, ((ulong) p1), ((ulong) p2),         \
                  ((ulong) p3), ((ulong) p4), ((ulong) p5));


#ifdef GS_DEBUG_TRACE

/* debug trace macros                                                       */
/*                                                                          */
/*   routine = routine ID                                                   */
/*   func    = sub function ID                                              */
/*   p1-p5   = trace data                                                   */

#define SGIODTRACE0(routine, func)                                           \
       GS_PARM_DBG0(HKWD_GS_SGIO_DD, sgiodd, TRC_PRIORITY, routine,        \
                   routine ## _ ## func); 

#define SGIODTRACE1(routine, func, p1)                                       \
       GS_PARM_DBG1(HKWD_GS_SGIO_DD, sgiodd, TRC_PRIORITY, routine,        \
                   routine ## _ ## func, ((ulong) p1));  

#define SGIODTRACE(routine, func, p1, p2, p3, p4, p5)                        \
       GS_PARM_DBG(HKWD_GS_SGIO_DD, sgiodd, TRC_PRIORITY, routine,         \
                   routine ## _ ## func, ((ulong) p1), ((ulong) p2),         \
                  ((ulong) p3), ((ulong) p4), ((ulong) p5)); 

#else
#define SGIODTRACE0(routine, func)
#define SGIODTRACE1(routine, func, p1)
#define SGIODTRACE(routine, func, p1, p2, p3, p4, p5)
#endif

#endif
