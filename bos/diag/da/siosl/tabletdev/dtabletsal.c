static char sccsid[] = "@(#)51  1.9  src/bos/diag/da/siosl/tabletdev/dtabletsal.c, databletsl, bos411, 9428A410j 4/1/94 17:01:14";
/*
 *   COMPONENT_NAME: DATABLETSL
 *
 *   FUNCTIONS: add_fru
 *		chk_return
 *		dsply_tst_hdr
 *		dsply_tst_lst
 *		dsply_tst_msg
 *		exit_da
 *		init_frub
 *		main
 *		setdamode
 *		tu_10
 *		tu_20
 *		tu_30
 *		tu_40
 *		tu_60
 *		tu_70
 *		tu_C0
 *		tu_test
 *		unconfigure_lpp_device
 *
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        <stdio.h>
#include        <fcntl.h>
#include        <nl_types.h>
#include        <limits.h>
#include        <locale.h>
#include        <sys/devinfo.h>
#include        <sys/ioctl.h>
#include        <sys/types.h>
#include        <sys/cfgodm.h>
#include	<sys/inputdd.h>
#include        "diag/diago.h"
#include        "diag/tm_input.h"
#include        "diag/tmdefs.h"
#include        "diag/da.h"
#include        "diag/diag_exit.h"
#include        "diag/atu.h"
#include        "diag/dcda_msg.h"
#include        "diag/da_cmn.h"
#include        "dtablet_msg.h"
#include        "dtablet.h"

extern	nl_catd diag_catopen(char *, int);
extern  int     slctn;
extern	int	tablet_type();
extern  int     diag_asl_init ();
extern  int     init_menu ();
extern  int     chk_asl_stat ();
extern  char	*diag_cat_gets();

int     d_state;
char    devtblt[32];
nl_catd catd = CATD_ERR;
struct  objlistinfo     c_info;
char    crit[100];

char    *msgstr;
char    option[256];
char    new_out[256], new_err[256];
char    *lpp_configure_method;
char    *lpp_unconfigure_method;
int     unconfigure_lpp = FALSE;

main ()  /* begin main */
{
	int	rc = 0; 
	struct	tabqueryid *tab_arg;
 
	int     setdamode ();
	void    exit_da ();
  	unsigned long    f_code;

	setlocale(LC_ALL,"");
	
	filedes = -1;
	fdes = -1;
	d_state = -1;
	init_dgodm ();
	DA_SETRC_STATUS (DA_STATUS_GOOD);
	DA_SETRC_USER (DA_USER_NOKEY);
	DA_SETRC_ERROR (DA_ERROR_NONE);
	DA_SETRC_TESTS (DA_TEST_NOTEST);
	DA_SETRC_MORE (DA_MORE_NOCONT);

	tab_arg = (struct tabqueryid *) malloc (1*sizeof(struct tabqueryid));
	msgstr = (char *) malloc (1200*sizeof(char));
        lpp_configure_method = (char *) malloc (256*sizeof(char));
        lpp_unconfigure_method = (char *) malloc (256*sizeof(char));

        if ((msgstr == (char *) NULL) || (tab_arg == NULL) ||
           (lpp_configure_method == (char *) NULL) ||
           (lpp_unconfigure_method == (char *) NULL))
        {
 		exit_da();
	}

	if ((getdainput (&tm_input)) == -1)
	{
	        DA_SETRC_TESTS (DA_TEST_FULL);
	        DA_SETRC_ERROR (DA_ERROR_OTHER);
	        DA_EXIT ();
	}

	da_mode = setdamode ();

	if (c_mode == CONSOLE)
	{
	        diag_asl_init ("NO_TYPE_AHEAD");
	        catd = diag_catopen(MF_DTABLET, 0);
	        init_menu (TU10);
	        asl_err = dsply_tst_hdr (menu_nmbr);
	        chk_asl_stat (asl_err);
	}
	
	d_state = get_device_status(tm_input.dname);
	if (d_state == AVAILABLE)
	{
	        strcpy (devtblt, "/dev/");
	        strcat (devtblt, tm_input.dname);
	        if ((filedes = open (devtblt, O_RDWR)) == -1)
	        {
	                DA_SETRC_ERROR (DA_ERROR_OPEN);
	                exit_da ();
	        }
		else
        	{
			/* query to see what type of input device */
                	ioctl(filedes, TABQUERYID, tab_arg);
                	switch(tab_arg->input_device)
                	{
                        	case TABSTYLUS:
                                	idev_id = STYLUS_INPUT_DEVICE;
                                	f_msg = TM_FM3;
                                	strcpy (inpt_dev,pen);
                                	break;
                        	case TABPUCK:
                                	idev_id = PUCK_INPUT_DEVICE;
                                	f_msg = TM_FM10;
                                	strcpy (inpt_dev,puck);
                                	break;
                	}  /* switch */
        	}
        	close(filedes);
		rc = unconfigure_lpp_device();
	}

	/* open the machine device driver */
	strcpy (devtblt, "/dev/bus0");
	if ((fdes = open (devtblt, O_RDWR)) == -1)
        {
                DA_SETRC_ERROR (DA_ERROR_OPEN);
                exit_da ();
        }
	
	if (d_mode == PD || d_mode == REPAIR)
	{
	        switch (da_mode)
	        {
	        case NO_MENU_TEST_MODE:
	                tu_10 (TU10);
	                chk_asl_stat (asl_err);
	                tu_20 (TU20);
	                chk_asl_stat (asl_err);
	                tu_30 (TU30);
	                break;
	        case ALL_TESTS_MODE:
	        case LOOP_MODE_TESTS:
	                tu_10 (TU10);
			if (l_mode == INLM)
			{
				sleep (3);
	                	chk_asl_stat (diag_asl_read
				    ( ASL_DIAG_LIST_CANCEL_EXIT_SC,0,0));
			}
	                tu_20 (TU20);
			if (l_mode == INLM)
			{
				sleep (3);
	                	chk_asl_stat (diag_asl_read
				    ( ASL_DIAG_LIST_CANCEL_EXIT_SC,0,0));
			}
	                tu_30 (TU30);
	                if (l_mode == NOTLM || l_mode == ENTERLM)
	                {
	                        tu_40 (TU40);
	                        tu_60 (TU60);
	                        tu_70 (TU70);
	                        tu_C0 (TUC0);   
	                }  /* endif */
	                break;
	        default:
	                break;
	        }  /* end switch (da_mode) */
	chk_asl_stat (asl_err);
	}  /* endif */
	DA_SETRC_TESTS (DA_TEST_FULL);
	exit_da ();
}  /* main end */

/*
 * NAME: setdamode
 *
 * FUNCTION:  Returns the execution environment to the DA to determine the
 *      execution mode using data defined in structure tm_input and header
 *      file dasync.h.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  damode
 */

int     setdamode ()  /* begin setdamode */
{
	int   damode = INVALID_TM_INPUT;
	uchar   bufstr[32];

	l_mode = tm_input.loopmode;    /* NOTLM, ENTERLM, INLM, EXITLM      */
	c_mode = tm_input.console;     /* CONSOLE, NO_CONSOLE               */
	a_mode = tm_input.advanced;    /* ADVANCED, NOT_ADVANCED            */
	s_mode = tm_input.system;      /* SYSTEM, NOT_SYSTEM                */
	e_mode = tm_input.exenv;       /* IPL, STD, MNT, CONC               */
	d_mode = tm_input.dmode;       /* ELA, PD, REPAIR, MS1, MS2,        */
	                               /* FREELANCE                         */
	if (s_mode == NOT_SYSTEM && c_mode == CONSOLE)
	{
	        if (l_mode == NOTLM)
	                damode = ALL_TESTS_MODE;
	        if (l_mode != NOTLM && e_mode != CONC)
	                damode = LOOP_MODE_TESTS;
	}

	if ((s_mode == NOT_SYSTEM && c_mode == NO_CONSOLE) ||
	    (s_mode == SYSTEM && c_mode == NO_CONSOLE) ||
	    (s_mode == SYSTEM && c_mode == CONSOLE))
	{
	        damode = NO_MENU_TEST_MODE;
	}

	sprintf (bufstr, "name = '%s'", tm_input.dname);
	cudv = get_CuDv_list(CuDv_CLASS, bufstr, &dinfo ,1 ,2);
	if (cudv == (struct CuDv *) -1)
	{
	        DA_SETRC_ERROR (DA_ERROR_OTHER);
	        exit_da;
	}
	menu_nmbr = cudv->PdDvLn->led << 12;
	return (damode);
}  /* setdamode end */

void    init_frub (tu_num)
int     tu_num;
{
	frub[0].sn = cudv->PdDvLn->led;
	switch (tu_num)
	{
	case 0x10:
	case 0x20:
	case 0x30:
	case 0x40:
	case 0x60:
	case 0x70:
	        tu_err += 0x0100;
	        break;
	case 0xC0:
	        tu_err += 0x0200;
	        break;
	}
	frub[0].rcode = tu_err;
	frub[0].rmsg = 0;
	frub[0].frus[0].conf = 0;
	frub[0].frus[1].conf = 0;
	frub[0].frus[0].fname[0] = '\0';
	frub[0].frus[1].fname[0] = '\0';
	frub[0].frus[0].fmsg = 0;
	frub[0].frus[1].fmsg = 0;
	frub[0].frus[0].fru_flag = 0;
	frub[0].frus[1].fru_flag = 0;
	frub[0].frus[0].fru_exempt = 0;
	frub[0].frus[1].fru_exempt = 0;
}

int     tu_10 (tu_num)
int     tu_num;
{
	int     tu_test ();
	void    init_frub ();
	void    add_fru ();
	void    exit_da ();

	if (l_mode != EXITLM)
	{
	        tu_err = tu_test (tu_num);
	        if (tu_err != 0)
	        {
	                switch (tu_err)
	                {
	                case 0x10:
	                case 0xFD:
	                case 0xFE:
	                case 0xFF:
	                        DA_SETRC_TESTS (DA_TEST_FULL);
	                        DA_SETRC_ERROR (DA_ERROR_OTHER);
	                        exit_da ();
	                        break;
	                case 0x11:
	                        init_frub (tu_num);
	                        frub[0].rmsg = TM_TU10;
	                        frub[0].frus[0].conf = conf10;
				frub[0].frus[1].conf = conf11;
				frub[0].frus[0].fru_flag = DA_NAME;
	                        frub[0].frus[1].fru_flag = PARENT_NAME;
	                        frub[0].frus[0].fru_exempt = EXEMPT;
				frub[0].frus[1].fru_exempt = NONEXEMPT;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                case 0x12:
	                case 0x13:
	                case 0x14:
	                case 0x15:
	                        init_frub (tu_num);
	                        frub[0].rmsg = TM_TU10;
	                        frub[0].frus[0].conf = conf2;
	                        frub[0].frus[1].conf = conf3;
	                        frub[0].frus[0].fru_flag = DA_NAME;
	                        frub[0].frus[1].fru_flag = PARENT_NAME;
	                        frub[0].frus[0].fru_exempt = EXEMPT;
	                        frub[0].frus[1].fru_exempt = NONEXEMPT;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                case 0x16:
	                        init_frub (tu_num);
	                        if (idev_id == PUCK_INPUT_DEVICE)
	                                frub[0].rcode += 3;
	                        frub[0].rmsg = TM_TU10;
	                        frub[0].frus[0].conf = conf1;
	                        strcpy (frub[0].frus[0].fname, inpt_dev);
	                        frub[0].frus[0].fmsg = f_msg;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                }  /* end switch (tu_err) */
	        }  /* endif */
	}  /* endif */
}  /* end tu_10 */

int     tu_20 (tu_num)
int     tu_num;
{
	int     tu_test ();
	void    init_frub ();
	void    add_fru ();
	void    exit_da ();

	if (l_mode != EXITLM)
	{
	        tu_err = tu_test (tu_num);
	        if (tu_err != 0)
	        {
	                switch (tu_err)
	                {
	                case 0x20:
	                case 0xFD:
	                case 0xFE:
	                case 0xFF:
	                        DA_SETRC_TESTS (DA_TEST_FULL);
	                        DA_SETRC_ERROR (DA_ERROR_OTHER);
	                        exit_da ();
	                        break;
	                case 0x21:
	                        init_frub (tu_num);
	                        frub[0].rmsg = TM_TU20;
	                        frub[0].frus[0].conf = conf1;
	                        frub[0].frus[0].fru_flag = PARENT_NAME;
	                        frub[0].frus[0].fru_exempt = NONEXEMPT;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                case 0x22:
	                case 0x23:
	                case 0x24:
	                        init_frub (tu_num);
	                        frub[0].rmsg = TM_TU20;
	                        frub[0].frus[0].conf = conf2;
	                        frub[0].frus[1].conf = conf3;
	                        frub[0].frus[0].fru_flag = DA_NAME;
	                        frub[0].frus[1].fru_flag = PARENT_NAME;
	                        frub[0].frus[0].fru_exempt = EXEMPT;
	                        frub[0].frus[1].fru_exempt = NONEXEMPT;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                }  /* end switch (tu_err) */
	        }  /* endif */
	}  /* endif */
}  /* end tu_20 */

int     tu_30 (tu_num)
int     tu_num;
{
	int     tu_test ();
	void    init_frub ();
	void    add_fru ();
	void    exit_da ();

	if (l_mode != EXITLM)
	{
	        tu_err = tu_test (tu_num);
	        if (tu_err == 0)
		{
			if ((l_mode == NOTLM || l_mode == ENTERLM)
				&& c_mode == CONSOLE && s_mode == NOT_SYSTEM)
				tu_err = tablet_type();	
		}
	        if (tu_err != 0)
	        {
	                switch (tu_err)
	                {
	                case 0x30:
	                case 0xFD:
	                case 0xFE:
	                case 0xFF:
	                        DA_SETRC_TESTS (DA_TEST_FULL);
	                        DA_SETRC_ERROR (DA_ERROR_OTHER);
	                        exit_da ();
	                        break;
	                case 0x31:
	                        init_frub (tu_num);
	                        frub[0].rmsg = TM_TU30;
	                        frub[0].frus[0].conf = conf6;
	                        frub[0].frus[1].conf = conf7;
	                        frub[0].frus[0].fru_flag = PARENT_NAME;
	                        frub[0].frus[1].fru_flag = DA_NAME;
	                        frub[0].frus[0].fru_exempt = NONEXEMPT;
	                        frub[0].frus[1].fru_exempt = EXEMPT;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                case 0x32:
	                case 0x33:
	                case 0x34:
	                case 0x35:
	                        init_frub (tu_num);
	                        frub[0].rmsg = TM_TU30;
	                        frub[0].frus[0].conf = conf1;
	                        frub[0].frus[0].fru_flag = DA_NAME;
	                        frub[0].frus[0].fru_exempt = EXEMPT;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                case 0x36:
	                        DA_SETRC_TESTS (DA_TEST_FULL);
	                        exit_da ();
	                        break;
	                }  /* end switch (tu_err) */
	        }  /* endif */
	}  /* endif */
}  /* end tu_30 */

int     tu_40 (tu_num)
int     tu_num;
{
	int     tu_test ();
	void    init_frub ();
	void    add_fru ();
	void    exit_da ();

	if (l_mode != EXITLM)
	{
	        tu_err = tu_test (tu_num);
	        if (tu_err != 0)
	        {
	                switch (tu_err)
	                {
	                case 0x40:
	                case 0xFD:
	                case 0xFE:
	                case 0xFF:
	                        DA_SETRC_TESTS (DA_TEST_FULL);
	                        DA_SETRC_ERROR (DA_ERROR_OTHER);
	                        exit_da ();
	                        break;
	                case 0x41:
	                        init_frub (tu_num);
	                        frub[0].rmsg = TM_TU40;
	                        frub[0].frus[0].conf = conf2;
	                        frub[0].frus[1].conf = conf3;
	                        frub[0].frus[0].fru_flag = PARENT_NAME;
	                        frub[0].frus[1].fru_flag = DA_NAME;
	                        frub[0].frus[0].fru_exempt = NONEXEMPT;
	                        frub[0].frus[1].fru_exempt = EXEMPT;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                case 0x42:
	                case 0x43:
	                        init_frub (tu_num);
	                        frub[0].rmsg = TM_TU40;
	                        frub[0].frus[0].conf = conf1;
	                        frub[0].frus[0].fru_flag = DA_NAME;
	                        frub[0].frus[0].fru_exempt = EXEMPT;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                }  /* end switch (tu_err) */
	        }  /* endif */
	}  /* endif */
}  /* end tu_40 */


int     tu_60 (tu_num)
int     tu_num;
{
	int     tu_test ();
	void    init_frub ();
	void    add_fru ();
	void    exit_da ();

	if (l_mode != EXITLM)
	{
	        tu_err = tu_test (tu_num);
	        if (tu_err != 0)
	        {
	                switch (tu_err)
	                {
	                case 0x60:
	                case 0xFD:
	                case 0xFE:
	                case 0xFF:
	                        DA_SETRC_TESTS (DA_TEST_FULL);
	                        DA_SETRC_ERROR (DA_ERROR_OTHER);
	                        exit_da ();
	                        break;
	                case 0x61:
	                        init_frub (tu_num);
	                        frub[0].rmsg = TM_TU60;
	                        frub[0].frus[0].conf = conf1;
	                        frub[0].frus[0].fru_flag = DA_NAME;
	                        frub[0].frus[0].fru_exempt = EXEMPT;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                case 0x62:
	                case 0x63:
	                case 0x64:
	                        init_frub (tu_num);
	                        if (idev_id == PUCK_INPUT_DEVICE)
	                                frub[0].rcode += 4;
	                        frub[0].rmsg = TM_TU60;
	                        frub[0].frus[0].conf = conf6;
	                        frub[0].frus[1].conf = conf7;
	                        strcpy (frub[0].frus[0].fname, inpt_dev);
	                        frub[0].frus[0].fmsg = f_msg;
	                        frub[0].frus[1].fru_flag = DA_NAME;
	                        frub[0].frus[1].fru_exempt = EXEMPT;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                case 0x65:
	                        init_frub (tu_num);
	                        if (idev_id == PUCK_INPUT_DEVICE)
	                                frub[0].rcode += 4;
	                        frub[0].rmsg = TM_TU60;
	                        frub[0].frus[0].conf = conf1;
	                        strcpy (frub[0].frus[0].fname, inpt_dev);
	                        frub[0].frus[0].fmsg = f_msg;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                }  /* end switch (tu_err) */
	        }  /* endif */
	}  /* endif */
}  /* end tu_60 */

int     tu_70 (tu_num)
int     tu_num;
{
	int     tu_test ();
	void    init_frub ();
	void    add_fru ();
	void    exit_da ();

	if (l_mode != EXITLM)
	{
	        tu_err = tu_test (tu_num);
	        if (tu_err != 0)
	        {
	                switch (tu_err)
	                {
	                case 0x70:
	                case 0xFD:
	                case 0xFE:
	                case 0xFF:
	                        DA_SETRC_TESTS (DA_TEST_FULL);
	                        DA_SETRC_ERROR (DA_ERROR_OTHER);
	                        exit_da ();
	                        break;
	                case 0x71:
	                        init_frub (tu_num);
	                        frub[0].rmsg = TM_TU70;
	                        frub[0].frus[0].conf = conf1;
	                        frub[0].frus[0].fru_flag = DA_NAME;
	                        frub[0].frus[0].fru_exempt = EXEMPT;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                case 0x72:
	                case 0x73:
	                case 0x74:
	                        init_frub (tu_num);
	                        if (idev_id == PUCK_INPUT_DEVICE)
	                                frub[0].rcode += 4;
	                        frub[0].rmsg = TM_TU70;
	                        frub[0].frus[0].conf = conf2;
	                        frub[0].frus[1].conf = conf3;
	                        strcpy (frub[0].frus[0].fname, inpt_dev);
	                        frub[0].frus[0].fmsg = f_msg;
	                        frub[0].frus[1].fru_flag = DA_NAME;
	                        frub[0].frus[1].fru_exempt = EXEMPT;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                case 0x75:
	                        init_frub (tu_num);
	                        if (idev_id == PUCK_INPUT_DEVICE)
	                                frub[0].rcode += 4;
	                        frub[0].rmsg = TM_TU70;
	                        frub[0].frus[0].conf = conf1;
	                        strcpy (frub[0].frus[0].fname, inpt_dev);
	                        frub[0].frus[0].fmsg = f_msg;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                }  /* end switch (tu_err) */
	        }  /* endif */
	}  /* endif */
}  /* end tu_70 */


int     tu_C0 (tu_num)
int     tu_num;
{
	int     tu_test ();
	void    init_frub ();
	void    add_fru ();
	void    exit_da ();

	if (l_mode != EXITLM)
	{
	        tu_err = tu_test (tu_num);
	        if (tu_err != 0)
	        {
	                switch (tu_err)
	                {
	                case 0xc0:
	                case 0xFD:
	                case 0xFE:
	                case 0xFF:
	                        DA_SETRC_TESTS (DA_TEST_FULL);
	                        DA_SETRC_ERROR (DA_ERROR_OTHER);
	                        exit_da ();
	                        break;
	                case 0xc1:
	                        init_frub (tu_num);
	                        frub[0].rcode &= 0x0F2F;
	                        frub[0].rmsg = TM_TUC0;
	                        frub[0].frus[0].conf = conf1;
	                        frub[0].frus[0].fru_flag = PARENT_NAME;
	                        frub[0].frus[0].fru_exempt = NONEXEMPT;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                case 0xc2:
	                case 0xc3:
	                case 0xc4:
	                        init_frub (tu_num);
	                        frub[0].rcode &= 0x0F2F;
	                        if (idev_id == PUCK_INPUT_DEVICE)
	                                frub[0].rcode += 4;
	                        frub[0].rmsg = TM_TUC0;
	                        frub[0].frus[0].conf = conf6;
	                        frub[0].frus[1].conf = conf7;
	                        frub[0].frus[0].fru_flag = DA_NAME;
	                        frub[0].frus[1].fru_flag = PARENT_NAME;
	                        frub[0].frus[0].fru_exempt = EXEMPT;
	                        frub[0].frus[1].fru_exempt = NONEXEMPT;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                case 0xc5:
	                        init_frub (tu_num);
	                        frub[0].rcode &= 0x0F2F;
	                        if (idev_id == PUCK_INPUT_DEVICE)
	                                frub[0].rcode += 4;
	                        frub[0].rmsg = TM_TUC0;
	                        frub[0].frus[0].conf = conf1;
	                        strcpy (frub[0].frus[0].fname, inpt_dev);
	                        frub[0].frus[0].fmsg = f_msg;
	                        add_fru ();
	                        exit_da ();
	                        break;
	                }  /* end switch (tu_err) */
	        }  /* endif */
	}  /* endif */
}  /* end tu_C0 */

/*
 * NAME: dsply_tst_hdr
 *
 * FUNCTION:  Builds a title line that describes the adapter/device under test
 *      and displays test status messages.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

int     dsply_tst_hdr (menu_num)        /* begin dsply_tst_hdr */
long    menu_num;
{
	char    msgstr[512];
	long    asl_rtn = DIAG_ASL_OK;
	struct  msglist da_menu[] =
	{
	        { TABLET_DIAG, TM_1A, },
	        { (int)NULL,(int)NULL,}, 
	};
	ASL_SCR_TYPE    menutype =
	    {
	        ASL_DIAG_OUTPUT_LEAVE_SC,0,1
	};
	ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

	memset (menu_da,0, sizeof (menu_da));
	da_menu[0].setid = TABLET_DIAG;
	if (a_mode == ADVANCED)
	        if (l_mode == NOTLM || l_mode == ENTERLM)
	                da_menu[0].msgid = TM_2B;
	        else
	                da_menu[0].msgid = TM_2C;
	else
	        da_menu[0].msgid = TM_1B;
	if (c_mode == CONSOLE)
		diag_display(menu_num, catd, da_menu, DIAG_MSGONLY,
	    		ASL_DIAG_OUTPUT_LEAVE_SC, &menutype, menu_da);
	if (l_mode != NOTLM && l_mode != ENTERLM)
	{
	        sprintf(msgstr, menu_da[0].text, tm_input.lcount,
	            tm_input.lerrors);
	        free (menu_da[0].text);
	        menu_da[0].text = (char *) malloc (strlen(msgstr)+1);
	        strcpy (menu_da[0].text, msgstr);
	}  /* endif */
	if (c_mode == CONSOLE)
		asl_rtn = diag_display (menu_num, catd, NULL, DIAG_IO,
	    		ASL_DIAG_OUTPUT_LEAVE_SC, &menutype, menu_da);
	return (asl_rtn);
}  /* dsply_tst_hdr end */

/*
 * NAME: dsply_tst_msg
 *
 * FUNCTION: Builds a title line that describes the adapter/device under test,
 *           and explains the test setup requirements.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

int     dsply_tst_msg (menu_num, tm_1)          
long    menu_num;
unsigned short     tm_1;
{
	char    msgstr[1024];
	long    asl_rtn = DIAG_ASL_OK;
	struct  msglist da_menu[] =
	{
	        { TABLET_DIAG, TM_2A, },
	        { TABLET_DIAG, 0,     },
	        { TABLET_DIAG, TM_9,  },
	        {(unsigned short)NULL, (unsigned short)NULL,},
	};
	ASL_SCR_TYPE    menutype =
	    {
	        ASL_DIAG_ENTER_SC,2,1
	};
	ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

	da_menu[1].msgid = tm_1;
	memset (menu_da,0, sizeof (menu_da));
	if (a_mode == ADVANCED)
	        da_menu[0].msgid = TM_2A;
	else
	        da_menu[0].msgid = TM_1A;
	if (c_mode == CONSOLE) {
		diag_display (menu_num, catd, da_menu, DIAG_MSGONLY,
	    ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da);
		asl_rtn = diag_display (menu_num, catd, NULL, DIAG_IO,
	    ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da);
	}
	return (asl_rtn);
} 

/*
 * NAME: dsply_tst_lst
 *
 * FUNCTION:  Builds a title line that describes the adapter/device under test
 *            and asks the user to reply to a question.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

int     dsply_tst_lst (menu_num, tm_3)        
long    menu_num;
unsigned short    tm_3;
{
	char    msgstr[1024];
	long    asl_rtn;
	struct  msglist da_menu[] =
	{
	        { TABLET_DIAG, TM_2A, },
	        { TABLET_DIAG, TM_5,  },
	        { TABLET_DIAG, TM_6,  },
	        { TABLET_DIAG, 0,     },
	        { (unsigned short)NULL,(unsigned short)NULL,},  
	};
	ASL_SCR_TYPE    menutype =
	    {
	        ASL_DIAG_ENTER_SC,3,1
	};
	ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

	da_menu[3].msgid = tm_3;
	memset (menu_da,0, sizeof (menu_da));
	if (a_mode == ADVANCED)
	        da_menu[0].msgid = TM_2A;
	else
	        da_menu[0].msgid = TM_1A;
	diag_display (menu_num, catd, da_menu, DIAG_MSGONLY,
	    ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, menu_da);
	sprintf (msgstr, menu_da[3].text,
	    diag_cat_gets (catd, TABLET_DIAG, TM_7));
	free (menu_da[3].text);
	menu_da[3].text = (char *) malloc (strlen(msgstr)+1);
	strcpy (menu_da[3].text, msgstr);
	asl_rtn = diag_display (menu_num, catd, NULL, DIAG_IO,
	    ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, menu_da);
	slctn = DIAG_ITEM_SELECTED (menutype);
	if (asl_rtn == ASL_COMMIT)
	        asl_rtn = ASL_ENTER;
	return (asl_rtn);
}  /* dsply_tst_lst end */

/*
 * NAME: tu_test
 *
 * FUNCTION: Executes the test unit and receives a return code.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */
int     tu_test (tstptr)  /* begin tu_test */
int     tstptr;
{
	int     tu_rc = 0;
	uchar   turc[4];
	int     exectu ();

	tucb_ptr.tu = tstptr;
	tucb_ptr.mfg = 0;
	tucb_ptr.loop = 1;
	tu_rc = exectu (fdes, &tucb_ptr);
	return (tu_rc);
}  /* tu_test end */

/*
 * NAME: add_fru
 *
 * FUNCTION: Interprets the return code then calls
 *      the addfrub function to report the FRU number, name and confidence
 *      level when an error is detected.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

void    add_fru ()
{
	strncpy(frub[0].dname, tm_input.dname, NAMESIZE);
	if (insert_frub (&tm_input, &frub[0]) != 0)
	{
	        DA_SETRC_TESTS (DA_TEST_FULL);
	        DA_SETRC_ERROR (DA_ERROR_OTHER);
	        exit_da ();
	}  /* endif */
	if (addfrub (&frub[0]) != 0)
	{
	        DA_SETRC_TESTS (DA_TEST_FULL);
	        DA_SETRC_ERROR (DA_ERROR_OTHER);
	        exit_da();
	}  /* endif */
	DA_SETRC_STATUS (DA_STATUS_BAD);
	DA_SETRC_USER (DA_USER_NOKEY);
	DA_SETRC_ERROR (DA_ERROR_NONE);
	DA_SETRC_TESTS (DA_TEST_FULL);
	DA_SETRC_MORE (DA_MORE_CONT);
}  /* add_fru end */

/*
 * NAME: exit_da
 *
 * FUNCTION: Executes the test unit, interprets the return code then calls
 *      the addfrub function to report the FRU number, name and confidence
 *      level when an error is detected.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

void    exit_da ()
{
	int	rc;
	
	if (d_state == AVAILABLE) 
	{
		if (unconfigure_lpp == TRUE)
		{

			sprintf(option," -l %s", tm_input.dname);
			rc = odm_run_method(lpp_configure_method, option,
					&new_out, &new_err);
			if (rc == -1)
			{
        			frub[0].sn = cudv->PdDvLn->led;
        			frub[0].rcode = 0x0278;
                                frub[0].rmsg = TM_CNFG_ERR;
                                frub[0].frus[0].conf = conf2;
                                frub[0].frus[1].conf = conf3;
                                frub[0].frus[0].fru_flag = DA_NAME;
                                frub[0].frus[1].fru_flag = PARENT_NAME;
                                frub[0].frus[0].fru_exempt = EXEMPT;
                                frub[0].frus[1].fru_exempt = NONEXEMPT;
                                add_fru ();
			}
		}	
	}
	term_dgodm ();
	if (fdes != -1)
	        close (fdes);
	if (catd != CATD_ERR)
	        catclose (catd);
	if (c_mode == CONSOLE)
		diag_asl_quit (NULL);
	DA_EXIT ();

}  /* end exit_da () */

int	chk_return(ret_code)
int	ret_code;
{
	if ( ret_code == -1 )
	{
		DA_SETRC_ERROR ( DA_ERROR_OTHER );
                exit_da ();
	}
}


/*
 * NAME: unconfigure_lpp_device
 *
 * FUNCTION:This function is called by main() only
 *      when the device is in the available state.
 *      Here,the device is unconfigured and its state is
 *      changed to define.
 *
 * RETURNS: 0 Good
 *          -1 Bad
 *
 */

int     unconfigure_lpp_device()
{
        struct PdDv     *pddv_p;
        struct CuDv	*cudv_p;
	char    criteria[128];
        int     result;

        sprintf (criteria, "name=%s AND chgstatus != 3",tm_input.dname);
	cudv_p = get_CuDv_list(CuDv_CLASS, criteria, &c_info, 1, 1);
	if (cudv_p == (struct CuDv *) NULL)
	{
		DA_SETRC_ERROR(DA_ERROR_OTHER);
		exit_da();
	}
	
	sprintf (criteria, "uniquetype=%s",cudv_p->PdDvLn_Lvalue);
        pddv_p = get_PdDv_list (PdDv_CLASS, criteria, &c_info, 1, 1);
        strcpy (lpp_unconfigure_method, pddv_p->Unconfigure);
	strcpy (lpp_configure_method, pddv_p->Configure);

        sprintf (option," -l %s", cudv_p->name);
        result = odm_run_method(lpp_unconfigure_method,option,
                                &new_out,&new_err);
        if (result != -1)
                unconfigure_lpp = TRUE;
        else
        {
        	frub[0].sn = cudv->PdDvLn->led;
        	frub[0].rcode = 0x0279;
                frub[0].rmsg = TM_UNCNFG_ERR;
                frub[0].frus[0].conf = conf2;
                frub[0].frus[1].conf = conf3;
                frub[0].frus[0].fru_flag = DA_NAME;
                frub[0].frus[1].fru_flag = PARENT_NAME;
                frub[0].frus[0].fru_exempt = EXEMPT;
                frub[0].frus[1].fru_exempt = NONEXEMPT;
                add_fru ();
		exit_da();
	}
	return(0);
}


