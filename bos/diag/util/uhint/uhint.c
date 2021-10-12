static char sccsid[] = "@(#)43	1.3  src/bos/diag/util/uhint/uhint.c, dutil, bos411, 9428A410j 6/8/94 16:07:48";
/*
 *   COMPONENT_NAME: DUTIL
 *
 *   FUNCTIONS: all_init
 *		close_all
 *		disp_menu
 *		int_hand
 *		main
 *		read_file
 *		select_readme
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>    
#include <errno.h>    
#include <time.h>    
#include <nl_types.h>
#include <locale.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/dir.h>

#include "diag/diag.h"
#include "diag/diago.h"
#include "uhint.h"

nl_catd catfd;

/*
 * NAME: main
 *                                                                    
 * FUNCTION: Display a CEREADME file
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: A specific file can be displayed.
 *		-n full_path_of_file 
 *
 * RETURNS:  NONE
 */

extern int errno;
extern char *optarg;
extern int opterr;
extern int optopt;
extern int optind;

struct ceread * * file_info;
struct ceread cefile_info[MAXFILES];

main(int argc, char * *argv)
{
	int i;
	int count=0;	/* number of files which match criteria in scandir */
	struct dirent * * list;
	long rc =0;
	char * directory;

	setlocale(LC_ALL,"");

	signal(SIGKILL,int_hand);	
	signal(SIGINT,int_hand);	
	signal(SIGQUIT,int_hand);	
	signal(SIGTERM,int_hand);	

		
	while((rc = getopt(argc,argv,"n:")) != EOF)
	{
		switch(rc)
		{
			case 'n':
				strcpy(cefile_info[count].path,optarg);
				count ++;
				break;
			default:
				printf("usage: uhint [-n file]\n");
				exit(1);
		}
	}

	/* 
	 * initialize everything
	 */
	all_init();


	/* Get directory where CEREADME files should be stored */
	directory = getenv("DIAGNOSTICS");

	/* 
	 * if DIAGNOSTICS environment variable does not exist, use default from 
	 * diag.h
	 */
	if( directory == (char *) NULL)	
		directory = DIAGNOSTICS;


	/* get listing of files in the diagnostics directory */
	if(count == 0)
	{
	 	count = scandir(directory,&list,select_readme,NULL);

		if(count <=0 )
		{
			close_all();
		}
	}

	file_info = (struct ceread  * *) calloc(count,sizeof(struct ceread));

	if(file_info == (struct ceread * * ) NULL)
	{
		close_all();
	}


	/* 
	 * point to ce_info file memory space
	 */
	for(i=0;i<count;i++)
	{
		file_info[i] = &cefile_info[i];
	}


	/*
	 * get path and name of CEREADME files 
	 */

	for(i=0;i<count && optarg == NULL;i++) 
	{
		strcpy(file_info[i]->path,directory);
		strcat(file_info[i]->path,"/");
		strcat(file_info[i]->path, list[i]->d_name );
	}


	/* display menu of CE Readme's */
	do
	{
		rc = disp_menu(SELECT,count);
		/*
		 * if there is one file, show it and then get out
		 */
		if(count == 1)
			break;
	}while(rc == 0); 

	close_all();
}

/*  */
/*
 * NAME: disp_menu
 *                                                                    
 * FUNCTION: Display menu for CE to choose which README to view and
 *			 then put up file on screen using ASL.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * RETURNS:
 *	int 
 */

int disp_menu(int menu,int files)
{
	long rc;
	int i;
	static int cefile;
	ASL_SCR_INFO * menuinfo;
	static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

	menuinfo = (ASL_SCR_INFO *) calloc(files+2,sizeof(ASL_SCR_INFO));
	if(menuinfo == (ASL_SCR_INFO * ) NULL)
	{
		close_all();
	}

	/* 
	 * allocate memory and copy in title
	 */
	menuinfo[0].text =(char *) calloc(
					strlen(diag_cat_gets(catfd,MSGHINT,MTITLE)),sizeof(char));
	if(menuinfo[0].text == (char *) NULL)
		close_all();

	strcpy(menuinfo[0].text,diag_cat_gets( catfd,MSGHINT,MTITLE ));


	switch(menu)
	{
		case SELECT:

		  /* if files > 1 then display files.
		   * otherwise, display one file
		   */
		  if(files > 1)
		  {

			for(i=0;i<files;i++)
			{
				menuinfo[i+1].text = file_info[i]->path;
			}
			/*
			 * allocate memory for text 
			 */
			menuinfo[files+1].text = (char *) calloc(
					strlen(diag_cat_gets(catfd,MSGHINT,MENTER2)),sizeof(char));

			if(menuinfo[files+1].text == (char *) NULL)
				close_all();

			/* 
			 * copy in text 
			 */
			strcpy(menuinfo[files+1].text,diag_cat_gets(catfd,MSGHINT,MENTER2));

			menutype.max_index = files+1;
			menutype.cur_index = 1;
			rc = diag_display(SELMENU,catfd,NULL,DIAG_IO,ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype,menuinfo); 

			if(rc == DIAG_ASL_CANCEL || rc == DIAG_ASL_EXIT || rc == DIAG_ASL_FAIL)
			{
				free(menuinfo[files+1].text);
				free(menuinfo);
				return(DIAG_ASL_EXIT);
			}

			cefile = menutype.cur_index;
		  }
		  else
			cefile = 1;

			rc = disp_menu(DISPLAYCE,1);
			break;

		case DISPLAYCE:

			menuinfo[files].text = read_file(cefile-1);

			/*
			 * allocate memory for text 
			 */
			menuinfo[files+1].text = (char *) calloc(
					strlen(diag_cat_gets(catfd,MSGHINT,MENTER3)),sizeof(char));

			if(menuinfo[files+1].text == (char *) NULL)
				close_all();

			/* 
			 * copy in text 
			 */
			strcpy(menuinfo[files+1].text,diag_cat_gets(catfd,MSGHINT,MENTER3));


			menutype.max_index = files+1;

			rc = diag_display(DISPCE,catfd,NULL,DIAG_IO,ASL_DIAG_KEYS_ENTER_SC, &menutype,menuinfo);

			
			free(menuinfo[files].text);
			free(menuinfo[files+1].text);
			break;

		default:

			rc = DIAG_ASL_FAIL;
			break;
	}

	free(menuinfo[0].text);
	free(menuinfo);

	if(rc == DIAG_ASL_EXIT || rc == DIAG_ASL_FAIL)
		return(DIAG_ASL_EXIT);
	else
		return(0);
}


/*  */
/*
 * NAME: int_hand
 *                                                                    
 * FUNCTION: Interupt handler to capture and handle SIGQUIT and SIGTERM
 *			 so that ASL is quit properly and catalog is closed.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * RETURNS:
 */

void int_hand(int sig)
{
	signal(sig,int_hand);
	close_all();
}
/*  */
/*
 * NAME: all_init
 *                                                                    
 * FUNCTION: Initialize locale,ASL, and open catalog file
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * RETURNS:
 */

void all_init()
{

	diag_asl_init("NO_TYPE_AHEAD");
   	catfd = diag_catopen (CAT_NAME,0);

}

/*  */
/*
 * NAME: close_all
 *                                                                    
 * FUNCTION: Used to quit ASL and close catalog and then exit to controller
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * RETURNS:
 */
void close_all()
{
	diag_asl_quit();
	catclose(catfd);
	exit(0);
}

/*  */
/*
 * NAME: select_readme
 *                                                                    
 * FUNCTION: Routine called by scandir to decide which files to pass back
 *			 in the array.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * RETURNS:
 */
select_readme(struct dirent * dir)
{
	/* 
	 * if there is a match of a file then return 
	 * a 1 to scandir to reflect a good match.
	 */
	if(strncmp(dir->d_name,"CEREAD",6) == 0)
		return(1);
	else	
		return(0);
} /* select_readme() end */

/*  */
/*
 * NAME: read_file
 *                                                                    
 * FUNCTION: Reads in one file and passes ptr to data back to calling function.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * RETURNS:
 */
char * read_file(int number)
{
	char * data;
	int fdes;
	int rc;
	int i;
	
	/* 
	 * get file information
	 */
	rc = stat(file_info[number]->path,&file_info[number]->info);
	if(rc < 0)
	{
		close_all();
	}

	fdes = open(file_info[number]->path,O_RDONLY);
	if(fdes < 0)
	{
		close_all();
	}

 	/* 
	 * allocate memory for file
	 */
	data = (char *) calloc(file_info[number]->info.st_size + 1,sizeof(char));
	if(data == (char *) NULL)
	{
		close_all();
	}
		
	/*
	 * read in data
	 */
	rc = read(fdes,data,file_info[number]->info.st_size) ;
	if(rc != file_info[number]->info.st_size) 
	{
		close_all();
	}

	close(fdes);

	/* insert a NULL at the end of the file */
	data[file_info[number]->info.st_size] = '\0';

	for(i=0;i<file_info[number]->info.st_size;i++)
	{
	    /* if this is a form feed, turn it into a carriage return line feed*/
		if(data[i] == 0x0C )
			data[i] = '\n';
	}

	return(data);
}
