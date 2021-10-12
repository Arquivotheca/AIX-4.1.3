static char sccsid[] = "@(#)97	1.3  src/bos/diag/da/corvette/common.c, dascsi, bos411, 9428A410j 9/1/93 18:10:52";
/*
 *   COMPONENT_NAME: DASCSI
 *
 *   FUNCTIONS: do_erd
 *		env_task
 *		get_er_string
 *		get_task_data
 *		get_tasks
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
#include	<stdio.h>
#include	<nl_types.h>
#include	<sys/cfgodm.h>
#include	<diag/diag.h>
#include	<diag/diago.h>
#include	<diag/tm_input.h>
#include	<diag/diag_exit.h>
#include	<diag/tmdefs.h>
#include	<diag/da.h>
#include	"common.h"

extern	struct	tm_input	tm_input;
extern	nl_catd	catd;
char	* get_er_string();
 
/*
 * NAME: do_erd()
 *
 * FUNCTION: Designed to do error recovery data.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: execution status.
 */

do_erd( all_tasks,idx_task,task_err,erd_prior,frus,goal,num_erd_type)
struct	task	*all_tasks;	/* Pointer to all tasks to be executed	*/
int	*idx_task;		/* Current executed task index		*/
int	task_err;		/* Task executed return code		*/
char	*erd_prior[];		/* Error data priority search list	*/
struct	fru_bucket	*frus;
struct	msglist	*goal;
int	num_erd_type;		/* Number of error data type stored 	*/
				/* in ODM				*/
{

	char	erd_string[512];
	char	*srch_crit;	/* search criteria			*/
	int	i=0;
	char	error[20];	/* space to convert task error into string */
	char	*err_str;
	char	*menu;		/* pointer to a menu message.		*/
	char	*mgoal;		/* pointer to a menu message.		*/
	char	*srns;		/* pointer to SRN.			*/
	int	rc=0;
	int	rc_code=0;
	int	retry=0;	/* flag is set when task execution goes	*/
				/* into retry.				*/

	/* loop thru the number of the priority list or number of diff. */
	/* erd in the data base.					*/
	for ( i=0; i<num_erd_type;i++)
	{
		srch_crit=calloc(strlen(erd_prior[i])+
			strlen(all_tasks[*idx_task].task_code),sizeof(char));
		sprintf(srch_crit,"%s%s",all_tasks[*idx_task].task_code,
				erd_prior[i]);

		srch_crit[1]='0';
		/* get data task from the data base if available.	*/
		(void)memset(erd_string,0,sizeof(erd_string));
		rc=get_task_data(srch_crit,&erd_string);
		free(srch_crit);

		/* if there is no data for this task then skip to the 	*/
		/* next item in the priority list.			*/
		if(strlen(erd_string) == 0)
			continue;

		/* there is error recovery data task. 			*/
		sprintf(error,"%4d",task_err);
		err_str=get_er_string(erd_string,error);
	 
		/* if the task error found in the error recovery data	*/
		/* then go through the while loop and execute all the 	*/
		/* instruction given by the data base.			*/
		while(strlen(err_str)!=0 && !retry)
		{
			switch(err_str[0])
			{
			case 'E':
				/* set all the exit code required by 	*/
				/* controller				*/
				DA_SETRC_STATUS(err_str[1]-'0');
				DA_SETRC_USER(err_str[2]-'0');
				DA_SETRC_ERROR(err_str[3]-'0');
				DA_SETRC_TESTS(err_str[4]-'0' );
				DA_SETRC_MORE(err_str[5]-'0');

				/* set flag to exit the da		*/
				rc_code=EXIT_NOW;

				/* increment the error recovery	*/
				/* data count so we do not go	*/
				/* through the rest of ERDs.	*/
				i=num_erd_type+1;
				break;
			case 'J':
				/* jump instruction is selected.	*/
				/* retry counter NOT exeeded 	*/
				if(all_tasks[*idx_task].retry_count
					<= (err_str[1] -'0'))
				{
					/* update retry count in task	*/
					all_tasks[*idx_task].
							retry_count++;
					*idx_task=atoi(&err_str[2]);
					retry=TRUE;
				}
				break;
			case 'S':
				/* report an SRN			*/
				sprintf(frus[(atoi(&err_str[1]))].dname,"%s",
					tm_input.dname);
				rc = insert_frub( &tm_input,&frus
					[atoi(&err_str[1])]);
				if ((rc != 0) ||
					(addfrub(&frus[
					atoi(&err_str[1])]) != 0 ))
				{
					DA_SETRC_ERROR(DA_ERROR_OTHER);
					rc_code=EXIT_NOW;
					i=num_erd_type+1;
					break;
				}
				/* point to the srn maybe needed in 	*/
				/* in menu goal				*/
				srns=frus[atoi(&err_str[1])].sn;
				break;
			case 'G':
				/* report a menugoal.			*/

				if(err_str[1] == 'S')
				{
					/* menugoal refer to an srn	*/
					menu = diag_cat_gets(catd,
					goal[atoi(&err_str[2])].setid,
					goal[atoi(&err_str[2])].msgid);
					mgoal = (char *)calloc(strlen(menu)
						+strlen(srns)
						+strlen(tm_input.dnameloc)
						+2,sizeof(char));
					/* insert the device location	*/
					/* and srn into the menugoal	*/
					sprintf(mgoal,menu,tm_input.dnameloc,
						srns);
				}
				else
				{
					menu = diag_cat_gets(catd,
					goal[atoi(&err_str[1])].setid,
					goal[atoi(&err_str[1])].msgid);
					
					/* insert the device location	*/
					/* into the menugoal		*/
					mgoal = calloc(strlen(menu)
						+strlen(tm_input.dnameloc)
						+2,sizeof(char));
					sprintf(mgoal,menu,tm_input.dnameloc);
				}
				menugoal(mgoal);
				free(mgoal);
				break;
			default:
				break;
			}
			err_str=strstr(err_str,",");
			if(err_str != NULL) 
				err_str++;
		}
	}
      return(rc_code);
}

/*
 * NAME: get_tasks()
 *
 * FUNCTION: Designed to allocate memory for each task and copy the task
 *	     into the allocated memory.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: struct task pointer.
 */

struct	task	*
get_tasks(string,count)
char	*string;	/* data string passed from the caller	*/
int	*count;		/* number of tasks found in a string.	*/
{
	char	*tmp;
	char	*newstr;	/* temp. task string.		*/
	struct	task	*ar_task;
	
	newstr=string;
	/* allocate memory for the first task.			*/
	ar_task=calloc(1,sizeof(struct task));
	while ( newstr != NULL )
	{
		/* Remove all the spaces before each task 	*/
		while(isspace(newstr[0])&&  newstr[0]!=NULL)
			newstr=newstr++;

		/* make sure the string task is not empty	*/
		if(newstr[0]==NULL)
			break;

		/* search for the next space after the task and */
		/* assign tmp = to that space.			*/

		tmp=strstr(newstr," ");
		ar_task=realloc(ar_task,(*count +1) * sizeof(struct task));

		/* copy the task into the array of tasks.	*/
		memset(ar_task[*count],0,sizeof(struct task));
		strncpy(ar_task[*count].task_code,newstr,4);
		ar_task[*count].task_code[4]='\0';
		ar_task[*count].retry_count=0;
		newstr=tmp++;	/* assign the newstr = to the next	*/
				/* char. after the space.		*/

		(*count)++;
      }
 
      return ( ar_task);
}
 
/*
 * NAME: get_er_string()
 *
 * FUNCTION: Designed to get the error string from the ODM.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: Pointer to the error string.
 */

char	*
get_er_string(str1,str2)
char	*str1;	/* task string obstracted from the data base.	*/
char	*str2;	/* task error code.				*/
{
	char	*tmp1;
	char	*tmp2;
	char	*tmp;
	int	tmp_len;

	tmp1 = calloc(strlen(str2)+2,sizeof(char));

	/* append ":" to task error code			*/
	sprintf(tmp1,"%s:",str2);
	tmp_len=strlen(tmp1);

	/* search for the task error string in ERD String	*/
	tmp1=strstr(str1,tmp1);
	if(tmp1 == NULL)
	{
		/* Search for default ERD 			*/
		tmp1=strstr(str1,"ALL:");
		tmp_len=4;
	}
	if(tmp1 != NULL)
	{
		/* assign tmp1 =found the error starting string.*/
		tmp1 = &tmp1[tmp_len];

		/* search for the first delimeter after ERD str.*/
		/* Allocate memory for the founded string then	*/
		/* copy it and return it as pointer.		*/

		tmp2=strstr( tmp1," ");
		tmp=calloc(strlen(tmp1)-strlen(tmp2)+1,sizeof(char));
		strncpy (tmp,tmp1,strlen(tmp1)-strlen(tmp2));
		tmp[strlen(tmp1)-strlen(tmp2)+1]='\0';

		return(tmp);
	}
	else
		return(tmp1);
}
 
/*
 * NAME: get_task_data()
 *
 * FUNCTION: Designed to get the task data from the data base if available.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: Return code of diag_get_att
 */

get_task_data(task,string)
char	*task;		/* PDiagAtt search type			*/
char	string[];	/* allocated memory for the odm data.	*/
{
	char	odm_srch_crit[80];	/* odm search criteria  */
	static	struct	CuDv *cudv;	/* Pointer to CuDv	*/	
	static	struct	listinfo	objinfo;
	int	bc;
	int	rc;

	static	short	tsk_flag=0;

	if (!tsk_flag)
	{
		/* get the cudv list for this application	*/
		(void)memset(odm_srch_crit,0,sizeof(odm_srch_crit));
       		sprintf( odm_srch_crit, "name = %s", tm_input.dname);

		cudv = get_CuDv_list(CuDv_CLASS,odm_srch_crit,&objinfo,16,2);
		if (cudv == (struct CuDv *)NULL)
		{
			clean_up();
			DA_SETRC_ERROR(DA_ERROR_OTHER);
			DA_EXIT();
		}
		tsk_flag=TRUE;
	}

	/* if the task code = "3***" then change to "3000" which 	*/
	/* contains all asl ERD codes.					*/
	if(task[0]=='3')
	{
		task[2]='0';
		task[3]='0';
	}

	rc=get_diag_att( cudv->PdDvLn->type, task, 's', &bc, string);

	return( rc);
}
 
/*
 * NAME: env_task()
 *
 * FUNCTION: Designed to check if the passed string allowed to execute
 *	     in supplied tm_input environment.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: TRUE or FALSE
 */

env_task(string)
char	*string;	/* pointer to data task.			*/
{
	short	mode_flag	= FALSE;
	if(tm_input.dmode == DMODE_ELA )
	{
		/* Task runs ELA ONLY */
		if( string[1] == '9' )
			return( TRUE );
		else
			return(FALSE);
	}
	else
	{

		switch(string[1])
		{
			case '0':
				/* task executes in all envirenment	*/
				mode_flag=TRUE;
				break;
			case '1':
				/* task execute in pretest		*/
				if(tm_input.exenv == EXENV_IPL)
					mode_flag=TRUE;
				break;
					
			case '2':
				/* task execute in StandAlone		*/
				if(tm_input.exenv == EXENV_STD)
					mode_flag=TRUE;
				break;
			case '3':
				/* task execute in Advance mode		*/
				if(tm_input.advanced == ADVANCED_TRUE)
					mode_flag=TRUE;
				break;
                        case '4':
                                /* task execute in Enter Loop mode      */      
                                if(tm_input.loopmode == LOOPMODE_ENTERLM)
					mode_flag=TRUE;
				break;
			case '5':
                                /* task execute in In Loop mode      */      
                                if(tm_input.loopmode == LOOPMODE_INLM)
					mode_flag=TRUE;
				break;
			case '6':
                                /* task execute in Exit Loop mode      */      
                                if(tm_input.loopmode == LOOPMODE_EXITLM)
					mode_flag=TRUE;
				break;
			case '7':
                                /* task execute in Customer mode only	*/
                                if(tm_input.advanced == ADVANCED_FALSE)
					mode_flag=TRUE;
				break;
			case '8':
                                /* task execute in Advanced & stand alone*/
                                if(tm_input.advanced == ADVANCED_TRUE &&
					tm_input.exenv == EXENV_STD)
					mode_flag=TRUE;
				break;
			case '9':
                                /* task execute in ELA mode only	*/
                                if(tm_input.dmode == DMODE_ELA)
					mode_flag=TRUE;
				break;
			case 'A':
                                /* task execute in PD mode only	*/
				if(tm_input.dmode == DMODE_PD)
					mode_flag=TRUE;
				break;
			default:
				break;
		}
	}
	return(mode_flag);
}
