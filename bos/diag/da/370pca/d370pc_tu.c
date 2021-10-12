static char sccsid[] = "@(#)71	1.2  src/bos/diag/da/370pca/d370pc_tu.c, da370pca, bos412, 9445Xdiag 11/2/94 15:30:06";
/*
 * COMPONENT_NAME: DA370PCA
 *
 * FUNCTIONS:   tu_test ()
 *              add_fru ()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include        "d370pc.h"

/*
 *NAME: tu_test
 *
 * FUNCTION: Executes the test unit and returns the completion code to the
 *      calling function.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

tu_test (tstptr)
int     tstptr;
{
	int     tu_err = TU_GOOD;
	int     rc, tu_num, aslrc;
	long    menuid;
	int     msgid;
	char	*buf;

	tu_num = tstptr;
	if (tstptr == TEST10 && (l_mode == NT_LP || l_mode == EN_LP))
	{
	        menuid = 0x862104;
	        msgid = DM370PC_7;
	        if ((rc = diag_y_n (menuid, msgid, &slctn)) != TU_GOOD)
	                return (rc);
	        if (slctn == 1)
	                ch_off = TRUE;
		else
		{
			ch_off = FALSE;
			if (l_mode == EN_LP)
			{
				testmode = N_I;
	               		rc = putdavar (da_input.dname, "mode",
				    DIAG_INT, &testmode);
	               		if (rc == FAIL)
	                       		return (rc = SW_ERR);
				maxtest = SET_0;
	               		rc = putdavar (da_input.dname, "max",
				    DIAG_INT, &maxtest);
	               		if (rc == FAIL)
	                       		return (rc = SW_ERR);
			}		
			return (rc = DONE);
	        }
	        menuid = 0x862105;
	        msgid = DM370PC_8;
	        if ((rc = diag_y_n (menuid, msgid, &slctn)) != TU_GOOD)
	                return (rc);
	        if (slctn == 1)
	                wrap_1 = TRUE;
	        else
		{
	                wrap_1 = FALSE;
	                return (rc = TU_GOOD);
	        }  /* endelse */
	        menuid = 0x862106;
	        msgid = DM370PC_9;
	        if ((rc = diag_action (menuid, msgid)) != TU_GOOD)
	                return (rc);
	}  /* endif */
	if ((tstptr == TEST15 && ch_off == TRUE) &&
	    (l_mode == NT_LP || l_mode == EN_LP))
	{
	        menuid = 0x862108;
	        msgid = DM370PC_11;
	        if ((rc = diag_y_n (menuid, msgid, &slctn)) != TU_GOOD)
	                return (rc);
	        if (slctn == 1)
	        {
	                wrap_2 = TRUE;
	                wrap_3 = TRUE;
	               	rc = putdavar (da_input.dname, "mode",
			    DIAG_INT, &testmode);
	               	if (rc == FAIL)
	                    	return (rc = SW_ERR);
	               	rc = putdavar (da_input.dname, "max",
			    DIAG_INT, &maxtest);
	               	if (rc == FAIL)
	                      	return (rc = SW_ERR);
			if (l_mode == EN_LP && wrap_1 == TRUE)
			{
				menuid = 0x862107;
				msgid = DM370PC_10;
				rc = diag_action (menuid, msgid);
				if (rc != TU_GOOD && tu_err == TU_GOOD)
					return (rc);
			}
	        }  /* endif */
	        else
	        {
	                wrap_2 = FALSE;
	                wrap_3 = FALSE;
			if (wrap_1 == TRUE && l_mode == EN_LP)
			{
				testmode = R_1;
	               		rc = putdavar (da_input.dname, "mode",
				    DIAG_INT, &testmode);
	               		if (rc == FAIL)
	                       		return (rc = SW_ERR);
				maxtest = SET_1;
	               		rc = putdavar (da_input.dname, "max",
				    DIAG_INT, &maxtest);
	               		if (rc == FAIL)
	                       		return (rc = SW_ERR);
			}		
			else
			{
				testmode = N_I;
	               		rc = putdavar (da_input.dname, "mode",
				    DIAG_INT, &testmode);
	               		if (rc == FAIL)
	                       		return (rc = SW_ERR);
				maxtest = SET_0;
	               		rc = putdavar (da_input.dname, "max",
				    DIAG_INT, &maxtest);
	               		if (rc == FAIL)
	                       		return (rc = SW_ERR);
			}		
	                return (rc = TU_GOOD);
	        }  /* endelse */
	        menuid = 0x862109;
	        msgid = DM370PC_12;
	        if ((rc = diag_action (menuid, msgid)) != TU_GOOD)
	                return (rc);
	}  /* endif */
	if (c_mode == CNT && (tstptr == TEST10 || tstptr == TEST15))
	{
	        aslrc = diag_stdby (Menu_nmbr, Msg_nmbr);
	        if ((rc = diag_asl_stat (aslrc)) != TU_GOOD)
	                return (rc);
	}
	if (l_mode != EX_LP)
	{
	        menuid = 0x862000;
	        if ((rc = exectu (fdes, &tucb_ptr)) != 0)
	        {
	                switch (rc)
	                {
	                case 1:
	                case 2:
	                case 4:
	                        if (tstptr > 9 && tstptr < 20)
	                                tu_num = tucb_ptr.header.tu + 6;
	                        if (tstptr >= 20)
	                                tu_num = tucb_ptr.header.tu + 12;
	                        strcpy (frub[0].dname, da_input.dname);
	                        frub[0].rcode = 0x100 + tu_num;
	                        frub[0].rmsg = D370_BMN + tstptr;
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
	                        switch (frub[0].rcode)
	                        {
	                        case 0x102:
	                                frub[0].frus[0].conf = conf2;
	                                frub[0].frus[1].conf = conf6;
	                                frub[0].frus[0].fru_flag = DA_NAME;
	                                frub[0].frus[1].fru_flag = PARENT_NAME;
	                                break;
	                        case 0x115:
	                                frub[0].frus[0].conf = conf1;
	                                frub[0].frus[0].fmsg = D370_CCIB;
	                                strcpy (frub[0].frus[0].fname, fnam1);
	                                break;
	                        default:
	                                frub[0].frus[0].conf = conf1;
	                                frub[0].frus[0].fmsg = DC_370PCA;
	                                frub[0].frus[0].fru_flag = DA_NAME;
	                                break;
	                        }
	                        tu_err = add_fru (&fru_set);
	                        break;
	                case 3:
	                default:
	                        tu_err = SW_ERR;
	                        break;
	                }  /* endswitch tu_err */
	        }  /* endif */
		if (c_mode == CNT)
		{
			aslrc = diag_asl_read (ASL_DIAG_OUTPUT_LEAVE_SC,
				FALSE, buf);
	        	if ((rc = diag_asl_stat (aslrc)) != TU_GOOD)
	                	return (rc);
	        }  /* endif */
	}  /* endif */
	switch (tstptr)
	{
	case TEST10:
		if ((l_mode == NT_LP) ||
		    (l_mode == EX_LP && testmode == R_1))
		{
			menuid = 0x862107;
			msgid = DM370PC_10;
			rc = diag_action (menuid, msgid);
			if (rc != TU_GOOD && tu_err == TU_GOOD)
				return (rc);
		}
		break;
	case TEST15:
		if (l_mode == NT_LP || l_mode == EX_LP)
		{
			menuid = 0x862110;
			msgid = DM370PC_13;
			rc = diag_action (menuid, msgid);
			if (rc != TU_GOOD && tu_err == TU_GOOD)
				return (rc);
		}
		break;
	} /* endswitch tstptr */
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
