static char sccsid[] = "@(#)65  1.5  src/bos/usr/bin/chslip/chslip.c, rcs, bos411, 9428A410j 11/21/93 15:22:48";
/**********************************************************************
 *   COMPONENT_NAME: RCS
 *
 *   FUNCTIONS: main
 *		usage
 *		valid_baud
 *		colon_check
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
 **********************************************************************/

#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <slips.h>
#include <locale.h>
#include <nl_types.h>
/*              include file for message texts          */
#include <chslip_msg.h> 

#define ODMDIR  "/etc/objrepos"
#define SUCCESS 0
#define FAIL 255
#define  NAME_LEN       21
#define  DESC_LEN       41
#define  LOCAL_LEN      257
#define  REMOTE_LEN     257
#define  DIALED_LEN     257
#define  MASK_LEN       16
#define  TTY_LEN        7
#define  DIALSTR_LEN    257
#define  SRCHSTR_LEN    513


extern int	optind;
extern char	*optarg;

nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/********************************************************/
/* prototypes for functions				*/
/********************************************************/
int main(int argc, char *argv[]);
void usage(void);
int valid_baud(int);
void colon_check(char *);

/********************************************************/
void usage(void)
{
	
	fprintf(stderr, catgets(scmc_catd, MS_chslip, M_MSG_1, 
		"Usage:    chslip  -n Connection_Name\n\t\t"
		"[ -c Connection Description ] \n\t\t"
		"  -r Remote Host Names (or dotted decimal addresses) \n\t\t"
		"  -l Internet Address of Local Host \n\t\t"
		"  -d Internet Address of Dialed Host \n\t\t"
		"[ -m Network Mask (hexadecimal or dotted decimal] \n\t\t"
		"  -p TTY Port Used For Slip Connection \n\t\t"
		"  -b Baud Rate \n\t\t"
		"  -s Dial String \n\t\t"
		"  -t Timeout\n") );
		catclose(scmc_catd);
		exit(FAIL);
}

/********************************************************/
int main(int argc, char *argv[])
{
	char *error_msg;
	char *p, *q; 		/* used for processing a dial string	*/
				/* containing embedded colons		*/
	int c; 			/* used for command line processing	*/
	int return_status;	/* status return from odm_err_msg	*/
	int update_return;	/* return from odm_change, if change  	*/
	                        /* can't be made to database it = -1   	*/
	int lock_id; 		/* lock id returned by odm		*/
	int n_flag = 0;     /* flags to let me know if we have these	*/
	int c_flag = 0;     /* command line options sent to this pgm	*/
	int r_flag = 0;     /* n = name, c = connection descr, r = remote */
	int l_flag = 0;     /* host name, l = local host name, d = dialed host*/
	int d_flag = 0;     /* m = netmask, p = tty port name, b = baud rate */
	int m_flag = 0;     /* s = dial string */
	int p_flag = 0;
	int b_flag = 0;
	int s_flag = 0;
	int t_flag = 0;

	struct slips *slip_ptr;  /* declare a slip pointer */
	struct slips myslip; /* declare a struct of slip characteristics */

	int    baud = 0;
	int    timeout = 0;

	char   name_str[NAME_LEN]; /* arrays to hold strings from command */
	char   desc_str[DESC_LEN]; /* line-names are descriptive and are  */
	char   remote_str[REMOTE_LEN];  /* like the flags above */
	char   local_str[LOCAL_LEN];
	char   dialed_str[DIALED_LEN];
	char   mask_str[MASK_LEN];
	char   tty_str[TTY_LEN];
	char   dial_str[DIALSTR_LEN];
	char   dial_buf[DIALSTR_LEN];
	char   srchstr[SRCHSTR_LEN];

	/* make sure all arrays contain nulls */
	bzero(name_str,NAME_LEN); 
	bzero(desc_str,DESC_LEN);
	bzero(remote_str,REMOTE_LEN);  
	bzero(local_str,LOCAL_LEN);
	bzero(dialed_str,DIALED_LEN);
	bzero(mask_str,MASK_LEN);
	bzero(tty_str,TTY_LEN);
	bzero(dial_str,DIALSTR_LEN);
	bzero(dial_buf,DIALSTR_LEN);
	bzero(srchstr,SRCHSTR_LEN);

	(void) setlocale(LC_ALL,"");

	 scmc_catd = catopen("chslip.cat",NL_CAT_LOCALE);
	
	/* not even close to enough args */
	if (argc < 2 )
	{
		usage();
	}
 
	odm_initialize();

	odm_set_path(ODMDIR);

	/* parse the command line and get options */
	while ((c = getopt(argc,argv,"n:c:r:l:d:m:p:b:s:t:")) != EOF)
		switch (c) 
		{
			case 'n':
		   	/* set flag if we get the option  */
			/* copy the option into the array */	
			/* do this for each case below    */
				colon_check(optarg);
				n_flag++;
        			strncpy(name_str,optarg,NAME_LEN-1);
			break;
			case 'c':
				colon_check(optarg);
				c_flag++;
        			strncpy(desc_str,optarg,DESC_LEN-1);
			break;

			case 'r':
				colon_check(optarg);
				r_flag++;
        			strncpy(remote_str,optarg,REMOTE_LEN-1);
			break;

			case 'l':
				colon_check(optarg);
				l_flag++;
        			strncpy(local_str,optarg,LOCAL_LEN-1);
			break;

			case 'd':
				colon_check(optarg);
				d_flag++;
        			strncpy(dialed_str,optarg,DIALED_LEN-1);
			break;

			case 'm':
				colon_check(optarg);
				m_flag++;
        			strncpy(mask_str,optarg,MASK_LEN-1);
			break;

			case 'p':
				colon_check(optarg);
				p_flag++;
        			strncpy(tty_str,optarg,TTY_LEN-1);
			break;

			case 'b':
				colon_check(optarg);
				b_flag++;
        			baud=atoi(optarg);
				if (baud == 0)
				{
					fprintf(stderr, catgets(scmc_catd, 
						MS_chslip, M_MSG_3,
						"A baud rate of 0 is "
						"not allowed.\n") );
						usage();
				}
				if (valid_baud(baud) != 0) 
				{
					fprintf(stderr, catgets(scmc_catd, 
						MS_chslip, M_MSG_2,
						"Invalid baud rate.\n") );
						usage();
				}
			break;

			case 's':
				s_flag++;
        			strncpy(dial_str,optarg,DIALSTR_LEN-1);
			break;

			case 't':
				colon_check(optarg);
				t_flag++;
        			timeout=atoi(optarg);
				if (timeout < 30)
				{
					fprintf(stderr, catgets(scmc_catd, 
						MS_chslip, M_MSG_8,
						"A timeout of less than "
						"30 seconds is"
						" not allowed.\n") );
						usage();
				}
			break;

			default:  /* unknown option - give usage */
				usage();
		}

	/* check to see if we have minimal 	*/
	/* options and do we have something 	*/
	/* to change - no sense in going on if	*/
	/* we aren't going to do something 	*/
	/* description & netmask are optional	*/

	if (!n_flag || !l_flag || !d_flag || !r_flag 
		|| !p_flag || !b_flag || !s_flag || !t_flag) 
	{
		usage();
	}

	/* were not changing all the SLIPS - just one */
	/* so establish a search string for the connection name */

 	if ((lock_id = odm_lock(ODMDIR,ODM_NOWAIT)) == -1 ) 
	{
		return_status = odm_err_msg(odmerrno, 
			&error_msg);

		if (return_status < 0)
			fprintf(stderr, catgets(scmc_catd, 
				MS_chslip, M_MSG_4,
		 		"Retrieval of error message failed.\n"));
		else
			fprintf(stderr,error_msg);

		catclose(scmc_catd);
		exit(FAIL);
	}

	sprintf(srchstr,"ConnName=%s",name_str);

	/* look up the desired SLIP */
	slip_ptr = odm_get_obj(slips_CLASS,srchstr,&myslip,TRUE); 

	if (slip_ptr == NULL)
	{
		fprintf(stderr, catgets(scmc_catd, 
			MS_chslip, M_MSG_5,
	 		"SLIP not found.\n") );
		catclose(scmc_catd);
		exit(FAIL);
	}

	if (slip_ptr == -1)
	{
		return_status = odm_err_msg(odmerrno, 
			&error_msg);

		if (return_status < 0)
		{
			fprintf(stderr, catgets(scmc_catd, MS_chslip, M_MSG_4,
			 	"Retrieval of error message failed.\n") );
			catclose(scmc_catd);
			exit(FAIL);
		}
		else
			fprintf(stderr,error_msg);

	}
		/* copy to the record only if we have an argument */
		/* for each of the following elements             */
		/* we are not allowed to change the slip name     */
		/* use rmslip then mkslip if you want a new name  */

	if (desc_str != NULL)
		strcpy(slip_ptr->ConnDesc,desc_str);

	if (r_flag) 
	{
		if (remote_str != NULL)
			strcpy(slip_ptr->RemoteHost,remote_str);
 	}

	if (l_flag) 
	{
		if (local_str != NULL)
			strcpy(slip_ptr->LocalHost,local_str);
	}

	if (d_flag) 
	{
		if (dialed_str != NULL)
			strcpy(slip_ptr->DialedHost,dialed_str);
	}

	if (m_flag) 
	{
		if (mask_str != NULL)
			strcpy(slip_ptr->NetMask,mask_str);
	}

	if (p_flag) 
	{
		if (tty_str != NULL)
			strcpy(slip_ptr->TTY_Port,tty_str);
	}

	if (b_flag) 
	{
		if (baud != 0)
			slip_ptr->BaudRate = baud;
	}

	if (s_flag) 
	{
		if (dial_str != NULL) 
		{
	        	if (strchr(dial_str,':') != NULL)
        		{
                		p = dial_str;
                		q = dial_buf;
                		while(*p != '\0')
                		{
                        		if (*p == ':')
                        		{
                                		*q++ = '#';
                                		*q++ = '!';
                        		}
                        		*q++ = *p++;
                		}
				strcpy(slip_ptr->DialString,dial_buf);
        		}
        		else
        		{
				strcpy(slip_ptr->DialString,dial_str);
			}
		}
	}

	if (t_flag) 
	{
		if (timeout != 0)
			slip_ptr->Timeout = timeout;
	}

	update_return = odm_change_obj(slips_CLASS, slip_ptr); 
	
	/* if update_return error is = -1 then change can't be made */
	if (update_return == -1) 
	{
		return_status = odm_err_msg(odmerrno, 
			&error_msg);

		if (return_status < 0)
			fprintf(stderr, catgets(scmc_catd, 
				MS_chslip, M_MSG_4,
				"Retrieval of error message failed.\n") );
		else
			fprintf(stderr,error_msg);

	}

	/* release the odm database */
	odm_unlock(lock_id);

	catclose(scmc_catd);

exit(update_return);

}

/****************************************/
/* listed below are all the valid baud 	*/
/* rates recognized by AIX, baud must 	*/
/* at least one of these otherwise we 	*/
/* have an invalid baud rate specified 	*/
/****************************************/

int valid_baud(int n)
{
	int rtn_val;

		switch (n) 
		{
			case     50 :
			case     75 :
			case    110 :
			case    134 :
			case    150 :
			case    200 :
			case    300 :
			case    600 :
			case   1200 :
			case   2400 :
			case   3600 :
			case   4800 :
			case   7200 :
			case   9600 :
			case  19200 :
			case  38400 :
			case  57600 :
				rtn_val = SUCCESS;
			break;

			default:
				rtn_val = FAIL;
			break;
		}
/****************************************/
/* any acceptable baud rate will 	*/
/* cause a SUCCESS or no error return 	*/
/*  otherwise a FAIL  condition		*/
/****************************************/
	return(rtn_val);
}

/****************************************/
/* this function checks for an imbedded	*/
/* colon in the optarg or optarg string	*/
/****************************************/
void colon_check(char *optarg_string)
{
	if (strchr(optarg_string,':') != NULL)
	{
		fprintf(stderr, catgets(scmc_catd,MS_chslip, M_MSG_9, 
			"Error changing %s;\nCharacter : is invalid.\n"),
			optarg_string);
		exit(FAIL);
	}
}

