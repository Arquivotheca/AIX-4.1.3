static char sccsid[] = "@(#)62  1.6.1.6  src/bos/diag/da/iop/asniopfru.c, daiop, bos41J, 9513A_all 3/9/95 09:25:02";
/*
 *   COMPONENT_NAME: DAIOP
 *
 *   FUNCTIONS: assign_iop_frub
 *		exit_with_fru
 *		exit_without_fru
 *		
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 */

#include "diag/da.h"
#include "diag/diago.h"
#include "diag/diag_exit.h"
#include "diag/tm_input.h"
#include "diag/tmdefs.h"

#include "iop.h"
#include "ioptuname.h"
#include "iop_frus.h"
#include "diag/modid.h"

extern int get_cpu_model(int *);
extern void chk_asl_return();
struct tm_input da_input;



/*
 * NAME: void exit_with_fru(rc)
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS: NONE
*/

void exit_with_fru(rc)
{
        int save_sn = frub[rc].sn;

        strcpy(frub[rc].dname,da_input.dname);
        insert_frub(&da_input,&frub[rc]);
        frub[rc].sn = save_sn;
        addfrub( &frub[ rc] );
        DA_SETRC_STATUS(DA_STATUS_BAD);
        DA_SETRC_TESTS(DA_TEST_FULL);
        clean_up();
}

/*
 * NAME: void exit_without_fru(rc)
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS: NONE
*/

void exit_without_fru()
{
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_SETRC_TESTS(DA_TEST_FULL);
        clean_up();
}


/*
 * NAME: void assign_iop_frub(rc, tstnum)
 *
 * FUNCTION: Assign a fru bucket (defined in iop_frus.h) to
 *           be passed to the diagnostic controller for dsiplay
 *
 * EXECUTION ENVIRONMENT: Called by do_iop_tu();
 *
 * RETURNS: NONE
*/

void assign_iop_frub(rc, tstnum)
int rc, tstnum;
{
	int tmp, is_smp;

	/* flag to know if remapping to IOD specific failure codes is needed */
	is_smp = IsPowerPC_SMP(get_cpu_model(&tmp));

	if (da_input.console == CONSOLE_TRUE)
		chk_asl_return(diag_asl_read(ASL_DIAG_KEYS_ENTER_SC,NULL,NULL));

        /* ------------------------------------------------------------ */
        /* Assigns the appropriate code to the FRU Bucket               */
        /* ------------------------------------------------------------ */
        switch (rc) {
	case EC_REG_TEST_FAILURE:
                if (tstnum == EC_REG_TEST)
                        exit_with_fru(rc);
                else
                        exit_without_fru();
		break;
        case NVRAM_TEST_FAILURE:
		if (is_smp)
			rc = IOD_NVRAM_TEST_FAILURE;
                if (tstnum == NVRAM_TEST)
                        exit_with_fru(rc);
                else
                        exit_without_fru();
                break;
        case TIME_OF_DAY_TEST_FAILURE :
		if (is_smp)
			rc = IOD_TIME_OF_DAY_FAILURE;
                if (tstnum == TIME_OF_DAY_TEST1)
                        exit_with_fru(rc);
                else
                        exit_without_fru();
                break;
        case TOD_NOT_RUNNING_FAILURE  :
        case TOD_AT_POR_FAILURE :
		if (is_smp) {
			if (rc == TOD_AT_POR_FAILURE)
				rc = IOD_TOD_AT_POR_FAILURE;
			else
				rc = IOD_TOD_NOT_RUNNING_FAIL;
		}
                if (tstnum == TIME_OF_DAY_TEST2)
                        exit_with_fru(rc);
                else
                        exit_without_fru();
                break;
        case KEYLOCK_TEST_FAILURE :
		if (is_smp)
			rc = IOD_KEYLOCK_TEST_FAILURE;
                if (tstnum == POWER_STATUS_4 )
                        exit_with_fru(rc);
                else
                        exit_without_fru();
                break;
        case BATTERY_TEST_FAILURE:
                if (tstnum == BATTERY_TEST)
                        exit_with_fru(rc);
                else
                        exit_without_fru();
                break;
        case LED_TEST_FAILURE:
                if (tstnum == LED_RESET || tstnum == LED_TEST)
                        exit_with_fru(rc);
                else
                        exit_without_fru();
                break;
        case LCD_TEST_FAILURE:
                if (tstnum == LCD_RESET || tstnum == LCD_TEST)
                        exit_with_fru(rc);
                else
                        exit_without_fru();
                break;
        case EPOW_CONN_FAILURE:
                exit_with_fru(rc);
                break;
        case BBU_FAILURE:
                exit_with_fru(rc);
                break;
	case EXP_CAB_PS_FAILURE:
                exit_with_fru(rc);
                break;
        case INVALID_TU_CALL:
        case TU_TEST_ORDER_ERROR:
        case AIX_ERROR:
        default:
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                DA_SETRC_TESTS(DA_TEST_FULL);
                clean_up();
                break;
        }
}




