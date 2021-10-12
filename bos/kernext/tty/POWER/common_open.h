/* @(#)02 1.4 src/bos/kernext/tty/POWER/common_open.h, sysxcommon, bos412, 9447A 11/11/94 14:26:54 */
/*
 *  
 * COMPONENT_NAME: (sysxtty) Open disciplines for streams driver
 *  
 * FUNCTIONS:
 *  
 * ORIGINS: 83
 *  
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/stream.h> /* For redefinition of e_sleep with pse_sleep */

#ifndef _H_OPENDISC
#define _H_OPENDISC

enum openDiscType {     /* Available open disciplines */
    DTRO_OPEN,          /* dtropen discipline */
    WTO_OPEN,           /* wtopen discipline */
    OTHER_OPEN
};

#define OPEN_SLEEP(aaa) (e_sleep((aaa), EVENT_SIGRET) == EVENT_SIG)
#define OPEN_WAKEUP(bbb) e_wakeup(bbb)

struct openDisc_entry {
    char name[16];
    int  (* open) ();        /* open routine */
    int  (* close) ();       /* close routine */
    int  (* input) ();       /* input routine */
    int  (* output) ();      /* output routine */
    int  (* service) ();     /* service routine */
};

typedef struct openDisc * openDiscp_t;

/* BE CAREFUL: type Field MUST be the first field of openDisc structure */
struct openDisc {
    enum openDiscType    type;
    caddr_t              driverStruct;
    int                  (* ddservice) ();
};



#endif /* _H_OPENDISC */
