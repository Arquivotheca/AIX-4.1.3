static char sccsid[] = "@(#)99  1.16  src/bldenv/pkgtools/media_list.c, pkgtools, bos412, GOLDA411a 3/2/94 11:40:41";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: append_mlevel_list
 *		append_peresl_list
 *		append_ptf_list
 *		append_req_list
 *		check_malloc_return
 *		create_index_list
 *		create_mlevel_list
 *		create_peresl_list
 *		create_ptf_list
 *		create_req_list
 *		find_in_mlevel_list
 *		find_in_peresl_list
 *		find_in_ptf_list
 *		find_in_req_list
 *		find_more_prereq_ptfs
 *		find_peresl_ptfs
 *		get_next_index_line
 *		get_next_mlevel_info
 *		get_next_peresl_info
 *		get_next_ptf_info
 *		get_next_req_info
 *		help
 *		is_a_valid_ptf
 *		main
 *		mlevel_list_empty
 *		output_ptfs
 *		peresl_list_empty
 *		print_out_cyclic_ptfs
 *		print_out_ptf
 *		ptf_list_empty
 *		remove_coreqs_and_ifreqs
 *		remove_ptf_from_ptf_list
 *		remove_ptf_from_req_list
 *		report_ptfs_not_found
 *		req_list_empty
 *		restore_current_posn
 *		rewind_index_list
 *		rewind_mlevel_list
 *		rewind_peresl_list
 *		rewind_ptf_list
 *		rewind_req_list
 *		save_current_posn
 *		store_server_and_req
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   Syntax:                                                           
 *   media_list    [-s dirname_ship] [-p dirname_prod]          
 *                 [-b dirname_build] [-v] [-t] [-e] [-h] [-?] [-x] 
 *                 [-p] [-o out_filename ] [-r peresl_filename]
 *                 [-u maintlevel_filename] -l mif_ccss_list_filename
 *   where:                                               
 *         -s dirname_ship  - path to mif directory on           
 *                            'ship' server.                     
 *         -p dirname_prod  - path to mif directory on           
 *                            'production' server.                
 *         -b dirname_build - path to mif directory on           
 *                            'build' server.                    
 *         -v         verbose option - prints extra infomation
 *                    on the prereqs found                    
 *         -l mif_ccss_list_filename filename containing list 
 *                                   of mif/ccss files to package.   
 *                                   (for example filename 3.2_gold: 
 *                                    u123456                
 *                                    u333333                
 *                                    u444444)   
 *         -o outfilename   - path & filename for ordered list
 *                            of prereqs.  sysout used if     
 *                            -o is not given. If rc!=0 no    
 *                            data is output to this file.   
 *                            This is the order that the      
 *                            ptf's could be stacked on a     
 *                            tape without ever needing to    
 *                            rewind the device on a install  
 *                            all option. (example output     
 *                            file /tmp/order.list:           
 *                            u444444                         
 *                            u123456                         
 *                            u333333 ).                      
 *         -r peresl_file   - Problem resolution file.
 *	   -u maintlevel_file - File containing all ptfs in specified
 *			      maintenance level plus all ptfs in any earlier 
 *			      maintenance levels.  These ptfs are excluded from
 *                            final output.
 *         -e  - exclude requisites; only ptfs from -l file   
 *               are printed out (in nice order)             
 *         -t  - duplicate PTF's are allowed on different     
 *               servers.                                    
 *         -x  - exclude supersedes; only prereqs, coreqs and 
 *               ifreqs ptf_filenames are printed out.       
 *         -n  - do NOT include preventive supersedes.
 *         -h | -?   options to list usage message and exit   
 */
/* Include section */
#include <stdio.h>
#include <fcntl.h>

/* Set up name of index files */
#define INDEX "index"

/*********************/
/* Data declarations */
/*********************/

/* Set up an array of index file lines */
/* *index_line will essentially become index_line[num_lines] by using malloc */
/* Index file line */
typedef struct {
            char *lpp1,
                 *ptf1,
                 *req_type,
                 *lpp2,
                 *ptf2;
               } index_line_t;

/* Array of lines */   /* index list type */
typedef struct {
    index_line_t *index_line;
             int num_lines,
                 cur_line;
               } index_list_t;

/******************************************/
/*         ptf_list structure             */
/*                                        */
/*    ----------    -----------           */
/*    |  ptf1  |    |  ptf2   |           */
/*    |        |--> |         |----|||    */
/*    |req_list|    | req_list|           */
/*    ----|-----    -----|-----           */
/*        V              V                */
/*    ----------    -----------           */
/*    | ptf1's |    | ptf2's  |           */
/*    |  req's |    | req's   |           */
/*    ----------    -----------           */
/******************************************/

/* Requisite list structure (linked list) */
/* req_info node */
struct req_info {
             char *req_ptf;  /* the req ptf for this node */
             char *req_type; /* relationship for this ptf */
  struct req_info *next_req; /* pointer to next req in list  */
                };

typedef struct req_info  req_info_t;  /* req info type */

/* req_list_type (linked list of req_info nodes) */
typedef struct {
        req_info_t *start,
                   *current,
                   *last;
               } req_list_t;


/* ptf list structure (linked list) */
struct ptf_info {   /* ptf info node */
        char *ptf;                      /* ptf for this node */
        char *server;                   /* Server this ptf is found on */
        char *parent_ptf;               /* ptf which prereqs this ptf */
        char *req_type;                 /* Relationship from parent ptf */
        req_list_t *req_list;           /* List of requisites for this ptf */
        struct ptf_info *next_ptf;      /* Pointer to next ptf in list */
                };

/* ptf info type */
typedef struct ptf_info  ptf_info_t;

/* ptf list type (linked list of ptf_info nodes) */
typedef struct {
        ptf_info_t *start,
                   *current,
                   *last;
               } ptf_list_t;


/* peresl list structure (linked list) */
struct peresl_info {   /* peresl info node */
        char *bad_ptf;                  /* ptf for this node */
        req_list_t *req_list;           /* List of fixes for this ptf */
        struct peresl_info *next_peresl;/* Pointer to next peresl in list */
                };
                      
/* peresl info type */
typedef struct peresl_info  peresl_info_t;

/* peresl list type (linked list of peresl_info nodes) */
typedef struct {
        peresl_info_t *start,
                   *current,
                   *last;
               } peresl_list_t;

/* maintlevel list structure (linked list) */
struct mlevel_info {   		/* mlevel info node */
        char *ptf;                  /* maintlevel ptf for this node */
        struct mlevel_info *next_mlevel;/* Pointer to next ptf in list */
                };
                      
/* mlevel info type */
typedef struct mlevel_info  mlevel_info_t;

/* mlevel list type (linked list of mlevel_info nodes) */
typedef struct {
        mlevel_info_t *start,
                   *current,
                   *last;
               } mlevel_list_t;

/*************************/
/* Function declarations */
/*************************/

void find_more_prereq_ptfs(index_list_t *index_list, char *server,
                             ptf_list_t *ptf_list );
int store_server_and_req(ptf_info_t *ptf_info, char *server,
                           char *req_ptf, char *req_type);
void report_ptfs_not_found(ptf_list_t *ptf_list);
void remove_coreqs_and_ifreqs(ptf_list_t *ptf_list);
void output_ptfs(FILE *out_file, ptf_list_t *ptf_list);
void print_out_ptf(FILE *out_file, ptf_info_t *ptf_info);
void remove_ptf_from_ptf_list(ptf_list_t *ptf_list,char *ptf_to_remove);
void remove_ptf_from_req_list(req_list_t *req_list,char *ptf_to_remove);
void print_out_cyclic_ptfs(ptf_list_t *ptf_list);
index_list_t *create_index_list(char *server_dir);
void rewind_index_list(index_list_t *index_list);
index_line_t *get_next_index_line(index_list_t *index_list);
ptf_list_t *create_ptf_list();
void append_ptf_list(ptf_list_t *ptf_list, char *new_ptf,
         char *new_server, char *new_parent_ptf, char *new_req_type);
ptf_info_t *find_in_ptf_list(ptf_list_t *ptf_list, char *chk_ptf);
void rewind_ptf_list(ptf_list_t *ptf_list);
ptf_info_t *get_next_ptf_info(ptf_list_t *ptf_list);
int ptf_list_empty(ptf_list_t *ptf_list);
req_list_t *create_req_list();
void append_req_list(req_list_t *req_list, char *new_req_ptf,
                                           char *new_req_type);
req_info_t *find_in_req_list(req_list_t *req_list, char *chk_req_ptf);
void rewind_req_list(req_list_t *req_list);
req_info_t *get_next_req_info(req_list_t *req_list);
int req_list_empty(req_list_t *req_list);
void save_current_posn(ptf_list_t *ptf_list,req_list_t *req_list);
void restore_current_posn(ptf_list_t *ptf_list,req_list_t *req_list);
void check_malloc_return(char *ptr);
int is_a_valid_ptf(char *ptf);
void help();
peresl_list_t *create_peresl_list(char *peresl_file);
void append_peresl_list(peresl_list_t *peresl_list, char *bad_ptf,
         char *new_peresl_ptf);
peresl_info_t *find_in_peresl_list(peresl_list_t *peresl_list, char *chk_ptf);
void rewind_peresl_list(peresl_list_t *peresl_list);
peresl_info_t *get_next_peresl_info(peresl_list_t *peresl_list);
int peresl_list_empty(peresl_list_t *peresl_list);
void find_peresl_ptfs(peresl_info_t *peresl_list, ptf_list_t *ptf_list );
mlevel_list_t *create_mlevel_list(char *mlevel_file);
void append_mlevel_list(mlevel_list_t *mlevel_list, char *ptf);
int find_in_mlevel_list(char *ptf);
mlevel_info_t *get_next_mlevel_info(mlevel_list_t *mlevel_list);
void rewind_mlevel_list(mlevel_list_t *mlevel_list);
int mlevel_list_empty(mlevel_list_t *mlevel_list);

/************************/
/* Initialize variables */
/************************/
int verbose = FALSE;
int exclude_mode = FALSE;
int exclude_supersede_mode = FALSE;
int exclude_preventative_mode = FALSE;
char build_server[] = "build server";
char prod_server[]  = "production server";
char ship_server[]  = "ship server";
char no_where[] = "no where";
char input_file[]  = "input file";
char usurper[]  = "prvsup";
char super[]  = "supersede";
char peresl[]  = "pe-resl";
char *dir_build = NULL;
char *dir_prod = NULL;
char *dir_ship = NULL;
char *peresl_file = NULL;
char *mlevel_file = NULL;
char *out_name = NULL;
char *ptf_name = NULL;
int return_code = 0;
int loop_finished;
int coreq_loop_finished;
int duplicate_ptf = FALSE;

/** var's to save ptrs **/
ptf_info_t *save_ptf_list_current;
req_info_t *save_req_list_current;

mlevel_list_t *mlevel_list;

/****************/
/* Main program */
/****************/
main(int argc, char *argv[]) {

  /* Files */
  FILE *out_file;
  FILE *ptf_file;

  /* Index lists that include ptfs found in each directory */
  index_list_t *build_list, *prod_list, *ship_list;

  /* ptf list of all ptfs found */
  ptf_list_t *ptf_list;

  peresl_list_t *peresl_list;

  /*  getopt variables */
  extern char *optarg;
  extern int optind;
  int c;

  /* Misc. var's */
  int err_code = 0;
  char ptf_param[100];
  char *period_ptr;
  char *slash_ptr;
  int ptfs_found;

  /* Parse the command line */
  while((c = getopt(argc , argv, "h?xtevnl:o:b:p:r:s:u:")) != EOF)
    switch(c) {
      case 'h': help();
      case '?': if(strcmp(argv[optind - 1],"-?") == 0)
                    help();        /* '-?' was actually entered */
                  else
                    err_code = 1;  /* an illegal option was entered */
                  break;
      case 'v': verbose = TRUE;break;
      case 'e': exclude_mode = TRUE;break;
      case 'x': exclude_supersede_mode = TRUE;break;
      case 'l': ptf_name = optarg;break;
      case 'o': out_name = optarg;break;
      case 'b': dir_build = optarg;break;
      case 'p': dir_prod = optarg;break;
      case 'r': peresl_file = optarg;break;
      case 's': dir_ship = optarg;break;
      case 't': duplicate_ptf = TRUE; break;
      case 'n': exclude_preventative_mode = TRUE;break;
      case 'u': mlevel_file = optarg;break;
      default : err_code=1;
    }


  /********************/
  /* Validate options */
  /********************/
  /* Check for at least one server */
  if((dir_build == NULL)&&(dir_prod == NULL)&&(dir_ship == NULL)) {
    printf("media_list: at least one server parameter is required\n");
    err_code = 1;
  }

  /* Check ptf's in the -l input file */
  if(ptf_name == NULL) {
    printf("media_list: -l ptf file_name parameter is required\n");
    err_code = 1;
  } else {
    ptf_file = fopen(ptf_name, "r");
    if(ptf_file == NULL) {
      printf("media_list: can't open ptf_filename: %s\n",ptf_name);
      err_code = 1;
    } else {
      /* read ptfs from the ptf_file and add to ptf_list */
      ptf_list=create_ptf_list();
      ptfs_found=FALSE;
      while(fscanf(ptf_file,"%s",ptf_param) != EOF) {
        ptfs_found=TRUE;

        /* strip off path if supplied */
        if((slash_ptr = strrchr(ptf_param,'/')) != NULL)
          strcpy(ptf_param, (char *) slash_ptr + 1);

        /* strip off '.ptf' if supplied */
        if((period_ptr = strrchr(ptf_param,'.')) != NULL)
          if(strcmp(period_ptr, ".ptf") == 0)
            *period_ptr = '\0';

        /* check if comply with ptf format */
        if(!is_a_valid_ptf(ptf_param)) {
          printf("media_list: %s is not a valid ptf.\n",ptf_param);
          printf("            Valid ptf names must be 7 characters in\n");
          printf("             length with the last 5 numeric.\n");
          err_code = 1;
        } else {
          /* append to ptf list if not already there */
          if(find_in_ptf_list(ptf_list, ptf_param) == NULL) {
            append_ptf_list(ptf_list,ptf_param,no_where,input_file,"none");
            /* print out extra info if verbose mode */
            if(verbose)
              printf("%s read from ptf input file\n",ptf_param);
          } else {
            printf("media_list:warning: input ptf %s",ptf_param);
            printf(" specified twice\n");
          }
        }
      }

      if(!ptfs_found) {
        printf("media_list: input ptf_file must contain a ptf\n");
        err_code = 1;
      }
    }
    fclose(ptf_file);
  }

  /* Verify that the output file is accessible */
  if(out_name == NULL)
     out_file=stdout;
  else {
    out_file = fopen(out_name, "w");
    if(out_file == NULL) {
      printf("media_list: can't open output: %s\n",out_name);
      err_code = 1;
    }
  }

  /* Give help and exit if an error was found */
  if(err_code != 0)
     help();
  
  /* For each server given, create index file lists */

  if(dir_build != NULL)
     build_list = create_index_list(dir_build);
  if(dir_prod != NULL)
     prod_list = create_index_list(dir_prod);
  if(dir_ship != NULL)
     ship_list = create_index_list(dir_ship);
  if(peresl_file != NULL)
     peresl_list = create_peresl_list(peresl_file);

  mlevel_list = create_mlevel_list(mlevel_file);

  /* Loop through the index lists adding prerequisite ptfs */
  /* to the ptf_list until a pass is made in which no more */
  /* prerequisite ptfs are found.  'loop_finished'         */
  /* will stay TRUE if no prereqs are added during a pass. */

  loop_finished=FALSE;
  while(!loop_finished) {
    loop_finished=TRUE;
    if(dir_build != NULL)
      find_more_prereq_ptfs(build_list,build_server,ptf_list);
    if(dir_prod != NULL)
      find_more_prereq_ptfs(prod_list,prod_server,ptf_list);
    if(dir_ship != NULL)
      find_more_prereq_ptfs(ship_list,ship_server,ptf_list);
    if(peresl_file != NULL)
      find_peresl_ptfs(peresl_list,ptf_list);
  }


  /* Check for and report any errors with ptfs that weren't found */
 report_ptfs_not_found(ptf_list); 

  /* Remove coreqs and ifreqs from the requisite lists */
  remove_coreqs_and_ifreqs(ptf_list);
     
  /* Output the list of ptfs in the correct order for the tape */
  output_ptfs(out_file, ptf_list); 

  /* For each server given, free index file lists */
  if(dir_build != NULL)
     free(build_list);
  if(dir_prod != NULL)
     free(prod_list);
  if(dir_ship != NULL)
     free(ship_list);

  /* Clean up and exit */
  sync();
  return(return_code);

} /* End of Main */


/*********************************************************************/

/*      index list and adds prereq ptfs it finds to the ptf_list.    */
/*      It sets loop_finished=FALSE if any new prereq ptfs were      */
/*      added to the ptf list.                                       */
/*********************************************************************/
/*                                                                   */
/*   ----------                  ----------    -----------           */
/*   |  ptf1  |                  |  ptf1  |    |  ptf2   |           */
/*   |server: |                  |server: |    |server:  |           */
/*   | nowhere|----|||           | cur ser|--> | nowhere |----|||    */
/*   |parent: |                  |parent: |    |parent:  |           */
/*   | in_file|                  | in_file|    | ptf1    |           */
/*   |req:    |                  |req:    |    |req:     |           */
/*   | none   |                  | none   |    | coreq   |           */
/*   |req_list|                  |req_list|    | req_list|           */
/*   ----|-----        becomes   ----|-----    -----|-----           */
/*       =                           V              =                */
/*       =                       ----------         =                */
/*                               |  ptf2  |                          */
/*                               |  coreq |                          */
/*                               ----|-----                          */
/*                                   =                               */
/*                                   =                               */
/*                                                                   */
/*       where the index line is   lpp:ptf1:coreq:lpp:ptf2           */
/*********************************************************************/

void find_more_prereq_ptfs(index_list_t *index_list,
                                   char *server,
                             ptf_list_t *ptf_list ) {
  index_line_t *cur_line;
  ptf_info_t *ptf_info;
  ptf_info_t *loc_in_ptf_list;
  int addFlag;

  /* For each line in the index file */
  rewind_index_list(index_list);
  while((cur_line = get_next_index_line(index_list)) != NULL) {
    /* Add the prereq ptf to the ptf list if the ptf on the left  */
    /* (ptf1) is in the list, and its prereq on the right (ptf2)  */
    /* is not in the list and it is actually a valid ptf          */
    if((ptf_info = find_in_ptf_list(ptf_list, cur_line->ptf1)) != NULL) {
      /* store the server and the requisite found for this ptf */
      addFlag=store_server_and_req(ptf_info,server,cur_line->ptf2,cur_line->req_type);
      if (addFlag) {
        if(is_a_valid_ptf(cur_line->ptf2)) {
          if (find_in_ptf_list(ptf_list, cur_line->ptf2) == NULL) {
            append_ptf_list(ptf_list, cur_line->ptf2, no_where,
                          cur_line->ptf1, cur_line->req_type);
            loop_finished = FALSE;
          }
        };
      }
      else
      /* If exclude_supersede mode is required and the req_type in the  */
      /* index line is "supersede",  change the first character of the  */
      /* ptf2's req_type to 's' when ptf2 has already been in the ptf list */
        if ((exclude_supersede_mode) && ( *(cur_line->req_type) ==  's' )){
          if ((loc_in_ptf_list = find_in_ptf_list(ptf_list, cur_line->ptf2)) != NULL)
              *(loc_in_ptf_list->req_type) = 's' ;
        };
    }
    else {
        /*  If -n was not specified and ptf1 supersedes a ptf in the list,*/
        /*  add it to the list.   -- preventative supersedes --           */
        if(((! exclude_preventative_mode) && ( *(cur_line->req_type) == 's')) &&
            (ptf_info = find_in_ptf_list(ptf_list, cur_line->ptf2)) != NULL) {
              if(is_a_valid_ptf(cur_line->ptf1)) {
                  append_ptf_list(ptf_list, cur_line->ptf1, no_where,
                                cur_line->ptf2, usurper);
                  loop_finished = FALSE;
              }
        }
    };
  };
}; /* end of find_more_prereq_ptfs */


/*********************************************************************/
/*  store_server_and_req: This function stores the server that the   */
/*      given ptf was found on and also stores the requisite that    */
/*      that was found.                                              */
/*********************************************************************/

int store_server_and_req(ptf_info_t *ptf_info,
                                char *server,
                                char *req_ptf,
                                char *req_type){
  int addFlag = 0;
  /* Check if ptf has already been found */
  if(strcmp(ptf_info->server,no_where)== 0) {
    ptf_info->server = server;
    addFlag = 1;
    if(verbose)
      printf("%s found on %s\n", ptf_info->ptf, server);
  } 
  else
     /* check if found already on a different server */
     if(strcmp(ptf_info->server,server) != 0) {
       if ( !duplicate_ptf ) {
         printf("media_list: %s was found on both the %s and %s\n",
                ptf_info->ptf, ptf_info->server, server);
         return_code = 1;
       }
       else
	 if (strcmp(server,build_server)==0) {
           ptf_info->server = server;
           addFlag = 1;
         }
     }
     else
        addFlag = 1;

  /* if requisite ptf is in the maintlevel list, ignore it */
  if(find_in_mlevel_list(req_ptf))
    addFlag = 0;

  if(addFlag) {
   if (strcmp(req_type, super) == 0)
    addFlag = 0;
   else {
      /* append requisite information if not already in requisite list*/
      if((is_a_valid_ptf(req_ptf))&&
         (find_in_req_list(ptf_info->req_list,req_ptf) == NULL)&& 
          ((strcmp(req_type, "prereq") == 0) && 
           (strcmp(ptf_info->ptf, req_ptf) != 0))){
        append_req_list(ptf_info->req_list,req_ptf,req_type);
       if(verbose)
          printf("%s is a %s of %s\n", req_ptf, req_type, ptf_info->ptf);
     }
   }
  }
  return (addFlag);
} /* end of store_server_and_req */


/*********************************************************************/
/*  report_ptfs_not_found: This function reads through the ptf_list  */
/*      checking for and reporting any ptfs were not found anywhere  */
/*      in the index files.  The program is exited if errors found.  */
/*********************************************************************/

void report_ptfs_not_found(ptf_list_t *ptf_list) {
  ptf_info_t *cur_info;

  /* for each ptf */
  rewind_ptf_list(ptf_list);
  while((cur_info = get_next_ptf_info(ptf_list)) != NULL) {
    /* check that the ptf was found somewhere */
    if(strcmp(cur_info->server, no_where) == 0) {
      /* check if ptf is from the input file */
      if(strcmp(cur_info->parent_ptf, input_file) == 0) {
        return_code=99;
        printf("media_list: ERROR: %s from stack list  file ",cur_info->ptf);
        printf("was not found in index files\n");
      } else {
        if (strcmp(cur_info->req_type, "supersede") == 0) {
           printf("media_list: WARNING: %s ( superseded PTF of %s ) ",
                   cur_info->ptf, cur_info->parent_ptf);
        }
        else {
           printf("media_list: WARNING: %s ( %s PTF of %s ) ",cur_info->ptf,
                   cur_info->req_type, cur_info->parent_ptf);
        }
        printf("was not found in index files\n");
      }
    }
  }

  if(return_code != 0) {
     printf("\nmedia_list: process terminated\n\n");
     exit(return_code);
  }
} /* end of report_pts_not_found */

/*********************************************************************/
/*  remove_coreqs_and_ifreqs: remove all coreqs and ifreqs found     */
/*           in the respective req_lists.                            */
/*********************************************************************/

void remove_coreqs_and_ifreqs(ptf_list_t *ptf_list) {
  ptf_info_t *ptf_info;
  req_info_t *prev_req_info;
  req_info_t *req_info;

  /* For each ptf in the ptf list */
  rewind_ptf_list(ptf_list);
  while((ptf_info = get_next_ptf_info(ptf_list)) != NULL) {
    /* for each requisite of the ptf */
    prev_req_info = NULL;
    rewind_req_list(ptf_info->req_list);
    while((req_info=get_next_req_info(ptf_info->req_list)) != NULL) {
      /* check if it's a coreq or ifreq */
      if((strcmp(req_info->req_type, "coreq") == 0) ||
         (strcmp(req_info->req_type, "ifreq") == 0)) {
        /* remove the coreq or ifreq */
        if(ptf_info->req_list->start == req_info)
          ptf_info->req_list->start = req_info->next_req;
        else
          prev_req_info->next_req = req_info->next_req;

        if(ptf_info->req_list->last == req_info)
          ptf_info->req_list->last = prev_req_info;
      } else
        prev_req_info = req_info;
    }
  }
} /* end of remove_coreqs_and_ifreqs */


/*********************************************************************/
/*  output_ptfs: read through ptf_list printing out in a nice order  */
/*      for a future install (i.e. a ptf is printed out only after   */
/*      all of its requisites have been printed)                     */
/*********************************************************************/

void output_ptfs(FILE *out_file,
           ptf_list_t *ptf_list) {
  ptf_info_t *ptf_info;
  req_info_t *prev_req_info;
  req_info_t *req_info;
  int ptf_outputted;

  /* While ptfs are left to be outputted */
  while(!ptf_list_empty(ptf_list)) {
    ptf_outputted=FALSE;
    /* For each ptf */
    /* Print out ptf if all of its requisites are already printed */
    rewind_ptf_list(ptf_list);
    while((ptf_info = get_next_ptf_info(ptf_list)) != NULL) {
      if(req_list_empty(ptf_info->req_list)) {
        ptf_outputted=TRUE;
        print_out_ptf(out_file,ptf_info);
        save_current_posn(ptf_list,ptf_info->req_list);
        remove_ptf_from_ptf_list(ptf_list, ptf_info->ptf);
        restore_current_posn(ptf_list,ptf_info->req_list);
      }
    }

    /* If no ptfs were outputted during a pass, every ptf must    */
    /* at least one requisite remaining.  Thus, there must be a   */
    /* cycle among the relationships in these remaining ptfs      */
    if(!ptf_outputted) {
      print_out_cyclic_ptfs(ptf_list);
      exit(1);
    }
  }
} /* end of output_ptfs */


/*********************************************************************/
/*  print_out_ptf: print out the given ptf to the output file with   */
/*      full file path.  Only print input file ptfs if -e exclude    */
/*      mode is activated.                                           */
/*********************************************************************/

void print_out_ptf(FILE *out_file, ptf_info_t *ptf_info) {
  char *dir_out;
  char *possible_slash;

  if((exclude_mode)&&(strcmp(ptf_info->parent_ptf,input_file) != 0))
    /* do not print */ ;
  else {
    /* If the first character of the ptf's req_type is 's', it indicates */
    /* that this ptf is a supersede of some ptf.  Cases of this ptf's    */
    /* req_type:                                                         */
    /*   "supersede": this ptf is a supersede of its parent.             */
    /*   "sfreq",                                                        */
    /*   "soreq",                                                        */
    /*   "srereq": this ptf is a ifreq, coreq or prereq of its parent    */
    /*             respectively, but a supersede of another ptf in the   */
    /*             list.                                                 */


    if ((exclude_supersede_mode) && ( *(ptf_info->req_type) == 's'  ))
     /* do not print */;
    else{
      if(strcmp(ptf_info->server, no_where) != 0) {
        if(strcmp(ptf_info->server,build_server) == 0)
          dir_out = dir_build;
        else
          if(strcmp(ptf_info->server,prod_server) == 0)
            dir_out = dir_prod;
          else
            dir_out = dir_ship;

        if(dir_out[strlen(dir_out) - 1] == '/')
          possible_slash = "";
        else
          possible_slash = "/";

        fprintf(out_file,"%s%s%s.ptf\n", dir_out, possible_slash, ptf_info->ptf);
     }
   }
  }
} /* end of print_out_ptf */


/*********************************************************************/
/*  remove_ptf_from_ptf_list: remove the given ptf from the ptf_list.*/
/*********************************************************************/

void remove_ptf_from_ptf_list(ptf_list_t *ptf_list, char *ptf_to_remove) {
  ptf_info_t *prev_ptf_info;
  ptf_info_t *ptf_info;

  /* For each ptf */
  prev_ptf_info = NULL;
  rewind_ptf_list(ptf_list);
  while((ptf_info = get_next_ptf_info(ptf_list)) != NULL) {
    /* If this the ptf_info is the ptf that is to be removed */
    if(strcmp(ptf_info->ptf,ptf_to_remove) == 0) {
      /* Remove ptf_info from ptf_list */
      if(ptf_list->start == ptf_info)
        ptf_list->start = ptf_info->next_ptf;
      else
        prev_ptf_info->next_ptf = ptf_info->next_ptf;

      if(ptf_list->last == ptf_info)
        ptf_list->last = prev_ptf_info;
    } else {
      /* Remove the ptf from this ptf's list of requisites */
      prev_ptf_info = ptf_info;
      remove_ptf_from_req_list(ptf_info->req_list, ptf_to_remove);
    }
  }
} /* end of remove_ptf_from_ptf_list */


/*********************************************************************/
/*  remove_ptf_from_req_list: remove the given ptf from the req_list.*/
/*********************************************************************/

void remove_ptf_from_req_list(req_list_t *req_list, char *ptf_to_remove) {
  req_info_t *prev_req_info;
  req_info_t *req_info;

  /* For each ptf */
  prev_req_info = NULL;
  rewind_req_list(req_list);
  while((req_info = get_next_req_info(req_list)) != NULL) {
    /* if this the ptf_into is the ptf that is to be removed */
    if(strcmp(req_info->req_ptf, ptf_to_remove) == 0) {
      /* remove req_info from req_list */
      if(req_list->start == req_info)
        req_list->start = req_info->next_req;
      else
        prev_req_info->next_req = req_info->next_req;

      if(req_list->last == req_info)
        req_list->last = prev_req_info;
    } else
      prev_req_info = req_info;
  }
} /* end of remove_ptf_from_req_list */

/*********************************************************************/
/*  print_out_cyclic_ptfs: print out all relationships remaining     */
/*      in the ptf_list.  The invalid cycle is contained somewhere   */
/*      among these remaining ptfs.                                  */
/*********************************************************************/

void print_out_cyclic_ptfs(ptf_list_t *ptf_list) {
  ptf_info_t *ptf_info;
  req_info_t *req_info;

  printf("\nmedia_list: circular dependencies found in index files\n");
  printf("media_list: cycle is contained in ptfs printed below\n");

  rewind_ptf_list(ptf_list);
  while((ptf_info = get_next_ptf_info(ptf_list)) != NULL) {
    printf("\n%s requisites:\n",ptf_info->ptf);
    rewind_req_list(ptf_info->req_list);
    while((req_info=get_next_req_info(ptf_info->req_list)) != NULL)
      printf("  %s is a %s\n",req_info->req_ptf,req_info->req_type);
  }
} /* end of print_out_cyclic_ptfs */


/*********************************************************************/
/*  create_index_list: this function reads the given index file      */
/*      and returns a pointer to the index list containing an        */
/*      array of index lines read from the index file.               */
/*********************************************************************/

index_list_t *create_index_list(char *server_dir) {
  /* Index file var's */
  index_list_t *index_list;
  int in_fd;
  char *in_name;
  int in_length;

  /* String pointers */
  char *file_buf;
  char *cur_char_ptr;
  char *cur_line_ptr;

  /* Counters and index */
  int line_count;
  int colon_count;
  int i;

  /* Set up in_name (the index file name) */
  in_name = (char *) malloc(strlen(server_dir)+strlen(INDEX)+2);
  check_malloc_return(in_name);
  strcpy(in_name,server_dir);
  if(in_name[strlen(in_name) - 1] != '/')  /* append '/' if needed */
    strcat(in_name,"/");
  strcat(in_name,INDEX);

  /* Open index file, read it into memory, and close it */
  in_fd = open(in_name, O_RDONLY);
  if(in_fd == -1) {
    printf("media_list: can't open: %s\n",in_name);
    exit(1);
  }

  in_length = lseek(in_fd, 0, SEEK_END); /* get length of file */
  lseek(in_fd, 0, SEEK_SET);     /* reset pointer to start of file */
  file_buf = (char *) malloc(in_length+1); /* +1 to have room for  */
  check_malloc_return(file_buf);           /*   end of string    */
  read(in_fd, file_buf, in_length);  /* read file into memory */
  file_buf[in_length] = '\0';        /* put on end of string */
  close(in_fd);

  /* Check format of index file (four colons on each line) and count */
  /* the number of lines for subsequent array allocation             */
  line_count=colon_count=0;
  cur_line_ptr=file_buf;
  for( i=0; i < in_length ; i++) {
    cur_char_ptr = file_buf + i;
    switch(*cur_char_ptr) {
      case ':' : colon_count++; break;
      case '\n':
      if(colon_count != 4) {
        *cur_char_ptr='\0';
        printf("media_list: index file %s has bad data\n",in_name);
        printf("line %d: '%s'\n",line_count+1, cur_line_ptr);
        printf("media_list: process terminated\n");
        exit(1);
      }
      line_count++;
      colon_count=0;
      cur_line_ptr=cur_char_ptr + 1;
    }
  }

  /* Allocate space for index_list */
  index_list = (index_list_t *) malloc(sizeof(index_list_t));
  check_malloc_return(index_list);

  /* Allocate space for index_line array */
  index_list->index_line =
       (index_line_t *) malloc(line_count * sizeof(index_line_t)+1);
  check_malloc_return(index_list->index_line);      /* +1 above ^^ */
        /* so malloc won't bomb if line_count is zero (empty file) */

  /* Set the array of index line pointers to point to the ptfs, lpps, */
  /* and req_type read into memory from the index file.  Replace      */
  /* colons and newlines with '\0' to separate the data.  The format  */
  /* of an index line is 'lpp1:ptf1:req_type:lpp2:ptf2\n'             */
  for( i=0; i < line_count ; i++) {
    if(i==0)
      index_list->index_line[i].lpp1  = strtok(file_buf,":\n");
    else
      index_list->index_line[i].lpp1  = strtok(NULL,":\n");
    index_list->index_line[i].ptf1     = strtok(NULL,":\n");
    index_list->index_line[i].req_type = strtok(NULL,":\n");
    index_list->index_line[i].lpp2     = strtok(NULL,":\n");
    index_list->index_line[i].ptf2     = strtok(NULL,":\n");

    /* check that req_type is valid */
    if((strcmp(index_list->index_line[i].req_type,"coreq")     != 0) &&
       (strcmp(index_list->index_line[i].req_type,"prereq")    != 0) &&
       (strcmp(index_list->index_line[i].req_type,"ifreq")     != 0) &&
       (strcmp(index_list->index_line[i].req_type,"pe-resl")     != 0) &&
       (strcmp(index_list->index_line[i].req_type,"supersede") != 0) ) {
      printf("media_list: index file %s has bad req type\n",in_name);
      printf("line %d: invalid type of '%s'\n",i+1,
             index_list->index_line[i].req_type);
      printf("media_list: process terminated\n");
      exit(1);
    }
  }

  index_list->num_lines = line_count;
  index_list->cur_line = 0;
  return(index_list);
} /* end of create index_list */


/*********************************************************************/
/*  rewind_index_list: set pointer to beginning of index list        */
/*********************************************************************/

void rewind_index_list(index_list_t *index_list) {
  index_list->cur_line = 0;
}


/*********************************************************************/
/*  get_next_index_line: return a pointer to the next line in index  */
/*      list; return NULL when at end of list                        */
/*********************************************************************/

index_line_t *get_next_index_line(index_list_t *index_list) {
  index_line_t *return_ptr;
  if(index_list->cur_line == index_list->num_lines)
    return_ptr = NULL;
  else
    return_ptr = &index_list->index_line[index_list->cur_line++];
  return(return_ptr);
}


/*********************************************************************/
/*  create_ptf_list: this function creates and returns an empty      */
/*      ptf_list (linked list)                                       */
/*********************************************************************/

ptf_list_t *create_ptf_list() {
  ptf_list_t *ptf_list;
  ptf_list = (ptf_list_t *) malloc(sizeof(ptf_list_t));
  check_malloc_return(ptf_list);
  ptf_list->start = NULL;
  ptf_list->last  = NULL;
  ptf_list->current = NULL;
  return(ptf_list);
}


/*********************************************************************/
/*  append_ptf_list:  This function appends the given ptf and its    */
/*      info to the given ptf linked list                            */
/*********************************************************************/

void append_ptf_list(ptf_list_t *ptf_list,
                           char *new_ptf,
                           char *new_server,
                           char *new_parent_ptf,
                           char *new_req_type) {
  ptf_info_t *new_ptf_info;

  /* Allocate memory space for new info */
  new_ptf_info = (ptf_info_t *) malloc(sizeof(ptf_info_t));
  check_malloc_return(new_ptf_info);
  new_ptf_info->ptf = (char *) malloc(strlen(new_ptf)+1);
  check_malloc_return(new_ptf_info->ptf);
  new_ptf_info->parent_ptf = (char *) malloc(strlen(new_parent_ptf)+1);
  check_malloc_return(new_ptf_info->parent_ptf);
  new_ptf_info->req_type = (char *) malloc(strlen(new_req_type)+1);
  check_malloc_return(new_ptf_info->req_type);

  /* Set pointers to point to new info */
  strcpy(new_ptf_info->ptf,new_ptf);
  new_ptf_info->server = new_server;
  strcpy(new_ptf_info->parent_ptf,new_parent_ptf);
  strcpy(new_ptf_info->req_type,new_req_type);
  new_ptf_info->req_list = create_req_list();

  /* Append to the beginning of list ( in final printout,      */
  /* ptfs should appear closer to their requisites if ptfs are */
  /* added to the beginning rather than the end of the list )  */
  new_ptf_info->next_ptf = ptf_list->start;
  if(ptf_list_empty(ptf_list)) {
     ptf_list->start = new_ptf_info;
     ptf_list->last  = new_ptf_info;
  } else {
    ptf_list->start = new_ptf_info;
  }
} /* end of append_ptf_list */


/*********************************************************************/
/*  find_in_ptf_list: This function checks whether the given ptf is  */
/*      in the given ptf linked list.  It returns either the ptf_node*/
/*      if found and otherwise NULL.                                 */
/*********************************************************************/

ptf_info_t *find_in_ptf_list(ptf_list_t *ptf_list,
                                   char *chk_ptf) {
  int found;
  ptf_info_t *cur_info;

  found=FALSE;
  rewind_ptf_list(ptf_list);
  while((!found)&&((cur_info = get_next_ptf_info(ptf_list)) != NULL))
     if(strcmp(cur_info->ptf, chk_ptf) == 0)
        found=TRUE;
  return(cur_info);
} /* end of find_in_ptf_list */


/*********************************************************************/
/*  rewind_ptf_list: set pointer to beginning of ptf list            */
/*********************************************************************/

void rewind_ptf_list(ptf_list_t *ptf_list) {
  ptf_list->current = ptf_list->start;
}


/*********************************************************************/
/*  get_next_ptf_info: return next info in ptf list; return NULL     */
/*      at end of list                                               */
/*********************************************************************/

ptf_info_t *get_next_ptf_info(ptf_list_t *ptf_list) {
  ptf_info_t *return_ptr;
  return_ptr = ptf_list->current;
  if(ptf_list->current != NULL)
    ptf_list->current = ptf_list->current->next_ptf;
  return(return_ptr);
} /* end of get_next_ptf_info */


/*********************************************************************/
/*  ptf_list_empty: returns TRUE if empty, FALSE if not              */
/*********************************************************************/

int ptf_list_empty(ptf_list_t *ptf_list) {
  if(ptf_list->start == NULL)
    return(TRUE);
  else
    return(FALSE);
}


/*********************************************************************/
/*  create_req_list: this function creates and returns an empty      */
/*      requisite list (linked list)                                 */
/*********************************************************************/

req_list_t *create_req_list() {
  req_list_t *req_list;
  req_list = (req_list_t *) malloc(sizeof(req_list_t));
  check_malloc_return(req_list);
  req_list->start = NULL;
  req_list->last  = NULL;
  req_list->current = NULL;
  return(req_list);
}


/*********************************************************************/
/*  append_req_list:  This function appends the given req and the    */
/*      requisite type to the given requisite linked list            */
/*********************************************************************/
void append_req_list(req_list_t *req_list, char *new_req_ptf,
                                           char *new_req_type) {
  req_info_t *new_req_info;

  /* Allocate memory space for new info */
  new_req_info = (req_info_t *) malloc(sizeof(req_info_t));
  check_malloc_return(new_req_info);
  new_req_info->req_ptf = (char *) malloc(strlen(new_req_ptf)+1);
  check_malloc_return(new_req_info->req_ptf);
  new_req_info->req_type = (char *) malloc(strlen(new_req_type)+1);
  check_malloc_return(new_req_info->req_type);

  /* Set pointers to point to new info */
  strcpy(new_req_info->req_ptf,new_req_ptf);
  strcpy(new_req_info->req_type,new_req_type);
  new_req_info->next_req = NULL;

  /* append to end of list */
  if(req_list_empty(req_list)) {
    req_list->start = new_req_info;
    req_list->last  = new_req_info;
  } else {
    req_list->last->next_req = new_req_info;
    req_list->last  = new_req_info;
  }
} /* end of append_req_list */


/*********************************************************************/
/*  find_in_req_list: This function checks whether the given req is  */
/*      in the given req linked list.  It returns either the req_node*/
/*      if found and otherwise NULL.                                 */
/*********************************************************************/

req_info_t *find_in_req_list(req_list_t *req_list,
                                   char *chk_req_ptf) {
  int found;
  req_info_t *cur_info;
  found=FALSE;
  rewind_req_list(req_list);
  while((!found)&&((cur_info = get_next_req_info(req_list)) != NULL))
    if(strcmp(cur_info->req_ptf, chk_req_ptf) == 0)
      found=TRUE;
  return(cur_info);
} /* end of find_in_req_list */


/*********************************************************************/
/*  rewind_req_list: set pointer to beginning of req list            */
/*********************************************************************/

void rewind_req_list(req_list_t *req_list) {
  req_list->current = req_list->start;
} /* end of rewind_req_list */


/*********************************************************************/
/*  get_next_req_info: return next info in req list; return NULL     */
/*      at end of list                                               */
/*********************************************************************/

req_info_t *get_next_req_info(req_list_t *req_list) {
  req_info_t *return_ptr;
  return_ptr = req_list->current;
  if(req_list->current != NULL)
    req_list->current = req_list->current->next_req;
  return(return_ptr);
} /* end of get_next_req_info */


/*********************************************************************/
/*  req_list_empty: returns TRUE if empty, FALSE if not              */
/*********************************************************************/

int req_list_empty(req_list_t *req_list) {
  if(req_list->start == NULL)
    return(TRUE);
  else
    return(FALSE);
}


/*********************************************************************/
/*  create_peresl_list: this function reads the given peresl file    */
/*      and returns a pointer to the peresl list containing an       */
/*      array of peresl PTFs read from the peresl file.              */
/*********************************************************************/

peresl_list_t *create_peresl_list(char *peresl_file) {
  /* Index file var's */
  peresl_list_t *peresl_list;
  peresl_info_t *peresl_info;
  int in_fd;
  int in_length;

  /* String pointers */
  char *file_buf;
  char *cur_line_ptr;
  char *cur_char_ptr;
  char *next_line;
  char *bad_ptf;
  char *resolve;
  char *apar;

  /* Counters and index */
  int line_count;
  int i;

  /* Open peresl file, read it into memory, and close it */
  in_fd = open(peresl_file, O_RDONLY);
  if(in_fd == -1) {
    printf("media_list: can't open: %s\n",peresl_file);
    exit(1);
  }

  in_length = lseek(in_fd, 0, SEEK_END); /* get length of file */
  lseek(in_fd, 0, SEEK_SET);     /* reset pointer to start of file */
  file_buf = (char *) malloc(in_length+1); /* +1 to have room for  */
  check_malloc_return(file_buf);           /*   end of string    */
  read(in_fd, file_buf, in_length);  /* read file into memory */
  file_buf[in_length] = '\0';        /* put on end of string */
  close(in_fd);

  /* Read the peresl and save in lists.  There may be header info    */
  /* and blank lines, so ignore anything that looks junky.           */
  /* Use only those lines with an apar  "IX something".              */
  /* Format is:                                                      */
  /* Bad_ptf  Defect/apar    Fixing_ptf   release                    */
  /*                                                                 */
  /* U407891  69648          U411113      bos320                     */
  /* U412495  IX32661        U412817      R320                       */
  /* U410648  76093          unfixed      unknown                    */

  /* Allocate space for peresl_list */
  peresl_list = (peresl_list_t *) malloc(sizeof(peresl_list_t));
  check_malloc_return(peresl_list);
  peresl_list->start = NULL;
  peresl_list->last  = NULL;
  peresl_list->current = NULL;
  
  line_count=0;
  cur_line_ptr=file_buf;
  for( i=0; i < in_length ; i++) {
      cur_char_ptr = file_buf + i;
      if (*cur_char_ptr != '\n')
          continue;
      *cur_char_ptr = '\0';
      bad_ptf = strtok(cur_line_ptr," \t");
      cur_line_ptr = cur_char_ptr + 1;
      if (! bad_ptf)
          continue;
      if (*bad_ptf != 'U')
          continue;
      if (! is_a_valid_ptf(bad_ptf))
          continue;
      apar = strtok(NULL," \t");
      if (! apar)
          continue;
      if ((*apar != 'I') && (*apar != 'i'))
          continue;
      apar++;
      if ((*apar != 'X') && (*apar != 'x'))
          continue;
      resolve = strtok(NULL," \t");
      if (! resolve)
          continue;
      /* Ignore if Fix same as the bad ptf    */
      if (strcmp(resolve, bad_ptf) == 0)
          continue;
      if (strcmp(resolve, "unfixed") != 0) {
          if (*resolve != 'U')
              continue;
          if (! is_a_valid_ptf(resolve))
              continue;
      }
          
      if (peresl_info = find_in_peresl_list(peresl_list, bad_ptf)) {
         if (find_in_req_list(peresl_info->req_list,resolve) == NULL) {
            append_req_list(peresl_info->req_list, resolve, peresl);
         }
      }
      else {
          append_peresl_list(peresl_list, bad_ptf, resolve);
      }
  }

  return(peresl_list);
} /* end of create peresl_list */

/*********************************************************************/
/*      index list and adds peresl ptfs it finds to the ptf_list.    */
/*      It sets loop_finished=FALSE if any new peresl ptfs were      */
/*      added to the ptf list.                                       */
/*********************************************************************/

void find_peresl_ptfs(peresl_info_t *peresl_list,
                             ptf_list_t *ptf_list ) {
  peresl_info_t *cur_line;
  req_info_t *cur_req;
  ptf_info_t *ptf_info;

  /* For each line in the peresl file */
  rewind_peresl_list(peresl_list);
  while((cur_line = get_next_peresl_info(peresl_list)) != NULL) {
    /* Add the resolving ptf to the ptf list if the ptf on the left  */
    /* (ptf1) is in the list, and its resolver on the right (ptf2)  */
    /* is not in the list and it is actually a valid ptf          */
    if((ptf_info = find_in_ptf_list(ptf_list, cur_line->bad_ptf)) != NULL) {
      /* For each resolving ptf in the peresl file */
      rewind_req_list(cur_line->req_list);
      while((cur_req = get_next_req_info(cur_line->req_list)) != NULL) {
        if (find_in_ptf_list(ptf_list, cur_req->req_ptf) == NULL) {
            if (strcmp(cur_req->req_ptf, "unfixed") == 0) {
                printf("media_list: WARNING: unfixed pe-resl for %s\n",cur_line->bad_ptf);
                *cur_req->req_ptf = 'U'; /* Telling once is enough. */
            }
            if (is_a_valid_ptf(cur_req->req_ptf)) {
                append_ptf_list(ptf_list, cur_req->req_ptf, no_where,
                          cur_line->bad_ptf, peresl);
                if(verbose)
                    printf("%s is a pe-resl of %s\n", cur_req->req_ptf, cur_line->bad_ptf);
                loop_finished = FALSE;
            }
        }
      }
    }
  }
}  /* end of find_peresl_ptfs */


/*********************************************************************/
/*  peresl_list_empty: returns TRUE if empty, FALSE if not           */
/*********************************************************************/

int peresl_list_empty(peresl_list_t *peresl_list) {
  if(peresl_list->start == NULL)
    return(TRUE);
  else
    return(FALSE);
}


/*********************************************************************/
/*  append_peresl_list:  This function appends the given ptf and its    */
/*      info to the given peresl linked list                            */
/*********************************************************************/

void append_peresl_list(peresl_list_t *peresl_list,
                           char *new_ptf,
                           char *new_peresl_ptf) {
  peresl_info_t *new_peresl_info;

  /* Allocate memory space for new info */
  new_peresl_info = (peresl_info_t *) malloc(sizeof(peresl_info_t));
  check_malloc_return(new_peresl_info);
  new_peresl_info->bad_ptf = (char *) malloc(strlen(new_ptf)+1);
  check_malloc_return(new_peresl_info->bad_ptf);

  /* Set pointers to point to new info */
  strcpy(new_peresl_info->bad_ptf,new_ptf);
  new_peresl_info->req_list = create_req_list();
  append_req_list(new_peresl_info->req_list,new_peresl_ptf, peresl);

  new_peresl_info->next_peresl = peresl_list->start;
  if(peresl_list_empty(peresl_list)) {
     peresl_list->start = new_peresl_info;
     peresl_list->last  = new_peresl_info;
  } else {
    peresl_list->start = new_peresl_info;
  }
} /* end of append_peresl_list */


/*********************************************************************/
/*  find_in_peresl_list: This function checks whether the given ptf  */
/*      is in the given peresl linked list.  It returns either the   */
/*      peresl pointer if found or  otherwise NULL.                  */
/*********************************************************************/

peresl_info_t *find_in_peresl_list(peresl_list_t *peresl_list,
                                   char *chk_ptf) {
  int found;
  peresl_info_t *cur_info;

  found=FALSE;
  rewind_peresl_list(peresl_list);
  while((!found)&&((cur_info = get_next_peresl_info(peresl_list)) != NULL))
     if(strcmp(cur_info->bad_ptf, chk_ptf) == 0)
        found=TRUE;
  return(cur_info);
} /* end of find_in_peresl_list */


/*********************************************************************/
/*  rewind_peresl_list: set pointer to beginning of peresl list      */
/*********************************************************************/

void rewind_peresl_list(peresl_list_t *peresl_list) {
  peresl_list->current = peresl_list->start;
} /* end of rewind_peresl_list */


/*********************************************************************/
/*  get_next_peresl_info: return next info in peresl list;           */
/*      return NULL at end of list                                   */
/*********************************************************************/

peresl_info_t *get_next_peresl_info(peresl_list_t *peresl_list) {
  peresl_info_t *return_ptr;
  return_ptr = peresl_list->current;
  if(peresl_list->current != NULL)
    peresl_list->current = peresl_list->current->next_peresl;
  return(return_ptr);
} /* end of get_next_peresl_info */


/*********************************************************************/
/*  mlevel_list_empty: returns TRUE if empty, FALSE if not           */
/*********************************************************************/

int mlevel_list_empty(mlevel_list_t *mlevel_list) {
  if(mlevel_list->start == NULL)
    return(TRUE);
  else
    return(FALSE);
}


/*********************************************************************/
/*  create_mlevel_list: this function reads the given mlevel file    */
/*      and returns a pointer to the mlevel list containing an       */
/*      array of mlevel PTFs read from the mlevel file.              */
/*      If no mlevel file was specified, this function simply        */
/*      creates an empty mlevel list.                                */
/*********************************************************************/

mlevel_list_t *create_mlevel_list(char *mlevel_file) {
  /* Input file var's */
  mlevel_info_t *mlevel_info;
  int in_fd;
  int in_length;

  /* String pointers */
  char *file_buf;
  char *cur_line_ptr;
  char *cur_char_ptr;
  char *ptf;

  /* Counter */
  int i;

  /* Allocate space for mlevel_list */
  mlevel_list = (mlevel_list_t *) malloc(sizeof(mlevel_list_t));
  check_malloc_return(mlevel_list);
  mlevel_list->start = NULL;
  mlevel_list->last  = NULL;
  mlevel_list->current = NULL;
  
  /* If a put level was not specified, return empty mlevel list */
  if(mlevel_file == NULL)
    return(mlevel_list);

  /* Open mlevel file, read it into memory, and close it */
  in_fd = open(mlevel_file, O_RDONLY);
  if(in_fd == -1) {
    printf("media_list: can't open: %s\n",mlevel_file);
    exit(1);
  }

  in_length = lseek(in_fd, 0, SEEK_END); /* get length of file */
  lseek(in_fd, 0, SEEK_SET);     /* reset pointer to start of file */
  file_buf = (char *) malloc(in_length+1); /* +1 to have room for  */
  check_malloc_return(file_buf);           /*   end of string    */
  read(in_fd, file_buf, in_length);  /* read file into memory */
  file_buf[in_length] = '\0';        /* put on end of string */
  close(in_fd);

  /* Read the mlevel file and save in a list.  */

  cur_line_ptr=file_buf;
  for( i=0; i < in_length ; i++) {
      cur_char_ptr = file_buf + i;
      if (*cur_char_ptr != '\n')
          continue;
      *cur_char_ptr = '\0';
      ptf = strtok(cur_line_ptr," \t");
      cur_line_ptr = cur_char_ptr + 1;
      if (! ptf)
          continue;
      if (*ptf != 'U')
          continue;
      if (! is_a_valid_ptf(ptf))
          continue;
          
      append_mlevel_list(mlevel_list, ptf);
  }

  return(mlevel_list);
} /* end of create mlevel_list */


/*********************************************************************/
/*  append_mlevel_list:  This function appends the given ptf         */
/*      to the given mlevel linked list                              */
/*********************************************************************/

void append_mlevel_list(mlevel_list_t *mlevel_list,
                           char *new_ptf) {
  mlevel_info_t *new_mlevel_info;

  /* Allocate memory space for new info */
  new_mlevel_info = (mlevel_info_t *) malloc(sizeof(mlevel_info_t));
  check_malloc_return(new_mlevel_info);
  new_mlevel_info->ptf = (char *) malloc(strlen(new_ptf)+1);
  check_malloc_return(new_mlevel_info->ptf);

  /* Set pointers to point to new info */
  strcpy(new_mlevel_info->ptf,new_ptf);

  new_mlevel_info->next_mlevel = mlevel_list->start;
  if(mlevel_list_empty(mlevel_list)) {
     mlevel_list->start = new_mlevel_info;
     mlevel_list->last  = new_mlevel_info;
  } else {
    mlevel_list->start = new_mlevel_info;
  }
} /* end of append_mlevel_list */


/*********************************************************************/
/*  find_in_mlevel_list: This function checks whether the given ptf  */
/*      is in the maintlevel linked list.                            */
/*********************************************************************/

int find_in_mlevel_list(char *chk_ptf) {
                                   
  int found;
  mlevel_info_t *cur_info;

  found=FALSE;
  rewind_mlevel_list(mlevel_list);
  while((!found)&&((cur_info = get_next_mlevel_info(mlevel_list)) != NULL))
     if(strcmp(cur_info->ptf, chk_ptf) == 0)
        found=TRUE;
  return(found);
} /* end of find_in_mlevel_list */


/*********************************************************************/
/*  get_next_mlevel_info: return next info in mlevel list;           */
/*      return NULL at end of list                                   */
/*********************************************************************/

mlevel_info_t *get_next_mlevel_info(mlevel_list_t *mlevel_list) {
  mlevel_info_t *return_ptr;
  return_ptr = mlevel_list->current;
  if(mlevel_list->current != NULL)
    mlevel_list->current = mlevel_list->current->next_mlevel;
  return(return_ptr);
} /* end of get_next_mlevel_info */


/*********************************************************************/
/*  rewind_mlevel_list: set pointer to beginning of mlevel list      */
/*********************************************************************/

void rewind_mlevel_list(mlevel_list_t *mlevel_list) {
  mlevel_list->current = mlevel_list->start;
} /* end of rewind_mlevel_list */


/*********************************************************************/
/*  save_current_posn: save current position in linked lists         */
/*********************************************************************/

void save_current_posn(ptf_list_t *ptf_list,
                       req_list_t *req_list) {
  save_ptf_list_current = ptf_list->current;
  save_req_list_current = req_list->current;
}


/*********************************************************************/
/*  restore_current_posn: restore saved position in linked lists     */
/*********************************************************************/

void restore_current_posn(ptf_list_t *ptf_list,req_list_t *req_list) {
  ptf_list->current = save_ptf_list_current;
  req_list->current = save_req_list_current;
}


/*********************************************************************/
/*  check_malloc_return: exit out if malloc failed ( ptr = null )    */
/*********************************************************************/

void check_malloc_return(char *ptr) {
  if (ptr == NULL) {
    printf("media_list: malloc was unable to allocate memory\n");
    printf("media_list: program terminated\n");
    exit(1);
  }
}


/*********************************************************************/
/*  is_a_valid_ptf: checks if last 5 chars are digits                */
/*                  returns TRUE or FALSE                            */
/*********************************************************************/

int is_a_valid_ptf(char *ptf) {
  if(strspn(ptf + strlen(ptf) - 5, "1234567890") == 5)
    return(TRUE);
  else
    return(FALSE);
}


/*********************************************************************/
/*  help:  print syntax and exit                                     */
/*********************************************************************/

void help() {
  printf("\n");
  printf("%s\n",sccsid);
  printf("Usage\n");
  printf("media_list    [-s dirname_ship] [-p dirname_prod]\n");
  printf("              [-b dirname_build] [-v] [-e] [-x] [-t] [-n] [-h] [-?]\n");
  printf("              [-r peresl_filename ]\n");
  printf("              [-o out_filename ] -l mif_ccss_list_filename\n");
  printf("     where:                                           \n");
  printf("            dirname_ship  - path to mif directory on 'ship' server.\n");
  printf("            dirname_prod  - path to mif directory on 'production' server.\n");
  printf("            dirname_build - path to mif directory on 'build' server.\n");
  printf("            -v         verbose option -  prints extra info\n");
  printf("            -l mif_ccss_list_filename - contains list\n");
  printf("                            of mif/ccss files to package.\n");
  printf("            -o outfilename - path & filename for ordered \n");
  printf("                             list of prereqs.\n");
  printf("            -r peresl_filename - path & filename of PTF error \n");
  printf("                             resolution file.\n");
  printf("            -e  - exclude requisites\n");
  printf("            -x  - exclude supersedes\n");
  printf("            -t  - duplicate PTF's allowed on different servers.\n");
  printf("            -n  - no preventative supersedes\n");
  printf("            -h | -?   options to list usage message\n");
  printf("\n");
  exit(1);
} /* end of help function */

/* END OF PROGRAM */

