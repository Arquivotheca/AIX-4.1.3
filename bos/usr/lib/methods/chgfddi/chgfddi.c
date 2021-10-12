static char sccsid[] = "@(#)39	1.5  src/bos/usr/lib/methods/chgfddi/chgfddi.c, sysxfddi, bos411, 9428A410j 3/9/94 16:43:51";
/*
 * COMPONENT_NAME: SYSXFDDI
 *
 * FUNCTIONS:   check_parms
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Change method for FDDI */

#include <stdio.h>
#include <cf.h>		/* Error codes */

#include <sys/cfgodm.h>
#include <sys/sysconfig.h>
#include <sys/device.h>

#ifdef CFGDEBUG
#include <errno.h>
#endif

#include "cfgdebug.h"
#include "pparms.h"

static  struct    attr_list *alist=NULL;   /* PdAt attribute list      */
int     how_many;               /* Used by getattr routine.            */
int     byte_cnt;               /* Byte count of attributes retrieved  */

#define GETATT(A,B,C)           getatt(alist,C,A,B,&byte_cnt)

/*
 * NAME:
 *	check_parms
 *
 * FUNCTION:
 *      This function does FDDI specific checks on the attributes.
 *      - make sure t_req > tvx
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int 
check_parms(attrs,pflag,tflag,devname,parent,loc,badattr)
struct attr *attrs;
int	pflag, tflag;
char	*devname,*parent,*loc,*badattr;
{
	struct	attr	*att_changed(),	/* function to get address from 
					   parameter list */
			*user_data_attr,/* attribute structure in list */
			*pmf_attr,      /* attribute structure in list */
			*alt_attr;	/* attribute structure in list */
	struct Class	*cudv;		/* customized device handle */
	struct CuDv	cusobj;		/* customized device object */
	char    attr_str[8];            /* temp string to read attrs */
	char	sstring[50];		/* search string */
	int     i;                      /* temp counter variable */
	int     rc;                     /* return code from odm functions */
	uint    tmp_t_req;              /* temp storage for t_req */
	uint    tmp_tvx;                /* temp storage for tvx */
	char    tmp_user_data[256];     /* temp storage for user data */
	char    tmp_alt_addr[8];        /* temp storage for alt_addr */
	int     length_of_attr;         /* temp storage for attr length */
	char    chg_t_req[20];          /* string for error list */
	char    chg_tvx[20];            /* string for error list */

	DEBUG_0("check_parms(): BEGIN check_parms()\n")
	strcpy( badattr, "\0" );        /* set bad attr list to null */
	DEBUG_1("check_parms(): length of bad attr list = %d\n",strlen(badattr))

	/*
	 * Get customized object for to fetch attriubtes
	 */

	/* Open customized devices object class */
	if ((int)(cudv=odm_open_class(CuDv_CLASS))==-1)
		return E_ODMOPEN;

	/* Get the customized object */
	sprintf (sstring,"name = '%s'",devname);
	if ((rc=(int)odm_get_obj(cudv,sstring,&cusobj,
				ODM_FIRST)) == 0)
		/* get object failed */
		return E_NOCuDv;
	else if (rc == -1)
		/* No object returned */
		return E_ODMGET;

	if (odm_close_class(cudv) <0)
		return E_ODMCLOSE;

	/* get list of attributes just in case */
	if ((alist=get_attr_list(devname,cusobj.PdDvLn_Lvalue,&how_many,16)) == NULL)
		return(E_ODMGET);

	/*
	 * Validate attributes
	 */
	
	/* Verify "t_req" > "tvx" */
	if(alt_attr=att_changed(attrs,"t_req")) {
		/* value changed, get value from input string */
		DEBUG_4("check_parms(): %s=%s,  %s=%s\n",
			"found t_req attribute in list",alt_attr->attribute,
			"alt_attr.value",alt_attr->value)
		convert_att(&tmp_t_req,'l',alt_attr->value,'n',&byte_cnt);
		strcpy( chg_t_req, "t_req," );
	}
	else {
		/* get value from database */
		rc=GETATT(&tmp_t_req,'l',"t_req");
		strcpy( chg_t_req, "" );
	}
	DEBUG_1 ("check_parms(): t_req = %d\n",tmp_t_req)

	if(alt_attr=att_changed(attrs,"tvx")) {
		/* value changed, get value from input string */
		DEBUG_4("check_parms(): %s=%s,  %s=%s\n",
			"found tvx attribute in list",alt_attr->attribute,
			"alt_attr.value",alt_attr->value)
		convert_att(&tmp_tvx,'l',alt_attr->value,'n',&byte_cnt);
		strcpy( chg_tvx, "tvx," );
	}
	else {
		/* get value from database */
		rc=GETATT(&tmp_tvx,'l',"tvx");
		strcpy( chg_tvx, "" );
	}
	DEBUG_1 ("check_parms(): tvx = %d\n",tmp_tvx)

	if(tmp_t_req > tmp_tvx) {
		/* value ok */
		DEBUG_0("check_parms(): t_req is > tvx, ok\n");
	}
	else {
		DEBUG_0("check_parms(): t_req or tvx invalid, error\n");
		/* copy strings of what parms were changed from
		 * the command line into badattr list
		 */
		strcpy( badattr, chg_t_req );
		strcat( badattr, chg_tvx );
	}

	/* Verify "user_data" has max length of 32 characters */
	if(user_data_attr=att_changed(attrs,"user_data")) {
		/* value changed, get value from input string */
		DEBUG_4("check_parms(): %s=%s,  %s=%s\n",
			"found user_data attribute in list",user_data_attr->attribute,
			"user_data_attr.value",user_data_attr->value)
		length_of_attr = strlen(user_data_attr->value);
		DEBUG_1("check_parms(): user_data has %d bytes\n",length_of_attr);
		if ( length_of_attr > 32 ) {
			DEBUG_0("check_parms(): user_data length is to large, error\n");
			strcat( badattr, "user_data," );
		}
	}

	/* Verify "pmf_passwd" for value of 0x0 or 16 hexadecimal char */
	if(pmf_attr=att_changed(attrs,"pmf_passwd")) {
		/* value changed, get value from input string */
		DEBUG_4("check_parms(): %s=%s,  %s=%s\n",
			"found pmf_passwd attribute in list",pmf_attr->attribute,
			"pmf_attr.value",pmf_attr->value)
		DEBUG_2("check_parms(): pmf_passwd = %s and bytes = %d\n",pmf_attr->value,strlen(pmf_attr->value));

		/* check for 0x0 or 0x + 16 digits. */
		if(strlen(pmf_attr->value) != 18 && strcmp(pmf_attr->value,"0x0")) {
			strcat( badattr, "pmf_passwd," );
		}
	}

	/* Verify "alt_addr" has 12 hexadecimal digits and correct bit settings */
	if(alt_attr=att_changed(attrs,"alt_addr")) {
		/* value changed, get value from input string */
		DEBUG_4("check_parms(): %s=%s,  %s=%s\n",
			"found alt_addr attribute in list",alt_attr->attribute,
			"alt_attr.value",alt_attr->value)
		length_of_attr = strlen(alt_attr->value);
		DEBUG_1("check_parms(): alt_addr has %d bytes\n",length_of_attr);
		convert_att(tmp_alt_addr,'b',alt_attr->value,'s',&byte_cnt);
		DEBUG_0("check_parms(): alt_addr[6] = ");
		for (i=0; i<6; i++) {
		DEBUG_1("%x ",tmp_alt_addr[i]);
		}
		DEBUG_0("\n");
		/* if length != 12 + 2(0x) or first nibble not 4,5,6,7, then error */
		if ( (length_of_attr != 14) || ((tmp_alt_addr[0] & 0xc0) != 0x40) ) {
			DEBUG_0("check_parms(): alt_addr is incorrect, error\n");
			strcat( badattr, "alt_addr," );
		}
	}

	/* check for errors */
	if( strlen(badattr) > 0 ) {
		/* errors */
		DEBUG_0("check_parms(): errors occured, returning E_INVATTR\n");
		return E_INVATTR;
	}

	return 0;
}
