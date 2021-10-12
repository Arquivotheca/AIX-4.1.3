static char sccsid[] = "@(#)14  1.11.1.2  src/bos/usr/lib/pios/rmvirprt.c, cmdpios, bos411, 9428A410j 10/29/93 15:12:43";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*** rmvirprt.c ***/

#include <locale.h>
#include "virprt.h"
#include "smit_class.h"

#define CMD_HDR_STR         "sm_cmd_hdr"
#define CMD_OPT_STR         "sm_cmd_opt"

/**** the following macros are taken from piolimits.h ****/
#define STRGZARG(a)     #a
#define STRGZS(a)       STRGZARG(a)
#define MAXVNMLEN       20
#define MAXQNMLEN       20
#define FLGSCTNM        "__FLG"
#define SYSSCTNM        "__SYS"
#define PFLSCTNM        "__PFL"
#define FILSCTNM        "__FIL"
#define ODM_LOCKTIME        (5)
#define ODM_QPRT_SCHALL_CRIT    "id LIKE 'ps_qprt_%." STRGZS(MAXQNMLEN) "s.%." \
                STRGZS(MAXVNMLEN) "s.*'"
#define ODM_JOBATTRS_SCH_CRIT   "id = 'ps_jobattrs_%." STRGZS(MAXQNMLEN) "s.%."\
                STRGZS(MAXVNMLEN) "s'"
#define ODM_SETUP_SCH_CRIT  "id = 'ps_setup_%." STRGZS(MAXQNMLEN) "s.%." \
                STRGZS(MAXVNMLEN) "s'"
#define ODM_FILTERS_SCH_CRIT    "id = 'ps_filters_%." STRGZS(MAXQNMLEN) "s.%." \
                STRGZS(MAXVNMLEN) "s'"
#define ODM_FLG_SCO_CRIT    "id LIKE 'ps_%." STRGZS(MAXQNMLEN) "s.%." \
                STRGZS(MAXVNMLEN) "s." FLGSCTNM ".*'"
#define ODM_SYS_SCO_CRIT    "id LIKE 'ps_%." STRGZS(MAXQNMLEN) "s.%." \
                STRGZS(MAXVNMLEN) "s." SYSSCTNM ".*'"
#define ODM_PFL_SCO_CRIT    "id LIKE 'ps_%." STRGZS(MAXQNMLEN) "s.%." \
                STRGZS(MAXVNMLEN) "s." PFLSCTNM ".*'"
#define ODM_FIL_SCO_CRIT    "id LIKE 'ps_%." STRGZS(MAXQNMLEN) "s.%." \
                STRGZS(MAXVNMLEN) "s." FILSCTNM ".*'"

/****  function prototypes ****/
void rm_odm_objects(void);

/*===========================================================================*/
void main(argc,argv)
int argc;
char *argv[];
{
    (void) setlocale(LC_ALL, "");

   /* Convert New Flag Letters (if present) To Old Flag Letters (s->d, d->v) */
    { CNVTFLAGS }

    putenv("IFS=' \t\n'");
    make_files();           /* create full path names from PIOBASEDIR */

    if ( argc == 1 ) 
		err_sub(ABORT,USAGE_RM);

    if (parse(argc,argv,"q:v:","qv",0,&att,2,&pqname,&vpname))
        err_sub(ABORT,USAGE_RM);

    sprintf(cusfile,"%s%s:%s",cuspath,pqname,vpname);
    switch( file_stat(cusfile) )
        {
        case DZNTXST: err_sub(ABORT,CUS_NOXST);             /* doesn't exist */

        case PERM_OK:                           /* exists */
			rm_odm_objects();
            unlink(mkdigestname());
            unlink(cusfile);
            break;

        case PERM_BAD: err_sub(ABORT,CUS_NOPEN);            /* can't open */
        }

    exit(0);
}

/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           rm_odm_objects                                               *
*                                                                              *
* DESCRIPTION:    Removes a virtual printer's odm objects from the             *
*                 alternate ODM.                                               *
*                                                                              *
* PARAMETERS:     None.                                                        *
*                                                                              *
* RETURN VALUES:  None.                                                        *
*                                                                              *
*                                                                              *
*******************************************************************************/
void rm_odm_objects(void)
{
int							odmld;			/* ODM lock descriptor */
CLASS_SYMBOL				cmd_hdr;		/* sm_cmd_hdr class */
CLASS_SYMBOL				cmd_opt;		/* sm_cmd_opt class */
char						wbuf[1024];	/* tmp work buffer */


    /* initialize the odm */
	if (odm_initialize() == -1)
		err_sub(ABORT,MSG_RM_ERRODMINIT);

    /* set the odm path */
	if ((odm_set_path(odmpath)) == (char *)-1)
		err_sub(ABORT,MSG_RM_ERRODMSP);

	/* lock the odm */
	if ((odmld = odm_lock(odmpath,ODM_LOCKTIME)) == -1)
		err_sub(ABORT,MSG_RM_ERRODMLOCK);

	/* open the sm_cmd_hdr class */
	if ((int)(cmd_hdr = odm_open_class(sm_cmd_hdr_CLASS)) == -1)
	{
		(void)strcpy(cmd,CMD_HDR_STR);
		err_sub(ABORT,MSG_RM_ERRODMOPEN);
	}

	/* open the sm_cmd_opt class */
	if ((int)(cmd_opt = odm_open_class(sm_cmd_opt_CLASS)) == -1)
	{ 
		(void)strcpy(cmd,CMD_OPT_STR);
		err_sub(ABORT,MSG_RM_ERRODMOPEN);
	}

	(void)sprintf(wbuf,ODM_QPRT_SCHALL_CRIT,pqname,vpname); 
	(void)odm_rm_obj(cmd_hdr,wbuf); 
	(void)sprintf(wbuf,ODM_SETUP_SCH_CRIT,pqname,vpname); 
	(void)odm_rm_obj(cmd_hdr,wbuf); 
	(void)sprintf(wbuf,ODM_JOBATTRS_SCH_CRIT,pqname,vpname); 
	(void)odm_rm_obj(cmd_hdr,wbuf); 
	(void)sprintf(wbuf,ODM_FILTERS_SCH_CRIT,pqname,vpname); 
	(void)odm_rm_obj(cmd_hdr,wbuf); 
	(void)sprintf(wbuf,ODM_FLG_SCO_CRIT,pqname,vpname); 
	(void)odm_rm_obj(cmd_opt,wbuf); 
	(void)sprintf(wbuf,ODM_SYS_SCO_CRIT,pqname,vpname); 
	(void)odm_rm_obj(cmd_opt,wbuf); 
	(void)sprintf(wbuf,ODM_PFL_SCO_CRIT,pqname,vpname); 
	(void)odm_rm_obj(cmd_opt,wbuf); 
	(void)sprintf(wbuf,ODM_FIL_SCO_CRIT,pqname,vpname); 
	(void)odm_rm_obj(cmd_opt,wbuf); 

    /* clean up */
	(void)odm_close_class(cmd_hdr);
	(void)odm_close_class(cmd_opt);
	(void)odm_unlock(odmld);
	(void)odm_terminate();

	return;
}
