static char sccsid[] = "@(#)67  1.5  src/bos/usr/bin/mkslip/mkslip.c, rcs, bos411, 9428A410j 11/21/93 15:22:52";
/*
 *   COMPONENT_NAME: RCS
 *
 *   FUNCTIONS: valid_baud
 *		main
 *		usage
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
 */



#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <slips.h>
#include <mkslip_msg.h>
/*              include file for message texts          */
#include <locale.h>
#include <nl_types.h>

nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#define SUCCESS 0
#define FAIL 255
#define	 NAME_LEN	21
#define	 DESC_LEN	41 	
#define	 LOCAL_LEN	257
#define	 REMOTE_LEN	257
#define	 DIALED_LEN	257
#define	 MASK_LEN	16
#define	 TTY_LEN	7 
#define	 DIALSTR_LEN	257
#define	 SRCHSTR_LEN	513
#define	 ODMDIR	"/etc/objrepos"

/*
**	prototypes
*/


/****************************************************************************/
int main(int argc, char *argv[]);
void usage(void);
int valid_baud(int);
void colon_check(char *);

/*
** declare external variables for getopt
*/ 
extern int	optind;
extern char	*optarg;

/****************************************************************************/
void usage(void)
{
	fprintf(stderr, catgets(scmc_catd, MS_mkslip, M_MSG_1, 
		"Usage: mkslip -n Connection_Name  \n"
		"\t[ -c Connection Description ] \n"
		"\t  -r Remote Host Name (or dotted decimal)  \n"
		"\t  -l Internet Address of Local Host  \n"
		"\t  -d Internet Address of Dialed Host  \n"
		"\t[ -m Network Mask (hexadecimal or dotted decimal] \n"
		"\t  -p TTY Port Used For Slip Connection  \n"
		"\t  -b Baud Rate  \n"
		"\t  -s Dial String  \n"
		"\t  -t Timeout\n") );
	catclose(scmc_catd);
	exit(FAIL);
}

int main(int argc, char *argv[])
{
	int	c;
	int	lock_id;
	int	add_rtn;
	int	return_status;
	int	n_flag = 0;
	int	c_flag = 0;
	int	r_flag = 0;
	int	l_flag = 0;
	int	d_flag = 0;
	int	m_flag = 0;
	int	p_flag = 0;
	int	b_flag = 0;
	int	s_flag = 0;
	int	t_flag = 0;

	struct	slips *slip_ptr;
	struct	slips myslip;
	struct	slips mk_slip;

	int	baud = 0;
	int	timeout = 0;
	char	*error_message;

	/* pointers used to manuiplate the dial string */
	/* when it contains a colon or colons          */
	char	*p,*q; 

	char	name_str[NAME_LEN];
	char	desc_str[DESC_LEN];
	char	remote_str[REMOTE_LEN];
	char	local_str[LOCAL_LEN];
	char	dialed_str[DIALED_LEN];
	char	mask_str[MASK_LEN];
	char	tty_str[TTY_LEN];
	char	dial_str[DIALSTR_LEN];
	char	dial_buf[DIALSTR_LEN];
	char	srchstr[SRCHSTR_LEN];

/****************************************/
/* put null bytes in all character 	*/
/* arrays to avoid unwanted data 	*/
/****************************************/
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

	 scmc_catd = catopen("mkslip.cat",NL_CAT_LOCALE);

/****************************************/
/* call the odm_initialize routine 	*/
/* to establish odm settings		*/
/* I must also all setpath below 	*/
/****************************************/
	odm_initialize();

	odm_set_path(ODMDIR);

	if ( argc < 2 ) 
	{
		fprintf(stderr, catgets(scmc_catd, MS_mkslip, M_MSG_2, 
			"This program needs at least 1 argument.\n") );
		usage();
	}

/****************************************/
/* parse the command line and set 	*/
/* flags as we get each option		*/
/****************************************/
	while ((c = getopt(argc,argv,"n:c:r:l:d:m:p:b:s:t:")) != EOF)
		switch (c) 
		{
			case 'n':
				colon_check(optarg);
				n_flag++;
				strncpy(name_str,optarg,NAME_LEN-1);
			break;

			case 'c':
				colon_check(optarg);
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
				if ((valid_baud(baud)) != 0) 
				{
					fprintf(stderr, catgets(scmc_catd, 
					MS_mkslip, M_MSG_3, "Invalid baud "
							"rate %d.\n") ,baud);
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
					MS_mkslip, M_MSG_8, "A Timeout of less"
							" than 30 seconds "
							"is not allowed.\n") );
					usage();
				}
			break;

			default:
				usage();
		}

	if (!n_flag || !r_flag || !l_flag || !d_flag 
		|| !p_flag || !b_flag || !s_flag || !t_flag ) 
	{
		usage();
	}

	if (name_str[0] == NULL) 
		usage();
	strcpy(mk_slip.ConnName,name_str);

	if (desc_str[0] != NULL) 
	strcpy(mk_slip.ConnDesc,desc_str);

	if (remote_str[0] == NULL) 
		usage();
	strcpy(mk_slip.RemoteHost,remote_str);

	if (local_str[0] == NULL) 
		usage();
	strcpy(mk_slip.LocalHost,local_str);

	if (dialed_str[0] == NULL) 
		usage();
	strcpy(mk_slip.DialedHost,dialed_str);

	if (mask_str[0] != NULL)
	strcpy(mk_slip.NetMask,mask_str);

	if (tty_str[0] == NULL) 
		usage();
	strcpy(mk_slip.TTY_Port,tty_str);

	mk_slip.BaudRate = baud;  

	if (dial_str[0] == NULL)
		usage();

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
		strcpy(mk_slip.DialString,dial_buf);
	}
	else
	{
		strcpy(mk_slip.DialString,dial_str);
	}

	mk_slip.Timeout = timeout;  

	sprintf(srchstr,"ConnName=%s",name_str);

 	if ((lock_id = odm_lock(ODMDIR,ODM_NOWAIT)) == -1 ) 
	{
		return_status = odm_err_msg(odmerrno,&error_message);
		
		if (return_status < 0)
			fprintf(stderr, catgets(scmc_catd, MS_mkslip, 
			M_MSG_5, "Retrieval of error message failed.\n") );
		else
			fprintf(stderr,error_message);

		catclose(scmc_catd);
		exit(FAIL);
	}

	slip_ptr = odm_get_obj(slips_CLASS,srchstr,&myslip,TRUE);

	if (slip_ptr == -1)
	{
		return_status = odm_err_msg(odmerrno,&error_message);
		
		if (return_status < 0)
			fprintf(stderr, catgets(scmc_catd, MS_mkslip, 
			M_MSG_5, "Retrieval of error message failed.\n") );
		else
			fprintf(stderr,error_message);

		catclose(scmc_catd);
		exit(FAIL);
	}

	if (slip_ptr != NULL)
	{
		fprintf(stderr,catgets(scmc_catd, MS_mkslip, M_MSG_7,
			"\nThis is a duplicate slip name.\n"
			"Please select another name.\n\n") );
			usage();
	}

	add_rtn = odm_add_obj(slips_CLASS,&mk_slip); 

	/* check the return code from the odm_add for failure */
	if (add_rtn == -1)
	{
		return_status = odm_err_msg(odmerrno,&error_message);
		
		if (return_status < 0)
			fprintf(stderr, catgets(scmc_catd, MS_mkslip, 
			M_MSG_5, "Retrieval of error message failed.\n") );
		else
			fprintf(stderr,error_message);

		catclose(scmc_catd);
		exit(FAIL);
	}
	odm_unlock(lock_id);
catclose(scmc_catd);
exit(SUCCESS);
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
		fprintf(stderr, catgets(scmc_catd,MS_mkslip, M_MSG_9, 
			"Error adding %s;\nCharacter : is invalid.\n"),
			optarg_string);
		exit(FAIL);
	}
}
