static char sccsid[] = "@(#)64   1.13  src/bos/usr/bin/errlg/errupdate/etcpy.c, cmderrlg, bos411, 9428A410j 10/11/93 13:17:28";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: ettocrc, majortoet
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

/*
 * FUNCTION:  ettocrc
 *    Extract the Alert ID values from the global ODM C structure T_errtmplt
 *    into a buffer. Then call crc() to generate a crc from its contents.
 *
 * RETURN VALUE:
 *    The crc of T_errtmplt
 *
 *
 * FUNCTION:  majortoet
 *    Fill in the members of the global ODM C structure T_errtmplt from
 *    the linked link of template stanza entries.
 *
 * RETURN VALUE:
 *    None
 */

#include <stdio.h>
#include <errupdate.h>
#include <parse.h>

#define CAT_LERROR ierror


static struct obj_errtmplt Etnull;

#define CP(member) \
{ \
	strcpy(cp,T_errtmplt.member); \
	cp += strlen(T_errtmplt.member); \
}

#define SP(member) \
{ \
	*cp++ = (unsigned char)((T_errtmplt.member) / 256); \
	*cp++ = (unsigned char)((T_errtmplt.member) % 256); \
}

#define LP(N,member) \
{ \
	for(i = 0; i < N; i++) { \
		if(T_errtmplt.member[i] == 0xFFFF) \
			break; \
		*cp++ = (unsigned char)((T_errtmplt.member[i]) / 256); \
		*cp++ = (unsigned char)((T_errtmplt.member[i]) % 256); \
	} \
	*cp++ = 0xFF; \
	*cp++ = 0xFF; \
}

unsigned ettocrc()
{
	unsigned char buf[512];
	unsigned char *cp;
	int i;
	unsigned rv;

	cp = buf;
	CP(et_class);
	CP(et_type);
	SP(et_desc);
	LP(4,et_probcauses);
	LP(4,et_usercauses);
	LP(4,et_useraction);
	LP(4,et_instcauses);
	LP(4,et_instaction);
	LP(4,et_failcauses);
	LP(4,et_failaction);
	LP(8,et_detail_descid);
	rv = crc(buf,cp-buf);
	return(rv);
}

#define EP(member) \
	T_errtmplt.et_/**/member = sp->s_number; ef.member++;

#define VCPY(from,to_array) \
	strncpy(to_array,from,sizeof(to_array)); \
	to_array[sizeof(to_array)-1] = '\0';

#define EPC(member) \
	VCPY(sp->s_string,T_errtmplt.et_/**/member); \
	ef.member++;

#define EP2(member) \
{ \
	symbol *tp; \
	for(tp = sp->s_symp; tp; tp = tp->s_next) { \
		if(ef.member > 3) \
			ep2error("member"); \
		else  \
			T_errtmplt.et_/**/member[ef.member++] = tp->s_number; \
	} \
}

static ep2error(member)
char *member;
{

	CAT_LERROR(CAT_UPD_TOOMANY,"Too many entries for member %s",member);
}

struct erridf {
	char class;
	char type;
	char desc;
	char probcauses;
	char usercauses;
	char useraction;
	char instcauses;
	char instaction;
	char failcauses;
	char failaction;
	char detail;
	char logflg;
	char alertflg;
	char reportflg;
	char encoding;
};

majortoet()
{
	struct erridf ef;
	symbol *sp;
	symbol *tsp;
	int i;
	int len;
	int detailidx;
	static initflg;

	if(!initflg) {
		initflg++;
		for(i = 0; i < 4; i++) {
			Etnull.et_probcauses[i] = 0xFFFF;
			Etnull.et_usercauses[i] = 0xFFFF;
			Etnull.et_useraction[i] = 0xFFFF;
			Etnull.et_instcauses[i] = 0xFFFF;
			Etnull.et_instaction[i] = 0xFFFF;
			Etnull.et_failcauses[i] = 0xFFFF;
			Etnull.et_failaction[i] = 0xFFFF;

/*	The detail data must remain 0x0000 for crcid calculation...
			Etnull.et_detail_descid[i] = 0xFFFF;
			Etnull.et_detail_descid[i+4] = 0xFFFF;
 */
		}
		Etnull.et_logflg     = ERR_LOGFLG;
		Etnull.et_alertflg   = ERR_ALERTFLG;
		Etnull.et_reportflg  = ERR_REPORTFLG;
		Etnull.et_label[0]   = '\0';
	}
	memcpy(&T_errtmplt,&Etnull,sizeof(T_errtmplt));
	memset(&ef,0,sizeof(ef));
	VCPY(Major->s_string,T_errtmplt.et_label);
	detailidx = 0;

	for(sp = Major->s_next; sp; sp = sp->s_next) {
		switch(sp->s_type) {
		case IERRCLASS: EPC(class);      break;
		case IERRTYPE:  EPC(type);       break;
		case IERRDESC:  EP(desc);        break;
		case IREPORT:   EP(reportflg);   break;
		case ILOG:      EP(logflg);      break;
		case IALERT:    EP(alertflg);    break;
		case IPROBCAUS: EP2(probcauses); break;
		case IUSERCAUS: EP2(usercauses); break;
		case IUSERACTN: EP2(useraction); break;
		case IINSTCAUS: EP2(instcauses); break;
		case IINSTACTN: EP2(instaction); break;
		case IFAILCAUS: EP2(failcauses); break;
		case IFAILACTN: EP2(failaction); break;
		case IDETAILDT:
			if(detailidx > 7) {
				cat_lerror(M(CAT_UPD_TOOMANYDET),
					"The template for %s has %d Detail Data Fields.\n\
Only %d are permitted.\n", Major->s_string, detailidx, 8);
				goto out;
			}
			tsp = sp->s_symp;
			T_errtmplt.et_detail_length[detailidx] = tsp->s_number;
			tsp = tsp->s_next;
			T_errtmplt.et_detail_descid[detailidx] = tsp->s_number;
			tsp = tsp->s_next;
			/*T_errtmplt.et_detail_encode[detailidx] = tsp->s_string[0];*/
			T_errtmplt.et_detail_encode[detailidx] = tsp->s_number;
			ef.encoding++;
			detailidx++;
			break;
		}
	}
out:
	if(Major->s_type == IPLUS) {
		if(ef.class == 0)
			CAT_LERROR(CAT_UPD_LE_CLASS,"Specify a Class in the template for %s.\n",
				Major->s_string);
		if(ef.type == 0)
			CAT_LERROR(CAT_UPD_LE_TYPE,"Specify an Err_Type in the template for %s.\n",
				Major->s_string);
		if(ef.desc == 0)
			CAT_LERROR(CAT_UPD_LE_DESC,"Specify an Err_Desc in the template for %s.\n",
				Major->s_string);
		if(ef.probcauses == 0)
			CAT_LERROR(CAT_UPD_LE_CAUSES,"Specify Prob_Causes in the template for %s.\n",
				Major->s_string);
		if((!ef.usercauses ^ !ef.useraction) ||
		   (!ef.instcauses ^ !ef.instaction) ||
		   (!ef.failcauses ^ !ef.failaction))
			CAT_LERROR(CAT_UPD_LE_CAUSEACTION,
				"Specify both the cause and the action in the template for %s.\n",
				Major->s_string);
		else if(!(ef.usercauses || ef.instcauses || ef.failcauses))
			CAT_LERROR(CAT_UPD_LE_ONECAUSE,
				"Specify at least one cause and action in the template for %s.\n",
				Major->s_string);
		len = 0;
		for(i = 0; i < detailidx; i++)
			len += T_errtmplt.et_detail_length[i];
		if(len > ERR_REC_MAX)
			CAT_LERROR(CAT_UPD_LENDET,
				"The length, %d, of a Detail Data field in the template for\n\
%s is too long.  The maximum length allowed is %d.\n",len,Major->s_string,ERR_REC_MAX);
	}
}

static ierror(n,s,a,b,c,d,e)
{

	if(Pass == 2) {
		genexit(1);
		}
	else
		cat_lerror(n,s,a,b,c,d,e);
}

