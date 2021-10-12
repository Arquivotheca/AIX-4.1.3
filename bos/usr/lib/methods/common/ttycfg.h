/* @(#)26  1.11  src/bos/usr/lib/methods/common/ttycfg.h, cfgtty, bos41J, 9520A_all 4/27/95 13:35:02 */
/*
 * COMPONENT_NAME: (CFGTTY) Common defines for tty configuration
 *
 * FUNCTIONS:
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _H_TTYCFG
#define _H_TTYCFG

#include <sys/stropts.h>

#define TTY_PDDV_CLASS      "tty"

/* Drivers and modules names */
#define LC_SJLS_MODULE      "lc_sjls"
#define LDTERM_MODULE       "ldterm"
#define LION_DRIVER         "liondd"
#define CXMA_DRIVER	    "cxmadd"
#define CXIA_DRIVER	    "cxiadd"
#define NLS_MODULE          "nls"
#define PTY_MODULE          "ptydd"
#define RS_DRIVER           "rsdd"
#define SPTR_MODULE         "sptr"
#define TIOC_MODULE         "tioc"
#define UC_SJLS_MODULE      "uc_sjls"
#define RSPC_DRIVER	    "isa/rsdd_rspc"
#define PCMCIA_DRIVER   "pcmcia/pcrsdd"

struct for_euc {
	int existing;
	char lower[FMNAMESZ+1];
	char upper[FMNAMESZ+1];
};

/* Structure used to convert attribute value in */
/* termios or termiox #define value */
struct convert {
    char * att_value;       /* Attribute value to convert */
    int    define_value;    /* termios/termiox #define value */
};                          /* corresponding with attribute value */

struct which_attribute {
    char * att_name;        /* Attribute name to find in ODM database */
    struct convert * convert_table; /* Structure to be used for conversion */
};

/* Some ODM attributes which may be used in several TTY configuration functions */
/* These defines MUST be in coherence with ODM database attribute names */
#define MODULES_ATT         "modules"
#define FLOW_DISP_ATT       "flow_disp"
#define RUNMODES_ATT        "runmodes"
#define BAUDRATE_ATT        "speed"
#define BPC_ATT             "bpc"
#define STOPS_ATT           "stops"
#define PARITY_ATT          "parity"
#define RTRIG_ATT           "rtrig"
#define TBC16_ATT           "tbc16"
#define CSMAP_ATT           "csmap"
#define LOGIN_ATT           "login"

/*64 port adapter */
#define TBC64_ATT           "tbc64"
#define IN_XPAR_ATT         "xprint_on_str"
#define LV_XPAR_ATT         "xprint_off_str"
#define PRIORITY_ATT        "xprint_priority"

/* load module support */
#define LOAD_MODULE         "load_module"

#endif /* _H_TTYCFG */
