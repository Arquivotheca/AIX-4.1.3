/* @(#)27	1.8  src/bos/diag/da/msla/dslacfg.h, damsla, bos411, 9428A410j 12/10/92 09:05:05 */
/*
 *   COMPONENT_NAME: DAMSLA
 *
 *   FUNCTIONS: Diagnostic header file.
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1990,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define CFG_GSW_CMD 	"mkdev -l "
#define CFG_HIA_CMD 	"mkdev -l " 
#define CFG_PARENT 	"mkdev -l " 
#define UNCFG_CMD 	"rmdev -l "
#define UNDEF_CMD 	"rmdev -d -l "
#define DEF_GSW_CMD 	"mkdev -t gsw -c msla -s msla -w 0 -d -p "
#define DEF_CFG_GSW_CMD "mkdev -t gsw -c msla -s msla -w 0  -p "
#define DEF_HIA_CMD 	"mkdev -t hia -c msla -s msla -w 0 -d -p "

#define GSW_PREFIX	"gsw"
#define HIA_PREFIX	"hia"
#define	DEVPATH		"/dev/"

#define S_LENGTH 	256
#define MAX_CMD_LENGTH 	200
#define MAX_PATH_LENGTH 20
#define NOT_PRESENT	-1

struct  hia_conf  {		/* This structure will save the original */
    int  num_sessions;		/* data configuration of the HIA before  */
    int  buffer_size;		/* unconfiguring it. Then after running  */
    int  lower_bond;		/* diagnostics over it we can restore in */
    int  link_speed;		/* its original state. 			 */
    int  num_5080_sess;
    int  lower_5080_bond;
    int  upper_5080_bond;
    char  addr_5080_chan[9];
};

