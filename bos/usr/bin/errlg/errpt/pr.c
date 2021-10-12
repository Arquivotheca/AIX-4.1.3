static char sccsid[] = "@(#)25	1.31  src/bos/usr/bin/errlg/errpt/pr.c, cmderrlg, bos411, 9428A410j 6/14/94 15:43:25";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: pr_init, pr_logentry, pr_tmpltentry, hextoascii, acsiitohex
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Functions to print entries for errpt.  The entries may be summary
 * or detailed, log or template entries.
 */

#define _ILS_MACROS
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/uio.h>
#include <ctype.h>
#include <langinfo.h>
#include <locale.h>
#include <time.h>
#include <sys/err_rec.h>
#include <sys/rasprobe.h>
#include <errlg.h>

/* extern rawflg; */
extern asciiflg;

char *hextoascii();

static char *timestr();
static char *datestr();
static char *descstr();

static char *msg_desc;
static char *msg_probcauses;
static char *msg_usercauses;
static char *msg_instcauses;
static char *msg_failcauses;
static char *msg_recactions;
static char *msg_detaildata;
static char *msg_symptomdata;

static char *msg_hdr;
static char *msg_ahdr;        /* Detailed Error Report header */
static char *msg_ohdr;     /* other data header */
static char *msg_thdr;
static char *msg_tahdr;
static char *msg_vhdr;     /* vpd data header */
static char *msg_fmt;
static char *msg_ofmt;     /* other data format */
static char *msg_tfmt;
static char *vpd_vtext;    /* vpd text, already formated */

#define MAX_SSCODEPT_STR_SIZE   SSDATA_MAX + 14 /* 14 includes flag codepoints*/
#define MAX_SS_CODEPTS          SSKWD_MAX + 2  /* 2 includes flag codepoints */

/* called by /errpt/main.c */
pr_init()
{
	char *s;

	cat_string(CAT_RPT_ERRDESC,    &s,"Description");
	msg_desc = stracpy(s);
	cat_string(CAT_RPT_PROBCAUSES, &s,"Probable Causes");
	msg_probcauses = stracpy(s);
	cat_string(CAT_RPT_USERCAUSES, &s,"User Causes");
	msg_usercauses = stracpy(s);
	cat_string(CAT_RPT_INSTCAUSES, &s,"Install Causes");
	msg_instcauses = stracpy(s);
	cat_string(CAT_RPT_FAILCAUSES, &s,"Failure Causes");
	msg_failcauses = stracpy(s);
	cat_string(CAT_RPT_RECACTIONS, &s,"Recommended Actions");
	msg_recactions = stracpy(s);
	cat_string(CAT_RPT_DETAIL_DATA,&s,"Detail Data");
	msg_detaildata = stracpy(s);
        cat_string(CAT_RPT_SYMPTOM_DATA,&s,"Symptom Data");
        msg_symptomdata = stracpy(s);

	/** Error templates header. */
	cat_string(CAT_RPT_HDR,&s,"\
IDENTIFIER TIMESTAMP  T C RESOURCE_NAME  DESCRIPTION\n");
	msg_hdr = stracpy(s);
	msg_fmt = "\
%08X   %10.10s %1.1s %1.1s %-14.14s %-.39s\n";

	/* Labels for detailed error report. This is default group. Others follow. */
	cat_string(CAT_RPT_AHDR,&s,"\
---------------------------------------------------------------------------\n\
LABEL:\t\t%s\n\
IDENTIFIER:\t%08X\n\
\n\
Date/Time:       %s\n\
Sequence Number: %d\n\
Machine Id:      %s\n\
Node Id:         %s\n\
Class:           %s\n\
Type:            %s\n\
Resource Name:   %s\n");
	msg_ahdr = stracpy(s);            /* Detailed Error Report labels */

	/* Labels for other data fields: these omitted on type = S or O. */
	cat_string(CAT_RPT_OHDR,&s,"\
Resource Class:  %s\n\
Resource Type:   %s\n\
Location:        %s\n");
	msg_ohdr = stracpy(s);            /* Detailed Error Report labels */

	/* Labels for VPD data fields: these omitted on type = S or O. */
	cat_string(CAT_RPT_VHDR,&s,"\
VPD:             \n");
	msg_vhdr = stracpy(s);            /* Detailed Error Report labels */

	/** T flag labels and header. */
	cat_string(CAT_RPT_THDR,&s,"\
Id       Label               Type CL Description\n");
	msg_thdr = stracpy(s);
	msg_tfmt = "\
%08X %-19.19s %-4.4s %1.1s  %-40.40s\n",

	    /** Labels for detailed errlog templates, not log entries. */
	cat_string(CAT_RPT_TAHDR,&s,"\
---------------------------------------------------------------------------\n\
IDENTIFIER %08X\n\
\n\
Label: %s\n\
Class: %s\n\
Type:  %s\n\
Loggable: %s   Reportable: %s   Alertable: %s\n\n");
	msg_tahdr = stracpy(s);
}

/*
 * NAME:      pr_logentry
 * FUNCTION:  print log entry under control of the detailedflg
 *            The entry is in the global structure T_errlog.
 *            If detailedflg is not set, print just a summary report.
 *            Otherwise, print the full entry including symptom data
 * 	      if T_errlog.el_symptom_length != 0. 
 *
 * INPUTS:    NONE
 * RETURNS:   NONE
 */

/* called by /errpt/rpt.c */
pr_logentry()
{
	static recordno;
	int rc = 0;

	if(asciiflg) {
		raw_ascii();
		rc = 0;
/*	} else if(rawflg) {
		pr_raw();
		rc = 0; */
	}
	else if(!detailedflg) {
		if(recordno == 0) {     /* print the header */
			printf(msg_hdr);
		}
		recordno++;
		printf(msg_fmt,
		    T_errlog.el_crcid,
		    datestr(T_errlog.el_timestamp),
		    T_errlog.el_type,
		    T_errlog.el_class,
		    T_errlog.el_resource,
		    descstr());
		rc = 0;
	}  /* Detailed Error Report header */
	else {
		printf(msg_ahdr,
		    T_errlog.el_label,
		    T_errlog.el_crcid,
		    timestr(T_errlog.el_timestamp),
		    T_errlog.el_sequence,
		    T_errlog.el_machineid,
		    T_errlog.el_nodeid,
		    T_errlog.el_class,
		    T_errlog.el_type,
		    T_errlog.el_resource);

		/* Print rclass, el_rtype, el_in, and el_vpd if el_class = H or U */
		if (!strcmp(T_errlog.el_class,"H") || !strcmp(T_errlog.el_class,"U")) {
			pr_other();
			if (T_errlog.el_vpd_ibm)
				pr_vpd(); /* Call routine to call routines that parse vpd info. */
		}

		/* Print an extra line to seperate the description causes. */
		printf("\n");

		pr_desc();
		pr_logdetail();

                /* Check to see if any symptom data exists, */
		/* if it does print it. 		    */
                if ( T_errlog.el_symptom_length != 0 )
                   pr_symptomdata();

		rc = 0;
	}

	fflush(stdout);
	return(rc);

}

#define AC0(c) ( (c) >= 'A' ? (c) - 'A' + 0x0A : (c) - '0' )
#define ACU(c) (AC0(c) << 4)
#define ACL(c) (AC0(c))

#define HC0(c) ( (c) >= 0xA ? (c) + 'A' - 0x0A : (c) + '0' )
#define HCU(c) ( HC0(((c) >> 4) & 0x0F) )
#define HCL(c) ( HC0( (c)       & 0x0F) )

static pr_logdetail()
{
	int i,j;
	int n;
	char *buf;
	int wcount;
	int c,c1;
	int encode,length,descid;
	int dsize;
	int nlflg;

	buf = T_errlog.el_detail_data;
	dsize = T_errlog.el_detail_length;
	nlflg = 0;
	for(i = 0; i < 8; i++) {
		descid = T_errtmplt.et_detail_descid[i];
		encode = T_errtmplt.et_detail_encode[i];
		length = MIN(T_errtmplt.et_detail_length[i],dsize);
		if(encode == 0)
			break;
		if(i == 0)
			printf("%s\n",msg_detaildata);
		printf("%s\n",codeptstr(descid,ERRSET_DETAILDATA));
		nlflg = 0;
		switch(encode) {
		case 'a':
		case 'A':
			for(j = 0; j < length; j++) {
				if((c = buf[j]) == '\0')
					break;
				else if(c == '\n' || c == '\t')
					putchar(c);
				else if(isprint(c)) {
				/*	if(rawflg && c == '\n') {
						putchar('~');
						putchar('~');
					}
					else */
						putchar(c);
				}
				else
					printf("-%s-",hextoascii(c));
			}
			break;
		case 'x':
		case 'X':
		case 'h':
		case 'H':
		default:
Draw:
			wcount = 0;
			for(j = 0; j < length; j++) {
				c = buf[j];
				c1 = HCU(c);
				putchar(c1);
				c1 = HCL(c);
				putchar(c1);
				wcount++;
				if(/* !rawflg && */ wcount % 2 == 0)
					putchar(' ');
				if(/* !rawflg && */ wcount % 32 == 0) {
					nlflg++;
					putchar('\n');
					wcount = 0;
				} else {
					nlflg = 0;
				}
			}
			break;
		case 'd':
		case 'D':
			/* if(rawflg)
				goto Draw; */
			n = 0;
			wcount = 0;
			for(j = 0; j < length; j++) {
				n = (n << 8) + buf[j];
				if(j % 4 == 3) {
					printf("%12d",n);
					wcount++;
					n = 0;
					if(wcount % 6 == 0) {
						nlflg++;
						putchar('\n');
						wcount = 0;
					} else {
						nlflg = 0;
					}
				}
			}
			if(j % 4)
				printf("%12d",n);
			break;
		}
		buf   += length;
		dsize -= length;
		if(/* !rawflg && */ !nlflg)
			putchar('\n');
	}
}

/*******************************************************************************
********
   NAME:     pr_symptomdata()
   FUNCTION: This function will print the symptom data message header followed b
y the
                symptom data.  The function setup_symptom() will be called in or
der
                to reformat the symptom data into the proper order for associati
on
                with codepoints and for passing to other applications.
********************************************************************************
*******/
static pr_symptomdata()
{
        struct sympt_data       *buf;
        struct sscodept         *output_buf;
        struct sscodept         *output_ptr;
        char                    *temp_ptr;
        char                    *end_ptr;

        /* Set buf pointer to symptom data */
        buf = (struct sympt_data *)T_errlog.el_symptom_data;

        /* Allocate memory to store the symptom data with its codepoints */
        output_buf = calloc ( 1,MAX_SSCODEPT_STR_SIZE );

        /* Validate that enough memory was available */
        if ( output_buf == NULL )
           cat_fatal(CAT_MALLOC_ERROR,"Unable to report symptom data, since \
there is not enough\nmemory at this time.\n");

        /* Reformat the symptom data into the proper order for   */
        /* association with codepoints -- output_buf will hold   */
        /* the reformated data and end_ptr will point to the end */
        /* of output_buf.                                        */
        setup_symptom(buf, output_buf, &end_ptr);

        /* Print symptom data header */
        printf("%s\n",msg_symptomdata);

        output_ptr = output_buf;

        /* Print the error's symptom codepoints along with its data */
        while ((char *)output_ptr < end_ptr)
           {
           printf("%s\n%s\n",codeptstr(output_ptr->codept,ERRSET_DETAILDATA),
                output_ptr->data);
           /* increment the pointer by adding the sizeof the codepoint */
           /* along with the length of the string data */
           temp_ptr = (char *)output_ptr;
           temp_ptr += strlen(output_ptr->data) +
			 sizeof(output_ptr->codept) + 1;
           output_ptr = (struct sscodept *)temp_ptr;
           }
        free(output_buf);
}

pr_tmpltentry()
{
	static recordno;

	if(!detailedflg) {
		if(recordno == 0)
			printf(msg_thdr);
		recordno++;
		printf(msg_tfmt,
		    T_errtmplt.et_crcid,
		    T_errtmplt.et_label,
		    T_errtmplt.et_type,
		    T_errtmplt.et_class,
		    descstr());
		return;
	}
	printf(msg_tahdr,
	    T_errtmplt.et_crcid,
	    T_errtmplt.et_label,
	    T_errtmplt.et_class,
	    T_errtmplt.et_type,
	    T_errtmplt.et_logflg    ? Yes : No,
	    T_errtmplt.et_reportflg ? Yes : No,
	    T_errtmplt.et_alertflg  ? Yes : No);
	pr_desc();
	pr_tmpltdetail();
}

static pr_tmpltdetail()
{
	int i;
	int encode,length,descid;

	for(i = 0; i < 8; i++) {
		encode = T_errtmplt.et_detail_encode[i];
		length = T_errtmplt.et_detail_length[i];
		descid = T_errtmplt.et_detail_descid[i];
		if(encode == 0)
			continue;
		if(i == 0)
			printf("%s\n",msg_detaildata);
		printf("%s\n",codeptstr(descid,ERRSET_DETAILDATA));
	}
}

/* Print resource class, type and location with this routine. */
pr_other()
{
	printf(msg_ohdr,
	    T_errlog.el_rclass,
	    T_errlog.el_rtype,
	    T_errlog.el_in);
} /* pr_other */

/* Print the CuVPD data with this routine. */
pr_vpd()
{
	/*
     Construct vpd data into printable form, space is allocated and
     pointer is returned in 'vpd_vtext'. Print 'VPD:' and data only if 
     T_errlog.el_vpd_ibm is not "NONE". T_errlog.el_vpd_user is the user 
     defined data. 
  */
	if (strcmp(T_errlog.el_vpd_ibm,"NONE") ) {
		printf(msg_vhdr);
		build_vpd(T_errlog.el_vpd_ibm,T_errlog.el_vpd_user,&vpd_vtext);
		printf(vpd_vtext);
	}

} /* pr_vpd */

/* Print the descriptions information with this routine. */
static pr_desc()
{

	prcatstr1(T_errtmplt.et_desc,      ERRSET_DESCRIPTION,msg_desc);
	prcatstr4(T_errtmplt.et_probcauses,ERRSET_PROBCAUSES,msg_probcauses);
	prcatstr4(T_errtmplt.et_usercauses,ERRSET_USERCAUSES,msg_usercauses);
	prcatstr3(T_errtmplt.et_useraction,ERRSET_RECACTIONS,msg_recactions);
	prcatstr4(T_errtmplt.et_instcauses,ERRSET_INSTCAUSES,msg_instcauses);
	prcatstr3(T_errtmplt.et_instaction,ERRSET_RECACTIONS,msg_recactions);
	prcatstr4(T_errtmplt.et_failcauses,ERRSET_FAILCAUSES,msg_failcauses);
	prcatstr3(T_errtmplt.et_failaction,ERRSET_RECACTIONS,msg_recactions);
}

static prcatstr1(alert,set,str)
char *str;
{

	if(alert == 0xFFFF)
		return;
	printf("%s\n%s\n",str,codeptstr(alert,set));
	printf("\n");
}

static prcatstr4(alerts,set,str)
unsigned short alerts[];
char *str;
{
	unsigned alert;
	int i;

	if(alerts[0] == 0xFFFF)
		return;
	printf("%s\n",str);
	for(i = 0; i < 4; i++) {
		alert = alerts[i];
		if(alert == 0xFFFF)
			break;
		printf("%s\n",codeptstr(alert,set));
	}
	printf("\n");
}

static prcatstr3(alerts,set,str)
unsigned short alerts[];
char *str;
{
	unsigned alert;
	int i;

	if(alerts[0] == 0xFFFF)
		return;
	printf("\t%s\n",str);
	for(i = 0; i < 4; i++) {
		alert = alerts[i];
		if(alert == 0xFFFF)
			break;
		printf("\t%s\n",codeptstr(alert,set));
	}
	printf("\n");
}

/*
 * print
 *    month day hour min sec
 * like
 *    Jan 12 13:34:12
 */
static char *timestr(timestamp)
{
	static char buf[22];
	strftime(buf,21,"%c",localtime((time_t *) &timestamp));
	/*  uses the time display defined by locale  */
	return(buf);
}

/*
 * print
 *    month day hour min year
 * like
 *    mmddhhmmyy
 */
static char *datestr(timestamp)
{
	static char buf[22];
	/* this is ugly because sccs tries to expand these
			   when they are contiguous. */
	strftime(buf,21,"%m%d%H\
%M\
%y",localtime((time_t *) &timestamp));
	return(buf);
}

static char *descstr()
{

	if (T_errtmplt.et_desc == 0xFFFF)
		return(codeptstr(T_errtmplt.et_detail_descid[0],ERRSET_DETAILDATA));
	else
		return(codeptstr(T_errtmplt.et_desc,ERRSET_DESCRIPTION));
}

struct r {
	char *r_name;
	char *r_entry;
	int   r_type;
};


/* This structure is used to hold the information for the symptom data   */
/* template, in order to generate template-like entries (like T_errtmplt */
/* entries) when symptom data exists. This is used because there are no  */
/* entries in T_errtmplt to hold symptom data.				 */
struct symptom_like_tmplt {
        unsigned short et_ttl_symptom_length;
        unsigned short et_symptom_encode[MAX_SS_CODEPTS];
        unsigned short et_symptom_desc_id[MAX_SS_CODEPTS];
        unsigned short et_symptom_length[MAX_SS_CODEPTS];
} symp_tmplt;

#define TY_STRING      1
#define TY_LONG        2
#define TY_SHORT       3
#define TY_BOOL        4
#define TY_DATA        5
#define TY_SHORT4      6
#define TY_SHORT8      7
#define TY_CHAR8       8
/* These are added especially for symptom data since we have to use a  */
/* different structure to get symptom data then the normal errlog data */
/* due to not having a template for the symptom data. 		       */
#define TY_SYMP_LONG   9
#define TY_SYMP_SHORT 10
#define TY_SYMP_CHAR  11
#define TY_SYMP_DATA  12

#define R_STRING(C,M)   { "M", (char *)C.M, TY_STRING }
#define R_LONG(C,M)     { "M", (char *)&C.M,TY_LONG   }
#define R_SHORT(C,M)    { "M", (char *)&C.M,TY_SHORT  }
#define R_BOOL(C,M)     { "M", (char *)&C.M,TY_BOOL   }
#define R_DATA(C,M)     { "M", (char *)C.M, TY_DATA   }
#define R_SHORT4(C,M)   { "M", (char *)C.M, TY_SHORT4 }
#define R_SHORT8(C,M)   { "M", (char *)C.M, TY_SHORT8 }
#define R_CHAR8(C,M)    { "M", (char *)C.M, TY_CHAR8  }
/* THE FOLLOWING FOUR #DEFINES SHOULD BE UNCOMMENTED WHEN FFDC IS IN PLACE */
/*
#define R_SYMP_LONG(C,M)  { "M", (char *)&C.M, TY_SYMP_LONG }
#define R_SYMP_SHORT(C,M) { "M", (char *)C.M, TY_SYMP_SHORT }
#define R_SYMP_CHAR(C,M)  { "M", (char *)C.M, TY_SYMP_CHAR  }
#define R_SYMP_DATA(C,M)  { "M", (char *)C.M, TY_SYMP_DATA  }
*/
static struct r r[] = {
	R_LONG(T_errlog,el_sequence),
	R_STRING(T_errlog,el_label),
	R_LONG(T_errlog,el_timestamp),
	R_LONG(T_errlog,el_crcid),
	R_STRING(T_errlog,el_machineid),
	R_STRING(T_errlog,el_nodeid),
	R_STRING(T_errlog,el_class),
	R_STRING(T_errlog,el_type),
	R_STRING(T_errlog,el_resource),
	R_STRING(T_errlog,el_rclass),
	R_STRING(T_errlog,el_rtype),
	R_STRING(T_errlog,el_vpd_ibm),
	R_STRING(T_errlog,el_vpd_user),
	R_STRING(T_errlog,el_in),
	R_STRING(T_errlog,el_connwhere),
	R_STRING(T_errtmplt,et_label),
	R_STRING(T_errtmplt,et_class),
	R_STRING(T_errtmplt,et_type),
	R_SHORT(T_errtmplt,et_desc),
	R_SHORT4(T_errtmplt,et_probcauses),
	R_SHORT4(T_errtmplt,et_usercauses),
	R_SHORT4(T_errtmplt,et_useraction),
	R_SHORT4(T_errtmplt,et_instcauses),
	R_SHORT4(T_errtmplt,et_instaction),
	R_SHORT4(T_errtmplt,et_failcauses),
	R_SHORT4(T_errtmplt,et_failaction),
	R_SHORT8(T_errtmplt,et_detail_length),
	R_SHORT8(T_errtmplt,et_detail_descid),
	R_CHAR8(T_errtmplt,et_detail_encode),
	/* THE FOLLOWING THREE LINES SHOULD BE UNCOMMENTED WHEN FFDC IS IN
	   PLACE */
	/*
	R_SYMP_SHORT(symp_tmplt,et_symptom_length),
	R_SYMP_SHORT(symp_tmplt,et_symptom_desc_id),
	R_SYMP_CHAR(symp_tmplt,et_symptom_encode), */
	R_BOOL(T_errtmplt,et_logflg),
	R_BOOL(T_errtmplt,et_alertflg),
	R_BOOL(T_errtmplt,et_reportflg),
	R_LONG(T_errlog,el_detail_length),
	R_DATA(T_errlog,el_detail_data),
	/* THE FOLLOWING TWO LINES SHOULD BE UNCOMMENTED WHEN FFDC IS IN
	   PLACE */
	/*
	R_SYMP_LONG(T_errlog,el_symptom_length),
	R_SYMP_DATA(T_errlog,el_symptom_data), */
	{
		0,0,0 			}
};

static raw_ascii()
{
	int i;
	int count;
	char *end_ptr;
	struct r *rp;
	struct sympt_data *output_ptr;
	short *wp;
	char *temp_ptr;
	struct sscodept *save_ptr;
	struct sscodept *output_buf;

        /* initialize template-like symptom data */
        memset(&symp_tmplt,NULL,sizeof(symp_tmplt));
        symp_tmplt.et_ttl_symptom_length = 0;

        /* Validate that symptom data exists - if not set the buffer that would
        /* have contained the symptom data to NULL */
        if (T_errlog.el_symptom_length != 0 )
           {
           /* Set pointer to symptom data */
           output_ptr = (struct sympt_data *)T_errlog.el_symptom_data;

           /* Allocate memory to store the symptom data with its codepoints */
           output_buf = calloc ( 1,MAX_SSCODEPT_STR_SIZE );

           /* Validate that enough memory was available */
           if ( output_buf == NULL )
              cat_warn(CAT_MALLOC_ERROR,"Unable to report symptom data, \
since there is not enough\nmemory at this time.");
           else
              {
              /* Reformat the symptom data into the proper order for   */
              /* association with codepoints -- output_buf will hold   */
              /* the reformated data and end_ptr will point to the end */
              /* of output_buf.                                             */
              raw_symptom(output_ptr, output_buf, &end_ptr);

              /* Save pointer since the original pointer address is lost */
              /* when we print the symptom data */
              save_ptr = output_buf;
              }
           }
        else
           output_buf = NULL;


	for(rp = r; rp->r_name; rp++) {
		printf("%-20s ",rp->r_name);
		switch(rp->r_type) {
		case TY_STRING:
			printf("%s\n",(char *)rp->r_entry);
			break;
		case TY_LONG:
			printf("0x%08x\n",*(int *)rp->r_entry);
			break;
                case TY_SYMP_LONG:
                        printf("0x%08x\n",symp_tmplt.et_ttl_symptom_length);
                        break;
		case TY_SHORT:
			printf("0x%04x\n",*(short *)rp->r_entry & 0xFFFF);
			break;
		case TY_BOOL:
			printf("%s\n",*(short *)rp->r_entry ? "TRUE" : "FALSE");
			break;
		case TY_DATA:
			for(i = 0; i < T_errlog.el_detail_length; i++)
				printf("%02X",T_errlog.el_detail_data[i]);
			printf("\n");
			break;
                case TY_SYMP_DATA:
                        /* Check to see if symptom data exists, if not leave bla
nk */
                        if ( output_buf != NULL )
                           {
                           /* Loop through symptom data until the end */
                           while ((char *)output_buf < end_ptr)
                              {
                              temp_ptr = output_buf->data;
                              while (*temp_ptr != NULL)
                                 {
                                 /* print one character at a time */
                                 printf("%2X", temp_ptr[0]);
                                 temp_ptr++;
                                 }
                              /* increment the pointer by adding the sizeof */
			      /* the codepoint along with the length of the */
			      /* string data + 1 for the NULL string term-  */
			      /* inator.				    */
			      temp_ptr = (char *)output_buf;
                              temp_ptr += strlen(output_buf->data) +
					  sizeof(output_buf->codept) + 1;
			      output_buf = (struct sscodept *)temp_ptr;
                              }
                           }
                        printf("\n");
                        break;
		case TY_SHORT4:
		case TY_SHORT8:
			count = rp->r_type == TY_SHORT4 ? 4 : 8;
			for(i = 0, wp = (short *)rp->r_entry; i < count; i++,wp++)
				printf("%s0x%04x",i == 0 ? "" : ",",*wp & 0xFFFF);
			printf("\n");
			break;
                case TY_SYMP_SHORT:
                        wp = (short *)rp->r_entry;
                        i = 0;

                        /* loop through the symptom data template and print */
			/* its corresponding value */
                        while ( *wp != NULL )
                        {
                            printf("%s0x%04x",i == 0 ? "" : ",",*wp & 0xFFFF);
                            i++;
                            wp++;
                        }

                        /* No symptom data exists */
                        if (i == 0)
                           printf("0x0000");

                        printf("\n");
                        break;
		case TY_CHAR8:
			for(i = 0, wp = (short *)rp->r_entry; i < 8; i++,wp++)
				printf("%s%c",i == 0 ? "" : ",",*wp ? *wp : 'X');
			printf("\n");
			break;
                case TY_SYMP_CHAR:
                        wp = (short *)rp->r_entry;
                        i = 0;

                        /* loop through the symptom data template and */
			/* print its corresponding value */
                        while ( *wp != NULL )
                        {
                           printf("%s%c",i == 0 ? "" : ",",*wp);
                           i++;
                           wp++;
                        }
                        /* No symptom data exists */
                        if (i == 0)
                           printf("X");

                        printf("\n");
                        break;
		}
	}
	printf("@@\n");

        /* Free the memory up if a malloc was done */
        if ( T_errlog.el_symptom_length != 0 )
           free(save_ptr);
}

/*******************************************************************************
********
   NAME:     raw_symptom()
   FUNCTION: This function will setup a symptom data template for the length,
                description, and encoding.
********************************************************************************
*******/
static raw_symptom(struct sympt_data *output_ptr, struct sscodept *output_buf, char **end_ptr)
{
        unsigned short             ndx=0;
        char *                     temp_ptr;

        setup_symptom(output_ptr, output_buf, end_ptr);

        while ((char *)output_buf < *end_ptr)
           {
           symp_tmplt.et_symptom_encode[ndx] = 'A';
           symp_tmplt.et_symptom_desc_id[ndx] = output_buf->codept;
           symp_tmplt.et_symptom_length[ndx] = strlen(output_buf->data);
           temp_ptr = (char *)output_buf;
	   /* Move the pointer the length of the symptom string + */
	   /* the size of the codepoint i.e. integer + 1 for NULL */
           temp_ptr += symp_tmplt.et_symptom_length[ndx] +
			 sizeof(output_buf->codept) + 1;
           output_buf = (struct sscodept *)temp_ptr;
           symp_tmplt.et_ttl_symptom_length +=
                 symp_tmplt.et_symptom_length[ndx];
           ndx++;
           }
}

char *hextoascii(c)
{
	static char buf[4];

	buf[0] = HCU(c);
	buf[1] = HCL(c);
	buf[2] = '\0';
	return(buf);
}


/*
  Procedure: pr_raw()
  Entry: T_errtmplt and T_errobj contain valid structure entries.
  Exit: If standard output is not a terminal the binary contents of
        T_errtmplt and T_errobj are written to standard output in one
        call to writevx.  Otherwise, pr_raw() exits without error.
        Generally, standard out will be a pipe, but it may be a
        standard non-tty file.
*/

/* scatter-write vector */
/* Commenting this out because we are removing the -r function.
static struct iovec wrtv[] = {
	{&T_errlog,sizeof(struct obj_errlog)},
	{&T_errtmplt,sizeof(struct obj_errtmplt)}
};

static pr_raw()
{

	if (!isatty(1))
		writevx(1,wrtv,2,0);  
	return(0);

}
*/
