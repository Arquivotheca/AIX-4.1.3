static char sccsid[] = "@(#)75	1.4.1.8  src/bos/diag/da/iop/do_iop_tu.c, daiop, bos41J, 9513A_all 3/9/95 09:25:14";
/*
 *   COMPONENT_NAME: DAIOP
 *
 *   FUNCTIONS: run_tu_test
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

#include <stdio.h>
#include <fcntl.h>
#include "diag/modid.h"
#include "diag/diag_exit.h"     /* exit codes                           */
#include "ioptuname.h"
#include "iop.h"
#include "tu_type.h"
#include "diag/da.h"



extern long dev_fdes;       /* file pointer ... either NVRAM or BUS */
extern int time_reset;      /* flag set to TRUE if the time is reset */
TUTYPE iop_tucb_t;
extern int key_pos;
extern int epow_test;       /* Flag to check the EPOW connector */
			
		       	    /* flag set to TRUE if DEAD_BATTERY_TEST    */
extern int battery_failed;  /* returned a 1. Must ask battery question. */
				
void run_tu_test(tn)
int tn ;
{
	int tmp;
	int count;
	int rc = NO_ERROR ;
	int battery_status = NO_ERROR;
	int dev_fdes2;

	iop_tucb_t.header.tu = tn;
	iop_tucb_t.header.loop = 1;
	iop_tucb_t.header.mfg = iop_tucb_t.header.r1 = iop_tucb_t.header.r2 = 0;

	switch(tn)
	{
	case NVRAM_TEST: 
		rc = exectu(dev_fdes,&iop_tucb_t);
                if(rc != NO_ERROR)
                   rc = NVRAM_TEST_FAILURE;
                 break;
	case DEAD_BATTERY_TEST:
		/* ............................................ */
		/* This test is performed by making a call to   */
		/* the ioctl() function. This is not a TU test. */
		/* ............................................ */
		rc = ioctl(dev_fdes, MIONVCHCK , &battery_status);
		if (rc == AIX_ERROR)
		{
			rc = NO_ERROR;
			break;
		}
		switch(battery_status)
		{
		case 0:
			rc = NO_ERROR;
			break;
		case 1:
			rc = NO_ERROR;
			battery_failed = TRUE;
			break;
		case 2:
			rc = NVRAM_TEST_FAILURE;
			tn = NVRAM_TEST;
			break;
		default:
			rc = NO_ERROR;
			break;
		} /* end battery_status switch */
		break;
	case LED_TEST:
	case LED_RESET: 
		rc = exectu(dev_fdes,&iop_tucb_t);
                if(rc != NO_ERROR)
                   rc = LED_TEST_FAILURE;
                 break;
	case LCD_TEST:
	case LCD_RESET: 
		rc = exectu(dev_fdes,&iop_tucb_t);
                if(rc != NO_ERROR)
                   rc = LCD_TEST_FAILURE;
                 break;
	case TIME_OF_DAY_TEST1: 
		rc = exectu(dev_fdes,&iop_tucb_t);
                if(rc != NO_ERROR)
                   rc = TIME_OF_DAY_TEST_FAILURE;
                 break;
	case EC_REG_TEST2:
		tn = iop_tucb_t.header.tu = EC_REG_TEST;
		rc = exectu(dev_fdes,&iop_tucb_t);
		sleep(2);
		if(rc != AIX_ERROR && rc != INVALID_TU_CALL)
		{
			rc = NO_ERROR;
		}
		else
		{
			rc = EC_REG_TEST_FAILURE;
		}
		break;
	case EC_REG_TEST:
	case POWER_STATUS_1:
	case POWER_STATUS_2:
		rc = exectu(dev_fdes,&iop_tucb_t);
		if(rc != AIX_ERROR && rc != INVALID_TU_CALL)
		{
			if (tn == EC_REG_TEST)
			{
				if (epow_test) 
				{
					/* check the EPOW connector  */
				        /* Look at bit 5, byte 0     */
				        /* MSB 0  (byte 0)   7 LSB   */
				        /*     0 0 0 0 0 X 0 0       */
				        /*  0 = installed, 1 = not   */
					if (rc & 0x04)
						rc = EPOW_CONN_FAILURE;
					else
						rc = NO_ERROR;
				}
				else 
				{
					rc = NO_ERROR;
				}
			}
			else
				rc = NO_ERROR;
		}
		break;
	case POWER_STATUS_4:
		rc = exectu(dev_fdes,&iop_tucb_t); 

                if( (rc & KEY_MASK) != key_pos)
			rc = KEYLOCK_TEST_FAILURE; 
                else
                        rc = NO_ERROR;
		break;
	case BATTERY_TEST:
		for(count=0;count<5;count++)
		{
			iop_tucb_t.header.tu = tn;
			iop_tucb_t.header.loop = 1;
			iop_tucb_t.header.mfg = iop_tucb_t.header.r1 = iop_tucb_t.header.r2 = 0;
			rc = exectu(dev_fdes,&iop_tucb_t);
			if( rc == BATTERY_LOW)
				rc = BATTERY_TEST_FAILURE;
			else
				break;
		}
		break;
	case TIME_OF_DAY_TEST2:
		rc = exectu(dev_fdes,&iop_tucb_t);
		if( rc == TOD_AT_POR_STATE || rc == TOD_NOT_RUNNING  ) 
                       {
			time_reset = TRUE;
			tn = iop_tucb_t.header.tu = TIME_OF_DAY_SET;
			rc = exectu(dev_fdes,&iop_tucb_t);
			tn = iop_tucb_t.header.tu = TIME_OF_DAY_TEST2;
			rc = exectu(dev_fdes,&iop_tucb_t);
		        if( rc == TOD_AT_POR_STATE )
                              rc = TOD_AT_POR_FAILURE;
                        if(rc == TOD_NOT_RUNNING  )  
                              rc = TOD_NOT_RUNNING_FAILURE ;
                        } 
		break ;
	/* the BBU test is just interpretting the results of TU 9 or power
	   status 2 */
	case BBU_TEST:
		iop_tucb_t.header.tu = POWER_STATUS_2;
		rc = exectu(dev_fdes,&iop_tucb_t);

		if( (rc = (rc & 0xC0)) != 0)
		{
			if(rc == 0xC0)
				rc = 0;
			else
				rc = BBU_FAILURE;
		}
		else
			rc = 0;
		break;
	/* check power status 2 to see if there are any fan problems. */
	case FAN_CHECK:
		iop_tucb_t.header.tu = POWER_STATUS_2;
		rc = exectu(dev_fdes,&iop_tucb_t);
		/* 0xf000 are the fan bits in the first byte */
		/* 0x003c are the fan bits in the second byte */

		/* check the first byte of the status register for bad fans */
		tmp = rc >> 8;
		tmp = tmp & 0xf0;

		switch(tmp)
		{
		    case 0x90:
		    case 0xA0:
			rc = PS1_FAN_FAILURE;
			break;
		    case 0xB0:
		    case 0xC0:
			rc = PS2_FAN_FAILURE;
		    case 0xD0:
		    case 0xE0:
			rc = MEDIA_FAN_FAILURE;
			break;
		    default:
			rc = 0;
			break;
		}

		/* if fan failure was found, break */
		if(rc != 0)
			break;

		/* check the second byte of the status register for bad fans */
		tmp = rc & 0x003c;
		switch(tmp)
		{
		    case 0x0024:
			rc = MEDIA_FAN_FAILURE;
			break;
		    case 0x0028:
		    case 0x002C:
		    case 0x0030:
			rc = CPU_FAN_FAILURE;
			break;
		    default:
			rc = 0;
			break;
		}

		break;
	/* check expansion cabinet power status */
	case GET_CAB_PS_TABLE:
		{
			extern struct fru_bucket frub[];
			int cab_num;
			for (cab_num = 1; cab_num < 8; ++cab_num) {
				iop_tucb_t.header.r1 = cab_num;
				rc = exectu(dev_fdes,&iop_tucb_t);
				if (rc == 1) {
					rc = EXP_CAB_PS_FAILURE;
					(void)sprintf(frub[rc].frus[0].fname,
						"Cabinet %d", cab_num);
					break;
				}
				if (rc) {
					rc = 0;
					break;
				}
			}
		}
		break;
	default:
		break;
	}
	if(rc != NO_ERROR)
		assign_iop_frub(rc,tn);
}


