static char sccsid[] = "@(#)00	1.3  src/bldenv/pkgtools/prereq_list.c, pkgtools, bos412, GOLDA411a 9/6/94 13:20:08";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: append_ptf_list
 *		check_for_prereq_ptfs
 *		check_malloc_return
 *		create_index_list
 *		create_ptf_list
 *		find_in_ptf_list
 *		find_more_prereq_ptfs
 *		get_next_index_line
 *		get_next_ptf_info
 *		help
 *		is_a_valid_ptf
 *		main
 *		report_errors
 *		rewind_index_list
 *		rewind_ptf_list
 *		store_server
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
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
	 char *ptf;        /* the ptf for this node */ 
	 char *server;     /* the server this ptf is found on */    
         char *parent_ptf; /* the ptf which needs (prereqs) this ptf */
         char *req_type;   /* relationship between ptfs (coreq,etc) */
	 struct ptf_info *next_ptf;
    };
typedef struct ptf_info  ptf_info_t;  /* ptf info type */

typedef struct {
	 ptf_info_t *start, *current, *last;
    } ptf_list_t;  /* ptf list type (linked list of ptf_info nodes) */

/** declare functions **/
index_list_t *create_index_list(char *server_dir);
void find_more_prereq_ptfs(index_list_t *index_list, char *server, 
                             ptf_list_t *ptf_list, int excludeSupers );
void check_for_prereq_ptfs(index_list_t *index_list, char *server, 
                             ptf_list_t *ptf_list );
void rewind_index_list(index_list_t *index_list);
index_line_t *get_next_index_line(index_list_t *index_list);
ptf_list_t *create_ptf_list();
void append_ptf_list(ptf_list_t *ptf_list, char *new_ptf, 
         char *new_server, char *new_parent_ptf, char *new_req_type);
ptf_info_t *find_in_ptf_list(ptf_list_t *ptf_list, char *chk_ptf);
void store_server(ptf_info_t *ptf_info, char *server);
void report_errors(ptf_list_t *ptf_list);
void rewind_ptf_list(ptf_list_t *ptf_list);
ptf_info_t *get_next_ptf_info(ptf_list_t *ptf_list);
void check_malloc_return(char *ptr);
int is_a_valid_ptf(char *ptf);
void help();

/** input parameter flags **/
int verbose = FALSE;
int check_mode = FALSE;

/** string constants **/
char build_server[] = "build server";
char prod_server[]  = "production server";
char ship_server[]  = "ship server";
char no_where[]  = "no where";
char command_line[]  = "command line";

/** argument file and directory names **/
char   *dir_build = NULL;
char   *dir_prod = NULL; 
char   *dir_ship = NULL;
  
/** Misc. vars  **/
int return_code = 0;
int loop_finished;


main(int argc, char *argv[])
{
  /* index file lists and ptf list */
  index_list_t *build_list, *prod_list, *ship_list;
  ptf_list_t *ptf_list;
  int excludeSupers=0;
  
  /*  getopt var's */
  extern char *optarg;
  extern int optind;
  int c;
  
  /* Misc. var's */
  int i;
  int err_code = 0;
  char *ptf_param;   /* used to point to input ptf params */
  char *period_ptr;  /* used to point to periods found in params */
  char *slash_ptr;  /* used to point to slashes found in params */
  

  /*** parse the command line ***/
  while((c = getopt(argc , argv, "xh?vcb:p:s:")) != EOF)
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
	 case 'x':excludeSupers++; break;
         default:err_code=1;
       } /* end of switch */


  /***  Check the parameters ***/

  if(dir_prod == NULL)
  { printf("prereq_list: -p production server parameter is required\n");
    err_code = 1;
  }

  if(dir_ship == NULL)
  { printf("prereq_list: -s ship server parameter is required\n");
    err_code = 1;
  }

  if(optind == argc)
  {  printf("prereq_list: one ptf_filename parameter is required\n");
     err_code = 1;
  }

  /*** strip off '.ptf' if supplied on input ptf_names	***/
  /*** and check if they comply with ptf_name format    ***/
  

  for( i = optind; i < argc ; i++)
  {  
     /* strip off path if supplied */
     if((slash_ptr = strrchr(ptf_param,'/')) != NULL)
        strcpy(ptf_param, (char *) slash_ptr + 1);

     /* strip off '.ptf' if supplied */
     ptf_param = argv[i];
     period_ptr = (char *) ptf_param + strlen(ptf_param) - 4;
     if(strcmp(period_ptr, ".ptf") == 0)
        *period_ptr = '\0';

     /* check if comply with ptf format */
     if(!is_a_valid_ptf(ptf_param))
     {  printf("prereq_list: %s is not a valid ptf\n",ptf_param);
        err_code = 1;
     }
  } /* end of for */
  

  /* Give help and exit if an error was found */
  if(err_code != 0)
     help(); 


  /*** For each server, create index file lists ***/

  if(dir_build != NULL)
     build_list = create_index_list(dir_build);
  prod_list = create_index_list(dir_prod);
  ship_list = create_index_list(dir_ship);
  

  /*** Create ptf_list and add ptf_filenames from the command line ***/

  ptf_list=create_ptf_list();
  for( i = optind; i < argc ; i++)
  {  ptf_param = argv[i];
     if(find_in_ptf_list(ptf_list, ptf_param) == NULL)
       append_ptf_list(ptf_list,ptf_param,no_where,command_line,"none");
     else
        printf("prereq_list: warning: %s given twice on command line\n",
                                                             ptf_param);
  } /* end of for */
 
 
  /*** Loop through the first index list adding prerequisite ptfs ***/
  /*** to the ptf_list until a pass is made in which no more      ***/
  /*** prerequisite ptfs are found.  'loop_finished'              ***/
  /*** will stay TRUE if no prereqs are added during a pass.      ***/

  loop_finished=FALSE;
  while(!loop_finished)
  {  loop_finished=TRUE;
     if(dir_build != NULL)
        find_more_prereq_ptfs(build_list,build_server,ptf_list, excludeSupers);
     else
        find_more_prereq_ptfs(prod_list,prod_server,ptf_list, excludeSupers);
  } /* end of while */


  /*** Pass once through servers further upstream to check if     ***/
  /*** the prerequisite ptfs found above are located upstream     ***/

  if(dir_build != NULL)
     check_for_prereq_ptfs(prod_list,prod_server,ptf_list);
  check_for_prereq_ptfs(ship_list,ship_server,ptf_list);
  
 
  /*** For each server given, free allocated index file list ***/

  if(dir_build != NULL)
     free(build_list);
  if(dir_prod != NULL)
     free(prod_list);
  if(dir_ship != NULL)
     free(ship_list);


  /*** Check for and report any errors with the ptfs ***/

  report_errors(ptf_list); 

 
  /*** clean up and exit ***/

  sync();
  if(check_mode)
    return(return_code);
  else
    return(0);
    

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
  {  printf("prereq_list: can't open: %s\n",in_name);
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
		printf("prereq_list: index file %s has bad data\n"
                                                             ,in_name);
                printf("line %d: '%s'\n",line_count+1, cur_line_ptr);
  	        printf("prereq_list: process terminated\n");
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
    {  	 printf("prereq_list: index %s has bad req type\n",in_name);
         printf("line %d: invalid type of '%s'\n",i+1, 
                     index_list->index_line[i].req_type);
         printf("prereq_list: process terminated\n");
         exit(1);
    }
  } /* end of for */

  index_list->num_lines = line_count;
  index_list->cur_line = 0;

  return(index_list);

} /* end of create index_list */


/*********************************************************************/
/*  find_more_prereq_ptfs:  this function reads through the given    */
/*      index list and adds prereq ptfs it finds to the ptf_list.    */
/*	It sets loop_finished=FALSE if any new prereq ptfs were      */
/*   	added to the ptf list.                                       */
/*  If excludeSupers flag is set, ignore supersede relationships     */
/*  when reading the index file.                                     */
/*********************************************************************/

void find_more_prereq_ptfs(index_list_t *index_list, char *server, 
                             ptf_list_t *ptf_list, int excludeSupers )
{
  index_line_t *cur_line;
  ptf_info_t *ptf_info;

  rewind_index_list(index_list);
  while((cur_line = get_next_index_line(index_list)) != NULL)   
  {
     if (excludeSupers && strstr ("supersede", cur_line->req_type) )
	 continue;
     /* add the prereq ptf to the ptf list if the ptf on the left  */
     /* (ptf1) is in the list, and its prereq on the right (ptf2)  */
     /* is not in the list and it is actually a valid ptf          */
     if((ptf_info = find_in_ptf_list(ptf_list, cur_line->ptf1)) != NULL)
     {  /* store that this ptf was found on this server */
        store_server(ptf_info, server);
 	
        if((is_a_valid_ptf(cur_line->ptf2))&&
           (find_in_ptf_list(ptf_list, cur_line->ptf2) == NULL))
        {  append_ptf_list(ptf_list, cur_line->ptf2, no_where,
                            cur_line->ptf1, cur_line->req_type);
	   loop_finished = FALSE;
          
           /* print out extra info if verbose mode */
           if(verbose)
              printf("%s is a %s of %s on %s\n", cur_line->ptf2,
                      cur_line->req_type, cur_line->ptf1, server);

        } 
     } 
  } 
} /* end of find_more_prereq_ptfs */ 


/*********************************************************************/
/*  check_for_prereq_ptfs:  this function reads through the given    */
/*      index list and if it finds a ptf that it is in the ptf_list, */
/*	it stores that it was found on this server.                  */
/*********************************************************************/

void check_for_prereq_ptfs(index_list_t *index_list, char *server, 
                             ptf_list_t *ptf_list )
{
  index_line_t *cur_line;
  ptf_info_t *ptf_info;

  rewind_index_list(index_list);
  while((cur_line = get_next_index_line(index_list)) != NULL)   
     if((ptf_info = find_in_ptf_list(ptf_list, cur_line->ptf1)) != NULL)
     {  /* store that this ptf was found on this server */
        store_server(ptf_info, server);
     } 
} /* end of check_for_prereq_ptfs */ 


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
/*  append_ptf_list:  This function appends the given ptf and its    */
/*	info to the given ptf linked list			     */
/*********************************************************************/

void append_ptf_list(ptf_list_t *ptf_list, char *new_ptf, 
         char *new_server, char *new_parent_ptf, char *new_req_type)
{
  ptf_info_t *new_ptf_info;

  /* get memory space for new info */
  new_ptf_info = (ptf_info_t *) malloc(sizeof(ptf_info_t));
  check_malloc_return(new_ptf_info);
  new_ptf_info->ptf = (char *) malloc(strlen(new_ptf)+1);
  check_malloc_return(new_ptf_info->ptf);
  new_ptf_info->parent_ptf = (char *) malloc(strlen(new_parent_ptf)+1);
  check_malloc_return(new_ptf_info->parent_ptf);
  new_ptf_info->req_type = (char *) malloc(strlen(new_req_type)+1);
  check_malloc_return(new_ptf_info->req_type);

  /* set pointers to point to new info */
  strcpy(new_ptf_info->ptf,new_ptf);
  new_ptf_info->server = new_server;
  strcpy(new_ptf_info->parent_ptf,new_parent_ptf);
  strcpy(new_ptf_info->req_type,new_req_type);
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
/*  find_in_ptf_list: This function checks whether the given ptf is  */
/*	in the given ptf linked list.  It returns either the ptf_node*/
/*      if found and otherwise NULL.                                 */
/*********************************************************************/

ptf_info_t *find_in_ptf_list(ptf_list_t *ptf_list, char *chk_ptf)
{
  int found; 
  ptf_info_t *cur_info;
    
  found=FALSE;
  rewind_ptf_list(ptf_list);
  while((!found)&&((cur_info = get_next_ptf_info(ptf_list)) != NULL))
     if(strcmp(cur_info->ptf, chk_ptf) == 0)
        found=TRUE;
  
  return(cur_info);

} /* end of in_ptf_list */


/*********************************************************************/
/*  store_server: This function stores the server that the given ptf */
/*      was found on.           				     */
/*********************************************************************/

void store_server(ptf_info_t *ptf_info, char *server)
{
  /* check if ptf has already been found */
  if(ptf_info->server == no_where)
  {  ptf_info->server = server; 
     if(verbose)
       printf("%s found on %s\n", ptf_info->ptf, server);
  }
  else
     /* check if found already on a different server */
     if(ptf_info->server != server)
     { printf("prereq_list: %s was found on both the %s and %s\n",
                         ptf_info->ptf, ptf_info->server, server);
       return_code = 1;
     }
 	

} /* end of store_server */


/*********************************************************************/
/*  report_errors: This function reads through the ptf_list          */
/*      checking for and reporting any errors found and setting an   */
/*      appropriate return code.                                     */
/*********************************************************************/

void report_errors(ptf_list_t *ptf_list)
{
  ptf_info_t *cur_info;
  char *first_server; /* the server that ptfs are being promoted from */
  int parm_error;    /* true if input parm wasn't in the first server*/
  int promote_error; /* true if any ptfs that need be promoted */
                     /*   were not specified on the command line */ 
  int missing_error; /* true if a ptf was not found anywhere */

  /* set up the server ptfs are being promoted from */
  if(dir_build != NULL)
    first_server = build_server;
  else
    first_server = prod_server;


  /*** check for the various errors ***/ 

  parm_error=promote_error=missing_error=FALSE;
  rewind_ptf_list(ptf_list);
  while((cur_info = get_next_ptf_info(ptf_list)) != NULL)
  {  /* check that input parms were found in the first server */
     if((strcmp(cur_info->parent_ptf, command_line) == 0)&&
        (strcmp(cur_info->server, first_server) != 0))
     { parm_error = TRUE; 
       return_code = 1;
     }
     /* check that other ptfs were not found in the first server */
     else if((strcmp(cur_info->parent_ptf, command_line) != 0)&&
             (strcmp(cur_info->server, first_server) == 0))
     { promote_error = TRUE; 
       return_code = 1;
     }
     /* check that the ptfs were found somewhere */
     else if(strcmp(cur_info->server, no_where) == 0)
     { missing_error = TRUE; 
       return_code = 1;
     }
  } /* end of while */


  /*** report all errors that were found above ***/

  if(return_code == 0)
  {  if(!check_mode)
       printf("\nprereq_list: promote would have been successful\n");
  }
  else
  {
     if(check_mode)
       printf("\nprereq_list: promote failed\n");
     else
       printf("\nprereq_list: promote would have failed\n");

     if(parm_error)
     {  printf("\nThe following command line ptf(s) were not in the index file");
        printf("\non the %s and thus can't be promoted:\n", first_server);

        rewind_ptf_list(ptf_list);
        while((cur_info = get_next_ptf_info(ptf_list)) != NULL)
        {  if((strcmp(cur_info->parent_ptf, command_line) == 0)&&
              (strcmp(cur_info->server, first_server) != 0))
              {  if(strcmp(cur_info->server, no_where) == 0)
                   printf("  %s  ( not found anywhere )\n", 
                                                   cur_info->ptf);
             	 else
                   printf("  %s  ( found on %s )\n", cur_info->ptf,  
                                               cur_info->server); 
              }
        }
     }
     
     if(promote_error)
     {  printf("\nThe following ptf(s) have to be promoted along with");
        printf(" the given ptfs:\n");

        rewind_ptf_list(ptf_list);
        while((cur_info = get_next_ptf_info(ptf_list)) != NULL)
        {  if((strcmp(cur_info->parent_ptf, command_line) != 0)&&
              (strcmp(cur_info->server, first_server) == 0))
              printf("  %s  ( %s of %s )\n", cur_info->ptf,  
                           cur_info->req_type, cur_info->parent_ptf); 
        }
     }

     if(missing_error)
     {  printf("\nThe following ptf(s) were not found anywhere:\n");

        rewind_ptf_list(ptf_list);
        while((cur_info = get_next_ptf_info(ptf_list)) != NULL)
        {  if((strcmp(cur_info->server, no_where) == 0)&&
              (strcmp(cur_info->parent_ptf,command_line) != 0))
              printf("  %s  ( %s of %s )\n", cur_info->ptf,  
                           cur_info->req_type, cur_info->parent_ptf); 
        }
     }
  } /* end of else ( on return_code == 0 ) */
} /* end of report_errors */


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
  {  printf("prereq_list: malloc was unable to allocate memory\n");
     printf("prereq_list: program terminated\n");
     exit(1);
  }
}


/*********************************************************************/
/*  is_a_valid_ptf: checks if last 5 chars are digits                */
/*                  returns TRUE or FALSE         		     */
/*********************************************************************/

int is_a_valid_ptf(char *ptf)         
{
   if(strspn(ptf + strlen(ptf) - 5, "1234567890") == 5)
     return(TRUE);
   else
     return(FALSE);
}


/*********************************************************************/
/*  help:  print syntax and exit				     */
/*********************************************************************/

void help()
{
 printf("\n");
 printf("%s\n",sccsid);
 printf("Usage:\n");
 printf("prereq_list    -s dirname_ship  -p dirname_prod\n");
 printf("              [-b dirname_build] [-v] [-c] [-h] [-?]\n");
 printf("               ptf_filename [ptf_filename] ....\n");
 printf("      where:\n");
 printf("              dirname_ship  - path to mif directory on\n");
 printf("                              'ship' server.\n");
 printf("              dirname_prod  - path to mif directory on\n");
 printf("                              'production' server.	\n");
 printf("              dirname_build - path to mif directory on\n");
 printf("                              'build' server.\n");
 printf("              -v         verbose option - prints extra info\n");
 printf("                         on the prereqs found  \n");
 printf("              -c         check option - gives a return code\n");
 printf("                         indicating success if no ptfs were\n");
 printf("                         missing or failure if not all ptfs\n");
 printf("                         were specified on command line.\n");
 printf("              ptf_filename - ptf_filename used on server.\n");
 printf("                           (ie. U0000257.ptf)\n");
 printf("                         This will allow execution from \n");
 printf("                           Motif or EMACS.\n");
 printf("              -h | -?   options to list usage message \n");
 printf("\n");
 exit(1);
} /* end of help function */
 
/* END OF PROGRAM */
