/* @(#)98       1.4  src/bos/kernel/include/pse/str_funnel.h, sysxpse, bos412, 9443A412c 10/19/94 10:15:01 */
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 *
 * ORIGINS: 83
 *
 */

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */
#ifndef _STR_FUNNEL_
#define _STR_FUNNEL_

#include <sys/strconf.h>

/* defines funnelized  module or driver */
struct funnel_qinit {
        int     (*fqi_putp)(queue_t *, mblk_t *);
        void    (*fqi_srvp)(queue_t *);
        int     (*fqi_qopen)();
        int     (*fqi_qclose)();
        int     (*fqi_qadmin)(void);
        struct module_info * fqi_minfo;
        struct module_stat * fqi_mstat;
	struct qinit * fqi_qinit;
};

int funnel_init(struct streamtab **, struct streamadm *);
void funnel_term(struct streamtab *);
int funnel_open_V3(queue_t *, dev_t *, int, int);
int funnel_open_V4(queue_t *, dev_t *, int, int, cred_t *);
int funnel_close_V3(queue_t *);
int funnel_close_V4(queue_t *, cred_t *);
int funnel_putp(queue_t *, mblk_t *);
void funnel_sq_wrapper(queue_t *);

#endif /* _STR_FUNNEL_ */
