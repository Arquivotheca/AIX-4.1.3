static char sccsid[] = "@(#)98  1.2  src/bldenv/pkgtools/rename/dependency_list/dependency_list.c, pkgtools, bos412, GOLDA411a 6/24/93 08:38:17";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: append_ptf_list
 *		check_malloc_return
 *		create_index_list
 *		create_ptf_list
 *		find_dependent_ptfs
 *		find_ptf_param
 *		get_next_index_line
 *		get_next_ptf_info
 *		help
 *		in_ptf_list
 *		main
 *		output_server_ptfs
 *		rewind_index_list
 *		rewind_ptf_list
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*--------------------------------------------------------------------*/
/*                                                                    */
/*  Purpose:                                                          */
/*      This program finds all upstream ptfs which directly           */
/*      or indirectly rely upon the specified ptf(s).  Only           */
/*      ptf(s) with the same lpp_name as the given ptf(s) are         */
/*      listed.  If -c check option is given, the program             */
/*      checks whether all dependent ptfs are included on the         */
/*      command line and returns either 0 or 1.  If the -c check      */
/*	option is not given, a list of dependent ptfs is generated.   */
/*                                                                    */
/*  Syntax:                                                           */
/*  'dependency_list   -b dirname_build  [-p dirname_prod]      \\    */
/*                    [-s dirname_ship] [-v] [-c] [-h] [-?]     \\    */
/*                    [-o outfilename ]      			\\    */
/*                    ptf_filename [ptf_filename] ....'               */
/*               where:						      */
/*                 dirname_prod  - path to mif directory on	      */
/*                                'production' server.		      */
/*                 dirname_ship  - path to mif directory on	      */
/*                                 'ship' server.		      */
/*                 dirname_build - path to mif directory on	      */
/*                                 'build' server.		      */
/*                 -o outfilename   - path & filename for list	      */
/*                                    of prereqs, coreqs, ifreqs,     */
/*			 	      and supercedes from index files.*/
/*                 -v         verbose option - prints extra infomation*/
/*                            on the dependencies found               */
/*                 -c         check option - prints out missing dep.  */
/*                            ptfs and gives a return code based on   */
/*                            whether any ptfs were missing from the  */
/*                            command line.                           */
/*                 ptf_name - ptffilename used in on server.	      */
/*                              (ie. U0000257.ptf)		      */
/*                            This will allow execution from Motif    */
/*                              or EMACS.			      */
/*                 -h | -?   options to list usage message and exit   */
/*                                                                    */
/*                                                                    */
/*  Change log:							      */
/*     07/01/91 spm: changed to handle empty index file        	      */
/*     07/02/91 spm: changed to handle -c check option         	      */
/*     07/09/91 spm: changed create_index_list procedure to use strtok*/
/*     07/30/91 spm: changed to make build server optional            */
/*--------------------------------------------------------------------*/
#include <stdio.h>
#include <fcntl.h>

/** set up name of index files **/
#define INDEX "index"

/** index file list structures (array of lines) **/
typedef struct { 
	 char *lpp1,*ptf1,*req_type,*lpp2,*ptf2;
    } index_line_t;   /* index line type */

typedef struct {
    	 index_line_t *index_line;  /* *index_line will essentially  */
                                    /* become index_line[num_lines]  */
         		  	    /* by using malloc       	     */
	 int num_lines, cur_line;
    } index_list_t;   /* index list type */ 

/** ptf list structure (linked list) **/
struct ptf_info {   /* ptf info node */
	 char *ptf;
	 char *server;
	 struct ptf_info *next_ptf;
    };
typedef struct ptf_info  ptf_info_t;  /* ptf info type */

typedef struct {
	 ptf_info_t *start, *current, *last;
    } ptf_list_t;  /* ptf list type (linked list of ptf_info nodes) */

/** declare functions **/
index_list_t *create_index_list(char *server_dir);
int find_ptf_param(index_list_t *index_list, char *server, 
                                                  char *ptf_param);
void find_dependent_ptfs(index_list_t *index_list, char *server, 
                        ptf_list_t *ptf_list);
void rewind_index_list(index_list_t *index_list);
index_line_t *get_next_index_line(index_list_t *index_list);
ptf_list_t *create_ptf_list();
void append_ptf_list(ptf_list_t *ptf_list, char *new_ptf, 
                                    		char *new_server);
int in_ptf_list(ptf_list_t *ptf_list, char *chk_ptf);
void rewind_ptf_list(ptf_list_t *ptf_list);
ptf_info_t *get_next_ptf_info(ptf_list_t *ptf_list);
void check_malloc_return(char *ptr);
void help();

/** input parameter flags **/
int verbose = FALSE;
int check_mode = FALSE;

/** Misc. vars  **/
int return_code = 0;
int loop_finished;

main(int argc, char *argv[])
{
  /* argument file and directory names */
  char   *dir_build = NULL;
  char   *dir_prod = NULL; 
  char   *dir_ship = NULL;
  char   *out_name = NULL; 
  
  /* output file descriptor */
  FILE *out_file;  

  /* index file lists and ptf list */
  index_list_t *build_list, *prod_list, *ship_list;
  ptf_list_t *ptf_list;
  
  /* server name constants */
  char build_server[] = "build server";
  char prod_server[]  = "production server";
  char ship_server[]  = "ship server";

  /*  getopt var's */
  extern char *optarg;
  extern int optind;
  int c;
  
  /* Misc. var's */
  int i;
  int err_code = 0;
  char *ptf_param;
  char *period_ptr;
  char *slash_ptr;
  int ptf_found;
  

  /*** parse the command line ***/
  while((c = getopt(argc , argv, "h?vcb:p:s:o:")) != EOF)
    switch(c)
       {
         case 'h':help();
         case '?':if(strcmp(argv[optind - 1],"-?") == 0)
                    help();        /* '-?' was actually entered */
                  else
                    err_code = 1;  /* an illegal option was entered */
                  break;
         case 'v':verbose = TRUE;break;
         case 'c':check_mode = TRUE;break;
         case 'b':dir_build = optarg;break;
         case 'p':dir_prod = optarg;break;
         case 's':dir_ship = optarg;break;
         case 'o':out_name = optarg;break;
         default:err_code=1;
       } /* end of switch */


  /***  Check the parameters ***/

  if((dir_build == NULL)&&(dir_prod == NULL)&&(dir_ship == NULL))
  { printf("dependency_list: one server parameter is required\n");
    err_code = 1;
  }

  if(optind == argc)
  {  printf("dependency_list: one ptf_name parameter is required\n");
     err_code = 1;
  }


  /* Give help and exit if an error was found */
  if(err_code != 0)
     help(); 


  /*** strip off '.ptf' and path if supplied on input ptf_names	***/

  for( i = optind; i < argc ; i++)
  {  ptf_param = argv[i];

     /* strip off .ptf */
     period_ptr = (char *) ptf_param + strlen(ptf_param) - 4;
     if(strcmp(period_ptr, ".ptf") == 0)
        *period_ptr = '\0';

     /* strip off path if supplied */
     if((slash_ptr = strrchr(ptf_param,'/')) != NULL) 
        strcpy(ptf_param, (char *) slash_ptr + 1);

  } /* end of for */


  /*** verify that output file is accessible ***/     
  
  if(out_name == NULL)
     out_file=stdout;
  else
  {  out_file = fopen(out_name, "w");
     if(out_file == NULL)
     {  printf("dependency_list: can't open output: %s\n",out_name);
        exit(1);
     }
  }


  /*** For each server, create index file lists ***/

  if(dir_ship != NULL)
     ship_list = create_index_list(dir_ship);
  if(dir_prod != NULL)
     prod_list = create_index_list(dir_prod);
  if(dir_build != NULL)
     build_list = create_index_list(dir_build);
  

  /*** verify that each ptf given on the command line is in the    ***/
  /*** index files and add the ptf and its server to the ptf_list  ***/

  ptf_list=create_ptf_list();
  for( i = optind; i < argc ; i++)
  {  ptf_param = argv[i];
     ptf_found = FALSE;

     if((!ptf_found)&&(dir_ship != NULL))
       if(ptf_found = find_ptf_param(ship_list,ship_server,ptf_param))
         append_ptf_list(ptf_list, ptf_param, ship_server);

     if((!ptf_found)&&(dir_prod != NULL))
       if(ptf_found = find_ptf_param(prod_list,prod_server,ptf_param))
         append_ptf_list(ptf_list, ptf_param, prod_server);

     if((!ptf_found)&&(dir_build != NULL))
       if(ptf_found=find_ptf_param(build_list,build_server,ptf_param))
         append_ptf_list(ptf_list, ptf_param, build_server);

     if(!ptf_found)
     {  printf("dependency_list: input parameter '%s' ",ptf_param);
        printf("was not found in the index files\n");
        err_code = 1;
     }
  } /* end of for */
  
  if(err_code != 0)  /* there were bad ptf parameter(s) */
  {  printf("dependency_list: process terminated\n");
     exit(1);
  }

  
  /*** Loop through the index lists adding dependent ptfs to the  ***/
  /*** ptf_list until a pass is made through the index lists      ***/
  /*** in which no more dependent ptfs are found.  loop_finished  ***/
  /*** will stay TRUE if no dependencies are added during a pass. ***/

  loop_finished=FALSE;
  while(!loop_finished)
  {  loop_finished=TRUE;
     if(dir_ship != NULL)
        find_dependent_ptfs(ship_list,ship_server,ptf_list);
     if(dir_prod != NULL)
        find_dependent_ptfs(prod_list,prod_server,ptf_list);
     if(dir_build != NULL)
        find_dependent_ptfs(build_list,build_server,ptf_list);
  } /* end of while */

 
  /*** For each server given, free index file lists ***/

  if(dir_build != NULL)
     free(build_list);
  if(dir_prod != NULL)
     free(prod_list);
  if(dir_ship != NULL)
     free(ship_list);


  /*** clean up and exit ***/

  sync();
  exit(return_code);

} /* End of Main */


/*********************************************************************/
/*  create_index_list: this function reads the given index file      */
/*      and returns a pointer to the index list containing an        */
/* 	array of index lines read from the index file. 		     */
/*********************************************************************/

index_list_t *create_index_list(char *server_dir)
{
  /* index file var's */
  index_list_t *index_list;
  int in_fd;
  char *in_name;
  int in_length;

  /* string pointers */
  char *file_buf;
  char *cur_char_ptr;
  char *cur_line_ptr;

  /* counters and index */
  int line_count;
  int colon_count;
  int i;


  /* set up in_name (the index file name) */
  in_name = (char *) malloc(strlen(server_dir)+strlen(INDEX)+2);
  check_malloc_return(in_name);
  strcpy(in_name,server_dir);
  if(in_name[strlen(in_name) - 1] != '/')  /* append '/' if needed */
    strcat(in_name,"/");
  strcat(in_name,INDEX);

  /* open index file, read it into memory, and close it */
  in_fd = open(in_name, O_RDONLY);  
  if(in_fd == -1)
  {  printf("dependency_list: can't open: %s\n",in_name);
     exit(1);
  }

  in_length = lseek(in_fd, 0, SEEK_END); /* get length of file */
  lseek(in_fd, 0, SEEK_SET);     /* reset pointer to start of file */
  file_buf = (char *) malloc(in_length+1); /* +1 to have room for  */
  check_malloc_return(file_buf);           /*   end of string    */
  read(in_fd, file_buf, in_length);  /* read file into memory */
  file_buf[in_length] = '\0';        /* put on end of string */
  close(in_fd);
 
 
  /* check format of index file (four colons on each line) and count */
  /* the number of lines for subsequent array allocation             */ 
  
  line_count=colon_count=0;
  cur_line_ptr=file_buf;
  for( i=0; i < in_length ; i++)
  {  cur_char_ptr = file_buf + i;
     switch(*cur_char_ptr) 
     {  case ':' : colon_count++; break; 
        case '\n':
            if(colon_count != 4)
  	    {   *cur_char_ptr='\0';
		printf("dependency_list: index file %s has bad data\n"
                                                             ,in_name);
                printf("line %d: '%s'\n",line_count+1, cur_line_ptr);
  	        printf("dependency_list: process terminated\n");
     		exit(1);
            }
            line_count++;
            colon_count=0;
            cur_line_ptr=cur_char_ptr + 1;
     } 
  } /* end of for */

  
  /* allocate space for index_list */
  index_list = (index_list_t *) malloc(sizeof(index_list_t));
  check_malloc_return(index_list);

  /* allocate space for index_line array */ 
  index_list->index_line = 
       (index_line_t *) malloc(line_count * sizeof(index_line_t)+1); 
  check_malloc_return(index_list->index_line);      /* +1 above ^^ */
        /* so malloc won't bomb if line_count is zero (empty file) */

  /* Set the array of index line pointers to point to the ptfs, lpps, */
  /* and req_type read into memory from the index file.  Replace      */
  /* colons and newlines with '\0' to separate the data.  The format  */
  /* of an index line is 'lpp1:ptf1:req_type:lpp2:ptf2\n'             */

  for( i=0; i < line_count ; i++)
  { if(i==0)
       index_list->index_line[i].lpp1  = strtok(file_buf,":\n");
    else
       index_list->index_line[i].lpp1  = strtok(NULL,":\n");
    index_list->index_line[i].ptf1     = strtok(NULL,":\n");
    index_list->index_line[i].req_type = strtok(NULL,":\n");
    index_list->index_line[i].lpp2     = strtok(NULL,":\n");
    index_list->index_line[i].ptf2     = strtok(NULL,":\n");

    /* check that req_type is valid */
    if( (strcmp(index_list->index_line[i].req_type,"coreq")     != 0) &&
        (strcmp(index_list->index_line[i].req_type,"prereq")    != 0) &&
        (strcmp(index_list->index_line[i].req_type,"ifreq")     != 0) &&
        (strcmp(index_list->index_line[i].req_type,"supersede") != 0) )
    {    printf("dependency_list: index %s has bad req type\n",in_name);
         printf("line %d: invalid type of '%s'\n",i+1,
                     index_list->index_line[i].req_type);
         printf("dependency_list: process terminated\n");
         exit(1);
    }
  }

  index_list->num_lines = line_count;
  index_list->cur_line = 0;

  return(index_list);

} /* end of create index_list */


/*********************************************************************/
/*  find_ptf_param: this function reads through the given index      */
/*      list and returns true if the given ptf is found in the list  */
/*********************************************************************/

int find_ptf_param(index_list_t *index_list, 
                   char *server, char *ptf_param)
{
  index_line_t *cur_line;
  int found;

  found = FALSE;
  rewind_index_list(index_list);
  while((!found)&&((cur_line=get_next_index_line(index_list)) != NULL))
  {  if(strcmp(cur_line->ptf1, ptf_param) == 0)
     {  found=TRUE;
        if(verbose)
          printf("parameter %s found on %s\n", ptf_param, server); 
     }
  }
  return(found);
} /* end of find_ptf_param */

  
/*********************************************************************/
/*  find_dependent_ptfs:  this function reads through the given      */
/*      index list and adds dependent ptfs it finds to the ptf_list. */
/*	It sets loop_finished=FALSE if any new dependent ptfs were   */
/*   	added to the ptf list.                                       */
/*********************************************************************/

void find_dependent_ptfs(index_list_t *index_list, char *server, 
                        ptf_list_t *ptf_list )
{
  index_line_t *cur_line;

  rewind_index_list(index_list);
  while((cur_line = get_next_index_line(index_list)) != NULL)   
  {  /* add the dependent ptf to the ptf list if they have the same  */
     /* lpp_name, the ptf on the right (ptf2) is in the list, and    */
     /* the ptf dependent on it (ptf1) is not in the list            */
     if(strcmp(cur_line->lpp1, cur_line->lpp2) == 0)
       if(in_ptf_list(ptf_list, cur_line->ptf2))
 	 if(!in_ptf_list(ptf_list, cur_line->ptf1))
         {  append_ptf_list(ptf_list, cur_line->ptf1, server);
	    loop_finished = FALSE;
          
            /* set return code for check mode */
            return_code=2;  /* return_code=2 on missing ptfs */

            printf("dependency_list: %s on %s has %s %s\n", 
             cur_line->ptf1,server, cur_line->req_type, cur_line->ptf2);
             
         } /* end of if(!in_ptf_list) */
  }
} /* end of find_dependent_ptfs */ 


/*********************************************************************/
/*  rewind_index_list: set pointer to beginning of index list        */
/*********************************************************************/

void rewind_index_list(index_list_t *index_list)
{
  index_list->cur_line = 0;
}


/*********************************************************************/
/*  get_next_index_line: return a pointer to the next line in index  */
/*	list; return NULL when at end of list	 		     */
/*********************************************************************/

index_line_t *get_next_index_line(index_list_t *index_list)
{
  index_line_t *return_ptr;

  if(index_list->cur_line == index_list->num_lines)
    return_ptr = NULL;
  else
    return_ptr = &index_list->index_line[index_list->cur_line++];

  return(return_ptr);
}


/*********************************************************************/ 
/*  create_ptf_list: this function creates and returns an empty      */
/*	ptf_list (linked list)           			     */ 
/*********************************************************************/

ptf_list_t *create_ptf_list()
{
  ptf_list_t *ptf_list;

  ptf_list = (ptf_list_t *) malloc(sizeof(ptf_list_t));
  check_malloc_return(ptf_list);
  ptf_list->start = NULL;
  ptf_list->last  = NULL;
  ptf_list->current=NULL;

  return(ptf_list);
}


/*********************************************************************/
/*  append_ptf_list:  This function appends the given ptf/server     */
/*	pair to the given ptf linked list			     */
/*********************************************************************/

void append_ptf_list(ptf_list_t *ptf_list, 
                     char *new_ptf, char *new_server)
{
  ptf_info_t *new_ptf_info;

  /* set up new info */
  new_ptf_info = (ptf_info_t *) malloc(sizeof(ptf_info_t));
  check_malloc_return(new_ptf_info);
  new_ptf_info->ptf = (char *) malloc(strlen(new_ptf)+1);
  check_malloc_return(new_ptf_info);
  strcpy(new_ptf_info->ptf,new_ptf);
  new_ptf_info->server = new_server;
  new_ptf_info->next_ptf = NULL;
 
  /* append to end of list */
  if(ptf_list->start == NULL)  /* is the list empty? */
  {  ptf_list->start = new_ptf_info;
     ptf_list->last  = new_ptf_info;
  }
  else
  {  ptf_list->last->next_ptf = new_ptf_info;
     ptf_list->last  = new_ptf_info;
  }
} /* end of append_ptf_list */
  

/*********************************************************************/
/*  in_ptf_list:  This function checks whether the given ptf is in   */
/*	the given ptf linked list and returns true or false.         */
/*********************************************************************/

int in_ptf_list(ptf_list_t *ptf_list, char *chk_ptf)
{
  ptf_info_t *cur_info;
  int found; 
    
  found=FALSE;
  rewind_ptf_list(ptf_list);
  while((!found)&&((cur_info = get_next_ptf_info(ptf_list)) != NULL))
  {  if(strcmp(cur_info->ptf, chk_ptf) == 0)
       found=TRUE;
  }
  return(found);

} /* end of in_ptf_list */


/*********************************************************************/
/*  output_server_ptfs: This function outputs the ptfs from the ptf  */
/*	list with the given server to the given output file.         */
/*********************************************************************/

void output_server_ptfs(FILE *out_file, ptf_list_t *ptf_list, 
                         			char *cur_server)
{
  ptf_info_t *cur_info;
    
  rewind_ptf_list(ptf_list);
  while((cur_info = get_next_ptf_info(ptf_list)) != NULL)
  {  if(strcmp(cur_info->server,cur_server) == 0)
        fprintf(out_file,"%s.ptf  %s\n",
                           cur_info->ptf, cur_info->server);
  }

} /* end of output_server_ptfs */


/*********************************************************************/
/*  rewind_ptf_list: set pointer to beginning of ptf list            */
/*********************************************************************/

void rewind_ptf_list(ptf_list_t *ptf_list)
{
  ptf_list->current = ptf_list->start;
}


/*********************************************************************/
/*  get_next_ptf_info: return next info in ptf list; return NULL     */
/*   	at end of list                                               */
/*********************************************************************/

ptf_info_t *get_next_ptf_info(ptf_list_t *ptf_list)
{
  ptf_info_t *return_ptr;

  return_ptr = ptf_list->current; 
  if(ptf_list->current != NULL)
    ptf_list->current = ptf_list->current->next_ptf;
  return(return_ptr);

} /* end of get_next_ptf_info */ 


/*********************************************************************/
/*  check_malloc_return: exit out if malloc failed ( ptr = null )    */
/*********************************************************************/

void check_malloc_return(char *ptr)
{
  if (ptr == NULL)
  {  printf("dependency_list: malloc was unable to allocate memory\n");
     printf("dependency_list: program terminated\n");
     exit(1);
  }
}


/*********************************************************************/
/*  help:  print syntax and exit				     */
/*********************************************************************/

void help()
{
 printf("\n");
 printf("%s\n",sccsid);
 printf("Usage\n");
 printf("dependency_list   [-b dirname_build]  [-p dirname_prod]   \n");
 printf("                  [-s dirname_ship] [-c] [-v] [-h] [-?]   \n");
 printf("                  [-o outfilename ] ptf_filenames \n");
 printf("        where:					\n");
 printf("             dirname_build - path to mif directory on\n");
 printf("                             'build' server.	\n");
 printf("             dirname_prod  - path to mif directory on	\n");
 printf("                            'production' server.	\n");
 printf("             dirname_ship  - path to mif directory on	\n");
 printf("                             'ship' server.	\n");
 printf("             -o outfilename - path & filename for list\n");
 printf("                              of dependencies\n");
 printf("             -v      verbose option - prints extra info\n");
 printf("             -c      check option - prints missing ptfs\n");
 printf("             ptf_name - ptffilename used on server.\n");
 printf("                        (ie. U0000257.ptf)	\n");
 printf("             -h | -?   options to list usage message \n");
 printf("\n");
 exit(1);
} /* end of help function */
 
/* END OF PROGRAM */
