static char sccsid[] = "@(#)21  1.12  src/bos/usr/lib/methods/cfgtty/set_termios.c, cfgtty, bos41J, 9521B_all 5/26/95 07:49:28";
/*
 * COMPONENT_NAME: (CFGTTY) Common functions for DDS builds
 *
 * FUNCTIONS: set_cc_tty, set_cflag, set_flow_disp,
 *            set_mode, set_termios, in_termios
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>          /* standard I/O */
#include <cf.h>             /* error messages */
#include <string.h>         /* string manipulation */

#include <sys/cfgdb.h>      /* config #define */
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>     /* config structures */
#include <termio.h>    /* termios structure */

#include "cfgdebug.h"
#include "ttycfg.h"

/*
 * ==============================================================================
 * common extern functions
 * ==============================================================================
 */
    extern int getatt();

/*
 * ==============================================================================
 * defines and structures
 * ==============================================================================
 */
/*
 * -------------------
 * ODM attribute names
 * -------------------
 */
/* for termios.c_cc */
#define INTR_ATT            "intr"
#define QUIT_ATT            "quit"
#define ERASE_ATT           "erase"
#define KILL_ATT            "kill"
#define EOF_ATT             "eof"
#define EOL_ATT             "eol"
#define EOL2_ATT            "eol2"
#define START_ATT           "start"
#define STOP_ATT            "stop"
#define SUSP_ATT            "susp"
#define DSUSP_ATT           "dsusp"
#define REPRINT_ATT         "reprint"
#define DISCARD_ATT         "discard"
#define WERASE_ATT          "werase" 
#define LNEXT_ATT           "lnext"

/* for flow discipline (termios.c_iflag or termiox.x_hflag) */
#define RESET_FLOW_DISP     "none"

#define END_OF_STRING       '\0'
#define SEPARATOR           ","

/*
 * -----------------------------------------------------------------------------
 * common structures for termios flags
 * -----------------------------------------------------------------------------
 */
/* Structure which lists all attributes which are used */
/* to build the termios structure. All 'XXX_ATT' defines are used */
/* Be careful: This structure must be updated if some new attributes */
/* ---------- are added or deleted in ODM database */
static char * termios_odm_att[] = {
/* for termios.c_cc */
    INTR_ATT,
    QUIT_ATT,
    ERASE_ATT,
    KILL_ATT,
    EOF_ATT,
    EOL_ATT,
    EOL2_ATT,
    START_ATT,
    STOP_ATT,
    SUSP_ATT,
    DSUSP_ATT,
    REPRINT_ATT,
    DISCARD_ATT,
    WERASE_ATT, 
    LNEXT_ATT,
/* for termios.c_cflag */
    BAUDRATE_ATT,
    BPC_ATT,
    STOPS_ATT,
    PARITY_ATT,
/* for termios.c_iflag */
    FLOW_DISP_ATT,
/* for termios.c_iflag, termios.c_oflag, termios.c_cflag, termios.c_lflag */
    RUNMODES_ATT,
    NULL
};

/* Structure used to define imodes (c_iflag), omodes (c_oflag), */
/* cmodes (c_cflag) and lmodes (c_lflag) */
struct which_mode {
    char *   name;
    tcflag_t set;
    tcflag_t reset;
};

/*
 * -----------------------------------------------------------------------------
 * structure for termios.c_cc (control characters)
 * -----------------------------------------------------------------------------
 */
/* Structure used to find which c_cc exist */
static struct convert valid_cc[] = {
    {INTR_ATT,    VINTR},
    {QUIT_ATT,    VQUIT},
    {ERASE_ATT,   VERASE},
    {KILL_ATT,    VKILL},
    {EOF_ATT,     VEOF},
    {EOL_ATT,     VEOL},
    {EOL2_ATT,    VEOL2},
    {START_ATT,   VSTART},
    {STOP_ATT,    VSTOP},
    {SUSP_ATT,    VSUSP},
    {DSUSP_ATT,   VDSUSP},
    {REPRINT_ATT, VREPRINT},
    {DISCARD_ATT, VDISCRD},
    {WERASE_ATT,  VWERSE},
    {LNEXT_ATT,   VLNEXT},
    {NULL,      0}
};

/*
 * -----------------------------------------------------------------------------
 * structures for termios.c_cflag
 * -----------------------------------------------------------------------------
 */
/* SPEED */
static struct convert convert_speed[] = {
    {"0",     B0},
    {"50",    B50},
    {"75",    B75},
    {"110",   B110},
    {"134",   B134},
    {"150",   B150},
    {"200",   B200},
    {"300",   B300},
    {"600",   B600},
    {"1200",  B1200},
    {"1800",  B1800},
    {"2400",  B2400},
    {"4800",  B4800},
    {"9600",  B9600},
    {"19200", B19200},
    {"38400", B38400},
    {"134.5", B134},            /* alternates go down here */
    {"19.2",  B19200},
    {"38.4",  B38400},
    {"exta",  B19200},
    {"extb",  B38400},
    {NULL,    0}
};
/* BPC */
static struct convert convert_bpc[] = {
    {"5",     CS5},
    {"6",     CS6},
    {"7",     CS7},
    {"8",     CS8},
    {NULL,    0}
};
/* STOPS */
static struct convert convert_stops[] = {
    {"1",     0},
    {"2",     CSTOPB},
    {NULL,    0}
};
/* PARITY */
static struct convert convert_parity[] = {
    {"none",  0},
    {"even",  PARENB},
    {"odd",   PARENB|PARODD},
    {"mark",  PARENB|PARODD|PAREXT},
    {"space", PARENB|PAREXT},
    {"ext",   PAREXT},
    {NULL,    0}
};
/* Structure used to find which c_cflag exist */
static struct which_attribute valid_cflag[] = {
    {BAUDRATE_ATT, &convert_speed},
    {BPC_ATT,      &convert_bpc},
    {STOPS_ATT,    &convert_stops},
    {PARITY_ATT,   &convert_parity},
    {NULL,         NULL}
};

/*
 * -----------------------------------------------------------------------------
 * structure for flow discipline in termios.c_iflag
 * This structure is used here only for flow discipline
 * -----------------------------------------------------------------------------
 */
/* Structure used to convert flow discipline attribute values */
static struct convert convert_flow_disp[] = {
    {"xon", IXON|IXOFF},
    {NULL,  0}
};
/* Structure used to find which flow discipline exist */
static struct which_attribute valid_flow_disp[] = {
    {FLOW_DISP_ATT, &convert_flow_disp},
    {NULL,          NULL}
};

/*
 * -----------------------------------------------------------------------------
 * structure for imodes
 * -----------------------------------------------------------------------------
 */
static struct which_mode imodes[] = {
    {"ignbrk",      IGNBRK,     0},
    {"-ignbrk",     0,          IGNBRK},
    {"brkint",      BRKINT,     0},
    {"-brkint",     0,          BRKINT},
    {"ignpar",      IGNPAR,     0},
    {"-ignpar",     0,          IGNPAR},
    {"parmrk",      PARMRK,     0},
    {"-parmrk",     0,          PARMRK},
    {"inpck",       INPCK,      0},
    {"-inpck",      0,          INPCK},
    {"istrip",      ISTRIP,     0},
    {"-istrip",     0,          ISTRIP},
    {"inlcr",       INLCR,      0},
    {"-inlcr",      0,          INLCR},
    {"igncr",       IGNCR,      0},
    {"-igncr",      0,          IGNCR},
    {"icrnl",       ICRNL,      0},
    {"-icrnl",      0,          ICRNL},
    {"ixon",        IXON,       0},
    {"-ixon",       0,          IXON},
    {"ixoff",       IXOFF,      0},
    {"-ixoff",      0,          IXOFF},
    {"ixany",       IXANY,      0},
    {"-ixany",      0,          IXANY},
    {"iuclc",       IUCLC,      0},
    {"-iuclc",      0,          IUCLC},
    {"imaxbel",     IMAXBEL,    0},
    {"-imaxbel",    0,          IMAXBEL},
    {NULL,          0,          0}
};

/*
 * -----------------------------------------------------------------------------
 * structure for omodes
 * -----------------------------------------------------------------------------
 */
static struct which_mode omodes[] = {
    {"opost",       OPOST,      0},
    {"-opost",      0,          OPOST},
    {"olcuc",       OLCUC,      0},
    {"-olcuc",      0,          OLCUC},
    {"onlcr",       ONLCR,      0},
    {"-onlcr",      0,          ONLCR},
    {"ocrnl",       OCRNL,      0},
    {"-ocrnl",      0,          OCRNL},
    {"onocr",       ONOCR,      0},
    {"-onocr",      0,          ONOCR},
    {"onlret",      ONLRET,     0},
    {"-onlret",     0,          ONLRET},
    {"ofill",       OFILL,      0},
    {"-ofill",      0,          OFILL},
    {"ofdel",       OFDEL,      0},
    {"-ofdel",      0,          OFDEL},
    {"cr0",         CR0,        CRDLY},
    {"cr1",         CR1,        CRDLY},
    {"cr2",         CR2,        CRDLY},
    {"cr3",         CR3,        CRDLY},
    {"tab0",        TAB0,       TABDLY},
    {"tab1",        TAB1,       TABDLY},
    {"tab2",        TAB2,       TABDLY},
    {"tab3",        TAB3,       TABDLY},
    {"bs0",         BS0,        BSDLY},
    {"bs1",         BS1,        BSDLY},
    {"ff0",         FF0,        FFDLY},
    {"ff1",         FF1,        FFDLY},
    {"nl0",         NL0,        NLDLY},
    {"nl1",         NL1,        NLDLY},
    {"vt0",         VT0,        VTDLY},
    {"vt1",         VT1,        VTDLY},
    {NULL,          0,          0}
};

/*
 * -----------------------------------------------------------------------------
 * structure for cmodes
 * -----------------------------------------------------------------------------
 */
/* All the attributes which already exist */
/* as ODM attribute are refused here to */
/* avoid mismatch between RUNMODES_ATT */
/* and other attributes */
static struct which_mode cmodes[] = {
    /* "cs5", "cs6", "cs7", "cs8" are refused at configuration time */
    /* because the character size is set with the "bpc" */
    /* attribute */
    /* "cstopb" is refused at configuration time */
    /* because the stop bits are set with the "stops" */
    /* attribute */
    {"cread",       CREAD,      0},
    {"-cread",      0,          CREAD},
    /* "parend", "parodd" are refused at configuration time */
    /* because the parity is set with the "parity" */
    /* attribute */
    {"hupcl",       HUPCL,      0},
    {"-hupcl",      0,          HUPCL},
    {"clocal",      CLOCAL,     0},
    {"-clocal",     0,          CLOCAL},
    /* "parext" is refused at configuration time */
    /* because the parity is set with the "parity" */
    /* attribute */
    {NULL,          0,          0}
};

/*
 * -----------------------------------------------------------------------------
 * structure for lmodes
 * -----------------------------------------------------------------------------
 */
static struct which_mode lmodes[] = {
    {"isig",        ISIG,       0},
    {"-isig",       0,          ISIG},
    {"icanon",      ICANON,     0},
    {"-icanon",     0,          ICANON},
    {"xcase",       XCASE,      0},
    {"-xcase",      0,          XCASE},
    {"echo",        ECHO,       0},
    {"-echo",       0,          ECHO},
    {"echoe",       ECHOE,      0},
    {"-echoe",      0,          ECHOE},
    {"echok",       ECHOK,      0},
    {"-echok",      0,          ECHOK},
    {"echonl",      ECHONL,     0},
    {"-echonl",     0,          ECHONL},
    {"noflsh",      NOFLSH,     0},
    {"-noflsh",     0,          NOFLSH},
    {"tostop",      TOSTOP,     0},
    {"-tostop",     0,          TOSTOP},
    {"echoctl",     ECHOCTL,    0},
    {"-echoctl",    0,          ECHOCTL},
    {"echoprt",     ECHOPRT,    0},
    {"-echoprt",    0,          ECHOPRT},
    {"echoke",      ECHOKE,     0},
    {"-echoke",     0,          ECHOKE},
    {"flusho",      FLUSHO,     0},
    {"-flusho",     0,          FLUSHO},
    {"altwerase",   ALTWERASE,  0},
    {"-altwerase",  0,          ALTWERASE},
    {"pending",     PENDIN,     0},
    {"-pending",    0,          PENDIN},
    {"iexten",      IEXTEN,     0},
    {"-iexten",     0,          IEXTEN},
    {NULL,          0,          0}

};

/*
 * =============================================================================
 *                       LOCAL UTILITY ROUTINES
 * =============================================================================
 * There are different handlings for TTY and SPTR.
 * For TTY, all ODM attributes exist to build the whole termios structure.
 * For SPTR, only the "c_cflag" and the "flow_disp" attributes are
 * significant. For other fields, default values are used.
 * =============================================================================
 */
/*
 * -----------------------------------------------------------------------------
 *                       SET_CC_TTY
 * -----------------------------------------------------------------------------
 * 
 * This function takes a pointer to a termios structure
 * and uses ODM database to update its c_cc field.
 * All the control characters must be found in the ODM database.
 *
 * This routine is to be used only within this file.
 * The device specific routines for the various device
 * specific config methods  must not call this function.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
static int set_cc_tty(cusDevPtr, termiosPtr, attrList)
struct CuDv *    cusDevPtr;
struct termios * termiosPtr;
struct attr_list *  attrList;
{
    extern int getatt();
    char current_char[ATTRVALSIZE];               /* To store found char */
    cc_t cc_t_char;
    int  return_code,scratch;

    struct convert * current_cc;

    return_code = 0;
    current_cc = valid_cc;


    /* While there are attributes and they are found */
    while ((current_cc->att_value != NULL)
           && !(return_code = getatt(attrList, current_cc->att_value,
                                     &current_char, 's', &scratch))) {
        DEBUG_2("set_cc_tty: '%s' found for '%s' character\n",
                current_char, current_cc->att_value);
        /* Convert displayable string into uchar value */
        cc_t_char = current_char[0];
        if (cc_t_char == '^') {
            cc_t_char = current_char[1];
            if (cc_t_char == '?') {
                cc_t_char = 0177;
            }
            else if (cc_t_char == '-') {
                cc_t_char = 0377;
            }
            else {
                cc_t_char &= 037;
            }
        }; /* End if (cc_t_char == '^') */
        termiosPtr->c_cc[current_cc->define_value] = cc_t_char;
        current_cc++;
    } /* End while ((current_cc->att_value != NULL) && ...))) */

    /* Check for errors */
    if (return_code) {
        DEBUG_1("set_cc_tty: %s attribute not found.\n", current_cc->att_value);
        return(return_code);
    }
    else {
        /* That's OK */
        return(0);
    }
} /* End static int set_cc_tty(...) */

/*
 * -----------------------------------------------------------------------------
 *                       SET_CFLAG
 * -----------------------------------------------------------------------------
 * 
 * This function takes a pointer to a termios structure
 * and uses ODM database to update its c_cflag field.
 * This function is used for TTY and SPTR.
 *
 * This routine is to be used only within this file.
 * The device specific routines for the various device
 * specific config methods  must not call this function.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
static int set_cflag(cusDevPtr, termiosPtr, attrList)
struct CuDv *    cusDevPtr;
struct termios * termiosPtr;
struct attr_list *    attrList;
{
    int  att_value_found;    /* if attribute value is found */
    int  return_code,scratch;
    char current_value[ATTRVALSIZE];
    char * cflag_value;

    struct which_attribute * current_att;
    struct convert *         current_convert;

    return_code = 0;
    att_value_found = 1;
    current_att = valid_cflag;
    /* While there are attributes, they are found and available value is found */
    while ((current_att->att_name != NULL)
           && att_value_found
           && !(return_code = getatt(attrList, current_att->att_name,
                                     &current_value, 's', &scratch))) {
            /*use only first entry in list*/
            cflag_value = strtok(current_value, SEPARATOR);
            DEBUG_2("set_cflag: %s found = %s\n",
                    current_att->att_name, cflag_value);
            /* Check to see if some available value is found and */
            /* and use of corresponding termios #define to set cflag */
            att_value_found = 0;
            current_convert = current_att->convert_table;
            /* While there are available values and none matches */
            while ((current_convert->att_value != NULL) && !att_value_found) {
                if (!strcmp(cflag_value, current_convert->att_value)) {
                    /* Special handling for BAUDRATE_ATT attribute */
                    if (!strcmp(current_att->att_name, BAUDRATE_ATT)) {
                        cfsetospeed(termiosPtr, current_convert->define_value);
                        cfsetispeed(termiosPtr, current_convert->define_value);
                    }
                    else {
                        termiosPtr->c_cflag |= current_convert->define_value;
                    } /* End if (!strcmp(current_odm_att->value, BAUDRATE_ATT)) */
                    att_value_found = 1;
                    current_att++;
                }
                else {
                    current_convert++;
                }
            } /* End while ((... != NULL) && !att_value_found) */
    } /* End while ((current_att->att_name != NULL) && att_value_found ....))) */

    /* Check for errors */
    if (!att_value_found) {
        DEBUG_1("set_cflag: Unavailable value for %s.\n", current_att->att_name);
        return(E_BADATTR);
    };
    if (return_code) {
        DEBUG_1("set_cflag: %s attribute not found.\n", current_att->att_name);
        return(return_code);
    }
    else {
        /* That's OK */
        return(0);
    }
} /* End static int set_cflag(...) */

/*
 * -----------------------------------------------------------------------------
 *                       SET_FLOW_DISP
 * -----------------------------------------------------------------------------
 * 
 * This function takes a pointer to a tcflag_t field
 * and uses ODM database to update it.
 * This function is used for TTY and SPTR.
 *
 * This routine is to be used within this file and for termiox structure updating.
 * The device specific routines for the various device
 * specific config methods must not call this function.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
int set_flow_disp(cusDevPtr, tcflagPtr, attrList, validFlowDisp)
struct CuDv  *   cusDevPtr;
tcflag_t     *   tcflagPtr;
struct attr_list  *   attrList;
struct which_attribute * validFlowDisp;
{
    int    att_value_found;    /* if attribute value is found */
    int    return_code,scratch;
    char   current_value[ATTRVALSIZE];
    char * current_flow;

    struct which_attribute * current_att;   /* current attribute */
    struct convert *         current_convert;

    /* ================= */
    /* Set termios iflag */
    /* ================= */
    return_code = 0;
    att_value_found = 1;
    current_att = validFlowDisp;
    /* While there are attributes and they are found */
    while ((current_att->att_name != NULL)
           && !(return_code = getatt(attrList, current_att->att_name,
                                     &current_value, 's', &scratch))) {
        DEBUG_2("set_flow_disp: %s found = %s\n",
                current_att->att_name, current_value);
        /* Check to see if some available value is found and */
        /* and use of corresponding #define to set value */
        /* pointed by tcflagPtr */
        if (strstr(current_value, RESET_FLOW_DISP) == NULL) {
            /* RESET_FLOW_DISP value is not found. So we can proceed */
            DEBUG_1("set_flow_disp: %s is not found\n", RESET_FLOW_DISP);
            current_flow = strtok(current_value, SEPARATOR);

            /* While there are values to search */
            /* We're processing all values */
            while (current_flow != NULL) {
                att_value_found = 0;
                current_convert = current_att->convert_table;
		DEBUG_1("set_flow_disp: %s is found\n", current_flow);

                /* While there are available values and none matches */
                while ((current_convert->att_value != NULL) && !att_value_found) {
                    if (!strcmp(current_flow, current_convert->att_value)) {
			DEBUG_1("set_flow_disp: %s equality is found\n",
				 current_flow);
			DEBUG_2("set_flow_disp: %s value = %d\n", current_flow,
				 current_convert->define_value);
                        (*tcflagPtr) |= (tcflag_t) current_convert->define_value;
                        att_value_found = 1;
                    }
                    else {
                        current_convert++;
                    }
                } /* End while ((... != NULL) && !att_value_found) */
                /* I don't return with error even if att_value_found */
                /* is still 0 because flow discipline can be set either */
                /* with termios, either with termiox structure */
                current_flow = strtok(NULL, SEPARATOR);
            } /* End while (current_flow != NULL) */
        }
        else {
            /* RESET_FLOW_DISP is found ===> Nothing to do */
            DEBUG_1("set_flow_disp: %s is found ===> No flow discipline\n",
                    RESET_FLOW_DISP);
        } /* End if (strstr(current_value, RESET_FLOW_DISP) == NULL) */
        current_att++;
    } /* End while ((current_att->att_name != NULL) ......))) */

    /* Check for errors */
    if (return_code) {
        DEBUG_1("set_flow_disp: %s attribute not found.\n", current_att->att_name);
        return(return_code);
    };

    return(0);
} /* End static int set_flow_disp(...) */

/*
 * -----------------------------------------------------------------------------
 *                       SET_MODE
 * -----------------------------------------------------------------------------
 * 
 * This function searches for a given mode in an
 * array.
 * This function is used to scan all the modes of
 * the termios structure. It is called from the "set_termios"
 * function.
 *
 * This routine is to be used only within this file.
 * The device specific routines for the various device
 * specific config methods  must not call this function.
 *
 * Return code: 1 if the given mode is found, 0 otherwise.
 * -----------------------------------------------------------------------------
 */
static int set_mode(whichModePtr, whichFlagPtr, modePtr)
struct which_mode * whichModePtr;
tcflag_t * whichFlagPtr;
char * modePtr;
{
    struct which_mode * current_mode;
    int    att_value_found;

    att_value_found = 0;
    current_mode = whichModePtr;
    /* Check to see if some available value is found and */
    /* and use of corresponding termios #define to set */
    /* the whichFlagPtr */
    while ((current_mode->name != NULL) && !att_value_found) {
        if (!strcmp(current_mode->name, modePtr)) {
            DEBUG_1("set_mode: '%s' is found\n",
                    current_mode->name);
            *whichFlagPtr &= ~current_mode->reset;
            *whichFlagPtr |= current_mode->set;
            att_value_found = 1;
        }
        else {
            current_mode++;
        } /* End if (!strcmp(current_mode->name, modePtr)) */
    } /* End while ((current_mode->name != NULL) && !att_value_found)  */

    return (att_value_found);
} /* End static int set_mode(...) */

/*
 * =============================================================================
 *                       SET_TERMIOS
 * =============================================================================
 * This function takes a pointer to a termios structure
 * and uses ODM database to update it.
 *
 * Control characters (c_cc) are set with the set_cc_tty function for TTY,
 * without any default control characters for SPTR,
 * specific attributes (BAUDRATE_ATT, bpc, stops, parity) are set with the set_cflag
 * function and flow control discipline is set with the set_flow_disp function.
 * Flow control discipline can be set with termiox or
 * termios structure. So, I need to drop a flag if
 * a flow control discipline is found for termios
 * structure.
 * All control characters, specific attributes and flow control discipline
 * attributes must be found, otherwise an error is returned.
 *
 * Run modes are taken from RUNMODES_ATT attribute. Each of them is
 * searched within imodes, omodes, cmodes and lmodes.
 * All run modes must be found, otherwise an error is returned.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */
int set_termios(cusDevPtr, termiosPtr, attrList)
struct CuDv *    cusDevPtr;
struct termios * termiosPtr;
struct attr_list *   attrList;
{
    int    att_value_found;         /* if attribute value is found */
    int    return_code,scratch;
    int    cc_ind;                  /* Only for DEBUG trace */

    char   run_modes_att[ATTRVALSIZE];  /* RUNMODES_ATT attribute */
    char * mode_ptr;                /* Pointer on mode which is found */
                                    /* in the RUNMODES_ATT attribute */


    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    bzero((char *)termiosPtr, sizeof(struct termios));

    /* ================================================================ */
    /* To set control characters, c_cflag (for BAUDRATE_ATT, bpc, stops and */
    /* parity) and c_iflag (for flow discipline) */
    /* ================================================================= */


    if (!strncmp(cusDevPtr->PdDvLn_Lvalue, TTY_PDDV_CLASS, strlen(TTY_PDDV_CLASS))) {
        if (return_code = set_cc_tty(cusDevPtr, termiosPtr, attrList)) {
            return(return_code);
        };
    }
	else {
		/* For Printer, only CSTART and CSTOP characters are used */
		/* and there are no such ODM attributs ==> I get "termio.h" */
		/* values */
		termiosPtr->c_cc[VSTOP] = CSTOP;
		termiosPtr->c_cc[VSTART] = CSTART;
	}

    if (return_code = set_cflag(cusDevPtr, termiosPtr, attrList)) {
        return(return_code);
    };
    if (return_code = set_flow_disp(cusDevPtr, &(termiosPtr->c_iflag), attrList,
				&valid_flow_disp)) {
        return(return_code);
    };

    /* =============================================================== */
    /* To set c_iflag, c_oflag, c_cflag, c_lflag depending on runmodes */
    /* There are runmodes only for TTY, none for SPTR */
    /* =============================================================== */
    if (!strncmp(cusDevPtr->PdDvLn_Lvalue, TTY_PDDV_CLASS, strlen(TTY_PDDV_CLASS))) {

        if ((return_code = getatt(attrList,RUNMODES_ATT, &run_modes_att,
				's',&scratch)) == 0){
            DEBUG_2("set_termios: %s found = %s\n",
                    RUNMODES_ATT, run_modes_att);

            /* Use of local variable to decode RUNMODES_ATT attribute */
            mode_ptr = strtok(run_modes_att, SEPARATOR);
            att_value_found = 1;
            /* While there are value to search and they are found */
            while ((mode_ptr != NULL) && att_value_found) {
                /* If the current mode is not found as imode, */
                /* neither as omode, neither as cmode, */
                /* neither as lmode, we return an error, */
                /* else we can search for an other mode. */
                /* As soon as we have found the mode, we stop */
                /* the search. */
                if (set_mode(imodes, &termiosPtr->c_iflag, mode_ptr)
                    || set_mode(omodes, &termiosPtr->c_oflag, mode_ptr)
                    || set_mode(cmodes, &termiosPtr->c_cflag, mode_ptr)
                    || set_mode(lmodes, &termiosPtr->c_lflag, mode_ptr)) {
                    mode_ptr = strtok(NULL, SEPARATOR);
                }
                else {
                    att_value_found = 0;
                } /* End if (set_mode(...) || ...) */
            } /* End while ((... != NULL) && att_value_found) */
        }
        else {
            DEBUG_1("set_termios: %s attribute not found.\n", RUNMODES_ATT);
            return(return_code);
        } /* End if ((return_code = getatt(...)) != 0) */

        /* Check for errors */
        if (!att_value_found) {
            DEBUG_2("set_termios: Unavailable value %s for %s.\n",
                    mode_ptr, RUNMODES_ATT);
            return(E_BADATTR);
        };
    }; /* End if (!strncmp(cusDevPtr->PdDvLn_Lvalue, TTY_PDDV_CLASS, strlen(TTY_PDDV_CLASS))) */

    DEBUG_0("set_termios: Dump of the built termios structure\n");
    DEBUG_1("\tc_iflag = 0x%2x\n", termiosPtr->c_iflag);
    DEBUG_1("\tc_oflag = 0x%2x\n", termiosPtr->c_oflag);
    DEBUG_1("\tc_cflag = 0x%2x\n", termiosPtr->c_cflag);
    DEBUG_1("\tc_lflag = 0x%2x\n", termiosPtr->c_lflag);
    for (cc_ind = 0; cc_ind < NCCS; cc_ind++) {
        DEBUG_2("\tc_cc[%d] = 0x%x\n", cc_ind, termiosPtr->c_cc[cc_ind]);
    }


    return(0);
} /* End int set_termios(...) */

/*
 * =============================================================================
 *                       IN_TERMIOS
 * =============================================================================
 * This function takes a pointer to an attribute list.
 * It searches if at least one attribute of this list concerns the termios structure.
 *
 * Return code: 0 if no attribute is to be used the termios structure, 1 otherwise.
 * =============================================================================
 */
int in_termios(attrList)
struct attr *    attrList;
{
    char ** current_termios_att;

    current_termios_att = termios_odm_att;
    while (*current_termios_att != NULL) {
        if (att_changed(attrList, *current_termios_att) != NULL) {
			DEBUG_1("in_termios: %s attribute changed\n",
					*current_termios_att);
            return(1);
        };
		current_termios_att++;
    }
return(0);
} /* End int in_termios(...) */
