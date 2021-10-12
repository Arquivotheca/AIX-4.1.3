static char sccsid[] = "@(#)37  1.1  src/bos/diag/da/mpa/dampa_tu.c, mpada, bos411, 9428A410j 4/30/93 12:18:36";
/*
 *   COMPONENT_NAME: (MPADIAG) MP/A DIAGNOSTICS
 *
 *   FUNCTIONS: add_fru
 *		tu_test
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        "dampa.h"

tu_test (tstptr)
int     tstptr;
{
	int     tu_err = TU_GOOD;
	int     rc, tu_num, aslrc;
	long    menuid;
	int     msgid;

	tu_num = tstptr;
	sprintf(msg,"l_mode is %d...just before wrap plug check.\n",l_mode);
	RPT_BUGGER(msg);

	if ( (tstptr == TEST2 || tstptr == TEST3 || tstptr == 6) &&
	    (testmode == A_T) &&
	    (l_mode == NT_LP || l_mode == EN_LP || l_mode == IN_LP) &&
	    (tucb_ptr.pr.wrap_type == 1) && (c_mode==CNT) )
	{
	    getdavar (da_input.dname, "wrp1", DIAG_INT, &wrap_1);
	    if (wrap_1 != TRUE) {
		sprintf(msg,"wrap_1 is not TRUE. check user_said_no.\n");
		RPT_BUGGER(msg);
		/* has user already told me know once during this set */
		if (l_mode == IN_LP) {
		     getdavar (da_input.dname, "usno", DIAG_INT, &user_said_no_wrap);
		}
		if (user_said_no_wrap) {
		    sprintf(msg,"User said no return no test.\n");
		    RPT_BUGGER(msg);
		    return(TU_GOOD);
		}
		menuid = 0x996100|tu_num;
		msgid = DMMPA_8;
	        if ((rc = diag_y_n (menuid, msgid, &slctn)) != TU_GOOD)
	                return (rc);
	        if (slctn == 1)
	        {
                        sprintf(msg,"Set wrap_1 to TRUE.\n");
                        RPT_BUGGER(msg);
	                wrap_1 = TRUE;
	                if ((rc = putdavar (da_input.dname, "wrp1", DIAG_INT,
	                    &wrap_1)) == FAIL)
	                        return (rc = SW_ERR);
	        }  /* endif */
	        else
	        {
                        sprintf(msg,"Set wrap_1 to FALSE., user_said_no to TRUE\n");
                        RPT_BUGGER(msg);
	                wrap_1 = FALSE;
			user_said_no_wrap = 1;
			if (l_mode == EN_LP) {
			     putdavar (da_input.dname, "usno", DIAG_INT, &user_said_no_wrap);
			}
			putdavar (da_input.dname, "wrp1", DIAG_INT, &wrap_1);
	                if ((rc = putdavar (da_input.dname, "wrp1", DIAG_INT,
	                    &wrap_1)) == FAIL)
	                        return (rc = SW_ERR);
	                return (rc = TU_GOOD);
	        }  /* endelse */
		menuid = 0x996100|tu_num;
		msgid = DMMPA_9;
	        if ((rc = diag_action (menuid, msgid)) != TU_GOOD)
	                return (rc);
	    }
	}  /* endif */
	/* display standby screen if console TRUE */
	if (c_mode == CNT)
	{
		if ((aslrc = diag_asl_init (NULL)) != DIAG_ASL_OK)
			exit_da (exitrc);
		catd = diag_catopen(MF_DAMPA, 0);
		Menu_nmbr = 0x996100|tu_num;
		Msg_nmbr = DMMPA_1;
		if (a_mode == ADT)
		{
			if (l_mode == NT_LP || l_mode == EN_LP)
			{
				sprintf(msg,"loop mode or enter loop mode.\n");
				RPT_BUGGER(msg);
				Menu_nmbr = 0x996100|tu_num;
				Msg_nmbr = DMMPA_2;
			}  /* endif */
			else
			{
				sprintf(msg,"In loop mode use msg 3.\n");
				RPT_BUGGER(msg);
				Menu_nmbr = 0x996100|tu_num;
				Msg_nmbr = DMMPA_3;
			}  /* endif */
		}  /* endif */
		aslrc = diag_stdby (Menu_nmbr, Msg_nmbr);
		if ((exitrc = diag_asl_stat (aslrc)) != TU_GOOD)
			exit_da (exitrc);
		if(tu_num == 1 || tu_num == 5) sleep(1);
	}  /* endif */


	if (l_mode != EX_LP)
	{
		menuid = 0x996000;
	        if ((rc = exectu (fdes, &tucb_ptr)) != 0)
	        {
			sprintf(msg,"run tu%d, rc = %d, l_mode %d\n",tu_num,rc, l_mode);
			RPT_BUGGER(msg);
			switch (rc)
	                {
	                case 1:
	                case 2:
			case 3:
			case 4:
	                        strcpy (frub[0].dname, da_input.dname);
	                        frub[0].rcode = 0x100 + tu_num;
				frub[0].rmsg = DMPA_BMN + tstptr;
				sprintf(msg,"Set rmsg to %d\n",frub[0].rmsg);
				RPT_BUGGER(msg);
				frub[0].frus[0].conf = 0;
	                        frub[0].frus[1].conf = 0;
	                        frub[0].frus[2].conf = 0;
	                        strncpy(frub[0].frus[0].fname, Null, NAMESIZE);
	                        strncpy(frub[0].frus[1].fname, Null, NAMESIZE);
	                        strncpy(frub[0].frus[2].fname, Null, NAMESIZE);
	                        frub[0].frus[0].fmsg = 0;
	                        frub[0].frus[1].fmsg = 0;
	                        frub[0].frus[2].fmsg = 0;
	                        frub[0].frus[0].fru_flag = 0;
	                        frub[0].frus[1].fru_flag = 0;
	                        frub[0].frus[2].fru_flag = 0;
	                        frub[0].frus[0].fru_exempt = 0;
	                        frub[0].frus[1].fru_exempt = 0;
	                        frub[0].frus[2].fru_exempt = 0;
				frub[0].frus[0].conf = conf1;
				frub[0].frus[0].fmsg = DC_MPA;
				frub[0].frus[0].fru_flag = DA_NAME;
	                        tu_err = add_fru (&fru_set);
	                        break;
	                default:
                                sprintf(msg,"Tu returned bad rc %d\n",rc);
                                RPT_BUGGER(msg);
	                        tu_err = SW_ERR;
	                        break;
	                }  /* endswitch tu_err */
	        }  /* endif */
		else {
		     sprintf(msg,"run tu%d, rc = %d, l_mode %d\n",tu_num,rc, l_mode);
		     RPT_BUGGER(msg);
		}
	}  /* endif */
	return (tu_err);
}  /* end tu_test */
/*
 * NAME: add_fru
 *
 * FUNCTION: Update a FRU bucket with the parent device name and associates
 *      the FRU bucket with the device currently being tested.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

int     add_fru (sflag)
int     *sflag;
{
	int     rc;

	if ((rc = insert_frub (&da_input, &frub[0])) != ZERO)
	{
	        *sflag = FALSE;
	        return (SW_ERR);
	}
        
	if ((rc = addfrub (&frub[0])) != ZERO)
	{
	        *sflag = FALSE;
	        return (SW_ERR);
	}
	*sflag = TRUE;
	return (TU_BAD);
}  /* end add_fru */
