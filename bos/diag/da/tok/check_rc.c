static char sccsid[] = "@(#)22	1.6  src/bos/diag/da/tok/check_rc.c, datok, bos411, 9428A410j 8/19/93 15:21:39";
/*
 *   COMPONENT_NAME: DATOK
 *
 *   FUNCTIONS: check_rc_tu1
 *		check_rc_tu2
 *		check_rc_tu7
 *		check_rc_tu8
 *		report_fru
 *		wrap_failed
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/da.h"
#include "diag/diag_exit.h"
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/dcda_msg.h"
#include "diag/bit_def.h"
#include "trn.h"

extern struct fru_bucket frub710;
extern struct fru_bucket frub711;
extern struct fru_bucket frub712;
extern struct fru_bucket frub713;
extern struct fru_bucket frub714;
extern struct fru_bucket frub715;
extern struct fru_bucket frub716;
extern struct fru_bucket frub717;
extern struct fru_bucket frub720;
extern struct fru_bucket frub721;
extern struct fru_bucket frub722;
extern struct fru_bucket frub810;
extern struct fru_bucket frub811;
extern struct fru_bucket frub812;
extern struct fru_bucket frub813;
extern struct fru_bucket frub814;
extern struct fru_bucket frub815;
extern struct fru_bucket frub816;
extern struct fru_bucket frub817;
extern struct fru_bucket frub820;
extern struct fru_bucket frub821;
extern struct fru_bucket frub822;
extern struct fru_bucket frub770;
extern struct fru_bucket frub880;
extern struct fru_bucket frub910;

extern struct tm_input da_input;
extern short speed;
extern uchar wrap_attempt;
extern short wiring;
extern char dname[NAMESIZE];

/*------------------------------------------------------------------------
 * NAME: check_rc_tu1
 *
 * FUNCTION: checks return code from test unit 1.  If it is valid it
 *		reports the appropriate frus.  Otherwise it changes the
 *		code to indicate a software error.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 	rc	the value as passed to it, unchanged
 *		0x9999	A software error occurred	
 *
 *------------------------------------------------------------------------*/

int check_rc_tu1(rc)
int rc;			/* return code from TU 1 */
{
	switch(rc)
	{
		case 0x01010030:
		case 0x01010031:
		case 0x01010032:
		case 0x01010033:
		case 0x01010034:
		case 0x01010035:
		case 0x01010036:
		case 0x01010018:
		case 0x01010019:
		case 0x0101001a:
		case 0x0101001b:
		case 0x0101001d:
		case 0x0101ffda:
			switch(speed)
				{
				case 7:
					report_fru(&frub710);
					break;
				case 8:
					report_fru(&frub810);
					break; 
				}
			break;
		case 0x01010011:
		case 0x01010012:
		case 0x01010013:
		case 0x01010014:
		case 0x01010015:
		case 0x01010016:
		case 0x01010017:
		case 0x01018c00:
		case 0x0101a400:
		case 0x0101ffdb:
		case 0x01020051:
		case 0x01020052:
		case 0x01020053:
		case 0x01020110:
		case 0x01020104:
		case 0x01020105:
			switch(speed)
			{
				case 7:
					report_fru(&frub712);
					break;  
				case 8:
					report_fru(&frub812);
					break;
			}
			break;
		case 0x0101001c:
		case 0x01018830:
		case 0x01018832:
		case 0x01018842:
		case 0x0101ffdc:
			switch(speed)
			{
				case 7:
					report_fru(&frub713);
		 			break;
				case 8:
					report_fru(&frub813);
					break;
			}
			break;
		case 0x01010211:
			wrap_failed();
			break;
		case 0x01019c00:
		case 0x01020020:
		case 0x01020050:
		case 0x01020055:
		case 0x01020084:
		case 0x01020085:
		case 0x01020094:
		case 0x01020095:
		case 0x01000002:
		case 0x01000006:
		case 0x01000009:
		case 0x01000011:
		case 0x0100000c:
		case 0x01000045:
			DA_SETRC_ERROR (DA_ERROR_OTHER);
			break;
		default:
			/* unrecognized return code */
			report_fru(&frub910);
			break;
	}
	return(rc);
}
/*-----------------------------------------------------------------------
 * NAME: check_rc_tu2
 *
 * FUNCTION: checks return code from test unit 2.  If it is valid it
 *		reports the appropriate frus.  Otherwise it changes the
 *		code to indicate a software error.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 	rc	the value as passed to it, unchanged
 *		0x9999	A software error occurred	
 *
 *------------------------------------------------------------------------*/


int check_rc_tu2(rc)
int rc;			/* return code from TU 1 */
{
	switch(rc)
		{
		case 0x02019000: 
		case 0x02019402: 
		case 0x02019403: 
		case 0x0201a400: 
		case 0x0201ffda: 
		case 0x02020060: 
		case 0x02020062: 
		case 0x02020064: 
		case 0x02020065: 
			switch(speed)
			{
				case 7:
					report_fru(&frub720);
					break;
				case 8:
					report_fru(&frub820);
					break;
			}
			break;
		case 0x02019400:
			switch(speed)
			{
				case 7:
					report_fru(&frub721);
					break; 
				case 8:
					report_fru(&frub821);
					break; 
			}
			break;
		case 0x02019100:
		case 0x02019401:
			switch(speed)
			{
				case 7:
					report_fru(&frub722);
					break;
				case 8:
					report_fru(&frub822);
					break;
			}
			break;
		case 0x02020001:
		case 0x02020010:
		case 0x02020011:
		case 0x02000006:
		case 0x02000009:
		case 0x02000045:
		case 0x02000046:
		case 0x0200004a:
			DA_SETRC_ERROR (DA_ERROR_OTHER);
			break;
		case 0x0202cccc:
			/* asynchronous channel check - no FRU */
			rc = 0;
			break;
		default:
			/* unrecognized return code */
			report_fru(&frub910);
			break;
		}
	return(rc);
}
/*-----------------------------------------------------------------------------
 * NAME: check_rc_tu7
 *
 * FUNCTION: checks return code from test unit 7.  If it is valid it
 *		reports the appropriate frus.  Otherwise it changes the
 *		code to indicate a software error.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 	rc	the value as passed to it, unchanged
 *		0x9999	A software error occurred	
 *
 *---------------------------------------------------------------------------*/

int check_rc_tu7(rc)
int rc;			/* return code from TU 1 */
{
	/* process results of TU 7. */
	switch(rc)
	{
		case 0x00000000:
			/* test passed */
			break;
		case 0x07010095:
		case 0x07020080:
		case 0x07020090:
		case 0x07020103:
			/* test failed with recognized return code
			 * SRN = 770
			*/
			report_fru(&frub770);
			break;	
		case 0x07020055:
		case 0x07020083:
		case 0x07020093:
		case 0x07000006:
		case 0x07000009:
			DA_SETRC_ERROR (DA_ERROR_OTHER);
			break;
		default:
			/* unrecognized return code */
			report_fru(&frub910);
			break;
	}
	return(rc);
}
/* -----------------------------------------------------------------------
 * NAME: check_rc_tu8
 *
 * FUNCTION: checks return code from test unit 8.  If it is valid it
 *		reports the appropriate frus.  Otherwise it changes the
 *		code to indicate a software error.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 	rc	the value as passed to it, unchanged
 *		0x9999	A software error occurred	
 *
 * ----------------------------------------------------------------------*/
int check_rc_tu8(rc)
int rc;			/* return code from TU 8 */
{
	switch(rc)
	{
		case 0x00000000:
			/* test passed */
			break;
		case 0x08010095:
		case 0x08020080:
		case 0x08020090:
		case 0x08020103:
			/* test failed with recognized return code.
			 * SRN = 880
			*/
			report_fru(&frub880);
			break;	
		case 0x08020055:
		case 0x08020083:
		case 0x08020093:
		case 0x08000006:
		case 0x08000009:
			DA_SETRC_ERROR (DA_ERROR_OTHER);
			break;
		default:
			/* unrecognized return code */
			report_fru(&frub910);
			break;
	}
	return(rc);

}
/* -----------------------------------------------------------------------
 * NAME: wrap_failed 
 *
 * FUNCTION: 
 * 
 * EXECUTION ENVIRONMENT:
 *
 * ----------------------------------------------------------------------*/
wrap_failed()
{
	switch(speed)
	{
		case 7:
			switch(wiring)
			{
				case NONTELE:
					switch(wrap_attempt)
					{
						case FALSE:
							report_fru(&frub714);
							break;
						case TRUE:
							report_fru(&frub715);
							break;
					}
					break;
				default:
					switch(wrap_attempt)
					{
						case FALSE:
							report_fru(&frub716);
							break;
						case TRUE:
							report_fru(&frub717);
							break;
					}
					break;
			}
 			break;
		case 8:
			switch(wiring)
			{
				case NONTELE:
					switch(wrap_attempt)
					{
						case FALSE:
							report_fru(&frub814);
							break;
						case TRUE:
							report_fru(&frub815);
							break;
					}
					break;
				default:
					switch(wrap_attempt)
					{
						case FALSE:
							report_fru(&frub816);
							break;
						case TRUE:
							report_fru(&frub817);
							break;
					}
					break;
			}
 			break;
	}
}
/*-------------------------------------------------------------------------
 * NAME: report_fru
 *
 * FUNCTION: reports the fru bucket whose address is specified as frub_addr
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 *--------------------------------------------------------------------------*/

report_fru(frub_addr)
struct fru_bucket *frub_addr;
{
	strcpy ( frub_addr->dname, dname);
	insert_frub(&da_input, frub_addr);
	addfrub(frub_addr);
	DA_SETRC_STATUS (DA_STATUS_BAD);
}
