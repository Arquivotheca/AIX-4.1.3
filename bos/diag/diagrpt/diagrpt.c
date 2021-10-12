static char sccsid[] = "@(#)94	1.4  src/bos/diag/diagrpt/diagrpt.c, cmddiag, bos411, 9428A410j 12/10/92 16:37:58";
/*
 *   COMPONENT_NAME: DUTIL
 *
 *   FUNCTIONS: disp_menu
 *		file_sort
 *		int_hand
 *		main
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>    
#include <sys/stat.h> 
#include <errno.h>    
#include <time.h>    
#include <asl.h>
#include <nl_types.h>
#include <locale.h>
#include <signal.h>
#include <fcntl.h>
#include "diag/diag.h"
#include "diag/diago.h"
#include "diagrpt_msg.h"

nl_catd catfd;

/*
 * NAME: main
 *                                                                    
 * FUNCTION: Display an error report file from previous diagnostic run.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * NOTES: A start date can be entered as a flag:
 *		-s startdate - (MMddyy)
 *		               Report diagnostic conclusions made after the
 *				date specified.
 *		-o This will cat the most recent data file
 *
 * RETURNS:  NONE
 */  

#define MAX_FILE_NUMBER 25
#define ERROR_FILE_NAME "diagrpt"
#define ERROR_FILE_EXT  "dat"
#define MENUNUMBER	0x802002

typedef struct {
		char	filename[256];
		struct	stat	buf;	/* holds file status info*/
} file_status_t, *file_status_ptr_t;

file_status_ptr_t  *file_sort();
void int_hand(int);
void disp_menu(int,file_status_ptr_t * );

extern int errno;
extern char *optarg;


int
main(argc, argv)
int	argc;
char	*argv[];
{

	int 	number_of_files;
	int	rc;
	int	i;		/* used as file name id			*/
	int	j;		/* used as index			*/
	int	c;
	char	command[256];
	char	filename[256];	/* holds error report filename		*/
	char	*ptr;
	char	*datadir; 	/* points to default data path   	*/
	file_status_t		file_info[MAX_FILE_NUMBER];
	file_status_ptr_t	*file_ptr;	
	static struct	tm	report_date;
	struct	tm		*file_date;
	long date1;
	long date2;
	int last_flag=-1;   /* flag to cat last file */


	

	/* if input argument is given - get starting date desired */
	if ( argc != 1 )
		while ((c = getopt(argc,argv,"os:")) != EOF ) {
			switch(c) {
				case 's' : 
					NLtmtime(optarg,"%2m%2d%2y", 
						 &report_date);
					break;
				case 'o' : /* cat last file */
					last_flag = 1;
					break;
				default :
					printf("usage: diagrpt [-s mmddyy]\n");
					exit(1);
			}
		}

	/* get directory path of error report files */
	if((datadir = (char *)getenv("DIAGDATADIR")) == NULL )
		datadir = DIAGDATA;

	/* The maximum number of data files is 25.                    	*/
	for ( j=0,i=1; i <= MAX_FILE_NUMBER; i++,j++ ) {
		sprintf(file_info[j].filename, "%s/%s%d.%s", datadir, 
				ERROR_FILE_NAME,i,ERROR_FILE_EXT);
		if((rc = stat(file_info[j].filename,&file_info[j].buf))!=0) {
			if (errno == ENOENT)	/* if file does not exist */
				break;		/*   break out            */
			return(-1);
		}
	}

	signal(SIGQUIT,int_hand);
	signal(SIGKILL,int_hand);
	signal(SIGINT,int_hand);
	signal(SIGTERM,int_hand); 

	number_of_files = i - 1;
	if ( number_of_files == 0 )
		return(0);
	file_ptr = file_sort(file_info, number_of_files);

	/*
	 * if option 'o', then cat out last file
	 */
	if(last_flag == 1)
	{
		sprintf(filename,"cat %s",&file_ptr[0]->filename);
		system(filename);
		exit(0);
	}

	if ( report_date.tm_year ) {

		date1 = (report_date.tm_year * 10000) + (report_date.tm_mon * 100)  \
				+ (report_date.tm_mday);

		/* 
		 * get date of files in directory 
		 */
		for ( i=0; i < number_of_files; i++ ) {
			file_date = localtime(&file_ptr[i]->buf.st_mtime);

			date2 = (file_date->tm_year * 10000) + (file_date->tm_mon * 100) \
					+ (file_date->tm_mday);

			if(date1 > date2)
			{
				if(i==0)
					return(0);
				else
					number_of_files = i++;

				break;
			}
			

		}

	}

	setlocale(LC_ALL,"");
	diag_asl_init("default");
	catfd = diag_catopen(MF_DIAGRPT,0);

	disp_menu(number_of_files,file_ptr);


	diag_asl_quit(NULL);
	catclose(catfd);
	free(file_ptr);
	return(0);
}

/*  */
/*
 * NAME: file_sort
 *                                                                    
 * FUNCTION: Sort files in an array by their time modified.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * NOTES: 
 *
 * RETURNS:
 *	file_status_ptr_t *
 */  

 file_status_ptr_t *
file_sort(file_info, num_elements)
file_status_t	*file_info;	
int		num_elements;
{
	int			i,j,gap;
	file_status_ptr_t	*file_sptr;	
	file_status_ptr_t	*temp;	

	temp = (file_status_ptr_t *) calloc(1,sizeof(temp[0]));
	file_sptr = (file_status_ptr_t *)
			calloc(num_elements,sizeof(file_sptr[0]));
	for (i = 0; i < num_elements; i++ )
		file_sptr[i] = &file_info[i];

	for ( gap = num_elements/2; gap > 0; gap /=2 )
		for ( i=gap; i < num_elements; i++ )
			for ( j=i-gap; j>=0 && file_sptr[j]->buf.st_mtime <
			    	    file_sptr[j+gap]->buf.st_mtime; j-=gap) {
				temp[0] = file_sptr[j];
				file_sptr[j] = file_sptr[j+gap];
				file_sptr[j+gap] = temp[0];
			}

	return(file_sptr);

}
/*  */
/*
 * NAME: disp_menu()
 *                                                                    
 * FUNCTION:  Display's menu with one diagnostic result.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * NOTES: 
 *
 * RETURNS:
 *	
 */  
void
disp_menu(int ind,file_status_ptr_t * f_ptr)
{
	int i;
	int rfile=-1;   /* file descripter for file to read */
	ASL_SCR_INFO * menuinfo;
	static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

	menuinfo = (ASL_SCR_INFO *) calloc(ind+2 ,sizeof(ASL_SCR_INFO) );

	if(menuinfo == (ASL_SCR_INFO *) NULL )
	{
		return;
	}
 	menuinfo[0].text = diag_cat_gets(catfd,MSGSET,DRPTTITLE); 
	for(i=0;i<ind;i++)
	{

			rfile = open(f_ptr[i]->filename,O_RDONLY);


			if( rfile < 0 )
			{
				return;
			}

			menuinfo[i+1].text = (char *) calloc(f_ptr[i]->buf.st_size + 100,sizeof(char));

			if(read(rfile,menuinfo[i+1].text,f_ptr[i]->buf.st_size) 
			 	!= (int) f_ptr[i]->buf.st_size)
			{

				return;
			}

			close(rfile);

			/* 
			 * remove old menu number and place in separator between
			 * files
			 */
			if(f_ptr[i]->buf.st_size == 0)
				continue;

			if(strtok(menuinfo[i+1].text,"\t") != NULL)
				menuinfo[i+1].text = strtok(menuinfo[i+1].text,"\t");

			if(strstr(menuinfo[i+1].text,"801") != NULL)
				memset(strstr(menuinfo[i+1].text,"801"),' ',6);

			memset( (char *)strrchr(menuinfo[i+1].text,'\0') , '\n',3); 
			memset( (char *)strrchr(menuinfo[i+1].text,'\n') , '-',81); 
			memset( (char *)strrchr(menuinfo[i+1].text,'-') , '\n',5); 
			*( (char *)strrchr(menuinfo[i+1].text,'\n')) = '\0';  

	}
 	menuinfo[ind+1].text = diag_cat_gets(catfd,MSGSET,DSCROLL);

	menutype.max_index = ind + 1 ;

	diag_display(MENUNUMBER,catfd,NULL,DIAG_IO,ASL_DIAG_KEYS_ENTER_SC, &menutype,menuinfo); 

	for(i=1;i<=ind;i++)
	{
		free(menuinfo[i].text);
	}

	free(menuinfo);

}
/*  */
/*
 * NAME: int_hand()
 *                                                                    
 * FUNCTION:  Cleans up after a quit or term signal is received.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * NOTES: 
 *
 * RETURNS:
 *	
 */  

void int_hand(int sig)
{
	signal(sig,int_hand);

	diag_asl_quit(NULL);
	catclose(catfd);

	exit(1);
}
