static char sccsid[] = "@(#)15  1.23  src/bos/diag/da/tablet/dtablet.c, datablet, bos411, 9428A410j 5/20/94 16:02:47";
/*
 *   COMPONENT_NAME: DATABLET
 *
 *   FUNCTIONS: add_fru
 *		dsply_tst_hdr
 *		dsply_tst_lst
 *		dsply_tst_msg
 *		chk_asl_stat
 *		tablet_type
 *		init_menu
 *		exit_da
 *		init_frub
 *		main
 *		setdamode
 *		fix_common_err
 *		tu_10
 *		tu_20
 *		tu_30
 *		tu_40
 *		tu_50
 *		tu_60
 *		tu_70
 *		tu_test
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1994
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
#include        <sys/errno.h>
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
#include	"tu_type.h"

extern	nl_catd diag_catopen(char *, int);
extern  int     diag_asl_init ();
extern  char	*diag_cat_gets();

void	init_menu();
void	add_fru();
void	exit_da();
int     d_state;
int     slctn;
int	config_tminput = 0;
char    devname[32];
char	kbddevname[20];
nl_catd catd = CATD_ERR;
struct  objlistinfo     c_info;
char    crit[100];
uint	arg;				/* argument for the ioctl call */
int	tabtype = 0;			/* default device type */
int	tabid = 0x926;			/* default device id */
char    option[256];
char    new_out[256], new_err[256];
char    *lpp_configure_method;
char    *lpp_unconfigure_method;
int     unconfigure_lpp = FALSE;


main ()  /* begin main */
{

  	char	*msgstr;
	int	err_code = 0;
	int	rc = -1;
        long	menu_number = 0;
	long	asl_err = 0;
	struct  CuDv    *cudv;
	struct tabqueryid *tab_arg;

	setlocale(LC_ALL,"");

	fdes = -1;
	filedes = -1;
	filedes_k = -1;
	d_state = -1;
	init_dgodm ();
	DA_SETRC_STATUS (DA_STATUS_GOOD);
	DA_SETRC_USER (DA_USER_NOKEY);
	DA_SETRC_ERROR (DA_ERROR_NONE);
	DA_SETRC_TESTS (DA_TEST_NOTEST);
	DA_SETRC_MORE (DA_MORE_NOCONT);

	if ((getdainput (&tm_input)) == -1)
	{
	        DA_SETRC_TESTS (DA_TEST_FULL);
	        DA_SETRC_ERROR (DA_ERROR_OTHER);
	        DA_EXIT ();
	}

	tab_arg = (struct tabqueryid *) malloc (1*sizeof(struct tabqueryid));
	if (tab_arg == (struct tabqueryid *) NULL)
		exit_da();	
	msgstr = (char *) malloc(1200 * sizeof(char));
        lpp_configure_method = (char *) malloc (256*sizeof(char));
        lpp_unconfigure_method = (char *) malloc (256*sizeof(char));

        if ((msgstr == (char *) NULL) || (tab_arg == NULL) ||
           (lpp_configure_method == (char *) NULL) ||
           (lpp_unconfigure_method == (char *) NULL))
        {
	        DA_SETRC_TESTS (DA_TEST_FULL);
	        DA_SETRC_ERROR (DA_ERROR_OTHER);
                exit_da();
        }

	
	da_mode = setdamode ();
	menu_number = menu_nmbr;
 /*
  *	Determine which type of tablet we are testing.
  *	Assign the correct service number to the fru bucket.
  */	
	sprintf( crit, "name = %s", tm_input.dname );
	cudv = get_CuDv_list( CuDv_CLASS, crit, &c_info, 1, 2 );
	if (cudv->PdDvLn->led == M11)
	{
		tabid = M11;
		tabtype = 0;
	}
	else	if (cudv->PdDvLn->led == M12)
		{
			tabid = M12;
			tabtype = 1;
		}
	
	if (c_mode == CONSOLE)
	{
	        diag_asl_init ("NO_TYPE_AHEAD");
	        catd = diag_catopen(MF_DTABLET, 0);
	        init_menu (TU10);
	        asl_err = dsply_tst_hdr (menu_nmbr);
	        chk_asl_stat (asl_err);
	}

	/* open the machine device driver */
	if ((fdes = open ("/dev/bus0", O_RDWR)) == -1)
        {
                DA_SETRC_ERROR (DA_ERROR_OTHER);
                exit_da ();
        }
	
	d_state = configure_device (tm_input.dname);
	config_tminput = 1;
        if (d_state != -1)
        {
		strcpy (devname, "/dev/");
        	strcat (devname, tm_input.dname);
		if ((filedes = open (devname, O_RDWR)) == -1)
		{
                	err_code = errno;
                	if ((err_code == EBUSY) || (err_code == EIO) ||
                  	(err_code == ENXIO)  || (err_code == ENODEV))
				DA_SETRC_ERROR (DA_ERROR_OPEN);
	        	exit_da ();
		}
		else
		{
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
		if ((rc = close(filedes)) == -1)
		{
			DA_SETRC_TESTS (DA_TEST_FULL);
	        	DA_SETRC_ERROR (DA_ERROR_OTHER);
	        	exit_da ();
		}
	/*	else
	                rc = unconfigure_lpp_device(); */
	}
	else
	{
		DA_SETRC_TESTS (DA_TEST_FULL);
	        DA_SETRC_ERROR (DA_ERROR_OTHER);
	        exit_da ();
	}
		
	/* see if keyboard attached and not missing */

	sprintf( crit,
	"parent = %s AND name != %s AND chgstatus != 3 AND status = 1",
                        tm_input.parent, tm_input.dname );
        cudv_k = get_CuDv_list( CuDv_CLASS, crit, &c_info, 1, 2 );
        
	if (cudv_k != (struct CuDv *) -1)
        {
	  if (c_info.num != 0)
	  {		
                strcpy(kbddevname,"/dev/");
                strcat(kbddevname, cudv_k->name);
		if ((filedes_k = open(kbddevname, O_RDWR)) != -1)
		{
			arg = KSDENABLE;
			if (ioctl(filedes_k,KSDIAGMODE, &arg) == 0)
                        {
                                sleep(2);
                                arg = KSDDISABLE;
                                ioctl(filedes_k, KSDIAGMODE, &arg);
                        }
			if ((rc = close(filedes_k)) == -1)
			{
				DA_SETRC_TESTS (DA_TEST_FULL);
	        		DA_SETRC_ERROR (DA_ERROR_OTHER);
	        		exit_da ();
			}
		}
		else
		{
			menu_number += 0x901;
			sprintf(msgstr, (char *)diag_cat_gets (catd, 1,
                                 FREE_DEVICE1), menu_number, cudv_k->name,
					cudv_k->location);
                        menugoal(msgstr);
			exit_da();
		}
		sleep(2);
	  } 
	}
	else
	{
		DA_SETRC_ERROR(DA_ERROR_OTHER);
                exit_da();
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
				tu_50 (TU50);
	                        tu_60 (TU60);
	                        tu_70 (TU70);
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
		if (tu_err == DEV_RESET_ERR)
			tu_err = 0x0113;
		else if ((tu_err >= DEV_RD_CFG_ERR) && 
				(tu_err <= ADP_PA_CFG_ERR))
			tu_err = 0x0114;
		else
			tu_err = 0x0112; 
	        break;
	case 0x20:
		if ((tu_err >= DEV_SET_WRAP_MDE_ERR) && 
				(tu_err <= DEV_WRP_MDE_ERR))
			tu_err = 0x0123;
		else if ((tu_err >= DEV_RST_WRP_MDE_ERR) && 
				(tu_err <= ADP_PC_RSET_WRAP_ERR))
			tu_err = 0x0124;
		break;
	case 0x30:
		if ((tu_err >= DEV_SET_CONVER_ERR) && 
				(tu_err <= DEV_SET_METRIC_ERR))
			tu_err = 0x0133;
		else if ((tu_err >= DEV_RESLN_BYTE1_ERR) && 
				(tu_err <= DEV_RESLN_LSB_ERR))
			tu_err = 0x0134;
		break;
	case 0x40:
		if (tu_err == DEV_FAIL_LED_TEST)
			tu_err = 0x0143;
		break;
	case 0x50:
		if (tu_err == DEV_FAIL_ACT)
			tu_err = 0x0164;
		else
			tu_err = 0x0163;
		break;
	case 0x60:
		if (tu_err == DEV_FL_PRES_SWTCH)
			tu_err = 0x0174;
		else
			tu_err = 0x0173;
		break;
	case 0x70:
	        if (tu_err == DEV_INCR_MDE_ERR)
			tu_err = 0x0283;
		else if (tu_err == DEV_FL_ENBL_TEST)
			tu_err = 0x0204;
		else
			tu_err = 0x0203;
	        break;
	}
	if (tu_num == 0x99)
		frub[0].rcode = 0;
	else
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

void	fix_common_err()
{
	int	num = 0x99;		/* arbitrary value for the tu number */
	switch(tu_err)
	{
	case ADP_DSBL_ERR:
	case DEV_DSBL_ERR:
		init_frub(num);
		frub[0].rcode = 0x0203;
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
	case DEV_FAIL_STATUS:
	case DEV_FL_STAT_BCOUNT:
	case ADP_EZ_BLK_ERR:
	case ADP_BLK_ACT_ERR:
	case DEV_RD_ST_ERR:
		init_frub (num);
                frub[0].rcode = 0x0115;
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
	case ADP_HDW_RSET_ERR:
	case ADP_HDW_REL_ERR:
	case ADP_BLK_INACT_ERR:
	case ADP_ENBL_IF_ERR:
	case ADP_FRM_O_ERR:
		init_frub (num);
		frub[0].rcode = 0x0113;
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
	case DEV_CONV_ERR:
	case DEV_CONV_YY_ERR:
		init_frub (num);
		frub[0].rcode = 0x0133;
		frub[0].rmsg = TM_TU30;
                frub[0].frus[0].conf = conf1;
                frub[0].frus[0].fru_flag = DA_NAME;
                frub[0].frus[0].fru_exempt = EXEMPT;
                add_fru ();
                exit_da ();
                break;
	case DEV_RESL_CMD84_ERR:
	case DEV_RESL_MSB_ERR:
	case DEV_RESL_CMD85_ERR:
	case DEV_RESL_LSB_ERR:
		init_frub (num);
		frub[0].rcode = 0x0134;
                frub[0].rmsg = TM_TU30;
                frub[0].frus[0].conf = conf1;
                frub[0].frus[0].fru_flag = DA_NAME;
                frub[0].frus[0].fru_exempt = EXEMPT;
                add_fru ();
                exit_da ();
                break;
	case DEV_SAMP_SPD_ERR:
	case DEV_SMP_SPD_YY_ERR:
	case DEV_ENBL_ERR:
		init_frub (num);
		frub[0].rcode = 0x0204;
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
	case DEV_INCR_ERR:
	case DEV_INCR_YY_ERR:
	case DEV_DAT_ERR:
	case DEV_DAT_YY_ERR:
		init_frub (num);
		frub[0].rcode = 0x0183;
                if (idev_id == PUCK_INPUT_DEVICE)
                	frub[0].rcode += 4;
                frub[0].rmsg = TM_TU80;
                frub[0].frus[0].conf = conf2;
                frub[0].frus[1].conf = conf3;
                strcpy (frub[0].frus[0].fname, inpt_dev);
                frub[0].frus[0].fmsg = f_msg;
                frub[0].frus[1].fru_flag = DA_NAME;
                frub[0].frus[1].fru_exempt = EXEMPT;
                add_fru ();
                exit_da ();
                break;
	default:
	/* added to capture other return codes */
		break;
	}  /* switch */
}

int     tu_10 (tu_num)
int     tu_num;
{

	if (l_mode != EXITLM)
	{
		init_menu(tu_num);
		dsply_tst_hdr(menu_nmbr);
	        tu_err = tu_test(tu_num);
		if (tu_err != 0)
	        {
	                switch (tu_err)
	                {
	                case DEV_RESET_ERR:
	                case DEV_RD_CFG_ERR:
	                case ADP_PC_CFG_ERR:
	                case ADP_PA_CFG_ERR:
			case DEV_UNKNOWN_ERR:
			case DEV_UNKNOWN_CFG_ERR:
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
			default:
				fix_common_err();
				break;		
	                }  /* end switch (tu_err) */
	        }  /* endif */
	}  /* endif */
}  /* end tu_10 */

int     tu_20 (tu_num)
int     tu_num;
{

	if (l_mode != EXITLM)
	{
		init_menu(tu_num);
		dsply_tst_hdr(menu_nmbr);
	        tu_err = tu_test(tu_num);
	        if (tu_err != 0)
	        {
	                switch (tu_err)
	                {
	                case DEV_SET_WRAP_MDE_ERR:
			case DEV_WRP_MDE_DATA_ERR:
			case ADP_PA_WRAP_ERR:
			case ADP_PC_WRAP_ERR:
			case DEV_WRP_MDE_ERR:
			case DEV_RST_WRP_MDE_ERR:
			case DEV_RST_WRP_MDE_DTA_ERR:
			case ADP_PA_RSET_WRAP_ERR:
			case ADP_PC_RSET_WRAP_ERR:
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
			default:
				fix_common_err();
				break;
	                }  /* end switch (tu_err) */
	        }  /* endif */
	}  /* endif */
}  /* end tu_20 */

int     tu_30 (tu_num)
int     tu_num;
{

	if (l_mode != EXITLM)
	{
		init_menu(tu_num);
		dsply_tst_hdr(menu_nmbr);
	        tu_err = tu_test(tu_num);
	        if (tu_err == 0)
		{
			if ((l_mode == NOTLM || l_mode == ENTERLM)
				&& c_mode == CONSOLE && s_mode == NOT_SYSTEM)
			{
				tu_err = tablet_type();	
			}
		}
	        if (tu_err != 0)
	        {
	                switch (tu_err)
	                {
	                case 0x36:
	                        DA_SETRC_TESTS (DA_TEST_FULL);
	                        exit_da ();
	                        break;
	                case DEV_SET_CONVER_ERR:
			case DEV_SET_METRIC_ERR:
			case DEV_RESLN_BYTE1_ERR:
	                case DEV_RESLN_MSB_ERR:
	                case DEV_RESLN_BYTE2_ERR:
			case DEV_RESLN_LSB_ERR:
	                        init_frub (tu_num);
	                        frub[0].rmsg = TM_TU30;
	                        frub[0].frus[0].conf = conf1;
	                        frub[0].frus[0].fru_flag = DA_NAME;
	                        frub[0].frus[0].fru_exempt = EXEMPT;
	                        add_fru ();
	                        exit_da ();
	                        break;
			default:
				fix_common_err();
				break;

	                }  /* end switch (tu_err) */
	        }  /* endif */
	}  /* endif */
}  /* end tu_30 */

int     tu_40 (tu_num)
int     tu_num;
{
	if (l_mode != EXITLM)
	{
	        tu_err = tu_test (tu_num);
		if (tu_err != 0)
	        {
	                switch (tu_err)
	                {
	                case DEV_FAIL_LED_TEST:
	                        init_frub (tu_num);
	                        frub[0].rmsg = TM_TU40;
	                        frub[0].frus[0].conf = conf1;
	                        frub[0].frus[0].fru_flag = DA_NAME;
	                        frub[0].frus[0].fru_exempt = EXEMPT;
	                        add_fru ();
	                        exit_da ();
	                        break;
			default:
				fix_common_err();
				break;
	                }  /* end switch (tu_err) */
	        }  /* endif */
	}  /* endif */
}  /* end tu_40 */

int     tu_50 (tu_num)
int     tu_num;
{

	if (l_mode != EXITLM)
	{
	        tu_err = tu_test (tu_num);
		if (tu_err != 0)
	        {
	                switch (tu_err)
	                {
	                case DEV_FAIL_ACT:
			case DEV_FAIL_INACT:
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
	                default:
				fix_common_err();
				break;
			}  /* end switch (tu_err) */
	        }  /* endif */
	}  /* endif */
}  /* end tu_50 */

int     tu_60 (tu_num)
int     tu_num;
{

	if (l_mode != EXITLM)
	{
	        tu_err = tu_test (tu_num);
	        if (tu_err != 0)
	        {
	                switch (tu_err)
	                {
	                case DEV_FL_PRES_SWTCH:
			case DEV_FL_REL_SWTCH:	
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
	                default:
				fix_common_err();
				break;

			}  /* end switch (tu_err) */
	        }  /* endif */
	}  /* endif */
}  /* end tu_60 */


int     tu_70 (tu_num)
int     tu_num;
{

	if (l_mode != EXITLM)
	{
	        tu_err = tu_test (tu_num);
	        if (tu_err != 0)
	        {
	                switch (tu_err)
	                {
			case DEV_INCR_MDE_ERR:
				init_frub (tu_num);
                                if (idev_id == PUCK_INPUT_DEVICE)
                                        frub[0].rcode += 4;
                                frub[0].rmsg = TM_TU80;
                                frub[0].frus[0].conf = conf2;
                                frub[0].frus[1].conf = conf3;
                                strcpy (frub[0].frus[0].fname, inpt_dev);
                                frub[0].frus[0].fmsg = f_msg;
                                frub[0].frus[1].fru_flag = DA_NAME;
                                frub[0].frus[1].fru_exempt = EXEMPT;
                                add_fru ();
                                exit_da ();
                                break;
			case DEV_FL_ENBL_TEST:
			case DEV_FL_DSBL_TEST:
	                        init_frub (tu_num);
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
	                default:
				fix_common_err();
				break;

			}  /* end switch (tu_err) */
	        }  /* endif */
	}  /* endif */
}  /* end tu_70 */

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
 * RETURNS: Return code from diag_display().
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
 * RETURNS: Return code from diag_display().
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
 * RETURNS: Return code from diag_display().
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
 * NAME: chk_asl_stat
 *
 * FUNCTION:  Checks keyboard input by user during diagnostics.  The escape
 *      or F3 keys will cause the DA to return to the controller with no
 *      further action.  The enter key will be treated as a command to continue
 *      testing.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

int     chk_asl_stat (asl_return)
long    asl_return;
{
        switch(asl_return)
        {
        case DIAG_ASL_OK:
        case DIAG_ASL_ENTER:
                asl_return = 0;
                break;
	case DIAG_ASL_ERR_SCREEN_SIZE:
        case DIAG_ASL_FAIL:
        case DIAG_ASL_ERR_NO_TERM:
        case DIAG_ASL_ERR_NO_SUCH_TERM:
        case DIAG_ASL_ERR_INITSCR:
                DA_SETRC_ERROR(DA_ERROR_OTHER);
		exit_da();
                break;
        case DIAG_ASL_EXIT:
                DA_SETRC_USER(DA_USER_EXIT);
                exit_da();
                break;
        case DIAG_ASL_CANCEL:
                DA_SETRC_USER(DA_USER_QUIT);
                exit_da();
                break;
        }
        return (asl_return);
}

/*
 * NAME: tablet_type
 *
 * FUNCTION:  Asks user if they are using 5083 tablet. If not, an additional
 *      screen is displayed directing the user to a hardcopy source of
 *      diagnostics tests. If yes, then tests continue normally.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: Return code from chk_asl_stat()
 */
int tablet_type()
{
        long asl_rc = 0;
        int rtn_c = 0;
        init_menu (TU30);
        menu_nmbr += 0x000001;
        asl_rc = dsply_tst_lst (menu_nmbr, TM_25A);
        if ((rtn_c = chk_asl_stat (asl_rc)) != TU_SUCCESS)
                return (rtn_c);
        if (slctn == ATU_NO)
	{
                menu_nmbr += 0x000001;
                asl_rc = dsply_tst_msg (menu_nmbr, TM_26A);
                if ((rtn_c = chk_asl_stat (asl_rc)) != TU_SUCCESS)
                        return (rtn_c);
                rtn_c = 0x36;
                return (rtn_c);
	}
	if (slctn == ATU_YES)
		return (0);
}

void    init_menu (tu_num)
int     tu_num;
{
        int     tu_menu;

        tu_menu = tu_num;
        tu_menu <<= 4;
        menu_nmbr &= 0xFFF000;
        tu_menu += 1;
        menu_nmbr += tu_menu;
}

/*
 * NAME: tu_test
 *
 * FUNCTION: Executes the test unit and receives a return code.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: Test unit return code.
 */
int     tu_test (tstptr)  /* begin tu_test */
int     tstptr;
{
	int     tu_rc = 0;
	extern	int     exectu ();

	struct	tucb
	{
		struct	tucb_t	header;
		struct	tucb_d	tuenv;
	} tuparams =
		{
			{ (int)NULL, (int)NULL, 1, (int)NULL, (int)NULL },
			{ NULL, (long)NULL, (int)NULL}
		};
	struct	tucb	*tucb_p = &tuparams;
	tucb_p->header.tu = tstptr;
	tucb_p->header.mfg = 0;
	tucb_p->header.loop = 1;

	if ( a_mode == ADVANCED )
		tucb_p->tuenv.ad_mode = 1;
	tucb_p->tuenv.catd = catd;
	tucb_p->tuenv.tabtype = tabtype;
	strcpy(tucb_p->tuenv.kbd_fd, kbddevname); 

	tu_rc = exectu (fdes, tucb_p);
        sleep(3);       /* solves timing problem on spicebush */
	if (tu_rc != 0)
	{
		switch (tu_rc)
		{
	            case EXIT_KEY_ENTERED:
			DA_SETRC_USER(DA_USER_EXIT);
			exit_da();
			break;
		    case CANCEL_KEY_ENTERED:
			DA_SETRC_USER(DA_USER_QUIT);
			exit_da();
			break;
		    default:
			break;
		}
	
		/* mask off all but the last four digits of the return code */
		tu_rc &= 0x8000FFFF;
		switch (tu_rc)
		{
			case 0xF02B:
			case 0xF02C:
			case 0xF02D:
			case 0xF02E:
			case 0xF02F:
				DA_SETRC_ERROR(DA_ERROR_OTHER);
				exit_da();
				break;
			default:
				break;
		}
	}
	return (tu_rc);

}  /* tu_test end */

/*
 * NAME: add_fru
 *
 * FUNCTION: Calls the insert_frub and addfrub functions to report the
 *      FRU number, name and confidence level when an error is detected.
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
 * FUNCTION:
 *      This is the exit point for the diagnostic application.
 *      Any devices that were opened during the execution of the
 *      diagnostic application are closed here. Keyboard diagnostics mode
 *      is disabled if it has already been enabled. If it was necessary
 *      to configure the device from within the diagnostic
 *      application then that device is unconfigured here.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is called by the main procedure which is forked and
 *      exec'd by the diagnostic supervisor, which runs as a process.
 *      This function runs an an application under the diagnostic subsystem.
 *
 * (NOTES:)
 *      Closes the keyboard device driver if successfully opened.
 *      Closes the message catalog if successfully opened.
 *      Closes the odm via the term_dgodm() call.
 *      Calls the DA_EXIT function to exit the code.
 *
 * (RECOVERY OPERATION:)
 *      Software error recovery is handled by setting
 *      error flags and returning these to the diagnostic
 *      supervisor.
 *
 * RETURNS: NONE
 */

void    exit_da ()
{
	if (((d_state != -1) || (d_state == NULL)) && (config_tminput))
	{
		if (unconfigure_lpp == TRUE)
		{
	        	d_state = initial_state (d_state, tm_input.dname);
	        	if (d_state == -1)
	        	{
	                	DA_SETRC_TESTS (DA_TEST_FULL);
	                	DA_SETRC_ERROR (DA_ERROR_OTHER);
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
        struct CuDv     *cudv_p;
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


