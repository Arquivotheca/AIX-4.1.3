/* @(#)70   1.2 src/bos/kernext/inputdd/inc/ktsmtrace.h, inputdd, bos41J, 9519A_all 5/9/95 07:25:11 */
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard/Tablet/Sound/Mouse DD - ktsmtrace.h
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define GS_USE_SYS_TRACE_TIMESTAMP  /* enable time stamps                   */
#include <graphics/gs_trace.h>

GS_MODULE(inputdd);                 /* create external reference to         */
                                    /* variables used by trace macros       */

/*  note: "inputdd" is the module name and represents a string that is      */
/*        appended to other characters to form a label. Normally one would  */
/*        use a #define for this but the compiler will not expand it        */
/*        correctly. "inputdd" is not used outside of this header file.     */


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
/*        (eg: code "send" instead of "hkwd_GS_kbdconfig_send")             */
/*--------------------------------------------------------------------------*/

enum  gs_inputdd_hkwd {

/* common                                                                   */
    hkwd_GS_put_iq                              = 0x001,
        hkwd_GS_put_iq_enter                         = hkwd_GS_ENTER,
        hkwd_GS_put_iq_exit                          = hkwd_GS_EXIT,
    hkwd_GS_put_oq                              = 0x002,
        hkwd_GS_put_oq_enter                         = hkwd_GS_ENTER,
        hkwd_GS_put_oq_exit                          = hkwd_GS_EXIT,
        hkwd_GS_put_oq_mid                           = 1,
    hkwd_GS_get_oq                              = 0x003,
        hkwd_GS_get_oq_enter                         = hkwd_GS_ENTER,
        hkwd_GS_get_oq_exit                          = hkwd_GS_EXIT,
    hkwd_GS_get_iq                              = 0x005,
        hkwd_GS_get_iq_enter                          = hkwd_GS_ENTER,
        hkwd_GS_get_iq_exit                           = hkwd_GS_EXIT,
    hkwd_GS_wait_oq                             = 0x006,
        hkwd_GS_wait_oq_enter                         = hkwd_GS_ENTER,
        hkwd_GS_wait_oq_exit                          = hkwd_GS_EXIT,
        hkwd_GS_wait_oq_one                           = 1,
    hkwd_GS_ktsm_sleep                          = 0x007,
        hkwd_GS_ktsm_sleep_enter                     = hkwd_GS_ENTER,
        hkwd_GS_ktsm_sleep_exit                      = hkwd_GS_EXIT,
    hkwd_GS_ktsm_putring                        = 0x008,
        hkwd_GS_ktsm_putring_enter                   = hkwd_GS_ENTER,
        hkwd_GS_ktsm_putring_exit                    = hkwd_GS_EXIT,
        hkwd_GS_ktsm_putring_header                  = 1,
    hkwd_GS_ktsm_log                            = 0x009,
        hkwd_GS_ktsm_log_enter                       = hkwd_GS_ENTER,
        hkwd_GS_ktsm_log_exit                        = hkwd_GS_EXIT,
    hkwd_GS_ktsm_uring                          = 0x00a,
        hkwd_GS_ktsm_uring_enter                     = hkwd_GS_ENTER,
        hkwd_GS_ktsm_uring_exit                      = hkwd_GS_EXIT,
    hkwd_GS_put_oq1                             = 0x00b,
        hkwd_GS_put_oq1_enter                        = hkwd_GS_ENTER,
    hkwd_GS_put_oq2                             = 0x00c,
        hkwd_GS_put_oq2_enter                        = hkwd_GS_ENTER,
    hkwd_GS_startwd                             = 0x00d,
        hkwd_GS_startwd_enter                        = hkwd_GS_ENTER,
    hkwd_GS_ktsm_rring                          = 0x00e,
        hkwd_GS_ktsm_rring_enter                     = hkwd_GS_ENTER,
        hkwd_GS_ktsm_rring_exit                      = hkwd_GS_EXIT,
        hkwd_GS_ktsm_rring_header                    = 1,
    hkwd_GS_ktsm_rflush                         = 0x00f,
        hkwd_GS_ktsm_rflush_enter                    = hkwd_GS_ENTER,

/* keyboard                                                                 */
    hkwd_GS_kbdconfig                           = 0x020,
        hkwd_GS_kbdconfig_enter                      = hkwd_GS_ENTER,
        hkwd_GS_kbdconfig_exit                       = hkwd_GS_EXIT,
    hkwd_GS_kbdmpx                              = 0x021,
        hkwd_GS_kbdmpx_enter                         = hkwd_GS_ENTER,
        hkwd_GS_kbdmpx_exit                          = hkwd_GS_EXIT,
    hkwd_GS_kbdopen                             = 0x022,
        hkwd_GS_kbdopen_enter                        = hkwd_GS_ENTER,
        hkwd_GS_kbdopen_exit                         = hkwd_GS_EXIT,
    hkwd_GS_kbdclose                            = 0x023,
        hkwd_GS_kbdclose_enter                       = hkwd_GS_ENTER,
        hkwd_GS_kbdclose_exit                        = hkwd_GS_EXIT,
    hkwd_GS_kbdioctl                            = 0x024,
        hkwd_GS_kbdioctl_enter                       = hkwd_GS_ENTER,
        hkwd_GS_kbdioctl_exit                        = hkwd_GS_EXIT,
    hkwd_GS_kbdintr                             = 0x025,
        hkwd_GS_kbdintr_enter                        = hkwd_GS_ENTER,
        hkwd_GS_kbdintr_exit                         = hkwd_GS_EXIT,
    hkwd_GS_k_send_q_frame                      = 0x026,
        hkwd_GS_k_send_q_frame_enter                 = hkwd_GS_ENTER,
        hkwd_GS_k_send_q_frame_exit                  = hkwd_GS_EXIT,
        hkwd_GS_k_send_q_frame_one                   = 1,
    hkwd_GS_k_watch_dog                         = 0x027,
        hkwd_GS_k_watch_dog_enter                     = hkwd_GS_ENTER,
        hkwd_GS_k_watch_dog_exit                      = hkwd_GS_EXIT,
        hkwd_GS_k_watch_dog_one                       = 1,
        hkwd_GS_k_watch_dog_two                       = 2,
    hkwd_GS_kbdsetup                            = 0x028,
        hkwd_GS_kbdsetup_enter                       = hkwd_GS_ENTER,
        hkwd_GS_kbdsetup_exit                        = hkwd_GS_EXIT,
    hkwd_GS_keyproc                             = 0x029,
        hkwd_GS_keyproc_enter                        = hkwd_GS_ENTER,
        hkwd_GS_keyproc_exit                         = hkwd_GS_EXIT,
    hkwd_GS_shift_status                        = 0x02a,
        hkwd_GS_shift_status_enter                   = hkwd_GS_ENTER,
        hkwd_GS_shift_status_exit                    = hkwd_GS_EXIT,
    hkwd_GS_proc_sak                            = 0x02b,
        hkwd_GS_proc_sak_enter                       = hkwd_GS_ENTER,
        hkwd_GS_proc_sak_exit                        = hkwd_GS_EXIT,
        hkwd_GS_proc_sak_callback                    = 1,
    hkwd_GS_un_sak                              = 0x02c,
        hkwd_GS_un_sak_enter                         = hkwd_GS_ENTER,
        hkwd_GS_un_sak_exit                          = hkwd_GS_EXIT,
    hkwd_GS_put_key                             = 0x02e,
        hkwd_GS_put_key_enter                        = hkwd_GS_ENTER,
        hkwd_GS_put_key_exit                         = hkwd_GS_EXIT,
    hkwd_GS_poll_appl                           = 0x02f,
        hkwd_GS_poll_appl_enter                      = hkwd_GS_ENTER,
        hkwd_GS_poll_appl_exit                       = hkwd_GS_EXIT,
    hkwd_GS_put_sq                              = 0x030,
        hkwd_GS_put_sq_enter                         = hkwd_GS_ENTER,
        hkwd_GS_put_sq_exit                          = hkwd_GS_EXIT,
        hkwd_GS_put_sq_qe                            = 1,
    hkwd_GS_next_sound                          = 0x031,
        hkwd_GS_next_sound_exit                      = hkwd_GS_EXIT,
    hkwd_GS_appl_killer                         = 0x032,
        hkwd_GS_appl_killer_enter                    = hkwd_GS_ENTER,
        hkwd_GS_appl_killer_exit                     = hkwd_GS_EXIT,
    hkwd_GS_proc_event                          = 0x033,
        hkwd_GS_proc_event_enter                    = hkwd_GS_ENTER,
        hkwd_GS_proc_event_exit                     = hkwd_GS_EXIT,
    hkwd_GS_keyioctl                            = 0x034,
        hkwd_GS_keyioctl_poll                       = 1,
        hkwd_GS_keyioctl_leds                       = 2,
        hkwd_GS_keyioctl_clicker                    = 3,
        hkwd_GS_keyioctl_volume                     = 4,
        hkwd_GS_keyioctl_trate                      = 5,
        hkwd_GS_keyioctl_tdelay                     = 6,
        hkwd_GS_keyioctl_diag                       = 7,
    hkwd_GS_sv_proc                             = 0x035,
        hkwd_GS_sv_proc_enter                        = hkwd_GS_ENTER,
        hkwd_GS_sv_proc_exit                         = hkwd_GS_EXIT,
    hkwd_GS_k_send_8042_cmd                     = 0x036,
        hkwd_GS_k_send_8042_cmd_enter                = hkwd_GS_ENTER,
        hkwd_GS_k_send_8042_cmd_exit                 = hkwd_GS_EXIT,

/* mouse                                                                     */
    hkwd_GS_mseconfig                           = 0x040,
        hkwd_GS_mseconfig_enter                      = hkwd_GS_ENTER,
        hkwd_GS_mseconfig_exit                       = hkwd_GS_EXIT,
    hkwd_GS_mseopen                             = 0x041,
        hkwd_GS_mseopen_enter                        = hkwd_GS_ENTER,
        hkwd_GS_mseopen_exit                         = hkwd_GS_EXIT,
    hkwd_GS_mseclose                            = 0x042,
        hkwd_GS_mseclose_enter                       = hkwd_GS_ENTER,
        hkwd_GS_mseclose_exit                        = hkwd_GS_EXIT,
    hkwd_GS_mseioctl                            = 0x043,
        hkwd_GS_mseioctl_enter                       = hkwd_GS_ENTER,
        hkwd_GS_mseioctl_exit                        = hkwd_GS_EXIT,
    hkwd_GS_mseintr                             = 0x044,
        hkwd_GS_mseintr_enter                        = hkwd_GS_ENTER,
        hkwd_GS_mseintr_exit                         = hkwd_GS_EXIT,
    hkwd_GS_m_send_cmd                          = 0x045,
        hkwd_GS_m_send_cmd_enter                     = hkwd_GS_ENTER,
        hkwd_GS_m_send_cmd_exit                      = hkwd_GS_EXIT,
        hkwd_GS_m_send_cmd_unblk                     = 1,
        hkwd_GS_m_send_cmd_busy                      = 2,
        hkwd_GS_m_send_cmd_blk                       = 3,
    hkwd_GS_m_send_q_frame                      = 0x046,
        hkwd_GS_m_send_q_frame_enter                 = hkwd_GS_ENTER,
        hkwd_GS_m_send_q_frame_exit                  = hkwd_GS_EXIT,
    hkwd_GS_m_watch_dog                         = 0x047,
        hkwd_GS_m_watch_dog_enter                     = hkwd_GS_ENTER,
        hkwd_GS_m_watch_dog_exit                      = hkwd_GS_EXIT,
    hkwd_GS_mseproc                             = 0x048,
        hkwd_GS_mseproc_event                         = 1,
    hkwd_GS_m_send_8042_cmd                     = 0x049,
        hkwd_GS_m_send_8042_cmd_enter                = hkwd_GS_ENTER,
        hkwd_GS_m_send_8042_cmd_exit                 = hkwd_GS_EXIT,

/* keyboard/tablet                                                          */
    hkwd_GS_ktsconfig                           = 0x050,
        hkwd_GS_ktsconfig_enter                      = hkwd_GS_ENTER,
        hkwd_GS_ktsconfig_exit                       = hkwd_GS_EXIT,
    hkwd_GS_ktsmpx                              = 0x051,
        hkwd_GS_ktsmpx_enter                         = hkwd_GS_ENTER,
        hkwd_GS_ktsmpx_exit                          = hkwd_GS_EXIT,
    hkwd_GS_ktsopen                             = 0x052,
        hkwd_GS_ktsopen_enter                        = hkwd_GS_ENTER,
        hkwd_GS_ktsopen_exit                         = hkwd_GS_EXIT,
    hkwd_GS_ktsclose                            = 0x053,
        hkwd_GS_ktsclose_enter                       = hkwd_GS_ENTER,
        hkwd_GS_ktsclose_exit                        = hkwd_GS_EXIT,
    hkwd_GS_ktsioctl                            = 0x054,
        hkwd_GS_ktsioctl_enter                       = hkwd_GS_ENTER,
        hkwd_GS_ktsioctl_exit                        = hkwd_GS_EXIT,
    hkwd_GS_ktsintr                             = 0x055,
        hkwd_GS_ktsintr_enter                        = hkwd_GS_ENTER,
        hkwd_GS_ktsintr_exit                         = hkwd_GS_EXIT,

/* tablet                                                                    */
    hkwd_GS_tabconfig                           = 0x060,
        hkwd_GS_tabconfig_enter                      = hkwd_GS_ENTER,
        hkwd_GS_tabconfig_exit                       = hkwd_GS_EXIT,
    hkwd_GS_tabopen                             = 0x061,
        hkwd_GS_tabopen_enter                        = hkwd_GS_ENTER,
        hkwd_GS_tabopen_exit                         = hkwd_GS_EXIT,
    hkwd_GS_tabclose                            = 0x062,
        hkwd_GS_tabclose_enter                       = hkwd_GS_ENTER,
        hkwd_GS_tabclose_exit                        = hkwd_GS_EXIT,
    hkwd_GS_tabioctl                            = 0x063,
        hkwd_GS_tabioctl_enter                       = hkwd_GS_ENTER,
        hkwd_GS_tabioctl_exit                        = hkwd_GS_EXIT,
    hkwd_GS_tabintr                             = 0x064,
        hkwd_GS_tabintr_enter                        = hkwd_GS_ENTER,
        hkwd_GS_tabintr_exit                         = hkwd_GS_EXIT,
    hkwd_GS_t_send_q_frame                      = 0x065,
        hkwd_GS_t_send_q_frame_enter                 = hkwd_GS_ENTER,
        hkwd_GS_t_send_q_frame_exit                  = hkwd_GS_EXIT,
    hkwd_GS_t_watch_dog                         = 0x066,
        hkwd_GS_t_watch_dog_enter                     = hkwd_GS_ENTER,
        hkwd_GS_t_watch_dog_exit                      = hkwd_GS_EXIT,
    hkwd_GS_tabproc                             = 0x067,
        hkwd_GS_tabproc_rcv                          = 1,

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

#define KTSMTRACE0(routine, func)                                            \
       GS_PARM_TRC0(HKWD_GS_INPUT_DD, inputdd, TRC_PRIORITY, routine,        \
                   routine ## _ ## func); 

#define KTSMTRACE1(routine, func, p1)                                        \
       GS_PARM_TRC1(HKWD_GS_INPUT_DD, inputdd, TRC_PRIORITY, routine,        \
                   routine ## _ ## func, ((ulong) p1)); 

#define KTSMTRACE(routine, func, p1, p2, p3, p4, p5)                         \
       GS_PARM_TRC(HKWD_GS_INPUT_DD, inputdd, TRC_PRIORITY, routine,         \
                   routine ## _ ## func, ((ulong) p1), ((ulong) p2),         \
                  ((ulong) p3), ((ulong) p4), ((ulong) p5));


#ifdef GS_DEBUG_TRACE

/* debug trace macros                                                       */
/*                                                                          */
/*   routine = routine ID                                                   */
/*   func    = sub function ID                                              */
/*   p1-p5   = trace data                                                   */

#define KTSMDTRACE0(routine, func)                                           \
       GS_PARM_DBG0(HKWD_GS_INPUT_DD, inputdd, TRC_PRIORITY, routine,        \
                   routine ## _ ## func); 

#define KTSMDTRACE1(routine, func, p1)                                       \
       GS_PARM_DBG1(HKWD_GS_INPUT_DD, inputdd, TRC_PRIORITY, routine,        \
                   routine ## _ ## func, ((ulong) p1));  

#define KTSMDTRACE(routine, func, p1, p2, p3, p4, p5)                        \
       GS_PARM_DBG(HKWD_GS_INPUT_DD, inputdd, TRC_PRIORITY, routine,         \
                   routine ## _ ## func, ((ulong) p1), ((ulong) p2),         \
                  ((ulong) p3), ((ulong) p4), ((ulong) p5)); 

#else
#define KTSMDTRACE0(routine, func)
#define KTSMDTRACE1(routine, func, p1)
#define KTSMDTRACE(routine, func, p1, p2, p3, p4, p5)
#endif
