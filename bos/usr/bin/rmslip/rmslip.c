static char sccsid[] = "@(#)68  1.3  src/bos/usr/bin/rmslip/rmslip.c, rcs, bos411, 9428A410j 11/21/93 15:22:55";
/*
 *   COMPONENT_NAME: RCS
 *
 *   FUNCTIONS: main
 *		usage
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
/*              include file for message texts          */
#include <rmslip_msg.h>
#include <locale.h>
#include <nl_types.h>

#define  ODMDIR   "/etc/objrepos"
#define  NO_ODM_LOCK  5
#define  FAIL  255
#define  NAME_STR_LEN  21
#define  DELE_STR_LEN 128
extern int	optind;
extern char	*optarg;
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/*****************************xx*/
/* prototypes			*/
/**************xx****************/
int main(int argc, char *argv[]);
void usage(void);

/***************xx*****************/
/* usage function                 */
/****************xx****************/
void usage(void)
{
	fprintf(stderr, catgets(scmc_catd, MS_rmslip, 
		M_MSG_1, "Usage: rmslip -n "
		"Connection_Name | rmslip  -a \n") );
	catclose(scmc_catd);
	exit(FAIL);
}

/********************************/
/* main function		*/
/********************************/ 
main(int argc, char *argv[])
{
	int c; 					/* getopt usage 	*/
	int lock_id;  				/* odm lock descriptor	*/
	int return_code;			/* odm return code  	*/
	int number_deleted;			/* odm returns the 	*/
						/* number of rm'd obj's	*/
	int return_status;			/* odm error status 	*/
	int delete_one_flag = 0; 		/* only deleting 1 slip */
	int delete_all_flag = 0; 		/* deleting all slip	*/
	struct slips rm_slip; 		/* pointer to a slip 	*/
						/* struct 		*/

	char   *error_msg; 			/* odm error message	*/

	char   name_str[NAME_STR_LEN]; 		/* name of the slip 	*/

	char   obj_to_delete[DELE_STR_LEN]; 	/* the delete string 	*/
						/* sent to the odm 	*/
						/* database manager	*/

	/* null the arrays 	*/
	bzero(name_str,NAME_STR_LEN);
	bzero(obj_to_delete,DELE_STR_LEN);

	(void) setlocale(LC_ALL,"");

	 scmc_catd = catopen("rmslip.cat",NL_CAT_LOCALE);

	/* usage if we don't get any args with this program */
	if (argc < 2) 
	{ 
		fprintf(stderr, catgets(scmc_catd, MS_rmslip, M_MSG_2, 
			"This program needs at least one argument.\n") );
		usage();
	}
	
	/* set up the odm database manager environ */
    odm_initialize(); 
	/* set up the odm path (diskless stations) */
	odm_set_path(ODMDIR);
 
	/* parse the command line using getopt */
	while ((c = getopt(argc,argv,"an:")) != EOF)
		switch (c) 
		{
			/* got a name so copy it into the name string	*/
			/* also set the delete one flag			*/
			case 'n':
				delete_one_flag++;
        			strncpy(name_str,optarg,NAME_STR_LEN-1);
			break;

			/* got the -a (delete all option	*/
			/* so set the delete all flag		*/
			case 'a':
        			delete_all_flag++;
			break;

			default:
				usage();
		}


	/* here we didn't get either flag */
	if (!delete_one_flag && !delete_all_flag)
		usage();

	/* can't delete one and all at the same time	*/
	/* getopt should handle not getting either flag	*/
	if (delete_one_flag && delete_all_flag) 
		usage();


	if (delete_one_flag) 
	{
		/* -n '' will not cause an error so check it here */
		if (name_str[0] == NULL ) 
		{ 
			usage();
   		}
		else 
		{
			/* build the string to delete the slip */
			sprintf(obj_to_delete,"ConnName = %s",name_str); 
   		}

	 	if ((lock_id = odm_lock(ODMDIR,ODM_NOWAIT)) < 0 ) 
		{
			fprintf(stderr, catgets(scmc_catd, MS_rmslip, 
				M_MSG_4, "Unable to obtain a lock "
					"on the odm database.\n"));
			catclose(scmc_catd);
		        exit(NO_ODM_LOCK);
		}
		number_deleted = odm_rm_obj(slips_CLASS,obj_to_delete); 

		/* number_deleted is the number of objects removed */
		if (number_deleted > 0) 
			return_code = 0;
		else
			return_code = FAIL;


		if (return_code == -1)
		{
                        return_status = odm_err_msg(odmerrno, &error_msg);

                	if (return_status < 0)
                       		 fprintf(stderr, catgets(scmc_catd, 
					MS_rmslip, M_MSG_5, 
					"Retrieval of error message "
						"failed.\n"));
                	else
                       		 fprintf(stderr,error_msg);

			catclose(scmc_catd);
			exit(FAIL);
		}

		odm_unlock(lock_id);
		catclose(scmc_catd);
		exit(return_code);
   	}

	if (delete_all_flag) 
	{
		/* if set to null it gets all slips  	*/
		/* and obj_to_delete[0] == 0		*/ 

	 	if ((lock_id = odm_lock(ODMDIR,ODM_NOWAIT)) < 0 ) 
		{
			fprintf(stderr, catgets(scmc_catd, MS_rmslip, M_MSG_4, 
			"Unable to obtain a lock on the odm database.\n") );
			catclose(scmc_catd);
		        exit(NO_ODM_LOCK);
		}
		number_deleted = odm_rm_obj(slips_CLASS,obj_to_delete); 

		/* number_deleted is the number of objects removed */
		if (number_deleted > 0) 
			return_code = 0;
		else
			return_code = FAIL;

		if (return_code == -1)
		{
                        return_status = odm_err_msg(odmerrno, &error_msg);

                	if (return_status < 0)
                       		 fprintf(stderr, catgets(scmc_catd, 
					MS_rmslip, M_MSG_5, 
					"Retrieval of error message "
						"failed.\n"));
                	else
                       		 fprintf(stderr,error_msg);

			catclose(scmc_catd);
			exit(FAIL);
		}

		odm_unlock(lock_id);
		catclose(scmc_catd);
		exit(return_code);
	}
}
