static char sccsid[] = "@(#)69  1.4  src/bos/usr/bin/errlg/errupdate/alertable.c, cmderrlg, bos412, 9446A412a 11/11/94 19:58:05";


/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS:  alertable, not_rsv_cdpts, rsvd_cdpt_SNA, err_SNA
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Functions to process codepoint restrictions.
 */

#include <errupdate.h>
#include <parse.h>

extern pflg;

int alertable(void);
int	not_rsvd_cdpts(void);
int	rsvd_cdpt_SNA(int cdpt_field,unsigned short codept,char set);
void err_SNA(int cdpt_field,char set,unsigned short codept);

/*
 * NAME:      alertable
 * FUNCTION:  Check a given error template that is requested alertable
 *            against the rules for alertablility. This function assumes
 *            the alert flag is set for the given template.
 * RETURNS:
 *            1 - template passes rules to be alertable.
 *            0 - template does NOT pass rules be alertable.
 */

 int
 alertable()
 {
	int	rc;

	if ( strcmp(T_errtmplt.et_type,"INFO") == 0 ) {
		/*
		 * Enforce rule that error type INFO 
		 * templates are not allowed to be alertable.
		 */
		cat_error(CAT_UPD_NOT_ALERTABLE_INFO,
"\nThe template for %s has the Err_Type field set to INFO\n\
and the Alert field set to TRUE.  Error Type INFO is not permitted\n\
in an Alert.\n\
Possible solutions are:\n\
1.  Change the Err_Type field to one that is permitted in an\n\
    Alert (UNKN, TEMP, PERM, PEND or PERF).\n\
2.  Change the Alert field to FALSE.\n", T_errtmplt.et_label); 
		rc = 0;
	}
	else if (strcmp(T_errtmplt.et_class,"O") == 0 ) {
		/*
		 * Enforce rule that error class Operator 
		 * templates are not allowed to be alertable.
		 */
		cat_error(CAT_UPD_NOT_ALERTABLE_OP,
"\nThe template for %s has the Class field set to O and\n\
the Alert field set to TRUE.  Error Class OPERATOR is not permitted in\n\
an Alert.\n\
Possible solutions are:\n\
1.  Change the Class field to one that is permitted in an Alert\n\
    (H, S, or U).\n\
2.  Change the Alert field to FALSE.\n",T_errtmplt.et_label); 
		rc = 0;
	}
	else if (pflg)		/* override other codepoint checking */
		rc = 1;
	else if (not_rsvd_cdpts())
		/*
		 *	Enforce rule that nonreserved codepoints may not be used
		 *	with alertable templates, unless override is requested.
		 */
		rc = 0;

	else
		/*
		 *  allow alertability
		 */
		rc = 1;

	return(rc);
 }

/*
 * NAME:      not_rsvd_cdpts
 * FUNCTION:  Check a given error template for using reserved
 *            codepoints.
 *
 *            Note that '0xFFFF' indicates that the entry was not
 *            set in the update file, hence no subsequent entries
 *            for that member were set and will not be checked.
 * RETURNS:
 *            1 - Template contains nonreserved codepoints.
 *            0 - Template contains reserved codepoint(s).
 */

static int
not_rsvd_cdpts()
{

	int	i;
	int	rc=0;

	if( T_errtmplt.et_desc != 0xFFFF && !rsvd_cdpt_SNA(IERRDESC,T_errtmplt.et_desc,'E'))
		rc=1;

	for(i=0; i<4; i++) {
		if( T_errtmplt.et_probcauses[i] == 0xFFFF)
			break;
		if(!rsvd_cdpt_SNA(IPROBCAUS,T_errtmplt.et_probcauses[i],'P')) {
			rc=1;
		}
	}

	for(i=0; i<4; i++) {
		if( T_errtmplt.et_usercauses[i] == 0xFFFF)
			break;
		if(!rsvd_cdpt_SNA(IUSERCAUS,T_errtmplt.et_usercauses[i],'U')) {
			rc=1;
		}
	}

	for(i=0; i<4; i++) {
		if( T_errtmplt.et_instcauses[i] == 0xFFFF)
			break;
		if(!rsvd_cdpt_SNA(IINSTCAUS,T_errtmplt.et_instcauses[i],'I')) {
			rc=1;
		}
	}

	for(i=0; i<4; i++) {
		if( T_errtmplt.et_failcauses[i] == 0xFFFF)
			break;
		if(!rsvd_cdpt_SNA(IFAILCAUS,T_errtmplt.et_failcauses[i],'F')) {
			rc=1;
		}
	}

	for(i=0; i<4; i++) {
		if( T_errtmplt.et_useraction[i] == 0xFFFF)
			break;
		else if(!rsvd_cdpt_SNA(IUSERACTN,T_errtmplt.et_useraction[i],'R')) {
			rc=1;
		}
	}

	for(i=0; i<4; i++) {
		if( T_errtmplt.et_instaction[i] == 0xFFFF)
			break;
		else if(!rsvd_cdpt_SNA(IINSTACTN,T_errtmplt.et_instaction[i],'R')) {
			rc=1;
		}
	}

	for(i=0; i<4; i++) {
		if( T_errtmplt.et_failaction[i] == 0xFFFF)
			break;
		else if(!rsvd_cdpt_SNA(IFAILACTN,T_errtmplt.et_failaction[i],'R')) {
			rc=1;
		}
	}

	for(i=0; i<8; i++) {	/* note there are EIGHT to check */
		if( T_errtmplt.et_detail_descid[i] == 0xFFFF)
			break;
		else if(!rsvd_cdpt_SNA(IDETAILDT,T_errtmplt.et_detail_descid[i],'D')) {
			rc=1;
		}
	}

	if (rc == 1) {
			cat_error(CAT_UPD_RSVD_CDPT, "\
The template for %s was not added because the Alert field\n\
is set to TRUE and the template contains the message id %s which is\n\
not permitted in an Alert.\n\
Possible solutions are:\n\
1.  Change the message id to one that is permitted in Alerts.\n\
2.  Change the Alert field to FALSE.\n\
3.  Run the errupdate command with the -p option to override this\n\
    restriction.  If you do this, you must also customize NetView to\n\
    recognize this message id when it receives this Alert.\n",T_errtmplt.et_label,T_errtmplt.et_crcid);
	}

	return (rc);
}

/*
 * NAME:      rsvd_cdpt_SNA
 * FUNCTION:  Check if a given codepoint is within the SNA
 *            reserved range for a given message set.
 * RETURNS:
 *            0 - codepoint IS NOT within SNA range for the given
 *                set, or message set is invalid.
 *            1 - codepoint IS within SNA range for the given  set.
 */

static int
rsvd_cdpt_SNA(
	int			cdpt_field,			/* template field using codepoint */
	unsigned 	short	codept,		/* codepoint msg number */
	char		set)				/* codepoint msg set */
{
	int	rc=0;

	switch (set) {
	case 'E':	/* Error Description */
	case 'P':	/* Probable Cause */
	case 'U':	/* User Cause */
	case 'I':	/* Install Cause */
	case 'F':	/* Fail Cause */
	case 'R':	/* XXXX Actions */
		if ((codept >= 0x0000 && codept <= 0xDFFF) ||
			(codept >= 0xF000 && codept < 0xFFFF))
			rc=1;
		break;

	case 'D':	/* Detail Description */
		if ((codept >= 0x0000 && codept <= 0x00DF) || (codept == 0x8001))
			rc=1;
		break;

	}

	if (!rc)
		err_SNA(cdpt_field,set,codept);

	return(rc);
}

/*
 * NAME:      err_SNA
 * FUNCTION:  Issue an error messag annunciating a bad codepoint.
 * RETURNS:   None.
 *
 */

static void
err_SNA(
	int cdpt_field,
	char set,
	unsigned short codept)
{
	char		*cdpt_field_name;
	int zero,df,dfff,f000,fffe;
	zero = 0x0000;
	df = 0x00df;
        dfff = 0xdfff;
        f000=0xf000;
        fffe=0xfffe;
	switch (cdpt_field) {
	case IERRDESC:
		cdpt_field_name = "Error Description";
		break;
	case IPROBCAUS:
		cdpt_field_name = "Probable Causes";
		break;
	case IUSERCAUS:
		cdpt_field_name = "User Causes";
		break;
	case IUSERACTN:
		cdpt_field_name = "User Actions";
		break;
	case IINSTCAUS:
		cdpt_field_name = "Install Causes";
		break;
	case IINSTACTN:
		cdpt_field_name = "Install Actions";
		break;
	case IFAILCAUS:
		cdpt_field_name = "Fail Causes";
		break;
	case IFAILACTN:
		cdpt_field_name = "Fail Actions";
		break;
	case IDETAILDT:
		cdpt_field_name = "Detail Description";
		break;
	default:
		cdpt_field_name = "UNKNOWN";
	}

	if(cdpt_field == IDETAILDT) 	/* only one subrange is alertable. */
		cat_error(CAT_UPD_RSVD_CDPT_DETAIL,
"For alertable template, \"%s\", \"%s\" must be\n\
in the range:\n\t%4.4x to %4.4x or equal to 8001.\n\
The supplied message text identifier %4.4x is NOT in range.\n\n"
,T_errtmplt.et_label,cdpt_field_name,zero,df,(unsigned int)codept);

	else
		cat_error(CAT_UPD_RSVD_CDPT_GENERIC,
"For alertable template, \"%s\", \"%s\" must be\n\
in the range:\n\t%4.4x to %4.4x or %4.4x to %4.4x\n\
The supplied message text identifier %4.4x is NOT in range.\n\n"
,T_errtmplt.et_label,cdpt_field_name,zero,dfff,f000,fffe,(unsigned int)codept);

}
