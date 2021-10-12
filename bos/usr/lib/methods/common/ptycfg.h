/* @(#)71 1.1 src/bos/usr/lib/methods/common/ptycfg.h, cfgmethods, bos411, 9428A410j 6/2/94 08:19:36 */
/*
 * COMPONENT_NAME: (CFGMETHODS) Common defines for pty configuration
 *
 * FUNCTIONS:
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _H_PTYCFG
#define _H_PTYCFG

/* To retrieve PTY object from common undefine routine */
#define PTY_UTYPE       "pty/pty/pty"

/* To create special files */
#define DEV_DIR             "/dev"
#define ATT_MASTER_FILE     "ptc"
#define ATT_SLAVE_DIR       "pts"

static char ptyp[] = "ptyXX";    /* path for pty files (BSD master) */
static char ttyp[] = "ttyXX";    /* path for tty files (BSD slave) */

static char seq1[] = "pqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static char seq2[] = "0123456789abcdef";

/* Dummy names for ATT and BSD slaves */
#define ATTDUMMY_MASTER     "ATTmaster"
#define ATTDUMMY_SLAVE      "ATTslave"
#define BSDDUMMY_SLAVE      "BSDslave"

#define ATTNUM_ATT          "ATTnum"
#define BSDNUM_ATT          "BSDnum"

#endif /* _H_PTYCFG */
