/* @(#)81	1.6  src/bos/usr/bin/que/digest.h, cmdque, bos411, 9431A411a 8/2/94 08:09:03 */
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: digest
 *
 * ORIGINS: 9, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/* Note:  digest.h was extracted from digest.c so that qadm/lsque.c and
 *        qadm/lsquedev.c could access common data structures.
 */


/* the different field names */
#define         FDEVICE         1
#define         FDISC           2
#define         FUP             3
#define         FACCTF          4
#define         FFILE           5
#define         FACCESS         6
#define         FFEED           7
#define         FHEAD           8
#define         FTRAIL          9
#define         FBACKEND        10
#define         FQNAME          11
#define         FDNAME          12
#define         FALIGN          13
#define         FHOSTID         14
#define         FRQ             15
#define         FSROS           16
#define         FLROS           17


/* the different value names */
#define VTRUE       1
#define VFALSE      2
#define VFCFS       3
#define VSJN        4
#define VNEVER      5
#define VALWAYS     6
#define VGROUP      7
#define VWRITE      8
#define VBOTH       9
#define VDEFAULT    10
#define VNUMBER     11
#define VTEXT       12
#define VNONE       "none"


/*----These constants are used to get members from the 'fnames[]' table */
#define DEVICE_INDEX		 0
#define DISCIPLINE_INDEX	 1
#define UP_INDEX		 2
#define ACCTFILE_INDEX		 3
#define FILE_INDEX		 4
#define ACCESS_INDEX		 5
#define FEED_INDEX		 6
#define HEADER_INDEX		 7
#define TRAILER_INDEX		 8
#define BACKEND_INDEX		 9
#define ALIGN_INDEX		10
#define HOST_INDEX		11
#define S_STATFILTER_INDEX	12
#define L_STATFILTER_INDEX	13
#define RQ_INDEX		14


/* used as the name field in the SMIT colon output. */
#define QUEUE_NAME	"name"


/* a structure for getting values from text */
struct namtab
{       char	*name;
        int	val;
	char	*default_value;
	char	*valstr;
};


/* table of field names (except FQNAME and FDNAME) */
struct namtab fnames[] =
{
        "device",       FDEVICE,	"",		"",
        "discipline",   FDISC,		"fcfs",		"",
        "up",           FUP,		"TRUE",		"",
        "acctfile",     FACCTF,		"FALSE",	"",
        "file",         FFILE,		"FALSE",	"",
        "access",       FACCESS,	"write",	"",
        "feed",         FFEED,		"never",	"",
        "header",       FHEAD,		"never",	"",
        "trailer",      FTRAIL,		"never",	"",
        "backend",      FBACKEND,	"",		"",
        "align",        FALIGN,		"FALSE",	"",
        "host",         FHOSTID,	"",		"",
        "s_statfilter", FSROS,		"",		"",
        "l_statfilter", FLROS,		"",		"",
        "rq",           FRQ,		"",		"",
        0,              0,		0,		0
};


/* table of value names (except VTEXT and VNUMBER) */
struct namtab vnames[] =
{       "TRUE",         VTRUE,		"",		"",
        "FALSE",        VFALSE,		"",             "",
        "fcfs",         VFCFS,		"",             "",
        "sjn",          VSJN,		"",             "",
        "never",        VNEVER,		"",             "",
        "always",       VALWAYS,	"",             "",
        "group",        VGROUP,		"",             "",
        "write",        VWRITE,		"",             "",
        "both",         VBOTH,		"",             "",
        "default",      VDEFAULT,	"",             "",
        0,              0,		0,		0
};


/*
 * a table of legal values for each field.  END signifies the end
 * of the list.  the value VTEXT is followed by the max length of
 * the text + 1, or 0 if no max.  VNUMBER is followed by the max number,
 * or 0 if no max.  also, this tells which kind of stanza the
 * field may legally appear in.
 */
#define END -1          /* an impossible value number */

struct legalvals
{       char field;
        char devstanza;
        int  value[4];      /* same type as vp in lread() */
} lv[] =
{
        FDEVICE,        FALSE,   VTEXT,  0,          END,    END,
        FDISC,          FALSE,   VFCFS,  VSJN,       END,    END,
        FUP,            FALSE,   VTRUE,  VFALSE,     END,    END,
        FACCTF,         FALSE,   VFALSE, VTEXT,      ACCTF,  END,
        FFILE,          TRUE,    VFALSE, VTEXT,      DEVFILE,END,
        FACCESS,        TRUE,    VWRITE, VBOTH,      END,    END,
        FFEED,          TRUE,    VNEVER, VNUMBER,    MAXFEED,END,
        FHEAD,          TRUE,    VNEVER, VGROUP,     VALWAYS,END,
        FTRAIL,         TRUE,    VNEVER, VGROUP,     VALWAYS,END,
        FBACKEND,       TRUE,    VTEXT,  BENAME,     END,    END,
        FQNAME,         FALSE,   VTEXT,  (QNAME +1), END,    END,
        FDNAME,         TRUE,    VTEXT,  (DNAME +1), END,    END,
        FALIGN,         TRUE,    VTRUE,  VFALSE,     END,    END,
        FHOSTID,        FALSE,   VTEXT,  HOST_SZ,    END,    END,
        FSROS,          FALSE,   VTEXT,  OSSIZE,     END,    END,
        FLROS,          FALSE,   VTEXT,  OSSIZE,     END,    END,
        FRQ,            FALSE,   VTEXT,  (QNAME +1), END,    END,
        0,              0,       0,      0,          0,      0
};


/* a structure for an attribute-value pair from config */
struct av
{       short atype;
        short vtype;
        union
        {   char *vtext;
            int  vnumber;
        } vval;
};

#define CONFLINE  255       /* max length of line in config */
