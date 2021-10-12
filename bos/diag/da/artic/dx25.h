/* @(#)73	1.1  src/bos/diag/da/artic/dx25.h, daartic, bos411, 9428A410j 4/1/94 15:18:31 */
/*
 *   COMPONENT_NAME: DAARTIC
 *
 *   FUNCTIONS: DIAG_NUM_ENTRIES
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


int reg_tu_seq[] =
{
        1, 18, 19, 3, 4, 5, 6, 7, 8, 10
};

int loop_tux21_seq [] =
{
        1, 18, 19, 3, 4, 5, 6, 7, 8, 14
};

int loop_tuv24_seq [] =
{
        1, 18, 19, 3, 4, 5, 6, 7, 8, 15
};

int loop_tuv35_seq [] =
{
        1, 18, 19, 3, 4, 5, 6, 7, 8, 16
};


/* fru_bucket is a structure that holds information for the diagnostic
   program to return to the diagnostic controller when a failure is found
   that needs to be reported. (FRU means Field Replacable Unit) */

struct fru_bucket frub[]=
{
        {"", FRUB1, 0x849, 0x210, R_X25_ADAPTER,
                {
                    {87, "", "", 0, DA_NAME, NONEXEMPT},
                    {13, "DRAM Sip", "00-00-00", F_X25_DRAM, NOT_IN_DB, EXEMPT},
                },
        },

        {"", FRUB1, 0x849, 0x101, R_X25_ADAPTER,
                {
                        {90, "", "", 0, DA_NAME, NONEXEMPT},
                        {10, "", "", 0, PARENT_NAME, NONEXEMPT},
                },
        },

        {"", FRUB1, 0x849, 0x101, R_X25_ADAPTER,
                {
                        {100, "", "", 0, DA_NAME, NONEXEMPT},
                },
        },

        {"", FRUB1, 0x849, 0x250, R_X25_DRAM,
                {
                    {90, "DRAM Sip", "00-00-00", F_X25_DRAM, NOT_IN_DB, EXEMPT},
                    {10, "", "", 0, DA_NAME, NONEXEMPT},
                },
        },

        {"", FRUB1, 0x849, 0x140, R_X21_CABLE,
                {
                {95, "X21 Cable", "00-00-00", F_X21_CABLE, NOT_IN_DB, EXEMPT},
                {5, "", "", 0, DA_NAME, NONEXEMPT},
                },
        },

        {"", FRUB1, 0x849, 0x150, R_V24_CABLE,
                {
                {95, "V24 Cable", "00-00-00", F_V24_CABLE, NOT_IN_DB, EXEMPT},
                {5, "", "", 0, DA_NAME, NONEXEMPT},
                },
        },

        {"", FRUB1, 0x849, 0x160, R_V35_CABLE,

                {
                {95, "V35 Cable", "00-00-00", F_V35_CABLE, NOT_IN_DB, EXEMPT},
                {5, "", "", 0, DA_NAME, NONEXEMPT},
                },
        },

        {"", FRUB1, 0x849, 0x330, R_ELA,
                {
                        {90, "", "", 0, DA_NAME, NONEXEMPT},
                        {10, "", "", 0, PARENT_NAME, NONEXEMPT},
                },
        },
};


struct msglist plug_37[] = {
                                {Q_PLUG_37_PIN,Q_PLUG_37_PIN_TITLE},
                                {Q_PLUG_37_PIN,Q_PLUG_37_PIN_YES},
                                {Q_PLUG_37_PIN,Q_PLUG_37_PIN_NO},
                                {Q_PLUG_37_PIN,Q_PLUG_37_PIN_ACTION},
                                (char) NULL
};

struct msglist which_int[] = {
                                {Q_WHICH_INT,Q_WHICH_INT_TITLE},
                                {Q_WHICH_INT,Q_WHICH_INT_X21},
                                {Q_WHICH_INT,Q_WHICH_INT_V24},
                                {Q_WHICH_INT,Q_WHICH_INT_V35},
                                {Q_WHICH_INT,Q_WHICH_INT_NO_IDEA},
                                {Q_WHICH_INT,Q_WHICH_INT_ACTION},
                                (char) NULL
};

struct msglist intx21_or_not[] = {
                                     {Q_INTX21_OR_NOT,Q_INTX21_OR_NOT_TITLE},
                                     {Q_INTX21_OR_NOT,Q_INTX21_OR_NOT_YES},
                                     {Q_INTX21_OR_NOT,Q_INTX21_OR_NOT_NO},
                                     {Q_INTX21_OR_NOT,Q_INTX21_OR_NOT_ACTION},
                                     (char) NULL
};

struct msglist intv24_or_not[] = {
                                     {Q_INTV24_OR_NOT,Q_INTV24_OR_NOT_TITLE},
                                     {Q_INTV24_OR_NOT,Q_INTV24_OR_NOT_YES},
                                     {Q_INTV24_OR_NOT,Q_INTV24_OR_NOT_NO},
                                     {Q_INTV24_OR_NOT,Q_INTV24_OR_NOT_ACTION},
                                     (char) NULL
};

struct msglist intv35_or_not[] = {
                                     {Q_INTV35_OR_NOT,Q_INTV35_OR_NOT_TITLE},
                                     {Q_INTV35_OR_NOT,Q_INTV35_OR_NOT_YES},
                                     {Q_INTV35_OR_NOT,Q_INTV35_OR_NOT_NO},
                                     {Q_INTV35_OR_NOT,Q_INTV35_OR_NOT_ACTION},
                                     (char) NULL
};

#define IS_CONSOLE  ( (int) (da_input.console == CONSOLE_TRUE) )

static ASL_SCR_INFO q_plug_37[DIAG_NUM_ENTRIES(plug_37)];
static ASL_SCR_INFO q_which_int[DIAG_NUM_ENTRIES(which_int)];
static ASL_SCR_INFO q_intx21_or_not[DIAG_NUM_ENTRIES(intx21_or_not)];
static ASL_SCR_INFO q_intv24_or_not[DIAG_NUM_ENTRIES(intv24_or_not)];
static ASL_SCR_INFO q_intv35_or_not[DIAG_NUM_ENTRIES(intv35_or_not)];
static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

TUTYPE          artic_tucb;       /* pointer to the previous structure */
struct errdata  err_data;       /* error log information */
struct CuAt    *cuat;           /* pointer to CuAt */
struct stat     tmpbuf;         /* tmp buffer for stat system call */

int             rc;             /* to hold functions return codes    */
int             i;              /* loop counter */
int             change_code;
int             envflag;
int             diskette_based = DIAG_FALSE;
unsigned int    rcode_val;      /* determine which rcode to display  */
unsigned long   attr_value;
char           *crit;           /* buffer */
char           *dname;
char            no_rcm_msg[512];

extern  int     filedes;         /* file descriptor from DD */
extern  char    diag_cat_gets ();
extern          getdainput();
extern          addfrub();
/* extern          exectu(); */
unsigned int    dtoh();


/*---------------------------------------*/
extern struct tm_input da_input;

extern       nl_catd diag_catopen (char *, int);
extern       nl_catd catd;
nl_catd      x25_catd;


FILE *trf;  /* global trace file */
char           slot[25];               /* slot where the device is plugged */
char           temp_str[256];           /* used for substitution */


int		current_plug = 0;	/* current wrap plug */
int		plug_name = 0;		/* dummy variable  */
int		interface = 0;         /* X21, V24, V35  or 0 for don't know */

